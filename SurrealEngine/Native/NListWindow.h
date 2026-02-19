#pragma once

#include "UObject/UObject.h"

class NListWindow
{
public:
	static void RegisterFunctions();

	static void AddRow(UObject* Self, const std::string& rowStr, int* clientData, int& ReturnValue);
	static void AddSortColumn(UObject* Self, int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive);
	static void DeleteAllRows(UObject* Self);
	static void DeleteRow(UObject* Self, int rowId);
	static void EnableAutoExpandColumns(UObject* Self, BitfieldBool* bAutoExpand);
	static void EnableAutoSort(UObject* Self, BitfieldBool* bAutoSort);
	static void EnableHotKeys(UObject* Self, BitfieldBool* bEnable);
	static void EnableMultiSelect(UObject* Self, BitfieldBool* bEnableMultiSelect);
	static void GetColumnAlignment(UObject* Self, int colIndex, uint8_t& ReturnValue);
	static void GetColumnColor(UObject* Self, int colIndex, Color& colColor);
	static void GetColumnFont(UObject* Self, int colIndex, UObject*& ReturnValue);
	static void GetColumnTitle(UObject* Self, int colIndex, std::string& ReturnValue);
	static void GetColumnType(UObject* Self, int colIndex, uint8_t& ReturnValue);
	static void GetColumnWidth(UObject* Self, int colIndex, float& ReturnValue);
	static void GetField(UObject* Self, int rowId, int colIndex, std::string& ReturnValue);
	static void GetFieldMargins(UObject* Self, float& marginWidth, float& marginHeight);
	static void GetFieldValue(UObject* Self, int rowId, int colIndex, float& ReturnValue);
	static void GetFocusRow(UObject* Self, int& ReturnValue);
	static void GetNumColumns(UObject* Self, int& ReturnValue);
	static void GetNumRows(UObject* Self, int& ReturnValue);
	static void GetNumSelectedRows(UObject* Self, int& ReturnValue);
	static void GetPageSize(UObject* Self, int& ReturnValue);
	static void GetRowClientInt(UObject* Self, int rowId, int& ReturnValue);
	static void GetRowClientObject(UObject* Self, int rowId, UObject*& ReturnValue);
	static void GetSelectedRow(UObject* Self, int& ReturnValue);
	static void HideColumn(UObject* Self, int colIndex, BitfieldBool* bHide);
	static void IndexToRowId(UObject* Self, int index, int& ReturnValue);
	static void IsAutoExpandColumnsEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsAutoSortEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsColumnHidden(UObject* Self, int colIndex, BitfieldBool& ReturnValue);
	static void IsMultiSelectEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsRowSelected(UObject* Self, int rowId, BitfieldBool& ReturnValue);
	static void ModifyRow(UObject* Self, int rowId, const std::string& rowStr);
	static void MoveRow(UObject* Self, uint8_t Move, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag);
	static void PlayListSound(UObject* Self, UObject* listSound, float* Volume, float* Pitch);
	static void RemoveSortColumn(UObject* Self, int colIndex);
	static void ResetSortColumns(UObject* Self, BitfieldBool* bSort);
	static void ResizeColumns(UObject* Self, BitfieldBool* bExpandOnly);
	static void RowIdToIndex(UObject* Self, int rowId, int& ReturnValue);
	static void SelectAllRows(UObject* Self, BitfieldBool* bSelect);
	static void SelectRow(UObject* Self, int rowId, BitfieldBool* bSelect);
	static void SelectToRow(UObject* Self, int rowId, BitfieldBool* bClearRows, BitfieldBool* bInvert, BitfieldBool* bSpanRows);
	static void SetColumnAlignment(UObject* Self, int colIndex, uint8_t newAlign);
	static void SetColumnColor(UObject* Self, int colIndex, const Color& NewColor);
	static void SetColumnFont(UObject* Self, int colIndex, UObject* NewFont);
	static void SetColumnTitle(UObject* Self, int colIndex, const std::string& Title);
	static void SetColumnType(UObject* Self, int colIndex, uint8_t newType, std::string* newFmt);
	static void SetColumnWidth(UObject* Self, int colIndex, float newWidth);
	static void SetDelimiter(UObject* Self, const std::string& newDelimiter);
	static void SetField(UObject* Self, int rowId, int colIndex, const std::string& fieldStr);
	static void SetFieldMargins(UObject* Self, float newMarginWidth, float newMarginHeight);
	static void SetFieldValue(UObject* Self, int rowId, int colIndex, float NewValue);
	static void SetFocusColor(UObject* Self, const Color& NewColor);
	static void SetFocusRow(UObject* Self, int rowId, BitfieldBool* bMoveTo, BitfieldBool* bAnchor);
	static void SetFocusTexture(UObject* Self, UObject* NewTexture);
	static void SetFocusThickness(UObject* Self, float newThickness);
	static void SetHighlightColor(UObject* Self, const Color& NewColor);
	static void SetHighlightTextColor(UObject* Self, const Color& NewColor);
	static void SetHighlightTexture(UObject* Self, UObject* NewTexture);
	static void SetHotKeyColumn(UObject* Self, int colIndex);
	static void SetListSounds(UObject* Self, UObject** ActivateSound, UObject** moveSound);
	static void SetNumColumns(UObject* Self, int newCols);
	static void SetRow(UObject* Self, int rowId, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag);
	static void SetRowClientInt(UObject* Self, int rowId, int clientInt);
	static void SetRowClientObject(UObject* Self, int rowId, UObject* clientObj);
	static void SetSortColumn(UObject* Self, int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive);
	static void ShowFocusRow(UObject* Self);
	static void Sort(UObject* Self);
	static void ToggleRowSelection(UObject* Self, int rowId);
};
