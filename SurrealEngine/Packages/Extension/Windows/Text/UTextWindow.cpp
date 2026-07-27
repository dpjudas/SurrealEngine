
#include "Precomp.h"
#include "UTextWindow.h"
#include "Utils/Logger.h"
#include "Engine.h"
#include "Packages/Engine/UCanvas.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Extension/Windows/UGC.h"

void UTextWindow::AppendText(const std::string& NewText)
{
	Text() += NewText;
	if (!NewText.empty())
		AskParentForReconfigure();
}

void UTextWindow::EnableTextAsAccelerator(std::optional<bool> bEnable)
{
	std::string accelText = bEnable ? Text() : "";
	SetAcceleratorText(accelText.length() > 0 ? accelText : "");
}

std::string UTextWindow::GetText()
{
	return Text();
}

int UTextWindow::GetTextLength()
{
	return (int)Text().size();
}

int UTextWindow::GetTextPart(int startPos, int Count, std::string& OutText)
{
	int start = std::max(startPos, 0);
	int end = std::min(startPos + Count, (int)Text().size());
	OutText = Text().substr(start, end - start);
	return (int)OutText.size();
}

void UTextWindow::ResetLines()
{
	// UNUSED from scripts.
	LogUnimplemented("TextWindow.ResetLines");
}

void UTextWindow::ResetMinWidth()
{
	// UNUSED from scripts.
	LogUnimplemented("TextWindow.ResetMinWidth");
}

void UTextWindow::SetLines(int newMinLines, int newMaxLines)
{
	if (minLines() != newMinLines || MaxLines() != newMaxLines)
	{
		minLines() = newMinLines;
		MaxLines() = newMaxLines;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetMaxLines(int newMaxLines)
{
	if (MaxLines() != newMaxLines)
	{
		MaxLines() = newMaxLines;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetMinLines(int newMinLines)
{
	if (minLines() != newMinLines)
	{
		minLines() = newMinLines;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetMinWidth(float newMinWidth)
{
	if (MinWidth() != newMinWidth)
	{
		MinWidth() = newMinWidth;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetText(const std::string& NewText)
{
	if (Text() != NewText)
	{
		Text() = NewText;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetTextAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	if (HAlign() != newHAlign || VAlign() != newVAlign)
	{
		HAlign() = newHAlign;
		VAlign() = newVAlign;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetTextMargins(float newHMargin, float newVMargin)
{
	if (hMargin() != newHMargin || vMargin() != newVMargin)
	{
		hMargin() = newHMargin;
		vMargin() = newVMargin;
		AskParentForReconfigure();
	}
}

void UTextWindow::SetWordWrap(bool bNewWordWrap)
{
	if (bWordWrap() != bNewWordWrap)
	{
		bWordWrap() = bNewWordWrap;
		AskParentForReconfigure();
	}
}

void UTextWindow::InitWindow()
{
	SetFont(engine->canvas->SmallFont());
	UWindow::InitWindow();
}

void UTextWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	//if (!Text().empty()) // Is this needed?
	{
		float xExtent = 0.0f, yExtent = 0.0f;
		UObject* oldNormalFont = nullptr;
		UObject* oldBoldFont = nullptr;
		engine->dxgc->GetFonts(oldNormalFont, oldBoldFont);
		engine->dxgc->SetFonts(normalFont(), boldFont());
		engine->dxgc->GetTextExtent(bWidthSpecified ? std::max(preferredWidth, MinWidth()) : 100000.0f, xExtent, yExtent, Text());
		engine->dxgc->SetFonts(oldNormalFont, oldBoldFont);

		if (UFont* font = normalFont())
		{
			int lineHeight = font->GetGlyph('X').VSize;
			int minHeight = minLines() * lineHeight;
			int maxHeight = MaxLines() * lineHeight;
			if (maxHeight > 0 && minHeight <= maxHeight)
				yExtent = std::clamp(yExtent, (float)minHeight, (float)maxHeight);
		}

		xExtent = std::max(xExtent, MinWidth());

		float xMargin = hMargin();
		float yMargin = vMargin();
		if (!bWidthSpecified)
			preferredWidth = xExtent + xMargin * 2.0f;
		if (!bHeightSpecified)
			preferredHeight = yExtent + yMargin * 2.0f;
	}

	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UTextWindow::DrawWindow(UGC* gc)
{
	if (normalFont()) // When should text windows draw their text? They are used for buttons, which sometimes draw themselves via UI
	{
		float xMargin = hMargin();
		float yMargin = vMargin();
		float w = Width() - 2.0f * xMargin;
		float h = Height() - 2.0f * yMargin;
		if (w > 0.0f && h > 0.0f)
		{
			gc->SetAlignments(HAlign(), VAlign());
			gc->DrawText(xMargin, yMargin, w, h, Text());
		}
		// DrawDebugBox(gc);
	}
	UWindow::DrawWindow(gc);
}
