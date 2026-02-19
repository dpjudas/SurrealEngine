#include "Precomp.h"
#include "NLargeTextWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NLargeTextWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("LargeTextWindow", "SetVerticalSpacing", &NLargeTextWindow::SetVerticalSpacing, 1860);
}

void NLargeTextWindow::SetVerticalSpacing(UObject* Self, float* newVSpace)
{
	LogUnimplemented("NLargeTextWindow::SetVerticalSpacing not implemented");
}
