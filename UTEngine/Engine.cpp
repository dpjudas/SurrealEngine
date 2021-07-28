
#include "Precomp.h"
#include "Engine.h"
#include "File.h"
#include "Collision.h"
#include "Renderer/UTRenderer.h"
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
#include "Audio/AudioPlayer.h"
#include "Audio/AudioSource.h"
#include "VM/Frame.h"
#include "VM/NativeFuncExtractor.h"
#include "VM/ScriptCall.h"
#include "UI/Debugger/DebuggerWindow.h"
#include <chrono>
#include <set>

#ifdef WIN32
namespace
{
	std::string from_utf16(const std::wstring& str)
	{
		if (str.empty()) return {};
		int needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
		if (needed == 0)
			throw std::runtime_error("WideCharToMultiByte failed");
		std::string result;
		result.resize(needed);
		needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size(), nullptr, nullptr);
		if (needed == 0)
			throw std::runtime_error("WideCharToMultiByte failed");
		return result;
	}
}
#endif

Engine* engine = nullptr;

Engine::Engine()
{
	engine = this;

#ifdef WIN32
	std::string utfolder;
	std::vector<wchar_t> buffer(1024);

	// Try use registry location
	HKEY regkey = 0;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Unreal Technology\\Installed Apps\\UnrealTournament", 0, KEY_READ, &regkey) == ERROR_SUCCESS)
	{
		DWORD type = 0;
		DWORD size = (DWORD)(buffer.size() * sizeof(wchar_t));
		LSTATUS result = RegQueryValueEx(regkey, L"Folder", 0, &type, (LPBYTE)buffer.data(), &size);
		if (result == ERROR_SUCCESS && type == REG_SZ)
		{
			buffer.back() = 0;
			utfolder = from_utf16(buffer.data());
		}
		RegCloseKey(regkey);
	}

	if (utfolder.empty()) // Try use subfolder next to exe
	{
		if (GetModuleFileName(0, buffer.data(), 1024))
		{
			buffer.back() = 0;
			std::string exepath = FilePath::remove_last_component(from_utf16(buffer.data()));
			utfolder = FilePath::combine(exepath, "Unreal Tournament");
		}
	}

	utfolder = R"(C:\Games\UnrealTournament436)";
	// utfolder = R"(C:\Games\Steam\steamapps\common\Unreal Gold)";

#else
	std::string utfolder = "/home/mbn/UnrealTournament";
#endif
	packages = std::make_unique<PackageManager>(utfolder);
}

Engine::~Engine()
{
	engine = nullptr;
}

