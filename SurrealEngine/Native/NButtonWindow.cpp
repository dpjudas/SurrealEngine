#include "Precomp.h"
#include "NButtonWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->ActivateButton(Key);
}

void NButtonWindow::EnableAutoRepeat(UObject* Self, BitfieldBool* bEnable, float* initialDelay, float* repeatRate)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->EnableAutoRepeat(bEnable, initialDelay, repeatRate);
}

void NButtonWindow::EnableRightMouseClick(UObject* Self, BitfieldBool* bEnable)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->EnableRightMouseClick(bEnable);
}

void NButtonWindow::PressButton(UObject* Self, uint8_t* Key)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->PressButton(Key);
}

void NButtonWindow::SetActivateDelay(UObject* Self, float* newDelay)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetActivateDelay(newDelay);
}

void NButtonWindow::SetButtonColors(UObject* Self, Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetButtonColors(Normal, pressed, normalFocus, pressedFocus, normalInsensitive, pressedInsensitive);
}

void NButtonWindow::SetButtonSounds(UObject* Self, UObject** pressSound, UObject** clickSound)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetButtonSounds(pressSound, clickSound);
}

void NButtonWindow::SetButtonTextures(UObject* Self, UObject** Normal, UObject** pressed, UObject** normalFocus, UObject** pressedFocus, UObject** normalInsensitive, UObject** pressedInsensitive)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetButtonTextures(Normal, pressed, normalFocus, pressedFocus, normalInsensitive, pressedInsensitive);
}

void NButtonWindow::SetTextColors(UObject* Self, Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetTextColors(Normal, pressed, normalFocus, pressedFocus, normalInsensitive, pressedInsensitive);
}
