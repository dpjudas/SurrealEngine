
#include "Precomp.h"
#include "UClipWindow.h"
#include "Utils/Logger.h"

void UClipWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	float w = 0.0f;
	float h = 0.0f;
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		float curW = 0.0f;
		float curH = 0.0f;
		cur->QueryPreferredSize(curW, curH);
		w = std::max(w, curW);
		h = std::max(h, curH);
	}
	preferredWidth = w;
	preferredHeight = h;

	UTabGroupWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UClipWindow::ConfigurationChanged()
{
	if (auto child = UObject::Cast<UWindow>(GetChild()))
	{
		float w = Width(), h = Height();
		if (bForceChildWidth() && !bForceChildHeight())
			h = child->QueryPreferredHeight(w);
		else if (!bForceChildWidth() && bForceChildHeight())
			w = child->QueryPreferredWidth(h);
		else if (!bForceChildWidth() && !bForceChildHeight())
			child->QueryPreferredSize(w, h);
		child->ConfigureChild(ChildPos.x, ChildPos.y, w, h);
	}

	UTabGroupWindow::ConfigurationChanged();
}

void UClipWindow::ForceChildSize(std::optional<bool> bNewForceChildWidth, std::optional<bool> bNewForceChildHeight)
{
	if (bNewForceChildWidth)
		bForceChildWidth() = *bNewForceChildWidth;
	if (bNewForceChildHeight)
		bForceChildHeight() = *bNewForceChildHeight;
}

UObject* UClipWindow::GetChild()
{
	// Not called by script
	return firstChild();
}

void UClipWindow::GetChildPosition(int& pNewX, int& pNewY)
{
	if (auto child = UObject::Cast<UWindow>(GetChild()))
	{
		pNewX = (int)std::round(ChildPos.x);
		pNewY = (int)std::round(ChildPos.y);
	}
	else
	{
		pNewX = 0;
		pNewY = 0;
	}
}

void UClipWindow::SetChildPosition(int newX, int newY)
{
	ChildPos.x = (float)newX;
	ChildPos.y = (float)newY;
}

void UClipWindow::EnableSnapToUnits(std::optional<bool> bNewSnapToUnits)
{
	// UNUSED from scripts.
	LogUnimplemented("ClipWindow.EnableSnapToUnits");
}

void UClipWindow::GetUnitSize(int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize)
{
	// UNUSED from scripts.
	LogUnimplemented("ClipWindow.GetUnitSize");
}

void UClipWindow::ResetUnitHeight()
{
	// UNUSED by scripts.
	LogUnimplemented("ClipWindow.ResetUnitHeight");
}

void UClipWindow::ResetUnitSize()
{
	// UNUSED by scripts.
	LogUnimplemented("ClipWindow.ResetUnitSize");
}

void UClipWindow::ResetUnitWidth()
{
	// UNUSED by scripts.
	LogUnimplemented("ClipWindow.ResetUnitWidth");
}

void UClipWindow::SetUnitHeight(int vUnits)
{
	// UNUSED from scripts.
	LogUnimplemented("ClipWindow.SetUnitHeight");
}

void UClipWindow::SetUnitSize(int hUnits, int vUnits)
{
	// UNUSED from scripts.
	LogUnimplemented("ClipWindow.SetUnitSize");
}

void UClipWindow::SetUnitWidth(int hUnits)
{
	// UNUSED from scripts.
	LogUnimplemented("ClipWindow.SetUnitWidth");
}
