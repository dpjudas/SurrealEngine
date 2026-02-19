#include "Precomp.h"
#include "NScrollAreaWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NScrollAreaWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ScrollAreaWindow", "AutoHideScrollbars", &NScrollAreaWindow::AutoHideScrollbars, 1703);
	RegisterVMNativeFunc_2("ScrollAreaWindow", "EnableScrolling", &NScrollAreaWindow::EnableScrolling, 1700);
	RegisterVMNativeFunc_2("ScrollAreaWindow", "SetAreaMargins", &NScrollAreaWindow::SetAreaMargins, 1702);
	RegisterVMNativeFunc_1("ScrollAreaWindow", "SetScrollbarDistance", &NScrollAreaWindow::SetScrollbarDistance, 1701);
}

void NScrollAreaWindow::AutoHideScrollbars(UObject* Self, BitfieldBool* bHide)
{
	LogUnimplemented("ScrollAreaWindow.AutoHideScrollbars");
}

void NScrollAreaWindow::EnableScrolling(UObject* Self, BitfieldBool* bHScrolling, BitfieldBool* bVScrolling)
{
	LogUnimplemented("ScrollAreaWindow.EnableScrolling");
}

void NScrollAreaWindow::SetAreaMargins(UObject* Self, float newMarginWidth, float newMarginHeight)
{
	LogUnimplemented("ScrollAreaWindow.SetAreaMargins");
}

void NScrollAreaWindow::SetScrollbarDistance(UObject* Self, float newDistance)
{
	LogUnimplemented("ScrollAreaWindow.SetScrollbarDistance");
}
