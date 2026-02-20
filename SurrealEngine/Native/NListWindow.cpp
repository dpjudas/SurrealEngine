#include "Precomp.h"
#include "NListWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NListWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_3("ListWindow", "AddRow", &NListWindow::AddRow, 1730);
	RegisterVMNativeFunc_3("ListWindow", "AddSortColumn", &NListWindow::AddSortColumn, 1781);
	RegisterVMNativeFunc_0("ListWindow", "DeleteAllRows", &NListWindow::DeleteAllRows, 1733);
	RegisterVMNativeFunc_1("ListWindow", "DeleteRow", &NListWindow::DeleteRow, 1731);
	RegisterVMNativeFunc_1("ListWindow", "EnableAutoExpandColumns", &NListWindow::EnableAutoExpandColumns, 1792);
	RegisterVMNativeFunc_1("ListWindow", "EnableAutoSort", &NListWindow::EnableAutoSort, 1790);
	RegisterVMNativeFunc_1("ListWindow", "EnableHotKeys", &NListWindow::EnableHotKeys, 1785);
	RegisterVMNativeFunc_1("ListWindow", "EnableMultiSelect", &NListWindow::EnableMultiSelect, 1794);
	RegisterVMNativeFunc_2("ListWindow", "GetColumnAlignment", &NListWindow::GetColumnAlignment, 1770);
	RegisterVMNativeFunc_2("ListWindow", "GetColumnColor", &NListWindow::GetColumnColor, 1772);
	RegisterVMNativeFunc_2("ListWindow", "GetColumnFont", &NListWindow::GetColumnFont, 1774);
	RegisterVMNativeFunc_2("ListWindow", "GetColumnTitle", &NListWindow::GetColumnTitle, 1766);
	RegisterVMNativeFunc_2("ListWindow", "GetColumnType", &NListWindow::GetColumnType, 1776);
	RegisterVMNativeFunc_2("ListWindow", "GetColumnWidth", &NListWindow::GetColumnWidth, 1768);
	RegisterVMNativeFunc_3("ListWindow", "GetField", &NListWindow::GetField, 1735);
	RegisterVMNativeFunc_2("ListWindow", "GetFieldMargins", &NListWindow::GetFieldMargins, 1801);
	RegisterVMNativeFunc_3("ListWindow", "GetFieldValue", &NListWindow::GetFieldValue, 1737);
	RegisterVMNativeFunc_1("ListWindow", "GetFocusRow", &NListWindow::GetFocusRow, 1756);
	RegisterVMNativeFunc_1("ListWindow", "GetNumColumns", &NListWindow::GetNumColumns, 1761);
	RegisterVMNativeFunc_1("ListWindow", "GetNumRows", &NListWindow::GetNumRows, 1740);
	RegisterVMNativeFunc_1("ListWindow", "GetNumSelectedRows", &NListWindow::GetNumSelectedRows, 1741);
	RegisterVMNativeFunc_1("ListWindow", "GetPageSize", &NListWindow::GetPageSize, 1802);
	RegisterVMNativeFunc_2("ListWindow", "GetRowClientInt", &NListWindow::GetRowClientInt, 1723);
	RegisterVMNativeFunc_2("ListWindow", "GetRowClientObject", &NListWindow::GetRowClientObject, 1725);
	RegisterVMNativeFunc_1("ListWindow", "GetSelectedRow", &NListWindow::GetSelectedRow, 1747);
	RegisterVMNativeFunc_2("ListWindow", "HideColumn", &NListWindow::HideColumn, 1777);
	RegisterVMNativeFunc_2("ListWindow", "IndexToRowId", &NListWindow::IndexToRowId, 1720);
	RegisterVMNativeFunc_1("ListWindow", "IsAutoExpandColumnsEnabled", &NListWindow::IsAutoExpandColumnsEnabled, 1793);
	RegisterVMNativeFunc_1("ListWindow", "IsAutoSortEnabled", &NListWindow::IsAutoSortEnabled, 1791);
	RegisterVMNativeFunc_2("ListWindow", "IsColumnHidden", &NListWindow::IsColumnHidden, 1778);
	RegisterVMNativeFunc_1("ListWindow", "IsMultiSelectEnabled", &NListWindow::IsMultiSelectEnabled, 1795);
	RegisterVMNativeFunc_2("ListWindow", "IsRowSelected", &NListWindow::IsRowSelected, 1746);
	RegisterVMNativeFunc_2("ListWindow", "ModifyRow", &NListWindow::ModifyRow, 1732);
	RegisterVMNativeFunc_4("ListWindow", "MoveRow", &NListWindow::MoveRow, 1750);
	RegisterVMNativeFunc_3("ListWindow", "PlayListSound", &NListWindow::PlayListSound, 1812);
	RegisterVMNativeFunc_1("ListWindow", "RemoveSortColumn", &NListWindow::RemoveSortColumn, 1782);
	RegisterVMNativeFunc_1("ListWindow", "ResetSortColumns", &NListWindow::ResetSortColumns, 1783);
	RegisterVMNativeFunc_1("ListWindow", "ResizeColumns", &NListWindow::ResizeColumns, 1762);
	RegisterVMNativeFunc_2("ListWindow", "RowIdToIndex", &NListWindow::RowIdToIndex, 1721);
	RegisterVMNativeFunc_1("ListWindow", "SelectAllRows", &NListWindow::SelectAllRows, 1743);
	RegisterVMNativeFunc_2("ListWindow", "SelectRow", &NListWindow::SelectRow, 1742);
	RegisterVMNativeFunc_4("ListWindow", "SelectToRow", &NListWindow::SelectToRow, 1744);
	RegisterVMNativeFunc_2("ListWindow", "SetColumnAlignment", &NListWindow::SetColumnAlignment, 1769);
	RegisterVMNativeFunc_2("ListWindow", "SetColumnColor", &NListWindow::SetColumnColor, 1771);
	RegisterVMNativeFunc_2("ListWindow", "SetColumnFont", &NListWindow::SetColumnFont, 1773);
	RegisterVMNativeFunc_2("ListWindow", "SetColumnTitle", &NListWindow::SetColumnTitle, 1765);
	RegisterVMNativeFunc_3("ListWindow", "SetColumnType", &NListWindow::SetColumnType, 1775);
	RegisterVMNativeFunc_2("ListWindow", "SetColumnWidth", &NListWindow::SetColumnWidth, 1767);
	RegisterVMNativeFunc_1("ListWindow", "SetDelimiter", &NListWindow::SetDelimiter, 1803);
	RegisterVMNativeFunc_3("ListWindow", "SetField", &NListWindow::SetField, 1734);
	RegisterVMNativeFunc_2("ListWindow", "SetFieldMargins", &NListWindow::SetFieldMargins, 1800);
	RegisterVMNativeFunc_3("ListWindow", "SetFieldValue", &NListWindow::SetFieldValue, 1736);
	RegisterVMNativeFunc_1("ListWindow", "SetFocusColor", &NListWindow::SetFocusColor, 1808);
	RegisterVMNativeFunc_3("ListWindow", "SetFocusRow", &NListWindow::SetFocusRow, 1755);
	RegisterVMNativeFunc_1("ListWindow", "SetFocusTexture", &NListWindow::SetFocusTexture, 1807);
	RegisterVMNativeFunc_1("ListWindow", "SetFocusThickness", &NListWindow::SetFocusThickness, 1809);
	RegisterVMNativeFunc_1("ListWindow", "SetHighlightColor", &NListWindow::SetHighlightColor, 1806);
	RegisterVMNativeFunc_1("ListWindow", "SetHighlightTextColor", &NListWindow::SetHighlightTextColor, 1804);
	RegisterVMNativeFunc_1("ListWindow", "SetHighlightTexture", &NListWindow::SetHighlightTexture, 1805);
	RegisterVMNativeFunc_1("ListWindow", "SetHotKeyColumn", &NListWindow::SetHotKeyColumn, 1786);
	RegisterVMNativeFunc_2("ListWindow", "SetListSounds", &NListWindow::SetListSounds, 1811);
	RegisterVMNativeFunc_1("ListWindow", "SetNumColumns", &NListWindow::SetNumColumns, 1760);
	RegisterVMNativeFunc_4("ListWindow", "SetRow", &NListWindow::SetRow, 1751);
	RegisterVMNativeFunc_2("ListWindow", "SetRowClientInt", &NListWindow::SetRowClientInt, 1722);
	RegisterVMNativeFunc_2("ListWindow", "SetRowClientObject", &NListWindow::SetRowClientObject, 1724);
	RegisterVMNativeFunc_3("ListWindow", "SetSortColumn", &NListWindow::SetSortColumn, 1780);
	RegisterVMNativeFunc_0("ListWindow", "ShowFocusRow", &NListWindow::ShowFocusRow, 1810);
	RegisterVMNativeFunc_0("ListWindow", "Sort", &NListWindow::Sort, 1784);
	RegisterVMNativeFunc_1("ListWindow", "ToggleRowSelection", &NListWindow::ToggleRowSelection, 1745);
}

