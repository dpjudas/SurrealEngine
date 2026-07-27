#pragma once

#include "UButtonWindow.h"

class UToggleWindow : public UButtonWindow
{
public:
	using UButtonWindow::UButtonWindow;

	void ChangeToggle();
	bool GetToggle();
	void SetToggle(bool bNewToggle);
	void SetToggleSounds(std::optional<UObject*> enableSound, std::optional<UObject*> disableSound);

	USound*& disableSound() { return Value<USound*>(PropOffsets_ToggleWindow.disableSound); }
	USound*& enableSound() { return Value<USound*>(PropOffsets_ToggleWindow.enableSound); }

	bool isToggled = false;
};
