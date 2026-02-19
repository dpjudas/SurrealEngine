#include "Precomp.h"
#include "NButtonWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NButtonWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ButtonWindow", "ActivateButton", &NButtonWindow::ActivateButton, 1590);
	RegisterVMNativeFunc_3("ButtonWindow", "EnableAutoRepeat", &NButtonWindow::EnableAutoRepeat, 1595);
	RegisterVMNativeFunc_1("ButtonWindow", "EnableRightMouseClick", &NButtonWindow::EnableRightMouseClick, 1596);
	RegisterVMNativeFunc_1("ButtonWindow", "PressButton", &NButtonWindow::PressButton, 1598);
	RegisterVMNativeFunc_1("ButtonWindow", "SetActivateDelay", &NButtonWindow::SetActivateDelay, 1591);
	RegisterVMNativeFunc_6("ButtonWindow", "SetButtonColors", &NButtonWindow::SetButtonColors, 1593);
	RegisterVMNativeFunc_2("ButtonWindow", "SetButtonSounds", &NButtonWindow::SetButtonSounds, 1597);
	RegisterVMNativeFunc_6("ButtonWindow", "SetButtonTextures", &NButtonWindow::SetButtonTextures, 1592);
	RegisterVMNativeFunc_6("ButtonWindow", "SetTextColors", &NButtonWindow::SetTextColors, 1594);
}

void NButtonWindow::ActivateButton(UObject* Self, uint8_t Key)
{
	LogUnimplemented("NButtonWindow::ActivateButton not implemented");
}

void NButtonWindow::EnableAutoRepeat(UObject* Self, BitfieldBool* bEnable, float* initialDelay, float* repeatRate)
{
	LogUnimplemented("NButtonWindow::EnableAutoRepeat not implemented");
}

void NButtonWindow::EnableRightMouseClick(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("NButtonWindow::EnableRightMouseClick not implemented");
}

void NButtonWindow::PressButton(UObject* Self, uint8_t* Key)
{
	LogUnimplemented("NButtonWindow::PressButton not implemented");
}

void NButtonWindow::SetActivateDelay(UObject* Self, float* newDelay)
{
	LogUnimplemented("NButtonWindow::SetActivateDelay not implemented");
}

void NButtonWindow::SetButtonColors(UObject* Self, Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	LogUnimplemented("NButtonWindow::SetButtonColors not implemented");
}

void NButtonWindow::SetButtonSounds(UObject* Self, UObject** pressSound, UObject** clickSound)
{
	LogUnimplemented("NButtonWindow::SetButtonSounds not implemented");
}

void NButtonWindow::SetButtonTextures(UObject* Self, UObject** Normal, UObject** pressed, UObject** normalFocus, UObject** pressedFocus, UObject** normalInsensitive, UObject** pressedInsensitive)
{
	LogUnimplemented("NButtonWindow::SetButtonTextures not implemented");
}

void NButtonWindow::SetTextColors(UObject* Self, Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	LogUnimplemented("NButtonWindow::SetTextColors not implemented");
}