void NListWindow::AddRow(UObject* Self, const std::string& rowStr, int* clientData, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->AddRow(rowStr, clientData);
}

void NListWindow::AddSortColumn(UObject* Self, int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->AddSortColumn(colIndex, bReverse, bCaseSensitive);
}

void NListWindow::DeleteAllRows(UObject* Self)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->DeleteAllRows();
}

void NListWindow::DeleteRow(UObject* Self, int rowId)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->DeleteRow(rowId);
}

void NListWindow::EnableAutoExpandColumns(UObject* Self, BitfieldBool* bAutoExpand)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->EnableAutoExpandColumns(bAutoExpand);
}

void NListWindow::EnableAutoSort(UObject* Self, BitfieldBool* bAutoSort)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->EnableAutoSort(bAutoSort);
}

void NListWindow::EnableHotKeys(UObject* Self, BitfieldBool* bEnable)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->EnableHotKeys(bEnable);
}

void NListWindow::EnableMultiSelect(UObject* Self, BitfieldBool* bEnableMultiSelect)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->EnableMultiSelect(bEnableMultiSelect);
}

void NListWindow::GetColumnAlignment(UObject* Self, int colIndex, uint8_t& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetColumnAlignment(colIndex);
}

void NListWindow::GetColumnColor(UObject* Self, int colIndex, Color& colColor)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->GetColumnColor(colIndex, colColor);
}

