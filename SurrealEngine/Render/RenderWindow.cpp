
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "UObject/UWindow.h"

void RenderSubsystem::DrawRootWindow()
{
	if (!engine->dxRootWindow)
		return;

	Device->SetSceneNode(&Canvas.Frame);

	UFont* font = engine->canvas->SmallFont();

	float curX = 0.0f, curY = 100.0f, curXL = 0.0f, curYL = 0.0f;
	DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, "DXRootWindow", PF_NoSmooth | PF_Masked, false);
	curX = 0.0f;
	DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, "DXRootWindow2", PF_NoSmooth | PF_Masked, false);
}
