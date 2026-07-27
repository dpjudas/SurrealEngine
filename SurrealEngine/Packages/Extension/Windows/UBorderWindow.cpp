
#include "Precomp.h"
#include "UBorderWindow.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "Packages/Extension/Windows/UGC.h"

void UBorderWindow::BaseMarginsFromBorder(std::optional<bool> bBorder)
{
	bMarginsFromBorder() = !bBorder || *bBorder;
}

void UBorderWindow::EnableResizing(std::optional<bool> bResize)
{
	bResizeable() = !bResize || *bResize;
}

void UBorderWindow::SetBorderMargins(std::optional<float> NewLeft, std::optional<float> NewRight, std::optional<float> newTop, std::optional<float> newBottom)
{
	if (NewLeft)
		childLeftMargin() = *NewLeft;
	if (NewRight)
		childRightMargin() = *NewRight;
	if (newTop)
		childTopMargin() = *newTop;
	if (newBottom)
		childBottomMargin() = *newBottom;
}

void UBorderWindow::SetBorders(std::optional<UObject*> bordTL, std::optional<UObject*> bordTR, std::optional<UObject*> bordBL, std::optional<UObject*> bordBR, std::optional<UObject*> bordL, std::optional<UObject*> bordR, std::optional<UObject*> bordT, std::optional<UObject*> bordB, std::optional<UObject*> newCenter)
{
	if (bordTL)
		borderTopLeft() = UObject::Cast<UTexture>(*bordTL);
	if (bordTR)
		borderTopRight() = UObject::Cast<UTexture>(*bordTR);
	if (bordBL)
		borderBottomLeft() = UObject::Cast<UTexture>(*bordBL);
	if (bordBR)
		borderBottomRight() = UObject::Cast<UTexture>(*bordBR);
	if (bordL)
		borderLeft() = UObject::Cast<UTexture>(*bordL);
	if (bordR)
		borderRight() = UObject::Cast<UTexture>(*bordR);
	if (bordT)
		borderTop() = UObject::Cast<UTexture>(*bordT);
	if (bordB)
		borderBottom() = UObject::Cast<UTexture>(*bordB);
	if (newCenter)
		center() = UObject::Cast<UTexture>(*newCenter);
}

void UBorderWindow::SetMoveCursors(std::optional<UObject*> Move, std::optional<UObject*> hMove, std::optional<UObject*> vMove, std::optional<UObject*> tlMove, std::optional<UObject*> trMove)
{
	if (Move)
		MoveCursor() = UObject::Cast<UTexture>(*Move);
	if (hMove)
		hMoveCursor() = UObject::Cast<UTexture>(*hMove);
	if (vMove)
		vMoveCursor() = UObject::Cast<UTexture>(*vMove);
	if (tlMove)
		tlMoveCursor() = UObject::Cast<UTexture>(*tlMove);
	if (trMove)
		trMoveCursor() = UObject::Cast<UTexture>(*trMove);
}

void UBorderWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	preferredWidth = 0.0f;
	preferredHeight = 0.0f;
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		float w = 0.0f, h = 0.0f;
		cur->QueryPreferredSize(w, h);
		preferredWidth = std::max(preferredWidth, w);
		preferredHeight += h;
	}

	preferredWidth += childLeftMargin();
	preferredWidth += childRightMargin();
	preferredHeight += childTopMargin();
	preferredHeight += childBottomMargin();

	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UBorderWindow::ConfigurationChanged()
{
	float y = 0.0f;

	y += childTopMargin();

	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		float contentWidth = std::max(Width() - childLeftMargin() - childRightMargin(), 0.0f);
		float h = cur->QueryPreferredHeight(contentWidth);
		float w = cur->QueryPreferredWidth(h);
		cur->ConfigureChild(childLeftMargin(), y, std::min(w, contentWidth), h);
		y += h;
	}

	UWindow::ConfigurationChanged();
}

void UBorderWindow::DrawWindow(UGC* gc)
{
	UObject* borders[9] =
	{
		borderTopLeft(),
		borderTopRight(),
		borderBottomLeft(),
		borderLeft(),
		borderBottomRight(),
		borderRight(),
		borderTop(),
		borderBottom(),
		center()
	};

	gc->DrawBorders(
		0.0f, 0.0f,
		Width(), Height(),
		childLeftMargin(), childRightMargin(),
		childTopMargin(), childBottomMargin(), borders, {}, {});

	UWindow::DrawWindow(gc);
	DrawDebugBox(gc);
}
