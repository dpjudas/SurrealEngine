
#include "Precomp.h"
#include "UListWindow.h"
#include "Engine.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Engine/Resources/USound.h"
#include "Packages/Extension/Windows/UGC.h"

void UListWindow::InitWindow()
{
	focusLine() = -1;
	anchorLine() = -1;
	lastIndex() = -1;
	bMultiSelect() = true;
	focusThickness() = 1.0f;
	UWindow::InitWindow();
}

int UListWindow::AddRow(const std::string& rowStr, std::optional<int> clientData)
{
	int id = nextRowId++;
	Item item;
	item.id = id;
	if (clientData.has_value())
		item.clientInt = clientData.value();
	size_t start = 0;
	for (size_t pos = 0; pos < rowStr.size(); pos++)
	{
		if (rowStr[pos] == ';')
		{
			item.cells.push_back(rowStr.substr(start, pos - start));
			start = pos + 1;
		}
	}
	if (start < rowStr.size())
		item.cells.push_back(rowStr.substr(start));
	items.push_back(std::move(item));
	return id;
}

void UListWindow::AddSortColumn(int colIndex, std::optional<bool> bReverse, std::optional<bool> bCaseSensitive)
{
	LogUnimplemented("ListWindow.AddSortColumn");
}

void UListWindow::DeleteAllRows()
{
	items.clear();
	nextRowId = 1;
}

void UListWindow::DeleteRow(int rowId)
{
	int index = RowIdToIndex(rowId);
	if (index >= 0)
		items.erase(items.begin() + index);
}

void UListWindow::EnableAutoExpandColumns(std::optional<bool> bAutoExpand)
{
	bAutoExpandColumns() = bAutoExpand.has_value() ? bAutoExpand.value() : true;
	// To do: actually expand the columns
	LogUnimplemented("ListWindow.EnableAutoExpandColumns");
}

void UListWindow::EnableAutoSort(std::optional<bool> bNewAutoSort)
{
	bAutoSort() = bNewAutoSort.has_value() ? bNewAutoSort.value() : true;
	// To do: actually do the sort
	LogUnimplemented("ListWindow.EnableAutoSort");
}

void UListWindow::EnableHotKeys(std::optional<bool> bEnable)
{
	bHotKeys() = bEnable.has_value() ? bEnable.value() : true;
}

void UListWindow::EnableMultiSelect(std::optional<bool> bEnableMultiSelect)
{
	bMultiSelect() = bEnableMultiSelect.has_value() ? bEnableMultiSelect.value() : true;
}

uint8_t UListWindow::GetColumnAlignment(int colIndex)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return 0;
	return (uint8_t)columns[colIndex].align;
}

void UListWindow::GetColumnColor(int colIndex, Color& colColor)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		colColor = { 255, 255, 255, 255 };
	else
		colColor = columns[colIndex].color;
}

UObject* UListWindow::GetColumnFont(int colIndex)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return nullptr;
	return columns[colIndex].font;
}

std::string UListWindow::GetColumnTitle(int colIndex)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return {};
	return columns[colIndex].title;
}

uint8_t UListWindow::GetColumnType(int colIndex)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return 0;
	return columns[colIndex].type;
}

float UListWindow::GetColumnWidth(int colIndex)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return 0;
	return columns[colIndex].width;
}

std::string UListWindow::GetField(int rowId, int colIndex)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return {};
	if (colIndex < 0 || items[rowIndex].cells.size() >= (size_t)colIndex)
		return {};
	return items[rowIndex].cells[colIndex];
}

void UListWindow::GetFieldMargins(float& marginWidth, float& marginHeight)
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.GetFieldMargins");
}

float UListWindow::GetFieldValue(int rowId, int colIndex)
{
	return (float)std::atof(GetField(rowId, colIndex).c_str());
}

int UListWindow::GetFocusRow()
{
	if (focusLine() < 0 || (size_t)focusLine() >= items.size())
		return 0;
	return items[focusLine()].id;
}

int UListWindow::GetNumColumns()
{
	return (int)columns.size();
}

int UListWindow::GetNumRows()
{
	return (int)items.size();
}

int UListWindow::GetNumSelectedRows()
{
	int count = 0;
	for (auto& item : items)
	{
		if (item.selected)
			count++;
	}
	return count++;
}

int UListWindow::GetPageSize()
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.GetPageSize");
	return 0;
}

int UListWindow::GetRowClientInt(int rowId)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return 0;
	return items[rowIndex].clientInt;
}

UObject* UListWindow::GetRowClientObject(int rowId)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return 0;
	return items[rowIndex].clientObj;
}

int UListWindow::GetSelectedRow()
{
	for (auto& item : items)
	{
		if (item.selected)
		{
			return item.id;
		}
	}
	return 0;
}

void UListWindow::HideColumn(int colIndex, std::optional<bool> bHide)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].hidden = bHide.has_value() ? bHide.value() : true;
}

