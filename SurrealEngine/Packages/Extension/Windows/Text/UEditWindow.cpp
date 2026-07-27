
#include "Precomp.h"
#include "UEditWindow.h"
#include "Engine.h"

void UEditWindow::ClearTextChangedFlag()
{
	LogUnimplemented("EditWindow.ClearTextChangedFlag");
}

void UEditWindow::ClearUndo()
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.ClearUndo");
}

void UEditWindow::Copy()
{
	LogUnimplemented("EditWindow.Copy");
}

void UEditWindow::Cut()
{
	LogUnimplemented("EditWindow.Cut");
}

void UEditWindow::DeleteChar(std::optional<bool> bBefore, std::optional<bool> bUndo)
{
	LogUnimplemented("EditWindow.DeleteChar");
}

void UEditWindow::EnableEditing(std::optional<bool> bEdit)
{
	LogUnimplemented("EditWindow.EnableEditing");
}

void UEditWindow::EnableSingleLineEditing(std::optional<bool> bSingle)
{
	LogUnimplemented("EditWindow.EnableSingleLineEditing");
}

void UEditWindow::EnableUppercaseOnly(std::optional<bool> bUppercase)
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.EnableUppercaseOnly");
}

int UEditWindow::GetInsertionPoint()
{

	LogUnimplemented("EditWindow.GetInsertionPoint");
	return 0;
}

void UEditWindow::GetSelectedArea(int& startPos, int& Count)
{
	LogUnimplemented("EditWindow.GetSelectedArea");
}

bool UEditWindow::HasTextChanged()
{
	LogUnimplemented("EditWindow.HasTextChanged");
	return false;
}

bool UEditWindow::InsertText(std::optional<std::string> InsertText, std::optional<bool> bUndo, std::optional<bool> bSelect)
{
	LogUnimplemented("EditWindow.InsertText");
	return false;
}

bool UEditWindow::IsEditingEnabled()
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.IsEditingEnabled");
	return false;
}

bool UEditWindow::IsSingleLineEditingEnabled()
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.IsSingleLineEditingEnabled");
	return false;
}

void UEditWindow::MoveInsertionPoint(uint8_t moveInsert, std::optional<bool> bDrag)
{
	LogUnimplemented("EditWindow.MoveInsertionPoint");
}

void UEditWindow::Paste()
{
	LogUnimplemented("EditWindow.Paste");
}

void UEditWindow::PlayEditSound(UObject* PlaySound, std::optional<float> Volume, std::optional<float> Pitch)
{
	LogUnimplemented("EditWindow.PlayEditSound");
}

void UEditWindow::Redo()
{
	LogUnimplemented("EditWindow.Redo");
}

void UEditWindow::SetEditCursor(std::optional<UObject*> newCursor, std::optional<UObject*> newCursorShadow, std::optional<Color> NewColor)
{
	LogUnimplemented("EditWindow.SetEditCursor");
}

void UEditWindow::SetEditSounds(std::optional<UObject*> typeSound, std::optional<UObject*> deleteSound, std::optional<UObject*> enterSound, std::optional<UObject*> moveSound)
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.SetEditSounds");
}

void UEditWindow::SetInsertionPoint(int NewPos, std::optional<bool> bDrag)
{
	LogUnimplemented("EditWindow.SetInsertionPoint");
}

void UEditWindow::SetInsertionPointBlinkRate(std::optional<float> blinkStart, std::optional<float> blinkPeriod)
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.SetInsertionPointBlinkRate");
}

void UEditWindow::SetInsertionPointTexture(std::optional<UObject*> NewTexture, std::optional<Color> NewColor)
{
	LogUnimplemented("EditWindow.SetInsertionPointTexture");
}

void UEditWindow::SetInsertionPointType(uint8_t newType, std::optional<float> prefWidth, std::optional<float> prefHeight)
{
	LogUnimplemented("EditWindow.SetInsertionPointType");
}

void UEditWindow::SetMaxSize(int newMaxSize)
{
	LogUnimplemented("EditWindow.SetMaxSize");
}

void UEditWindow::SetMaxUndos(int newMaxUndos)
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.SetMaxUndos");
}

void UEditWindow::SetSelectedArea(int startPos, int Count)
{
	LogUnimplemented("EditWindow.SetSelectedArea");
}

void UEditWindow::SetSelectedAreaTextColor(std::optional<Color> NewColor)
{
	LogUnimplemented("EditWindow.SetSelectedAreaTextColor");
}

void UEditWindow::SetSelectedAreaTexture(std::optional<UObject*> NewTexture, std::optional<Color> NewColor)
{
	LogUnimplemented("EditWindow.SetSelectedAreaTexture");
}

void UEditWindow::SetTextChangedFlag(std::optional<bool> bSet)
{
	// UNUSED from scripts.
	LogUnimplemented("EditWindow.SetTextChangedFlag");
}

void UEditWindow::Undo()
{
	LogUnimplemented("EditWindow.Undo");
}
