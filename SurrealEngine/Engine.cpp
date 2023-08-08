
#include "Precomp.h"
#include "Engine.h"
#include "File.h"
#include "Render/RenderSubsystem.h"
#include "Package/PackageManager.h"
#include "Package/ObjectStream.h"
#include "UObject/ULevel.h"
#include "UObject/UFont.h"
#include "UObject/UMesh.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/UMusic.h"
#include "UObject/USound.h"
#include "UObject/UClass.h"
#include "UObject/UClient.h"
#include "UObject/NativeObjExtractor.h"
#include "Math/quaternion.h"
#include "Math/FrustumPlanes.h"
#include "Window/Window.h"
#include "RenderDevice/RenderDevice.h"
#include "Audio/AudioSubsystem.h"
#include "VM/Frame.h"
#include "VM/NativeFuncExtractor.h"
#include "VM/ScriptCall.h"
#include "UI/Debugger/DebuggerWindow.h"
#include <chrono>
#include <set>

Engine* engine = nullptr;

Engine::Engine(GameLaunchInfo launchinfo) : LaunchInfo(launchinfo)
{
	engine = this;

	packages = std::make_unique<PackageManager>(LaunchInfo.folder, LaunchInfo.engineVersion, LaunchInfo.gameName);
}

Engine::~Engine()
{
	engine = nullptr;
}

void Engine::Run()
{
	std::srand((unsigned int)std::time(nullptr));

	LoadEngineSettings();
	LoadKeybindings();

	window = DisplayWindow::Create(this);

	int width = StartupFullscreen ? FullscreenViewportX : WindowedViewportX;
	int height = StartupFullscreen ? FullscreenViewportY : WindowedViewportY;

	window->OpenWindow(width, height, StartupFullscreen);

	audio = std::make_unique<AudioSubsystem>();
	render = std::make_unique<RenderSubsystem>(window->GetRenderDevice());

	client = UObject::Cast<UClient>(packages->NewObject("client", "Engine", "Client"));
	viewport = UObject::Cast<UViewport>(packages->NewObject("viewport", "Engine", "Viewport"));
	canvas = UObject::Cast<UCanvas>(packages->NewObject("canvas", "Engine", "Canvas"));

	std::string consolestr = packages->GetIniValue("system", "Engine.Engine", "Console");
	std::string consolepkg = consolestr.substr(0, consolestr.find('.'));
	std::string consolecls = consolestr.substr(consolestr.find('.') + 1);
	console = UObject::Cast<UConsole>(packages->NewObject("console", consolepkg, consolecls));

	console->Viewport() = viewport;
	canvas->Viewport() = viewport;
	viewport->Console() = console;

	if (!LaunchInfo.noEntryMap)
		LoadEntryMap();

	if (LaunchInfo.url.empty())
		LoadMap(GetDefaultURL(packages->GetIniValue("system", "URL", "LocalMap")));
	else
		LoadMap(UnrealURL(GetDefaultURL(packages->GetIniValue("system", "URL", "LocalMap")), LaunchInfo.url));

	LoginPlayer();

	bool firstCall = true;
	while (!quit)
	{
		float realTimeElapsed = CalcTimeElapsed();
		float entryLevelElapsed = EntryLevel ? clamp(realTimeElapsed * EntryLevelInfo->TimeDilation(), 1.0f / 400.0f, 1.0f / 2.5f) : 0.0f;
		float levelElapsed = clamp(realTimeElapsed * LevelInfo->TimeDilation(), 1.0f / 400.0f, 1.0f / 2.5f);

		if (EntryLevel)
			EntryLevelInfo->TimeSeconds() += entryLevelElapsed;
		LevelInfo->TimeSeconds() += levelElapsed;

		UpdateInput(realTimeElapsed);

		CallEvent(console, EventName::Tick, { ExpressionValue::FloatValue(levelElapsed) });

		// To do: set these to true if the frame rate is too low
		LevelInfo->bDropDetail() = false;
		LevelInfo->bAggressiveLOD() = false;

		if (EntryLevel)
			EntryLevel->Tick(entryLevelElapsed);
		Level->Tick(levelElapsed);

		if (!LevelInfo->NextURL().empty())
		{
			LevelInfo->NextSwitchCountdown() -= levelElapsed;
			if (LevelInfo->NextSwitchCountdown() <= 0.0f)
			{
				if (LevelInfo->NextURL() == "?RESTART")
				{
					LoadMap(LevelInfo->URL, Level->TravelInfo);
					LoginPlayer();
				}
				else if (LevelInfo->bNextItems())
				{
					auto travelInfo = Level->TravelInfo;
					for (UActor* actor : Level->Actors)
					{
						UPlayerPawn* pawn = UObject::TryCast<UPlayerPawn>(actor);
						if (pawn && pawn->Player())
						{
							std::vector<ObjectTravelInfo> actorTravelInfo;
							for (UInventory* item = pawn->Inventory(); item != nullptr; item = item->Inventory())
							{
								ObjectTravelInfo objInfo(item);
								actorTravelInfo.push_back(std::move(objInfo));
							}
							std::string playerName = pawn->PlayerReplicationInfo()->PlayerName();
							travelInfo[playerName] = ObjectTravelInfo::ToString(actorTravelInfo);
						}
					}
					LoadMap(UnrealURL(LevelInfo->URL, LevelInfo->NextURL()), travelInfo);
					LoginPlayer();
				}
				else
				{
					LoadMap(UnrealURL(LevelInfo->URL, LevelInfo->NextURL()), {});
					LoginPlayer();
				}
			}
		}

		if (!ClientTravelInfo.URL.empty())
		{
			// To do: need to do something about that travel type and transfering of items

			UnrealURL url(LevelInfo->URL, ClientTravelInfo.URL);
			LogMessage("Client travel to " + url.ToString());
			LoadMap(url);
			LoginPlayer();
		}

		// To do: improve CallEvent so parameter passing isn't this painful
		UFunction* funcPlayerCalcView = FindEventFunction(viewport->Actor(), "PlayerCalcView");
		if (funcPlayerCalcView)
		{
			UObjectProperty objprop({}, nullptr, ObjectFlags::NoFlags);
			UStructProperty vecprop({}, nullptr, ObjectFlags::NoFlags);
			UStructProperty rotprop({}, nullptr, ObjectFlags::NoFlags);
			vecprop.Struct = UObject::Cast<UStructProperty>(funcPlayerCalcView->Properties[1])->Struct;
			rotprop.Struct = UObject::Cast<UStructProperty>(funcPlayerCalcView->Properties[2])->Struct;
			CameraActor = viewport->Actor();
			CameraLocation = viewport->Actor()->Location();
			CameraRotation = viewport->Actor()->Rotation();
			CameraFovAngle = viewport->Actor()->FovAngle();
			CallEvent(viewport->Actor(), EventName::PlayerCalcView, {
				ExpressionValue::Variable(&CameraActor, &objprop),
				ExpressionValue::Variable(&CameraLocation, &vecprop),
				ExpressionValue::Variable(&CameraRotation, &rotprop)
				});
		}

		UpdateAudio();

		render->DrawGame(levelElapsed);
	}
}

