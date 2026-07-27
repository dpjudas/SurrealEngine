#pragma once

#include "UToggleWindow.h"

class UCheckboxWindow : public UToggleWindow
{
public:
	using UToggleWindow::UToggleWindow;

	void SetCheckboxColor(const Color& NewColor);
	void SetCheckboxSpacing(float newSpacing);
	void SetCheckboxStyle(uint8_t NewStyle);
	void SetCheckboxTextures(std::optional<UObject*> toggleOff, std::optional<UObject*> toggleOn, std::optional<float> textureWidth, std::optional<float> textureHeight);
	void ShowCheckboxOnRightSide(std::optional<bool> bRight);

	BitfieldBool bRightSide() { return BoolValue(PropOffsets_CheckboxWindow.bRightSide); }
	Color& checkboxColor() { return Value<Color>(PropOffsets_CheckboxWindow.checkboxColor); }
	float& checkboxSpacing() { return Value<float>(PropOffsets_CheckboxWindow.checkboxSpacing); }
	uint8_t& checkboxStyle() { return Value<uint8_t>(PropOffsets_CheckboxWindow.checkboxStyle); }
	float& textureHeight() { return Value<float>(PropOffsets_CheckboxWindow.textureHeight); }
	float& textureWidth() { return Value<float>(PropOffsets_CheckboxWindow.textureWidth); }
	UTexture*& toggleOff() { return Value<UTexture*>(PropOffsets_CheckboxWindow.toggleOff); }
	UTexture*& toggleOn() { return Value<UTexture*>(PropOffsets_CheckboxWindow.toggleOn); }
};
