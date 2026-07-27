#pragma once

#include "UWindow.h"

class UViewportWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void ClearZBuffer(std::optional<bool> bClear);
	void EnableViewport(std::optional<bool> bEnable);
	void SetDefaultTexture(std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	void SetFOVAngle(std::optional<float> newAngle);
	void SetRelativeLocation(std::optional<vec3> relLoc);
	void SetRelativeRotation(std::optional<Rotator> relRot);
	void SetRotation(std::optional<Rotator> NewRotation);
	void SetViewportActor(std::optional<UObject*> newOriginActor, std::optional<bool> bEyeLevel, std::optional<bool> bEnable);
	void SetViewportLocation(const vec3& NewLocation, std::optional<bool> bEnable);
	void SetWatchActor(std::optional<UObject*> newWatchActor, std::optional<bool> bEyeLevel);
	void ShowViewportActor(std::optional<bool> bShow);
	void ShowWeapons(std::optional<bool> bShow);

	Color& DefaultColor() { return Value<Color>(PropOffsets_ViewportWindow.DefaultColor); }
	UTexture*& DefaultTexture() { return Value<UTexture*>(PropOffsets_ViewportWindow.DefaultTexture); }
	float& FOV() { return Value<float>(PropOffsets_ViewportWindow.FOV); }
	vec3& Location() { return Value<vec3>(PropOffsets_ViewportWindow.Location); }
	Rotator& Rotation() { return Value<Rotator>(PropOffsets_ViewportWindow.Rotation); }
	BitfieldBool bClearZ() { return BoolValue(PropOffsets_ViewportWindow.bClearZ); }
	BitfieldBool bEnableViewport() { return BoolValue(PropOffsets_ViewportWindow.bEnableViewport); }
	BitfieldBool bOriginActorDestroyed() { return BoolValue(PropOffsets_ViewportWindow.bOriginActorDestroyed); }
	BitfieldBool bShowActor() { return BoolValue(PropOffsets_ViewportWindow.bShowActor); }
	BitfieldBool bShowWeapons() { return BoolValue(PropOffsets_ViewportWindow.bShowWeapons); }
	BitfieldBool bUseEyeHeight() { return BoolValue(PropOffsets_ViewportWindow.bUseEyeHeight); }
	BitfieldBool bUseViewRotation() { return BoolValue(PropOffsets_ViewportWindow.bUseViewRotation); }
	BitfieldBool bWatchEyeHeight() { return BoolValue(PropOffsets_ViewportWindow.bWatchEyeHeight); }
	vec3& lastLocation() { return Value<vec3>(PropOffsets_ViewportWindow.lastLocation); }
	Rotator& lastRotation() { return Value<Rotator>(PropOffsets_ViewportWindow.lastRotation); }
	UActor*& originActor() { return Value<UActor*>(PropOffsets_ViewportWindow.originActor); }
	vec3& relLocation() { return Value<vec3>(PropOffsets_ViewportWindow.relLocation); }
	Rotator& relRotation() { return Value<Rotator>(PropOffsets_ViewportWindow.relRotation); }
	int& viewportFrame() { return Value<int>(PropOffsets_ViewportWindow.viewportFrame); }
	UActor*& watchActor() { return Value<UActor*>(PropOffsets_ViewportWindow.watchActor); }
};
