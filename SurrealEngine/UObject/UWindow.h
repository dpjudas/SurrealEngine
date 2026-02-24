#pragma once

#include "UObject.h"

class UCanvas;
class UTexture;
class UGC;
class UFont;
class USound;
class UPlayerPawnExt;
class UScaleWindow;
class UScaleManagerWindow;
class UActor;

enum class EFlagType : uint8_t
{
	Bool,
	Byte,
	Int,
	Float,
	Name,
	Vector,
	Rotator,
};

enum class EHAlign : uint8_t
{
	Left,
	Center,
	Right,
	Full
};

enum class EVAlign : uint8_t
{
	Top,
	Center,
	Bottom,
	Full
};

enum class EHDirection : uint8_t
{
	LeftToRight,
	RightToLeft
};

enum class EVDirection : uint8_t
{
	VDIR_TopToBottom,
	VDIR_BottomToTop
};

enum class EOrder : uint8_t
{
	Right,
	Left,
	Down,
	Up,
	RightThenDown,
	RightThenUp,
	LeftThenDown,
	LeftThenUp,
	DownThenRight,
	DownThenLeft,
	UpThenRight,
	UpThenLeft
};

enum class EMouseFocusMode : uint8_t
{
	None,
	Click,
	Enter,
	EnterLeave
};

enum class EMove : uint8_t
{
	Left,
	Right,
	Up,
	Down
};

enum class EOrientation : uint8_t
{
	Horizontal,
	Vertical
};

enum class EMoveList : uint8_t
{
	Up,
	Down,
	PageUp,
	PageDown,
	Home,
	End
};

enum class EMoveInsert : uint8_t
{
	Up,
	Down,
	Left,
	Right,
	WordLeft,
	WordRight,
	StartOfLine,
	EndOfLine,
	PageUp,
	PageDown,
	Home,
	End
};

enum class EInsertionPointType : uint8_t
{
	Insert,
	Underscore,
	Block,
	RawInsert,
	RawOverlay
};

enum class EColumnType : uint8_t
{
	String,
	Float,
	Time
};

enum class EMoveThumb : uint8_t
{
	Home,
	End,
	Prev,
	Next,
	StepUp,
	StepDown,
	PageUp,
	PageDown
};

enum class EDrawStyle : uint8_t
{
	None,
	Normal,
	Masked,
	Translucent,
	Modulated
};

struct ClipRect
{
	float originX, originY;
	float clipX, clipY;
	float clipWidth, clipHeight;
};

class UExtensionObject : public UObject
{
public:
	using UObject::UObject;
};

class UWindow : public UExtensionObject
{
public:
	using UExtensionObject::UExtensionObject;

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

	void DetachFromParent();

	UTexture*& Background() { return Value<UTexture*>(PropOffsets_Window.Background); }
	float& Height() { return Value<float>(PropOffsets_Window.Height); }
	float& SoundVolume() { return Value<float>(PropOffsets_Window.SoundVolume); }
	Color& TextColor() { return Value<Color>(PropOffsets_Window.TextColor); }
	float& Width() { return Value<float>(PropOffsets_Window.Width); }
	float& X() { return Value<float>(PropOffsets_Window.X); }
	float& Y() { return Value<float>(PropOffsets_Window.Y); }
	int& acceleratorKey() { return Value<int>(PropOffsets_Window.acceleratorKey); }
	int& att() { return Value<int>(PropOffsets_Window.att); }
	BitfieldBool bBeingDestroyed() { return BoolValue(PropOffsets_Window.bBeingDestroyed); }
	BitfieldBool bConfigured() { return BoolValue(PropOffsets_Window.bConfigured); }
	BitfieldBool bDrawRawBackground() { return BoolValue(PropOffsets_Window.bDrawRawBackground); }
	BitfieldBool bIsInitialized() { return BoolValue(PropOffsets_Window.bIsInitialized); }
	BitfieldBool bIsSelectable() { return BoolValue(PropOffsets_Window.bIsSelectable); }
	BitfieldBool bIsSensitive() { return BoolValue(PropOffsets_Window.bIsSensitive); }
	BitfieldBool bIsVisible() { return BoolValue(PropOffsets_Window.bIsVisible); }
	BitfieldBool bLastHeightSpecified() { return BoolValue(PropOffsets_Window.bLastHeightSpecified); }
	BitfieldBool bLastWidthSpecified() { return BoolValue(PropOffsets_Window.bLastWidthSpecified); }
	BitfieldBool bNeedsQuery() { return BoolValue(PropOffsets_Window.bNeedsQuery); }
	BitfieldBool bNeedsReconfigure() { return BoolValue(PropOffsets_Window.bNeedsReconfigure); }
	BitfieldBool bSmoothBackground() { return BoolValue(PropOffsets_Window.bSmoothBackground); }
	BitfieldBool bSpecialText() { return BoolValue(PropOffsets_Window.bSpecialText); }
	BitfieldBool bStretchBackground() { return BoolValue(PropOffsets_Window.bStretchBackground); }
	BitfieldBool bTextTranslucent() { return BoolValue(PropOffsets_Window.bTextTranslucent); }
	BitfieldBool bTickEnabled() { return BoolValue(PropOffsets_Window.bTickEnabled); }
	uint8_t& backgroundStyle() { return Value<uint8_t>(PropOffsets_Window.backgroundStyle); }
	UFont*& boldFont() { return Value<UFont*>(PropOffsets_Window.boldFont); }
	UObject*& clientObject() { return Value<UObject*>(PropOffsets_Window.clientObject); }
	int& colMajorIndex() { return Value<int>(PropOffsets_Window.colMajorIndex); }
	UTexture*& defaultCursor() { return Value<UTexture*>(PropOffsets_Window.defaultCursor); }
	Color& defaultCursorColor() { return Value<Color>(PropOffsets_Window.defaultCursorColor); }
	UTexture*& defaultCursorShadow() { return Value<UTexture*>(PropOffsets_Window.defaultCursorShadow); }
	float& defaultHotX() { return Value<float>(PropOffsets_Window.defaultHotX); }
	float& defaultHotY() { return Value<float>(PropOffsets_Window.defaultHotY); }
	UWindow*& firstChild() { return Value<UWindow*>(PropOffsets_Window.firstChild); }
	int& firstTimer() { return Value<int>(PropOffsets_Window.firstTimer); }
	USound*& focusSound() { return Value<USound*>(PropOffsets_Window.focusSound); }
	int& freeTimer() { return Value<int>(PropOffsets_Window.freeTimer); }
	UGC*& gGc() { return Value<UGC*>(PropOffsets_Window.gGc); }
	float& hMargin0() { return Value<float>(PropOffsets_Window.hMargin0); }
	float& hMargin1() { return Value<float>(PropOffsets_Window.hMargin1); }
	float& hardcodedHeight() { return Value<float>(PropOffsets_Window.hardcodedHeight); }
	float& hardcodedWidth() { return Value<float>(PropOffsets_Window.hardcodedWidth); }
	float& holdHeight() { return Value<float>(PropOffsets_Window.holdHeight); }
	float& holdWidth() { return Value<float>(PropOffsets_Window.holdWidth); }
	float& holdX() { return Value<float>(PropOffsets_Window.holdX); }
	float& holdY() { return Value<float>(PropOffsets_Window.holdY); }
	USound*& invisibleSound() { return Value<USound*>(PropOffsets_Window.invisibleSound); }
	UWindow*& lastChild() { return Value<UWindow*>(PropOffsets_Window.lastChild); }
	float& lastQueryHeight() { return Value<float>(PropOffsets_Window.lastQueryHeight); }
	float& lastQueryWidth() { return Value<float>(PropOffsets_Window.lastQueryWidth); }
	float& lastSpecifiedHeight() { return Value<float>(PropOffsets_Window.lastSpecifiedHeight); }
	float& lastSpecifiedWidth() { return Value<float>(PropOffsets_Window.lastSpecifiedWidth); }
	int& lockCount() { return Value<int>(PropOffsets_Window.lockCount); }
	int& maxClicks() { return Value<int>(PropOffsets_Window.maxClicks); }
	UWindow*& nextSibling() { return Value<UWindow*>(PropOffsets_Window.nextSibling); }
	UFont*& normalFont() { return Value<UFont*>(PropOffsets_Window.normalFont); }
	UWindow*& parentOwner() { return Value<UWindow*>(PropOffsets_Window.parentOwner); }
	UWindow*& prevSibling() { return Value<UWindow*>(PropOffsets_Window.prevSibling); }
	int& rowMajorIndex() { return Value<int>(PropOffsets_Window.rowMajorIndex); }
	//Plane& textPlane() { return Value<Plane>(PropOffsets_Window.textPlane); }
	float& textVSpacing() { return Value<float>(PropOffsets_Window.textVSpacing); }
	Color& tileColor() { return Value<Color>(PropOffsets_Window.tileColor); }
	//Plane& tilePlane() { return Value<Plane>(PropOffsets_Window.tilePlane); }
	USound*& unfocusSound() { return Value<USound*>(PropOffsets_Window.unfocusSound); }
	float& vMargin0() { return Value<float>(PropOffsets_Window.vMargin0); }
	float& vMargin1() { return Value<float>(PropOffsets_Window.vMargin1); }
	USound*& visibleSound() { return Value<USound*>(PropOffsets_Window.visibleSound); }
	UGC*& wGc() { return Value<UGC*>(PropOffsets_Window.wGc); }
	ClipRect& winClipRect() { return Value<ClipRect>(PropOffsets_Window.winClipRect); }
	uint8_t& winHAlign() { return Value<uint8_t>(PropOffsets_Window.winHAlign); }
	UWindow*& winParent() { return Value<UWindow*>(PropOffsets_Window.winParent); }
	uint8_t& winVAlign() { return Value<uint8_t>(PropOffsets_Window.winVAlign); }
	uint8_t& windowType() { return Value<uint8_t>(PropOffsets_Window.windowType); }

