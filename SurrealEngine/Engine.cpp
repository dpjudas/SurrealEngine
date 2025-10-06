
#include "Precomp.h"
#include "Engine.h"
#include "Utils/File.h"
#include "Utils/StrCompare.h"
#include "Render/RenderSubsystem.h"
#include "Package/PackageManager.h"
#include "Package/ObjectStream.h"
#include "UObject/ULevel.h"
#include "UObject/UFont.h"
#include "UObject/UMesh.h"
#include "UObject/UActor.h"
#include "UObject/ObjectTravelInfo.h"
#include "UObject/UTexture.h"
#include "UObject/UMusic.h"
#include "UObject/USound.h"
#include "UObject/UClass.h"
#include "UObject/UClient.h"
#include "UObject/USubsystem.h"
#include "Math/quaternion.h"
#include "Math/FrustumPlanes.h"
#include "GameWindow.h"
#include "RenderDevice/RenderDevice.h"
#include "VM/Frame.h"
#include "VM/ScriptCall.h"
#include "Video/VideoPlayer.h"
#include <chrono>
#include <set>

Engine* engine = nullptr;

Engine::Engine(GameLaunchInfo launchinfo) : LaunchInfo(launchinfo)
{
	engine = this;

	packages = std::make_unique<PackageManager>(LaunchInfo);

	std::srand((unsigned int)std::time(nullptr));

	gameengine = UObject::Cast<UGameEngine>(packages->NewObject("gameengine", "Engine", "GameEngine"));
	audiodev = UObject::Cast<USurrealAudioDevice>(packages->NewObject("audiodev", "Engine", "SurrealAudioDevice"));
	renderdev = UObject::Cast<USurrealRenderDevice>(packages->NewObject("renderdev", "Engine", "SurrealRenderDevice"));
	netdev = UObject::Cast<USurrealNetworkDevice>(packages->NewObject("netdev", "Engine", "SurrealNetworkDevice"));
	client = UObject::Cast<USurrealClient>(packages->NewObject("client", "Engine", "SurrealClient"));
	viewport = UObject::Cast<UViewport>(packages->NewObject("viewport", "Engine", "Viewport"));
	canvas = UObject::Cast<UCanvas>(packages->NewObject("canvas", "Engine", "Canvas"));
	DefaultTexture = UObject::Cast<UTexture>(packages->GetPackage("Engine")->GetUObject("Texture", "DefaultTexture"));

	std::string consolestr = packages->GetIniValue("system", "Engine.Engine", "Console");
	std::string consolepkg = consolestr.substr(0, consolestr.find('.'));
	std::string consolecls = consolestr.substr(consolestr.find('.') + 1);
	console = UObject::Cast<UConsole>(packages->NewObject("console", consolepkg, consolecls));

	console->Viewport() = viewport;
	canvas->Viewport() = viewport;
	viewport->Console() = console;
}

Engine::~Engine()
{
	if (audiodev)
		audiodev->ShutdownDevice();

	engine = nullptr;
}

