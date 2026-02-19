#include "Precomp.h"
#include "NBorderWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NBorderWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("BorderWindow", "BaseMarginsFromBorder", &NBorderWindow::BaseMarginsFromBorder, 1532);
	RegisterVMNativeFunc_1("BorderWindow", "EnableResizing", &NBorderWindow::EnableResizing, 1533);
	RegisterVMNativeFunc_4("BorderWindow", "SetBorderMargins", &NBorderWindow::SetBorderMargins, 1531);
	RegisterVMNativeFunc_9("BorderWindow", "SetBorders", &NBorderWindow::SetBorders, 1530);
	RegisterVMNativeFunc_5("BorderWindow", "SetMoveCursors", &NBorderWindow::SetMoveCursors, 1534);
}

void NBorderWindow::BaseMarginsFromBorder(UObject* Self, BitfieldBool* bBorder)
{
	Exception::Throw("NBorderWindow::BaseMarginsFromBorder not implemented");
}

void NBorderWindow::EnableResizing(UObject* Self, BitfieldBool* bResize)
{
	Exception::Throw("NBorderWindow::EnableResizing not implemented");
}

void NBorderWindow::SetBorderMargins(UObject* Self, float* NewLeft, float* NewRight, float* newTop, float* newBottom)
{
	Exception::Throw("NBorderWindow::SetBorderMargins not implemented");
}

void NBorderWindow::SetBorders(UObject* Self, UObject** bordTL, UObject** bordTR, UObject** bordBL, UObject** bordBR, UObject** bordL, UObject** bordR, UObject** bordT, UObject** bordB, UObject** center)
{
	Exception::Throw("NBorderWindow::SetBorders not implemented");
}

void NBorderWindow::SetMoveCursors(UObject* Self, UObject** Move, UObject** hMove, UObject** vMove, UObject** tlMove, UObject** trMove)
{
	Exception::Throw("NBorderWindow::SetMoveCursors not implemented");
}
