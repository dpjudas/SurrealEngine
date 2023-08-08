#pragma once

#include "Math/vec.h"
#include "Math/mat.h"
#include "Math/floating.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "UObject/UObject.h"
#include "GameFolder.h"
#include <set>
#include <list>

class RenderSubsystem;
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
class AudioSubsystem;
class Rotator;
class ExpressionValue;
class UnrealURL;
struct FTextureInfo;
struct FSceneNode;
struct FSurfaceFacet;
struct MeshFace;

struct LogMessageLine
{
	float Time;
	std::string Source;
	std::string Text;
};

class Engine : public DisplayWindowHost
{
public:
	Engine(GameLaunchInfo launchinfo);
	~Engine();

	void Run();
	void ClientTravel(const std::string& URL, uint8_t travelType, bool transferItems);
	UnrealURL GetDefaultURL(const std::string& map);
	void LoadEntryMap();
	void LoadMap(const UnrealURL& url, const std::map<std::string, std::string>& travelInfo = {});
	void LoginPlayer();
	std::string ConsoleCommand(UObject* context, const std::string& command, bool& found);

	void UpdateInput(float timeElapsed);
	void InputCommand(const std::string& command, EInputKey key, int delta);

	void LockCursor();
	void UnlockCursor();

	bool ExecCommand(const std::vector<std::string>& args);
	std::vector<std::string> GetArgs(const std::string& commandline);
	std::vector<std::string> GetSubcommands(const std::string& commandline);

	void UpdateAudio();

	void Key(DisplayWindow* window, std::string key) override;
	void InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta = 0) override;
	void FocusChange(bool lost) override { SetPause(lost); }
	void MouseMove(float x, float y) override;
	bool MouseCursorVisible() override;
	void WindowClose(DisplayWindow* window) override;

	void SetPause(bool value);

	UClient* client = nullptr;
	UViewport* viewport = nullptr;
	UCanvas* canvas = nullptr;
	UConsole* console = nullptr;

	ULevelInfo* EntryLevelInfo = nullptr;
	ULevel* EntryLevel = nullptr;
	UGameInfo* EntryGameInfo = nullptr;

	ULevelInfo* LevelInfo = nullptr;
	ULevel* Level = nullptr;
	UGameInfo* GameInfo = nullptr;
	struct
	{
		std::string URL;
		uint8_t TravelType = 0;
		bool TransferItems = false;
	} ClientTravelInfo;

	void LogMessage(const std::string& message);
	void LogUnimplemented(const std::string& message);

	std::list<LogMessageLine> Log;

	GameLaunchInfo LaunchInfo;
	std::unique_ptr<PackageManager> packages;
	std::unique_ptr<DisplayWindow> window; // TODO: Move into UViewport
	std::unique_ptr<RenderSubsystem> render;
	std::unique_ptr<AudioSubsystem> audio;

	float CalcTimeElapsed();

	UActor* CameraActor = nullptr;
	vec3 CameraLocation;
	Rotator CameraRotation;
	float CameraFovAngle = 95.0f;

	// Collision debug
	BspNode* PlayerBspNode = nullptr;
	vec3 PlayerHitNormal;
	vec3 PlayerHitLocation;

	bool quit = false;

	uint64_t lastTime = 0;

	void LoadEngineSettings();
	int WindowedViewportX;
	int WindowedViewportY;
	int FullscreenViewportX;
	int FullscreenViewportY;
	bool StartupFullscreen;

	void LoadKeybindings();
	std::map<std::string, std::string> keybindings;
	std::map<std::string, std::string> inputAliases;
	static const char* keynames[256];

	struct ActiveInputAxis
	{
		float Value;
		EInputKey Key;
	};

	std::map<std::string, EInputKey> activeInputButtons;
	std::map<std::string, ActiveInputAxis> activeInputAxes;

	std::function<void()> tickDebugger;
};

extern Engine* engine;
