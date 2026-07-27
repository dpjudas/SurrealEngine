
#include "Precomp.h"
#include "URadioBoxWindow.h"
#include "Utils/Logger.h"

UObject* URadioBoxWindow::GetEnabledToggle()
{
	// UNUSED from scripts.
	LogUnimplemented("RadioBoxWindow.GetEnabledToggle");
	return nullptr;
}

void URadioBoxWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	preferredWidth = 0.0f;
	preferredHeight = 0.0f;
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		float w = 0.0f, h = 0.0f;
		cur->QueryPreferredSize(w, h);
		preferredHeight = std::max(preferredHeight, h);
		preferredWidth += h;
	}
}

void URadioBoxWindow::ConfigurationChanged()
{
	if (bSizeChildrenToParent())
	{
		int childCount = 0;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			if (cur->bIsVisible())
				childCount++;
		}
		if (childCount == 0)
			return;

		float totalWidth = Width();
		float h = Height();
		float x = 0.0f;
		int idx = 0;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			if (cur->bIsVisible())
			{
				float w = totalWidth * idx / childCount;
				cur->ConfigureChild(x, 0.0f, w, h);
				x += w;
				idx++;
			}
		}
	}
	else if (!bSizeParentToChildren())
	{
		float x = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = 0.0f, h = 0.0f;
			cur->QueryPreferredSize(w, h);
			cur->ConfigureChild(x, 0.0f, w, h);
			x += w;
		}
	}

	UTabGroupWindow::ConfigurationChanged();
}
