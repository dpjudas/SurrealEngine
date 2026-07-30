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

	void InitWindow() override;
	void DrawWindow(UGC* gc) override;
	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;
	bool MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks) override;
	bool MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks) override;

	BitfieldBool bRightSide() { return BoolValue(PropOffsets_CheckboxWindow.bRightSide); }
	Color& checkboxColor() { return Value<Color>(PropOffsets_CheckboxWindow.checkboxColor); }
	float& checkboxSpacing() { return Value<float>(PropOffsets_CheckboxWindow.checkboxSpacing); }
	uint8_t& checkboxStyle() { return Value<uint8_t>(PropOffsets_CheckboxWindow.checkboxStyle); }
	float& textureHeight() { return Value<float>(PropOffsets_CheckboxWindow.textureHeight); }
	float& textureWidth() { return Value<float>(PropOffsets_CheckboxWindow.textureWidth); }
	UTexture*& toggleOff() { return Value<UTexture*>(PropOffsets_CheckboxWindow.toggleOff); }
	UTexture*& toggleOn() { return Value<UTexture*>(PropOffsets_CheckboxWindow.toggleOn); }
};
