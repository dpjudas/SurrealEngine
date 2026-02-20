#include "Precomp.h"
#include "NClipWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NClipWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ClipWindow", "EnableSnapToUnits", &NClipWindow::EnableSnapToUnits, 1690);
	RegisterVMNativeFunc_2("ClipWindow", "ForceChildSize", &NClipWindow::ForceChildSize, 1689);
	RegisterVMNativeFunc_1("ClipWindow", "GetChild", &NClipWindow::GetChild, 1691);
	RegisterVMNativeFunc_2("ClipWindow", "GetChildPosition", &NClipWindow::GetChildPosition, 1681);
	RegisterVMNativeFunc_4("ClipWindow", "GetUnitSize", &NClipWindow::GetUnitSize, 1688);
	RegisterVMNativeFunc_0("ClipWindow", "ResetUnitHeight", &NClipWindow::ResetUnitHeight, 1687);
	RegisterVMNativeFunc_0("ClipWindow", "ResetUnitSize", &NClipWindow::ResetUnitSize, 1685);
	RegisterVMNativeFunc_0("ClipWindow", "ResetUnitWidth", &NClipWindow::ResetUnitWidth, 1686);
	RegisterVMNativeFunc_2("ClipWindow", "SetChildPosition", &NClipWindow::SetChildPosition, 1680);
	RegisterVMNativeFunc_1("ClipWindow", "SetUnitHeight", &NClipWindow::SetUnitHeight, 1684);
	RegisterVMNativeFunc_2("ClipWindow", "SetUnitSize", &NClipWindow::SetUnitSize, 1682);
	RegisterVMNativeFunc_1("ClipWindow", "SetUnitWidth", &NClipWindow::SetUnitWidth, 1683);
}

void NClipWindow::EnableSnapToUnits(UObject* Self, BitfieldBool* bNewSnapToUnits)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->EnableSnapToUnits(bNewSnapToUnits);
}

void NClipWindow::ForceChildSize(UObject* Self, BitfieldBool* bNewForceChildWidth, BitfieldBool* bNewForceChildHeight)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->ForceChildSize(bNewForceChildWidth, bNewForceChildHeight);
}

void NClipWindow::GetChild(UObject* Self, UObject*& ReturnValue)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	ReturnValue = clip->GetChild();
}

void NClipWindow::GetChildPosition(UObject* Self, int& pNewX, int& pNewY)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->GetChildPosition(pNewX, pNewY);
}

void NClipWindow::GetUnitSize(UObject* Self, int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->GetUnitSize(pAreaHSize, pAreaVSize, pChildHSize, childVSize);
}

void NClipWindow::ResetUnitHeight(UObject* Self)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->ResetUnitHeight();
}

void NClipWindow::ResetUnitSize(UObject* Self)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->ResetUnitSize();
}

void NClipWindow::ResetUnitWidth(UObject* Self)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->ResetUnitWidth();
}

void NClipWindow::SetChildPosition(UObject* Self, int newX, int newY)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->SetChildPosition(newX, newY);
}

void NClipWindow::SetUnitHeight(UObject* Self, int vUnits)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->SetUnitHeight(vUnits);
}

void NClipWindow::SetUnitSize(UObject* Self, int hUnits, int vUnits)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->SetUnitSize(hUnits, vUnits);
}

void NClipWindow::SetUnitWidth(UObject* Self, int hUnits)
{
	UClipWindow* clip = UObject::Cast<UClipWindow>(Self);
	clip->SetUnitWidth(hUnits);
}
