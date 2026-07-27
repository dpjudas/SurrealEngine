
#include "Precomp.h"
#include "UClipWindow.h"
#include "Utils/Logger.h"

void UClipWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	// To do: how does this work?
	UTabGroupWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UClipWindow::ConfigurationChanged()
{
	// To do: how does this work?
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		cur->ConfigureChild(0.0f, 0.0f, Width(), Height());
	}

	UTabGroupWindow::ConfigurationChanged();
}

void UClipWindow::EnableSnapToUnits(std::optional<bool> bNewSnapToUnits)
{
	// UNUSED from scripts.
	LogUnimplemented("ClipWindow.EnableSnapToUnits");
}

void UClipWindow::ForceChildSize(std::optional<bool> bNewForceChildWidth, std::optional<bool> bNewForceChildHeight)
{
	LogUnimplemented("ClipWindow.ForceChildSize");
}

UObject* UClipWindow::GetChild()
{
	// Not called by script
	LogUnimplemented("ClipWindow.GetChild");
	return nullptr;
}

void UClipWindow::GetChildPosition(int& pNewX, int& pNewY)
{
	LogUnimplemented("ClipWindow.GetChildPosition");
}

void UClipWindow::GetUnitSize(int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize)
{
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

void UClipWindow::SetChildPosition(int newX, int newY)
{
	LogUnimplemented("ClipWindow.SetChildPosition");
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
