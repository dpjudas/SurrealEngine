#pragma once

#include "UTabGroupWindow.h"

class UModalWindow : public UTabGroupWindow
{
public:
	using UTabGroupWindow::UTabGroupWindow;

	bool IsCurrentModal();
	void SetMouseFocusMode(uint8_t newFocusMode);

	UTexture* GetCursor() override { return defaultCursor(); }
	Color GetCursorColor() override { return defaultCursorColor(); }
	UTexture* GetCursorShadow() override { return defaultCursorShadow(); }
	float GetCursorHotX() override { return defaultHotX(); }
	float GetCursorHotY() override { return defaultHotY(); }

	UWindow*& acceleratorTable() { return Value<UWindow*>(PropOffsets_ModalWindow.acceleratorTable); }
	BitfieldBool bDirtyAccelerators() { return BoolValue(PropOffsets_ModalWindow.bDirtyAccelerators); }
	uint8_t& focusMode() { return Value<uint8_t>(PropOffsets_ModalWindow.focusMode); }
	UWindow*& preferredFocus() { return Value<UWindow*>(PropOffsets_ModalWindow.preferredFocus); }
	//DynamicArray& tabGroupWindowList() { return Value<DynamicArray>(PropOffsets_ModalWindow.tabGroupWindowList); }
};