void Engine::Run()
{
	LoadEngineSettings();
	LoadKeybindings();

	OpenWindow();

	audiodev->InitDevice();
	render = std::make_unique<RenderSubsystem>(window->GetRenderDevice());

	if (engine->LaunchInfo.engineVersion > 219 && !client->StartupFullscreen)
		viewport->bWindowsMouseAvailable() = true;

	window->LockCursor();

	if (packages->IsKlingonHonorGuard())
	{
		PlayAVI({ "playavi", "INTRO.AVI", "N" });
	}

	if (!LaunchInfo.noEntryMap)
		LoadEntryMap();

	if (LaunchInfo.url.empty())
		LoadMap(GetDefaultURL(packages->GetIniValue("system", "URL", "LocalMap")));
	else
		LoadMap(UnrealURL(GetDefaultURL(packages->GetIniValue("system", "URL", "LocalMap")), LaunchInfo.url));

	LoginPlayer();

	auto objprop = GC::Alloc<UObjectProperty>(NameString(), nullptr, ObjectFlags::NoFlags);
	auto vecprop = GC::Alloc<UStructProperty>(NameString(), nullptr, ObjectFlags::NoFlags);
	auto rotprop = GC::Alloc<UStructProperty>(NameString(), nullptr, ObjectFlags::NoFlags);

	bool firstCall = true;
	while (!quit)
	{
		// Main game loop should consist of these 4 steps:
		// Tick everything
		// Render the scene
		// Check if there is a request to save the game: save the game if that's the case
		// Check if there is a new map to load (next level, saved game etc.): load it if that's the case

		// Tick everything
		float realTimeElapsed = CalcTimeElapsed();
		float entryLevelElapsed = EntryLevel ? realTimeElapsed * clamp(EntryLevelInfo->TimeDilation(), 0.0025f, 25.0f) : 0.0f;
		float levelElapsed = realTimeElapsed * clamp(LevelInfo->TimeDilation(), 0.0025f, 25.0f);

		TotalTime += realTimeElapsed;

		if (EntryLevel)
			EntryLevelInfo->TimeSeconds() += entryLevelElapsed;
		LevelInfo->TimeSeconds() += levelElapsed;
		Logger::Get()->SetTimeSeconds(LevelInfo->TimeSeconds());

		// Update the time fields
		std::time_t now = std::time(nullptr);
		std::tm* timedesc = std::localtime(&now);

		LevelInfo->Year() = timedesc->tm_year;
		LevelInfo->Month() = timedesc->tm_mon;
		LevelInfo->Day() = timedesc->tm_mday;
		LevelInfo->DayOfWeek() = timedesc->tm_wday;
		LevelInfo->Hour() = timedesc->tm_hour;
		LevelInfo->Minute() = timedesc->tm_min;
		LevelInfo->Second() = timedesc->tm_sec;
		LevelInfo->Millisecond() = 0; // No timedesc equivalent for LevelInfo->Millisecond()

		UpdateInput(realTimeElapsed);

		SetPause(!LevelInfo->Pauser().empty());

		// Do NOT pause this Tick event otherwise some messages will stay on screen forever.
		CallEvent(console, EventName::Tick, { ExpressionValue::FloatValue(levelElapsed) });

		// To do: set these to true if the frame rate is too low
		if (LaunchInfo.engineVersion >= 436)
		{
			LevelInfo->bDropDetail() = false;
			LevelInfo->bAggressiveLOD() = false;
		}

		if (EntryLevel)
			EntryLevel->Tick(entryLevelElapsed, m_GamePaused);
		Level->Tick(levelElapsed, m_GamePaused);

		// To do: improve CallEvent so parameter passing isn't this painful
		UFunction* funcPlayerCalcView = viewport->Actor() ? FindEventFunction(viewport->Actor(), "PlayerCalcView") : nullptr;
		if (funcPlayerCalcView)
		{
			vecprop->Struct = UObject::Cast<UStructProperty>(funcPlayerCalcView->Properties[1])->Struct;
			rotprop->Struct = UObject::Cast<UStructProperty>(funcPlayerCalcView->Properties[2])->Struct;
			CameraActor = viewport->Actor();
			CameraLocation = viewport->Actor()->Location();
			CameraRotation = viewport->Actor()->Rotation();
			CameraFovAngle = viewport->Actor()->FovAngle();
			CallEvent(viewport->Actor(), EventName::PlayerCalcView, {
				ExpressionValue::Variable(&CameraActor, objprop),
				ExpressionValue::Variable(&CameraLocation, vecprop),
				ExpressionValue::Variable(&CameraRotation, rotprop)
				});
		}

		UpdateAudio();

		ViewportX = 0;
		ViewportY = 0;
		ViewportWidth = engine->window->GetPixelWidth();
		ViewportHeight = engine->window->GetPixelHeight();
		render->DrawGame(levelElapsed);

		// Save the game if there is a request for it
		if (SaveGameInfo.SaveGameSlot != DONT_SAVE_GAME)
		{
			SaveGameToSlot(SaveGameInfo.SaveGameSlot, SaveGameInfo.SaveGameDescription);

			SaveGameInfo.SaveGameSlot = DONT_SAVE_GAME;
			SaveGameInfo.SaveGameDescription.clear();
		}

		// Check if there is a new map to load
		if (!LevelInfo->NextURL().empty())
		{
			LevelInfo->NextSwitchCountdown() -= levelElapsed;
			if (LevelInfo->NextSwitchCountdown() <= 0.0f)
			{
				if (UnrealURL(LevelInfo->NextURL()).HasOption("restart"))
				{
					LoadMap(LevelInfo->URL, Level->TravelInfo);
					LoginPlayer();
				}
				else if (LevelInfo->bNextItems())
				{
					LoadMap(UnrealURL(LevelInfo->URL, LevelInfo->NextURL()), CreateTravelInfo(true));
					LoginPlayer();
				}
				else
				{
					LoadMap(UnrealURL(LevelInfo->URL, LevelInfo->NextURL()), {});
					LoginPlayer();
				}
			}
		}

		if (ClientTravelInfo.URL.HasOption("restart"))
		{
			LoadMap(LevelInfo->URL, Level->TravelInfo);
			LoginPlayer();
		}

		if (ClientTravelInfo.URL.HasOption("load"))
		{
			UnrealURL url(ClientTravelInfo.URL);
			LoadFromSaveFile(url);
			LoginPlayer();
		}

		if (!ClientTravelInfo.URL.Map.empty())
		{
			// To do: need to do something about that travel type and transfering of items

			UnrealURL url(ClientTravelInfo.URL);
			LogMessage("Client travel to " + url.ToString());
			LoadMap(url, CreateTravelInfo(ClientTravelInfo.TransferItems));
			LoginPlayer();
		}
	}

	window->UnlockCursor();

	if (packages->MissingSESystemIni())
	{
		// Add the missing Subsystem entries
		client->SaveConfig();
		audiodev->SaveConfig();
		renderdev->SaveConfig();
	}
	packages->SetIniValue("System", "Engine.SurrealWindowSystem", "WindowSystem", windowingSystemName);
	packages->SaveAllIniFiles();

	CloseWindow();
}

