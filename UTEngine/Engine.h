#pragma once

#include "Math/vec.h"
#include "Math/mat.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include <set>
#include <list>

class Collision;
class UTRenderer;
class PackageManager;
class UObject;
class ULevel;
class UModel;
class DisplayWindow;
class UTexture;
class UActor;
class UFont;
class UMesh;
class ULodMesh;
class USkeletalMesh;
class ULevelSummary;
class ULevelInfo;
class UZoneInfo;
class UClient;
class UViewport;
class UCanvas;
class UConsole;
class UPlayerPawn;
class UGameInfo;
class UGameReplicationInfo;
class UPlayerReplicationInfo;
class BspSurface;
class BspNode;
class LightMapIndex;
class FrustumPlanes;
class AudioPlayer;
class Rotator;
class ExpressionValue;
struct FTextureInfo;
struct FSceneNode;
struct FSurfaceFacet;
struct MeshFace;


class Engine
{
public:
	Engine();
	~Engine();

	void Run();
	void LoadMap(std::string mapName);
	std::string ConsoleCommand(UObject* context, const std::string& command, bool& found);

	void WindowClose(DisplayWindow* window);
	void Key(DisplayWindow* window, std::string key);
	void InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta = 0);
	void SetPause(bool value);

	UClient* client = nullptr;
	UViewport* viewport = nullptr;
	UCanvas* canvas = nullptr;
	UConsole* console = nullptr;

	ULevelSummary* EntryLevelSummary = nullptr;
	ULevelInfo* EntryLevelInfo = nullptr;
	ULevel* EntryLevel = nullptr;
	UGameInfo* EntryGameInfo = nullptr;

	ULevelSummary* LevelSummary = nullptr;
	ULevelInfo* LevelInfo = nullptr;
	ULevel* Level = nullptr;
	UGameInfo* GameInfo = nullptr;

	std::list<std::string> Log;

	std::unique_ptr<PackageManager> packages;
	std::unique_ptr<DisplayWindow> window;
	std::unique_ptr<UTRenderer> renderer;
	std::unique_ptr<Collision> collision;
	std::unique_ptr<AudioPlayer> audioplayer;
	std::unique_ptr<AudioPlayer> soundslot;

	float CalcTimeElapsed();

	struct ActorPos
	{
		float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;
		vec3 Location = { 200.0f, 1200.0f, 1300.0f };
	};
	
	ActorPos Camera;

	struct
	{
		float SpeedX = 0.08f;
		float SpeedY = 0.08f;
	} Mouse;

	struct
	{
		bool Forward = false;
		bool Backward = false;
		bool StrafeLeft = false;
		bool StrafeRight = false;
		bool Jump = false;
		bool Crouch = false;
	} Buttons;

	bool quit = false;

	uint64_t lastTime = 0;
};

extern Engine* engine;