void Engine::Run()
{
	std::srand((unsigned int)std::time(nullptr));

	LoadKeybindings();

	window = DisplayWindow::Create(this);
	window->OpenWindow(1800, 950, true);

	collision = std::make_unique<Collision>();
	renderer = std::make_unique<UTRenderer>();
	renderer->uiscale = std::max((window->SizeY + 540) / 1080, 1);

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

	UnrealURL url;
	url.Map = "Entry.unr";
	for (std::string optionKey : { "Name", "Class", "team", "skin", "Face", "Voice", "OverrideClass" })
	{
		url.Options.push_back(optionKey + "=" + packages->GetIniValue("user", "DefaultPlayer", optionKey));
	}

	// The entry map is the map you see in the game when no other map is playing. For example when disconnected from a server. It is always loaded and running.
	LoadMap(url);
	EntryLevelSummary = LevelSummary;
	EntryLevelInfo = LevelInfo;
	EntryLevel = Level;

	//url.Map = "CityIntro.unr";
	//url.Map = "UT-Logo-Map.unr";
	//url.Map = "DM-Liandri.unr";
	//url.Map = "DM-Codex.unr";
	url.Map = "DM-Barricade.unr";
	//url.Map = "DM-Deck16][.unr";
	//url.Map = "DM-KGalleon.unr";
	//url.Map = "DM-Turbine.unr";
	//url.Map = "DM-Tempest.unr";
	//url.Map = "DM-Grinder.unr";
	//url.Map = "DM-HyperBlast.unr";
	//url.Map = "DM-Peak.unr";
	//url.Map = "CTF-Coret.unr";
	//url.Map = "CTF-Dreary.unr";
	//url.Map = "CTF-Command.unr";
	//url.Map = "CTF-November.unr";
	//url.Map = "CTF-Gauntlet.unr";
	//url.Map = "CTF-EternalCave.unr";
	//url.Map = "CTF-Niven.unr";
	//url.Map = "CTF-Face.unr";
	//url.Map = "DOM-Sesmar.unr";
	//url.Map = "EOL_Deathmatch.unr";
	//url.Map = "UTCredits.unr";

	LoadMap(url);

	bool firstCall = true;
	bool ticked = false;
	while (!quit)
	{
		float elapsed = CalcTimeElapsed();

		window->Tick();

		CallEvent(console, "Tick", { ExpressionValue::FloatValue(elapsed) });

		// To do: owned actors must tick before their children:
		for (size_t i = 0; i < Level->Actors.size(); i++)
		{
			UActor* actor = Level->Actors[i];
			if (actor)
			{
				actor->Tick(elapsed, ticked);

				if (actor->Role() >= ROLE_SimulatedProxy && actor->LifeSpan() != 0.0f)
				{
					actor->LifeSpan() = std::max(actor->LifeSpan() - elapsed, 0.0f);
					if (actor->LifeSpan() == 0.0f)
					{
						CallEvent(actor, "Expired");
						// To do: destroy actor
					}
				}
			}
		}
		ticked = !ticked;

		Camera.Location = viewport->Actor()->Location();

		if (firstCall) // Unscript execution doesn't work well enough for us to use the viewport actor as our camera yet
		{
			firstCall = false;
			Camera.Yaw = viewport->Actor()->Rotation().YawDegrees();
			Camera.Roll = viewport->Actor()->Rotation().RollDegrees();
			Camera.Pitch = viewport->Actor()->Rotation().PitchDegrees();
		}
		else
		{
			quaternion viewrotation = normalize(quaternion::euler(radians(-Camera.Pitch), radians(-Camera.Roll), radians(90.0f - Camera.Yaw), EulerOrder::yxz));
			vec3 vel = { 0.0f };
			if (Buttons.StrafeLeft) vel.x = 1.0f;
			if (Buttons.StrafeRight) vel.x = -1.0f;
			if (Buttons.Forward) vel.y = 1.0f;
			if (Buttons.Backward) vel.y = -1.0f;
			if (vel != vec3(0.0f))
				vel = normalize(vel);
			vel = inverse(viewrotation) * vel;
			Camera.Location += vel * (elapsed * 650.0f);
			viewport->Actor()->Location() = Camera.Location;
		}

		engine->renderer->AutoUV += elapsed * 64.0f;
		for (UTexture* tex : engine->renderer->Textures)
			tex->Update(elapsed);

		RenderDevice* device = window->GetRenderDevice();
		device->BeginFrame();
		device->BeginScenePass();

		int sizeX = (int)(window->SizeX / (float)renderer->uiscale);
		int sizeY = (int)(window->SizeY / (float)renderer->uiscale);
		canvas->CurX() = 0.0f;
		canvas->CurY() = 0.0f;
		console->FrameX() = (float)sizeX;
		console->FrameY() = (float)sizeY;
		canvas->ClipX() = (float)sizeX;
		canvas->ClipY() = (float)sizeY;
		canvas->SizeX() = sizeX;
		canvas->SizeY() = sizeY;
		//viewport->bShowWindowsMouse() = true; // bShowWindowsMouse is set to true by WindowConsole if mouse cursor should be visible
		//viewport->bWindowsMouseAvailable() = true; // if true then RenderUWindow updates mouse pos from (WindowsMouseX,WindowsMouseY), otherwise it uses KeyEvent(IK_MouseX, delta) + KeyEvent(IK_MouseY, delta). Maybe used for windowed mode?
		//viewport->WindowsMouseX() = 10.0f;
		//viewport->WindowsMouseY() = 200.0f;
		CallEvent(canvas, "Reset");

		CallEvent(console, "PreRender", { ExpressionValue::ObjectValue(canvas) });
		CallEvent(viewport->Actor(), "PreRender", { ExpressionValue::ObjectValue(canvas) });
		if (console->bNoDrawWorld() == false)
		{
			renderer->scene.DrawScene();
			CallEvent(viewport->Actor(), "RenderOverlays", { ExpressionValue::ObjectValue(canvas) });
		}
		CallEvent(viewport->Actor(), "PostRender", { ExpressionValue::ObjectValue(canvas) });
		CallEvent(console, "PostRender", { ExpressionValue::ObjectValue(canvas) });
		renderer->scene.DrawTimedemoStats();
		device->EndFlash(0.5f, vec4(1.0f, 0.0f, 0.0f, 1.0f));

		device->EndScenePass();
		device->EndFrame(true);
	}
}