void Engine::PlayAVI(const Array<std::string>& args)
{
	if (args.size() < 3)
		return;

	// What did KHG request?
	std::string buildup, breakdown, video;
	if (args.size() > 4 && args[2] == "C")
	{
		buildup = args[1];
		video = args[3];
		breakdown = "breakdn.avi";
	}
	else if (args.size() > 3 && args[2] == "Y")
	{
		buildup = "buildup.avi";
		video = args[1];
		breakdown = "breakdn.avi";
	}
	else
	{
		video = args[1];
	}

	playingAvi = true;
	skipAvi = false;

	try
	{
		// Load the videos

		std::unique_ptr<VideoPlayer> buildupPlayer, videoPlayer, breakdownPlayer;
		if (!buildup.empty())
			buildupPlayer = VideoPlayer::Create(packages->GetVideoFilename(buildup));
		if (!video.empty())
			videoPlayer = VideoPlayer::Create(packages->GetVideoFilename(video));
		if (!breakdown.empty())
			breakdownPlayer = VideoPlayer::Create(packages->GetVideoFilename(breakdown));

		CalcTimeElapsed(); // Reset so load time doesn't affect playback

		UnrealMipmap* background = nullptr;
		if (buildupPlayer)
		{
			background = PlayVideo(buildupPlayer.get(), nullptr);
			if (!background)
			{
				CalcTimeElapsed();
				return;
			}

			// Cut a hole in the background image
			uint32_t* pixels = (uint32_t*)background->Data.data();
			int w = background->Width;
			int h = background->Height;

			ivec2 v0 = { 232, 125 };
			ivec2 v1 = { 161, 246 };
			ivec2 v2 = { 406, 125 };
			ivec2 v3 = { 475, 246 };
			ivec2 v4 = { 258, 125 };
			ivec2 v5 = { 270, 144 };
			ivec2 v6 = { 380, 125 };
			ivec2 v7 = { 368, 144 };
			ivec2 v8 = { 210, 327 };
			ivec2 v9 = { 427, 327 };

			for (int y = 126; y < 144; y++)
			{
				int x0 = (int)(v0.x + 0.5f + (y - v0.y + 0.5f) * (v1.x - v0.x) / (v1.y - v0.y));
				int x1 = (int)(v4.x + 0.5f + (y - v4.y + 0.5f) * (v5.x - v4.x) / (v5.y - v4.y));
				int x2 = (int)(v6.x + 0.5f + (y - v6.y + 0.5f) * (v7.x - v6.x) / (v7.y - v6.y));
				int x3 = (int)(v2.x + 0.5f + (y - v2.y + 0.5f) * (v3.x - v2.x) / (v3.y - v2.y));

				pixels[x0 + y * w] = 0x80000000;
				pixels[x1 - 1 + y * w] = 0x80000000;
				for (int x = x0 + 1; x < x1 - 1; x++)
					pixels[x + y * w] = 0;

				pixels[x2 + y * w] = 0x80000000;
				pixels[x3 - 1 + y * w] = 0x80000000;
				for (int x = x2 + 1; x < x3 - 1; x++)
					pixels[x + y * w] = 0;
			}

			for (int y = 144; y < 246; y++)
			{
				int x0 = (int)(v0.x + 0.5f + (y - v0.y + 0.5f) * (v1.x - v0.x) / (v1.y - v0.y));
				int x1 = (int)(v2.x + 0.5f + (y - v2.y + 0.5f) * (v3.x - v2.x) / (v3.y - v2.y));
				pixels[x0 + y * w] = 0x80000000;
				pixels[x1 - 1 + y * w] = 0x80000000;
				for (int x = x0 + 1; x < x1 - 1; x++)
					pixels[x + y * w] = 0;
			}

			for (int y = 246; y < 325; y++)
			{
				int x0 = (int)(v1.x + 0.5f + (y - v1.y + 0.5f) * (v8.x - v1.x) / (v8.y - v1.y));
				int x1 = (int)(v3.x + 0.5f + (y - v3.y + 0.5f) * (v9.x - v3.x) / (v9.y - v3.y));
				pixels[x0 + y * w] = 0x80000000;
				pixels[x1 - 1 + y * w] = 0x80000000;
				for (int x = x0 + 1; x < x1 - 1; x++)
					pixels[x + y * w] = 0;
			}
		}

		if (videoPlayer)
		{
			if (!PlayVideo(videoPlayer.get(), background))
			{
				playingAvi = false;
				CalcTimeElapsed();
				return;
			}
		}

		if (breakdownPlayer)
		{
			PlayVideo(breakdownPlayer.get(), nullptr);
		}
	}
	catch (const std::exception& e)
	{
		LogMessage("Error playing " + video + ": " + e.what());
	}

	playingAvi = false;
	CalcTimeElapsed(); // Reset so game isn't affected
}

UnrealMipmap* Engine::PlayVideo(VideoPlayer* video, UnrealMipmap* background)
{
	UnrealMipmap* frame = nullptr;

	FTextureInfo texinfo[2];
	texinfo[0].CacheID = 0xffffffff'ffffffffULL;
	texinfo[0].Format = TextureFormat::BGRA8;
	texinfo[0].NumMips = 1;

	if (background)
	{
		texinfo[1].CacheID = 0xffffffff'fffffffeULL;
		texinfo[1].Format = TextureFormat::BGRA8;
		texinfo[1].NumMips = 1;
		texinfo[1].Mips = background;
		texinfo[1].USize = background->Width;
		texinfo[1].VSize = background->Height;
		texinfo[1].bRealtimeChanged = true;
	}

	audiodev->SetViewport(nullptr);
	audiodev->GetDevice()->PlayMusic(video->GetAudio());

	float timestamp = 0.0f;
	int curframe = -1;
	while (!quit && !skipAvi)
	{
		timestamp += CalcTimeElapsed();

		bool done = false;
		while (curframe < video->GetFrameIndexForTime(timestamp))
		{
			while (true)
			{
				UnrealMipmap* nextframe = video->NextVideoFrame();
				if (nextframe)
				{
					frame = nextframe;
					curframe++;
					texinfo[0].bRealtimeChanged = true;
					break;
				}
				if (!video->Decode())
				{
					done = true;
					break;
				}
			}
			if (done)
				break;
		}
		if (done)
			break;

		audiodev->GetDevice()->Update();
		GameWindow::ProcessEvents();

		if (frame)
		{
			texinfo[0].Mips = frame;
			texinfo[0].USize = frame->Width;
			texinfo[0].VSize = frame->Height;

			ViewportX = 0;
			ViewportY = 0;
			ViewportWidth = engine->window->GetPixelWidth();
			ViewportHeight = engine->window->GetPixelHeight();
			render->DrawVideoFrame(&texinfo[0], background ? &texinfo[1] : nullptr);
		}
	}

	audiodev->GetDevice()->PlayMusic(nullptr);

	if (quit || skipAvi)
		return nullptr;

	return frame;
}

