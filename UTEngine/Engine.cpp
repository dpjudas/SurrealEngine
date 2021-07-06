
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
#include <chrono>
#include <set>

Engine* engine = nullptr;

Engine::Engine()
{
	engine = this;
	packages = std::make_unique<PackageManager>("C:\\Games\\UnrealTournament436");
}

Engine::~Engine()
{
	engine = nullptr;
}

void Engine::Run()
{
	window = Window::Create(this);
	//window->OpenWindow(1800, 950, false);
	window->OpenWindow(1920 * 4, 1080 * 4, true);

	collision = std::make_unique<Collision>();
	renderer = std::make_unique<UTRenderer>();

	//LoadMap("CityIntro");
	LoadMap("DM-Liandri");
	//LoadMap("DM-Codex");
	//LoadMap("DM-Barricade");
	//LoadMap("DM-Deck16][");
	//LoadMap("DM-KGalleon");
	//LoadMap("DM-Turbine");
	//LoadMap("DM-Tempest");
	//LoadMap("DM-Grinder");
	//LoadMap("DM-HyperBlast");
	//LoadMap("CTF-Coret");
	//LoadMap("CTF-Dreary");
	//LoadMap("CTF-Command");
	//LoadMap("CTF-November");
	//LoadMap("CTF-Gauntlet");
	//LoadMap("CTF-EternalCave");
	//LoadMap("CTF-Niven");
	//LoadMap("CTF-Face");

#if 0
	if (LevelInfo->HasProperty("Song"))
	{
		auto music = UObject::Cast<UMusic>(LevelInfo->GetUObject("Song"));
		audioplayer = AudioPlayer::Create(AudioSource::CreateMod(music->Data));
	}
#endif

	UClient* client = UObject::Cast<UClient>(packages->NewObject("client", "Engine", "Client"));
	UViewport* viewport = UObject::Cast<UViewport>(packages->NewObject("viewport", "Engine", "Viewport"));
	UCanvas* canvas = UObject::Cast<UCanvas>(packages->NewObject("canvas", "Engine", "Canvas"));
	UConsole* console = UObject::Cast<UConsole>(packages->NewObject("console", "UTMenu", "UTConsole"));
	UPlayerPawn* pawn = UObject::Cast<UPlayerPawn>(packages->NewObject("pawn", "Engine", "PlayerPawn"));
	UGameInfo* gameinfo = UObject::Cast<UGameInfo>(packages->NewObject("gameinfo", "Engine", "GameInfo"));
	UPlayerReplicationInfo* repinfo = UObject::Cast<UPlayerReplicationInfo>(packages->NewObject("repinfo", "Engine", "PlayerReplicationInfo"));

	gameinfo->Level() = LevelInfo;
	LevelInfo->Game() = gameinfo;

	console->Viewport() = viewport;
	console->FrameX() = (float)window->SizeX;
	console->FrameY() = (float)window->SizeY;

	canvas->Viewport() = viewport;
	canvas->SizeX() = window->SizeX;
	canvas->SizeY() = window->SizeY;

	viewport->Console() = console;
	viewport->Actor() = pawn;

	pawn->Level() = LevelInfo;
	pawn->PlayerReplicationInfo() = repinfo;

	CallEvent(console, "VideoChange");
	CallEvent(console, "NotifyLevelChange");

	//ExpressionValue result = CallEvent(console, "KeyType", { ExpressionValue::ByteValue(27/*0xc0*/) });
	//ExpressionValue result2 = CallEvent(console, "KeyEvent", { ExpressionValue::ByteValue(27/*0xc0*/), ExpressionValue::ByteValue(1), ExpressionValue::FloatValue(0.0f) });

	while (!quit)
	{
		float elapsed = CalcTimeElapsed();
		Tick(elapsed);

		//CallEvent(console, "Tick", { ExpressionValue::FloatValue(elapsed) });

		for (UTexture* tex : engine->renderer->Textures)
			tex->Update();

		RenderDevice* device = window->GetRenderDevice();
		device->BeginFrame();

		//CallEvent(console, "PreRender", { ExpressionValue::ObjectValue(canvas) });
		renderer->scene.DrawScene();
		//CallEvent(console, "PostRender", { ExpressionValue::ObjectValue(canvas) });

		device->EndFrame(true);
	}

	window->CloseWindow();
	window.reset();
}

void Engine::Tick(float timeElapsed)
{
	window->Tick();

	for (UActor* actor : level->Actors)
	{
		if (actor)
		{
			auto& rot = actor->Rotation();
			auto& rate = actor->RotationRate();
			rot.Yaw = (rot.Yaw + (int)(static_cast<int16_t>(rate.Yaw & 0xffff) * timeElapsed)) & 0xffff;
			rot.Pitch = (rot.Pitch + (int)(static_cast<int16_t>(rate.Pitch & 0xffff) * timeElapsed)) & 0xffff;
			rot.Roll = (rot.Roll + (int)(static_cast<int16_t>(rate.Roll & 0xffff) * timeElapsed)) & 0xffff;
		}
	}

	renderer->AutoUVTime += timeElapsed;

	quaternion viewrotation = normalize(quaternion::euler(radians(-Camera.Pitch), radians(-Camera.Roll), radians(-Camera.Yaw), EulerOrder::yxz));
	vec3 vel = { 0.0f };
	if (Buttons.StrafeLeft) vel.x = 1.0f;
	if (Buttons.StrafeRight) vel.x = -1.0f;
	if (Buttons.Forward) vel.y = 1.0f;
	if (Buttons.Backward) vel.y = -1.0f;
	if (vel != vec3(0.0f))
		vel = normalize(vel);
	vel = inverse(viewrotation) * vel;
	Camera.Location += vel * (timeElapsed * 650.0f);
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

void Engine::Key(Window* viewport, std::string key)
{
}

void Engine::InputEvent(Window* viewport, EInputKey key, EInputType type, int delta)
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
	case VK_SPACE:
		Buttons.Jump = (type == IST_Press);
		break;
	case VK_SHIFT:
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

void Engine::SetPause(bool value)
{
}

void Engine::WindowClose(Window* viewport)
{
	quit = true;
}

void Engine::LoadMap(const std::string& packageName)
{
	Package* package = packages->GetPackage(packageName);

	LevelSummary = UObject::Cast<ULevelSummary>(package->GetUObject("LevelSummary", "LevelSummary"));
	LevelInfo = UObject::Cast<ULevelInfo>(package->GetUObject("LevelInfo", "LevelInfo0"));
	level = UObject::Cast<ULevel>(package->GetUObject("Level", "MyLevel"));

	std::set<UActor*> lightset;
	for (UActor* light : level->Model->Lights)
	{
		lightset.insert(light);
	}

	for (UActor* light : lightset)
		engine->renderer->Lights.push_back(light);

	for (BspSurface& surf : level->Model->Surfaces)
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

	for (UActor* actor : level->Actors)
	{
		if (actor && actor->Base)
		{
			if (actor->Base->Name == "PlayerStart")
			{
				Camera.Location = actor->Location();
				Camera.Location.z += 70;

				auto prop = actor->Rotation();
				Camera.Yaw = prop.YawDegrees() - 90.0f;
				Camera.Pitch = prop.PitchDegrees();
				Camera.Roll = prop.RollDegrees();
			}
			else if (actor->Base->Name == "SkyZoneInfo")
			{
				SkyZoneInfo = actor;
			}
		}
	}
}
