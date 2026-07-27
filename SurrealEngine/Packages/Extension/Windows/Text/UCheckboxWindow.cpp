
#include "Precomp.h"
#include "UCheckboxWindow.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

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
