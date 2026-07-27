
#include "Precomp.h"
#include "UModalWindow.h"
#include "URootWindow.h"

bool UModalWindow::IsCurrentModal()
{
	URootWindow* root = GetRootWindow();
	if (root)
	{
		for (UWindow* child = root->lastChild(); child; child = child->prevSibling())
		{
			if (child->bIsVisible())
			{
				if (auto modal = UObject::TryCast<UModalWindow>(child))
					return modal == this;
			}
		}
	}
	return false;
}

void UModalWindow::SetMouseFocusMode(uint8_t newFocusMode)
{
	focusMode() = newFocusMode;
}
