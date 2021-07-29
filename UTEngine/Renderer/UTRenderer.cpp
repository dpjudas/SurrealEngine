
#include "Precomp.h"
#include "UTRenderer.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"
#include "Engine.h"

UTRenderer::UTRenderer()
{
}

UTRenderer::~UTRenderer()
{
}

void UTRenderer::OnMapLoaded()
{
	// Cache some light and texture info
	std::set<UActor*> lightset;
	for (UActor* light : engine->Level->Model->Lights)
		lightset.insert(light);
	engine->renderer->Lights.clear();
	for (UActor* light : lightset)
		engine->renderer->Lights.push_back(light);
	engine->renderer->Textures.clear();
	for (BspSurface& surf : engine->Level->Model->Surfaces)
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
}
