
#include "Precomp.h"
#include "UWindow.h"
#include "Engine.h"

void UWindow::AddActorRef(UObject* refActor)
{
	LogUnimplemented("Window.AddActorRef");
}

int UWindow::AddTimer(float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName)
{
	LogUnimplemented("Window.AddTimer");
	return 0;
}

void UWindow::AskParentForReconfigure()
{
	LogUnimplemented("Window.AskParentForReconfigure");
}

void UWindow::AskParentToShowArea(float* areaX, float* areaY, float* areaWidth, float* areaHeight)
{
	LogUnimplemented("Window.AskParentToShowArea");
}

std::string UWindow::CarriageReturn()
{
	LogUnimplemented("Window.CarriageReturn");
	return "";
}

void UWindow::ChangeStyle()
{
	LogUnimplemented("Window.ChangeStyle");
}

void UWindow::ConfigureChild(float newX, float newY, float newWidth, float NewHeight)
{
	LogUnimplemented("Window.ConfigureChild");
}

void UWindow::ConvertCoordinates(UObject* fromWin, float fromX, float fromY, UObject* toWin, float& toX, float& toY)
{
	LogUnimplemented("Window.ConvertCoordinates");
}

std::string UWindow::ConvertScriptString(const std::string& oldStr)
{
	LogUnimplemented("Window.ConvertScriptString");
	return "";
}

bool UWindow::ConvertVectorToCoordinates(const vec3& Location, float& relativeX, float& relativeY)
{
	LogUnimplemented("Window.ConvertVectorToCoordinates");
	return false;
}

void UWindow::Destroy()
{
	LogUnimplemented("Window.Destroy");
}

void UWindow::DestroyAllChildren()
{
	LogUnimplemented("Window.DestroyAllChildren");
}

void UWindow::DisableWindow()
{
	LogUnimplemented("Window.DisableWindow");
}

void UWindow::EnableSpecialText(BitfieldBool* bEnable)
{
	LogUnimplemented("Window.EnableSpecialText");
}

void UWindow::EnableTranslucentText(BitfieldBool* bEnable)
{
	LogUnimplemented("Window.EnableTranslucentText");
}

void UWindow::EnableWindow(BitfieldBool* bEnable)
{
	LogUnimplemented("Window.EnableWindow");
}

UObject* UWindow::FindWindow(float pointX, float pointY, float& relativeX, float& relativeY)
{
	LogUnimplemented("Window.FindWindow");
	return nullptr;
}

UObject* UWindow::GetBottomChild(BitfieldBool* bVisibleOnly)
{
	LogUnimplemented("Window.GetBottomChild");
	return nullptr;
}

UObject* UWindow::GetClientObject()
{
	LogUnimplemented("Window.GetClientObject");
	return nullptr;
}

void UWindow::GetCursorPos(float& MouseX, float& MouseY)
{
	LogUnimplemented("Window.GetCursorPos");
}

UObject* UWindow::GetFocusWindow()
{
	LogUnimplemented("Window.GetFocusWindow");
	return nullptr;
}

UObject* UWindow::GetGC()
{
	LogUnimplemented("Window.GetGC");
	return nullptr;
}

UObject* UWindow::GetHigherSibling(BitfieldBool* bVisibleOnly)
{
	LogUnimplemented("Window.GetHigherSibling");
	return nullptr;
}

UObject* UWindow::GetLowerSibling(BitfieldBool* bVisibleOnly)
{
	LogUnimplemented("Window.GetLowerSibling");
	return nullptr;
}

UObject* UWindow::GetModalWindow()
{
	LogUnimplemented("Window.GetModalWindow");
	return nullptr;
}

UObject* UWindow::GetParent()
{
	LogUnimplemented("Window.GetParent");
	return nullptr;
}

UObject* UWindow::GetPlayerPawn()
{
	LogUnimplemented("Window.GetPlayerPawn");
	return nullptr;
}

UObject* UWindow::GetRootWindow()
{
	LogUnimplemented("Window.GetRootWindow");
	return nullptr;
}

UObject* UWindow::GetTabGroupWindow()
{
	LogUnimplemented("Window.GetTabGroupWindow");
	return nullptr;
}

