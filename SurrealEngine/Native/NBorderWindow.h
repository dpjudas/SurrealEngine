#pragma once

#include "UObject/UObject.h"

class NBorderWindow
{
public:
	static void RegisterFunctions();

	static void BaseMarginsFromBorder(UObject* Self, BitfieldBool* bBorder);
	static void EnableResizing(UObject* Self, BitfieldBool* bResize);
	static void SetBorderMargins(UObject* Self, float* NewLeft, float* NewRight, float* newTop, float* newBottom);
	static void SetBorders(UObject* Self, UObject** bordTL, UObject** bordTR, UObject** bordBL, UObject** bordBR, UObject** bordL, UObject** bordR, UObject** bordT, UObject** bordB, UObject** center);
	static void SetMoveCursors(UObject* Self, UObject** Move, UObject** hMove, UObject** vMove, UObject** tlMove, UObject** trMove);
};
