#include "Precomp.h"
#include "NTextLogWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NTextLogWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_2("TextLogWindow", "AddLog", &NTextLogWindow::AddLog, 1570);
	RegisterVMNativeFunc_0("TextLogWindow", "ClearLog", &NTextLogWindow::ClearLog, 1571);
	RegisterVMNativeFunc_1("TextLogWindow", "PauseLog", &NTextLogWindow::PauseLog, 1573);
	RegisterVMNativeFunc_1("TextLogWindow", "SetTextTimeout", &NTextLogWindow::SetTextTimeout, 1572);
}

void NTextLogWindow::AddLog(UObject* Self, const std::string& NewText, const Color& linecol)
{
	LogUnimplemented("TextLogWindow.AddLog");
}

void NTextLogWindow::ClearLog(UObject* Self)
{
	LogUnimplemented("TextLogWindow.ClearLog");
}

void NTextLogWindow::PauseLog(UObject* Self, bool bNewPauseState)
{
	LogUnimplemented("TextLogWindow.PauseLog");
}

void NTextLogWindow::SetTextTimeout(UObject* Self, float newTimeout)
{
	LogUnimplemented("TextLogWindow.SetTextTimeout");
}
