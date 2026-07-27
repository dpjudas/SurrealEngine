
#include "Precomp.h"
#include "UToggleWindow.h"
#include "Packages/Engine/Resources/USound.h"

void UToggleWindow::ChangeToggle()
{
	SetToggle(!GetToggle());
}

bool UToggleWindow::GetToggle()
{
	return isToggled;
}

void UToggleWindow::SetToggle(bool bNewToggle)
{
	if (isToggled != bNewToggle)
	{
		isToggled = bNewToggle;
		for (UWindow* cur = this; cur; cur = cur->parentOwner())
		{
			if (cur->ToggleChanged(this, bNewToggle))
				break;
		}
	}
}

void UToggleWindow::SetToggleSounds(std::optional<UObject*> newEnableSound, std::optional<UObject*> newDisableSound)
{
	if (newEnableSound)
		enableSound() = UObject::Cast<USound>(*newEnableSound);
	if (newDisableSound)
		disableSound() = UObject::Cast<USound>(*newDisableSound);
}
