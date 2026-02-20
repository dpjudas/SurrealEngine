#include "Precomp.h"
#include "NModalWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NModalWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ModalWindow", "IsCurrentModal", &NModalWindow::IsCurrentModal, 1501);
	RegisterVMNativeFunc_1("ModalWindow", "SetMouseFocusMode", &NModalWindow::SetMouseFocusMode, 1500);
}

void NModalWindow::IsCurrentModal(UObject* Self, BitfieldBool& ReturnValue)
{
	UModalWindow* modal = UObject::Cast<UModalWindow>(Self);
	ReturnValue = modal->IsCurrentModal();
}

void NModalWindow::SetMouseFocusMode(UObject* Self, uint8_t newFocusMode)
{
	UModalWindow* modal = UObject::Cast<UModalWindow>(Self);
	modal->SetMouseFocusMode(newFocusMode);
}
