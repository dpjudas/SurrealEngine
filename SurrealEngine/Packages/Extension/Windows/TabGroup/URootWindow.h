#pragma once

#include "UModalWindow.h"

class URootWindow : public UModalWindow
{
public:
	using UModalWindow::UModalWindow;

	void EnablePositionalSound(std::optional<bool> bEnable);
	void EnableRendering(std::optional<bool> bRender);
	UObject* GenerateSnapshot(std::optional<bool> bFilter);
	bool IsPositionalSoundEnabled();
	bool IsRenderingEnabled();
	void LockMouse(std::optional<bool> bLockMove, std::optional<bool> bLockButton);
	void ResetRenderViewport();
	void SetDefaultEditCursor(std::optional<UObject*> newEditCursor);
	void SetDefaultMovementCursors(std::optional<UObject*> newMovementCursor, std::optional<UObject*> newHorizontalMovementCursor, std::optional<UObject*> newVerticalMovementCursor, std::optional<UObject*> newTopLeftMovementCursor, std::optional<UObject*> newTopRightMovementCursor);
	void SetRawBackground(std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	void SetRawBackgroundSize(float newWidth, float NewHeight);
	void SetRenderViewport(float newX, float newY, float newWidth, float NewHeight);
	void SetSnapshotSize(float newWidth, float NewHeight);
	void ShowCursor(std::optional<bool> bShow);
	void StretchRawBackground(std::optional<bool> bStretch);

	void WindowReady() override;
	void PostDrawWindow(UGC* gc) override;

	bool SetRootFocusWindow(UWindow* NewFocusWindow);
	void SetRootCursorPos(float newMouseX, float newMouseY);
	UWindow* GetCursorFocus(float& relativeX, float& relativeY);
	bool OnWindowMouseMove(const Point& pos);
	bool OnWindowMouseDown(const Point& pos, EInputKey key);
	bool OnWindowMouseDoubleclick(const Point& pos, EInputKey key);
	bool OnWindowMouseUp(const Point& pos, EInputKey key);
	bool OnWindowMouseWheel(const Point& pos, EInputKey key);
	bool OnWindowRawMouseMove(int dx, int dy);
	bool OnWindowKeyChar(std::string chars);
	bool OnWindowKeyDown(EInputKey key);
	bool OnWindowKeyUp(EInputKey key);

	bool IsCursorVisible();
	bool IsModalOpen();

	UTexture*& DefaultMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.DefaultMoveCursor); }
	UWindow*& FocusWindow() { return Value<UWindow*>(PropOffsets_RootWindow.FocusWindow); }
	float& MouseX() { return Value<float>(PropOffsets_RootWindow.MouseX); }
	float& MouseY() { return Value<float>(PropOffsets_RootWindow.MouseY); }
	int& TickCycles() { return Value<int>(PropOffsets_RootWindow.TickCycles); }
	BitfieldBool bClipRender() { return BoolValue(PropOffsets_RootWindow.bClipRender); }
	BitfieldBool bCursorVisible() { return BoolValue(PropOffsets_RootWindow.bCursorVisible); }
	BitfieldBool bMouseButtonLocked() { return BoolValue(PropOffsets_RootWindow.bMouseButtonLocked); }
	BitfieldBool bMouseMoveLocked() { return BoolValue(PropOffsets_RootWindow.bMouseMoveLocked); }
	BitfieldBool bMouseMoved() { return BoolValue(PropOffsets_RootWindow.bMouseMoved); }
	BitfieldBool bPositionalSound() { return BoolValue(PropOffsets_RootWindow.bPositionalSound); }
	BitfieldBool bRender() { return BoolValue(PropOffsets_RootWindow.bRender); }
	BitfieldBool bShowFrames() { return BoolValue(PropOffsets_RootWindow.bShowFrames); }
	BitfieldBool bShowStats() { return BoolValue(PropOffsets_RootWindow.bShowStats); }
	BitfieldBool bStretchRawBackground() { return BoolValue(PropOffsets_RootWindow.bStretchRawBackground); }
	int& clickCount() { return Value<int>(PropOffsets_RootWindow.clickCount); }
	UTexture*& debugTexture() { return Value<UTexture*>(PropOffsets_RootWindow.debugTexture); }
	UTexture*& defaultEditCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultEditCursor); }
	UTexture*& defaultHorizontalMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultHorizontalMoveCursor); }
	UTexture*& defaultTopLeftMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultTopLeftMoveCursor); }
	UTexture*& defaultTopRightMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultTopRightMoveCursor); }
	UTexture*& defaultVerticalMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultVerticalMoveCursor); }
	float& firstButtonMouseX() { return Value<float>(PropOffsets_RootWindow.firstButtonMouseX); }
	float& firstButtonMouseY() { return Value<float>(PropOffsets_RootWindow.firstButtonMouseY); }
	float& frameTimer() { return Value<float>(PropOffsets_RootWindow.frameTimer); }
	UWindow*& grabbedWindow() { return Value<UWindow*>(PropOffsets_RootWindow.grabbedWindow); }
	int& hMultiplier() { return Value<int>(PropOffsets_RootWindow.hMultiplier); }
	int& handleKeyboardRef() { return Value<int>(PropOffsets_RootWindow.handleKeyboardRef); }
	int& handleMouseRef() { return Value<int>(PropOffsets_RootWindow.handleMouseRef); }
	int& initCount() { return Value<int>(PropOffsets_RootWindow.initCount); }
	uint8_t& keyDownMap() { return Value<uint8_t>(PropOffsets_RootWindow.keyDownMap); }
	float& lastButtonPress() { return Value<float>(PropOffsets_RootWindow.lastButtonPress); }
	int& lastButtonType() { return Value<int>(PropOffsets_RootWindow.lastButtonType); }
	UWindow*& lastButtonWindow() { return Value<UWindow*>(PropOffsets_RootWindow.lastButtonWindow); }
	UWindow*& lastMouseWindow() { return Value<UWindow*>(PropOffsets_RootWindow.lastMouseWindow); }
	float& maxMouseDist() { return Value<float>(PropOffsets_RootWindow.maxMouseDist); }
	float& multiClickTimeout() { return Value<float>(PropOffsets_RootWindow.multiClickTimeout); }
	URootWindow*& nextRootWindow() { return Value<URootWindow*>(PropOffsets_RootWindow.nextRootWindow); }
	int& paintCycles() { return Value<int>(PropOffsets_RootWindow.paintCycles); }
	UPlayerPawnExt*& parentPawn() { return Value<UPlayerPawnExt*>(PropOffsets_RootWindow.parentPawn); }
	float& prevMouseX() { return Value<float>(PropOffsets_RootWindow.prevMouseX); }
	float& prevMouseY() { return Value<float>(PropOffsets_RootWindow.prevMouseY); }
	UTexture*& rawBackground() { return Value<UTexture*>(PropOffsets_RootWindow.rawBackground); }
	float& rawBackgroundHeight() { return Value<float>(PropOffsets_RootWindow.rawBackgroundHeight); }
	float& rawBackgroundWidth() { return Value<float>(PropOffsets_RootWindow.rawBackgroundWidth); }
	Color& rawColor() { return Value<Color>(PropOffsets_RootWindow.rawColor); }
	float& renderHeight() { return Value<float>(PropOffsets_RootWindow.renderHeight); }
	float& renderWidth() { return Value<float>(PropOffsets_RootWindow.renderWidth); }
	float& renderX() { return Value<float>(PropOffsets_RootWindow.renderX); }
	float& renderY() { return Value<float>(PropOffsets_RootWindow.renderY); }
	int& rootFrame() { return Value<int>(PropOffsets_RootWindow.rootFrame); }
	int& snapshotHeight() { return Value<int>(PropOffsets_RootWindow.snapshotHeight); }
	int& snapshotWidth() { return Value<int>(PropOffsets_RootWindow.snapshotWidth); }
	int& vMultiplier() { return Value<int>(PropOffsets_RootWindow.vMultiplier); }

	bool RenderViewportSet = false;
};
