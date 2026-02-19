#include "Precomp.h"
#include "NToggleWindow.h"
#include "VM/NativeFunc.h"
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
	LogUnimplemented("NToggleWindow::ChangeToggle not implemented");
}

void NToggleWindow::GetToggle(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NToggleWindow::GetToggle not implemented");
	ReturnValue = false;
}

void NToggleWindow::SetToggle(UObject* Self, bool bNewToggle)
{
	LogUnimplemented("NToggleWindow::SetToggle not implemented");
}

void NToggleWindow::SetToggleSounds(UObject* Self, UObject** enableSound, UObject** disableSound)
{
	LogUnimplemented("NToggleWindow::SetToggleSounds not implemented");
}