float UWindow::GetTickOffset()
{
	LogUnimplemented("Window.GetTickOffset");
	return 0.0f;
}

UObject* UWindow::GetTopChild(BitfieldBool* bVisibleOnly)
{
	LogUnimplemented("Window.GetTopChild");
	return nullptr;
}

void UWindow::GrabMouse()
{
	LogUnimplemented("Window.GrabMouse");
}

void UWindow::Hide()
{
	LogUnimplemented("Window.Hide");
}

bool UWindow::IsActorValid(UObject* refActor)
{
	LogUnimplemented("Window.IsActorValid");
	return false;
}

bool UWindow::IsFocusWindow()
{
	LogUnimplemented("Window.IsFocusWindow");
	return false;
}

bool UWindow::IsKeyDown(uint8_t Key)
{
	LogUnimplemented("Window.IsKeyDown");
	return false;
}

bool UWindow::IsPointInWindow(float pointX, float pointY)
{
	LogUnimplemented("Window.IsPointInWindow");
	return false;
}

bool UWindow::IsSensitive(BitfieldBool* bRecurse)
{
	LogUnimplemented("Window.IsSensitive");
	return false;
}

bool UWindow::IsVisible(BitfieldBool* bRecurse)
{
	LogUnimplemented("Window.IsVisible");
	return false;
}

void UWindow::Lower()
{
	LogUnimplemented("Window.Lower");
}

UObject* UWindow::MoveFocusDown()
{
	LogUnimplemented("Window.MoveFocusDown");
	return nullptr;
}

UObject* UWindow::MoveFocusLeft()
{
	LogUnimplemented("Window.MoveFocusLeft");
	return nullptr;
}

UObject* UWindow::MoveFocusRight()
{
	LogUnimplemented("Window.MoveFocusRight");
	return nullptr;
}

UObject* UWindow::MoveFocusUp()
{
	LogUnimplemented("Window.MoveFocusUp");
	return nullptr;
}

UObject* UWindow::MoveTabGroupNext()
{
	LogUnimplemented("Window.MoveTabGroupNext");
	return nullptr;
}

UObject* UWindow::MoveTabGroupPrev()
{
	LogUnimplemented("Window.MoveTabGroupPrev");
	return nullptr;
}

UObject* UWindow::NewChild(UObject* NewClass, BitfieldBool* bShow)
{
	LogUnimplemented("Window.NewChild");
	return nullptr;
}

void UWindow::PlaySound(UObject* newsound, float* Volume, float* Pitch, float* posX, float* posY)
{
	LogUnimplemented("Window.PlaySound");
}

void UWindow::QueryGranularity(float& hGranularity, float& vGranularity)
{
	LogUnimplemented("Window.QueryGranularity");
}

float UWindow::QueryPreferredHeight(float queryWidth)
{
	LogUnimplemented("Window.QueryPreferredHeight");
	return 0.0f;
}

void UWindow::QueryPreferredSize(float& preferredWidth, float& preferredHeight)
{
	LogUnimplemented("Window.QueryPreferredSize");
}

float UWindow::QueryPreferredWidth(float queryHeight)
{
	LogUnimplemented("Window.QueryPreferredWidth");
	return 0.0f;
}

void UWindow::Raise()
{
	LogUnimplemented("Window.Raise");
}

void UWindow::ReleaseGC(UObject* GC)
{
	LogUnimplemented("Window.ReleaseGC");
}

void UWindow::RemoveActorRef(UObject* refActor)
{
	LogUnimplemented("Window.RemoveActorRef");
}

void UWindow::RemoveTimer(int timerId)
{
	LogUnimplemented("Window.RemoveTimer");
}

void UWindow::ResetHeight()
{
	LogUnimplemented("Window.ResetHeight");
}

void UWindow::ResetSize()
{
	LogUnimplemented("Window.ResetSize");
}

void UWindow::ResetWidth()
{
	LogUnimplemented("Window.ResetWidth");
}

void UWindow::ResizeChild()
{
	LogUnimplemented("Window.ResizeChild");
}

