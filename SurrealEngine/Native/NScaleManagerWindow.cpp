#include "Precomp.h"
#include "NScaleManagerWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NScaleManagerWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_2("ScaleManagerWindow", "SetManagerAlignments", &NScaleManagerWindow::SetManagerAlignments, 1668);
	RegisterVMNativeFunc_2("ScaleManagerWindow", "SetManagerMargins", &NScaleManagerWindow::SetManagerMargins, 1666);
	RegisterVMNativeFunc_1("ScaleManagerWindow", "SetManagerOrientation", &NScaleManagerWindow::SetManagerOrientation, 1663);
	RegisterVMNativeFunc_1("ScaleManagerWindow", "SetMarginSpacing", &NScaleManagerWindow::SetMarginSpacing, 1667);
	RegisterVMNativeFunc_1("ScaleManagerWindow", "SetScale", &NScaleManagerWindow::SetScale, 1662);
	RegisterVMNativeFunc_2("ScaleManagerWindow", "SetScaleButtons", &NScaleManagerWindow::SetScaleButtons, 1660);
	RegisterVMNativeFunc_1("ScaleManagerWindow", "SetValueField", &NScaleManagerWindow::SetValueField, 1661);
	RegisterVMNativeFunc_1("ScaleManagerWindow", "StretchScaleField", &NScaleManagerWindow::StretchScaleField, 1664);
	RegisterVMNativeFunc_1("ScaleManagerWindow", "StretchValueField", &NScaleManagerWindow::StretchValueField, 1665);
}

void NScaleManagerWindow::SetManagerAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	Exception::Throw("NScaleManagerWindow::SetManagerAlignments not implemented");
}

void NScaleManagerWindow::SetManagerMargins(UObject* Self, float* newMarginWidth, float* newMarginHeight)
{
	Exception::Throw("NScaleManagerWindow::SetManagerMargins not implemented");
}

void NScaleManagerWindow::SetManagerOrientation(UObject* Self, uint8_t newOrientation)
{
	Exception::Throw("NScaleManagerWindow::SetManagerOrientation not implemented");
}

void NScaleManagerWindow::SetMarginSpacing(UObject* Self, float* newSpacing)
{
	Exception::Throw("NScaleManagerWindow::SetMarginSpacing not implemented");
}

void NScaleManagerWindow::SetScale(UObject* Self, UObject* NewScale)
{
	Exception::Throw("NScaleManagerWindow::SetScale not implemented");
}

void NScaleManagerWindow::SetScaleButtons(UObject* Self, UObject* newDecButton, UObject* newIncButton)
{
	Exception::Throw("NScaleManagerWindow::SetScaleButtons not implemented");
}

void NScaleManagerWindow::SetValueField(UObject* Self, UObject* newValueField)
{
	Exception::Throw("NScaleManagerWindow::SetValueField not implemented");
}

void NScaleManagerWindow::StretchScaleField(UObject* Self, BitfieldBool* bNewStretch)
{
	Exception::Throw("NScaleManagerWindow::StretchScaleField not implemented");
}

void NScaleManagerWindow::StretchValueField(UObject* Self, BitfieldBool* bNewStretch)
{
	Exception::Throw("NScaleManagerWindow::StretchValueField not implemented");
}