void Engine::UpdateAudio()
{
	mat4 translate = mat4::translate(vec3(0.0f) - CameraLocation);
	mat4 listener = Coords::ViewToAudioDev().ToMatrix() * Coords::Rotation(CameraRotation).ToMatrix() * translate;

	audio->SetViewport(viewport);
	audio->Update(listener);
}

void Engine::ClientTravel(const std::string& newURL, uint8_t travelType, bool transferItems)
{
	ClientTravelInfo.URL = newURL;
	ClientTravelInfo.TravelType = travelType;
	ClientTravelInfo.TransferItems = transferItems;
}

UnrealURL Engine::GetDefaultURL(const std::string& map)
{
	UnrealURL url;
	url.Map = map;
	for (std::string optionKey : { "Name", "Class", "team", "skin", "Face", "Voice", "OverrideClass" })
	{
		url.Options.push_back(optionKey + "=" + packages->GetIniValue("user", "DefaultPlayer", optionKey));
	}
	return url;
}

void Engine::LoadEntryMap()
{
	// The entry map is the map you see in the game when no other map is playing. For example when disconnected from a server. It is always loaded and running.
	LoadMap(GetDefaultURL("Entry.unr"));
	EntryLevelInfo = LevelInfo;
	EntryLevel = Level;
	LevelInfo = nullptr;
	Level = nullptr;
}