void Engine::UpdateAudio()
{
	mat4 translate = mat4::translate(vec3(0.0f) - CameraLocation);
	mat4 listener = Coords::ViewToAudioDev().ToMatrix() * Coords::Rotation(CameraRotation).ToMatrix() * translate;

	audiodev->SetViewport(viewport);
	audiodev->Update(listener);
}

void Engine::ClientTravel(const std::string& newURL, ETravelType travelType, bool transferItems)
{
	UnrealURL url(newURL);

	// If the URL doesn't contain the player info, add them here.
	// As they have to persist somehow
	for (std::string optionKey : { "Name", "Class", "team", "skin", "Face", "Voice", "OverrideClass" })
	{
		if (engine->packages->GetEngineVersion() > 219)
		{
			if (url.HasOption(optionKey))
				engine->packages->SetIniValue("User", "DefaultPlayer", optionKey, url.GetOption(optionKey));
			else
				url.AddOrReplaceOption(optionKey + "=" + packages->GetIniValue("user", "DefaultPlayer", optionKey));
		}
		else
		{
			if (url.HasOption(optionKey))
				engine->packages->SetIniValue("System", "URL", optionKey, url.GetOption(optionKey));
			else
				url.AddOrReplaceOption(optionKey + "=" + packages->GetIniValue("System", "URL", optionKey));
		}
	}

	if (travelType == ETravelType::TRAVEL_Absolute)
		ClientTravelInfo.URL = url;
	else if (travelType == ETravelType::TRAVEL_Partial)
	{
		auto name = ClientTravelInfo.URL.GetOption("name");
		ClientTravelInfo.URL = url;
		ClientTravelInfo.URL.AddOrReplaceOption("name=" + name);
	}
	else if (travelType == ETravelType::TRAVEL_Relative)
		ClientTravelInfo.URL = UnrealURL(ClientTravelInfo.URL, url);
	ClientTravelInfo.TravelType = travelType;
	ClientTravelInfo.TransferItems = transferItems;
}

UnrealURL Engine::GetDefaultURL(const std::string& map)
{
	UnrealURL url;
	std::string teleporterTag = "";
	std::string finalMapName = map;

	size_t tagPos = map.find('#');

	if (tagPos != std::string::npos)
	{
		teleporterTag = map.substr(tagPos + 1);
		finalMapName = map.substr(0, tagPos);
	}
	if (map.find("." + packages->GetMapExtension()) == std::string::npos)
		url.Map = finalMapName + "." + packages->GetMapExtension();
	else
		url.Map = finalMapName;

	if (!teleporterTag.empty())
		url.Portal = teleporterTag;
	for (std::string optionKey : { "Name", "Class", "team", "skin", "Face", "Voice", "OverrideClass" })
	{
		url.Options.push_back(optionKey + "=" + packages->GetIniValue("user", "DefaultPlayer", optionKey));
	}
	return url;
}

void Engine::LoadEntryMap()
{
	// The entry map is the map you see in the game when no other map is playing. For example when disconnected from a server. It is always loaded and running.
	LoadMap(GetDefaultURL("Entry"));
	EntryLevelInfo = LevelInfo;
	EntryLevel = Level;
	EntryLevelPackage = std::move(LevelPackage);
	LevelInfo = nullptr;
	Level = nullptr;
	viewport->Actor() = nullptr;
}

void Engine::UnloadMap()
{
	if (!LevelPackage)
		return;

	LevelInfo = nullptr;
	Level = nullptr;
	viewport->Actor() = nullptr;
	packages->UnloadMap(std::move(LevelPackage));
}

void Engine::LoadMap(const UnrealURL& url, const std::map<std::string, std::string>& travelInfo)
{
	ClientTravelInfo.URL.Clear();

	if (Level)
		CallEvent(console, EventName::NotifyLevelChange);

	if (url.HasOption("entry")) // Not sure what the purpose of this kind of travel is - do nothing for now.
		return;

	audiodev->StopSounds();
	UnloadMap();

	// Load map objects

	LevelPackage = packages->LoadMap(url.Map);

	LevelInfo = UObject::Cast<ULevelInfo>(LevelPackage->GetUObject("LevelInfo", "LevelInfo0"));
	if (packages->GetEngineVersion() < 300) // Unknown when this changed
	{
		for (int grr = 1; !LevelInfo && grr < 20; grr++)
			LevelInfo = UObject::Cast<ULevelInfo>(LevelPackage->GetUObject("LevelInfo", "LevelInfo" + std::to_string(grr)));
	}
	if (!LevelInfo)
		Exception::Throw("Could not find the LevelInfo object for this map!");

	LevelInfo->ComputerName() = "MyComputer";
	LevelInfo->HubStackLevel() = 0; // To do: handle level hubs
	LevelInfo->EngineVersion() = std::to_string(LaunchInfo.engineVersion) + " SE";
	if (packages->GetEngineVersion() > 219)
		LevelInfo->MinNetVersion() = std::to_string(LaunchInfo.engineVersion) + " SE";
	LevelInfo->bHighDetailMode() = true;
	LevelInfo->NetMode() = 0; // NM_StandAlone
	LevelInfo->DefaultTexture() = engine->DefaultTexture;

	LevelInfo->URL = url;

	Level = UObject::Cast<ULevel>(LevelPackage->GetUObject("Level", "MyLevel"));
	if (!Level)
		Exception::Throw("Could not find the Level object for this map!");

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
			Level->Collision.AddToCollision(actor);
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
		Exception::Throw("Could not find any gameinfo class!");

	// Spawn GameInfo actor
	GameInfo = UObject::Cast<UGameInfo>(packages->NewObject("gameinfo", gameInfoClass));
	GameInfo->XLevel() = Level;
	GameInfo->Level() = LevelInfo;
	Level->Collision.AddToCollision(GameInfo);
	GameInfo->Tag() = gameInfoClass->Name;
	GameInfo->bTicked() = false;
	GameInfo->InitActorZone();
	GameInfo->Index = (int)Level->Actors.size();
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

	auto stringProp = GC::Alloc<UStringProperty>("", nullptr, ObjectFlags::NoFlags);
	std::string error;

	LevelInfo->bStartup() = true;
	CallEvent(GameInfo, EventName::InitGame, { ExpressionValue::StringValue(options), ExpressionValue::Variable(&error, stringProp) });
	if (!error.empty())
		Exception::Throw("InitGame failed: " + error);
	for (UActor* actor : Level->Actors) { if (actor) CallEvent(actor, EventName::PreBeginPlay); }
	for (UActor* actor : Level->Actors) { if (actor) CallEvent(actor, EventName::BeginPlay); }
	for (UActor* actor : Level->Actors) { if (actor) CallEvent(actor, EventName::PostBeginPlay); }
	for (UActor* actor : Level->Actors) { if (actor) CallEvent(actor, EventName::SetInitialState); }
	for (UActor* actor : Level->Actors) { if (actor) actor->InitBase(); }
	LevelInfo->bStartup() = false;

	if (LevelInfo->Game())
		CallEvent(LevelInfo->Game(), "DetailChange", {});
}