	bool FirstDraw = true;
	bool FixedWidth = false;
	bool FixedHeight = false;
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

	Color& DefaultColor() { return Value<Color>(PropOffsets_ViewportWindow.DefaultColor); }
	UTexture*& DefaultTexture() { return Value<UTexture*>(PropOffsets_ViewportWindow.DefaultTexture); }
	float& FOV() { return Value<float>(PropOffsets_ViewportWindow.FOV); }
	vec3& Location() { return Value<vec3>(PropOffsets_ViewportWindow.Location); }
	Rotator& Rotation() { return Value<Rotator>(PropOffsets_ViewportWindow.Rotation); }
	BitfieldBool bClearZ() { return BoolValue(PropOffsets_ViewportWindow.bClearZ); }
	BitfieldBool bEnableViewport() { return BoolValue(PropOffsets_ViewportWindow.bEnableViewport); }
	BitfieldBool bOriginActorDestroyed() { return BoolValue(PropOffsets_ViewportWindow.bOriginActorDestroyed); }
	BitfieldBool bShowActor() { return BoolValue(PropOffsets_ViewportWindow.bShowActor); }
	BitfieldBool bShowWeapons() { return BoolValue(PropOffsets_ViewportWindow.bShowWeapons); }
	BitfieldBool bUseEyeHeight() { return BoolValue(PropOffsets_ViewportWindow.bUseEyeHeight); }
	BitfieldBool bUseViewRotation() { return BoolValue(PropOffsets_ViewportWindow.bUseViewRotation); }
	BitfieldBool bWatchEyeHeight() { return BoolValue(PropOffsets_ViewportWindow.bWatchEyeHeight); }
	vec3& lastLocation() { return Value<vec3>(PropOffsets_ViewportWindow.lastLocation); }
	Rotator& lastRotation() { return Value<Rotator>(PropOffsets_ViewportWindow.lastRotation); }
	UActor*& originActor() { return Value<UActor*>(PropOffsets_ViewportWindow.originActor); }
	vec3& relLocation() { return Value<vec3>(PropOffsets_ViewportWindow.relLocation); }
	Rotator& relRotation() { return Value<Rotator>(PropOffsets_ViewportWindow.relRotation); }
	int& viewportFrame() { return Value<int>(PropOffsets_ViewportWindow.viewportFrame); }
	UActor*& watchActor() { return Value<UActor*>(PropOffsets_ViewportWindow.watchActor); }
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

	BitfieldBool bEqualHeight() { return BoolValue(PropOffsets_TileWindow.bEqualHeight); }
	BitfieldBool bEqualWidth() { return BoolValue(PropOffsets_TileWindow.bEqualWidth); }
	BitfieldBool bFillParent() { return BoolValue(PropOffsets_TileWindow.bFillParent); }
	BitfieldBool bWrap() { return BoolValue(PropOffsets_TileWindow.bWrap); }
	uint8_t& hChildAlign() { return Value<uint8_t>(PropOffsets_TileWindow.hChildAlign); }
	uint8_t& hDirection() { return Value<uint8_t>(PropOffsets_TileWindow.hDirection); }
	float& hMargin() { return Value<float>(PropOffsets_TileWindow.hMargin); }
	float& majorSpacing() { return Value<float>(PropOffsets_TileWindow.majorSpacing); }
	float& minorSpacing() { return Value<float>(PropOffsets_TileWindow.minorSpacing); }
	uint8_t& orientation() { return Value<uint8_t>(PropOffsets_TileWindow.orientation); }
	//DynamicArray& rowArray() { return Value<DynamicArray>(PropOffsets_TileWindow.rowArray); }
	uint8_t& vChildAlign() { return Value<uint8_t>(PropOffsets_TileWindow.vChildAlign); }
	uint8_t& vDirection() { return Value<uint8_t>(PropOffsets_TileWindow.vDirection); }
	float& vMargin() { return Value<float>(PropOffsets_TileWindow.vMargin); }
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

