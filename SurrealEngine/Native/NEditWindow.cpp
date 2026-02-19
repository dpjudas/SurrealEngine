#include "Precomp.h"
#include "NEditWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NEditWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_0("EditWindow", "ClearTextChangedFlag", &NEditWindow::ClearTextChangedFlag, 1890);
	RegisterVMNativeFunc_0("EditWindow", "ClearUndo", &NEditWindow::ClearUndo, 1912);
	RegisterVMNativeFunc_0("EditWindow", "Copy", &NEditWindow::Copy, 1915);
	RegisterVMNativeFunc_0("EditWindow", "Cut", &NEditWindow::Cut, 1916);
	RegisterVMNativeFunc_2("EditWindow", "DeleteChar", &NEditWindow::DeleteChar, 1901);
	RegisterVMNativeFunc_1("EditWindow", "EnableEditing", &NEditWindow::EnableEditing, 1885);
	RegisterVMNativeFunc_1("EditWindow", "EnableSingleLineEditing", &NEditWindow::EnableSingleLineEditing, 1887);
	RegisterVMNativeFunc_1("EditWindow", "EnableUppercaseOnly", &NEditWindow::EnableUppercaseOnly, 1889);
	RegisterVMNativeFunc_1("EditWindow", "GetInsertionPoint", &NEditWindow::GetInsertionPoint, 1882);
	RegisterVMNativeFunc_2("EditWindow", "GetSelectedArea", &NEditWindow::GetSelectedArea, 1884);
	RegisterVMNativeFunc_1("EditWindow", "HasTextChanged", &NEditWindow::HasTextChanged, 1892);
	RegisterVMNativeFunc_4("EditWindow", "InsertText", &NEditWindow::InsertText, 1900);
	RegisterVMNativeFunc_1("EditWindow", "IsEditingEnabled", &NEditWindow::IsEditingEnabled, 1886);
	RegisterVMNativeFunc_1("EditWindow", "IsSingleLineEditingEnabled", &NEditWindow::IsSingleLineEditingEnabled, 1888);
	RegisterVMNativeFunc_2("EditWindow", "MoveInsertionPoint", &NEditWindow::MoveInsertionPoint, 1880);
	RegisterVMNativeFunc_0("EditWindow", "Paste", &NEditWindow::Paste, 1917);
	RegisterVMNativeFunc_3("EditWindow", "PlayEditSound", &NEditWindow::PlayEditSound, 1921);
	RegisterVMNativeFunc_0("EditWindow", "Redo", &NEditWindow::Redo, 1911);
	RegisterVMNativeFunc_3("EditWindow", "SetEditCursor", &NEditWindow::SetEditCursor, 1909);
	RegisterVMNativeFunc_4("EditWindow", "SetEditSounds", &NEditWindow::SetEditSounds, 1920);
	RegisterVMNativeFunc_2("EditWindow", "SetInsertionPoint", &NEditWindow::SetInsertionPoint, 1881);
	RegisterVMNativeFunc_2("EditWindow", "SetInsertionPointBlinkRate", &NEditWindow::SetInsertionPointBlinkRate, 1902);
	RegisterVMNativeFunc_2("EditWindow", "SetInsertionPointTexture", &NEditWindow::SetInsertionPointTexture, 1905);
	RegisterVMNativeFunc_3("EditWindow", "SetInsertionPointType", &NEditWindow::SetInsertionPointType, 1906);
	RegisterVMNativeFunc_1("EditWindow", "SetMaxSize", &NEditWindow::SetMaxSize, 1895);
	RegisterVMNativeFunc_1("EditWindow", "SetMaxUndos", &NEditWindow::SetMaxUndos, 1896);
	RegisterVMNativeFunc_2("EditWindow", "SetSelectedArea", &NEditWindow::SetSelectedArea, 1883);
	RegisterVMNativeFunc_1("EditWindow", "SetSelectedAreaTextColor", &NEditWindow::SetSelectedAreaTextColor, 1908);
	RegisterVMNativeFunc_2("EditWindow", "SetSelectedAreaTexture", &NEditWindow::SetSelectedAreaTexture, 1907);
	RegisterVMNativeFunc_1("EditWindow", "SetTextChangedFlag", &NEditWindow::SetTextChangedFlag, 1891);
	RegisterVMNativeFunc_0("EditWindow", "Undo", &NEditWindow::Undo, 1910);
}

void NEditWindow::ClearTextChangedFlag(UObject* Self)
{
	Exception::Throw("NEditWindow::ClearTextChangedFlag not implemented");
}

void NEditWindow::ClearUndo(UObject* Self)
{
	Exception::Throw("NEditWindow::ClearUndo not implemented");
}

void NEditWindow::Copy(UObject* Self)
{
	Exception::Throw("NEditWindow::Copy not implemented");
}

