#pragma once

#include "Logger.h"
#include "Math/vec.h"
#include "Math/mat.h"
#include "Math/floating.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "UObject/UObject.h"
#include "UObject/UnrealURL.h"
#include "GameFolder.h"
#include <set>
#include <list>

class RenderSubsystem;
class PackageManager;
class UObject;
class ULevel;
class UModel;
class GameWindow;
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
class UGameEngine;
class USurrealRenderDevice;
class USurrealAudioDevice;
class USurrealNetworkDevice;
class USurrealClient;
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

class Engine : public GameWindowHost
{
public:
	Engine(GameLaunchInfo launchinfo);
	~Engine();

	void Run();
	void ClientTravel(const std::string& URL, uint8_t travelType, bool transferItems);
	UnrealURL GetDefaultURL(const std::string& map);
	void LoadEntryMap();
	void LoadMap(const UnrealURL& url, const std::map<std::string, std::string>& travelInfo = {});
	void UnloadMap();
	void LoginPlayer();

	UObject* FindObject(NameString name, NameString className);

	std::string ConsoleCommand(UObject* context, const std::string& command, BitfieldBool& found);

	void UpdateInput(float timeElapsed);
	void InputCommand(const std::string& command, EInputKey key, int delta);

	void LockCursor();
	void UnlockCursor();

	bool ExecCommand(const std::vector<std::string>& args);
	std::vector<std::string> GetArgs(const std::string& commandline);
	std::vector<std::string> GetSubcommands(const std::string& commandline);

	void UpdateAudio();

	void OpenWindow();
	void CloseWindow();
	void TickWindow();

	void Key(std::string key);
	void InputEvent(EInputKey key, EInputType type, int delta = 0);

	void OnWindowPaint() override;
	void OnWindowMouseMove(const Point& pos) override;
	void OnWindowMouseDown(const Point& pos, EInputKey key) override;
	void OnWindowMouseDoubleclick(const Point& pos, EInputKey key) override;
	void OnWindowMouseUp(const Point& pos, EInputKey key) override;
	void OnWindowMouseWheel(const Point& pos, EInputKey key) override;
	void OnWindowRawMouseMove(int dx, int dy) override;
	void OnWindowKeyChar(std::string chars) override;
	void OnWindowKeyDown(EInputKey key) override;
	void OnWindowKeyUp(EInputKey key) override;
	void OnWindowGeometryChanged() override;
	void OnWindowClose() override;
	void OnWindowActivated() override;
	void OnWindowDeactivated() override;
	void OnWindowDpiScaleChanged() override;

	void SetPause(bool value);

	std::string ParseClassName(std::string className);

	UGameEngine* gameengine = nullptr;
	USurrealRenderDevice* renderdev = nullptr;
	USurrealAudioDevice* audiodev = nullptr;
	USurrealNetworkDevice* netdev = nullptr;
	USurrealClient* client = nullptr;
	UViewport* viewport = nullptr;
	UCanvas* canvas = nullptr;
	UConsole* console = nullptr;

	ULevelInfo* EntryLevelInfo = nullptr;
	ULevel* EntryLevel = nullptr;
	UGameInfo* EntryGameInfo = nullptr;

	ULevelInfo* LevelInfo = nullptr;
	ULevel* Level = nullptr;
	Package* LevelPackage = nullptr;
	UGameInfo* GameInfo = nullptr;
	UTexture* DefaultTexture = nullptr;
	struct
	{
		UnrealURL URL;
		uint8_t TravelType = 0;
		bool TransferItems = false;
	} ClientTravelInfo;

	GameLaunchInfo LaunchInfo;
	std::unique_ptr<PackageManager> packages;
	std::unique_ptr<GameWindow> window; // TODO: Move into UViewport
	std::unique_ptr<RenderSubsystem> render;
	std::unique_ptr<AudioSubsystem> audio;

	int MouseMoveX = 0;
	int MouseMoveY = 0;

	float CalcTimeElapsed();

	int ViewportX = 0;
	int ViewportY = 0;
	int ViewportWidth = 0;
	int ViewportHeight = 0;
	UActor* CameraActor = nullptr;
	vec3 CameraLocation = vec3(0.0f);
	Rotator CameraRotation = Rotator(0,0,0);
	float CameraFovAngle = 95.0f;

	std::string windowingSystemName;

	// Collision debug
	BspNode* PlayerBspNode = nullptr;
	vec3 PlayerHitNormal = vec3(0.0f);
	vec3 PlayerHitLocation = vec3(0.0f);

	bool quit = false;

	uint64_t lastTime = 0;

	void LoadEngineSettings();

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
