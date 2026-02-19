#include "Precomp.h"
#include "NRootWindow.h"
#include "VM/NativeFunc.h"
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
	Exception::Throw("NRootWindow::EnablePositionalSound not implemented");
}

void NRootWindow::EnableRendering(UObject* Self, BitfieldBool* bRender)
{
	Exception::Throw("NRootWindow::EnableRendering not implemented");
}

void NRootWindow::GenerateSnapshot(UObject* Self, BitfieldBool* bFilter, UObject*& ReturnValue)
{
	Exception::Throw("NRootWindow::GenerateSnapshot not implemented");
}

void NRootWindow::IsPositionalSoundEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NRootWindow::IsPositionalSoundEnabled not implemented");
}

void NRootWindow::IsRenderingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NRootWindow::IsRenderingEnabled not implemented");
}

void NRootWindow::LockMouse(UObject* Self, BitfieldBool* bLockMove, BitfieldBool* bLockButton)
{
	Exception::Throw("NRootWindow::LockMouse not implemented");
}

void NRootWindow::ResetRenderViewport(UObject* Self)
{
	Exception::Throw("NRootWindow::ResetRenderViewport not implemented");
}

void NRootWindow::SetDefaultEditCursor(UObject* Self, UObject** newEditCursor)
{
	Exception::Throw("NRootWindow::SetDefaultEditCursor not implemented");
}

void NRootWindow::SetDefaultMovementCursors(UObject* Self, UObject** newMovementCursor, UObject** newHorizontalMovementCursor, UObject** newVerticalMovementCursor, UObject** newTopLeftMovementCursor, UObject** newTopRightMovementCursor)
{
	Exception::Throw("NRootWindow::SetDefaultMovementCursors not implemented");
}

void NRootWindow::SetRawBackground(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	Exception::Throw("NRootWindow::SetRawBackground not implemented");
}

void NRootWindow::SetRawBackgroundSize(UObject* Self, float newWidth, float NewHeight)
{
	Exception::Throw("NRootWindow::SetRawBackgroundSize not implemented");
}

void NRootWindow::SetRenderViewport(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	Exception::Throw("NRootWindow::SetRenderViewport not implemented");
}

void NRootWindow::SetSnapshotSize(UObject* Self, float newWidth, float NewHeight)
{
	Exception::Throw("NRootWindow::SetSnapshotSize not implemented");
}

void NRootWindow::ShowCursor(UObject* Self, BitfieldBool* bShow)
{
	Exception::Throw("NRootWindow::ShowCursor not implemented");
}

void NRootWindow::StretchRawBackground(UObject* Self, BitfieldBool* bStretch)
{
	Exception::Throw("NRootWindow::StretchRawBackground not implemented");
}
