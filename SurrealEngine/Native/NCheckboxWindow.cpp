#include "Precomp.h"
#include "NCheckboxWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NCheckboxWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("CheckboxWindow", "SetCheckboxColor", &NCheckboxWindow::SetCheckboxColor, 1844);
	RegisterVMNativeFunc_1("CheckboxWindow", "SetCheckboxSpacing", &NCheckboxWindow::SetCheckboxSpacing, 1841);
	RegisterVMNativeFunc_1("CheckboxWindow", "SetCheckboxStyle", &NCheckboxWindow::SetCheckboxStyle, 1843);
	RegisterVMNativeFunc_4("CheckboxWindow", "SetCheckboxTextures", &NCheckboxWindow::SetCheckboxTextures, 1840);
	RegisterVMNativeFunc_1("CheckboxWindow", "ShowCheckboxOnRightSide", &NCheckboxWindow::ShowCheckboxOnRightSide, 1842);
}

void NCheckboxWindow::SetCheckboxColor(UObject* Self, const Color& NewColor)
{
	UCheckboxWindow* checkbox = UObject::Cast<UCheckboxWindow>(Self);
	checkbox->SetCheckboxColor(NewColor);
}

void NCheckboxWindow::SetCheckboxSpacing(UObject* Self, float newSpacing)
{
	UCheckboxWindow* checkbox = UObject::Cast<UCheckboxWindow>(Self);
	checkbox->SetCheckboxSpacing(newSpacing);
}

void NCheckboxWindow::SetCheckboxStyle(UObject* Self, uint8_t NewStyle)
{
	UCheckboxWindow* checkbox = UObject::Cast<UCheckboxWindow>(Self);
	checkbox->SetCheckboxStyle(NewStyle);
}

void NCheckboxWindow::SetCheckboxTextures(UObject* Self, UObject** toggleOff, UObject** toggleOn, float* textureWidth, float* textureHeight)
{
	UCheckboxWindow* checkbox = UObject::Cast<UCheckboxWindow>(Self);
	checkbox->SetCheckboxTextures(toggleOff, toggleOn, textureWidth, textureHeight);
}

void NCheckboxWindow::ShowCheckboxOnRightSide(UObject* Self, BitfieldBool* bRight)
{
	UCheckboxWindow* checkbox = UObject::Cast<UCheckboxWindow>(Self);
	checkbox->ShowCheckboxOnRightSide(bRight);
}
