#include "Precomp.h"
#include "NRadioBoxWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NRadioBoxWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("RadioBoxWindow", "GetEnabledToggle", &NRadioBoxWindow::GetEnabledToggle, 1820);
}

void NRadioBoxWindow::GetEnabledToggle(UObject* Self, UObject*& ReturnValue)
{
	URadioBoxWindow* radiobox = UObject::Cast<URadioBoxWindow>(Self);
	ReturnValue = radiobox->GetEnabledToggle();
}
