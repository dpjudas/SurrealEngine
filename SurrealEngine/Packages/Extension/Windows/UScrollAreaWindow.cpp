
#include "Precomp.h"
#include "UScrollAreaWindow.h"
#include "UScaleManagerWindow.h"
#include "UScaleWindow.h"
#include "TabGroup/UClipWindow.h"
#include "Text/UButtonWindow.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Packages/Core/UClass.h"

void UScrollAreaWindow::InitWindow()
{
	ClipWindow() = UObject::Cast<UClipWindow>(NewChild(engine->packages->FindClass("Extension.ClipWindow")));
	hScale() = UObject::Cast<UScaleWindow>(NewChild(engine->packages->FindClass("Extension.ScaleWindow")));
	vScale() = UObject::Cast<UScaleWindow>(NewChild(engine->packages->FindClass("Extension.ScaleWindow")));
	hScaleMgr() = UObject::Cast<UScaleManagerWindow>(NewChild(engine->packages->FindClass("Extension.ScaleManagerWindow")));
	vScaleMgr() = UObject::Cast<UScaleManagerWindow>(NewChild(engine->packages->FindClass("Extension.ScaleManagerWindow")));
	DownButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));
	LeftButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));
	RightButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));
	UpButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));

	ClipWindow()->bIsVisible() = true;
	hScale()->bIsVisible() = false;
	vScale()->bIsVisible() = false;
	hScaleMgr()->bIsVisible() = false;
	vScaleMgr()->bIsVisible() = false;
	DownButton()->bIsVisible() = false;
	LeftButton()->bIsVisible() = false;
	RightButton()->bIsVisible() = false;
	UpButton()->bIsVisible() = false;

	UWindow::InitWindow();
}

void UScrollAreaWindow::ConfigurationChanged()
{
	// To do: position the scrollbars (is that scalemgr or scale?), the scroll buttons and clip window properly
	// To do: how is the size of the clip window determined?

	ClipWindow()->ConfigureChild(0.0f, 0.0f, Width(), Height());
	UWindow::ConfigurationChanged();
}

void UScrollAreaWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	// To do: have to take the scrollbars into account

	if (!bWidthSpecified && !bHeightSpecified)
	{
		ClipWindow()->QueryPreferredSize(preferredWidth, preferredHeight);
	}
	else if (!bWidthSpecified)
	{
		ClipWindow()->QueryPreferredWidth(preferredHeight);
	}
	else // if (!bHeightSpecified)
	{
		ClipWindow()->QueryPreferredHeight(preferredWidth);
	}
}

void UScrollAreaWindow::AutoHideScrollbars(std::optional<bool> bHide)
{
	// UNUSED from scripts.
	LogUnimplemented("ScrollAreaWindow.AutoHideScrollbars");
}

void UScrollAreaWindow::EnableScrolling(std::optional<bool> bHScrolling, std::optional<bool> bVScrolling)
{
	LogUnimplemented("ScrollAreaWindow.EnableScrolling");
}

void UScrollAreaWindow::SetAreaMargins(float newMarginWidth, float newMarginHeight)
{
	marginWidth() = newMarginWidth;
	marginHeight() = newMarginHeight;
}

void UScrollAreaWindow::SetScrollbarDistance(float newDistance)
{
	scrollbarDistance() = newDistance;
}
