
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
	float curY = 100.0f;
	DrawWindowInfo(font, engine->dxRootWindow, 0, curY);
}

void RenderSubsystem::DrawWindowInfo(UFont* font, UWindow* window, int depth, float& curY)
{
	float curX = depth * 20.0f, curXL = 0.0f, curYL = 0.0f;
	DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, UObject::GetUClassFullName(window).ToString(), PF_NoSmooth | PF_Masked, false);
	for (UWindow* child : window->Children)
	{
		DrawWindowInfo(font, child, depth + 1, curY);
	}
}
