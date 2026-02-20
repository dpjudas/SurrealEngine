#include "Precomp.h"
#include "NBorderWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->BaseMarginsFromBorder(bBorder);
}

void NBorderWindow::EnableResizing(UObject* Self, BitfieldBool* bResize)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->EnableResizing(bResize);
}

void NBorderWindow::SetBorderMargins(UObject* Self, float* NewLeft, float* NewRight, float* newTop, float* newBottom)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->SetBorderMargins(NewLeft, NewRight, newTop, newBottom);
}

void NBorderWindow::SetBorders(UObject* Self, UObject** bordTL, UObject** bordTR, UObject** bordBL, UObject** bordBR, UObject** bordL, UObject** bordR, UObject** bordT, UObject** bordB, UObject** center)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->SetBorders(bordTL, bordTR, bordBL, bordBR, bordL, bordR, bordT, bordB, center);
}

void NBorderWindow::SetMoveCursors(UObject* Self, UObject** Move, UObject** hMove, UObject** vMove, UObject** tlMove, UObject** trMove)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->SetMoveCursors(Move, hMove, vMove, tlMove, trMove);
}