void Engine::LoadMap(const UnrealURL& url, const std::map<std::string, std::string>& travelInfo)
{
	ClientTravelInfo.URL.clear();

	if (Level)
		CallEvent(console, EventName::NotifyLevelChange);

	if (url.HasOption("entry")) // Not sure what the purpose of this kind of travel is - do nothing for now.
		return;

	// Load map objects
	Package* package = packages->GetPackage(FilePath::remove_extension(url.Map));

	LevelInfo = UObject::Cast<ULevelInfo>(package->GetUObject("LevelInfo", "LevelInfo0"));
	if (packages->IsUnreal1())
	{
		for (int grr = 1; !LevelInfo && grr < 20; grr++)
			LevelInfo = UObject::Cast<ULevelInfo>(package->GetUObject("LevelInfo", "LevelInfo" + std::to_string(grr)));
	}
	if (!LevelInfo)
		throw std::runtime_error("Could not find the LevelInfo object for this map!");

	LevelInfo->ComputerName() = "MyComputer";
	LevelInfo->HubStackLevel() = 0; // To do: handle level hubs
	LevelInfo->EngineVersion() = "500";
	if (packages->GetEngineVersion() > 219)
		LevelInfo->MinNetVersion() = "500";
	LevelInfo->bHighDetailMode() = true;
	LevelInfo->NetMode() = 0; // NM_StandAlone

	LevelInfo->URL = url;

	Level = UObject::Cast<ULevel>(package->GetUObject("Level", "MyLevel"));
	if (!Level)
		throw std::runtime_error("Could not find the Level object for this map!");

	Level->TravelInfo = travelInfo; // Initially used travel info for level restart

	// Remove the actors meant for the editor (to do: should we do this at the package manager level?)
	for (UActor*& actor : Level->Actors)
	{
		if (actor && AllFlags(actor->Flags, ObjectFlags::NotForServer))
		{
			actor->bDeleteMe() = true;
			actor = nullptr;
		}
	}

	// Link actors to the level
	for (UActor* actor : Level->Actors)
	{
		if (actor)
		{
			actor->XLevel() = Level;
			Level->Hash.AddToCollision(actor);
		}
	}

	// Find the game info class
	UClass* gameInfoClass = packages->FindClass(LevelInfo->URL.GetOption("game"));
	if (!gameInfoClass)
		gameInfoClass = LevelInfo->DefaultGameType();
	if (!gameInfoClass)
		gameInfoClass = packages->FindClass(packages->GetIniValue("system", "Engine.Engine", "DefaultGame"));
	if (!gameInfoClass)
		gameInfoClass = packages->FindClass("Botpack.DeathMatchPlus");
	if (!gameInfoClass)
		throw std::runtime_error("Could not find any gameinfo class!");

	// Spawn GameInfo actor
	GameInfo = UObject::Cast<UGameInfo>(packages->NewObject("gameinfo", gameInfoClass));
	GameInfo->XLevel() = Level;
	GameInfo->Level() = LevelInfo;
	Level->Hash.AddToCollision(GameInfo);
	GameInfo->Tag() = gameInfoClass->Name;
	GameInfo->bTicked() = false;
	GameInfo->InitActorZone();

	Level->Actors.push_back(GameInfo);

	// Note: this is never true. But maybe it will be once map loading or level hubs are implemented? If not, delete it!
	if (LevelInfo->bBegunPlay())
	{
		CallEvent(GameInfo, EventName::Spawned);
		CallEvent(GameInfo, EventName::PreBeginPlay);
		CallEvent(GameInfo, EventName::BeginPlay);
		CallEvent(GameInfo, EventName::PostBeginPlay);
		CallEvent(GameInfo, EventName::SetInitialState);

		if (packages->GetEngineVersion() > 219)
		{
			NameString attachTag = GameInfo->AttachTag();
			if (!attachTag.IsNone())
			{
				for (UActor* actor : Level->Actors)
				{
					if (actor && actor->Tag() == attachTag)
					{
						actor->SetBase(GameInfo, false);
					}
				}
			}
		}
		if (packages->GetEngineVersion() >= 400)
		{
			for (USpawnNotify* notifyObj = LevelInfo->SpawnNotify(); notifyObj != nullptr; notifyObj = notifyObj->Next())
			{
				UClass* cls = notifyObj->ActorClass();
				if (cls && GameInfo->IsA(cls->Name))
					GameInfo = UObject::Cast<UGameInfo>(CallEvent(notifyObj, EventName::SpawnNotification, { ExpressionValue::ObjectValue(GameInfo) }).ToObject());
			}
		}
	}
	LevelInfo->Game() = GameInfo;

	if (!LevelInfo->bBegunPlay())
	{
		LevelInfo->TimeSeconds() = 0.0f;
		LevelInfo->bBegunPlay() = true;
	}

	std::string options = url.GetOptions();

	UStringProperty stringProp("", nullptr, ObjectFlags::NoFlags);
	std::string error;

	LevelInfo->bStartup() = true;
	CallEvent(GameInfo, EventName::InitGame, { ExpressionValue::StringValue(options), ExpressionValue::Variable(&error, &stringProp) });
	if (!error.empty())
		throw std::runtime_error("InitGame failed: " + error);
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, EventName::PreBeginPlay); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, EventName::BeginPlay); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, EventName::PostBeginPlay); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, EventName::SetInitialState); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) actor->InitBase(); }
	LevelInfo->bStartup() = false;

	audio->StopSounds();
}

