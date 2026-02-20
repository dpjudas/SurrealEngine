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
};

class UTileWindow : public UWindow
{
public:
    using UWindow::UWindow;
};

class UTextWindow : public UWindow
{
public:
    using UWindow::UWindow;
};

class UButtonWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;
};

class UToggleWindow : public UButtonWindow
{
public:
    using UButtonWindow::UButtonWindow;
};

class UCheckboxWindow : public UToggleWindow
{
public:
    using UToggleWindow::UToggleWindow;
};

class UTextLogWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;
};

class ULargeTextWindow : public UTextWindow
{
public:
    using UTextWindow::UTextWindow;
};

class UEditWindow : public ULargeTextWindow
{
public:
    using ULargeTextWindow::ULargeTextWindow;
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
};

class UClipWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;
};

class UModalWindow : public UTabGroupWindow
{
public:
    using UTabGroupWindow::UTabGroupWindow;
};

class URootWindow : public UModalWindow
{
public:
    using UModalWindow::UModalWindow;
};

class UScrollAreaWindow : public UWindow
{
public:
    using UWindow::UWindow;
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
};

class UGC : public UObject
{
public:
	using UObject::UObject;
};
