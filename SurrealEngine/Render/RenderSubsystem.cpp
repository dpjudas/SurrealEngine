
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

RenderSubsystem::RenderSubsystem(RenderDevice* renderdevice) : Device(renderdevice)
{
}

void RenderSubsystem::DrawGame(float levelTimeElapsed)
{
	FrameCounter++;
	LevelTimeElapsed = levelTimeElapsed;
	AutoUV += levelTimeElapsed * 64.0f;

	Device->Brightness = engine->Brightness;
	Device->Lock(0.5f, vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f));

	ResetCanvas();
	PreRender();

	if (engine->console->bNoDrawWorld() == false)
	{
		DrawScene();
		RenderOverlays();
	}

	PostRender();

	Device->Unlock(true);
}

void RenderSubsystem::DrawEditorViewport()
{
	Device->Brightness = engine->Brightness;
	DrawScene();
}

void RenderSubsystem::UpdateTexture(UTexture* tex)
{
	if (tex && tex->FrameCounter != FrameCounter)
	{
		tex->Update(LevelTimeElapsed);
		tex->FrameCounter = FrameCounter;
	}
}

void RenderSubsystem::OnMapLoaded()
{
	Device->Flush(true);

	Light.Lights.clear();
	Light.lmtextures.clear();
	Light.fogtextures.clear();

	std::set<UActor*> lightset;
	for (UActor* light : engine->Level->Model->Lights)
		lightset.insert(light);
	for (UActor* light : lightset)
		Light.Lights.push_back(light);
}