void Engine::LoginPlayer()
{
	UnrealURL url = LevelInfo->URL;
	std::map<std::string, std::string> travelInfo = Level->TravelInfo;

	UStringProperty stringProp("", nullptr, ObjectFlags::NoFlags);
	std::string error, failcode;

	std::string portal = url.GetPortal();
	std::string options = url.GetOptions();

	std::string playerPawnClass = url.GetOption("Class");
	if (playerPawnClass.empty())
		playerPawnClass = packages->GetIniValue("system", "URL", "Class");
	UClass* pawnClass = packages->FindClass(playerPawnClass);

	// Perform PreLogin check (supposedly, good for early rejection in network games)
	CallEvent(LevelInfo->Game(), EventName::PreLogin, {
		ExpressionValue::StringValue(options),
		ExpressionValue::Variable(&error, &stringProp),
		ExpressionValue::Variable(&failcode, &stringProp),
		});
	if (!error.empty() || !failcode.empty())
		throw std::runtime_error("GameInfo prelogin failed: " + error + " (" + failcode + ")");

	// Create viewport pawn
	size_t numActors = Level->Actors.size();
	UPlayerPawn* pawn = UObject::Cast<UPlayerPawn>(CallEvent(LevelInfo->Game(), EventName::Login, {
		ExpressionValue::StringValue(portal),
		ExpressionValue::StringValue(options),
		ExpressionValue::Variable(&error, &stringProp),
		ExpressionValue::ObjectValue(pawnClass)
		}).ToObject());
	if (!pawn || !error.empty())
		throw std::runtime_error("GameInfo login failed: " + error);
	bool actorActuallySpawned = Level->Actors.size() != numActors;

	// Assign the pawn to the viewport
	viewport->Actor() = pawn;
	viewport->Actor()->Player() = viewport;
	CallEvent(viewport->Actor(), EventName::Possess);

	// Transfer travel actors to the new map

	CallEvent(pawn, EventName::TravelPreAccept);

	std::vector<std::pair<UActor*, ObjectTravelInfo>> acceptedActors;
	if (actorActuallySpawned)
	{
		std::string playerName = url.GetOption("Name");
		if (playerName.empty())
			playerName = packages->GetIniValue("system", "URL", "Name");

		auto it = travelInfo.find(playerName);
		if (!playerName.empty() && it != travelInfo.end())
		{
			for (const ObjectTravelInfo& objInfo : ObjectTravelInfo::Parse(it->second))
			{
				UClass* cls = packages->FindClass(objInfo.ClassName);
				UActor* acceptedActor = nullptr;
				if (cls)
					acceptedActor = pawn->Spawn(cls, nullptr, NameString(), nullptr, nullptr);

				if (acceptedActor)
				{
					acceptedActors.push_back({ acceptedActor, objInfo });
				}
				else
				{
					LogMessage("Could not spawn travelling actor " + objInfo.ClassName);
				}
			}

			for (auto& it : acceptedActors)
			{
				UActor* acceptedActor = it.first;
				const ObjectTravelInfo& objInfo = it.second;

				// To do: load properties
			}
		}
	}

	for (auto it = acceptedActors.rbegin(); it != acceptedActors.rend(); ++it)
		CallEvent((*it).first, EventName::TravelPreAccept);

	CallEvent(LevelInfo->Game(), EventName::AcceptInventory, { ExpressionValue::ObjectValue(pawn) });

	for (auto it = acceptedActors.rbegin(); it != acceptedActors.rend(); ++it)
		CallEvent((*it).first, EventName::TravelPostAccept);

	CallEvent(pawn, EventName::TravelPostAccept);
	CallEvent(LevelInfo->Game(), EventName::PostLogin, { ExpressionValue::ObjectValue(pawn) });

	render->OnMapLoaded();
}

float Engine::CalcTimeElapsed()
{
	using namespace std::chrono;

	uint64_t currentTime = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
	if (lastTime == 0)
		lastTime = currentTime;

	uint64_t deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	return clamp(deltaTime / 1'000'000.0f, 0.0f, 1.0f);
}

