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

void NBorderWindow::BaseMarginsFromBorder(UObject* Self, std::optional<bool> bBorder)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->BaseMarginsFromBorder(bBorder);
}

void NBorderWindow::EnableResizing(UObject* Self, std::optional<bool> bResize)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->EnableResizing(bResize);
}

void NBorderWindow::SetBorderMargins(UObject* Self, std::optional<float> NewLeft, std::optional<float> NewRight, std::optional<float> newTop, std::optional<float> newBottom)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->SetBorderMargins(NewLeft, NewRight, newTop, newBottom);
}

void NBorderWindow::SetBorders(UObject* Self, std::optional<UObject*> bordTL, std::optional<UObject*> bordTR, std::optional<UObject*> bordBL, std::optional<UObject*> bordBR, std::optional<UObject*> bordL, std::optional<UObject*> bordR, std::optional<UObject*> bordT, std::optional<UObject*> bordB, std::optional<UObject*> center)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->SetBorders(bordTL, bordTR, bordBL, bordBR, bordL, bordR, bordT, bordB, center);
}

void NBorderWindow::SetMoveCursors(UObject* Self, std::optional<UObject*> Move, std::optional<UObject*> hMove, std::optional<UObject*> vMove, std::optional<UObject*> tlMove, std::optional<UObject*> trMove)
{
	UBorderWindow* border = UObject::Cast<UBorderWindow>(Self);
	border->SetMoveCursors(Move, hMove, vMove, tlMove, trMove);
}