void NListWindow::GetColumnFont(UObject* Self, int colIndex, UObject*& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetColumnFont(colIndex);
}

void NListWindow::GetColumnTitle(UObject* Self, int colIndex, std::string& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetColumnTitle(colIndex);
}

void NListWindow::GetColumnType(UObject* Self, int colIndex, uint8_t& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetColumnType(colIndex);
}

void NListWindow::GetColumnWidth(UObject* Self, int colIndex, float& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetColumnWidth(colIndex);
}

void NListWindow::GetField(UObject* Self, int rowId, int colIndex, std::string& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetField(rowId, colIndex);
}

void NListWindow::GetFieldMargins(UObject* Self, float& marginWidth, float& marginHeight)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->GetFieldMargins(marginWidth, marginHeight);
}

void NListWindow::GetFieldValue(UObject* Self, int rowId, int colIndex, float& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetFieldValue(rowId, colIndex);
}

void NListWindow::GetFocusRow(UObject* Self, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetFocusRow();
}

void NListWindow::GetNumColumns(UObject* Self, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetNumColumns();
}

void NListWindow::GetNumRows(UObject* Self, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetNumRows();
}

void NListWindow::GetNumSelectedRows(UObject* Self, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetNumSelectedRows();
}

void NListWindow::GetPageSize(UObject* Self, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetPageSize();
}

void NListWindow::GetRowClientInt(UObject* Self, int rowId, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetRowClientInt(rowId);
}

void NListWindow::GetRowClientObject(UObject* Self, int rowId, UObject*& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetRowClientObject(rowId);
}

void NListWindow::GetSelectedRow(UObject* Self, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->GetSelectedRow();
}

void NListWindow::HideColumn(UObject* Self, int colIndex, BitfieldBool* bHide)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->HideColumn(colIndex, bHide);
}

void NListWindow::IndexToRowId(UObject* Self, int index, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->IndexToRowId(index);
}

void NListWindow::IsAutoExpandColumnsEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->IsAutoExpandColumnsEnabled();
}

void NListWindow::IsAutoSortEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->IsAutoSortEnabled();
}

void NListWindow::IsColumnHidden(UObject* Self, int colIndex, BitfieldBool& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->IsColumnHidden(colIndex);
}

void NListWindow::IsMultiSelectEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->IsMultiSelectEnabled();
}

void NListWindow::IsRowSelected(UObject* Self, int rowId, BitfieldBool& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->IsRowSelected(rowId);
}

void NListWindow::ModifyRow(UObject* Self, int rowId, const std::string& rowStr)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->ModifyRow(rowId, rowStr);
}

void NListWindow::MoveRow(UObject* Self, uint8_t Move, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->MoveRow(Move, bSelect, bClearRows, bDrag);
}

void NListWindow::PlayListSound(UObject* Self, UObject* listSound, float* Volume, float* Pitch)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->PlayListSound(listSound, Volume, Pitch);
}

void NListWindow::RemoveSortColumn(UObject* Self, int colIndex)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->RemoveSortColumn(colIndex);
}

void NListWindow::ResetSortColumns(UObject* Self, BitfieldBool* bSort)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->ResetSortColumns(bSort);
}