std::string Engine::ConsoleCommand(UObject* context, const std::string& commandline, bool& found)
{
	found = false;

	std::vector<std::string> args = GetArgs(commandline);
	if (args.empty())
	{
		return {};
	}

	std::string command = args[0];
	for (char& c : command) c = std::tolower(c);

	found = true;
	if (command == "exit" || command == "quit")
	{
		quit = true;
	}
	else if (command == "timedemo" && args.size() == 2)
	{
		render->ShowTimedemoStats = args[1] == "1";
	}
	else if (command == "collisiondebug" && args.size() == 2)
	{
		render->ShowCollisionDebug = args[1] == "1";
	}
	else if (command == "showlog")
	{
		//Frame::ShowDebuggerWindow();
	}
	/*else if (command == "playsong")
	{
		auto music = LevelInfo->Song();
		if (music)
			audio->PlayMusic(AudioSource::CreateMod(music->Data, true, 0, LevelInfo->SongSection()));
	}
	else if (command == "stopsong")
	{
		audio->PlayMusic(nullptr);
	}*/
	else if (command == "getres")
	{
		// TODO: actually query this
		return "1920x1080 1024x768 800x600";
	}
	else if (command == "getcolordepths")
	{
		return "32 16";
	}
	else if (command == "getcurrentres")
	{
		return "1920x1080";
	}
	else if (command == "getcurrentcolordepth")
	{
		return "32";
	}
	else if (command == "getping")
	{
		return "0";
	}
	else if (command == "getloss")
	{
		return "0";
	}
	else if (command == "keyname" && args.size() == 2)
	{
		int index = std::atoi(args[1].c_str());
		return keynames[(uint8_t)index];
	}
	else if (command == "keybinding" && args.size() == 2)
	{
		std::string name = args[1];
		return keybindings[name];
	}
	else if (command == "get" && args.size() == 3)
	{
		std::string section = args[1];
		std::string key = args[2];
		if (section.size() > 4 && section.substr(0, 4) == "ini:")
		{
			size_t pos = section.find_first_of('.', 4);
			if (pos != std::string::npos)
			{
				std::string packageName = section.substr(4, pos - 4);
				std::string sectionName = section.substr(pos + 1);
				if (packageName == "Engine" || packageName == "Core")
					packageName = "system";
				try
				{
					return packages->GetIniValue(packageName, sectionName, key);
				}
				catch (const std::exception& e)
				{
					LogMessage("Could not get ini value from " + section + ": " + e.what());
				}
			}
		}
		else if (section == "windrv.windowsclient")
		{
			if (key == "usejoystick")
			{
				return "0";
			}
			else if (key == "UseDirectInput")
			{
				return "1";
			}
		}

		LogMessage("Unknown get: " + commandline);
	}
	else if (command == "set" && args.size() == 4)
	{
		LogUnimplemented("Set command is not implemented: " + commandline);
	}
	else if (command == "setres" && args.size() == 2)
	{
		LogUnimplemented("SetRes command is not implemented: " + commandline);
	}
	else
	{
		if (!ExecCommand(args))
		{
			LogMessage("Unknown command: " + commandline);
			found = false;
		}
	}
	return {};
}

std::vector<std::string> Engine::GetArgs(const std::string& commandline)
{
	std::vector<std::string> args;
	size_t i = 0;
	while (i < commandline.size())
	{
		size_t j = commandline.find_first_not_of(" \t", i);
		if (j == std::string::npos)
			break;
		i = j;
		j = commandline.find_first_of(" \t", i);
		if (j == std::string::npos)
			j = commandline.size();
		if (j > i)
			args.push_back(commandline.substr(i, j - i));
		i = j;
	}
	return args;
}

std::vector<std::string> Engine::GetSubcommands(const std::string& command)
{
	std::vector<std::string> subcommands;
	size_t pos = 0;
	while (pos < command.size())
	{
		size_t endpos = command.find('|', pos);
		if (endpos == std::string::npos)
			endpos = command.size();

		std::string subcommand = command.substr(pos, endpos - pos);
		if (!subcommand.empty())
			subcommands.push_back(subcommand);
		pos = endpos + 1;
	}
	return subcommands;
}

void Engine::LoadEngineSettings()
{
	WindowedViewportX = std::stoi(packages->GetIniValue(LaunchInfo.gameName, "WinDrv.WindowsClient", "WindowedViewportX"));
	WindowedViewportY = std::stoi(packages->GetIniValue(LaunchInfo.gameName, "WinDrv.WindowsClient", "WindowedViewportY"));
	FullscreenViewportX = std::stoi(packages->GetIniValue(LaunchInfo.gameName, "WinDrv.WindowsClient", "FullscreenViewportX"));
	FullscreenViewportY = std::stoi(packages->GetIniValue(LaunchInfo.gameName, "WinDrv.WindowsClient", "FullscreenViewportY"));
	StartupFullscreen = packages->GetIniValue(LaunchInfo.gameName, "WinDrv.WindowsClient", "StartupFullscreen") == "True";
}

