#pragma once

#include "UObject/UObject.h"

class NRootWindow
{
public:
	static void RegisterFunctions();

	static void EnablePositionalSound(UObject* Self, BitfieldBool* bEnable);
	static void EnableRendering(UObject* Self, BitfieldBool* bRender);
	static void GenerateSnapshot(UObject* Self, BitfieldBool* bFilter, UObject*& ReturnValue);
	static void IsPositionalSoundEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsRenderingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void LockMouse(UObject* Self, BitfieldBool* bLockMove, BitfieldBool* bLockButton);
	static void ResetRenderViewport(UObject* Self);
	static void SetDefaultEditCursor(UObject* Self, UObject** newEditCursor);
	static void SetDefaultMovementCursors(UObject* Self, UObject** newMovementCursor, UObject** newHorizontalMovementCursor, UObject** newVerticalMovementCursor, UObject** newTopLeftMovementCursor, UObject** newTopRightMovementCursor);
	static void SetRawBackground(UObject* Self, UObject** NewTexture, Color* NewColor);
	static void SetRawBackgroundSize(UObject* Self, float newWidth, float NewHeight);
	static void SetRenderViewport(UObject* Self, float newX, float newY, float newWidth, float NewHeight);
	static void SetSnapshotSize(UObject* Self, float newWidth, float NewHeight);
	static void ShowCursor(UObject* Self, BitfieldBool* bShow);
	static void StretchRawBackground(UObject* Self, BitfieldBool* bStretch);
};
