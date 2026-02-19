#include "Precomp.h"
#include "NModalWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NModalWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ModalWindow", "IsCurrentModal", &NModalWindow::IsCurrentModal, 1501);
	RegisterVMNativeFunc_1("ModalWindow", "SetMouseFocusMode", &NModalWindow::SetMouseFocusMode, 1500);
}

void NModalWindow::IsCurrentModal(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NModalWindow::IsCurrentModal not implemented");
	ReturnValue = false;
}

void NModalWindow::SetMouseFocusMode(UObject* Self, uint8_t newFocusMode)
{
	LogUnimplemented("NModalWindow::SetMouseFocusMode not implemented");
}