void Engine::LoadKeybindings()
{
	for (int i = 0; i < 256; i++)
	{
		std::string keyname = keynames[i];
		keybindings[keyname] = packages->GetIniValue("user", "Engine.Input", keyname);
	}

	for (int i = 0; i < 40; i++)
	{
		std::string alias = packages->GetIniValue("user", "Engine.Input", "Aliases[" + std::to_string(i) + "]");

		// Total trash parsing, but it will do for the aliases I have! Feel free to improve it!
		std::string commandStart = "(Command=\"";
		std::string commandSplit = "\",Alias=";
		std::string commandEnd = ")";
		if (alias.size() > commandStart.size() + commandSplit.size() + commandEnd.size())
		{
			size_t pos = alias.find(commandSplit, commandStart.size());
			if (pos != std::string::npos)
			{
				size_t pos2 = alias.find(commandEnd, pos);
				if (pos2 != std::string::npos)
				{
					std::string aliasCommand = alias.substr(commandStart.size(), pos - commandStart.size());
					std::string aliasName = alias.substr(pos + commandSplit.size(), pos2 - pos - commandSplit.size());
					if (!aliasName.empty() && aliasName != "None")
						inputAliases[aliasName] = aliasCommand;
				}
			}
		}
	}
}

void Engine::UpdateInput(float timeElapsed)
{
	if (timeElapsed <= 0.0f)
		return;

	InputEvent(window.get(), IK_MouseX, IST_Axis, 0);
	InputEvent(window.get(), IK_MouseY, IST_Axis, 0);
	window->Tick();
	if (tickDebugger)
		tickDebugger();
	for (auto& it : activeInputButtons)
		viewport->Actor()->SetBool(it.first, true);
	for (auto& it : activeInputAxes)
	{
		if (it.first == "aMouseX" || it.first == "aMouseY")
		{
			viewport->Actor()->SetFloat(it.first, it.second.Value / (timeElapsed * 150.0f));
		}
		else
		{
			viewport->Actor()->SetFloat(it.first, it.second.Value);
		}
	}
}

void Engine::MouseMove(float x, float y)
{
	viewport->WindowsMouseX() = x;
	viewport->WindowsMouseY() = y;
}

bool Engine::MouseCursorVisible()
{
	return viewport->bShowWindowsMouse();
}

void Engine::Key(DisplayWindow* viewport, std::string key)
{
	if (Frame::RunState != FrameRunState::Running)
		return;

	for (char c : key)
	{
		CallEvent(console, EventName::KeyType, { ExpressionValue::ByteValue(c) });
	}
}

