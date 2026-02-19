#include "Precomp.h"
#include "NRadioBoxWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NRadioBoxWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("RadioBoxWindow", "GetEnabledToggle", &NRadioBoxWindow::GetEnabledToggle, 1820);
}

void NRadioBoxWindow::GetEnabledToggle(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("RadioBoxWindow.GetEnabledToggle");
	ReturnValue = nullptr;
}
