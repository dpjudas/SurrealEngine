#pragma once

#include "UObject.h"

class UWindow : public UObject
{
public:
	using UObject::UObject;

	void AddActorRef(UObject* refActor);
	int AddTimer(float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName);
	void AskParentForReconfigure();
	void AskParentToShowArea(float* areaX, float* areaY, float* areaWidth, float* areaHeight);
	std::string CarriageReturn();
	void ChangeStyle();
	void ConfigureChild(float newX, float newY, float newWidth, float NewHeight);
	void ConvertCoordinates(UObject* fromWin, float fromX, float fromY, UObject* toWin, float& toX, float& toY);
	std::string ConvertScriptString(const std::string& oldStr);
	bool ConvertVectorToCoordinates(const vec3& Location, float& relativeX, float& relativeY);
	void Destroy();
	void DestroyAllChildren();
	void DisableWindow();
	void EnableSpecialText(BitfieldBool* bEnable);
	void EnableTranslucentText(BitfieldBool* bEnable);
	void EnableWindow(BitfieldBool* bEnable);
	UObject* FindWindow(float pointX, float pointY, float& relativeX, float& relativeY);
	UObject* GetBottomChild(BitfieldBool* bVisibleOnly);
	UObject* GetClientObject();
	void GetCursorPos(float& MouseX, float& MouseY);
	UObject* GetFocusWindow();
	UObject* GetGC();
	UObject* GetHigherSibling(BitfieldBool* bVisibleOnly);
	UObject* GetLowerSibling(BitfieldBool* bVisibleOnly);
	UObject* GetModalWindow();
	UObject* GetParent();
	UObject* GetPlayerPawn();
	UObject* GetRootWindow();
	UObject* GetTabGroupWindow();
	float GetTickOffset();
	UObject* GetTopChild(BitfieldBool* bVisibleOnly);
	void GrabMouse();
	void Hide();
	bool IsActorValid(UObject* refActor);
	bool IsFocusWindow();
	bool IsKeyDown(uint8_t Key);
	bool IsPointInWindow(float pointX, float pointY);
	bool IsSensitive(BitfieldBool* bRecurse);
	bool IsVisible(BitfieldBool* bRecurse);
	void Lower();
	UObject* MoveFocusDown();
	UObject* MoveFocusLeft();
	UObject* MoveFocusRight();
	UObject* MoveFocusUp();
	UObject* MoveTabGroupNext();
	UObject* MoveTabGroupPrev();
	UObject* NewChild(UObject* NewClass, BitfieldBool* bShow);
	void PlaySound(UObject* newsound, float* Volume, float* Pitch, float* posX, float* posY);
	void QueryGranularity(float& hGranularity, float& vGranularity);
	float QueryPreferredHeight(float queryWidth);
	void QueryPreferredSize(float& preferredWidth, float& preferredHeight);
	float QueryPreferredWidth(float queryHeight);
	void Raise();
	void ReleaseGC(UObject* GC);
	void RemoveActorRef(UObject* refActor);
	void RemoveTimer(int timerId);
	void ResetHeight();
	void ResetSize();
	void ResetWidth();
	void ResizeChild();
	void SetAcceleratorText(const std::string& newStr);
	void SetBackground(UObject* newBackground);
	void SetBackgroundSmoothing(bool newSmoothing);
	void SetBackgroundStretching(bool newStretching);
	void SetBackgroundStyle(uint8_t NewStyle);
	void SetBaselineData(float* newBaselineOffset, float* newUnderlineHeight);
	void SetBoldFont(UObject* fn);
	void SetChildVisibility(bool bNewVisibility);
	void SetClientObject(UObject* newClientObject);
	void SetConfiguration(float newX, float newY, float newWidth, float NewHeight);
	void SetCursorPos(float newMouseX, float newMouseY);
	void SetDefaultCursor(UObject* tX, UObject** shadowTexture, float* HotX, float* HotY, Color* cursorColor);
	void SetFocusSounds(UObject** focusSound, UObject** unfocusSound);
	bool SetFocusWindow(UObject* NewFocusWindow);
	void SetFont(UObject* fn);
	void SetFonts(UObject* nFont, UObject* bFont);
	void SetHeight(float NewHeight);
	void SetNormalFont(UObject* fn);
	void SetPos(float newX, float newY);
	void SetSelectability(bool newSelectability);
	void SetSensitivity(bool newSensitivity);
	void SetSize(float newWidth, float NewHeight);
	void SetSoundVolume(float newVolume);
	void SetTextColor(const Color& NewColor);
	void SetTileColor(const Color& NewColor);
	void SetVisibilitySounds(UObject** visSound, UObject** invisSound);
	void SetWidth(float newWidth);
	void SetWindowAlignments(uint8_t HAlign, uint8_t VAlign, float* hMargin0, float* vMargin0, float* hMargin1, float* vMargin1);
	void Show(BitfieldBool* bShow);
	void UngrabMouse();
};

class UViewportWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void ClearZBuffer(BitfieldBool* bClear);
	void EnableViewport(BitfieldBool* bEnable);
	void SetDefaultTexture(UObject** NewTexture, Color* NewColor);
	void SetFOVAngle(float* newAngle);
	void SetRelativeLocation(vec3* relLoc);
	void SetRelativeRotation(Rotator* relRot);
	void SetRotation(Rotator* NewRotation);
	void SetViewportActor(UObject** newOriginActor, BitfieldBool* bEyeLevel, BitfieldBool* bEnable);
	void SetViewportLocation(const vec3& NewLocation, BitfieldBool* bEnable);
	void SetWatchActor(UObject** newWatchActor, BitfieldBool* bEyeLevel);
	void ShowViewportActor(BitfieldBool* bShow);
	void ShowWeapons(BitfieldBool* bShow);
};

class UTileWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void EnableWrapping(bool bWrapOn);
	void FillParent(bool FillParent);
	void MakeHeightsEqual(bool bEqual);
	void MakeWidthsEqual(bool bEqual);
	void SetChildAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetDirections(uint8_t newHDir, uint8_t newVDir);
	void SetMajorSpacing(float newSpacing);
	void SetMargins(float newHMargin, float newVMargin);
	void SetMinorSpacing(float newSpacing);
	void SetOrder(uint8_t newOrder);
	void SetOrientation(uint8_t newOrientation);
};

class UTextWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void AppendText(const std::string& NewText);
	void EnableTextAsAccelerator(BitfieldBool* bEnable);
	std::string GetText();
	int GetTextLength();
	int GetTextPart(int startPos, int Count, std::string& OutText);
	void ResetLines();
	void ResetMinWidth();
	void SetLines(int newMinLines, int newMaxLines);
	void SetMaxLines(int newMaxLines);
	void SetMinLines(int newMinLines);
	void SetMinWidth(float newMinWidth);
	void SetText(const std::string& NewText);
	void SetTextAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetTextMargins(float newHMargin, float newVMargin);
	void SetWordWrap(bool bNewWordWrap);
};

class UButtonWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;

	void ActivateButton(uint8_t Key);
	void EnableAutoRepeat(BitfieldBool* bEnable, float* initialDelay, float* repeatRate);
	void EnableRightMouseClick(BitfieldBool* bEnable);
	void PressButton(uint8_t* Key);
	void SetActivateDelay(float* newDelay);
	void SetButtonColors(Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive);
	void SetButtonSounds(UObject** pressSound, UObject** clickSound);
	void SetButtonTextures(UObject** Normal, UObject** pressed, UObject** normalFocus, UObject** pressedFocus, UObject** normalInsensitive, UObject** pressedInsensitive);
	void SetTextColors(Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive);
};

class UToggleWindow : public UButtonWindow
{
public:
    using UButtonWindow::UButtonWindow;

	void ChangeToggle();
	bool GetToggle();
	void SetToggle(bool bNewToggle);
	void SetToggleSounds(UObject** enableSound, UObject** disableSound);
};

class UCheckboxWindow : public UToggleWindow
{
public:
    using UToggleWindow::UToggleWindow;

	void SetCheckboxColor(const Color& NewColor);
	void SetCheckboxSpacing(float newSpacing);
	void SetCheckboxStyle(uint8_t NewStyle);
	void SetCheckboxTextures(UObject** toggleOff, UObject** toggleOn, float* textureWidth, float* textureHeight);
	void ShowCheckboxOnRightSide(BitfieldBool* bRight);
};

class UTextLogWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;

	void AddLog(const std::string& NewText, const Color& linecol);
	void ClearLog();
	void PauseLog(bool bNewPauseState);
	void SetTextTimeout(float newTimeout);
};

class ULargeTextWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;

	void SetVerticalSpacing(float* newVSpace);
};

class UEditWindow : public ULargeTextWindow
{
public:
    using ULargeTextWindow::ULargeTextWindow;

	void ClearTextChangedFlag();
	void ClearUndo();
	void Copy();
	void Cut();
	void DeleteChar(BitfieldBool* bBefore, BitfieldBool* bUndo);
	void EnableEditing(BitfieldBool* bEdit);
	void EnableSingleLineEditing(BitfieldBool* bSingle);
	void EnableUppercaseOnly(BitfieldBool* bUppercase);
	int GetInsertionPoint();
	void GetSelectedArea(int& startPos, int& Count);
	bool HasTextChanged();
	bool InsertText(std::string* InsertText, BitfieldBool* bUndo, BitfieldBool* bSelect);
	bool IsEditingEnabled();
	bool IsSingleLineEditingEnabled();
	void MoveInsertionPoint(uint8_t moveInsert, BitfieldBool* bDrag);
	void Paste();
	void PlayEditSound(UObject* PlaySound, float* Volume, float* Pitch);
	void Redo();
	void SetEditCursor(UObject** newCursor, UObject** newCursorShadow, Color* NewColor);
	void SetEditSounds(UObject** typeSound, UObject** deleteSound, UObject** enterSound, UObject** moveSound);
	void SetInsertionPoint(int NewPos, BitfieldBool* bDrag);
	void SetInsertionPointBlinkRate(float* blinkStart, float* blinkPeriod);
	void SetInsertionPointTexture(UObject** NewTexture, Color* NewColor);
	void SetInsertionPointType(uint8_t newType, float* prefWidth, float* prefHeight);
	void SetMaxSize(int newMaxSize);
	void SetMaxUndos(int newMaxUndos);
	void SetSelectedArea(int startPos, int Count);
	void SetSelectedAreaTextColor(Color* NewColor);
	void SetSelectedAreaTexture(UObject** NewTexture, Color* NewColor);
	void SetTextChangedFlag(BitfieldBool* bSet);
	void Undo();
};

class UTabGroupWindow : public UWindow
{
public:
    using UWindow::UWindow;
};

class URadioBoxWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;

	UObject* GetEnabledToggle();
};

class UClipWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;

	void EnableSnapToUnits(BitfieldBool* bNewSnapToUnits);
	void ForceChildSize(BitfieldBool* bNewForceChildWidth, BitfieldBool* bNewForceChildHeight);
	UObject* GetChild();
	void GetChildPosition(int& pNewX, int& pNewY);
	void GetUnitSize(int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize);
	void ResetUnitHeight();
	void ResetUnitSize();
	void ResetUnitWidth();
	void SetChildPosition(int newX, int newY);
	void SetUnitHeight(int vUnits);
	void SetUnitSize(int hUnits, int vUnits);
	void SetUnitWidth(int hUnits);
};

class UModalWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;

	bool IsCurrentModal();
	void SetMouseFocusMode(uint8_t newFocusMode);
};

class URootWindow : public UModalWindow
{
public:
    using UModalWindow::UModalWindow;

	void EnablePositionalSound(BitfieldBool* bEnable);
	void EnableRendering(BitfieldBool* bRender);
	UObject* GenerateSnapshot(BitfieldBool* bFilter);
	bool IsPositionalSoundEnabled();
	bool IsRenderingEnabled();
	void LockMouse(BitfieldBool* bLockMove, BitfieldBool* bLockButton);
	void ResetRenderViewport();
	void SetDefaultEditCursor(UObject** newEditCursor);
	void SetDefaultMovementCursors(UObject** newMovementCursor, UObject** newHorizontalMovementCursor, UObject** newVerticalMovementCursor, UObject** newTopLeftMovementCursor, UObject** newTopRightMovementCursor);
	void SetRawBackground(UObject** NewTexture, Color* NewColor);
	void SetRawBackgroundSize(float newWidth, float NewHeight);
	void SetRenderViewport(float newX, float newY, float newWidth, float NewHeight);
	void SetSnapshotSize(float newWidth, float NewHeight);
	void ShowCursor(BitfieldBool* bShow);
	void StretchRawBackground(BitfieldBool* bStretch);
};

class UScrollAreaWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void AutoHideScrollbars(BitfieldBool* bHide);
	void EnableScrolling(BitfieldBool* bHScrolling, BitfieldBool* bVScrolling);
	void SetAreaMargins(float newMarginWidth, float newMarginHeight);
	void SetScrollbarDistance(float newDistance);
};

class UScaleWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void ClearAllEnumerations();
	void EnableStretchedScale(BitfieldBool* bNewStretch);
	int GetNumTicks();
	int GetThumbSpan();
	int GetTickPosition();
	float GetValue();
	std::string GetValueString();
	void GetValues(float& fromValue, float& toValue);
	void MoveThumb(uint8_t MoveThumb);
	void PlayScaleSound(UObject* newsound, float* Volume, float* Pitch);
	void SetBorderPattern(UObject* NewTexture);
	void SetEnumeration(int tickPos, const std::string& newStr);
	void SetNumTicks(int newNumTicks);
	void SetScaleBorder(float* newBorderSize, Color* NewColor);
	void SetScaleColor(const Color& NewColor);
	void SetScaleMargins(float* marginWidth, float* marginHeight);
	void SetScaleOrientation(uint8_t newOrientation);
	void SetScaleSounds(UObject** setSound, UObject** clickSound, UObject** dragSound);
	void SetScaleStyle(uint8_t NewStyle);
	void SetScaleTexture(UObject* NewTexture, float* newWidth, float* NewHeight, float* newStart, float* newEnd);
	void SetThumbBorder(float* newBorderSize, Color* NewColor);
	void SetThumbCaps(UObject* preCap, UObject* postCap, float* preCapWidth, float* preCapHeight, float* postCapWidth, float* postCapHeight);
	void SetThumbColor(const Color& NewColor);
	void SetThumbSpan(int* newRange);
	void SetThumbStep(int NewStep);
	void SetThumbStyle(uint8_t NewStyle);
	void SetThumbTexture(UObject* NewTexture, float* newWidth, float* NewHeight);
	void SetTickColor(const Color& NewColor);
	void SetTickPosition(int newPosition);
	void SetTickStyle(uint8_t NewStyle);
	void SetTickTexture(UObject* tickTexture, BitfieldBool* bDrawEndTicks, float* newWidth, float* NewHeight);
	void SetValue(float NewValue);
	void SetValueFormat(const std::string& newFmt);
	void SetValueRange(float newFrom, float newTo);
};

class UScaleManagerWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void SetManagerAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetManagerMargins(float* newMarginWidth, float* newMarginHeight);
	void SetManagerOrientation(uint8_t newOrientation);
	void SetMarginSpacing(float* newSpacing);
	void SetScale(UObject* NewScale);
	void SetScaleButtons(UObject* newDecButton, UObject* newIncButton);
	void SetValueField(UObject* newValueField);
	void StretchScaleField(BitfieldBool* bNewStretch);
	void StretchValueField(BitfieldBool* bNewStretch);
};

class UListWindow : public UWindow
{
public:
    using UWindow::UWindow;

