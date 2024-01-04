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
	void UnloadMap();
	void LoginPlayer();
	std::string ConsoleCommand(UObject* context, const std::string& command, BitfieldBool& found);

	void UpdateInput(float timeElapsed);
	void InputCommand(const std::string& command, EInputKey key, int delta);

	void LockCursor();
	void UnlockCursor();

	bool ExecCommand(const std::vector<std::string>& args);
	std::vector<std::string> GetArgs(const std::string& commandline);
	std::vector<std::string> GetSubcommands(const std::string& commandline);

	void UpdateAudio();

	void OpenWindow(int width, int height, bool fullscreen);
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
	std::string GetRenderDeviceProperty(NameString propertyName);
	std::string GetAudioDeviceProperty(NameString propertyName);
	std::string GetNetworkDeviceProperty(NameString propertyName);
	std::string GetViewportManagerProperty(NameString propertyName);

	void SetRenderDeviceProperty(NameString propertyName, const std::string& value);
	void SetAudioDeviceProperty(NameString propertyName, const std::string& value);
	void SetNetworkDeviceProperty(NameString propertyName, const std::string& value);
	void SetViewportManagerProperty(NameString propertyName, const std::string& value);

	// This is just temporary for now. They should be on our subsystem classes
	struct
	{
		struct
		{
			std::string Class = "Surreal.RenderDevice";
			bool Translucency = true;
			bool VolumetricLighting = true;
			bool ShinySurfaces = true;
			bool Coronas = true;
			bool HighDetailActors = true;
		} RenderDevice;
		struct
		{
			std::string Class = "Surreal.AudioDevice";
			bool UseFilter = true;
			bool UseSurround = true;
			bool UseStereo = true;
			bool UseCDMusic = false;
			bool UseDigitalMusic = true;
			bool UseSpatial = true;
			bool UseReverb = true;
			bool Use3dHardware = true;
			bool LowSoundQuality = false;
			bool ReverseStereo = false;
			int Latency = 40;
			int OutputRate = 44100;
			int Channels = 16;
			uint8_t MusicVolume = 160;
			uint8_t SoundVolume = 200;
			float AmbientFactor = 0.7f;
		} AudioDevice;
		struct
		{
			std::string Class = "Surreal.NetworkDevice";
		} NetworkDevice;
		struct
		{
			std::string Class = "WinDrv.WindowsClient"; // ideally Surreal.ViewportManager, but some seriously sloppy code in unrealscript references WinDrv directly
			bool StartupFullscreen = false;
			int WindowedViewportX = 1920;
			int WindowedViewportY = 1080;
			int WindowedColorBits = 32;
			int FullscreenViewportX = 0;
			int FullscreenViewportY = 0;
			int FullscreenColorBits = 32;
			float Brightness = 0.5f;
			bool UseJoystick = false;
			bool UseDirectInput = true;
			int MinDesiredFrameRate = 200;
			bool Decals = true;
			bool NoDynamicLights = false;
			std::string TextureDetail = "High";
			std::string SkinDetail = "High";
		} ViewportManager;
	} Subsystem;

	UClient* client = nullptr;
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

	// Collision debug
	BspNode* PlayerBspNode = nullptr;
	vec3 PlayerHitNormal;
	vec3 PlayerHitLocation;

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
	std::function<void(const LogMessageLine& line)> printLogDebugger;
};

extern Engine* engine;
