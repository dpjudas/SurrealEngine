
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

	// Rescale UI with the assumption it was originally designed for 800x600
	float virtualHeight = 600.0f;
	float virtualScale = engine->ViewportHeight != 0 ? engine->ViewportHeight / virtualHeight : 1.0f;
	float virtualWidth = engine->ViewportWidth / virtualScale;

	if (engine->dxRootWindow->Width() != virtualWidth || engine->dxRootWindow->Height() != virtualHeight)
	{
		engine->dxRootWindow->ConfigureChild(0.0f, 0.0f, virtualWidth, virtualHeight);
	}

	for (UWindow* child = engine->dxRootWindow->firstChild(); child; child = child->nextSibling())
	{
		if (child->FirstDraw && child->bIsVisible())
		{
			auto halign = (EHAlign)child->winHAlign();
			auto valign = (EVAlign)child->winVAlign();

			float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;
			child->QueryPreferredSize(width, height);

			if (halign == EHAlign::Left)
				x = 0.0f;
			else if (halign == EHAlign::Center)
				x = (640.0f - width) * 0.5f;
			else if (halign == EHAlign::Right)
				x = 640.0f - width;
			else if (halign == EHAlign::Full)
				width = 640.0f;

			if (valign == EVAlign::Top)
				y = 0.0f;
			else if (valign == EVAlign::Center)
				y = (480.0f - height) * 0.5f;
			else if (valign == EVAlign::Bottom)
				y = 480.0f - height;
			else if (valign == EVAlign::Full)
				height = 480.0f;

			child->ConfigureChild(x, y, width, height);
		}
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

	if (window->FirstDraw)
	{
		window->FirstDraw = false;
		CallEvent(window, "WindowReady");
	}

	offsetX += window->X();
	offsetY += window->Y();

	ResetWindowGC(window, offsetX, offsetY);
	CallEvent(window, "DrawWindow", { ExpressionValue::ObjectValue(engine->dxgc) });

#if 0
	float x0 = window->X() + offsetX;
	float y0 = window->Y() + offsetY;
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
	CallEvent(window, "PostDrawWindow", { ExpressionValue::ObjectValue(engine->dxgc) });
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
