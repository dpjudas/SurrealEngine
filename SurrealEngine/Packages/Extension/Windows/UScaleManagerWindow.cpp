
#include "Precomp.h"
#include "UScaleManagerWindow.h"
#include "UScaleWindow.h"
#include "Text/UButtonWindow.h"

void UScaleManagerWindow::SetManagerAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	childHAlign() = newHAlign;
	childVAlign() = newVAlign;
}

void UScaleManagerWindow::SetManagerMargins(std::optional<float> newMarginWidth, std::optional<float> newMarginHeight)
{
	if (newMarginWidth)
		marginWidth() = *newMarginWidth;
	if (newMarginHeight)
		marginHeight() = *newMarginHeight;
}

void UScaleManagerWindow::SetManagerOrientation(uint8_t newOrientation)
{
	orientation() = newOrientation;
}

void UScaleManagerWindow::SetMarginSpacing(std::optional<float> newSpacing)
{
	if (newSpacing)
		Spacing() = *newSpacing;
}

void UScaleManagerWindow::SetScale(UObject* NewScale)
{
	Scale() = UObject::Cast<UScaleWindow>(NewScale);
}

void UScaleManagerWindow::SetScaleButtons(UObject* newDecButton, UObject* newIncButton)
{
	if (newDecButton)
		decButton() = UObject::Cast<UButtonWindow>(newDecButton);
	if (newIncButton)
		incButton() = UObject::Cast<UButtonWindow>(newIncButton);
}

void UScaleManagerWindow::SetValueField(UObject* newValueField)
{
	valueField() = UObject::Cast<UTextWindow>(newValueField);
}

void UScaleManagerWindow::StretchScaleField(std::optional<bool> bNewStretch)
{
	bStretchScaleField() = !bNewStretch || *bNewStretch;
}

void UScaleManagerWindow::StretchValueField(std::optional<bool> bNewStretch)
{
	bStretchValueField() = !bNewStretch || *bNewStretch;
}

void UScaleManagerWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	if (auto scale = Scale())
	{
		if (!bWidthSpecified && !bHeightSpecified)
			scale->QueryPreferredSize(preferredWidth, preferredHeight);
		else if (bHeightSpecified)
			scale->QueryPreferredWidth(preferredHeight);
		else if (bWidthSpecified)
			scale->QueryPreferredHeight(preferredWidth);
	}
	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UScaleManagerWindow::ConfigurationChanged()
{
	if (auto scale = Scale())
	{
		scale->SetConfiguration(0.0f, 0.0f, Width(), Height());
	}

	UWindow::ConfigurationChanged();
}