void UWindow::SetAcceleratorText(const std::string& newStr)
{
	LogUnimplemented("Window.SetAcceleratorText");
}

void UWindow::SetBackground(UObject* newBackground)
{
	LogUnimplemented("Window.SetBackground");
}

void UWindow::SetBackgroundSmoothing(bool newSmoothing)
{
	LogUnimplemented("Window.SetBackgroundSmoothing");
}

void UWindow::SetBackgroundStretching(bool newStretching)
{
	LogUnimplemented("Window.SetBackgroundStretching");
}

void UWindow::SetBackgroundStyle(uint8_t NewStyle)
{
	LogUnimplemented("Window.SetBackgroundStyle");
}

void UWindow::SetBaselineData(float* newBaselineOffset, float* newUnderlineHeight)
{
	LogUnimplemented("Window.SetBaselineData");
}

void UWindow::SetBoldFont(UObject* fn)
{
	LogUnimplemented("Window.SetBoldFont");
}

void UWindow::SetChildVisibility(bool bNewVisibility)
{
	LogUnimplemented("Window.SetChildVisibility");
}

void UWindow::SetClientObject(UObject* newClientObject)
{
	LogUnimplemented("Window.SetClientObject");
}

void UWindow::SetConfiguration(float newX, float newY, float newWidth, float NewHeight)
{
	LogUnimplemented("Window.SetConfiguration");
}

void UWindow::SetCursorPos(float newMouseX, float newMouseY)
{
	LogUnimplemented("Window.SetCursorPos");
}

void UWindow::SetDefaultCursor(UObject* tX, UObject** shadowTexture, float* HotX, float* HotY, Color* cursorColor)
{
	LogUnimplemented("Window.SetDefaultCursor");
}

void UWindow::SetFocusSounds(UObject** focusSound, UObject** unfocusSound)
{
	LogUnimplemented("Window.SetFocusSounds");
}

bool UWindow::SetFocusWindow(UObject* NewFocusWindow)
{
	LogUnimplemented("Window.SetFocusWindow");
	return false;
}

void UWindow::SetFont(UObject* fn)
{
	LogUnimplemented("Window.SetFont");
}

void UWindow::SetFonts(UObject* nFont, UObject* bFont)
{
	LogUnimplemented("Window.SetFonts");
}

void UWindow::SetHeight(float NewHeight)
{
	LogUnimplemented("Window.SetHeight");
}

void UWindow::SetNormalFont(UObject* fn)
{
	LogUnimplemented("Window.SetNormalFont");
}

void UWindow::SetPos(float newX, float newY)
{
	LogUnimplemented("Window.SetPos");
}

void UWindow::SetSelectability(bool newSelectability)
{
	LogUnimplemented("Window.SetSelectability");
}

void UWindow::SetSensitivity(bool newSensitivity)
{
	LogUnimplemented("Window.SetSensitivity");
}

void UWindow::SetSize(float newWidth, float NewHeight)
{
	LogUnimplemented("Window.SetSize");
}

void UWindow::SetSoundVolume(float newVolume)
{
	LogUnimplemented("Window.SetSoundVolume");
}

void UWindow::SetTextColor(const Color& NewColor)
{
	LogUnimplemented("Window.SetTextColor");
}

void UWindow::SetTileColor(const Color& NewColor)
{
	LogUnimplemented("Window.SetTileColor");
}

void UWindow::SetVisibilitySounds(UObject** visSound, UObject** invisSound)
{
	LogUnimplemented("Window.SetVisibilitySounds");
}

void UWindow::SetWidth(float newWidth)
{
	LogUnimplemented("Window.SetWidth");
}

void UWindow::SetWindowAlignments(uint8_t HAlign, uint8_t VAlign, float* hMargin0, float* vMargin0, float* hMargin1, float* vMargin1)
{
	LogUnimplemented("Window.SetWindowAlignments");
}

void UWindow::Show(BitfieldBool* bShow)
{
	LogUnimplemented("Window.Show");
}

void UWindow::UngrabMouse()
{
	LogUnimplemented("Window.UngrabMouse");
}

/////////////////////////////////////////////////////////////////////////////

