#pragma once

#include "UObject/UObject.h"

class NBorderWindow
{
public:
	static void RegisterFunctions();

	static void BaseMarginsFromBorder(UObject* Self, std::optional<bool> bBorder);
	static void EnableResizing(UObject* Self, std::optional<bool> bResize);
	static void SetBorderMargins(UObject* Self, std::optional<float> NewLeft, std::optional<float> NewRight, std::optional<float> newTop, std::optional<float> newBottom);
	static void SetBorders(UObject* Self, std::optional<UObject*> bordTL, std::optional<UObject*> bordTR, std::optional<UObject*> bordBL, std::optional<UObject*> bordBR, std::optional<UObject*> bordL, std::optional<UObject*> bordR, std::optional<UObject*> bordT, std::optional<UObject*> bordB, std::optional<UObject*> center);
	static void SetMoveCursors(UObject* Self, std::optional<UObject*> Move, std::optional<UObject*> hMove, std::optional<UObject*> vMove, std::optional<UObject*> tlMove, std::optional<UObject*> trMove);
};