	int AddRow(const std::string& rowStr, int* clientData);
	void AddSortColumn(int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive);
	void DeleteAllRows();
	void DeleteRow(int rowId);
	void EnableAutoExpandColumns(BitfieldBool* bAutoExpand);
	void EnableAutoSort(BitfieldBool* bAutoSort);
	void EnableHotKeys(BitfieldBool* bEnable);
	void EnableMultiSelect(BitfieldBool* bEnableMultiSelect);
	uint8_t GetColumnAlignment(int colIndex);
	void GetColumnColor(int colIndex, Color& colColor);
	UObject* GetColumnFont(int colIndex);
	std::string GetColumnTitle(int colIndex);
	uint8_t GetColumnType(int colIndex);
	float GetColumnWidth(int colIndex);
	std::string GetField(int rowId, int colIndex);
	void GetFieldMargins(float& marginWidth, float& marginHeight);
	float GetFieldValue(int rowId, int colIndex);
	int GetFocusRow();
	int GetNumColumns();
	int GetNumRows();
	int GetNumSelectedRows();
	int GetPageSize();
	int GetRowClientInt(int rowId);
	UObject* GetRowClientObject(int rowId);
	int GetSelectedRow();
	void HideColumn(int colIndex, BitfieldBool* bHide);
	int IndexToRowId(int index);
	bool IsAutoExpandColumnsEnabled();
	bool IsAutoSortEnabled();
	bool IsColumnHidden(int colIndex);
	bool IsMultiSelectEnabled();
	bool IsRowSelected(int rowId);
	void ModifyRow(int rowId, const std::string& rowStr);
	void MoveRow(uint8_t Move, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag);
	void PlayListSound(UObject* listSound, float* Volume, float* Pitch);
	void RemoveSortColumn(int colIndex);
	void ResetSortColumns(BitfieldBool* bSort);
	void ResizeColumns(BitfieldBool* bExpandOnly);
	int RowIdToIndex(int rowId);
	void SelectAllRows(BitfieldBool* bSelect);
	void SelectRow(int rowId, BitfieldBool* bSelect);
	void SelectToRow(int rowId, BitfieldBool* bClearRows, BitfieldBool* bInvert, BitfieldBool* bSpanRows);
	void SetColumnAlignment(int colIndex, uint8_t newAlign);
	void SetColumnColor(int colIndex, const Color& NewColor);
	void SetColumnFont(int colIndex, UObject* NewFont);
	void SetColumnTitle(int colIndex, const std::string& Title);
	void SetColumnType(int colIndex, uint8_t newType, std::string* newFmt);
	void SetColumnWidth(int colIndex, float newWidth);
	void SetDelimiter(const std::string& newDelimiter);
	void SetField(int rowId, int colIndex, const std::string& fieldStr);
	void SetFieldMargins(float newMarginWidth, float newMarginHeight);
	void SetFieldValue(int rowId, int colIndex, float NewValue);
	void SetFocusColor(const Color& NewColor);
	void SetFocusRow(int rowId, BitfieldBool* bMoveTo, BitfieldBool* bAnchor);
	void SetFocusTexture(UObject* NewTexture);
	void SetFocusThickness(float newThickness);
	void SetHighlightColor(const Color& NewColor);
	void SetHighlightTextColor(const Color& NewColor);
	void SetHighlightTexture(UObject* NewTexture);
	void SetHotKeyColumn(int colIndex);
	void SetListSounds(UObject** ActivateSound, UObject** moveSound);
	void SetNumColumns(int newCols);
	void SetRow(int rowId, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag);
	void SetRowClientInt(int rowId, int clientInt);
	void SetRowClientObject(int rowId, UObject* clientObj);
	void SetSortColumn(int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive);
	void ShowFocusRow();
	void Sort();
	void ToggleRowSelection(int rowId);
};

class UComputerWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void ClearLine(int rowToClear);
	void ClearScreen();
	void EnableWordWrap(BitfieldBool* bNewWordWrap);
	void FadeOutText(float* fadeDuration);
	void GetChar(const std::string& inputKey, BitfieldBool* bEcho);
	void GetInput(int MaxLength, const std::string& inputKey, std::string* defaultInputString, std::string* inputMask);
	float GetThrottle();
	bool IsBufferFlushed();
	bool IsPaused();
	void Pause(float* pauseLength);
	void PlaySoundLater(UObject* newsound);
	void Print(const std::string& printText, BitfieldBool* bNewLine);
	void PrintGraphic(UObject* Graphic, int Width, int Height, int* posX, int* posY, BitfieldBool* bStatic, BitfieldBool* bPixelPos);
	void PrintLn();
	void ResetThrottle();
	void Resume();
	void SetBackgroundTextures(UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6);
	void SetComputerSoundVolume(float newSoundVolume);
	void SetCursorBlinkSpeed(float newBlinkSpeed);
	void SetCursorColor(const Color& newCursorColor);
	void SetCursorTexture(UObject* newCursorTexture, int* newCursorWidth, int* newCursorHeight);
	void SetFadeSpeed(float fadeSpeed);
	void SetFontColor(const Color& newFontColor);
	void SetTextFont(UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor);
	void SetTextPosition(int posX, int posY);
	void SetTextSize(int newCols, int newRows);
	void SetTextSound(UObject* newTextSound);
	void SetTextTiming(float newTiming);
	void SetTextWindowPosition(int newX, int newY);
	void SetThrottle(float throttleModifier);
	void SetTypingSound(UObject* newTypingSound);
	void SetTypingSoundVolume(float newSoundVolume);
	void ShowTextCursor(BitfieldBool* bShow);
};

class UBorderWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void BaseMarginsFromBorder(BitfieldBool* bBorder);
	void EnableResizing(BitfieldBool* bResize);
	void SetBorderMargins(float* NewLeft, float* NewRight, float* newTop, float* newBottom);
	void SetBorders(UObject** bordTL, UObject** bordTR, UObject** bordBL, UObject** bordBR, UObject** bordL, UObject** bordR, UObject** bordT, UObject** bordB, UObject** center);
	void SetMoveCursors(UObject** Move, UObject** hMove, UObject** vMove, UObject** tlMove, UObject** trMove);
};

class UGC : public UObject
{
public:
	using UObject::UObject;

	void ClearZ();
	void CopyGC(UObject* Copy);
	void DrawActor(UObject* Actor, BitfieldBool* bClearZ, BitfieldBool* bConstrain, BitfieldBool* bUnlit, float* DrawScale, float* ScaleGlow, UObject** Skin);
	void DrawBorders(float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject* borders, BitfieldBool* bStretchHorizontally, BitfieldBool* bStretchVertically);
	void DrawBox(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX);
	void DrawIcon(float DestX, float DestY, UObject* tX);
	void DrawPattern(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX);
	void DrawStretchedTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX);
	void DrawText(float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr);
	void DrawTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX);
	void EnableDrawing(bool bDrawEnabled);
	void EnableMasking(bool bNewMasking);
	void EnableModulation(bool bNewModulation);
	void EnableSmoothing(bool bNewSmoothing);
	void EnableSpecialText(bool bNewSpecialText);
	void EnableTranslucency(bool bNewTranslucency);
	void EnableTranslucentText(bool bNewTranslucency);
	void EnableWordWrap(bool bNewWordWrap);
	void GetAlignments(uint8_t& HAlign, uint8_t& VAlign);
	float GetFontHeight(BitfieldBool* bIncludeSpace);
	void GetFonts(UObject*& normalFont, UObject*& boldFont);
	uint8_t GetHorizontalAlignment();
	uint8_t GetStyle();
	void GetTextColor(Color& TextColor);
	void GetTextExtent(float destWidth, float& xExtent, float& yExtent, const std::string& textStr);
	float GetTextVSpacing();
	void GetTileColor(Color& tileColor);
	uint8_t GetVerticalAlignment();
	void Intersect(float ClipX, float ClipY, float clipWidth, float clipHeight);
	bool IsDrawingEnabled();
	bool IsMaskingEnabled();
	bool IsModulationEnabled();
	bool IsSmoothingEnabled();
	bool IsSpecialTextEnabled();
	bool IsTranslucencyEnabled();
	bool IsTranslucentTextEnabled();
	bool IsWordWrapEnabled();
	void PopGC(int* gcNum);
	int PushGC();
	void SetAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetBaselineData(float* newBaselineOffset, float* newUnderlineHeight);
	void SetBoldFont(UObject* newBoldFont);
	void SetFont(UObject* NewFont);
	void SetFonts(UObject* newNormalFont, UObject* newBoldFont);
	void SetHorizontalAlignment(uint8_t newHAlign);
	void SetNormalFont(UObject* newNormalFont);
	void SetStyle(uint8_t NewStyle);
	void SetTextColor(const Color& newTextColor);
	void SetTextVSpacing(float newVSpacing);
	void SetTileColor(const Color& newTileColor);
	void SetVerticalAlignment(uint8_t newVAlign);
};
