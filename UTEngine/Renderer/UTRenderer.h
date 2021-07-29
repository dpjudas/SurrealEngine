#pragma once

#include "BrushRender.h"
#include "CanvasRender.h"
#include "CoronaRender.h"
#include "LightRender.h"
#include "MeshRender.h"
#include "SceneRender.h"
#include "SpriteRender.h"
#include <set>

class UActor;
class UTexture;

class UTRenderer
{
public:
	UTRenderer();
	~UTRenderer();

	void OnMapLoaded();

	std::vector<UActor*> Lights;
	std::set<UTexture*> Textures;

	uint64_t startFPSTime = 0;
	int framesDrawn = 0;
	int fps = 0;
	int sceneDrawNumber = 1;
	bool showTimedemoStats = false;

	float AutoUV = 0.0f;

	int uiscale = 3;

	BrushRender brush;
	CanvasRender canvas;
	CoronaRender corona;
	LightRender light;
	MeshRender mesh;
	SceneRender scene;
	SpriteRender sprite;
};
