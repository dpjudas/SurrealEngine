
#include "Precomp.h"
#include "UCheckboxWindow.h"
#include "Packages/Extension/Windows/UGC.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "Engine.h"

void UCheckboxWindow::SetCheckboxColor(const Color& NewColor)
{
	checkboxColor() = NewColor;
}

void UCheckboxWindow::SetCheckboxSpacing(float newSpacing)
{
	checkboxSpacing() = newSpacing;
}

void UCheckboxWindow::SetCheckboxStyle(uint8_t NewStyle)
{
	checkboxStyle() = NewStyle;
}

void UCheckboxWindow::SetCheckboxTextures(std::optional<UObject*> newToggleOff, std::optional<UObject*> newToggleOn, std::optional<float> newTextureWidth, std::optional<float> newTextureHeight)
{
	if (newToggleOff)
		toggleOff() = UObject::Cast<UTexture>(*newToggleOff);
	if (newToggleOn)
		toggleOn() = UObject::Cast<UTexture>(*newToggleOn);
	if (newTextureWidth)
		textureWidth() = *newTextureWidth;
	if (newTextureHeight)
		textureHeight() = *newTextureHeight;
}

void UCheckboxWindow::ShowCheckboxOnRightSide(std::optional<bool> bRight)
{
	bRightSide() = !bRight || *bRight;
}

void UCheckboxWindow::InitWindow()
{
	UToggleWindow::InitWindow();
}

void UCheckboxWindow::DrawWindow(UGC* gc)
{
	UWindow::DrawWindow(gc);

	float xMargin = hMargin();
	float yMargin = vMargin();
	float x = xMargin;
	float y = yMargin;
	float w = Width() - 2.0f * xMargin;
	float h = Height() - 2.0f * yMargin;
	if (w <= 0.0f || h <= 0.0f)
		return;

	UTexture* tex = GetToggle() ? toggleOn() : toggleOff();
	if (!tex || !normalFont())
		return;

	if (!bRightSide())
	{
		// To do: figure out how checkboxSpacing should be applied

		x += (float)tex->USize();// + checkboxSpacing();
		if (w - x > 0.0f)
		{
			gc->SetAlignments(HAlign(), VAlign());
			gc->DrawText(x, y, w - x, h, Text());
		}
		gc->SetStyle((EDrawStyle)checkboxStyle());
		gc->SetTileColor(checkboxColor());
		gc->DrawIcon(xMargin, y + 1.0f /* + (h - (float)tex->VSize()) * 0.5f*/, tex);
	}
	else // Note: this is never used as ShowCheckboxOnRightSide is never called
	{
		float xExtent = 0.0f, yExtent = 0.0f;
		gc->GetTextExtent(100000.0f, xExtent, yExtent, Text());
		gc->SetAlignments(HAlign(), VAlign());
		gc->DrawText(x, y, w, h, Text());
		gc->SetStyle((EDrawStyle)checkboxStyle());
		gc->SetTileColor(checkboxColor());
		gc->DrawIcon(x + xExtent + checkboxSpacing(), y + 1.0f/* + (h - (float)tex->VSize()) * 0.5f*/, tex);
	}

	// DrawDebugBox(gc);
}

void UCheckboxWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
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

	if (auto tex = GetToggle() ? toggleOn() : toggleOff())
	{
		xExtent += (float)tex->USize() + checkboxSpacing();
		yExtent = std::max(yExtent, (float)tex->VSize());
	}

	float xMargin = hMargin();
	float yMargin = vMargin();
	if (!bWidthSpecified)
		preferredWidth = xExtent + xMargin * 2.0f;
	if (!bHeightSpecified)
		preferredHeight = yExtent + yMargin * 2.0f;

	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

bool UCheckboxWindow::MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks)
{
	// Should we capture the mouse here?
	SetFocusWindow(this);
	return true;
}

bool UCheckboxWindow::MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks)
{
	SetToggle(!GetToggle());
	for (UWindow* cur = this; cur != nullptr; cur = cur->parentOwner())
	{
		if (ToggleChanged(this, GetToggle()))
			break;
	}
	return true;
}
