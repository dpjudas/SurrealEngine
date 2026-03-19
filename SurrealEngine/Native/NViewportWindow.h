#pragma once

#include "UObject/UObject.h"

class NViewportWindow
{
public:
	static void RegisterFunctions();

	static void ClearZBuffer(UObject* Self, std::optional<bool> bClear);
	static void EnableViewport(UObject* Self, std::optional<bool> bEnable);
	static void SetDefaultTexture(UObject* Self, std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	static void SetFOVAngle(UObject* Self, std::optional<float> newAngle);
	static void SetRelativeLocation(UObject* Self, std::optional<vec3> relLoc);
	static void SetRelativeRotation(UObject* Self, std::optional<Rotator> relRot);
	static void SetRotation(UObject* Self, std::optional<Rotator> NewRotation);
	static void SetViewportActor(UObject* Self, std::optional<UObject*> newOriginActor, std::optional<bool> bEyeLevel, std::optional<bool> bEnable);
	static void SetViewportLocation(UObject* Self, const vec3& NewLocation, std::optional<bool> bEnable);
	static void SetWatchActor(UObject* Self, std::optional<UObject*> newWatchActor, std::optional<bool> bEyeLevel);
	static void ShowViewportActor(UObject* Self, std::optional<bool> bShow);
	static void ShowWeapons(UObject* Self, std::optional<bool> bShow);
};
