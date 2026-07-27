
#include "Precomp.h"
#include "UViewportWindow.h"
#include "Engine.h"

void UViewportWindow::ClearZBuffer(std::optional<bool> bClear)
{
	LogUnimplemented("ViewportWindow.ClearZBuffer");
}

void UViewportWindow::EnableViewport(std::optional<bool> bEnable)
{
	LogUnimplemented("ViewportWindow.EnableViewport");
}

void UViewportWindow::SetDefaultTexture(std::optional<UObject*> NewTexture, std::optional<Color> NewColor)
{
	if (NewTexture)
		DefaultTexture() = UObject::Cast<UTexture>(*NewTexture);
	if (NewColor)
		DefaultColor() = *NewColor;
}

void UViewportWindow::SetFOVAngle(std::optional<float> newAngle)
{
	LogUnimplemented("ViewportWindow.SetFOVAngle");
}

void UViewportWindow::SetRelativeLocation(std::optional<vec3> relLoc)
{
	LogUnimplemented("ViewportWindow.SetRelativeLocation");
}

void UViewportWindow::SetRelativeRotation(std::optional<Rotator> relRot)
{
	LogUnimplemented("ViewportWindow.SetRelativeRotation");
}

void UViewportWindow::SetRotation(std::optional<Rotator> NewRotation)
{
	LogUnimplemented("ViewportWindow.SetRotation");
}

void UViewportWindow::SetViewportActor(std::optional<UObject*> newOriginActor, std::optional<bool> bEyeLevel, std::optional<bool> bEnable)
{
	LogUnimplemented("ViewportWindow.SetViewportActor");
}

void UViewportWindow::SetViewportLocation(const vec3& NewLocation, std::optional<bool> bEnable)
{
	LogUnimplemented("ViewportWindow.SetViewportLocation");
}

void UViewportWindow::SetWatchActor(std::optional<UObject*> newWatchActor, std::optional<bool> bEyeLevel)
{
	if (newWatchActor)
		watchActor() = UObject::Cast<UActor>(*newWatchActor);
	if (bEyeLevel)
		bWatchEyeHeight() = *bEyeLevel;
}

void UViewportWindow::ShowViewportActor(std::optional<bool> bShow)
{
	bShowActor() = !bShow || *bShow;
}

void UViewportWindow::ShowWeapons(std::optional<bool> bShow)
{
	bShowWeapons() = !bShow || *bShow;
}