	uint8_t& HAlign() { return Value<uint8_t>(PropOffsets_TextWindow.HAlign); }
	int& MaxLines() { return Value<int>(PropOffsets_TextWindow.MaxLines); }
	float& MinWidth() { return Value<float>(PropOffsets_TextWindow.MinWidth); }
	std::string& Text() { return Value<std::string>(PropOffsets_TextWindow.Text); }
	uint8_t& VAlign() { return Value<uint8_t>(PropOffsets_TextWindow.VAlign); }
	BitfieldBool bTextIsAccelerator() { return BoolValue(PropOffsets_TextWindow.bTextIsAccelerator); }
	BitfieldBool bWordWrap() { return BoolValue(PropOffsets_TextWindow.bWordWrap); }
	float& hMargin() { return Value<float>(PropOffsets_TextWindow.hMargin); }
	int& minLines() { return Value<int>(PropOffsets_TextWindow.minLines); }
	float& vMargin() { return Value<float>(PropOffsets_TextWindow.vMargin); }
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

	//ButtonDisplayInfo& Info() { return Value<ButtonDisplayInfo>(PropOffsets_ButtonWindow.Info); }
	float& activateDelay() { return Value<float>(PropOffsets_ButtonWindow.activateDelay); }
	int& activateTimer() { return Value<int>(PropOffsets_ButtonWindow.activateTimer); }
	BitfieldBool bAutoRepeat() { return BoolValue(PropOffsets_ButtonWindow.bAutoRepeat); }
	BitfieldBool bButtonPressed() { return BoolValue(PropOffsets_ButtonWindow.bButtonPressed); }
	BitfieldBool bEnableRightMouseClick() { return BoolValue(PropOffsets_ButtonWindow.bEnableRightMouseClick); }
	BitfieldBool bMousePressed() { return BoolValue(PropOffsets_ButtonWindow.bMousePressed); }
	USound*& clickSound() { return Value<USound*>(PropOffsets_ButtonWindow.clickSound); }
	Color& curTextColor() { return Value<Color>(PropOffsets_ButtonWindow.curTextColor); }
	UTexture*& curTexture() { return Value<UTexture*>(PropOffsets_ButtonWindow.curTexture); }
	Color& curTileColor() { return Value<Color>(PropOffsets_ButtonWindow.curTileColor); }
	float& initialDelay() { return Value<float>(PropOffsets_ButtonWindow.initialDelay); }
	uint8_t& lastInputKey() { return Value<uint8_t>(PropOffsets_ButtonWindow.lastInputKey); }
	USound*& pressSound() { return Value<USound*>(PropOffsets_ButtonWindow.pressSound); }
	float& repeatRate() { return Value<float>(PropOffsets_ButtonWindow.repeatRate); }
	float& repeatTime() { return Value<float>(PropOffsets_ButtonWindow.repeatTime); }
};

class UToggleWindow : public UButtonWindow
{
public:
    using UButtonWindow::UButtonWindow;

	void ChangeToggle();
	bool GetToggle();
	void SetToggle(bool bNewToggle);
	void SetToggleSounds(UObject** enableSound, UObject** disableSound);

	USound*& disableSound() { return Value<USound*>(PropOffsets_ToggleWindow.disableSound); }
	USound*& enableSound() { return Value<USound*>(PropOffsets_ToggleWindow.enableSound); }
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

	BitfieldBool bRightSide() { return BoolValue(PropOffsets_CheckboxWindow.bRightSide); }
	Color& checkboxColor() { return Value<Color>(PropOffsets_CheckboxWindow.checkboxColor); }
	float& checkboxSpacing() { return Value<float>(PropOffsets_CheckboxWindow.checkboxSpacing); }
	uint8_t& checkboxStyle() { return Value<uint8_t>(PropOffsets_CheckboxWindow.checkboxStyle); }
	float& textureHeight() { return Value<float>(PropOffsets_CheckboxWindow.textureHeight); }
	float& textureWidth() { return Value<float>(PropOffsets_CheckboxWindow.textureWidth); }
	UTexture*& toggleOff() { return Value<UTexture*>(PropOffsets_CheckboxWindow.toggleOff); }
	UTexture*& toggleOn() { return Value<UTexture*>(PropOffsets_CheckboxWindow.toggleOn); }
};

class UTextLogWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;

	void AddLog(const std::string& NewText, const Color& linecol);
	void ClearLog();
	void PauseLog(bool bNewPauseState);
	void SetTextTimeout(float newTimeout);

	//DynamicArray& Lines() { return Value<DynamicArray>(PropOffsets_TextLogWindow.Lines); }
	BitfieldBool bPaused() { return BoolValue(PropOffsets_TextLogWindow.bPaused); }
	BitfieldBool bTooTall() { return BoolValue(PropOffsets_TextLogWindow.bTooTall); }
	float& textTimeout() { return Value<float>(PropOffsets_TextLogWindow.textTimeout); }
};

class ULargeTextWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;

	void SetVerticalSpacing(float* newVSpace);

	float& lineHeight() { return Value<float>(PropOffsets_LargeTextWindow.lineHeight); }
	//DynamicArray& queryRowData() { return Value<DynamicArray>(PropOffsets_LargeTextWindow.queryRowData); }
	//XTextParams& queryTextParams() { return Value<XTextParams>(PropOffsets_LargeTextWindow.queryTextParams); }
	//DynamicArray& rowData() { return Value<DynamicArray>(PropOffsets_LargeTextWindow.rowData); }
	//DynamicArray& tempRowData() { return Value<DynamicArray>(PropOffsets_LargeTextWindow.tempRowData); }
	//XTextParams& textParams() { return Value<XTextParams>(PropOffsets_LargeTextWindow.textParams); }
	float& vSpace() { return Value<float>(PropOffsets_LargeTextWindow.vSpace); }
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

class UTabGroupWindow : public UWindow
{
public:
    using UWindow::UWindow;

	BitfieldBool bSizeChildrenToParent() { return BoolValue(PropOffsets_TabGroupWindow.bSizeChildrenToParent); }
	BitfieldBool bSizeParentToChildren() { return BoolValue(PropOffsets_TabGroupWindow.bSizeParentToChildren); }
	//DynamicArray& colMajorWindowList() { return Value<DynamicArray>(PropOffsets_TabGroupWindow.colMajorWindowList); }
	float& firstAbsX() { return Value<float>(PropOffsets_TabGroupWindow.firstAbsX); }
	float& firstAbsY() { return Value<float>(PropOffsets_TabGroupWindow.firstAbsY); }
	//DynamicArray& rowMajorWindowList() { return Value<DynamicArray>(PropOffsets_TabGroupWindow.rowMajorWindowList); }
	int& tabGroupIndex() { return Value<int>(PropOffsets_TabGroupWindow.tabGroupIndex); }
};

class URadioBoxWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;

	UObject* GetEnabledToggle();

	BitfieldBool bOneCheck() { return BoolValue(PropOffsets_RadioBoxWindow.bOneCheck); }
	UToggleWindow*& currentSelection() { return Value<UToggleWindow*>(PropOffsets_RadioBoxWindow.currentSelection); }
	//DynamicArray& toggleButtons() { return Value<DynamicArray>(PropOffsets_RadioBoxWindow.toggleButtons); }
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

	int& areaHSize() { return Value<int>(PropOffsets_ClipWindow.areaHSize); }
	int& areaVSize() { return Value<int>(PropOffsets_ClipWindow.areaVSize); }
	BitfieldBool bFillWindow() { return BoolValue(PropOffsets_ClipWindow.bFillWindow); }
	BitfieldBool bForceChildHeight() { return BoolValue(PropOffsets_ClipWindow.bForceChildHeight); }
	BitfieldBool bForceChildWidth() { return BoolValue(PropOffsets_ClipWindow.bForceChildWidth); }
	BitfieldBool bSnapToUnits() { return BoolValue(PropOffsets_ClipWindow.bSnapToUnits); }
	int& childH() { return Value<int>(PropOffsets_ClipWindow.childH); }
	int& childHSize() { return Value<int>(PropOffsets_ClipWindow.childHSize); }
	int& childV() { return Value<int>(PropOffsets_ClipWindow.childV); }
	int& childVSize() { return Value<int>(PropOffsets_ClipWindow.childVSize); }
	float& hMult() { return Value<float>(PropOffsets_ClipWindow.hMult); }
	int& prefHUnits() { return Value<int>(PropOffsets_ClipWindow.prefHUnits); }
	int& prefVUnits() { return Value<int>(PropOffsets_ClipWindow.prefVUnits); }
	float& vMult() { return Value<float>(PropOffsets_ClipWindow.vMult); }
};

class UModalWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;

	bool IsCurrentModal();
	void SetMouseFocusMode(uint8_t newFocusMode);

	UWindow*& acceleratorTable() { return Value<UWindow*>(PropOffsets_ModalWindow.acceleratorTable); }
	BitfieldBool bDirtyAccelerators() { return BoolValue(PropOffsets_ModalWindow.bDirtyAccelerators); }
	uint8_t& focusMode() { return Value<uint8_t>(PropOffsets_ModalWindow.focusMode); }
	UWindow*& preferredFocus() { return Value<UWindow*>(PropOffsets_ModalWindow.preferredFocus); }
	//DynamicArray& tabGroupWindowList() { return Value<DynamicArray>(PropOffsets_ModalWindow.tabGroupWindowList); }
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

	UTexture*& DefaultMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.DefaultMoveCursor); }
	UWindow*& FocusWindow() { return Value<UWindow*>(PropOffsets_RootWindow.FocusWindow); }
	float& MouseX() { return Value<float>(PropOffsets_RootWindow.MouseX); }
	float& MouseY() { return Value<float>(PropOffsets_RootWindow.MouseY); }
	int& TickCycles() { return Value<int>(PropOffsets_RootWindow.TickCycles); }
	BitfieldBool bClipRender() { return BoolValue(PropOffsets_RootWindow.bClipRender); }
	BitfieldBool bCursorVisible() { return BoolValue(PropOffsets_RootWindow.bCursorVisible); }
	BitfieldBool bMouseButtonLocked() { return BoolValue(PropOffsets_RootWindow.bMouseButtonLocked); }
	BitfieldBool bMouseMoveLocked() { return BoolValue(PropOffsets_RootWindow.bMouseMoveLocked); }
	BitfieldBool bMouseMoved() { return BoolValue(PropOffsets_RootWindow.bMouseMoved); }
	BitfieldBool bPositionalSound() { return BoolValue(PropOffsets_RootWindow.bPositionalSound); }
	BitfieldBool bRender() { return BoolValue(PropOffsets_RootWindow.bRender); }
	BitfieldBool bShowFrames() { return BoolValue(PropOffsets_RootWindow.bShowFrames); }
	BitfieldBool bShowStats() { return BoolValue(PropOffsets_RootWindow.bShowStats); }
	BitfieldBool bStretchRawBackground() { return BoolValue(PropOffsets_RootWindow.bStretchRawBackground); }
	int& clickCount() { return Value<int>(PropOffsets_RootWindow.clickCount); }
	UTexture*& debugTexture() { return Value<UTexture*>(PropOffsets_RootWindow.debugTexture); }
	UTexture*& defaultEditCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultEditCursor); }
	UTexture*& defaultHorizontalMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultHorizontalMoveCursor); }
	UTexture*& defaultTopLeftMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultTopLeftMoveCursor); }
	UTexture*& defaultTopRightMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultTopRightMoveCursor); }
	UTexture*& defaultVerticalMoveCursor() { return Value<UTexture*>(PropOffsets_RootWindow.defaultVerticalMoveCursor); }
	float& firstButtonMouseX() { return Value<float>(PropOffsets_RootWindow.firstButtonMouseX); }
	float& firstButtonMouseY() { return Value<float>(PropOffsets_RootWindow.firstButtonMouseY); }
	float& frameTimer() { return Value<float>(PropOffsets_RootWindow.frameTimer); }
	UWindow*& grabbedWindow() { return Value<UWindow*>(PropOffsets_RootWindow.grabbedWindow); }
	int& hMultiplier() { return Value<int>(PropOffsets_RootWindow.hMultiplier); }
	int& handleKeyboardRef() { return Value<int>(PropOffsets_RootWindow.handleKeyboardRef); }
	int& handleMouseRef() { return Value<int>(PropOffsets_RootWindow.handleMouseRef); }
	int& initCount() { return Value<int>(PropOffsets_RootWindow.initCount); }
	uint8_t& keyDownMap() { return Value<uint8_t>(PropOffsets_RootWindow.keyDownMap); }
	float& lastButtonPress() { return Value<float>(PropOffsets_RootWindow.lastButtonPress); }
	int& lastButtonType() { return Value<int>(PropOffsets_RootWindow.lastButtonType); }
	UWindow*& lastButtonWindow() { return Value<UWindow*>(PropOffsets_RootWindow.lastButtonWindow); }
	UWindow*& lastMouseWindow() { return Value<UWindow*>(PropOffsets_RootWindow.lastMouseWindow); }
	float& maxMouseDist() { return Value<float>(PropOffsets_RootWindow.maxMouseDist); }
	float& multiClickTimeout() { return Value<float>(PropOffsets_RootWindow.multiClickTimeout); }
	URootWindow*& nextRootWindow() { return Value<URootWindow*>(PropOffsets_RootWindow.nextRootWindow); }
	int& paintCycles() { return Value<int>(PropOffsets_RootWindow.paintCycles); }
	UPlayerPawnExt*& parentPawn() { return Value<UPlayerPawnExt*>(PropOffsets_RootWindow.parentPawn); }
	float& prevMouseX() { return Value<float>(PropOffsets_RootWindow.prevMouseX); }
	float& prevMouseY() { return Value<float>(PropOffsets_RootWindow.prevMouseY); }
	UTexture*& rawBackground() { return Value<UTexture*>(PropOffsets_RootWindow.rawBackground); }
	float& rawBackgroundHeight() { return Value<float>(PropOffsets_RootWindow.rawBackgroundHeight); }
	float& rawBackgroundWidth() { return Value<float>(PropOffsets_RootWindow.rawBackgroundWidth); }
	Color& rawColor() { return Value<Color>(PropOffsets_RootWindow.rawColor); }
	float& renderHeight() { return Value<float>(PropOffsets_RootWindow.renderHeight); }
	float& renderWidth() { return Value<float>(PropOffsets_RootWindow.renderWidth); }
	float& renderX() { return Value<float>(PropOffsets_RootWindow.renderX); }
	float& renderY() { return Value<float>(PropOffsets_RootWindow.renderY); }
	int& rootFrame() { return Value<int>(PropOffsets_RootWindow.rootFrame); }
	int& snapshotHeight() { return Value<int>(PropOffsets_RootWindow.snapshotHeight); }
	int& snapshotWidth() { return Value<int>(PropOffsets_RootWindow.snapshotWidth); }
	int& vMultiplier() { return Value<int>(PropOffsets_RootWindow.vMultiplier); }
};

