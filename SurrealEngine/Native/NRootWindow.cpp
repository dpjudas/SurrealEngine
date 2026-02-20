#include "Precomp.h"
#include "NRootWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NRootWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("RootWindow", "EnablePositionalSound", &NRootWindow::EnablePositionalSound, 1519);
	RegisterVMNativeFunc_1("RootWindow", "EnableRendering", &NRootWindow::EnableRendering, 1512);
	RegisterVMNativeFunc_2("RootWindow", "GenerateSnapshot", &NRootWindow::GenerateSnapshot, 1524);
	RegisterVMNativeFunc_1("RootWindow", "IsPositionalSoundEnabled", &NRootWindow::IsPositionalSoundEnabled, 1520);
	RegisterVMNativeFunc_1("RootWindow", "IsRenderingEnabled", &NRootWindow::IsRenderingEnabled, 1513);
	RegisterVMNativeFunc_2("RootWindow", "LockMouse", &NRootWindow::LockMouse, 1521);
	RegisterVMNativeFunc_0("RootWindow", "ResetRenderViewport", &NRootWindow::ResetRenderViewport, 1515);
	RegisterVMNativeFunc_1("RootWindow", "SetDefaultEditCursor", &NRootWindow::SetDefaultEditCursor, 1510);
	RegisterVMNativeFunc_5("RootWindow", "SetDefaultMovementCursors", &NRootWindow::SetDefaultMovementCursors, 1511);
	RegisterVMNativeFunc_2("RootWindow", "SetRawBackground", &NRootWindow::SetRawBackground, 1516);
	RegisterVMNativeFunc_2("RootWindow", "SetRawBackgroundSize", &NRootWindow::SetRawBackgroundSize, 1517);
	RegisterVMNativeFunc_4("RootWindow", "SetRenderViewport", &NRootWindow::SetRenderViewport, 1514);
	RegisterVMNativeFunc_2("RootWindow", "SetSnapshotSize", &NRootWindow::SetSnapshotSize, 1523);
	RegisterVMNativeFunc_1("RootWindow", "ShowCursor", &NRootWindow::ShowCursor, 1522);
	RegisterVMNativeFunc_1("RootWindow", "StretchRawBackground", &NRootWindow::StretchRawBackground, 1518);
}

void NRootWindow::EnablePositionalSound(UObject* Self, BitfieldBool* bEnable)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->EnablePositionalSound(bEnable);
}

void NRootWindow::EnableRendering(UObject* Self, BitfieldBool* bRender)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->EnableRendering(bRender);
}

void NRootWindow::GenerateSnapshot(UObject* Self, BitfieldBool* bFilter, UObject*& ReturnValue)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	ReturnValue = root->GenerateSnapshot(bFilter);
}

void NRootWindow::IsPositionalSoundEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	ReturnValue = root->IsPositionalSoundEnabled();
}

void NRootWindow::IsRenderingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	ReturnValue = root->IsRenderingEnabled();
}

void NRootWindow::LockMouse(UObject* Self, BitfieldBool* bLockMove, BitfieldBool* bLockButton)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->LockMouse(bLockMove, bLockButton);
}

void NRootWindow::ResetRenderViewport(UObject* Self)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->ResetRenderViewport();
}

void NRootWindow::SetDefaultEditCursor(UObject* Self, UObject** newEditCursor)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->SetDefaultEditCursor(newEditCursor);
}

void NRootWindow::SetDefaultMovementCursors(UObject* Self, UObject** newMovementCursor, UObject** newHorizontalMovementCursor, UObject** newVerticalMovementCursor, UObject** newTopLeftMovementCursor, UObject** newTopRightMovementCursor)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->SetDefaultMovementCursors(newMovementCursor, newHorizontalMovementCursor, newVerticalMovementCursor, newTopLeftMovementCursor, newTopRightMovementCursor);
}

void NRootWindow::SetRawBackground(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->SetRawBackground(NewTexture, NewColor);
}

void NRootWindow::SetRawBackgroundSize(UObject* Self, float newWidth, float NewHeight)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->SetRawBackgroundSize(newWidth, NewHeight);
}

void NRootWindow::SetRenderViewport(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->SetRenderViewport(newX, newY, newWidth, NewHeight);
}

void NRootWindow::SetSnapshotSize(UObject* Self, float newWidth, float NewHeight)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->SetSnapshotSize(newWidth, NewHeight);
}

void NRootWindow::ShowCursor(UObject* Self, BitfieldBool* bShow)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->ShowCursor(bShow);
}

void NRootWindow::StretchRawBackground(UObject* Self, BitfieldBool* bStretch)
{
	URootWindow* root = UObject::Cast<URootWindow>(Self);
	root->StretchRawBackground(bStretch);
}
