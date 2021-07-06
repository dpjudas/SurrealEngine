#pragma once

#include "Math/vec.h"
#include "Math/mat.h"
#include "RenderDevice/RenderDevice.h"
#include <set>

class Collision;
class UTRenderer;
class PackageManager;
class UObject;
class ULevel;
class UModel;
class Window;
class UTexture;
class UActor;
class UFont;
class UMesh;
class ULodMesh;
class USkeletalMesh;
class ULevelSummary;
class ULevelInfo;
class UZoneInfo;
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

enum EInputKey
{
	IK_LeftMouse,
	IK_MiddleMouse,
	IK_RightMouse,
	IK_MouseWheelUp,
	IK_MouseWheelDown,
	IK_MouseX,
	IK_MouseY
};

enum EInputType
{
	IST_Press,
	IST_Release,
	IST_Axis
};

class Engine
{
public:
	Engine();
	~Engine();

	void Run();

	void WindowClose(Window* window);
	void Key(Window* window, std::string key);
	void InputEvent(Window* window, EInputKey key, EInputType type, int delta = 0);
	void SetPause(bool value);

	ULevelSummary* LevelSummary = nullptr;
	ULevelInfo* LevelInfo = nullptr;
	ULevel* level = nullptr;
	UActor* SkyZoneInfo = nullptr;

	std::unique_ptr<PackageManager> packages;
	std::unique_ptr<Window> window;
	std::unique_ptr<UTRenderer> renderer;
	std::unique_ptr<Collision> collision;
	std::unique_ptr<AudioPlayer> audioplayer;

	void Tick(float timeElapsed);

	float CalcTimeElapsed();

	void LoadMap(const std::string& packageName);

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
