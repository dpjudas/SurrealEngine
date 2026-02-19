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
	LogUnimplemented("ClipWindow.EnableSnapToUnits");
}

void NClipWindow::ForceChildSize(UObject* Self, BitfieldBool* bNewForceChildWidth, BitfieldBool* bNewForceChildHeight)
{
	LogUnimplemented("ClipWindow.ForceChildSize");
}

void NClipWindow::GetChild(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("ClipWindow.GetChild");
	ReturnValue = nullptr;
}

void NClipWindow::GetChildPosition(UObject* Self, int& pNewX, int& pNewY)
{
	LogUnimplemented("ClipWindow.GetChildPosition");
}

void NClipWindow::GetUnitSize(UObject* Self, int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize)
{
	LogUnimplemented("ClipWindow.GetUnitSize");
}

void NClipWindow::ResetUnitHeight(UObject* Self)
{
	LogUnimplemented("ClipWindow.ResetUnitHeight");
}

void NClipWindow::ResetUnitSize(UObject* Self)
{
	LogUnimplemented("ClipWindow.ResetUnitSize");
}

void NClipWindow::ResetUnitWidth(UObject* Self)
{
	LogUnimplemented("ClipWindow.ResetUnitWidth");
}

void NClipWindow::SetChildPosition(UObject* Self, int newX, int newY)
{
	LogUnimplemented("ClipWindow.SetChildPosition");
}

void NClipWindow::SetUnitHeight(UObject* Self, int vUnits)
{
	LogUnimplemented("ClipWindow.SetUnitHeight");
}

void NClipWindow::SetUnitSize(UObject* Self, int hUnits, int vUnits)
{
	LogUnimplemented("ClipWindow.SetUnitSize");
}

void NClipWindow::SetUnitWidth(UObject* Self, int hUnits)
{
	LogUnimplemented("ClipWindow.SetUnitWidth");
}
