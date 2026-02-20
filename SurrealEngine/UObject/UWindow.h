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
};

class UScaleManagerWindow : public UWindow
{
public:
    using UWindow::UWindow;
};

class UListWindow : public UWindow
{
public:
    using UWindow::UWindow;
};

class UComputerWindow : public UWindow
{
public:
    using UWindow::UWindow;
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
};
