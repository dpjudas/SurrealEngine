#include "Precomp.h"
#include "NCheckboxWindow.h"
#include "VM/NativeFunc.h"
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
	LogUnimplemented("CheckboxWindow.SetCheckboxColor");
}

void NCheckboxWindow::SetCheckboxSpacing(UObject* Self, float newSpacing)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxSpacing");
}

void NCheckboxWindow::SetCheckboxStyle(UObject* Self, uint8_t NewStyle)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxStyle");
}

void NCheckboxWindow::SetCheckboxTextures(UObject* Self, UObject** toggleOff, UObject** toggleOn, float* textureWidth, float* textureHeight)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxTextures");
}

void NCheckboxWindow::ShowCheckboxOnRightSide(UObject* Self, BitfieldBool* bRight)
{
	LogUnimplemented("CheckboxWindow.ShowCheckboxOnRightSide");
}
