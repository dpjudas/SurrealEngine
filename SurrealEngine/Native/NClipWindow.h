#pragma once

#include "UObject/UObject.h"

class NClipWindow
{
public:
	static void RegisterFunctions();

	static void EnableSnapToUnits(UObject* Self, BitfieldBool* bNewSnapToUnits);
	static void ForceChildSize(UObject* Self, BitfieldBool* bNewForceChildWidth, BitfieldBool* bNewForceChildHeight);
	static void GetChild(UObject* Self, UObject*& ReturnValue);
	static void GetChildPosition(UObject* Self, int& pNewX, int& pNewY);
	static void GetUnitSize(UObject* Self, int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize);
	static void ResetUnitHeight(UObject* Self);
	static void ResetUnitSize(UObject* Self);
	static void ResetUnitWidth(UObject* Self);
	static void SetChildPosition(UObject* Self, int newX, int newY);
	static void SetUnitHeight(UObject* Self, int vUnits);
	static void SetUnitSize(UObject* Self, int hUnits, int vUnits);
	static void SetUnitWidth(UObject* Self, int hUnits);
};