class UScrollAreaWindow : public UWindow
{
public:
    using UWindow::UWindow;

	void AutoHideScrollbars(BitfieldBool* bHide);
	void EnableScrolling(BitfieldBool* bHScrolling, BitfieldBool* bVScrolling);
	void SetAreaMargins(float newMarginWidth, float newMarginHeight);
	void SetScrollbarDistance(float newDistance);

	UClipWindow*& ClipWindow() { return Value<UClipWindow*>(PropOffsets_ScrollAreaWindow.ClipWindow); }
	UButtonWindow*& DownButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.DownButton); }
	UButtonWindow*& LeftButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.LeftButton); }
	UButtonWindow*& RightButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.RightButton); }
	UButtonWindow*& UpButton() { return Value<UButtonWindow*>(PropOffsets_ScrollAreaWindow.UpButton); }
	BitfieldBool bHLastShow() { return BoolValue(PropOffsets_ScrollAreaWindow.bHLastShow); }
	BitfieldBool bHideScrollbars() { return BoolValue(PropOffsets_ScrollAreaWindow.bHideScrollbars); }
	BitfieldBool bVLastShow() { return BoolValue(PropOffsets_ScrollAreaWindow.bVLastShow); }
	UScaleWindow*& hScale() { return Value<UScaleWindow*>(PropOffsets_ScrollAreaWindow.hScale); }
	UScaleManagerWindow*& hScaleMgr() { return Value<UScaleManagerWindow*>(PropOffsets_ScrollAreaWindow.hScaleMgr); }
	float& marginHeight() { return Value<float>(PropOffsets_ScrollAreaWindow.marginHeight); }
	float& marginWidth() { return Value<float>(PropOffsets_ScrollAreaWindow.marginWidth); }
	float& scrollbarDistance() { return Value<float>(PropOffsets_ScrollAreaWindow.scrollbarDistance); }
	UScaleWindow*& vScale() { return Value<UScaleWindow*>(PropOffsets_ScrollAreaWindow.vScale); }
	UScaleManagerWindow*& vScaleMgr() { return Value<UScaleManagerWindow*>(PropOffsets_ScrollAreaWindow.vScaleMgr); }
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

	float& RemainingTime() { return Value<float>(PropOffsets_ScaleWindow.RemainingTime); }
	float& ThumbHeight() { return Value<float>(PropOffsets_ScaleWindow.ThumbHeight); }
	float& ThumbWidth() { return Value<float>(PropOffsets_ScaleWindow.ThumbWidth); }
	float& absEndScale() { return Value<float>(PropOffsets_ScaleWindow.absEndScale); }
	float& absStartScale() { return Value<float>(PropOffsets_ScaleWindow.absStartScale); }
	BitfieldBool bDraggingThumb() { return BoolValue(PropOffsets_ScaleWindow.bDraggingThumb); }
	BitfieldBool bDrawEndTicks() { return BoolValue(PropOffsets_ScaleWindow.bDrawEndTicks); }
	BitfieldBool bRepeatScaleTexture() { return BoolValue(PropOffsets_ScaleWindow.bRepeatScaleTexture); }
	BitfieldBool bRepeatThumbTexture() { return BoolValue(PropOffsets_ScaleWindow.bRepeatThumbTexture); }
	BitfieldBool bSpanThumb() { return BoolValue(PropOffsets_ScaleWindow.bSpanThumb); }
	BitfieldBool bStretchScale() { return BoolValue(PropOffsets_ScaleWindow.bStretchScale); }
	UTexture*& borderPattern() { return Value<UTexture*>(PropOffsets_ScaleWindow.borderPattern); }
	USound*& clickSound() { return Value<USound*>(PropOffsets_ScaleWindow.clickSound); }
	int& currentPos() { return Value<int>(PropOffsets_ScaleWindow.currentPos); }
	USound*& dragSound() { return Value<USound*>(PropOffsets_ScaleWindow.dragSound); }
	float& endOffset() { return Value<float>(PropOffsets_ScaleWindow.endOffset); }
	//DynamicArray& enumStrings() { return Value<DynamicArray>(PropOffsets_ScaleWindow.enumStrings); }
	float& fromValue() { return Value<float>(PropOffsets_ScaleWindow.fromValue); }
	float& initialDelay() { return Value<float>(PropOffsets_ScaleWindow.initialDelay); }
	int& initialPos() { return Value<int>(PropOffsets_ScaleWindow.initialPos); }
	float& marginHeight() { return Value<float>(PropOffsets_ScaleWindow.marginHeight); }
	float& marginWidth() { return Value<float>(PropOffsets_ScaleWindow.marginWidth); }
	float& mousePos() { return Value<float>(PropOffsets_ScaleWindow.mousePos); }
	int& numPositions() { return Value<int>(PropOffsets_ScaleWindow.numPositions); }
	uint8_t& orientation() { return Value<uint8_t>(PropOffsets_ScaleWindow.orientation); }
	float& postCapH() { return Value<float>(PropOffsets_ScaleWindow.postCapH); }
	float& postCapHeight() { return Value<float>(PropOffsets_ScaleWindow.postCapHeight); }
	UTexture*& postCapTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.postCapTexture); }
	float& postCapW() { return Value<float>(PropOffsets_ScaleWindow.postCapW); }
	float& postCapWidth() { return Value<float>(PropOffsets_ScaleWindow.postCapWidth); }
	float& postCapXOff() { return Value<float>(PropOffsets_ScaleWindow.postCapXOff); }
	float& postCapYOff() { return Value<float>(PropOffsets_ScaleWindow.postCapYOff); }
	float& preCapH() { return Value<float>(PropOffsets_ScaleWindow.preCapH); }
	float& preCapHeight() { return Value<float>(PropOffsets_ScaleWindow.preCapHeight); }
	UTexture*& preCapTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.preCapTexture); }
	float& preCapW() { return Value<float>(PropOffsets_ScaleWindow.preCapW); }
	float& preCapWidth() { return Value<float>(PropOffsets_ScaleWindow.preCapWidth); }
	float& preCapXOff() { return Value<float>(PropOffsets_ScaleWindow.preCapXOff); }
	float& preCapYOff() { return Value<float>(PropOffsets_ScaleWindow.preCapYOff); }
	uint8_t& repeatDir() { return Value<uint8_t>(PropOffsets_ScaleWindow.repeatDir); }
	float& repeatRate() { return Value<float>(PropOffsets_ScaleWindow.repeatRate); }
	Color& scaleBorderColor() { return Value<Color>(PropOffsets_ScaleWindow.scaleBorderColor); }
	float& scaleBorderSize() { return Value<float>(PropOffsets_ScaleWindow.scaleBorderSize); }
	Color& scaleColor() { return Value<Color>(PropOffsets_ScaleWindow.scaleColor); }
	float& scaleH() { return Value<float>(PropOffsets_ScaleWindow.scaleH); }
	float& scaleHeight() { return Value<float>(PropOffsets_ScaleWindow.scaleHeight); }
	uint8_t& scaleStyle() { return Value<uint8_t>(PropOffsets_ScaleWindow.scaleStyle); }
	UTexture*& scaleTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.scaleTexture); }
	float& scaleW() { return Value<float>(PropOffsets_ScaleWindow.scaleW); }
	float& scaleWidth() { return Value<float>(PropOffsets_ScaleWindow.scaleWidth); }
	float& scaleX() { return Value<float>(PropOffsets_ScaleWindow.scaleX); }
	float& scaleY() { return Value<float>(PropOffsets_ScaleWindow.scaleY); }
	USound*& setSound() { return Value<USound*>(PropOffsets_ScaleWindow.setSound); }
	int& spanRange() { return Value<int>(PropOffsets_ScaleWindow.spanRange); }
	float& startOffset() { return Value<float>(PropOffsets_ScaleWindow.startOffset); }
	Color& thumbBorderColor() { return Value<Color>(PropOffsets_ScaleWindow.thumbBorderColor); }
	float& thumbBorderSize() { return Value<float>(PropOffsets_ScaleWindow.thumbBorderSize); }
	Color& thumbColor() { return Value<Color>(PropOffsets_ScaleWindow.thumbColor); }
	float& thumbH() { return Value<float>(PropOffsets_ScaleWindow.thumbH); }
	int& thumbStep() { return Value<int>(PropOffsets_ScaleWindow.thumbStep); }
	uint8_t& thumbStyle() { return Value<uint8_t>(PropOffsets_ScaleWindow.thumbStyle); }
	UTexture*& thumbTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.thumbTexture); }
	float& thumbW() { return Value<float>(PropOffsets_ScaleWindow.thumbW); }
	float& thumbX() { return Value<float>(PropOffsets_ScaleWindow.thumbX); }
	float& thumbY() { return Value<float>(PropOffsets_ScaleWindow.thumbY); }
	Color& tickColor() { return Value<Color>(PropOffsets_ScaleWindow.tickColor); }
	float& tickH() { return Value<float>(PropOffsets_ScaleWindow.tickH); }
	float& tickHeight() { return Value<float>(PropOffsets_ScaleWindow.tickHeight); }
	uint8_t& tickStyle() { return Value<uint8_t>(PropOffsets_ScaleWindow.tickStyle); }
	UTexture*& tickTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.tickTexture); }
	float& tickW() { return Value<float>(PropOffsets_ScaleWindow.tickW); }
	float& tickWidth() { return Value<float>(PropOffsets_ScaleWindow.tickWidth); }
	float& tickX() { return Value<float>(PropOffsets_ScaleWindow.tickX); }
	float& tickY() { return Value<float>(PropOffsets_ScaleWindow.tickY); }
	float& toValue() { return Value<float>(PropOffsets_ScaleWindow.toValue); }
	std::string& valueFmt() { return Value<std::string>(PropOffsets_ScaleWindow.valueFmt); }
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

	UScaleWindow*& Scale() { return Value<UScaleWindow*>(PropOffsets_ScaleManagerWindow.Scale); }
	float& Spacing() { return Value<float>(PropOffsets_ScaleManagerWindow.Spacing); }
	BitfieldBool bStretchScaleField() { return BoolValue(PropOffsets_ScaleManagerWindow.bStretchScaleField); }
	BitfieldBool bStretchValueField() { return BoolValue(PropOffsets_ScaleManagerWindow.bStretchValueField); }
	uint8_t& childHAlign() { return Value<uint8_t>(PropOffsets_ScaleManagerWindow.childHAlign); }
	uint8_t& childVAlign() { return Value<uint8_t>(PropOffsets_ScaleManagerWindow.childVAlign); }
	UButtonWindow*& decButton() { return Value<UButtonWindow*>(PropOffsets_ScaleManagerWindow.decButton); }
	UButtonWindow*& incButton() { return Value<UButtonWindow*>(PropOffsets_ScaleManagerWindow.incButton); }
	float& marginHeight() { return Value<float>(PropOffsets_ScaleManagerWindow.marginHeight); }
	float& marginWidth() { return Value<float>(PropOffsets_ScaleManagerWindow.marginWidth); }
	uint8_t& orientation() { return Value<uint8_t>(PropOffsets_ScaleManagerWindow.orientation); }
	UTextWindow*& valueField() { return Value<UTextWindow*>(PropOffsets_ScaleManagerWindow.valueField); }
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

	USound*& ActivateSound() { return Value<USound*>(PropOffsets_ListWindow.ActivateSound); }
	std::string& Delimiter() { return Value<std::string>(PropOffsets_ListWindow.Delimiter); }
	int& anchorLine() { return Value<int>(PropOffsets_ListWindow.anchorLine); }
	BitfieldBool bAutoExpandColumns() { return BoolValue(PropOffsets_ListWindow.bAutoExpandColumns); }
	BitfieldBool bAutoSort() { return BoolValue(PropOffsets_ListWindow.bAutoSort); }
	BitfieldBool bDragging() { return BoolValue(PropOffsets_ListWindow.bDragging); }
	BitfieldBool bHotKeys() { return BoolValue(PropOffsets_ListWindow.bHotKeys); }
	BitfieldBool bMultiSelect() { return BoolValue(PropOffsets_ListWindow.bMultiSelect); }
	float& colMargin() { return Value<float>(PropOffsets_ListWindow.colMargin); }
	//DynamicArray& cols() { return Value<DynamicArray>(PropOffsets_ListWindow.cols); }
	Color& focusColor() { return Value<Color>(PropOffsets_ListWindow.focusColor); }
	int& focusLine() { return Value<int>(PropOffsets_ListWindow.focusLine); }
	UTexture*& focusTexture() { return Value<UTexture*>(PropOffsets_ListWindow.focusTexture); }
	float& focusThickness() { return Value<float>(PropOffsets_ListWindow.focusThickness); }
	Color& highlightColor() { return Value<Color>(PropOffsets_ListWindow.highlightColor); }
	UTexture*& highlightTexture() { return Value<UTexture*>(PropOffsets_ListWindow.highlightTexture); }
	int& hotKeyCol() { return Value<int>(PropOffsets_ListWindow.hotKeyCol); }
	std::string& hotKeyString() { return Value<std::string>(PropOffsets_ListWindow.hotKeyString); }
	float& hotKeyTimer() { return Value<float>(PropOffsets_ListWindow.hotKeyTimer); }
	Color& inverseColor() { return Value<Color>(PropOffsets_ListWindow.inverseColor); }
	int& lastIndex() { return Value<int>(PropOffsets_ListWindow.lastIndex); }
	float& lineSize() { return Value<float>(PropOffsets_ListWindow.lineSize); }
	USound*& moveSound() { return Value<USound*>(PropOffsets_ListWindow.moveSound); }
	int& numSelected() { return Value<int>(PropOffsets_ListWindow.numSelected); }
	float& remainingDelay() { return Value<float>(PropOffsets_ListWindow.remainingDelay); }
	float& rowMargin() { return Value<float>(PropOffsets_ListWindow.rowMargin); }
	//DynamicArray& rows() { return Value<DynamicArray>(PropOffsets_ListWindow.rows); }
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

	Color& FontColor() { return Value<Color>(PropOffsets_ComputerWindow.FontColor); }
	UPlayerPawnExt*& Player() { return Value<UPlayerPawnExt*>(PropOffsets_ComputerWindow.Player); }
	UWindow*& TextWindow() { return Value<UWindow*>(PropOffsets_ComputerWindow.TextWindow); }
	int& TextX() { return Value<int>(PropOffsets_ComputerWindow.TextX); }
	int& TextY() { return Value<int>(PropOffsets_ComputerWindow.TextY); }
	BitfieldBool bComputerStartInvoked() { return BoolValue(PropOffsets_ComputerWindow.bComputerStartInvoked); }
	BitfieldBool bCursorVisible() { return BoolValue(PropOffsets_ComputerWindow.bCursorVisible); }
	BitfieldBool bEchoKey() { return BoolValue(PropOffsets_ComputerWindow.bEchoKey); }
	BitfieldBool bFirstTick() { return BoolValue(PropOffsets_ComputerWindow.bFirstTick); }
	BitfieldBool bGamePaused() { return BoolValue(PropOffsets_ComputerWindow.bGamePaused); }
	BitfieldBool bIgnoreGamePaused() { return BoolValue(PropOffsets_ComputerWindow.bIgnoreGamePaused); }
	BitfieldBool bIgnoreTick() { return BoolValue(PropOffsets_ComputerWindow.bIgnoreTick); }
	BitfieldBool bInvokeComputerStart() { return BoolValue(PropOffsets_ComputerWindow.bInvokeComputerStart); }
	BitfieldBool bLastLineWrapped() { return BoolValue(PropOffsets_ComputerWindow.bLastLineWrapped); }
	BitfieldBool bPauseProcessing() { return BoolValue(PropOffsets_ComputerWindow.bPauseProcessing); }
	BitfieldBool bShowCursor() { return BoolValue(PropOffsets_ComputerWindow.bShowCursor); }
	BitfieldBool bWaitingForKey() { return BoolValue(PropOffsets_ComputerWindow.bWaitingForKey); }
	BitfieldBool bWordWrap() { return BoolValue(PropOffsets_ComputerWindow.bWordWrap); }
	UTexture*& backgroundTextures() { return Value<UTexture*>(PropOffsets_ComputerWindow.backgroundTextures); }
	Color& colGraphicTile() { return Value<Color>(PropOffsets_ComputerWindow.colGraphicTile); }
	float& computerSoundVolume() { return Value<float>(PropOffsets_ComputerWindow.computerSoundVolume); }
	float& cursorBlinkSpeed() { return Value<float>(PropOffsets_ComputerWindow.cursorBlinkSpeed); }
	Color& cursorColor() { return Value<Color>(PropOffsets_ComputerWindow.cursorColor); }
	int& cursorHeight() { return Value<int>(PropOffsets_ComputerWindow.cursorHeight); }
	float& cursorNextEvent() { return Value<float>(PropOffsets_ComputerWindow.cursorNextEvent); }
	UTexture*& cursorTexture() { return Value<UTexture*>(PropOffsets_ComputerWindow.cursorTexture); }
	int& cursorWidth() { return Value<int>(PropOffsets_ComputerWindow.cursorWidth); }
	//DynamicArray& displayBuffer() { return Value<DynamicArray>(PropOffsets_ComputerWindow.displayBuffer); }
	float& eventTimeInterval() { return Value<float>(PropOffsets_ComputerWindow.eventTimeInterval); }
	float& fadeOutStart() { return Value<float>(PropOffsets_ComputerWindow.fadeOutStart); }
	float& fadeOutTimer() { return Value<float>(PropOffsets_ComputerWindow.fadeOutTimer); }
	float& fadeSpeed() { return Value<float>(PropOffsets_ComputerWindow.fadeSpeed); }
	int& fontHeight() { return Value<int>(PropOffsets_ComputerWindow.fontHeight); }
	int& fontWidth() { return Value<int>(PropOffsets_ComputerWindow.fontWidth); }
	std::string& inputKey() { return Value<std::string>(PropOffsets_ComputerWindow.inputKey); }
	std::string& inputMask() { return Value<std::string>(PropOffsets_ComputerWindow.inputMask); }
	UEditWindow*& inputWindow() { return Value<UEditWindow*>(PropOffsets_ComputerWindow.inputWindow); }
	//DynamicArray& queuedBuffer() { return Value<DynamicArray>(PropOffsets_ComputerWindow.queuedBuffer); }
	int& queuedBufferStart() { return Value<int>(PropOffsets_ComputerWindow.queuedBufferStart); }
	int& textCols() { return Value<int>(PropOffsets_ComputerWindow.textCols); }
	UFont*& textFont() { return Value<UFont*>(PropOffsets_ComputerWindow.textFont); }
	int& textRows() { return Value<int>(PropOffsets_ComputerWindow.textRows); }
	USound*& textSound() { return Value<USound*>(PropOffsets_ComputerWindow.textSound); }
	float& throttle() { return Value<float>(PropOffsets_ComputerWindow.throttle); }
	float& timeCurrent() { return Value<float>(PropOffsets_ComputerWindow.timeCurrent); }
	float& timeLastEvent() { return Value<float>(PropOffsets_ComputerWindow.timeLastEvent); }
	float& timeNextEvent() { return Value<float>(PropOffsets_ComputerWindow.timeNextEvent); }
	USound*& typingSound() { return Value<USound*>(PropOffsets_ComputerWindow.typingSound); }
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

	float& BottomMargin() { return Value<float>(PropOffsets_BorderWindow.BottomMargin); }
	float& DragX() { return Value<float>(PropOffsets_BorderWindow.DragX); }
	float& DragY() { return Value<float>(PropOffsets_BorderWindow.DragY); }
	UTexture*& MoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.MoveCursor); }
	float& TopMargin() { return Value<float>(PropOffsets_BorderWindow.TopMargin); }
	BitfieldBool bDownDrag() { return BoolValue(PropOffsets_BorderWindow.bDownDrag); }
	BitfieldBool bLeftDrag() { return BoolValue(PropOffsets_BorderWindow.bLeftDrag); }
	BitfieldBool bMarginsFromBorder() { return BoolValue(PropOffsets_BorderWindow.bMarginsFromBorder); }
	BitfieldBool bResizeable() { return BoolValue(PropOffsets_BorderWindow.bResizeable); }
	BitfieldBool bRightDrag() { return BoolValue(PropOffsets_BorderWindow.bRightDrag); }
	BitfieldBool bSmoothBorder() { return BoolValue(PropOffsets_BorderWindow.bSmoothBorder); }
	BitfieldBool bStretchBorder() { return BoolValue(PropOffsets_BorderWindow.bStretchBorder); }
	BitfieldBool bUpDrag() { return BoolValue(PropOffsets_BorderWindow.bUpDrag); }
	UTexture*& borderBottom() { return Value<UTexture*>(PropOffsets_BorderWindow.borderBottom); }
	UTexture*& borderBottomLeft() { return Value<UTexture*>(PropOffsets_BorderWindow.borderBottomLeft); }
	UTexture*& borderBottomRight() { return Value<UTexture*>(PropOffsets_BorderWindow.borderBottomRight); }
	UTexture*& borderLeft() { return Value<UTexture*>(PropOffsets_BorderWindow.borderLeft); }
	UTexture*& borderRight() { return Value<UTexture*>(PropOffsets_BorderWindow.borderRight); }
	uint8_t& borderStyle() { return Value<uint8_t>(PropOffsets_BorderWindow.borderStyle); }
	UTexture*& borderTop() { return Value<UTexture*>(PropOffsets_BorderWindow.borderTop); }
	UTexture*& borderTopLeft() { return Value<UTexture*>(PropOffsets_BorderWindow.borderTopLeft); }
	UTexture*& borderTopRight() { return Value<UTexture*>(PropOffsets_BorderWindow.borderTopRight); }
	UTexture*& center() { return Value<UTexture*>(PropOffsets_BorderWindow.center); }
	float& childBottomMargin() { return Value<float>(PropOffsets_BorderWindow.childBottomMargin); }
	float& childLeftMargin() { return Value<float>(PropOffsets_BorderWindow.childLeftMargin); }
	float& childRightMargin() { return Value<float>(PropOffsets_BorderWindow.childRightMargin); }
	float& childTopMargin() { return Value<float>(PropOffsets_BorderWindow.childTopMargin); }
	float& dragHeight() { return Value<float>(PropOffsets_BorderWindow.dragHeight); }
	float& dragWidth() { return Value<float>(PropOffsets_BorderWindow.dragWidth); }
	UTexture*& hMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.hMoveCursor); }
	float& lastMouseX() { return Value<float>(PropOffsets_BorderWindow.lastMouseX); }
	float& lastMouseY() { return Value<float>(PropOffsets_BorderWindow.lastMouseY); }
	float& leftMargin() { return Value<float>(PropOffsets_BorderWindow.leftMargin); }
	float& rightMargin() { return Value<float>(PropOffsets_BorderWindow.rightMargin); }
	UTexture*& tlMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.tlMoveCursor); }
	UTexture*& trMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.trMoveCursor); }
	UTexture*& vMoveCursor() { return Value<UTexture*>(PropOffsets_BorderWindow.vMoveCursor); }
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

	UCanvas*& Canvas() { return Value<UCanvas*>(PropOffsets_GC.Canvas); }
	uint8_t& HAlign() { return Value<uint8_t>(PropOffsets_GC.HAlign); }
	int& PolyFlags() { return Value<int>(PropOffsets_GC.PolyFlags); }
	uint8_t& Style() { return Value<uint8_t>(PropOffsets_GC.Style); }
	Color& TextColor() { return Value<Color>(PropOffsets_GC.TextColor); }
	uint8_t& VAlign() { return Value<uint8_t>(PropOffsets_GC.VAlign); }
	BitfieldBool bDrawEnabled() { return BoolValue(PropOffsets_GC.bDrawEnabled); }
	BitfieldBool bFree() { return BoolValue(PropOffsets_GC.bFree); }
	BitfieldBool bMasked() { return BoolValue(PropOffsets_GC.bMasked); }
	BitfieldBool bModulated() { return BoolValue(PropOffsets_GC.bModulated); }
	BitfieldBool bParseMetachars() { return BoolValue(PropOffsets_GC.bParseMetachars); }
	BitfieldBool bSmoothed() { return BoolValue(PropOffsets_GC.bSmoothed); }
	BitfieldBool bTextTranslucent() { return BoolValue(PropOffsets_GC.bTextTranslucent); }
	BitfieldBool bTranslucent() { return BoolValue(PropOffsets_GC.bTranslucent); }
	BitfieldBool bWordWrap() { return BoolValue(PropOffsets_GC.bWordWrap); }
	float& baselineOffset() { return Value<float>(PropOffsets_GC.baselineOffset); }
	UFont*& boldFont() { return Value<UFont*>(PropOffsets_GC.boldFont); }
	ClipRect& gcClipRect() { return Value<ClipRect>(PropOffsets_GC.gcClipRect); }
	int& gcCount() { return Value<int>(PropOffsets_GC.gcCount); }
	UGC*& gcFree() { return Value<UGC*>(PropOffsets_GC.gcFree); }
	UGC*& gcOwner() { return Value<UGC*>(PropOffsets_GC.gcOwner); }
	UGC*& gcStack() { return Value<UGC*>(PropOffsets_GC.gcStack); }
	int& hMultiplier() { return Value<int>(PropOffsets_GC.hMultiplier); }
	UFont*& normalFont() { return Value<UFont*>(PropOffsets_GC.normalFont); }
	//Plane& textPlane() { return Value<Plane>(PropOffsets_GC.textPlane); }
	int& textPolyFlags() { return Value<int>(PropOffsets_GC.textPolyFlags); }
	float& textVSpacing() { return Value<float>(PropOffsets_GC.textVSpacing); }
	Color& tileColor() { return Value<Color>(PropOffsets_GC.tileColor); }
	//Plane& tilePlane() { return Value<Plane>(PropOffsets_GC.tilePlane); }
	float& underlineHeight() { return Value<float>(PropOffsets_GC.underlineHeight); }
	UTexture*& underlineTexture() { return Value<UTexture*>(PropOffsets_GC.underlineTexture); }
	int& vMultiplier() { return Value<int>(PropOffsets_GC.vMultiplier); }

	// To do: apply cliprect?
	float offsetX = 0.0f;
	float offsetY = 0.0f;
};

struct ActorRef  
{  
    UActor* Actor = nullptr;  
    int RefCount = 0;  
};  