void NListWindow::ResizeColumns(UObject* Self, BitfieldBool* bExpandOnly)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->ResizeColumns(bExpandOnly);
}

void NListWindow::RowIdToIndex(UObject* Self, int rowId, int& ReturnValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	ReturnValue = window->RowIdToIndex(rowId);
}

void NListWindow::SelectAllRows(UObject* Self, BitfieldBool* bSelect)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SelectAllRows(bSelect);
}

void NListWindow::SelectRow(UObject* Self, int rowId, BitfieldBool* bSelect)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SelectRow(rowId, bSelect);
}

void NListWindow::SelectToRow(UObject* Self, int rowId, BitfieldBool* bClearRows, BitfieldBool* bInvert, BitfieldBool* bSpanRows)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SelectToRow(rowId, bClearRows, bInvert, bSpanRows);
}

void NListWindow::SetColumnAlignment(UObject* Self, int colIndex, uint8_t newAlign)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetColumnAlignment(colIndex, newAlign);
}

void NListWindow::SetColumnColor(UObject* Self, int colIndex, const Color& NewColor)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetColumnColor(colIndex, NewColor);
}

void NListWindow::SetColumnFont(UObject* Self, int colIndex, UObject* NewFont)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetColumnFont(colIndex, NewFont);
}

void NListWindow::SetColumnTitle(UObject* Self, int colIndex, const std::string& Title)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetColumnTitle(colIndex, Title);
}

void NListWindow::SetColumnType(UObject* Self, int colIndex, uint8_t newType, std::string* newFmt)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetColumnType(colIndex, newType, newFmt);
}

void NListWindow::SetColumnWidth(UObject* Self, int colIndex, float newWidth)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetColumnWidth(colIndex, newWidth);
}

void NListWindow::SetDelimiter(UObject* Self, const std::string& newDelimiter)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetDelimiter(newDelimiter);
}

void NListWindow::SetField(UObject* Self, int rowId, int colIndex, const std::string& fieldStr)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetField(rowId, colIndex, fieldStr);
}

void NListWindow::SetFieldMargins(UObject* Self, float newMarginWidth, float newMarginHeight)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetFieldMargins(newMarginWidth, newMarginHeight);
}

void NListWindow::SetFieldValue(UObject* Self, int rowId, int colIndex, float NewValue)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetFieldValue(rowId, colIndex, NewValue);
}

void NListWindow::SetFocusColor(UObject* Self, const Color& NewColor)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetFocusColor(NewColor);
}

void NListWindow::SetFocusRow(UObject* Self, int rowId, BitfieldBool* bMoveTo, BitfieldBool* bAnchor)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetFocusRow(rowId, bMoveTo, bAnchor);
}

void NListWindow::SetFocusTexture(UObject* Self, UObject* NewTexture)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetFocusTexture(NewTexture);
}

void NListWindow::SetFocusThickness(UObject* Self, float newThickness)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetFocusThickness(newThickness);
}

void NListWindow::SetHighlightColor(UObject* Self, const Color& NewColor)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetHighlightColor(NewColor);
}

void NListWindow::SetHighlightTextColor(UObject* Self, const Color& NewColor)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetHighlightTextColor(NewColor);
}

void NListWindow::SetHighlightTexture(UObject* Self, UObject* NewTexture)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetHighlightTexture(NewTexture);
}

void NListWindow::SetHotKeyColumn(UObject* Self, int colIndex)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetHotKeyColumn(colIndex);
}

void NListWindow::SetListSounds(UObject* Self, UObject** ActivateSound, UObject** moveSound)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetListSounds(ActivateSound, moveSound);
}

void NListWindow::SetNumColumns(UObject* Self, int newCols)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetNumColumns(newCols);
}

void NListWindow::SetRow(UObject* Self, int rowId, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetRow(rowId, bSelect, bClearRows, bDrag);
}

void NListWindow::SetRowClientInt(UObject* Self, int rowId, int clientInt)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetRowClientInt(rowId, clientInt);
}

void NListWindow::SetRowClientObject(UObject* Self, int rowId, UObject* clientObj)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetRowClientObject(rowId, clientObj);
}

void NListWindow::SetSortColumn(UObject* Self, int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->SetSortColumn(colIndex, bReverse, bCaseSensitive);
}

void NListWindow::ShowFocusRow(UObject* Self)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->ShowFocusRow();
}

void NListWindow::Sort(UObject* Self)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->Sort();
}

void NListWindow::ToggleRowSelection(UObject* Self, int rowId)
{
	UListWindow* window = UObject::Cast<UListWindow>(Self);
	window->ToggleRowSelection(rowId);
}
