#pragma once

#include "UWindow.h"

class UButtonWindow;
class UTextWindow;

class UScaleManagerWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void SetManagerAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetManagerMargins(std::optional<float> newMarginWidth, std::optional<float> newMarginHeight);
	void SetManagerOrientation(uint8_t newOrientation);
	void SetMarginSpacing(std::optional<float> newSpacing);
	void SetScale(UObject* NewScale);
	void SetScaleButtons(UObject* newDecButton, UObject* newIncButton);
	void SetValueField(UObject* newValueField);
	void StretchScaleField(std::optional<bool> bNewStretch);
	void StretchValueField(std::optional<bool> bNewStretch);

	UScaleWindow*& Scale() { return Value<UScaleWindow*>(PropOffsets_ScaleManagerWindow.Scale); }
	float& Spacing() { return Value<float>(PropOffsets_ScaleManagerWindow.Spacing); }
	BitfieldBool bStretchScaleField() { return BoolValue(PropOffsets_ScaleManagerWindow.bStretchScaleField); }
	BitfieldBool bStretchValueField() { return BoolValue(PropOffsets_ScaleManagerWindow.bStretchValueField); }
	uint8_t& childHAlign() { return Value<uint8_t>(PropOffsets_ScaleManagerWindow.childHAlign); }
	uint8_t& childVAlign() { return Value<uint8_t>(PropOffsets_ScaleManagerWindow.childVAlign); }
	UButtonWindow*& decButton() { return Value<UButtonWindow*>(PropOffsets_ScaleManagerWindow.decButton); }
	UButtonWindow*& incButton() { return Value<UButtonWindow*>(PropOffsets_ScaleManagerWindow.incButton); }
	float& marginHeight() { return Value<float>(PropOffsets_ScaleManagerWindow.marginHeight); }
	float& marginWidth() { return Value<float>(PropOffsets_ScaleManagerWindow.marginWidth); }
	uint8_t& orientation() { return Value<uint8_t>(PropOffsets_ScaleManagerWindow.orientation); }
	UTextWindow*& valueField() { return Value<UTextWindow*>(PropOffsets_ScaleManagerWindow.valueField); }
};