void NEditWindow::Cut(UObject* Self)
{
	Exception::Throw("NEditWindow::Cut not implemented");
}

void NEditWindow::DeleteChar(UObject* Self, BitfieldBool* bBefore, BitfieldBool* bUndo)
{
	Exception::Throw("NEditWindow::DeleteChar not implemented");
}

void NEditWindow::EnableEditing(UObject* Self, BitfieldBool* bEdit)
{
	Exception::Throw("NEditWindow::EnableEditing not implemented");
}

void NEditWindow::EnableSingleLineEditing(UObject* Self, BitfieldBool* bSingle)
{
	Exception::Throw("NEditWindow::EnableSingleLineEditing not implemented");
}

void NEditWindow::EnableUppercaseOnly(UObject* Self, BitfieldBool* bUppercase)
{
	Exception::Throw("NEditWindow::EnableUppercaseOnly not implemented");
}

void NEditWindow::GetInsertionPoint(UObject* Self, int& ReturnValue)
{
	Exception::Throw("NEditWindow::GetInsertionPoint not implemented");
}

void NEditWindow::GetSelectedArea(UObject* Self, int& startPos, int& Count)
{
	Exception::Throw("NEditWindow::GetSelectedArea not implemented");
}

void NEditWindow::HasTextChanged(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NEditWindow::HasTextChanged not implemented");
}

void NEditWindow::InsertText(UObject* Self, std::string* InsertText, BitfieldBool* bUndo, BitfieldBool* bSelect, BitfieldBool& ReturnValue)
{
	Exception::Throw("NEditWindow::InsertText not implemented");
}

void NEditWindow::IsEditingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NEditWindow::IsEditingEnabled not implemented");
}

void NEditWindow::IsSingleLineEditingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NEditWindow::IsSingleLineEditingEnabled not implemented");
}

void NEditWindow::MoveInsertionPoint(UObject* Self, uint8_t moveInsert, BitfieldBool* bDrag)
{
	Exception::Throw("NEditWindow::MoveInsertionPoint not implemented");
}

void NEditWindow::Paste(UObject* Self)
{
	Exception::Throw("NEditWindow::Paste not implemented");
}

void NEditWindow::PlayEditSound(UObject* Self, UObject* PlaySound, float* Volume, float* Pitch)
{
	Exception::Throw("NEditWindow::PlayEditSound not implemented");
}

void NEditWindow::Redo(UObject* Self)
{
	Exception::Throw("NEditWindow::Redo not implemented");
}

void NEditWindow::SetEditCursor(UObject* Self, UObject** newCursor, UObject** newCursorShadow, Color* NewColor)
{
	Exception::Throw("NEditWindow::SetEditCursor not implemented");
}

void NEditWindow::SetEditSounds(UObject* Self, UObject** typeSound, UObject** deleteSound, UObject** enterSound, UObject** moveSound)
{
	Exception::Throw("NEditWindow::SetEditSounds not implemented");
}

void NEditWindow::SetInsertionPoint(UObject* Self, int NewPos, BitfieldBool* bDrag)
{
	Exception::Throw("NEditWindow::SetInsertionPoint not implemented");
}

void NEditWindow::SetInsertionPointBlinkRate(UObject* Self, float* blinkStart, float* blinkPeriod)
{
	Exception::Throw("NEditWindow::SetInsertionPointBlinkRate not implemented");
}

void NEditWindow::SetInsertionPointTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	Exception::Throw("NEditWindow::SetInsertionPointTexture not implemented");
}

void NEditWindow::SetInsertionPointType(UObject* Self, uint8_t newType, float* prefWidth, float* prefHeight)
{
	Exception::Throw("NEditWindow::SetInsertionPointType not implemented");
}

void NEditWindow::SetMaxSize(UObject* Self, int newMaxSize)
{
	Exception::Throw("NEditWindow::SetMaxSize not implemented");
}

void NEditWindow::SetMaxUndos(UObject* Self, int newMaxUndos)
{
	Exception::Throw("NEditWindow::SetMaxUndos not implemented");
}

void NEditWindow::SetSelectedArea(UObject* Self, int startPos, int Count)
{
	Exception::Throw("NEditWindow::SetSelectedArea not implemented");
}

void NEditWindow::SetSelectedAreaTextColor(UObject* Self, Color* NewColor)
{
	Exception::Throw("NEditWindow::SetSelectedAreaTextColor not implemented");
}

void NEditWindow::SetSelectedAreaTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	Exception::Throw("NEditWindow::SetSelectedAreaTexture not implemented");
}

void NEditWindow::SetTextChangedFlag(UObject* Self, BitfieldBool* bSet)
{
	Exception::Throw("NEditWindow::SetTextChangedFlag not implemented");
}

void NEditWindow::Undo(UObject* Self)
{
	Exception::Throw("NEditWindow::Undo not implemented");
}
