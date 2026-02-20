#include "Precomp.h"
#include "NEditWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->ClearTextChangedFlag();
}

void NEditWindow::ClearUndo(UObject* Self)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->ClearUndo();
}

void NEditWindow::Copy(UObject* Self)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->Copy();
}

void NEditWindow::Cut(UObject* Self)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->Cut();
}

void NEditWindow::DeleteChar(UObject* Self, BitfieldBool* bBefore, BitfieldBool* bUndo)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->DeleteChar(bBefore, bUndo);
}

void NEditWindow::EnableEditing(UObject* Self, BitfieldBool* bEdit)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->EnableEditing(bEdit);
}

void NEditWindow::EnableSingleLineEditing(UObject* Self, BitfieldBool* bSingle)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->EnableSingleLineEditing(bSingle);
}

void NEditWindow::EnableUppercaseOnly(UObject* Self, BitfieldBool* bUppercase)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->EnableUppercaseOnly(bUppercase);
}

void NEditWindow::GetInsertionPoint(UObject* Self, int& ReturnValue)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	ReturnValue = editwindow->GetInsertionPoint();
}

void NEditWindow::GetSelectedArea(UObject* Self, int& startPos, int& Count)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->GetSelectedArea(startPos, Count);
}

void NEditWindow::HasTextChanged(UObject* Self, BitfieldBool& ReturnValue)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	ReturnValue = editwindow->HasTextChanged();
}

void NEditWindow::InsertText(UObject* Self, std::string* InsertText, BitfieldBool* bUndo, BitfieldBool* bSelect, BitfieldBool& ReturnValue)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	ReturnValue = editwindow->InsertText(InsertText, bUndo, bSelect);
}

void NEditWindow::IsEditingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	ReturnValue = editwindow->IsEditingEnabled();
}

void NEditWindow::IsSingleLineEditingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	ReturnValue = editwindow->IsSingleLineEditingEnabled();
}

void NEditWindow::MoveInsertionPoint(UObject* Self, uint8_t moveInsert, BitfieldBool* bDrag)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->MoveInsertionPoint(moveInsert, bDrag);
}

void NEditWindow::Paste(UObject* Self)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->Paste();
}

void NEditWindow::PlayEditSound(UObject* Self, UObject* PlaySound, float* Volume, float* Pitch)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->PlayEditSound(PlaySound, Volume, Pitch);
}

void NEditWindow::Redo(UObject* Self)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->Redo();
}

void NEditWindow::SetEditCursor(UObject* Self, UObject** newCursor, UObject** newCursorShadow, Color* NewColor)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetEditCursor(newCursor, newCursorShadow, NewColor);
}

void NEditWindow::SetEditSounds(UObject* Self, UObject** typeSound, UObject** deleteSound, UObject** enterSound, UObject** moveSound)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetEditSounds(typeSound, deleteSound, enterSound, moveSound);
}

void NEditWindow::SetInsertionPoint(UObject* Self, int NewPos, BitfieldBool* bDrag)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetInsertionPoint(NewPos, bDrag);
}

void NEditWindow::SetInsertionPointBlinkRate(UObject* Self, float* blinkStart, float* blinkPeriod)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetInsertionPointBlinkRate(blinkStart, blinkPeriod);
}

void NEditWindow::SetInsertionPointTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetInsertionPointTexture(NewTexture, NewColor);
}

void NEditWindow::SetInsertionPointType(UObject* Self, uint8_t newType, float* prefWidth, float* prefHeight)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetInsertionPointType(newType, prefWidth, prefHeight);
}

void NEditWindow::SetMaxSize(UObject* Self, int newMaxSize)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetMaxSize(newMaxSize);
}

void NEditWindow::SetMaxUndos(UObject* Self, int newMaxUndos)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetMaxUndos(newMaxUndos);
}

void NEditWindow::SetSelectedArea(UObject* Self, int startPos, int Count)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetSelectedArea(startPos, Count);
}

void NEditWindow::SetSelectedAreaTextColor(UObject* Self, Color* NewColor)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetSelectedAreaTextColor(NewColor);
}

void NEditWindow::SetSelectedAreaTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetSelectedAreaTexture(NewTexture, NewColor);
}

void NEditWindow::SetTextChangedFlag(UObject* Self, BitfieldBool* bSet)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->SetTextChangedFlag(bSet);
}

void NEditWindow::Undo(UObject* Self)
{
	UEditWindow* editwindow = UObject::Cast<UEditWindow>(Self);
	editwindow->Undo();
}
