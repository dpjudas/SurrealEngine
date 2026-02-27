
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "UObject/UWindow.h"

void RenderSubsystem::PreRenderWindows(UCanvas* canvas)
{
	// What needs to be done here?
}

void RenderSubsystem::PostRenderWindows(UCanvas* canvas)
{
	if (!engine->dxRootWindow)
		return;

	Device->SetSceneNode(&Canvas.Frame);
	Device->ClearZ();

	// Rescale UI with the assumption it was originally designed for 800x600
	float virtualHeight = 600.0f;
	float virtualScale = engine->ViewportHeight != 0 ? engine->ViewportHeight / virtualHeight : 1.0f;
	float virtualWidth = engine->ViewportWidth / virtualScale;

	if (engine->dxRootWindow->Width() != virtualWidth || engine->dxRootWindow->Height() != virtualHeight)
	{
		engine->dxRootWindow->ConfigureChild(0.0f, 0.0f, virtualWidth, virtualHeight);
	}

	DrawWindow(engine->dxRootWindow, 0.0f, 0.0f);

	UFont* font = engine->canvas->SmallFont();
	float curY = 100.0f;
	DrawWindowInfo(font, engine->dxRootWindow, 0, curY);
}

void RenderSubsystem::ResetWindowGC(UWindow* window, float offsetX, float offsetY)
{
	engine->dxgc->EnableDrawing(true);
	engine->dxgc->EnableMasking(false);
	engine->dxgc->EnableModulation(false);
	engine->dxgc->EnableSmoothing(window->bSmoothBackground());
	engine->dxgc->EnableSpecialText(window->bSpecialText());
	engine->dxgc->EnableTranslucency(true);
	engine->dxgc->EnableTranslucentText(window->bTextTranslucent());
	engine->dxgc->EnableWordWrap(true);
	engine->dxgc->SetBaselineData(&window->BaselineOffset, &window->UnderlineHeight);
	engine->dxgc->SetFonts(window->normalFont(), window->boldFont());
	engine->dxgc->SetHorizontalAlignment((uint8_t)EHAlign::Left);
	engine->dxgc->SetStyle(window->backgroundStyle());
	engine->dxgc->SetTextColor(window->TextColor());
	engine->dxgc->SetTextVSpacing(window->textVSpacing());
	engine->dxgc->SetTileColor(window->tileColor());
	engine->dxgc->SetVerticalAlignment((uint8_t)EVAlign::Top);
	engine->dxgc->offsetX = offsetX;
	engine->dxgc->offsetY = offsetY;
}

void RenderSubsystem::DrawWindow(UWindow* window, float offsetX, float offsetY)
{
	if (!window->bIsVisible())
		return;

	if (!window->bConfigured())
		window->AskParentForReconfigure();

	UWindow* parent = window->parentOwner();
	if (parent)
	{
		EHAlign halign = (EHAlign)window->winHAlign();
		EVAlign valign = (EVAlign)window->winVAlign();
		float leftMargin = window->hMargin0();
		float rightMargin = window->hMargin1();
		float topMargin = window->vMargin0();
		float bottomMargin = window->vMargin1();

		float pWidth = parent->Width();
		float pHeight = parent->Height();
		float width = window->Width();
		float height = window->Height();

		float x = 0.0f, y = 0.0f;
		if (halign == EHAlign::Left)
			x = window->X();
		else if (halign == EHAlign::Center)
			x = (pWidth - width) * 0.5f + window->X();
		else if (halign == EHAlign::Right)
			x = pWidth - width - window->X();

		if (valign == EVAlign::Top)
			y = window->Y();
		else if (valign == EVAlign::Center)
			y = (pHeight - height) * 0.5f + window->Y();
		else if (valign == EVAlign::Bottom)
			y = pHeight - height - window->Y();

		offsetX += x;
		offsetY += y;
	}

	if (window->FirstDraw)
	{
		window->FirstDraw = false;
		window->WindowReady();
	}

	ResetWindowGC(window, offsetX, offsetY);
	window->DrawWindow(engine->dxgc);

#if 0
	float x0 = offsetX;
	float y0 = offsetY;
	float x1 = x0 + window->Width();
	float y1 = y0 + window->Height();
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x0 * Canvas.uiscale, y0 * Canvas.uiscale, 1.0f), vec3(x1 * Canvas.uiscale, y0 * Canvas.uiscale, 1.0f));
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x0 * Canvas.uiscale, y0 * Canvas.uiscale, 1.0f), vec3(x0 * Canvas.uiscale, y1 * Canvas.uiscale, 1.0f));
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x1 * Canvas.uiscale, y0 * Canvas.uiscale, 1.0f), vec3(x1 * Canvas.uiscale, y1 * Canvas.uiscale, 1.0f));
	Device->Draw2DLine(&Canvas.Frame, vec4(1.0f), 0, vec3(x0 * Canvas.uiscale, y1 * Canvas.uiscale, 1.0f), vec3(x1 * Canvas.uiscale, y1 * Canvas.uiscale, 1.0f));
#endif

	for (UWindow* child = window->firstChild(); child; child = child->nextSibling())
	{
		DrawWindow(child, offsetX, offsetY);
	}

	ResetWindowGC(window, offsetX, offsetY);
	window->PostDrawWindow(engine->dxgc);
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

	vec4 color = vec4(window->bIsVisible() ? 1.0f : 0.5f);
	if (window->bIsVisible() && (window->Width() <= 0.5f || window->Height() <= 0.5f))
		color = vec4(1.0f, 0.2f, 0.2f, 1.0f);
	float curX = depth * 20.0f, curXL = 0.0f, curYL = 0.0f;
	DrawText(font, color, 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
	if (window->bIsVisible())
	{
		for (UWindow* child = window->firstChild(); child; child = child->nextSibling())
		{
			DrawWindowInfo(font, child, depth + 1, curY);
		}
	}
}
