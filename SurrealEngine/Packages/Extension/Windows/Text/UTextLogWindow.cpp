
#include "Precomp.h"
#include "UTextLogWindow.h"
#include "Packages/Extension/Windows/UGC.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "Engine.h"

void UTextLogWindow::AddLog(const std::string& NewText, const Color& linecol)
{
	LogEntry entry;
	entry.text = NewText;
	entry.color = linecol;
	entry.timestamp = totalTime + textTimeout();
	logs.push_back(entry);
}

void UTextLogWindow::ClearLog()
{
	logs.clear();
}

void UTextLogWindow::PauseLog(bool bNewPauseState)
{
	bPaused() = bNewPauseState;
}

void UTextLogWindow::SetTextTimeout(float newTimeout)
{
	textTimeout() = newTimeout;
}

void UTextLogWindow::Tick(float timeElapsed)
{
	UTextWindow::Tick(timeElapsed);
	if (!bPaused())
	{
		totalTime += timeElapsed;

		while (!logs.empty() && logs.front().timestamp < totalTime)
			logs.erase(logs.begin());
	}
}

void UTextLogWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	float xExtent = 0.0f, yExtent = 0.0f;
	UObject* oldNormalFont = nullptr;
	UObject* oldBoldFont = nullptr;
	engine->dxgc->GetFonts(oldNormalFont, oldBoldFont);
	engine->dxgc->SetFonts(normalFont(), boldFont());

	for (const LogEntry& line : logs)
	{
		float xLineExtent = 0.0f, yLineExtent = 0.0f;
		engine->dxgc->GetTextExtent(bWidthSpecified ? std::max(preferredWidth, MinWidth()) : 100000.0f, xLineExtent, yLineExtent, line.text);
		xExtent = std::max(xExtent, xLineExtent);
		yExtent += yLineExtent;
	}

	engine->dxgc->SetFonts(oldNormalFont, oldBoldFont);

	if (UFont* font = normalFont())
	{
		int lineHeight = font->GetGlyph('X').VSize;
		int minHeight = minLines() * lineHeight;
		int maxHeight = MaxLines() * lineHeight;
		if (maxHeight > 0 && minHeight <= maxHeight)
			yExtent = std::clamp(yExtent, (float)minHeight, (float)maxHeight);
	}

	float xMargin = hMargin();
	float yMargin = vMargin();
	if (!bWidthSpecified)
		preferredWidth = xExtent + xMargin * 2.0f;
	if (!bHeightSpecified)
		preferredHeight = yExtent + yMargin * 2.0f;

	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UTextLogWindow::DrawWindow(UGC* gc)
{
	UWindow::DrawWindow(gc);

	float xMargin = hMargin();
	float yMargin = vMargin();
	float x = xMargin;
	float y = yMargin;
	float w = Width() - 2.0f * xMargin;
	float h = Height() - 2.0f * yMargin;

	gc->SetAlignments(HAlign(), VAlign());
	gc->EnableWordWrap(true);

	for (const LogEntry& line : logs)
	{
		float xExtent = 0.0f, yExtent = 0.0f;
		gc->GetTextExtent(Width(), xExtent, yExtent, line.text);
		gc->SetTextColor(line.color);
		gc->DrawText(x, y, w, h - y, line.text);
		y += yExtent;
	}
}
