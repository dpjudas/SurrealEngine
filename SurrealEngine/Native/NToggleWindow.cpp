#include "Precomp.h"
#include "NToggleWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NToggleWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_0("ToggleWindow", "ChangeToggle", &NToggleWindow::ChangeToggle, 1610);
	RegisterVMNativeFunc_1("ToggleWindow", "GetToggle", &NToggleWindow::GetToggle, 1612);
	RegisterVMNativeFunc_1("ToggleWindow", "SetToggle", &NToggleWindow::SetToggle, 1611);
	RegisterVMNativeFunc_2("ToggleWindow", "SetToggleSounds", &NToggleWindow::SetToggleSounds, 1613);
}

void NToggleWindow::ChangeToggle(UObject* Self)
{
	UToggleWindow* toggle = UObject::Cast<UToggleWindow>(Self);
	toggle->ChangeToggle();
}

void NToggleWindow::GetToggle(UObject* Self, BitfieldBool& ReturnValue)
{
	UToggleWindow* toggle = UObject::Cast<UToggleWindow>(Self);
	ReturnValue = toggle->GetToggle();
}

void NToggleWindow::SetToggle(UObject* Self, bool bNewToggle)
{
	UToggleWindow* toggle = UObject::Cast<UToggleWindow>(Self);
	toggle->SetToggle(bNewToggle);
}

void NToggleWindow::SetToggleSounds(UObject* Self, UObject** enableSound, UObject** disableSound)
{
	UToggleWindow* toggle = UObject::Cast<UToggleWindow>(Self);
	toggle->SetToggleSounds(enableSound, disableSound);
}
