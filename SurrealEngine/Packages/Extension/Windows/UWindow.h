#pragma once

#include "Packages/Extension/UExtensionObject.h"
#include "GameWindow.h"

class UCanvas;
class UTexture;
class UGC;
class UFont;
class USound;
class UPlayerPawnExt;
class UScaleWindow;
class UScaleManagerWindow;
class UActor;
class URootWindow;
enum EInputKey;
enum EInputType;

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
	TopToBottom,
	BottomToTop
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

class UWindow : public UExtensionObject
{
public:
	using UExtensionObject::UExtensionObject;

	void AddActorRef(UObject* refActor);
	int AddTimer(float TimeOut, std::optional<bool> bLoop, std::optional<int> clientData, std::optional<NameString> functionName);
	void AskParentForReconfigure();
	void AskParentToShowArea(std::optional<float> areaX, std::optional<float> areaY, std::optional<float> areaWidth, std::optional<float> areaHeight);
	std::string CarriageReturn();
	void ChangeStyle();
	void ConfigureChild(float newX, float newY, float newWidth, float NewHeight);
	void ConvertCoordinates(UWindow* fromWin, float fromX, float fromY, UWindow* toWin, float& toX, float& toY);
	std::string ConvertScriptString(const std::string& oldStr);
	bool ConvertVectorToCoordinates(const vec3& Location, float& relativeX, float& relativeY);
	void Destroy();
	void DestroyAllChildren();
	void DisableWindow();
	void EnableSpecialText(std::optional<bool> bEnable);
	void EnableTranslucentText(std::optional<bool> bEnable);
	void EnableWindow(std::optional<bool> bEnable);
	UWindow* FindWindow(float pointX, float pointY, float& relativeX, float& relativeY);
	UObject* GetBottomChild(std::optional<bool> bVisibleOnly);
	UObject* GetClientObject();
	void GetCursorPos(float& MouseX, float& MouseY);
	UObject* GetFocusWindow();
	UObject* GetGC();
	UObject* GetHigherSibling(std::optional<bool> bVisibleOnly);
	UObject* GetLowerSibling(std::optional<bool> bVisibleOnly);
	UObject* GetModalWindow();
	UObject* GetParent();
	UObject* GetPlayerPawn();
	URootWindow* GetRootWindow();
	UObject* GetTabGroupWindow();
	float GetTickOffset();
	UObject* GetTopChild(std::optional<bool> bVisibleOnly);
	void GrabMouse();
	void Hide();
	bool IsActorValid(UObject* refActor);
	bool IsFocusWindow();
	bool IsKeyDown(uint8_t Key);
	bool IsPointInWindow(float pointX, float pointY);
	bool IsSensitive(std::optional<bool> bRecurse);
	bool IsVisible(std::optional<bool> bRecurse);
	void Lower();
	//UObject* MoveFocus(EMove direction);
	UObject* MoveFocusDown();
	UObject* MoveFocusLeft();
	UObject* MoveFocusRight();
	UObject* MoveFocusUp();
	//UObject* MoveTabGroup(EMove dir);
	UObject* MoveTabGroupNext();
	UObject* MoveTabGroupPrev();
	UObject* NewChild(UObject* NewClass, std::optional<bool> bShow = {});
	void PlaySound(UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch, std::optional<float> posX, std::optional<float> posY);
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
	void SetBaselineData(std::optional<float> newBaselineOffset, std::optional<float> newUnderlineHeight);
	void SetBoldFont(UObject* fn);
	void SetChildVisibility(bool bNewVisibility);
	void SetClientObject(UObject* newClientObject);
	void SetConfiguration(float newX, float newY, float newWidth, float NewHeight);
	void SetCursorPos(float newMouseX, float newMouseY);
	void SetDefaultCursor(UObject* tX, std::optional<UObject*> shadowTexture, std::optional<float> HotX, std::optional<float> HotY, std::optional<Color> cursorColor);
	void SetFocusSounds(std::optional<UObject*> focusSound, std::optional<UObject*> unfocusSound);
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
	void SetVisibilitySounds(std::optional<UObject*> visSound, std::optional<UObject*> invisSound);
	void SetWidth(float newWidth);
	void SetWindowAlignments(uint8_t HAlign, uint8_t VAlign, std::optional<float> hMargin0, std::optional<float> vMargin0, std::optional<float> hMargin1, std::optional<float> vMargin1);
	void Show(std::optional<bool> bShow);
	void UngrabMouse();

