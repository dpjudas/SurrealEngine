#pragma once

#include "UWindow.h"

class UBorderWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void BaseMarginsFromBorder(std::optional<bool> bBorder);
	void EnableResizing(std::optional<bool> bResize);
	void SetBorderMargins(std::optional<float> NewLeft, std::optional<float> NewRight, std::optional<float> newTop, std::optional<float> newBottom);
	void SetBorders(std::optional<UObject*> bordTL, std::optional<UObject*> bordTR, std::optional<UObject*> bordBL, std::optional<UObject*> bordBR, std::optional<UObject*> bordL, std::optional<UObject*> bordR, std::optional<UObject*> bordT, std::optional<UObject*> bordB, std::optional<UObject*> center);
	void SetMoveCursors(std::optional<UObject*> Move, std::optional<UObject*> hMove, std::optional<UObject*> vMove, std::optional<UObject*> tlMove, std::optional<UObject*> trMove);

	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;
	void ConfigurationChanged() override;
	void DrawWindow(UGC* gc) override;

	float& BottomMargin() { return Value<float>(PropOffsets_BorderWindow.BottomMargin); }
	float& DragX() { return Value<float>(PropOffsets_BorderWindow.DragX); }
	float& DragY() { return Value<float>(PropOffsets_BorderWindow.DragY); }
	UTexture*& MoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.MoveCursor); }
	float& TopMargin() { return Value<float>(PropOffsets_BorderWindow.TopMargin); }
	BitfieldBool bDownDrag() { return BoolValue(PropOffsets_BorderWindow.bDownDrag); }
	BitfieldBool bLeftDrag() { return BoolValue(PropOffsets_BorderWindow.bLeftDrag); }
	BitfieldBool bMarginsFromBorder() { return BoolValue(PropOffsets_BorderWindow.bMarginsFromBorder); }
	BitfieldBool bResizeable() { return BoolValue(PropOffsets_BorderWindow.bResizeable); }
	BitfieldBool bRightDrag() { return BoolValue(PropOffsets_BorderWindow.bRightDrag); }
	BitfieldBool bSmoothBorder() { return BoolValue(PropOffsets_BorderWindow.bSmoothBorder); }
	BitfieldBool bStretchBorder() { return BoolValue(PropOffsets_BorderWindow.bStretchBorder); }
	BitfieldBool bUpDrag() { return BoolValue(PropOffsets_BorderWindow.bUpDrag); }
	UTexture*& borderBottom() { return Value<UTexture*>(PropOffsets_BorderWindow.borderBottom); }
	UTexture*& borderBottomLeft() { return Value<UTexture*>(PropOffsets_BorderWindow.borderBottomLeft); }
	UTexture*& borderBottomRight() { return Value<UTexture*>(PropOffsets_BorderWindow.borderBottomRight); }
	UTexture*& borderLeft() { return Value<UTexture*>(PropOffsets_BorderWindow.borderLeft); }
	UTexture*& borderRight() { return Value<UTexture*>(PropOffsets_BorderWindow.borderRight); }
	uint8_t& borderStyle() { return Value<uint8_t>(PropOffsets_BorderWindow.borderStyle); }
	UTexture*& borderTop() { return Value<UTexture*>(PropOffsets_BorderWindow.borderTop); }
	UTexture*& borderTopLeft() { return Value<UTexture*>(PropOffsets_BorderWindow.borderTopLeft); }
	UTexture*& borderTopRight() { return Value<UTexture*>(PropOffsets_BorderWindow.borderTopRight); }
	UTexture*& center() { return Value<UTexture*>(PropOffsets_BorderWindow.center); }
	float& childBottomMargin() { return Value<float>(PropOffsets_BorderWindow.childBottomMargin); }
	float& childLeftMargin() { return Value<float>(PropOffsets_BorderWindow.childLeftMargin); }
	float& childRightMargin() { return Value<float>(PropOffsets_BorderWindow.childRightMargin); }
	float& childTopMargin() { return Value<float>(PropOffsets_BorderWindow.childTopMargin); }
	float& dragHeight() { return Value<float>(PropOffsets_BorderWindow.dragHeight); }
	float& dragWidth() { return Value<float>(PropOffsets_BorderWindow.dragWidth); }
	UTexture*& hMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.hMoveCursor); }
	float& lastMouseX() { return Value<float>(PropOffsets_BorderWindow.lastMouseX); }
	float& lastMouseY() { return Value<float>(PropOffsets_BorderWindow.lastMouseY); }
	float& leftMargin() { return Value<float>(PropOffsets_BorderWindow.leftMargin); }
	float& rightMargin() { return Value<float>(PropOffsets_BorderWindow.rightMargin); }
	UTexture*& tlMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.tlMoveCursor); }
	UTexture*& trMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.trMoveCursor); }
	UTexture*& vMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.vMoveCursor); }
};
