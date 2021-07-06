
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
#include "Viewport/Viewport.h"
#include "RenderDevice/RenderDevice.h"
#include "Audio/AudioPlayer.h"
#include "Audio/AudioSource.h"
#include "VM/Frame.h"
#include "VM/NativeFuncExtractor.h"
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

ExpressionValue Engine::InvokeEvent(UObject* Context, const std::string& name, std::vector<ExpressionValue> args)
{
	// Search states first

	for (UClass* cls = Context->Base; cls != nullptr; cls = cls->Base)
	{
		for (UField* field = cls->Children; field != nullptr; field = field->Next)
		{
			UState* state = UObject::TryCast<UState>(field);
			if (state && state->Name == Context->StateName)
			{
				for (UField* field2 = state->Children; field2 != nullptr; field2 = field2->Next)
				{
					UFunction* func = UObject::TryCast<UFunction>(field2);
					if (func && func->Name == name)
					{
						return Frame::Call(func, Context, std::move(args));
					}
				}
			}
		}
	}

	// Search normal member functions next

	for (UClass* cls = Context->Base; cls != nullptr; cls = cls->Base)
	{
		for (UField* field = cls->Children; field != nullptr; field = field->Next)
		{
			UFunction* func = UObject::TryCast<UFunction>(field);
			if (func && func->Name == name)
			{
				return Frame::Call(func, Context, std::move(args));
			}
		}
	}

	throw std::runtime_error("Event " + name + " not found on object");
}

void Engine::Run()
{
	viewport = Viewport::Create(this);
	//viewport->OpenWindow(1800, 950, false);
	viewport->OpenWindow(1920 * 4, 1080 * 4, true);

	collision = std::make_unique<Collision>();
	renderer = std::make_unique<UTRenderer>();

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
	UViewport* playerviewport = UObject::Cast<UViewport>(packages->NewObject("viewport", "Engine", "Viewport"));
	UCanvas* canvas = UObject::Cast<UCanvas>(packages->NewObject("canvas", "Engine", "Canvas"));
	UConsole* console = UObject::Cast<UConsole>(packages->NewObject("console", "UTMenu", "UTConsole"));
	UPlayerPawn* pawn = UObject::Cast<UPlayerPawn>(packages->NewObject("pawn", "Engine", "PlayerPawn"));
	UGameInfo* gameinfo = UObject::Cast<UGameInfo>(packages->NewObject("gameinfo", "Engine", "GameInfo"));
	UPlayerReplicationInfo* repinfo = UObject::Cast<UPlayerReplicationInfo>(packages->NewObject("repinfo", "Engine", "PlayerReplicationInfo"));

	gameinfo->Level() = LevelInfo;
	LevelInfo->Game() = gameinfo;

	console->Viewport() = playerviewport;
	console->FrameX() = (float)viewport->SizeX;
	console->FrameY() = (float)viewport->SizeY;

	canvas->Viewport() = playerviewport;
	canvas->SizeX() = viewport->SizeX;
	canvas->SizeY() = viewport->SizeY;

	playerviewport->Console() = console;
	playerviewport->Actor() = pawn;

	pawn->Level() = LevelInfo;
	pawn->PlayerReplicationInfo() = repinfo;

	InvokeEvent(console, "VideoChange", { });
	InvokeEvent(console, "NotifyLevelChange", { });
	//InvokeEvent(console, "ConnectFailure", { ExpressionValue::StringValue("404"), ExpressionValue::StringValue("unreal://foobar") });
	//ExpressionValue result = InvokeEvent(console, "KeyType", { ExpressionValue::ByteValue(27/*0xc0*/) });
	//ExpressionValue result2 = InvokeEvent(console, "KeyEvent", { ExpressionValue::ByteValue(27/*0xc0*/), ExpressionValue::ByteValue(1), ExpressionValue::FloatValue(0.0f) });

	while (!quit)
	{
		float elapsed = CalcTimeElapsed();
		Tick(elapsed);

		//InvokeEvent(console, "Tick", { ExpressionValue::FloatValue(elapsed) });

		for (UTexture* tex : engine->renderer->Textures)
			tex->Update();

		RenderDevice* device = viewport->GetRenderDevice();
		device->BeginFrame();

		//InvokeEvent(console, "PreRender", { ExpressionValue::ObjectValue(canvas) });
		renderer->scene.DrawScene();
		//InvokeEvent(console, "PostRender", { ExpressionValue::ObjectValue(canvas) });

		device->EndFrame(true);
	}

	viewport->CloseWindow();
	viewport.reset();
}

void Engine::Tick(float timeElapsed)
{
	viewport->Tick();

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

void Engine::Key(Viewport* viewport, std::string key)
{
}

void Engine::InputEvent(Viewport* viewport, EInputKey key, EInputType type, int delta)
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

void Engine::WindowClose(Viewport* viewport)
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
