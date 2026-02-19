#include "Precomp.h"
#include "NListWindow.h"
#include "VM/NativeFunc.h"
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
	LogUnimplemented("NListWindow::AddRow not implemented");
	ReturnValue = 0;
}

void NListWindow::AddSortColumn(UObject* Self, int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive)
{
	LogUnimplemented("NListWindow::AddSortColumn not implemented");
}

void NListWindow::DeleteAllRows(UObject* Self)
{
	LogUnimplemented("NListWindow::DeleteAllRows not implemented");
}

void NListWindow::DeleteRow(UObject* Self, int rowId)
{
	LogUnimplemented("NListWindow::DeleteRow not implemented");
}

void NListWindow::EnableAutoExpandColumns(UObject* Self, BitfieldBool* bAutoExpand)
{
	LogUnimplemented("NListWindow::EnableAutoExpandColumns not implemented");
}

void NListWindow::EnableAutoSort(UObject* Self, BitfieldBool* bAutoSort)
{
	LogUnimplemented("NListWindow::EnableAutoSort not implemented");
}

void NListWindow::EnableHotKeys(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("NListWindow::EnableHotKeys not implemented");
}

void NListWindow::EnableMultiSelect(UObject* Self, BitfieldBool* bEnableMultiSelect)
{
	LogUnimplemented("NListWindow::EnableMultiSelect not implemented");
}

void NListWindow::GetColumnAlignment(UObject* Self, int colIndex, uint8_t& ReturnValue)
{
	LogUnimplemented("NListWindow::GetColumnAlignment not implemented");
	ReturnValue = 0;
}

void NListWindow::GetColumnColor(UObject* Self, int colIndex, Color& colColor)
{
	LogUnimplemented("NListWindow::GetColumnColor not implemented");
}

void NListWindow::GetColumnFont(UObject* Self, int colIndex, UObject*& ReturnValue)
{
	LogUnimplemented("NListWindow::GetColumnFont not implemented");
	ReturnValue = nullptr;
}

void NListWindow::GetColumnTitle(UObject* Self, int colIndex, std::string& ReturnValue)
{
	LogUnimplemented("NListWindow::GetColumnTitle not implemented");
	ReturnValue = "";
}

void NListWindow::GetColumnType(UObject* Self, int colIndex, uint8_t& ReturnValue)
{
	LogUnimplemented("NListWindow::GetColumnType not implemented");
	ReturnValue = 0;
}

void NListWindow::GetColumnWidth(UObject* Self, int colIndex, float& ReturnValue)
{
	LogUnimplemented("NListWindow::GetColumnWidth not implemented");
	ReturnValue = 0.0f;
}

void NListWindow::GetField(UObject* Self, int rowId, int colIndex, std::string& ReturnValue)
{
	LogUnimplemented("NListWindow::GetField not implemented");
	ReturnValue = "";
}

void NListWindow::GetFieldMargins(UObject* Self, float& marginWidth, float& marginHeight)
{
	LogUnimplemented("NListWindow::GetFieldMargins not implemented");
}

void NListWindow::GetFieldValue(UObject* Self, int rowId, int colIndex, float& ReturnValue)
{
	LogUnimplemented("NListWindow::GetFieldValue not implemented");
	ReturnValue = 0.0f;
}

void NListWindow::GetFocusRow(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetFocusRow not implemented");
	ReturnValue = 0;
}

void NListWindow::GetNumColumns(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetNumColumns not implemented");
	ReturnValue = 0;
}

void NListWindow::GetNumRows(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetNumRows not implemented");
	ReturnValue = 0;
}

void NListWindow::GetNumSelectedRows(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetNumSelectedRows not implemented");
	ReturnValue = 0;
}

void NListWindow::GetPageSize(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetPageSize not implemented");
	ReturnValue = 0;
}

void NListWindow::GetRowClientInt(UObject* Self, int rowId, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetRowClientInt not implemented");
	ReturnValue = 0;
}

