#include "Precomp.h"
#include "NLargeTextWindow.h"
#include "VM/NativeFunc.h"
#include "Packages/Extension/Windows/Text/ULargeTextWindow.h"

void NLargeTextWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("LargeTextWindow", "SetVerticalSpacing", &NLargeTextWindow::SetVerticalSpacing, 1860);
}

void NLargeTextWindow::SetVerticalSpacing(UObject* Self, std::optional<float> newVSpace)
{
	ULargeTextWindow* textwindow = UObject::Cast<ULargeTextWindow>(Self);
	textwindow->SetVerticalSpacing(newVSpace);
}
