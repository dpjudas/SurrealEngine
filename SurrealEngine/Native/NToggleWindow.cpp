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
	Exception::Throw("NToggleWindow::ChangeToggle not implemented");
}

void NToggleWindow::GetToggle(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NToggleWindow::GetToggle not implemented");
}

void NToggleWindow::SetToggle(UObject* Self, bool bNewToggle)
{
	Exception::Throw("NToggleWindow::SetToggle not implemented");
}

void NToggleWindow::SetToggleSounds(UObject* Self, UObject** enableSound, UObject** disableSound)
{
	Exception::Throw("NToggleWindow::SetToggleSounds not implemented");
}