void NListWindow::GetRowClientObject(UObject* Self, int rowId, UObject*& ReturnValue)
{
	LogUnimplemented("NListWindow::GetRowClientObject not implemented");
	ReturnValue = 0;
}

void NListWindow::GetSelectedRow(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NListWindow::GetSelectedRow not implemented");
	ReturnValue = 0;
}

void NListWindow::HideColumn(UObject* Self, int colIndex, BitfieldBool* bHide)
{
	LogUnimplemented("NListWindow::HideColumn not implemented");
}

void NListWindow::IndexToRowId(UObject* Self, int index, int& ReturnValue)
{
	LogUnimplemented("NListWindow::IndexToRowId not implemented");
	ReturnValue = 0;
}

void NListWindow::IsAutoExpandColumnsEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NListWindow::IsAutoExpandColumnsEnabled not implemented");
	ReturnValue = false;
}

void NListWindow::IsAutoSortEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NListWindow::IsAutoSortEnabled not implemented");
	ReturnValue = false;
}

void NListWindow::IsColumnHidden(UObject* Self, int colIndex, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NListWindow::IsColumnHidden not implemented");
	ReturnValue = false;
}

void NListWindow::IsMultiSelectEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NListWindow::IsMultiSelectEnabled not implemented");
	ReturnValue = false;
}

void NListWindow::IsRowSelected(UObject* Self, int rowId, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NListWindow::IsRowSelected not implemented");
	ReturnValue = false;
}

void NListWindow::ModifyRow(UObject* Self, int rowId, const std::string& rowStr)
{
	LogUnimplemented("NListWindow::ModifyRow not implemented");
}

void NListWindow::MoveRow(UObject* Self, uint8_t Move, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag)
{
	LogUnimplemented("NListWindow::MoveRow not implemented");
}

void NListWindow::PlayListSound(UObject* Self, UObject* listSound, float* Volume, float* Pitch)
{
	LogUnimplemented("NListWindow::PlayListSound not implemented");
}

void NListWindow::RemoveSortColumn(UObject* Self, int colIndex)
{
	LogUnimplemented("NListWindow::RemoveSortColumn not implemented");
}

void NListWindow::ResetSortColumns(UObject* Self, BitfieldBool* bSort)
{
	LogUnimplemented("NListWindow::ResetSortColumns not implemented");
}

void NListWindow::ResizeColumns(UObject* Self, BitfieldBool* bExpandOnly)
{
	LogUnimplemented("NListWindow::ResizeColumns not implemented");
}

void NListWindow::RowIdToIndex(UObject* Self, int rowId, int& ReturnValue)
{
	LogUnimplemented("NListWindow::RowIdToIndex not implemented");
	ReturnValue = 0;
}

void NListWindow::SelectAllRows(UObject* Self, BitfieldBool* bSelect)
{
	LogUnimplemented("NListWindow::SelectAllRows not implemented");
}

void NListWindow::SelectRow(UObject* Self, int rowId, BitfieldBool* bSelect)
{
	LogUnimplemented("NListWindow::SelectRow not implemented");
}

void NListWindow::SelectToRow(UObject* Self, int rowId, BitfieldBool* bClearRows, BitfieldBool* bInvert, BitfieldBool* bSpanRows)
{
	LogUnimplemented("NListWindow::SelectToRow not implemented");
}

void NListWindow::SetColumnAlignment(UObject* Self, int colIndex, uint8_t newAlign)
{
	LogUnimplemented("NListWindow::SetColumnAlignment not implemented");
}

void NListWindow::SetColumnColor(UObject* Self, int colIndex, const Color& NewColor)
{
	LogUnimplemented("NListWindow::SetColumnColor not implemented");
}

void NListWindow::SetColumnFont(UObject* Self, int colIndex, UObject* NewFont)
{
	LogUnimplemented("NListWindow::SetColumnFont not implemented");
}