	void DetachFromParent();
	void UpdateLayout();

	void DrawDebugBox(UGC* gc);

	// Events (sent to unrealscript, can be overriden by native windows)
	virtual void InitWindow();
	virtual void DestroyWindow();
	virtual void WindowReady();
	virtual void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight);
	virtual void ParentRequestedGranularity(float& hGranularity, float& vGranularity);
	virtual void ChildRequestedVisibilityChange(UWindow* childWin, bool bNewVisibility);
	virtual bool ChildRequestedReconfiguration(UWindow* childWin);
	virtual void ChildRequestedShowArea(UWindow* child, float showX, float showY, float showWidth, float showHeight);
	virtual void ConfigurationChanged();
	virtual void VisibilityChanged(bool bNewVisibility);
	virtual void SensitivityChanged(bool bNewSensitivity);
	virtual void MouseMoved(float newX, float newY);
	virtual bool RawMouseButtonPressed(float pointX, float pointY, EInputKey button, EInputType iState);
	virtual bool RawKeyPressed(EInputKey key, EInputType iState, bool bRepeat);
	virtual bool MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks);
	virtual bool MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks);
	virtual bool KeyPressed(std::string key);
	virtual bool AcceleratorKeyPressed(std::string key);
	virtual bool VirtualKeyPressed(EInputKey key, bool bRepeat);
	virtual void MouseEnteredWindow();
	virtual void MouseLeftWindow();
	virtual void FocusEnteredWindow();
	virtual void FocusLeftWindow();
	virtual void FocusEnteredDescendant(UWindow* enterWindow);
	virtual void FocusLeftDescendant(UWindow* leaveWindow);
	virtual bool ButtonActivated(UWindow* button);
	virtual bool ToggleChanged(UWindow* button, bool bNewToggle);
	virtual bool BoxOptionSelected(UWindow* box, int buttonNumber);
	virtual bool ScalePositionChanged(UWindow* scale, int newTickPosition, float newValue, bool bFinal);
	virtual bool ScaleRangeChanged(UWindow* scale, int fromTick, int toTick, float fromValue, float toValue, bool bFinal);
	virtual bool ScaleAttributesChanged(UWindow* scale, int tickPosition, int tickSpan, int numTicks);
	virtual bool ClipAttributesChanged(UWindow* scale, int newClipWidth, int newClipHeight, int newChildWidth, int newChildHeight);
	virtual bool ListRowActivated(UWindow* list, int rowId);
	virtual bool ListSelectionChanged(UWindow* list, int numSelections, int focusRowId);
	virtual bool TextChanged(UWindow* edit, bool bModified);
	virtual bool EditActivated(UWindow* edit, bool bModified);
	virtual void DrawWindow(UGC* gc);
	virtual void PostDrawWindow(UGC* gc);
	virtual void ChildAdded(UWindow* child);
	virtual void ChildRemoved(UWindow* child);
	virtual void DescendantAdded(UWindow* descendant);
	virtual void DescendantRemoved(UWindow* descendant);

	virtual void Tick(float timeElapsed);

	static float GetVirtualWidth();
	static float GetExtendedVirtualWidth();
	static float GetVirtualHeight();
	static float GetVirtualScale();

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
	float BaselineOffset = 0.0f;
	float UnderlineHeight = 0.0f;
	float UsedX = 0.0f;
	float UsedY = 0.0f;

	class WTimer
	{
	public:
		NameString FunctionName;
		float Timeout = 0.0f;
		bool Loop = false;
		int ClientData = 0;
		float TimeLeft = 0.0f;
	};

	std::map<int, WTimer> ActiveTimers;
	int NextTimerId = 0;
};
