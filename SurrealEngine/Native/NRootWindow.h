#pragma once

#include "UObject/UObject.h"

class NRootWindow
{
public:
	static void RegisterFunctions();

	static void EnablePositionalSound(UObject* Self, std::optional<bool> bEnable);
	static void EnableRendering(UObject* Self, std::optional<bool> bRender);
	static void GenerateSnapshot(UObject* Self, std::optional<bool> bFilter, UObject*& ReturnValue);
	static void IsPositionalSoundEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsRenderingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void LockMouse(UObject* Self, std::optional<bool> bLockMove, std::optional<bool> bLockButton);
	static void ResetRenderViewport(UObject* Self);
	static void SetDefaultEditCursor(UObject* Self, std::optional<UObject*> newEditCursor);
	static void SetDefaultMovementCursors(UObject* Self, std::optional<UObject*> newMovementCursor, std::optional<UObject*> newHorizontalMovementCursor, std::optional<UObject*> newVerticalMovementCursor, std::optional<UObject*> newTopLeftMovementCursor, std::optional<UObject*> newTopRightMovementCursor);
	static void SetRawBackground(UObject* Self, std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	static void SetRawBackgroundSize(UObject* Self, float newWidth, float NewHeight);
	static void SetRenderViewport(UObject* Self, float newX, float newY, float newWidth, float NewHeight);
	static void SetSnapshotSize(UObject* Self, float newWidth, float NewHeight);
	static void ShowCursor(UObject* Self, std::optional<bool> bShow);
	static void StretchRawBackground(UObject* Self, std::optional<bool> bStretch);
};