void Engine::ClientTravel(const std::string& newURL, uint8_t travelType, bool transferItems)
{
	// To do: need to do something about that travel type and transfering of items

	UnrealURL url(LevelInfo->URL, newURL);
	engine->LogMessage("Client travel to " + url.ToString());
	LoadMap(url);
}

void Engine::LoadMap(const UnrealURL& url)
{
	if (Level)
		CallEvent(console, "NotifyLevelChange");

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
	LevelInfo->MinNetVersion() = "500";
	LevelInfo->bHighDetailMode() = true;

	LevelInfo->URL = url;

	LevelSummary = UObject::Cast<ULevelSummary>(package->GetUObject("LevelSummary", "LevelSummary"));

	Level = UObject::Cast<ULevel>(package->GetUObject("Level", "MyLevel"));
	if (!Level)
		throw std::runtime_error("Could not find the Level object for this map!");

	if (packages->IsUnreal1())
		return;

	// Link actors to the level
	for (UActor* actor : Level->Actors)
	{
		if (actor)
			actor->XLevel() = Level;
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
	Level->Actors.push_back(GameInfo);
	GameInfo->XLevel() = Level;
	GameInfo->Level() = LevelInfo;
	GameInfo->Tag() = gameInfoClass->Name;
	GameInfo->bTicked() = false;
	if (LevelInfo->bBegunPlay())
	{
		CallEvent(GameInfo, "Spawned");
		CallEvent(GameInfo, "PreBeginPlay");
		CallEvent(GameInfo, "BeginPlay");
		CallEvent(GameInfo, "PostBeginPlay");
		CallEvent(GameInfo, "SetInitialState");
		std::string attachTag = GameInfo->AttachTag();
		if (!attachTag.empty())
		{
			for (UActor* actor : Level->Actors)
			{
				if (actor && actor->Tag() == attachTag)
				{
					CallEvent(actor, "Attach", { ExpressionValue::ObjectValue(GameInfo) });
				}
			}
		}
		for (USpawnNotify* notifyObj = LevelInfo->SpawnNotify(); notifyObj != nullptr; notifyObj = notifyObj->Next())
		{
			UClass* cls = notifyObj->ActorClass();
			if (cls && GameInfo->IsA(cls->Name))
				GameInfo = UObject::Cast<UGameInfo>(CallEvent(notifyObj, "SpawnNotification", { ExpressionValue::ObjectValue(GameInfo) }).ToObject());
		}
	}
	LevelInfo->Game() = GameInfo;

	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) actor->CreateDefaultState(); }

	if (!LevelInfo->bBegunPlay())
	{
		LevelInfo->TimeSeconds() = 0.0f;
		LevelInfo->bBegunPlay() = true;
	}

	LevelInfo->bStartup() = true;
	CallEvent(GameInfo, "InitGame");
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, "PreBeginPlay"); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, "BeginPlay"); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, "PostBeginPlay"); }
	for (size_t i = 0; i < Level->Actors.size(); i++) { UActor* actor = Level->Actors[i]; if (actor) CallEvent(actor, "SetInitialState"); }
	LevelInfo->bStartup() = false;

	// Create viewport pawn
	std::string playerPawnClass = packages->GetIniValue("system", "URL", "Class");
	std::string pawnPackageName = playerPawnClass.substr(0, playerPawnClass.find('.'));
	std::string pawnClassName = playerPawnClass.substr(playerPawnClass.find('.') + 1);
	UClass* pawnClass = UObject::Cast<UClass>(packages->GetPackage(pawnPackageName)->GetUObject("Class", pawnClassName));
	std::string portal, options, error;
	UStringProperty stringProp("", nullptr, ObjectFlags::NoFlags);
	UPlayerPawn* pawn = UObject::Cast<UPlayerPawn>(CallEvent(LevelInfo->Game(), "Login", {
		ExpressionValue::StringValue(portal),
		ExpressionValue::StringValue(options),
		ExpressionValue::Variable(&error, &stringProp),
		ExpressionValue::ObjectValue(pawnClass)
		}).ToObject());
	if (!pawn)
		throw std::runtime_error("GameInfo login failed: " + error);

	// Assign the pawn to the viewport
	viewport->Actor() = pawn;
	viewport->Actor()->Player() = viewport;
	if (!packages->IsUnreal1()) // To do: this crashes for unreal 1 atm
		CallEvent(viewport->Actor(), "Possess");

	CallEvent(pawn, "TravelPreAccept");
	// To do: handle level inventory transfer here
	CallEvent(pawn, "TravelPostAccept");

	CallEvent(LevelInfo->Game(), "PostLogin", { ExpressionValue::ObjectValue(pawn) });

	// Cache some light and texture info
	std::set<UActor*> lightset;
	for (UActor* light : Level->Model->Lights)
		lightset.insert(light);
	engine->renderer->Lights.clear();
	for (UActor* light : lightset)
		engine->renderer->Lights.push_back(light);
	engine->renderer->Textures.clear();
	for (BspSurface& surf : Level->Model->Surfaces)
	{
		if (surf.Material)
		{
			engine->renderer->Textures.insert(surf.Material);
			if (surf.Material->DetailTexture())
				engine->renderer->Textures.insert(surf.Material->DetailTexture());
			if (surf.Material->MacroTexture())
				engine->renderer->Textures.insert(surf.Material->MacroTexture());
		}
	}
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
		renderer->showTimedemoStats = args[1] == "1";
	}
	else if (command == "showlog")
	{
		Frame::ShowDebuggerWindow();
	}
	else if (command == "playsong")
	{
		auto music = LevelInfo->Song();
		if (music)
			audioplayer = AudioPlayer::Create(AudioSource::CreateMod(music->Data, true, 0, LevelInfo->SongSection()));
	}
	else if (command == "stopsong")
	{
		audioplayer.reset();
	}
	else if (command == "getres")
	{
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
		LogMessage("Unknown command: " + commandline);
		found = false;
	}
	return {};
}