void Engine::LoadFromSaveFile(const UnrealURL& url)
{
	ClientTravelInfo.URL.Clear();

	if (Level)
		CallEvent(console, EventName::NotifyLevelChange);

	if (url.HasOption("entry")) // Not sure what the purpose of this kind of travel is - do nothing for now.
		return;

	Package* savefilePackage = nullptr;

	if (url.HasOption("load"))
	{
		uint32_t slotNum = Convert::to_uint32(url.GetOption("load"));
		savefilePackage = packages->LoadSaveSlot(slotNum);
	}

	if (!savefilePackage)
		return;

	audiodev->StopSounds();
	UnloadMap();

	LevelPackage = savefilePackage;

	LevelInfo = UObject::Cast<ULevelInfo>(LevelPackage->GetUObject("LevelInfo", "LevelInfo0"));
	if (packages->GetEngineVersion() < 300) // Unknown when this changed
	{
		for (int grr = 1; !LevelInfo && grr < 20; grr++)
			LevelInfo = UObject::Cast<ULevelInfo>(LevelPackage->GetUObject("LevelInfo", "LevelInfo" + std::to_string(grr)));
	}
	if (!LevelInfo)
		Exception::Throw("Could not find the LevelInfo object for this map!");

	/*
	LevelInfo->ComputerName() = "MyComputer";
	LevelInfo->HubStackLevel() = 0; // To do: handle level hubs
	*/
	LevelInfo->EngineVersion() = std::to_string(LaunchInfo.engineVersion) + " SE";
	if (packages->GetEngineVersion() > 219)
		LevelInfo->MinNetVersion() = std::to_string(LaunchInfo.engineVersion) + " SE";
	LevelInfo->bHighDetailMode() = true;
	/*
	LevelInfo->NetMode() = 0; // NM_StandAlone
	LevelInfo->DefaultTexture() = engine->DefaultTexture;
	*/

	Level = UObject::Cast<ULevel>(LevelPackage->GetUObject("Level", "MyLevel"));
	if (!Level)
		Exception::Throw("Could not find the Level object for this map!");

	// Link actors to the level
	for (UActor* actor : Level->Actors)
	{
		if (actor)
		{
			actor->XLevel() = Level;
			Level->Collision.AddToCollision(actor);
		}
	}
}

void Engine::SaveGameToSlot(int32_t slotNum, const std::string& saveDescription) const
{
	if (slotNum < -1 || (!packages->IsDeusEx() && slotNum <= 0))
		Exception::Throw("Invalid save slot: " + std::to_string(slotNum));

	if (packages->IsDeusEx())
	{
		// Handle Deus Ex separately, as saving a game there creates several files
	}
	else
	{
		const auto saveFolderPath = fs::path(LaunchInfo.gameRootFolder) / "Save";
		const std::string saveFileName = "Save" + std::to_string(slotNum) + "." + packages->GetSaveExtension();
		const std::string saveFileFullPath = (saveFolderPath / saveFileName).string();
		LevelPackage->Save(Level, saveFileFullPath);
	}
}

std::map<std::string, std::string> Engine::CreateTravelInfo(bool transferItems)
{
	auto travelInfo = Level->TravelInfo;
	for (UActor* actor : Level->Actors)
	{
		UPlayerPawn* pawn = UObject::TryCast<UPlayerPawn>(actor);
		if (pawn && pawn->Player())
		{
			std::string playerName = engine->LaunchInfo.engineVersion > 219 ? pawn->PlayerReplicationInfo()->PlayerName() : std::string("Player"); // To do: how to get the travel player name?
			travelInfo[playerName] = ActorTravelInfo::Create(pawn, transferItems);
		}
	}
	return travelInfo;
}