int UListWindow::IndexToRowId(int index)
{
	if (index < 0 || (size_t)index >= items.size())
		return -1;
	return items[index].id;
}

bool UListWindow::IsAutoExpandColumnsEnabled()
{
	return bAutoExpandColumns();
}

bool UListWindow::IsAutoSortEnabled()
{
	return bAutoSort();
}

bool UListWindow::IsColumnHidden(int colIndex)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return false;
	return columns[colIndex].hidden;
}

bool UListWindow::IsMultiSelectEnabled()
{
	return bMultiSelect();
}

bool UListWindow::IsRowSelected(int rowId)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return false;
	return items[rowIndex].selected;
}

void UListWindow::ModifyRow(int rowId, const std::string& rowStr)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return;

	auto& item = items[rowIndex];
	item.cells.clear();
	size_t start = 0;
	for (size_t pos = 0; pos < rowStr.size(); pos++)
	{
		if (rowStr[pos] == ';')
		{
			item.cells.push_back(rowStr.substr(start, pos - start));
			start = pos + 1;
		}
	}
	if (start < rowStr.size())
		item.cells.push_back(rowStr.substr(start));
}

void UListWindow::MoveRow(uint8_t Move, std::optional<bool> bSelect, std::optional<bool> bClearRows, std::optional<bool> bDrag)
{
	LogUnimplemented("ListWindow.MoveRow");
}

void UListWindow::PlayListSound(UObject* listSound, std::optional<float> Volume, std::optional<float> Pitch)
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.PlayListSound");
}

void UListWindow::RemoveSortColumn(int colIndex)
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.RemoveSortColumn");
}

void UListWindow::ResetSortColumns(std::optional<bool> bSort)
{
	LogUnimplemented("ListWindow.ResetSortColumns");
}

void UListWindow::ResizeColumns(std::optional<bool> bExpandOnly)
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.ResizeColumns");
}

int UListWindow::RowIdToIndex(int rowId)
{
	int index = 0;
	for (auto& item : items)
	{
		if (item.id == rowId)
		{
			return index;
		}
		index++;
	}
	return -1;
}

void UListWindow::SelectAllRows(std::optional<bool> bSelect)
{
	bool selected = bSelect.has_value() ? bSelect.value() : true;
	for (auto& item : items)
	{
		item.selected = selected;
	}
}

void UListWindow::SelectRow(int rowId, std::optional<bool> bSelect)
{
	bool selected = bSelect.has_value() ? bSelect.value() : true;
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex != -1)
		items[rowIndex].selected = selected;
}

void UListWindow::SelectToRow(int rowId, std::optional<bool> bClearRows, std::optional<bool> bInvert, std::optional<bool> bSpanRows)
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.SelectToRow");
}

void UListWindow::SetColumnAlignment(int colIndex, uint8_t newAlign)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].align = (EHAlign)newAlign;
}

void UListWindow::SetColumnColor(int colIndex, const Color& NewColor)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].color = NewColor;
}

void UListWindow::SetColumnFont(int colIndex, UObject* NewFont)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].font = UObject::Cast<UFont>(NewFont);
}

void UListWindow::SetColumnTitle(int colIndex, const std::string& Title)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].title = Title;
}

void UListWindow::SetColumnType(int colIndex, uint8_t newType, std::optional<std::string> newFmt)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].type = newType;
	columns[colIndex].format = newFmt;
}

void UListWindow::SetColumnWidth(int colIndex, float newWidth)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	columns[colIndex].width = newWidth;
}

void UListWindow::SetDelimiter(const std::string& newDelimiter)
{
	Delimiter() = newDelimiter;
}

void UListWindow::SetField(int rowId, int colIndex, const std::string& fieldStr)
{
	if (colIndex < 0 || (size_t)colIndex >= columns.size())
		return;
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return;
	if (items[rowIndex].cells.size() <= (size_t)colIndex)
		items[rowIndex].cells.resize(colIndex + 1);
	items[rowIndex].cells[colIndex] = fieldStr;
}

void UListWindow::SetFieldMargins(float newMarginWidth, float newMarginHeight)
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.SetFieldMargins");
}

void UListWindow::SetFieldValue(int rowId, int colIndex, float NewValue)
{
	SetField(rowId, colIndex, std::to_string(NewValue));
}

void UListWindow::SetFocusColor(const Color& NewColor)
{
	focusColor() = NewColor;
}

void UListWindow::SetFocusRow(int rowId, std::optional<bool> bMoveTo, std::optional<bool> bAnchor)
{
	// Note: bMoveTo and bAnchor is never used directly from script
	focusLine() = RowIdToIndex(rowId);
}

void UListWindow::SetFocusTexture(UObject* NewTexture)
{
	focusTexture() = UObject::Cast<UTexture>(NewTexture);
}

void UListWindow::SetFocusThickness(float newThickness)
{
	focusThickness() = newThickness;
}