void Engine::LoadKeybindings()
{
	for (int i = 0; i < 256; i++)
	{
		std::string keyname = keynames[i];
		keybindings[keyname] = packages->GetIniValue("user", "Engine.Input", keyname);
	}
}

void Engine::Key(DisplayWindow* viewport, std::string key)
{
	if (Frame::RunState != FrameRunState::Running)
		return;

	for (char c : key)
	{
		CallEvent(console, "KeyType", { ExpressionValue::ByteValue(c) });
	}
}

void Engine::InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta)
{
	if (Frame::RunState != FrameRunState::Running)
		return;

	bool handled = CallEvent(console, "KeyEvent", { ExpressionValue::ByteValue(key), ExpressionValue::ByteValue(type), ExpressionValue::FloatValue((float)delta) }).ToBool();

	if (!handled)
	{
		switch (key)
		{
		case 'W':
			Buttons.Forward = (type == IST_Press);
			break;
		case 'S':
			Buttons.Backward = (type == IST_Press);
			break;
		case 'A':
			Buttons.StrafeLeft = (type == IST_Press);
			break;
		case 'D':
			Buttons.StrafeRight = (type == IST_Press);
			break;
		case IK_Space:
			Buttons.Jump = (type == IST_Press);
			break;
		case IK_Shift:
			Buttons.Crouch = (type == IST_Press);
			break;
		case IK_LeftMouse:
			break;
		case IK_MiddleMouse:
			break;
		case IK_RightMouse:
			break;
		case IK_MouseWheelDown:
			break;
		case IK_MouseWheelUp:
			break;
		case IK_MouseX:
			Camera.Yaw += delta * Mouse.SpeedX;
			while (Camera.Yaw < 0.0f) Camera.Yaw += 360.0f;
			while (Camera.Yaw >= 360.0f) Camera.Yaw -= 360.0f;
			break;
		case IK_MouseY:
			Camera.Pitch = clamp(Camera.Pitch + delta * Mouse.SpeedY, -90.0f, 90.0f);
			break;
		}
	}
}

void Engine::SetPause(bool value)
{
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
				name = s->Name;
			else
				name = s->Name + "." + name;
		}

		LogMessageLine line;
		line.Source = name;
		line.Text = message;
		Log.push_back(std::move(line));
	}
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
