
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

	ResetCanvas();
	Device->SetSceneNode(&Canvas.Frame);
	Device->ClearZ();

	DrawWindow(engine->dxRootWindow, 600.0f, 0.0f);

	UFont* font = engine->canvas->SmallFont();
	float curY = 100.0f;
	DrawWindowInfo(font, engine->dxRootWindow, 0, curY);
}

void RenderSubsystem::DrawWindow(UWindow* window, float offsetX, float offsetY)
{
	if (!window->bIsVisible())
		return;

	float x = window->X() * 2.0f + offsetX;
	float y = window->Y() * 2.0f + offsetY;
	float w = window->Width() * 2.0f;
	float h = window->Height() * 2.0f;
	if (h == 0.0f)
		h = 40.0f;
	vec3 scale((float)Canvas.uiscale, (float)Canvas.uiscale, 1.0f);

	// To do: should we reset everything here? Since the unrealscript code never seems to set things back it may be needed
	// Could also be that it needs to be initialized to the values on UWindow
	engine->dxgc->bDrawEnabled() = true;
	engine->dxgc->offsetX = x;
	engine->dxgc->offsetY = y;
	engine->dxgc->scale = scale;

	CallEvent(window, "DrawWindow", { ExpressionValue::ObjectValue(engine->dxgc) });

	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x, y, 1.0f) * scale, vec3(x + w, y, 1.0f) * scale);
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x, y, 1.0f) * scale, vec3(x, y + h, 1.0f) * scale);
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x + w, y, 1.0f) * scale, vec3(x + w, y + h, 1.0f) * scale);
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x, y + h, 1.0f) * scale, vec3(x + w, y + h, 1.0f) * scale);

	for (UWindow* child = window->firstChild(); child; child = child->nextSibling())
	{
		DrawWindow(child, x, y);
	}
}

void RenderSubsystem::DrawWindowInfo(UFont* font, UWindow* window, int depth, float& curY)
{
	std::string text = UObject::GetUClassFullName(window).ToString();
	text += " - ";
	text += typeid(*window).name();

	float x = window->X();
	float y = window->Y();
	float w = window->Width();
	float h = window->Height();
	text += " x = " + std::to_string((int)x);
	text += " y = " + std::to_string((int)y);
	text += " w = " + std::to_string((int)w);
	text += " h = " + std::to_string((int)h);

	if (window->Background())
		text += " background";

	if (auto button = UObject::TryCast<UButtonWindow>(window))
	{
		if (button->curTexture())
			text += " curtexture";
	}

	vec4 color = vec4(window->bIsVisible() ? 1.0f : 0.5f);
	float curX = depth * 20.0f, curXL = 0.0f, curYL = 0.0f;
	DrawText(font, color, 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
	for (UWindow* child = window->firstChild(); child; child = child->nextSibling())
	{
		DrawWindowInfo(font, child, depth + 1, curY);
	}
}
