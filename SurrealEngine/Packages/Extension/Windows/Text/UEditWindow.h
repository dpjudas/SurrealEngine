#pragma once

#include "ULargeTextWindow.h"

class UEditWindow : public ULargeTextWindow
{
public:
	using ULargeTextWindow::ULargeTextWindow;

	void ClearTextChangedFlag();
	void ClearUndo();
	void Copy();
	void Cut();
	void DeleteChar(std::optional<bool> bBefore, std::optional<bool> bUndo);
	void EnableEditing(std::optional<bool> bEdit);
	void EnableSingleLineEditing(std::optional<bool> bSingle);
	void EnableUppercaseOnly(std::optional<bool> bUppercase);
	int GetInsertionPoint();
	void GetSelectedArea(int& startPos, int& Count);
	bool HasTextChanged();
	bool InsertText(std::optional<std::string> InsertText, std::optional<bool> bUndo, std::optional<bool> bSelect);
	bool IsEditingEnabled();
	bool IsSingleLineEditingEnabled();
	void MoveInsertionPoint(uint8_t moveInsert, std::optional<bool> bDrag);
	void Paste();
	void PlayEditSound(UObject* PlaySound, std::optional<float> Volume, std::optional<float> Pitch);
	void Redo();
	void SetEditCursor(std::optional<UObject*> newCursor, std::optional<UObject*> newCursorShadow, std::optional<Color> NewColor);
	void SetEditSounds(std::optional<UObject*> typeSound, std::optional<UObject*> deleteSound, std::optional<UObject*> enterSound, std::optional<UObject*> moveSound);
	void SetInsertionPoint(int NewPos, std::optional<bool> bDrag);
	void SetInsertionPointBlinkRate(std::optional<float> blinkStart, std::optional<float> blinkPeriod);
	void SetInsertionPointTexture(std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	void SetInsertionPointType(uint8_t newType, std::optional<float> prefWidth, std::optional<float> prefHeight);
	void SetMaxSize(int newMaxSize);
	void SetMaxUndos(int newMaxUndos);
	void SetSelectedArea(int startPos, int Count);
	void SetSelectedAreaTextColor(std::optional<Color> NewColor);
	void SetSelectedAreaTexture(std::optional<UObject*> NewTexture, std::optional<Color> NewColor);
	void SetTextChangedFlag(std::optional<bool> bSet);
	void Undo();

	BitfieldBool bCursorShowing() { return BoolValue(PropOffsets_EditWindow.bCursorShowing); }
	BitfieldBool bDragging() { return BoolValue(PropOffsets_EditWindow.bDragging); }
	BitfieldBool bEditable() { return BoolValue(PropOffsets_EditWindow.bEditable); }
	BitfieldBool bSelectWords() { return BoolValue(PropOffsets_EditWindow.bSelectWords); }
	BitfieldBool bSingleLine() { return BoolValue(PropOffsets_EditWindow.bSingleLine); }
	BitfieldBool bUppercaseOnly() { return BoolValue(PropOffsets_EditWindow.bUppercaseOnly); }
	float& blinkDelay() { return Value<float>(PropOffsets_EditWindow.blinkDelay); }
	float& blinkPeriod() { return Value<float>(PropOffsets_EditWindow.blinkPeriod); }
	float& blinkStart() { return Value<float>(PropOffsets_EditWindow.blinkStart); }
	//DynamicArray& bufferList() { return Value<DynamicArray>(PropOffsets_EditWindow.bufferList); }
	int& currentUndo() { return Value<int>(PropOffsets_EditWindow.currentUndo); }
	USound*& deleteSound() { return Value<USound*>(PropOffsets_EditWindow.deleteSound); }
	float& dragDelay() { return Value<float>(PropOffsets_EditWindow.dragDelay); }
	UTexture*& editCursor() { return Value<UTexture*>(PropOffsets_EditWindow.editCursor); }
	Color& editCursorColor() { return Value<Color>(PropOffsets_EditWindow.editCursorColor); }
	UTexture*& editCursorShadow() { return Value<UTexture*>(PropOffsets_EditWindow.editCursorShadow); }
	USound*& enterSound() { return Value<USound*>(PropOffsets_EditWindow.enterSound); }
	Color& insertColor() { return Value<Color>(PropOffsets_EditWindow.insertColor); }
	float& insertHeight() { return Value<float>(PropOffsets_EditWindow.insertHeight); }
	int& insertHookPos() { return Value<int>(PropOffsets_EditWindow.insertHookPos); }
	int& insertPos() { return Value<int>(PropOffsets_EditWindow.insertPos); }
	float& insertPrefHeight() { return Value<float>(PropOffsets_EditWindow.insertPrefHeight); }
	float& insertPrefWidth() { return Value<float>(PropOffsets_EditWindow.insertPrefWidth); }
	float& insertPreferredCol() { return Value<float>(PropOffsets_EditWindow.insertPreferredCol); }
	UTexture*& insertTexture() { return Value<UTexture*>(PropOffsets_EditWindow.insertTexture); }
	uint8_t& insertType() { return Value<uint8_t>(PropOffsets_EditWindow.insertType); }
	float& insertWidth() { return Value<float>(PropOffsets_EditWindow.insertWidth); }
	float& insertX() { return Value<float>(PropOffsets_EditWindow.insertX); }
	float& insertY() { return Value<float>(PropOffsets_EditWindow.insertY); }
	Color& inverseColor() { return Value<Color>(PropOffsets_EditWindow.inverseColor); }
	float& lastConfigHeight() { return Value<float>(PropOffsets_EditWindow.lastConfigHeight); }
	float& lastConfigWidth() { return Value<float>(PropOffsets_EditWindow.lastConfigWidth); }
	int& maxSize() { return Value<int>(PropOffsets_EditWindow.maxSize); }
	int& maxUndos() { return Value<int>(PropOffsets_EditWindow.maxUndos); }
	USound*& moveSound() { return Value<USound*>(PropOffsets_EditWindow.moveSound); }
	Color& selectColor() { return Value<Color>(PropOffsets_EditWindow.selectColor); }
	int& selectEnd() { return Value<int>(PropOffsets_EditWindow.selectEnd); }
	int& selectEndRow() { return Value<int>(PropOffsets_EditWindow.selectEndRow); }
	float& selectEndX() { return Value<float>(PropOffsets_EditWindow.selectEndX); }
	int& selectStart() { return Value<int>(PropOffsets_EditWindow.selectStart); }
	int& selectStartRow() { return Value<int>(PropOffsets_EditWindow.selectStartRow); }
	float& selectStartX() { return Value<float>(PropOffsets_EditWindow.selectStartX); }
	UTexture*& selectTexture() { return Value<UTexture*>(PropOffsets_EditWindow.selectTexture); }
	float& showAreaHeight() { return Value<float>(PropOffsets_EditWindow.showAreaHeight); }
	float& showAreaWidth() { return Value<float>(PropOffsets_EditWindow.showAreaWidth); }
	float& showAreaX() { return Value<float>(PropOffsets_EditWindow.showAreaX); }
	float& showAreaY() { return Value<float>(PropOffsets_EditWindow.showAreaY); }
	USound*& typeSound() { return Value<USound*>(PropOffsets_EditWindow.typeSound); }
	int& unchangedUndo() { return Value<int>(PropOffsets_EditWindow.unchangedUndo); }
};
