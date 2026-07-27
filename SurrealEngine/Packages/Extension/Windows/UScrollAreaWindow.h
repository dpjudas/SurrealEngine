#pragma once

#include "UWindow.h"

class UClipWindow;
class UButtonWindow;

class UScrollAreaWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void InitWindow() override;
	void ConfigurationChanged() override;
	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;

	void AutoHideScrollbars(std::optional<bool> bHide);
	void EnableScrolling(std::optional<bool> bHScrolling, std::optional<bool> bVScrolling);
	void SetAreaMargins(float newMarginWidth, float newMarginHeight);
	void SetScrollbarDistance(float newDistance);

	UClipWindow*& ClipWindow() { return Value<UClipWindow*>(PropOffsets_ScrollAreaWindow.ClipWindow); }
	UButtonWindow*& DownButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.DownButton); }
	UButtonWindow*& LeftButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.LeftButton); }
	UButtonWindow*& RightButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.RightButton); }
	UButtonWindow*& UpButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.UpButton); }
	BitfieldBool bHLastShow() { return BoolValue(PropOffsets_ScrollAreaWindow.bHLastShow); }
	BitfieldBool bHideScrollbars() { return BoolValue(PropOffsets_ScrollAreaWindow.bHideScrollbars); }
	BitfieldBool bVLastShow() { return BoolValue(PropOffsets_ScrollAreaWindow.bVLastShow); }
	UScaleWindow*& hScale() { return Value<UScaleWindow*>(PropOffsets_ScrollAreaWindow.hScale); }
	UScaleManagerWindow*& hScaleMgr() { return Value<UScaleManagerWindow*>(PropOffsets_ScrollAreaWindow.hScaleMgr); }
	float& marginHeight() { return Value<float>(PropOffsets_ScrollAreaWindow.marginHeight); }
	float& marginWidth() { return Value<float>(PropOffsets_ScrollAreaWindow.marginWidth); }
	float& scrollbarDistance() { return Value<float>(PropOffsets_ScrollAreaWindow.scrollbarDistance); }
	UScaleWindow*& vScale() { return Value<UScaleWindow*>(PropOffsets_ScrollAreaWindow.vScale); }
	UScaleManagerWindow*& vScaleMgr() { return Value<UScaleManagerWindow*>(PropOffsets_ScrollAreaWindow.vScaleMgr); }
};