void UListWindow::SetHighlightColor(const Color& NewColor)
{
	highlightColor() = NewColor;
}

void UListWindow::SetHighlightTextColor(const Color& NewColor)
{
	highlightTextColor = NewColor;
}

void UListWindow::SetHighlightTexture(UObject* NewTexture)
{
	highlightTexture() = UObject::Cast<UTexture>(NewTexture);
}

void UListWindow::SetHotKeyColumn(int colIndex)
{
	hotKeyCol() = colIndex;
}

void UListWindow::SetListSounds(std::optional<UObject*> newActivateSound, std::optional<UObject*> newMoveSound)
{
	if (newActivateSound.has_value())
		ActivateSound() = UObject::Cast<USound>(newActivateSound.value());
	if (newMoveSound.has_value())
		moveSound() = UObject::Cast<USound>(newMoveSound.value());
}

void UListWindow::SetNumColumns(int newCols)
{
	columns.resize(newCols);
}

void UListWindow::SetRow(int rowId, std::optional<bool> bSelect, std::optional<bool> bClearRows, std::optional<bool> bDrag)
{
	if (!bClearRows.has_value() || *bClearRows)
		SelectAllRows(false);
	if (!bSelect.has_value() || *bSelect)
		SelectRow(rowId, true);
	// Should this also call SetFocusRow()?
}

void UListWindow::SetRowClientInt(int rowId, int clientInt)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return;
	items[rowIndex].clientInt = clientInt;
}

void UListWindow::SetRowClientObject(int rowId, UObject* clientObj)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return;
	items[rowIndex].clientObj = clientObj;
}

void UListWindow::SetSortColumn(int colIndex, std::optional<bool> bReverse, std::optional<bool> bCaseSensitive)
{
	LogUnimplemented("ListWindow.SetSortColumn");
}

void UListWindow::ShowFocusRow()
{
	// UNUSED from scripts.
	LogUnimplemented("ListWindow.ShowFocusRow");
}

void UListWindow::Sort()
{
	LogUnimplemented("ListWindow.Sort");
}

void UListWindow::ToggleRowSelection(int rowId)
{
	int rowIndex = RowIdToIndex(rowId);
	if (rowIndex == -1)
		return;
	items[rowIndex].selected = !items[rowIndex].selected;
}

void UListWindow::DrawWindow(UGC* gc)
{
	UFont* font = normalFont();
	if (!font)
		return;

	float w = Width();
	float h = Height();
	float lineHeight = (float)font->GetGlyph('X').VSize + 2;
	lineSize() = lineHeight;

	float y = 0.0f;
	int lineIndex = 0;
	for (auto& item : items)
	{
		if (lineIndex == focusLine())
		{
			gc->SetTextColor(highlightTextColor);
			gc->SetTileColor(focusColor());
			if (focusTexture())
				gc->DrawTexture(0.0f, y, w, lineHeight, 0.0f, 0.0f, focusTexture());
		}

		if (item.selected)
		{
			float t = focusThickness();
			gc->SetTextColor(highlightTextColor);
			gc->SetTileColor(highlightColor());
			if (highlightTexture())
				gc->DrawTexture(t, y + t, std::max(w - 2.0f * t, 0.0f), std::max(lineHeight - t * 2.0f, 0.0f), 0.0f, 0.0f, highlightTexture());
		}
		else
		{
			gc->SetTextColor(TextColor());
			gc->SetTileColor(tileColor());
		}

		float x = 0.0f;
		size_t colIndex = 0;
		for (auto& col : columns)
		{
			if (item.cells.size() > colIndex)
			{
				UFont* colFont = col.font ? col.font : font;
				gc->SetFont(colFont);
				gc->SetAlignments((uint8_t)col.align, (uint8_t)EVAlign::Center);

				//if (!item.selected)
				//	gc->SetTextColor(col.color);

				gc->DrawText(x, y, col.width, lineHeight, item.cells[colIndex]);
			}
			x += col.width;
			colIndex++;
		}
		y += lineHeight;
		lineIndex++;
	}
}

bool UListWindow::MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks)
{
	if (UWindow::MouseButtonPressed(pointX, pointY, button, numClicks))
		return true;

	if (lineSize() <= 0.0f)
		return true;

	int index = (int)std::floor(pointY / lineSize());
	int rowId = IndexToRowId(index);
	if (rowId > 0)
	{
		SetRow(rowId, true, true, false);
		SetFocusRow(rowId, false, false);
		DispatchListSelectionChanged();
	}

	return true;
}

bool UListWindow::MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks)
{
	return UWindow::MouseButtonReleased(pointX, pointY, button, numClicks);
}

void UListWindow::DispatchListSelectionChanged()
{
	int numSelections = GetNumSelectedRows();
	int focusRowId = GetFocusRow();
	for (UWindow* cur = this; cur; cur = cur->parentOwner())
	{
		if (cur->ListSelectionChanged(this, numSelections, focusRowId))
			break;
	}
}
