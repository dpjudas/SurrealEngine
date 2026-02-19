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
	LogUnimplemented("ScaleManagerWindow.SetManagerAlignments");
}

void NScaleManagerWindow::SetManagerMargins(UObject* Self, float* newMarginWidth, float* newMarginHeight)
{
	LogUnimplemented("ScaleManagerWindow.SetManagerMargins");
}

void NScaleManagerWindow::SetManagerOrientation(UObject* Self, uint8_t newOrientation)
{
	LogUnimplemented("ScaleManagerWindow.SetManagerOrientation");
}

void NScaleManagerWindow::SetMarginSpacing(UObject* Self, float* newSpacing)
{
	LogUnimplemented("ScaleManagerWindow.SetMarginSpacing");
}

void NScaleManagerWindow::SetScale(UObject* Self, UObject* NewScale)
{
	LogUnimplemented("ScaleManagerWindow.SetScale");
}

void NScaleManagerWindow::SetScaleButtons(UObject* Self, UObject* newDecButton, UObject* newIncButton)
{
	LogUnimplemented("ScaleManagerWindow.SetScaleButtons");
}

void NScaleManagerWindow::SetValueField(UObject* Self, UObject* newValueField)
{
	LogUnimplemented("ScaleManagerWindow.SetValueField");
}

void NScaleManagerWindow::StretchScaleField(UObject* Self, BitfieldBool* bNewStretch)
{
	LogUnimplemented("ScaleManagerWindow.StretchScaleField");
}

void NScaleManagerWindow::StretchValueField(UObject* Self, BitfieldBool* bNewStretch)
{
	LogUnimplemented("ScaleManagerWindow.StretchValueField");
}
