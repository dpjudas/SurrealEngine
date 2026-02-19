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
	Exception::Throw("NModalWindow::IsCurrentModal not implemented");
}

void NModalWindow::SetMouseFocusMode(UObject* Self, uint8_t newFocusMode)
{
	Exception::Throw("NModalWindow::SetMouseFocusMode not implemented");
}
