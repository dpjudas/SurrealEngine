#include "Precomp.h"
#include "NClipWindow.h"
#include "VM/NativeFunc.h"
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
	LogUnimplemented("NClipWindow::EnableSnapToUnits not implemented");
}

void NClipWindow::ForceChildSize(UObject* Self, BitfieldBool* bNewForceChildWidth, BitfieldBool* bNewForceChildHeight)
{
	LogUnimplemented("NClipWindow::ForceChildSize not implemented");
}

void NClipWindow::GetChild(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("NClipWindow::GetChild not implemented");
	ReturnValue = nullptr;
}

void NClipWindow::GetChildPosition(UObject* Self, int& pNewX, int& pNewY)
{
	LogUnimplemented("NClipWindow::GetChildPosition not implemented");
}

void NClipWindow::GetUnitSize(UObject* Self, int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize)
{
	LogUnimplemented("NClipWindow::GetUnitSize not implemented");
}

void NClipWindow::ResetUnitHeight(UObject* Self)
{
	LogUnimplemented("NClipWindow::ResetUnitHeight not implemented");
}

void NClipWindow::ResetUnitSize(UObject* Self)
{
	LogUnimplemented("NClipWindow::ResetUnitSize not implemented");
}

void NClipWindow::ResetUnitWidth(UObject* Self)
{
	LogUnimplemented("NClipWindow::ResetUnitWidth not implemented");
}

void NClipWindow::SetChildPosition(UObject* Self, int newX, int newY)
{
	LogUnimplemented("NClipWindow::SetChildPosition not implemented");
}

void NClipWindow::SetUnitHeight(UObject* Self, int vUnits)
{
	LogUnimplemented("NClipWindow::SetUnitHeight not implemented");
}

void NClipWindow::SetUnitSize(UObject* Self, int hUnits, int vUnits)
{
	LogUnimplemented("NClipWindow::SetUnitSize not implemented");
}

void NClipWindow::SetUnitWidth(UObject* Self, int hUnits)
{
	LogUnimplemented("NClipWindow::SetUnitWidth not implemented");
}