void NListWindow::SetColumnTitle(UObject* Self, int colIndex, const std::string& Title)
{
	LogUnimplemented("NListWindow::SetColumnTitle not implemented");
}

void NListWindow::SetColumnType(UObject* Self, int colIndex, uint8_t newType, std::string* newFmt)
{
	LogUnimplemented("NListWindow::SetColumnType not implemented");
}

void NListWindow::SetColumnWidth(UObject* Self, int colIndex, float newWidth)
{
	LogUnimplemented("NListWindow::SetColumnWidth not implemented");
}

void NListWindow::SetDelimiter(UObject* Self, const std::string& newDelimiter)
{
	LogUnimplemented("NListWindow::SetDelimiter not implemented");
}

void NListWindow::SetField(UObject* Self, int rowId, int colIndex, const std::string& fieldStr)
{
	LogUnimplemented("NListWindow::SetField not implemented");
}

void NListWindow::SetFieldMargins(UObject* Self, float newMarginWidth, float newMarginHeight)
{
	LogUnimplemented("NListWindow::SetFieldMargins not implemented");
}

void NListWindow::SetFieldValue(UObject* Self, int rowId, int colIndex, float NewValue)
{
	LogUnimplemented("NListWindow::SetFieldValue not implemented");
}

void NListWindow::SetFocusColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("NListWindow::SetFocusColor not implemented");
}

void NListWindow::SetFocusRow(UObject* Self, int rowId, BitfieldBool* bMoveTo, BitfieldBool* bAnchor)
{
	LogUnimplemented("NListWindow::SetFocusRow not implemented");
}

void NListWindow::SetFocusTexture(UObject* Self, UObject* NewTexture)
{
	LogUnimplemented("NListWindow::SetFocusTexture not implemented");
}

void NListWindow::SetFocusThickness(UObject* Self, float newThickness)
{
	LogUnimplemented("NListWindow::SetFocusThickness not implemented");
}

void NListWindow::SetHighlightColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("NListWindow::SetHighlightColor not implemented");
}

void NListWindow::SetHighlightTextColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("NListWindow::SetHighlightTextColor not implemented");
}

void NListWindow::SetHighlightTexture(UObject* Self, UObject* NewTexture)
{
	LogUnimplemented("NListWindow::SetHighlightTexture not implemented");
}

void NListWindow::SetHotKeyColumn(UObject* Self, int colIndex)
{
	LogUnimplemented("NListWindow::SetHotKeyColumn not implemented");
}

void NListWindow::SetListSounds(UObject* Self, UObject** ActivateSound, UObject** moveSound)
{
	LogUnimplemented("NListWindow::SetListSounds not implemented");
}

void NListWindow::SetNumColumns(UObject* Self, int newCols)
{
	LogUnimplemented("NListWindow::SetNumColumns not implemented");
}

void NListWindow::SetRow(UObject* Self, int rowId, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag)
{
	LogUnimplemented("NListWindow::SetRow not implemented");
}

void NListWindow::SetRowClientInt(UObject* Self, int rowId, int clientInt)
{
	LogUnimplemented("NListWindow::SetRowClientInt not implemented");
}

void NListWindow::SetRowClientObject(UObject* Self, int rowId, UObject* clientObj)
{
	LogUnimplemented("NListWindow::SetRowClientObject not implemented");
}

void NListWindow::SetSortColumn(UObject* Self, int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive)
{
	LogUnimplemented("NListWindow::SetSortColumn not implemented");
}

void NListWindow::ShowFocusRow(UObject* Self)
{
	LogUnimplemented("NListWindow::ShowFocusRow not implemented");
}

void NListWindow::Sort(UObject* Self)
{
	LogUnimplemented("NListWindow::Sort not implemented");
}

void NListWindow::ToggleRowSelection(UObject* Self, int rowId)
{
	LogUnimplemented("NListWindow::ToggleRowSelection not implemented");
}
