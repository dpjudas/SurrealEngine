
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
	hScaleMgr() = UObject::Cast<UScaleManagerWindow>(NewChild(engine->packages->FindClass("Extension.ScaleManagerWindow")));
	vScaleMgr() = UObject::Cast<UScaleManagerWindow>(NewChild(engine->packages->FindClass("Extension.ScaleManagerWindow")));
	hScale() = UObject::Cast<UScaleWindow>(hScaleMgr()->NewChild(engine->packages->FindClass("Extension.ScaleWindow")));
	vScale() = UObject::Cast<UScaleWindow>(vScaleMgr()->NewChild(engine->packages->FindClass("Extension.ScaleWindow")));
	DownButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));
	LeftButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));
	RightButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));
	UpButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow")));

	ClipWindow()->ForceChildSize(false, false);
	hScale()->SetScaleOrientation((uint8_t)EOrientation::Horizontal);
	vScale()->SetScaleOrientation((uint8_t)EOrientation::Vertical);
	hScaleMgr()->SetScale(hScale());
	vScaleMgr()->SetScale(vScale());

	UWindow::InitWindow();
}

void UScrollAreaWindow::ConfigurationChanged()
{
	float vScrollWidth = 0.0f, hScrollHeight = 0.0f;
	float upW = 0.0f, upH = 0.0f;
	float downW = 0.0f, downH = 0.0f;
	float leftW = 0.0f, leftH = 0.0f;
	float rightW = 0.0f, rightH = 0.0f;
	if (vScaleMgr()->bIsVisible())
	{
		UpButton()->QueryPreferredSize(upW, upH);
		DownButton()->QueryPreferredSize(downW, downH);
		vScrollWidth = upW;
	}
	if (hScaleMgr()->bIsVisible())
	{
		LeftButton()->QueryPreferredSize(leftW, leftH);
		RightButton()->QueryPreferredSize(rightW, rightH);
		hScrollHeight = leftH;
	}

	ClipWindow()->ConfigureChild(0.0f, 0.0f, Width() - vScrollWidth, Height() - hScrollHeight);
	hScaleMgr()->ConfigureChild(leftW, Height() - hScrollHeight, Width() - leftW - rightW, hScrollHeight);
	vScaleMgr()->ConfigureChild(Width() - vScrollWidth, upH, vScrollWidth, Height() - upH - downH);

	UpButton()->SetPos(Width() - upW, 0.0f);
	DownButton()->SetPos(Width() - downW, Height() - hScrollHeight - downH);
	LeftButton()->SetPos(0.0f, Height() - leftH);
	RightButton()->SetPos(Width() - rightW - vScrollWidth, Height() - rightH);

	UWindow::ConfigurationChanged();
}

void UScrollAreaWindow::DrawWindow(UGC* gc)
{
	// We are doing this here because we can't do it in ConfigurationChanged. The child configuration events are deferred.
	// We don't know the final layout of the child window until now.
	if (auto content = UObject::Cast<UWindow>(ClipWindow()->GetChild()))
	{
		int childX = 0, childY = 0;
		ClipWindow()->GetChildPosition(childX, childY);
		hScale()->SetThumbSpan((int)ClipWindow()->Width());
		hScale()->SetValueRange(0.0f, std::max(content->Width() - ClipWindow()->Width(), 0.0f));
		hScale()->SetValue((float)childX);
		vScale()->SetThumbSpan((int)ClipWindow()->Height());
		vScale()->SetValueRange(0.0f, std::max(content->Height() - ClipWindow()->Height(), 0.0f));
		vScale()->SetValue((float)childY);
	}

	UWindow::DrawWindow(gc);
}

void UScrollAreaWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	float vScrollWidth = vScale()->ThumbWidth();
	float hScrollHeight = hScale()->ThumbHeight();

	if (!bWidthSpecified && !bHeightSpecified)
	{
		ClipWindow()->QueryPreferredSize(preferredWidth, preferredHeight);
		preferredWidth += hScrollHeight;
		preferredHeight += vScrollWidth;
	}
	else if (!bWidthSpecified)
	{
		preferredWidth = ClipWindow()->QueryPreferredWidth(preferredHeight - hScrollHeight) + hScrollHeight;
	}
	else // if (!bHeightSpecified)
	{
		preferredHeight = ClipWindow()->QueryPreferredHeight(preferredWidth - vScrollWidth) + vScrollWidth;
	}
}

void UScrollAreaWindow::AutoHideScrollbars(std::optional<bool> bHide)
{
	bHideScrollbars() = bHide ? *bHide : true;
	AskParentForReconfigure();
}

void UScrollAreaWindow::EnableScrolling(std::optional<bool> bHScrolling, std::optional<bool> bVScrolling)
{
	if (bHScrolling)
	{
		bool show = *bHScrolling;
		hScaleMgr()->bIsVisible() = show;
		LeftButton()->bIsVisible() = show;
		RightButton()->bIsVisible() = show;
	}
	if (bVScrolling)
	{
		bool show = *bVScrolling;
		vScaleMgr()->bIsVisible() = show;
		UpButton()->bIsVisible() = show;
		DownButton()->bIsVisible() = show;
	}
	ClipWindow()->ForceChildSize(!hScaleMgr()->bIsVisible(), !vScaleMgr()->bIsVisible());
	AskParentForReconfigure();
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
