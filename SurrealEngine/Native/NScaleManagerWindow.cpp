#include "Precomp.h"
#include "NScaleManagerWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetManagerAlignments(newHAlign, newVAlign);
}

void NScaleManagerWindow::SetManagerMargins(UObject* Self, float* newMarginWidth, float* newMarginHeight)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetManagerMargins(newMarginWidth, newMarginHeight);
}

void NScaleManagerWindow::SetManagerOrientation(UObject* Self, uint8_t newOrientation)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetManagerOrientation(newOrientation);
}

void NScaleManagerWindow::SetMarginSpacing(UObject* Self, float* newSpacing)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetMarginSpacing(newSpacing);
}

void NScaleManagerWindow::SetScale(UObject* Self, UObject* NewScale)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetScale(NewScale);
}

void NScaleManagerWindow::SetScaleButtons(UObject* Self, UObject* newDecButton, UObject* newIncButton)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetScaleButtons(newDecButton, newIncButton);
}

void NScaleManagerWindow::SetValueField(UObject* Self, UObject* newValueField)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->SetValueField(newValueField);
}

void NScaleManagerWindow::StretchScaleField(UObject* Self, BitfieldBool* bNewStretch)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->StretchScaleField(bNewStretch);
}

void NScaleManagerWindow::StretchValueField(UObject* Self, BitfieldBool* bNewStretch)
{
	UScaleManagerWindow* window = UObject::Cast<UScaleManagerWindow>(Self);
	window->StretchValueField(bNewStretch);
}
