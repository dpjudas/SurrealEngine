#pragma once

#include "UObject/UObject.h"

class NEditWindow
{
public:
	static void RegisterFunctions();

	static void ClearTextChangedFlag(UObject* Self);
	static void ClearUndo(UObject* Self);
	static void Copy(UObject* Self);
	static void Cut(UObject* Self);
	static void DeleteChar(UObject* Self, std::optional<bool> bBefore, std::optional<bool> bUndo);
	static void EnableEditing(UObject* Self, std::optional<bool> bEdit);
	static void EnableSingleLineEditing(UObject* Self, std::optional<bool> bSingle);
	static void EnableUppercaseOnly(UObject* Self, std::optional<bool> bUppercase);
	static void GetInsertionPoint(UObject* Self, int& ReturnValue);
	static void GetSelectedArea(UObject* Self, int& startPos, int& Count);
	static void HasTextChanged(UObject* Self, BitfieldBool& ReturnValue);
	static void InsertText(UObject* Self, std::optional<std::string> InsertText, std::optional<bool> bUndo, std::optional<bool> bSelect, BitfieldBool& ReturnValue);
	static void IsEditingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsSingleLineEditingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void MoveInsertionPoint(UObject* Self, uint8_t moveInsert, std::optional<bool> bDrag);
	static void Paste(UObject* Self);
	static void PlayEditSound(UObject* Self, UObject* PlaySound, std::optional<float> Volume, std::optional<float> Pitch);
	static void Redo(UObject* Self);
	static void SetEditCursor(UObject* Self, std::optional<UObject*> newCursor, std::optional<UObject*> newCursorShadow, std::optional<Color> NewColor);
	static void SetEditSounds(UObject* Self, std::optional<UObject*> typeSound, std::optional<UObject*> deleteSound, std::optional<UObject*> enterSound, std::optional<UObject*> moveSound);
	static void SetInsertionPoint(UObject* Self, int NewPos, std::optional<bool> bDrag);
	static void SetInsertionPointBlinkRate(UObject* Self, std::optional<float> blinkStart, std::optional<float> blinkPeriod);
	static void SetInsertionPointTexture(UObject* Self, std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	static void SetInsertionPointType(UObject* Self, uint8_t newType, std::optional<float> prefWidth, std::optional<float> prefHeight);
	static void SetMaxSize(UObject* Self, int newMaxSize);
	static void SetMaxUndos(UObject* Self, int newMaxUndos);
	static void SetSelectedArea(UObject* Self, int startPos, int Count);
	static void SetSelectedAreaTextColor(UObject* Self, std::optional<Color> NewColor);
	static void SetSelectedAreaTexture(UObject* Self, std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	static void SetTextChangedFlag(UObject* Self, std::optional<bool> bSet);
	static void Undo(UObject* Self);
};
