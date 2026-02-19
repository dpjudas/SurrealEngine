#pragma once

#include "UObject/UObject.h"

class NViewportWindow
{
public:
	static void RegisterFunctions();

	static void ClearZBuffer(UObject* Self, BitfieldBool* bClear);
	static void EnableViewport(UObject* Self, BitfieldBool* bEnable);
	static void SetDefaultTexture(UObject* Self, UObject** NewTexture, Color* NewColor);
	static void SetFOVAngle(UObject* Self, float* newAngle);
	static void SetRelativeLocation(UObject* Self, vec3* relLoc);
	static void SetRelativeRotation(UObject* Self, Rotator* relRot);
	static void SetRotation(UObject* Self, Rotator* NewRotation);
	static void SetViewportActor(UObject* Self, UObject** newOriginActor, BitfieldBool* bEyeLevel, BitfieldBool* bEnable);
	static void SetViewportLocation(UObject* Self, const vec3& NewLocation, BitfieldBool* bEnable);
	static void SetWatchActor(UObject* Self, UObject** newWatchActor, BitfieldBool* bEyeLevel);
	static void ShowViewportActor(UObject* Self, BitfieldBool* bShow);
	static void ShowWeapons(UObject* Self, BitfieldBool* bShow);
};
