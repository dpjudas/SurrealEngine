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
	LogUnimplemented("NEditWindow::ClearTextChangedFlag not implemented");
}

void NEditWindow::ClearUndo(UObject* Self)
{
	LogUnimplemented("NEditWindow::ClearUndo not implemented");
}

void NEditWindow::Copy(UObject* Self)
{
	LogUnimplemented("NEditWindow::Copy not implemented");
}

void NEditWindow::Cut(UObject* Self)
{
	LogUnimplemented("NEditWindow::Cut not implemented");
}

void NEditWindow::DeleteChar(UObject* Self, BitfieldBool* bBefore, BitfieldBool* bUndo)
{
	LogUnimplemented("NEditWindow::DeleteChar not implemented");
}

void NEditWindow::EnableEditing(UObject* Self, BitfieldBool* bEdit)
{
	LogUnimplemented("NEditWindow::EnableEditing not implemented");
}

void NEditWindow::EnableSingleLineEditing(UObject* Self, BitfieldBool* bSingle)
{
	LogUnimplemented("NEditWindow::EnableSingleLineEditing not implemented");
}

void NEditWindow::EnableUppercaseOnly(UObject* Self, BitfieldBool* bUppercase)
{
	LogUnimplemented("NEditWindow::EnableUppercaseOnly not implemented");
}

void NEditWindow::GetInsertionPoint(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NEditWindow::GetInsertionPoint not implemented");
	ReturnValue = 0;
}

void NEditWindow::GetSelectedArea(UObject* Self, int& startPos, int& Count)
{
	LogUnimplemented("NEditWindow::GetSelectedArea not implemented");
}

void NEditWindow::HasTextChanged(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NEditWindow::HasTextChanged not implemented");
	ReturnValue = false;
}

void NEditWindow::InsertText(UObject* Self, std::string* InsertText, BitfieldBool* bUndo, BitfieldBool* bSelect, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NEditWindow::InsertText not implemented");
	ReturnValue = false;
}

void NEditWindow::IsEditingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NEditWindow::IsEditingEnabled not implemented");
	ReturnValue = false;
}

void NEditWindow::IsSingleLineEditingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NEditWindow::IsSingleLineEditingEnabled not implemented");
	ReturnValue = false;
}

void NEditWindow::MoveInsertionPoint(UObject* Self, uint8_t moveInsert, BitfieldBool* bDrag)
{
	LogUnimplemented("NEditWindow::MoveInsertionPoint not implemented");
}

void NEditWindow::Paste(UObject* Self)
{
	LogUnimplemented("NEditWindow::Paste not implemented");
}

void NEditWindow::PlayEditSound(UObject* Self, UObject* PlaySound, float* Volume, float* Pitch)
{
	LogUnimplemented("NEditWindow::PlayEditSound not implemented");
}

void NEditWindow::Redo(UObject* Self)
{
	LogUnimplemented("NEditWindow::Redo not implemented");
}

void NEditWindow::SetEditCursor(UObject* Self, UObject** newCursor, UObject** newCursorShadow, Color* NewColor)
{
	LogUnimplemented("NEditWindow::SetEditCursor not implemented");
}

void NEditWindow::SetEditSounds(UObject* Self, UObject** typeSound, UObject** deleteSound, UObject** enterSound, UObject** moveSound)
{
	LogUnimplemented("NEditWindow::SetEditSounds not implemented");
}

void NEditWindow::SetInsertionPoint(UObject* Self, int NewPos, BitfieldBool* bDrag)
{
	LogUnimplemented("NEditWindow::SetInsertionPoint not implemented");
}

void NEditWindow::SetInsertionPointBlinkRate(UObject* Self, float* blinkStart, float* blinkPeriod)
{
	LogUnimplemented("NEditWindow::SetInsertionPointBlinkRate not implemented");
}

void NEditWindow::SetInsertionPointTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("NEditWindow::SetInsertionPointTexture not implemented");
}

void NEditWindow::SetInsertionPointType(UObject* Self, uint8_t newType, float* prefWidth, float* prefHeight)
{
	LogUnimplemented("NEditWindow::SetInsertionPointType not implemented");
}

void NEditWindow::SetMaxSize(UObject* Self, int newMaxSize)
{
	LogUnimplemented("NEditWindow::SetMaxSize not implemented");
}

void NEditWindow::SetMaxUndos(UObject* Self, int newMaxUndos)
{
	LogUnimplemented("NEditWindow::SetMaxUndos not implemented");
}

void NEditWindow::SetSelectedArea(UObject* Self, int startPos, int Count)
{
	LogUnimplemented("NEditWindow::SetSelectedArea not implemented");
}

void NEditWindow::SetSelectedAreaTextColor(UObject* Self, Color* NewColor)
{
	LogUnimplemented("NEditWindow::SetSelectedAreaTextColor not implemented");
}

void NEditWindow::SetSelectedAreaTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("NEditWindow::SetSelectedAreaTexture not implemented");
}

void NEditWindow::SetTextChangedFlag(UObject* Self, BitfieldBool* bSet)
{
	LogUnimplemented("NEditWindow::SetTextChangedFlag not implemented");
}

void NEditWindow::Undo(UObject* Self)
{
	LogUnimplemented("NEditWindow::Undo not implemented");
}