void Engine::LoginPlayer()
{
	UnrealURL url = LevelInfo->URL;
	std::map<std::string, std::string> travelInfo = Level->TravelInfo;

	auto stringProp = GC::Alloc<UStringProperty>("", nullptr, ObjectFlags::NoFlags);
	std::string error, failcode;

	std::string portal = url.GetPortal();
	std::string options = url.GetOptions();

	std::string playerPawnClass = url.GetOption("Class");
	if (playerPawnClass.empty())
		playerPawnClass = packages->GetIniValue("system", "URL", "Class");
	UClass* pawnClass = packages->FindClass(playerPawnClass);

	// Perform PreLogin check (used for early rejection in network games)
	CallEvent(LevelInfo->Game(), EventName::PreLogin, {
		ExpressionValue::StringValue(options),
		ExpressionValue::Variable(&error, stringProp),
		ExpressionValue::Variable(&failcode, stringProp),
		});
	if (!error.empty() || !failcode.empty())
		Exception::Throw("GameInfo prelogin failed: " + error + " (" + failcode + ")");

	// Create viewport pawn
	size_t numActors = Level->Actors.size();
	UPlayerPawn* pawn = UObject::Cast<UPlayerPawn>(CallEvent(LevelInfo->Game(), EventName::Login, {
		ExpressionValue::StringValue(portal),
		ExpressionValue::StringValue(options),
		ExpressionValue::Variable(&error, stringProp),
		ExpressionValue::ObjectValue(pawnClass)
		}).ToObject());
	if (!pawn || !error.empty())
		Exception::Throw("GameInfo login failed: " + error);
	bool actorActuallySpawned = Level->Actors.size() != numActors;

	pawn->LoadProperties();

	// Assign the pawn to the viewport
	viewport->Actor() = pawn;
	viewport->Actor()->Player() = viewport;
	CallEvent(viewport->Actor(), EventName::Possess);

	// Transfer travel actors to the new map

	CallEvent(pawn, EventName::TravelPreAccept);

	Array<UActor*> acceptedActors;
	if (actorActuallySpawned && ClientTravelInfo.TravelType == ETravelType::TRAVEL_Relative)
	{
		std::string playerName = url.GetOption("Name");
		if (playerName.empty())
			playerName = packages->GetIniValue("system", "URL", "Name");

		auto it = travelInfo.find(playerName);
		if (!playerName.empty() && it != travelInfo.end())
		{
			acceptedActors = ActorTravelInfo::Accept(pawn, it->second);
		}
		else
		{
			if (travelInfo.empty())
				LogMessage("Skipping travel transfer. No travel data");
			else if (playerName.empty())
				LogMessage("Skipping travel transfer. Player name is empty");
			else
				LogMessage("Skipping travel transfer. Player '" + playerName + "' not found in travel info");
		}
	}

	for (UActor* actor : acceptedActors)
		CallEvent(actor, EventName::TravelPreAccept);

	CallEvent(LevelInfo->Game(), EventName::AcceptInventory, { ExpressionValue::ObjectValue(pawn) });

	for (UActor* actor : acceptedActors)
		CallEvent(actor, EventName::TravelPostAccept);

	CallEvent(pawn, EventName::TravelPostAccept);
	CallEvent(LevelInfo->Game(), EventName::PostLogin, { ExpressionValue::ObjectValue(pawn) });

	render->OnMapLoaded();
}

UZoneInfo* Engine::GetZoneActor(int zoneIndex)
{
	if (auto zone = UObject::TryCast<UZoneInfo>(Level->Model->Zones[zoneIndex].ZoneActor))
		return zone;
	else
		return LevelInfo;
}

