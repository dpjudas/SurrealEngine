#include "Precomp.h"
#include "NTileWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NTileWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("TileWindow", "EnableWrapping", &NTileWindow::EnableWrapping, 1542);
	RegisterVMNativeFunc_1("TileWindow", "FillParent", &NTileWindow::FillParent, 1543);
	RegisterVMNativeFunc_1("TileWindow", "MakeHeightsEqual", &NTileWindow::MakeHeightsEqual, 1545);
	RegisterVMNativeFunc_1("TileWindow", "MakeWidthsEqual", &NTileWindow::MakeWidthsEqual, 1544);
	RegisterVMNativeFunc_2("TileWindow", "SetChildAlignments", &NTileWindow::SetChildAlignments, 1541);
	RegisterVMNativeFunc_2("TileWindow", "SetDirections", &NTileWindow::SetDirections, 1537);
	RegisterVMNativeFunc_1("TileWindow", "SetMajorSpacing", &NTileWindow::SetMajorSpacing, 1540);
	RegisterVMNativeFunc_2("TileWindow", "SetMargins", &NTileWindow::SetMargins, 1535);
	RegisterVMNativeFunc_1("TileWindow", "SetMinorSpacing", &NTileWindow::SetMinorSpacing, 1539);
	RegisterVMNativeFunc_1("TileWindow", "SetOrder", &NTileWindow::SetOrder, 1538);
	RegisterVMNativeFunc_1("TileWindow", "SetOrientation", &NTileWindow::SetOrientation, 1536);
}

void NTileWindow::EnableWrapping(UObject* Self, bool bWrapOn)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->EnableWrapping(bWrapOn);
}

void NTileWindow::FillParent(UObject* Self, bool FillParent)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->FillParent(FillParent);
}

void NTileWindow::MakeHeightsEqual(UObject* Self, bool bEqual)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->MakeHeightsEqual(bEqual);
}

void NTileWindow::MakeWidthsEqual(UObject* Self, bool bEqual)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->MakeWidthsEqual(bEqual);
}

void NTileWindow::SetChildAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetChildAlignments(newHAlign, newVAlign);
}

void NTileWindow::SetDirections(UObject* Self, uint8_t newHDir, uint8_t newVDir)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetDirections(newHDir, newVDir);
}

void NTileWindow::SetMajorSpacing(UObject* Self, float newSpacing)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetMajorSpacing(newSpacing);
}

void NTileWindow::SetMargins(UObject* Self, float newHMargin, float newVMargin)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetMargins(newHMargin, newVMargin);
}

void NTileWindow::SetMinorSpacing(UObject* Self, float newSpacing)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetMinorSpacing(newSpacing);
}

void NTileWindow::SetOrder(UObject* Self, uint8_t newOrder)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetOrder(newOrder);
}

void NTileWindow::SetOrientation(UObject* Self, uint8_t newOrientation)
{
	UTileWindow* tile = UObject::Cast<UTileWindow>(Self);
	tile->SetOrientation(newOrientation);
}
