
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

	// The entry map is the map you see in the game when no other map is playing. For example when disconnected from a server. It is always loaded and running.
	LoadMap("Entry");
	EntryLevelSummary = LevelSummary;
	EntryLevelInfo = LevelInfo;
	EntryLevel = Level;

	//std::string mapName = "CityIntro";
	//std::string mapName = "UT-Logo-Map";
	//std::string mapName = "DM-Liandri";
	//std::string mapName = "DM-Codex";
	std::string mapName = "DM-Barricade";
	//std::string mapName = "DM-Deck16][";
	//std::string mapName = "DM-KGalleon";
	//std::string mapName = "DM-Turbine";
	//std::string mapName = "DM-Tempest";
	//std::string mapName = "DM-Grinder";
	//std::string mapName = "DM-HyperBlast";
	//std::string mapName = "DM-Peak";
	//std::string mapName = "CTF-Coret";
	//std::string mapName = "CTF-Dreary";
	//std::string mapName = "CTF-Command";
	//std::string mapName = "CTF-November";
	//std::string mapName = "CTF-Gauntlet";
	//std::string mapName = "CTF-EternalCave";
	//std::string mapName = "CTF-Niven";
	//std::string mapName = "CTF-Face";
	//std::string mapName = "DOM-Sesmar";
	//std::string mapName = "EOL_Deathmatch";
	//std::string mapName = "UTCredits";

	if (packages->IsUnreal1())
	{
		LoadMap("Dark");
		while (!quit)
		{
			float elapsed = CalcTimeElapsed();
			window->Tick();

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
			renderer->scene.DrawScene();
			renderer->scene.DrawTimedemoStats();
			device->EndFlash(0.5f, vec4(1.0f, 0.0f, 0.0f, 1.0f));
			device->EndScenePass();
			device->EndFrame(true);
		}
		return;
	}

	LoadMap(mapName);

	CallEvent(console, "VideoChange");
	CallEvent(console, "NotifyLevelChange");

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

		if (firstCall) // Unscript execution doesn't work well enough for us to use the viewport actor as our camera yet
		{
			firstCall = false;
			Camera.Location = viewport->Actor()->Location();
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

void Engine::LoadMap(std::string mapName)
{
	// Load map objects
	Package* package = packages->GetPackage(mapName);

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

	// Spawn GameInfo actor
	std::string gameName = packages->GetIniValue("system", "Engine.Engine", "DefaultGame");
	if (gameName.empty() || gameName.find('.') == std::string::npos) gameName = "Botpack.DeathMatchPlus";
	std::string gamePackageName = gameName.substr(0, gameName.find('.'));
	std::string gameClassName = gameName.substr(gameName.find('.') + 1);
	UClass* gameInfoClass = UObject::Cast<UClass>(packages->GetPackage(gamePackageName)->GetUObject("Class", gameClassName));
	GameInfo = UObject::Cast<UGameInfo>(packages->NewObject("gameinfo", gamePackageName, gameClassName));
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
		if (LevelInfo->HasProperty("Song"))
		{
			auto music = UObject::Cast<UMusic>(LevelInfo->GetUObject("Song"));
			if (music)
				audioplayer = AudioPlayer::Create(AudioSource::CreateMod(music->Data));
		}
	}
	else if (command == "stopsong")
	{
		audioplayer.reset();
	}
	else
	{
		found = false;
	}
	return {};
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

	CallEvent(console, "KeyEvent", { ExpressionValue::ByteValue(key), ExpressionValue::ByteValue(type), ExpressionValue::FloatValue((float)delta) });

	if (!viewport->bShowWindowsMouse())
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
		UFunction* func = Frame::Callstack.back()->Func;
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