UObject* Engine::FindObject(NameString name, NameString className)
{
	for (auto actor : Level->Actors)
	{
		if (actor && actor->Name == name && UObject::GetUClassFullName(actor) == className)
			return actor;
	}

	return nullptr;
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

std::string Engine::ParseClassName(std::string className)
{
	// Workaround for broken unrealscript code referencing windrv.windowsclient directly
	if (className == "windrv.windowsclient")
	{
		className = "ini:Engine.ViewportManager";
	}

	if (className.size() < 4 || className.substr(0, 4) != "ini:")
		return className;

	size_t pos = className.find_last_of('.');
	if (pos == std::string::npos)
		Exception::Throw("Parse error");

	NameString sectionName = className.substr(4, pos - 4);
	NameString keyName = className.substr(pos + 1);

	// Override the ini file for things that are internal in Surreal Engine
	if (sectionName == "Engine.Engine")
	{
		if (keyName == "GameRenderDevice" || keyName == "WindowedRenderDevice")
		{
			return renderdev->Class;
		}
		else if (keyName == "AudioDevice")
		{
			return audiodev->Class;
		}
		else if (keyName == "NetworkDevice")
		{
			return netdev->Class;
		}
		else if (keyName == "ViewportManager")
		{
			return client->Class;
		}
	}

	return packages->GetIniValue("system", sectionName, keyName);
}

std::string Engine::ConsoleCommand(UObject* context, const std::string& commandline, BitfieldBool& found)
{
	found = false;

	Array<std::string> args = GetArgs(commandline);
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
	else if (command == "stat" && args.size() == 2)
	{
		render->ShowRenderStats = 0;

		if (args[1] == "render")
			render->ShowRenderStats = 1;
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
		return window->GetAvailableResolutions();
	}
	else if (command == "getcolordepths")
	{
		return "32 16";
	}
	else if (command == "getcurrentres")
	{
		int width = window->GetPixelWidth();
		int height = window->GetPixelHeight();

		return std::to_string(width) + "x" + std::to_string(height);
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
		uint8_t index = Convert::to_uint8(args[1]);
		return keynames[index];
	}
	else if (command == "keybinding" && args.size() == 2)
	{
		const std::string& name = args[1];
		return keybindings[name];
	}
	else if ((command == "open" || command == "start") && args.size() == 2)
	{
		const std::string& maparg = args[1];

		UnrealURL url(maparg);

		for (auto& map : packages->GetMaps())
		{
			std::string mapname = fs::path(map).stem().string();

			if (StrCompare::equals_ignore_case(mapname, url.Map))
			{
				ClientTravel(url.ToString(), ETravelType::TRAVEL_Absolute, false);
				return {};
			}	
		}

		LogMessage("Couldn't find map " + maparg);
	}
	else if (command == "switchlevel" && args.size() == 2)
	{
		// This works like open/start, but keeps the difficulty level
		// As well as the game type
		const std::string& maparg = args[1];

		UnrealURL url(maparg);

		// First check if the provided URL has a difficulty option
		std::string difficulty = url.GetOption("difficulty");

		// If there isn't, try to get it from the current level's options
		if (difficulty.empty())
		{
			difficulty = LevelInfo->URL.GetOption("difficulty");
			if (!difficulty.empty())
				url.AddOrReplaceOption("difficulty=" + difficulty);
		}

		// If there still isn't, try to figure the current difficulty out using LevelInfo
		if (difficulty.empty())
		{
			if (LevelInfo->bDifficulty0())
				difficulty = "0";
			else if (LevelInfo->bDifficulty1())
				difficulty = "1";
			else if (LevelInfo->bDifficulty2())
				difficulty = "2";
			else if (LevelInfo->bDifficulty3())
				difficulty = "3";
			else
				difficulty = "2"; // Assume "Normal" difficulty

			url.AddOrReplaceOption("difficulty=" + difficulty);
		}

		for (auto& map : packages->GetMaps())
		{
			std::string mapname = fs::path(map).stem().string();

			if (StrCompare::equals_ignore_case(mapname, url.Map))
			{
				LevelInfo->NextURL() = url.ToString();
				return {};
			}
		}

		LogMessage("Couldn't find map " + maparg);
	}
	else if (command == "savegame" && (args.size() == 2 || args.size() == 3))
	{

		int32_t slotNum;
		// slotNum not being parsable shouldn't cause a crash
		try
		{
			slotNum = Convert::to_int32(args[1]);
		}
		catch (...)
		{
			return {};
		}

		SaveGameInfo.SaveGameSlot = slotNum;

		if (args.size() == 3)
			SaveGameInfo.SaveGameDescription = args[2];

		// SaveGameToSlot(slotNum, "");

		//LogMessage("SaveGame command not fully implemented yet!");
		return {};
	}
	else if (command == "get" && args.size() == 3)
	{
		NameString className = ParseClassName(args[1]);
		NameString propertyName = args[2];

		UClass* cls = packages->FindClass(className);
		if (!cls)
		{
			LogMessage("Could not find class '" + className.ToString() + "': " + commandline);
			return {};
		}

		if (className == renderdev->Class)
		{
			return renderdev->GetPropertyAsString(propertyName);
		}
		else if (className == audiodev->Class)
		{
			return audiodev->GetPropertyAsString(propertyName);
		}
		else if (className == netdev->Class)
		{
			return netdev->GetPropertyAsString(propertyName);
		}
		else if (className == client->Class)
		{
			return client->GetPropertyAsString(propertyName);
		}
		else
		{
			try
			{
				return cls->GetPropertyAsString(propertyName);
			}
			catch (const std::exception&)
			{
				LogMessage("Could not get property '" + propertyName.ToString() + "': " + commandline);
				return {};
			}
		}
	}
	else if (command == "set" && args.size() == 4)
	{
		NameString className = ParseClassName(args[1]);
		NameString propertyName = args[2];
		std::string value = args[3];

		// Special input setting handling
		if (className == "input")
		{
			keybindings[propertyName.ToString()] = value;
			packages->SetIniValue("user", "Engine.Input", propertyName, value);
			return {};
		}

		UClass* cls = packages->FindClass(className);
		if (!cls)
		{
			LogMessage("Could not find class '" + className.ToString() + "': " + commandline);
			return {};
		}

		if (className == renderdev->Class)
		{
			renderdev->SetPropertyFromString(propertyName, value);
		}
		else if (className == audiodev->Class)
		{
			audiodev->SetPropertyFromString(propertyName, value);
		}
		else if (className == netdev->Class)
		{
			netdev->SetPropertyFromString(propertyName, value);
		}
		else if (className == client->Class)
		{
			client->SetPropertyFromString(propertyName, value);
		}
		else
		{
			try
			{
				cls->SetPropertyFromString(propertyName, value);
			}
			catch (const std::exception&)
			{
				LogMessage("Could not set property '" + propertyName.ToString() + "': " + commandline);
			}
			return {};
		}
	}
	else if (command == "setres" && args.size() == 2)
	{
		window->SetResolution(args[1]);
	}
	else if (command == "togglefullscreen")
	{
		bool isFullscreen = window->IsFullscreen();

		// Get the resolution to SWITCH TO
		int width = isFullscreen ? client->WindowedViewportX : client->FullscreenViewportX;
		int height = isFullscreen ? client->WindowedViewportY : client->FullscreenViewportY;

		Size resolution;
		resolution.width = width;
		resolution.height = height;

		window->ToggleWindowFullscreen(resolution);

		return {};
	}
	else if (command == "prsq")
	{
		// Klingon Honor Guard: CD check
		// "mpgameplay"
		// "mpinstall"
		return "mpgameplay";
	}
	else if (command == "getsplash")
	{
		return khgSplashScreen ? "true" : "false";
	}
	else if (command == "setsplash")
	{
		khgSplashScreen = true;
		return {};
	}
	else if (command == "playavi")
	{
		PlayAVI(args);
		return {};
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

Array<std::string> Engine::GetArgs(const std::string& commandline)
{
	Array<std::string> args;
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

Array<std::string> Engine::GetSubcommands(const std::string& command)
{
	Array<std::string> subcommands;
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
	if (packages->MissingSESystemIni())
	{
		client->LoadProperties("WinDrv.WindowsClient");
		audiodev->LoadProperties("Galaxy.GalaxyAudioSubsystem");
		renderdev->LoadProperties("D3DDrv.Direct3DRenderDevice");
	}
	else
	{
		client->LoadProperties();
		audiodev->LoadProperties();
		renderdev->LoadProperties();
	}

#ifdef WIN32
	windowingSystemName = packages->GetIniValue("System", "Engine.SurrealWindowSystem", "WindowSystem", "Win32");
#else
	windowingSystemName = packages->GetIniValue("System", "Engine.SurrealWindowSystem", "WindowSystem", "SDL2");
#endif
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

	TickWindow();
	if (tickDebugger)
		tickDebugger();

	if (!viewport->Actor())
		return;

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

void Engine::OpenWindow()
{
	if (!window)
		window = GameWindow::Create(this);

	int width = client->StartupFullscreen ? client->FullscreenViewportX : client->WindowedViewportX;
	int height = client->StartupFullscreen ? client->FullscreenViewportY : client->WindowedViewportY;
	bool fullscreen = client->StartupFullscreen;

	std::string versionString = !LaunchInfo.gameVersionString.empty() ? " (v" + LaunchInfo.gameVersionString + ")" : "";

	window->SetWindowTitle(LaunchInfo.gameName + versionString + " - Surreal Engine");
	window->SetFrameGeometry(Rect::xywh(0.0, 0.0, width, height));

	if (fullscreen)
		window->ShowFullscreen();
	else
		window->ShowNormal();
}

void Engine::CloseWindow()
{
	window.reset();
}

void Engine::TickWindow()
{
	if (window && engine->LaunchInfo.engineVersion > 219)
	{
		if (viewport->bShowWindowsMouse() && viewport->bWindowsMouseAvailable())
			window->UnlockCursor();
		else
			window->LockCursor();
	}

	InputEvent(IK_MouseX, IST_Axis, 0);
	InputEvent(IK_MouseY, IST_Axis, 0);

	GameWindow::ProcessEvents();

	if (MouseMoveX != 0 || MouseMoveY != 0)
	{
		int dx = MouseMoveX;
		int dy = MouseMoveY;
		MouseMoveX = 0;
		MouseMoveY = 0;

		// Send to input subsystem.
		if (dx)
			InputEvent(IK_MouseX, IST_Axis, dx);
		if (dy)
			InputEvent(IK_MouseY, IST_Axis, -dy);
	}
}

void Engine::OnWindowPaint()
{
}

void Engine::OnWindowMouseMove(const Point& pos)
{
	if (playingAvi)
		return;

	if (engine->LaunchInfo.engineVersion > 219)
	{
		viewport->WindowsMouseX() = (float)(pos.x * window->GetDpiScale());
		viewport->WindowsMouseY() = (float)(pos.y * window->GetDpiScale());
	}
}

void Engine::OnWindowMouseDown(const Point& pos, EInputKey key)
{
	if (playingAvi)
		return;

	InputEvent(key, IST_Press);
}

void Engine::OnWindowMouseDoubleclick(const Point& pos, EInputKey key)
{
}

void Engine::OnWindowMouseUp(const Point& pos, EInputKey key)
{
	if (playingAvi)
		return;

	InputEvent(key, IST_Release);
}

void Engine::OnWindowMouseWheel(const Point& pos, EInputKey key)
{
	if (playingAvi)
		return;

	InputEvent(key, IST_Press);
	InputEvent(key, IST_Release);
}

void Engine::OnWindowRawMouseMove(int dx, int dy)
{
	if (playingAvi)
		return;

	MouseMoveX += dx;
	MouseMoveY += dy;
}

void Engine::OnWindowKeyChar(std::string chars)
{
	if (playingAvi)
		return;

	Key(chars);
}

void Engine::OnWindowKeyDown(EInputKey key)
{
	if (playingAvi)
	{
		if (key == EInputKey::IK_Escape)
			skipAvi = true;
		return;
	}

	InputEvent(key, IST_Press);
}

void Engine::OnWindowKeyUp(EInputKey key)
{
	if (playingAvi)
		return;

	InputEvent(key, IST_Release);
}

void Engine::OnWindowGeometryChanged()
{
}

void Engine::OnWindowClose()
{
	quit = true;
}

void Engine::OnWindowActivated()
{
	//SetPause(false);
}

void Engine::OnWindowDeactivated()
{
	//SetPause(true);
}

void Engine::OnWindowDpiScaleChanged()
{
}

void Engine::LockCursor()
{
	if (window)
		window->LockCursor();
}

void Engine::UnlockCursor()
{
	if (window)
		window->UnlockCursor();
}

void Engine::Key(std::string key)
{
	if (Frame::RunState != FrameRunState::Running || playingAvi)
		return;

	for (char c : key)
	{
		CallEvent(console, EventName::KeyType, { ExpressionValue::ByteValue(c) });
	}
}

void Engine::InputEvent(EInputKey key, EInputType type, int delta)
{
	if (Frame::RunState != FrameRunState::Running || playingAvi)
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

bool Engine::ExecCommand(const Array<std::string>& args)
{
	for (UObject* target : { static_cast<UObject*>(viewport->Actor()), static_cast<UObject*>(console) })
	{
		if (!target)
			continue;

		UFunction* func = FindEventFunction(target, args[0]);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Exec))
		{
			Array<ExpressionValue> vmArgs;
			int argindex = 0;
			for (UField* field = func->Children; field != nullptr; field = field->Next)
			{
				UProperty* prop = UObject::TryCast<UProperty>(field);
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
		Array<std::string> args = GetArgs(commandline);
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
	m_GamePaused = value;
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
