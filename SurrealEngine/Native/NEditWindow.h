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
	static void DeleteChar(UObject* Self, BitfieldBool* bBefore, BitfieldBool* bUndo);
	static void EnableEditing(UObject* Self, BitfieldBool* bEdit);
	static void EnableSingleLineEditing(UObject* Self, BitfieldBool* bSingle);
	static void EnableUppercaseOnly(UObject* Self, BitfieldBool* bUppercase);
	static void GetInsertionPoint(UObject* Self, int& ReturnValue);
	static void GetSelectedArea(UObject* Self, int& startPos, int& Count);
	static void HasTextChanged(UObject* Self, BitfieldBool& ReturnValue);
	static void InsertText(UObject* Self, std::string* InsertText, BitfieldBool* bUndo, BitfieldBool* bSelect, BitfieldBool& ReturnValue);
	static void IsEditingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsSingleLineEditingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void MoveInsertionPoint(UObject* Self, uint8_t moveInsert, BitfieldBool* bDrag);
	static void Paste(UObject* Self);
	static void PlayEditSound(UObject* Self, UObject* PlaySound, float* Volume, float* Pitch);
	static void Redo(UObject* Self);
	static void SetEditCursor(UObject* Self, UObject** newCursor, UObject** newCursorShadow, Color* NewColor);
	static void SetEditSounds(UObject* Self, UObject** typeSound, UObject** deleteSound, UObject** enterSound, UObject** moveSound);
	static void SetInsertionPoint(UObject* Self, int NewPos, BitfieldBool* bDrag);
	static void SetInsertionPointBlinkRate(UObject* Self, float* blinkStart, float* blinkPeriod);
	static void SetInsertionPointTexture(UObject* Self, UObject** NewTexture, Color* NewColor);
	static void SetInsertionPointType(UObject* Self, uint8_t newType, float* prefWidth, float* prefHeight);
	static void SetMaxSize(UObject* Self, int newMaxSize);
	static void SetMaxUndos(UObject* Self, int newMaxUndos);
	static void SetSelectedArea(UObject* Self, int startPos, int Count);
	static void SetSelectedAreaTextColor(UObject* Self, Color* NewColor);
	static void SetSelectedAreaTexture(UObject* Self, UObject** NewTexture, Color* NewColor);
	static void SetTextChangedFlag(UObject* Self, BitfieldBool* bSet);
	static void Undo(UObject* Self);
};
