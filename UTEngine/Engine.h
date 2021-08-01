#pragma once

#include "Math/vec.h"
#include "Math/mat.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "UObject/UObject.h"
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
class AudioMixer;
class Rotator;
class ExpressionValue;
class UnrealURL;
struct FTextureInfo;
struct FSceneNode;
struct FSurfaceFacet;
struct MeshFace;

struct LogMessageLine
{
	std::string Source;
	std::string Text;
};

class Engine
{
public:
	Engine();
	~Engine();

	std::string FindGameFolder();
	void Run();
	void ClientTravel(const std::string& URL, uint8_t travelType, bool transferItems);
	UnrealURL GetDefaultURL(const std::string& map);
	void LoadEntryMap();
	void LoadMap(const UnrealURL& url);
	std::string ConsoleCommand(UObject* context, const std::string& command, bool& found);

	void WindowClose(DisplayWindow* window);
	void UpdateInput();
	void Key(DisplayWindow* window, std::string key);
	void InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta = 0);
	void InputCommand(const std::string& command, EInputKey key, int delta);
	void SetPause(bool value);

	std::vector<std::string> GetArgs(const std::string& commandline);
	std::vector<std::string> GetSubcommands(const std::string& commandline);

	void UpdateAudio();

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

	void LogMessage(const std::string& message);
	void LogUnimplemented(const std::string& message);

	std::list<LogMessageLine> Log;

	std::unique_ptr<PackageManager> packages;
	std::unique_ptr<DisplayWindow> window;
	std::unique_ptr<UTRenderer> renderer;
	std::unique_ptr<Collision> collision;
	std::unique_ptr<AudioMixer> audio;

	float CalcTimeElapsed();

	struct ActorPos
	{
		float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;
		vec3 Location = { 200.0f, 1200.0f, 1300.0f };
	};
	
	UActor* CameraActor = nullptr;
	vec3 CameraLocation;
	Rotator CameraRotation;

	bool quit = false;

	uint64_t lastTime = 0;

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
};

extern Engine* engine;
