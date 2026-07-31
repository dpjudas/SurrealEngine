
#include "Precomp.h"
#include "UViewportWindow.h"
#include "Engine.h"

void UViewportWindow::ClearZBuffer(std::optional<bool> bClear)
{
	bClearZ() = bClear.has_value() ? bClear.value() : true;
}

void UViewportWindow::EnableViewport(std::optional<bool> bEnable)
{
	bEnableViewport() = bEnable.has_value() ? bEnable.value() : true;
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
	if (newAngle.has_value())
		FOV() = newAngle.value();
}

void UViewportWindow::SetRelativeLocation(std::optional<vec3> relLoc)
{
	if (relLoc.has_value())
		relLocation() = relLoc.value();
}

void UViewportWindow::SetRelativeRotation(std::optional<Rotator> relRot)
{
	if (relRot.has_value())
		relRotation() = relRot.value();
}

void UViewportWindow::SetRotation(std::optional<Rotator> NewRotation)
{
	if (NewRotation.has_value())
		Rotation() = NewRotation.value();
}

void UViewportWindow::SetViewportActor(std::optional<UObject*> newOriginActor, std::optional<bool> bEyeLevel, std::optional<bool> bEnable)
{
	if (newOriginActor.has_value())
		originActor() = UObject::Cast<UActor>(newOriginActor.value());
	if (bEyeLevel.has_value())
		bUseEyeHeight() = bEyeLevel.value();
	if (bEnable.has_value())
		bEnableViewport() = bEnable.value();
}

void UViewportWindow::SetViewportLocation(const vec3& NewLocation, std::optional<bool> bEnable)
{
	Location() = NewLocation;
	if (bEnable.has_value())
		bEnableViewport() = bEnable.value();
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

void UViewportWindow::DrawWindow(UGC* gc)
{
	if (bEnableViewport())
	{
		// To do: draw world here
	}

	UWindow::DrawWindow(gc);
}
