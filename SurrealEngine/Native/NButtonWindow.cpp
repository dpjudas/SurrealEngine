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
	button->ActivateButton((EInputKey)Key);
}

void NButtonWindow::EnableAutoRepeat(UObject* Self, std::optional<bool> bEnable, std::optional<float> initialDelay, std::optional<float> repeatRate)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->EnableAutoRepeat(bEnable, initialDelay, repeatRate);
}

void NButtonWindow::EnableRightMouseClick(UObject* Self, std::optional<bool> bEnable)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->EnableRightMouseClick(bEnable);
}

void NButtonWindow::PressButton(UObject* Self, std::optional<uint8_t> Key)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->PressButton(Key);
}

void NButtonWindow::SetActivateDelay(UObject* Self, std::optional<float> newDelay)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetActivateDelay(newDelay);
}

void NButtonWindow::SetButtonColors(UObject* Self, std::optional<Color> Normal, std::optional<Color> pressed, std::optional<Color> normalFocus, std::optional<Color> pressedFocus, std::optional<Color> normalInsensitive, std::optional<Color> pressedInsensitive)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetButtonColors(Normal, pressed, normalFocus, pressedFocus, normalInsensitive, pressedInsensitive);
}

void NButtonWindow::SetButtonSounds(UObject* Self, std::optional<UObject*> pressSound, std::optional<UObject*> clickSound)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetButtonSounds(pressSound, clickSound);
}

void NButtonWindow::SetButtonTextures(UObject* Self, std::optional<UObject*> Normal, std::optional<UObject*> pressed, std::optional<UObject*> normalFocus, std::optional<UObject*> pressedFocus, std::optional<UObject*> normalInsensitive, std::optional<UObject*> pressedInsensitive)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetButtonTextures(Normal, pressed, normalFocus, pressedFocus, normalInsensitive, pressedInsensitive);
}

void NButtonWindow::SetTextColors(UObject* Self, std::optional<Color> Normal, std::optional<Color> pressed, std::optional<Color> normalFocus, std::optional<Color> pressedFocus, std::optional<Color> normalInsensitive, std::optional<Color> pressedInsensitive)
{
	UButtonWindow* button = UObject::Cast<UButtonWindow>(Self);
	button->SetTextColors(Normal, pressed, normalFocus, pressedFocus, normalInsensitive, pressedInsensitive);
}
