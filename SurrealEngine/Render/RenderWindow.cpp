
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "UObject/UWindow.h"

void RenderSubsystem::PreRenderWindows(UCanvas* canvas)
{
	if (!engine->dxRootWindow)
		return;

	engine->dxRootWindow->UpdateLayout();
}

void RenderSubsystem::PostRenderWindows(UCanvas* canvas)
{
	if (!engine->dxRootWindow)
		return;

	Device->SetSceneNode(&Canvas.Frame);
	Device->ClearZ();

	DrawWindow(engine->dxRootWindow, 0.0f, 0.0f);

	/*
	float curY = 100.0f;
	DrawWindowInfo(engine->canvas->SmallFont(), engine->dxRootWindow, 0, curY);
	*/
}

void RenderSubsystem::ResetWindowGC(UWindow* window, float offsetX, float offsetY)
{
	// Reset everything first
	engine->dxgc->HAlign() = (uint8_t)EHAlign::Left;
	engine->dxgc->PolyFlags() = 0;
	engine->dxgc->Style() = 0;
	engine->dxgc->TextColor() = { 255, 255, 255, 255 };
	engine->dxgc->VAlign() = (uint8_t)EVAlign::Top;
	engine->dxgc->bDrawEnabled() = true;
	engine->dxgc->bFree() = false;
	engine->dxgc->bMasked() = false;
	engine->dxgc->bModulated() = false;
	engine->dxgc->bParseMetachars() = false;
	engine->dxgc->bSmoothed() = false;
	engine->dxgc->bTextTranslucent() = false;
	engine->dxgc->bTranslucent() = false;
	engine->dxgc->bWordWrap() = true;
	engine->dxgc->baselineOffset() = 0.0f;
	engine->dxgc->boldFont() = nullptr;
	engine->dxgc->gcClipRect() = {};
	engine->dxgc->gcCount() = 0;
	engine->dxgc->gcFree() = nullptr;
	engine->dxgc->gcOwner() = nullptr;
	engine->dxgc->gcStack() = nullptr;
	engine->dxgc->hMultiplier() = 0;
	engine->dxgc->normalFont() = nullptr;
	//engine->dxgc->textPlane() = {}
	engine->dxgc->textPolyFlags() = 0;
	engine->dxgc->textVSpacing() = 0.0f;
	engine->dxgc->tileColor() = { 255, 255, 255, 255 };
	//engine->dxgc->tilePlane() = {};
	engine->dxgc->underlineHeight() = 0.0f;
	engine->dxgc->underlineTexture() = nullptr;
	engine->dxgc->vMultiplier() = 0;
	engine->dxgc->SpecialTextEnabled = false;

	// Copy state from window
	engine->dxgc->EnableSpecialText(window->bSpecialText());
	engine->dxgc->EnableTranslucentText(window->bTextTranslucent());
	engine->dxgc->SetBaselineData(window->BaselineOffset, window->UnderlineHeight);
	engine->dxgc->SetFonts(window->normalFont(), window->boldFont());
	engine->dxgc->SetTextColor(window->TextColor());
	engine->dxgc->SetTextVSpacing(window->textVSpacing());
	engine->dxgc->SetTileColor(window->tileColor());

	// Setup origin and clipping for the window
	engine->dxgc->offsetX = offsetX;
	engine->dxgc->offsetY = offsetY;
	engine->dxgc->clipBox = engine->dxgc->ScaleRect(Rectf::xywh(offsetX, offsetY, window->Width(), window->Height()));
}

void RenderSubsystem::DrawWindow(UWindow* window, float offsetX, float offsetY)
{
	if (!window->bIsVisible())
		return;

	offsetX += window->UsedX;
	offsetY += window->UsedY;

	ResetWindowGC(window, offsetX, offsetY);
	window->DrawWindow(engine->dxgc);

	for (UWindow* child = window->firstChild(); child; child = child->nextSibling())
	{
		DrawWindow(child, offsetX, offsetY);
	}

	ResetWindowGC(window, offsetX, offsetY);
	window->PostDrawWindow(engine->dxgc);
}

void RenderSubsystem::DrawWindowInfo(UFont* font, UWindow* window, int depth, float& curY)
{
	if (!window->bIsVisible())
		return;

	std::string text = UObject::GetUClassFullName(window).ToString();
	text += " - ";
	text += typeid(*window).name();

	float x = window->UsedX;
	float y = window->UsedY;
	float w = window->Width();
	float h = window->Height();
	text += " x = " + std::to_string((int)x);
	text += " y = " + std::to_string((int)y);
	text += " w = " + std::to_string((int)w);
	text += " h = " + std::to_string((int)h);

	vec4 color = vec4(window->bConfigured() ? 1.0f : 0.5f);
	if (window->bConfigured() && (window->Width() <= 0.5f || window->Height() <= 0.5f))
		color = vec4(1.0f, 0.2f, 0.2f, 1.0f);
	if (window == engine->dxRootWindow->FocusWindow())
		color = vec4(0.5f, 1.0f, 0.5f, 1.0f);
	if (window == engine->dxRootWindow->lastMouseWindow())
		color = vec4(0.5f, 0.5f, 1.0f, 1.0f);
	float curX = depth * 20.0f, curXL = 0.0f, curYL = 0.0f;
	DrawText(font, color, 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
	for (UWindow* child = window->lastChild(); child; child = child->prevSibling())
	{
		DrawWindowInfo(font, child, depth + 1, curY);
	}
}
