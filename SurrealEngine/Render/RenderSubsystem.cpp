
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
	AutoUV += levelTimeElapsed * 64.0f;

	for (UTexture* tex : Textures)
		tex->Update(levelTimeElapsed);

	Light.FogFrameCounter++;

	vec3 flashScale = 0.5f;
	vec3 flashFog = vec3(1.0f, 0.0f, 0.0f);

	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(engine->CameraActor);
	if (player)
	{
		flashScale = player->FlashScale();
		flashFog = player->FlashFog();
	}

	Device->Lock(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), vec4(0.0f));

	ResetCanvas();
	PreRender();

	if (engine->console->bNoDrawWorld() == false)
	{
		DrawScene();
		RenderOverlays();
	}

	PostRender();
	Device->EndFlash();

	Device->Unlock(true);
}

void RenderSubsystem::DrawEditorViewport()
{
	DrawScene();
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

	Textures.clear();
	for (BspSurface& surf : engine->Level->Model->Surfaces)
	{
		if (surf.Material)
		{
			Textures.insert(surf.Material);
			if (surf.Material->DetailTexture())
				Textures.insert(surf.Material->DetailTexture());
			if (surf.Material->MacroTexture())
				Textures.insert(surf.Material->MacroTexture());
		}
	}
}