void Engine::InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta)
{
	if (Frame::RunState != FrameRunState::Running)
		return;

	bool handled = CallEvent(console, EventName::KeyEvent, { ExpressionValue::ByteValue(key), ExpressionValue::ByteValue(type), ExpressionValue::FloatValue((float)delta) }).ToBool();
	
	if (!handled)
	{
		if ((type == EInputType::IST_Press || type == EInputType::IST_Axis) && key >= 0 && key < 256)
		{
			if (type == EInputType::IST_Press)
				delta = 20;
			else
				delta *= 16;

			for (const std::string& command : GetSubcommands(keybindings[keynames[key]]))
			{
				auto it = inputAliases.find(command);
				if (it != inputAliases.end())
				{
					InputCommand(it->second, key, delta);
				}
				else
				{
					InputCommand(command, key, delta);
				}
			}
		}
		else if (type == EInputType::IST_Release)
		{
			for (auto it = activeInputButtons.begin(); it != activeInputButtons.end();)
			{
				if (it->second == key)
				{
					viewport->Actor()->SetBool(it->first, false);
					it = activeInputButtons.erase(it);
				}
				else
				{
					++it;
				}
			}

			for (auto it = activeInputAxes.begin(); it != activeInputAxes.end();)
			{
				if (it->second.Key == key)
				{
					viewport->Actor()->SetFloat(it->first, 0.0f);
					it = activeInputAxes.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}
}

bool Engine::ExecCommand(const std::vector<std::string>& args)
{
	for (UObject* target : { static_cast<UObject*>(viewport->Actor()), static_cast<UObject*>(console) })
	{
		if (!target)
			continue;

		UFunction* func = FindEventFunction(target, args[0]);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Exec))
		{
			std::vector<ExpressionValue> vmArgs;
			int argindex = 0;
			for (UField* field = func->Children; field != nullptr; field = field->Next)
			{
				UProperty* prop = dynamic_cast<UProperty*>(field);
				if (prop)
				{
					if (AllFlags(prop->PropFlags, PropertyFlags::Parm) && !AllFlags(prop->PropFlags, PropertyFlags::Parm | PropertyFlags::ReturnParm))
					{
						std::string arg = (1 + argindex < args.size()) ? args[1 + argindex] : std::string("0");
						switch (prop->ValueType)
						{
						case ExpressionValueType::Nothing: vmArgs.push_back(ExpressionValue::NothingValue()); break;
						case ExpressionValueType::ValueByte: vmArgs.push_back(ExpressionValue::ByteValue(std::atoi(arg.c_str()))); break;
						case ExpressionValueType::ValueInt: vmArgs.push_back(ExpressionValue::IntValue(std::atoi(arg.c_str()))); break;
						case ExpressionValueType::ValueBool: vmArgs.push_back(ExpressionValue::BoolValue(arg == "1" || arg == "true")); break;
						case ExpressionValueType::ValueFloat: vmArgs.push_back(ExpressionValue::FloatValue((float)std::atof(arg.c_str()))); break;
						case ExpressionValueType::ValueString: vmArgs.push_back(ExpressionValue::StringValue(arg)); break;
						case ExpressionValueType::ValueName: vmArgs.push_back(ExpressionValue::NameValue(arg)); break;
						default: break;
						}
						argindex++;
					}
				}
			}

			CallEvent(target, func->Name, vmArgs);
			return true;
		}
	}

	return false;
}

void Engine::InputCommand(const std::string& commands, EInputKey key, int delta)
{
	for (const std::string& commandline : GetSubcommands(commands))
	{
		std::vector<std::string> args = GetArgs(commandline);
		if (!args.empty())
		{
			std::string command = args[0];
			for (char& c : command) c = std::tolower(c);

			if (command == "button" && args.size() == 2)
			{
				activeInputButtons[args[1]] = key;
			}
			else if (command == "axis" && args.size() == 3)
			{
				float speed = 1.0f;
				if (args[2].size() > 6 && args[2].substr(0, 6) == "Speed=")
					speed = (float)std::atof(args[2].substr(6).c_str());
				activeInputAxes[args[1]] = { speed * delta, key };
			}
			else
			{
				ExecCommand(args);
			}
		}
	}
}

void Engine::SetPause(bool value)
{
}

void Engine::LockCursor()
{
	if (window)
		window->bLockCursor = true;
}

void Engine::UnlockCursor()
{
	if (window)
		window->bLockCursor = false;
}

void Engine::WindowClose(DisplayWindow* viewport)
{
	quit = true;
}

void Engine::LogMessage(const std::string& message)
{
	if (!Frame::Callstack.empty() && Frame::Callstack.back()->Func)
	{
		UStruct* func = Frame::Callstack.back()->Func;
		std::string name;
		for (UStruct* s = func; s != nullptr; s = s->StructParent)
		{
			if (name.empty())
				name = s->Name.ToString();
			else
				name = s->Name.ToString() + "." + name;
		}

		LogMessageLine line;
		line.Time = LevelInfo->TimeSeconds();
		line.Source = name;
		line.Text = message;
		Log.push_back(std::move(line));
	}
	else
	{
		LogMessageLine line;
		line.Time = LevelInfo->TimeSeconds();
		line.Text = message;
		Log.push_back(std::move(line));
	}

	if (printLogDebugger)
		printLogDebugger(Log.back());
}

void Engine::LogUnimplemented(const std::string& message)
{
	LogMessage("Unimplemented: " + message);
}

const char* Engine::keynames[256] =
{
	/*00*/ "None", "LeftMouse", "RightMouse", "Cancel",
	/*04*/ "MiddleMouse", "Unknown05", "Unknown06", "Unknown07",
	/*08*/ "Backspace", "Tab", "Unknown0A", "Unknown0B",
	/*0C*/ "Unknown0C", "Enter", "Unknown0E", "Unknown0F",
	/*10*/ "Shift", "Ctrl", "Alt", "Pause",
	/*14*/ "CapsLock", "Unknown15", "Unknown16", "Unknown17",
	/*18*/ "Unknown18", "Unknown19", "Unknown1A", "Escape",
	/*1C*/ "Unknown1C", "Unknown1D", "Unknown1E", "Unknown1F",
	/*20*/ "Space", "PageUp", "PageDown", "End",
	/*24*/ "Home", "Left", "Up", "Right",
	/*28*/ "Down", "Select", "Print", "Execute",
	/*2C*/ "PrintScrn", "Insert", "Delete", "Help",
	/*30*/ "0", "1", "2", "3",
	/*34*/ "4", "5", "6", "7",
	/*38*/ "8", "9", "Unknown3A", "Unknown3B",
	/*3C*/ "Unknown3C", "Unknown3D", "Unknown3E", "Unknown3F",
	/*40*/ "Unknown40", "A", "B", "C",
	/*44*/ "D", "E", "F", "G",
	/*48*/ "H", "I", "J", "K",
	/*4C*/ "L", "M", "N", "O",
	/*50*/ "P", "Q", "R", "S",
	/*54*/ "T", "U", "V", "W",
	/*58*/ "X", "Y", "Z", "Unknown5B",
	/*5C*/ "Unknown5C", "Unknown5D", "Unknown5E", "Unknown5F",
	/*60*/ "NumPad0", "NumPad1", "NumPad2", "NumPad3",
	/*64*/ "NumPad4", "NumPad5", "NumPad6", "NumPad7",
	/*68*/ "NumPad8", "NumPad9", "GreyStar", "GreyPlus",
	/*6C*/ "Separator", "GreyMinus", "NumPadPeriod", "GreySlash",
	/*70*/ "F1", "F2", "F3", "F4",
	/*74*/ "F5", "F6", "F7", "F8",
	/*78*/ "F9", "F10", "F11", "F12",
	/*7C*/ "F13", "F14", "F15", "F16",
	/*80*/ "F17", "F18", "F19", "F20",
	/*84*/ "F21", "F22", "F23", "F24",
	/*88*/ "Unknown88", "Unknown89", "Unknown8A", "Unknown8B",
	/*8C*/ "Unknown8C", "Unknown8D", "Unknown8E", "Unknown8F",
	/*90*/ "NumLock", "ScrollLock", "Unknown92", "Unknown93",
	/*94*/ "Unknown94", "Unknown95", "Unknown96", "Unknown97",
	/*98*/ "Unknown98", "Unknown99", "Unknown9A", "Unknown9B",
	/*9C*/ "Unknown9C", "Unknown9D", "Unknown9E", "Unknown9F",
	/*A0*/ "LShift", "RShift", "LControl", "RControl",
	/*A4*/ "UnknownA4", "UnknownA5", "UnknownA6", "UnknownA7",
	/*A8*/ "UnknownA8", "UnknownA9", "UnknownAA", "UnknownAB",
	/*AC*/ "UnknownAC", "UnknownAD", "UnknownAE", "UnknownAF",
	/*B0*/ "UnknownB0", "UnknownB1", "UnknownB2", "UnknownB3",
	/*B4*/ "UnknownB4", "UnknownB5", "UnknownB6", "UnknownB7",
	/*B8*/ "UnknownB8", "UnknownB9", "Semicolon", "Equals",
	/*BC*/ "Comma", "Minus", "Period", "Slash",
	/*C0*/ "Tilde", "UnknownC1", "UnknownC2", "UnknownC3",
	/*C4*/ "UnknownC4", "UnknownC5", "UnknownC6", "UnknownC7",
	/*C8*/ "Joy1", "Joy2", "Joy3", "Joy4",
	/*CC*/ "Joy5", "Joy6", "Joy7", "Joy8",
	/*D0*/ "Joy9", "Joy10", "Joy11", "Joy12",
	/*D4*/ "Joy13", "Joy14", "Joy15", "Joy16",
	/*D8*/ "UnknownD8", "UnknownD9", "UnknownDA", "LeftBracket",
	/*DC*/ "Backslash", "RightBracket", "SingleQuote", "UnknownDF",
	/*E0*/ "JoyX", "JoyY", "JoyZ", "JoyR",
	/*E4*/ "MouseX", "MouseY", "MouseZ", "MouseW",
	/*E8*/ "JoyU", "JoyV", "UnknownEA", "UnknownEB",
	/*EC*/ "MouseWheelUp", "MouseWheelDown", "Unknown10E", "Unknown10F",
	/*F0*/ "JoyPovUp", "JoyPovDown", "JoyPovLeft", "JoyPovRight",
	/*F4*/ "UnknownF4", "UnknownF5", "Attn", "CrSel",
	/*F8*/ "ExSel", "ErEof", "Play", "Zoom",
	/*FC*/ "NoName", "PA1", "OEMClear", ""
};
