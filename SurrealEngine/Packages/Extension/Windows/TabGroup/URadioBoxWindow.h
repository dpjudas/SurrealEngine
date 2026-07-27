#pragma once

#include "UTabGroupWindow.h"

class UToggleWindow;

class URadioBoxWindow : public UTabGroupWindow
{
public:
	using UTabGroupWindow::UTabGroupWindow;

	UObject* GetEnabledToggle();

	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;
	void ConfigurationChanged() override;

	BitfieldBool bOneCheck() { return BoolValue(PropOffsets_RadioBoxWindow.bOneCheck); }
	UToggleWindow*& currentSelection() { return Value<UToggleWindow*>(PropOffsets_RadioBoxWindow.currentSelection); }
	//DynamicArray& toggleButtons() { return Value<DynamicArray>(PropOffsets_RadioBoxWindow.toggleButtons); }
};
