
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

void UViewportWindow::ClearZBuffer(BitfieldBool* bClear)
{
	LogUnimplemented("ViewportWindow.ClearZBuffer");
}

void UViewportWindow::EnableViewport(BitfieldBool* bEnable)
{
	LogUnimplemented("ViewportWindow.EnableViewport");
}

void UViewportWindow::SetDefaultTexture(UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("ViewportWindow.SetDefaultTexture");
}

void UViewportWindow::SetFOVAngle(float* newAngle)
{
	LogUnimplemented("ViewportWindow.SetFOVAngle");
}

void UViewportWindow::SetRelativeLocation(vec3* relLoc)
{
	LogUnimplemented("ViewportWindow.SetRelativeLocation");
}

void UViewportWindow::SetRelativeRotation(Rotator* relRot)
{
	LogUnimplemented("ViewportWindow.SetRelativeRotation");
}

void UViewportWindow::SetRotation(Rotator* NewRotation)
{
	LogUnimplemented("ViewportWindow.SetRotation");
}

void UViewportWindow::SetViewportActor(UObject** newOriginActor, BitfieldBool* bEyeLevel, BitfieldBool* bEnable)
{
	LogUnimplemented("ViewportWindow.SetViewportActor");
}

void UViewportWindow::SetViewportLocation(const vec3& NewLocation, BitfieldBool* bEnable)
{
	LogUnimplemented("ViewportWindow.SetViewportLocation");
}

void UViewportWindow::SetWatchActor(UObject** newWatchActor, BitfieldBool* bEyeLevel)
{
	LogUnimplemented("ViewportWindow.SetWatchActor");
}

void UViewportWindow::ShowViewportActor(BitfieldBool* bShow)
{
	LogUnimplemented("ViewportWindow.ShowViewportActor");
}

void UViewportWindow::ShowWeapons(BitfieldBool* bShow)
{
	LogUnimplemented("ViewportWindow.ShowWeapons");
}

/////////////////////////////////////////////////////////////////////////////

void UTileWindow::EnableWrapping(bool bWrapOn)
{
	LogUnimplemented("TileWindow.EnableWrapping");
}

void UTileWindow::FillParent(bool FillParent)
{
	LogUnimplemented("TileWindow.FillParent");
}

void UTileWindow::MakeHeightsEqual(bool bEqual)
{
	LogUnimplemented("TileWindow.MakeHeightsEqual");
}

void UTileWindow::MakeWidthsEqual(bool bEqual)
{
	LogUnimplemented("TileWindow.MakeWidthsEqual");
}

void UTileWindow::SetChildAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	LogUnimplemented("TileWindow.SetChildAlignments");
}

void UTileWindow::SetDirections(uint8_t newHDir, uint8_t newVDir)
{
	LogUnimplemented("TileWindow.SetDirections");
}

void UTileWindow::SetMajorSpacing(float newSpacing)
{
	LogUnimplemented("TileWindow.SetMajorSpacing");
}

void UTileWindow::SetMargins(float newHMargin, float newVMargin)
{
	LogUnimplemented("TileWindow.SetMargins");
}

void UTileWindow::SetMinorSpacing(float newSpacing)
{
	LogUnimplemented("TileWindow.SetMinorSpacing");
}

void UTileWindow::SetOrder(uint8_t newOrder)
{
	LogUnimplemented("TileWindow.SetOrder");
}

void UTileWindow::SetOrientation(uint8_t newOrientation)
{
	LogUnimplemented("TileWindow.SetOrientation");
}

/////////////////////////////////////////////////////////////////////////////

void UTextWindow::AppendText(const std::string& NewText)
{
	LogUnimplemented("TextWindow.AppendText");
}

void UTextWindow::EnableTextAsAccelerator(BitfieldBool* bEnable)
{
	LogUnimplemented("TextWindow.EnableTextAsAccelerator");
}

std::string UTextWindow::GetText()
{
	LogUnimplemented("TextWindow.GetText");
	return "";
}

int UTextWindow::GetTextLength()
{
	LogUnimplemented("TextWindow.GetTextLength");
	return 0;
}

int UTextWindow::GetTextPart(int startPos, int Count, std::string& OutText)
{
	LogUnimplemented("TextWindow.GetTextPart");
	return 0;
}

void UTextWindow::ResetLines()
{
	LogUnimplemented("TextWindow.ResetLines");
}

void UTextWindow::ResetMinWidth()
{
	LogUnimplemented("TextWindow.ResetMinWidth");
}

void UTextWindow::SetLines(int newMinLines, int newMaxLines)
{
	LogUnimplemented("TextWindow.SetLines");
}

void UTextWindow::SetMaxLines(int newMaxLines)
{
	LogUnimplemented("TextWindow.SetMaxLines");
}

void UTextWindow::SetMinLines(int newMinLines)
{
	LogUnimplemented("TextWindow.SetMinLines");
}

void UTextWindow::SetMinWidth(float newMinWidth)
{
	LogUnimplemented("TextWindow.SetMinWidth");
}

void UTextWindow::SetText(const std::string& NewText)
{
	LogUnimplemented("TextWindow.SetText");
}

void UTextWindow::SetTextAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	LogUnimplemented("TextWindow.SetTextAlignments");
}

void UTextWindow::SetTextMargins(float newHMargin, float newVMargin)
{
	LogUnimplemented("TextWindow.SetTextMargins");
}

void UTextWindow::SetWordWrap(bool bNewWordWrap)
{
	LogUnimplemented("TextWindow.SetWordWrap");
}

/////////////////////////////////////////////////////////////////////////////

void UButtonWindow::ActivateButton(uint8_t Key)
{
	LogUnimplemented("ButtonWindow.ActivateButton");
}

void UButtonWindow::EnableAutoRepeat(BitfieldBool* bEnable, float* initialDelay, float* repeatRate)
{
	LogUnimplemented("ButtonWindow.EnableAutoRepeat");
}

void UButtonWindow::EnableRightMouseClick(BitfieldBool* bEnable)
{
	LogUnimplemented("ButtonWindow.EnableRightMouseClick");
}

void UButtonWindow::PressButton(uint8_t* Key)
{
	LogUnimplemented("ButtonWindow.PressButton");
}

void UButtonWindow::SetActivateDelay(float* newDelay)
{
	LogUnimplemented("ButtonWindow.SetActivateDelay");
}

void UButtonWindow::SetButtonColors(Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	LogUnimplemented("ButtonWindow.SetButtonColors");
}

void UButtonWindow::SetButtonSounds(UObject** pressSound, UObject** clickSound)
{
	LogUnimplemented("ButtonWindow.SetButtonSounds");
}

void UButtonWindow::SetButtonTextures(UObject** Normal, UObject** pressed, UObject** normalFocus, UObject** pressedFocus, UObject** normalInsensitive, UObject** pressedInsensitive)
{
	LogUnimplemented("ButtonWindow.SetButtonTextures");
}

void UButtonWindow::SetTextColors(Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	LogUnimplemented("ButtonWindow.SetTextColors");
}

/////////////////////////////////////////////////////////////////////////////

void UToggleWindow::ChangeToggle()
{
	LogUnimplemented("ToggleWindow.ChangeToggle");
}

bool UToggleWindow::GetToggle()
{
	LogUnimplemented("ToggleWindow.GetToggle");
	return false;
}

void UToggleWindow::SetToggle(bool bNewToggle)
{
	LogUnimplemented("ToggleWindow.SetToggle");
}

void UToggleWindow::SetToggleSounds(UObject** enableSound, UObject** disableSound)
{
	LogUnimplemented("ToggleWindow.SetToggleSounds");
}

/////////////////////////////////////////////////////////////////////////////

void UCheckboxWindow::SetCheckboxColor(const Color& NewColor)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxColor");
}

void UCheckboxWindow::SetCheckboxSpacing(float newSpacing)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxSpacing");
}

void UCheckboxWindow::SetCheckboxStyle(uint8_t NewStyle)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxStyle");
}

void UCheckboxWindow::SetCheckboxTextures(UObject** toggleOff, UObject** toggleOn, float* textureWidth, float* textureHeight)
{
	LogUnimplemented("CheckboxWindow.SetCheckboxTextures");
}

void UCheckboxWindow::ShowCheckboxOnRightSide(BitfieldBool* bRight)
{
	LogUnimplemented("CheckboxWindow.ShowCheckboxOnRightSide");
}

/////////////////////////////////////////////////////////////////////////////

void UTextLogWindow::AddLog(const std::string& NewText, const Color& linecol)
{
	LogUnimplemented("TextLogWindow.AddLog");
}

void UTextLogWindow::ClearLog()
{
	LogUnimplemented("TextLogWindow.ClearLog");
}

void UTextLogWindow::PauseLog(bool bNewPauseState)
{
	LogUnimplemented("TextLogWindow.PauseLog");
}

void UTextLogWindow::SetTextTimeout(float newTimeout)
{
	LogUnimplemented("TextLogWindow.SetTextTimeout");
}

/////////////////////////////////////////////////////////////////////////////

void ULargeTextWindow::SetVerticalSpacing(float* newVSpace)
{
	LogUnimplemented("LargeTextWindow.SetVerticalSpacing");
}

/////////////////////////////////////////////////////////////////////////////

void UEditWindow::ClearTextChangedFlag()
{
	LogUnimplemented("EditWindow.ClearTextChangedFlag");
}

void UEditWindow::ClearUndo()
{
	LogUnimplemented("EditWindow.ClearUndo");
}

void UEditWindow::Copy()
{
	LogUnimplemented("EditWindow.Copy");
}

void UEditWindow::Cut()
{
	LogUnimplemented("EditWindow.Cut");
}

void UEditWindow::DeleteChar(BitfieldBool* bBefore, BitfieldBool* bUndo)
{
	LogUnimplemented("EditWindow.DeleteChar");
}

void UEditWindow::EnableEditing(BitfieldBool* bEdit)
{
	LogUnimplemented("EditWindow.EnableEditing");
}

void UEditWindow::EnableSingleLineEditing(BitfieldBool* bSingle)
{
	LogUnimplemented("EditWindow.EnableSingleLineEditing");
}

void UEditWindow::EnableUppercaseOnly(BitfieldBool* bUppercase)
{
	LogUnimplemented("EditWindow.EnableUppercaseOnly");
}

int UEditWindow::GetInsertionPoint()
{
	LogUnimplemented("EditWindow.GetInsertionPoint");
	return 0;
}

void UEditWindow::GetSelectedArea(int& startPos, int& Count)
{
	LogUnimplemented("EditWindow.GetSelectedArea");
}

bool UEditWindow::HasTextChanged()
{
	LogUnimplemented("EditWindow.HasTextChanged");
	return false;
}

bool UEditWindow::InsertText(std::string* InsertText, BitfieldBool* bUndo, BitfieldBool* bSelect)
{
	LogUnimplemented("EditWindow.InsertText");
	return false;
}

bool UEditWindow::IsEditingEnabled()
{
	LogUnimplemented("EditWindow.IsEditingEnabled");
	return false;
}

bool UEditWindow::IsSingleLineEditingEnabled()
{
	LogUnimplemented("EditWindow.IsSingleLineEditingEnabled");
	return false;
}

void UEditWindow::MoveInsertionPoint(uint8_t moveInsert, BitfieldBool* bDrag)
{
	LogUnimplemented("EditWindow.MoveInsertionPoint");
}

void UEditWindow::Paste()
{
	LogUnimplemented("EditWindow.Paste");
}

void UEditWindow::PlayEditSound(UObject* PlaySound, float* Volume, float* Pitch)
{
	LogUnimplemented("EditWindow.PlayEditSound");
}

void UEditWindow::Redo()
{
	LogUnimplemented("EditWindow.Redo");
}

void UEditWindow::SetEditCursor(UObject** newCursor, UObject** newCursorShadow, Color* NewColor)
{
	LogUnimplemented("EditWindow.SetEditCursor");
}

void UEditWindow::SetEditSounds(UObject** typeSound, UObject** deleteSound, UObject** enterSound, UObject** moveSound)
{
	LogUnimplemented("EditWindow.SetEditSounds");
}

void UEditWindow::SetInsertionPoint(int NewPos, BitfieldBool* bDrag)
{
	LogUnimplemented("EditWindow.SetInsertionPoint");
}

void UEditWindow::SetInsertionPointBlinkRate(float* blinkStart, float* blinkPeriod)
{
	LogUnimplemented("EditWindow.SetInsertionPointBlinkRate");
}

void UEditWindow::SetInsertionPointTexture(UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("EditWindow.SetInsertionPointTexture");
}

void UEditWindow::SetInsertionPointType(uint8_t newType, float* prefWidth, float* prefHeight)
{
	LogUnimplemented("EditWindow.SetInsertionPointType");
}

void UEditWindow::SetMaxSize(int newMaxSize)
{
	LogUnimplemented("EditWindow.SetMaxSize");
}

void UEditWindow::SetMaxUndos(int newMaxUndos)
{
	LogUnimplemented("EditWindow.SetMaxUndos");
}

void UEditWindow::SetSelectedArea(int startPos, int Count)
{
	LogUnimplemented("EditWindow.SetSelectedArea");
}

void UEditWindow::SetSelectedAreaTextColor(Color* NewColor)
{
	LogUnimplemented("EditWindow.SetSelectedAreaTextColor");
}

void UEditWindow::SetSelectedAreaTexture(UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("EditWindow.SetSelectedAreaTexture");
}

void UEditWindow::SetTextChangedFlag(BitfieldBool* bSet)
{
	LogUnimplemented("EditWindow.SetTextChangedFlag");
}

void UEditWindow::Undo()
{
	LogUnimplemented("EditWindow.Undo");
}

/////////////////////////////////////////////////////////////////////////////

UObject* URadioBoxWindow::GetEnabledToggle()
{
	LogUnimplemented("RadioBoxWindow.GetEnabledToggle");
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////

void UClipWindow::EnableSnapToUnits(BitfieldBool* bNewSnapToUnits)
{
	LogUnimplemented("ClipWindow.EnableSnapToUnits");
}

void UClipWindow::ForceChildSize(BitfieldBool* bNewForceChildWidth, BitfieldBool* bNewForceChildHeight)
{
	LogUnimplemented("ClipWindow.ForceChildSize");
}

UObject* UClipWindow::GetChild()
{
	LogUnimplemented("ClipWindow.GetChild");
	return nullptr;
}

void UClipWindow::GetChildPosition(int& pNewX, int& pNewY)
{
	LogUnimplemented("ClipWindow.GetChildPosition");
}

void UClipWindow::GetUnitSize(int& pAreaHSize, int& pAreaVSize, int& pChildHSize, int& childVSize)
{
	LogUnimplemented("ClipWindow.GetUnitSize");
}

void UClipWindow::ResetUnitHeight()
{
	LogUnimplemented("ClipWindow.ResetUnitHeight");
}

void UClipWindow::ResetUnitSize()
{
	LogUnimplemented("ClipWindow.ResetUnitSize");
}

void UClipWindow::ResetUnitWidth()
{
	LogUnimplemented("ClipWindow.ResetUnitWidth");
}

void UClipWindow::SetChildPosition(int newX, int newY)
{
	LogUnimplemented("ClipWindow.SetChildPosition");
}

void UClipWindow::SetUnitHeight(int vUnits)
{
	LogUnimplemented("ClipWindow.SetUnitHeight");
}

void UClipWindow::SetUnitSize(int hUnits, int vUnits)
{
	LogUnimplemented("ClipWindow.SetUnitSize");
}

void UClipWindow::SetUnitWidth(int hUnits)
{
	LogUnimplemented("ClipWindow.SetUnitWidth");
}

/////////////////////////////////////////////////////////////////////////////

bool UModalWindow::IsCurrentModal()
{
	LogUnimplemented("ModalWindow.IsCurrentModal");
	return false;
}

void UModalWindow::SetMouseFocusMode(uint8_t newFocusMode)
{
	LogUnimplemented("ModalWindow.SetMouseFocusMode");
}

/////////////////////////////////////////////////////////////////////////////

void URootWindow::EnablePositionalSound(BitfieldBool* bEnable)
{
	LogUnimplemented("RootWindow.EnablePositionalSound");
}

void URootWindow::EnableRendering(BitfieldBool* bRender)
{
	LogUnimplemented("RootWindow.EnableRendering");
}

UObject* URootWindow::GenerateSnapshot(BitfieldBool* bFilter)
{
	LogUnimplemented("RootWindow.GenerateSnapshot");
	return nullptr;
}

bool URootWindow::IsPositionalSoundEnabled()
{
	LogUnimplemented("RootWindow.IsPositionalSoundEnabled");
	return false;
}

bool URootWindow::IsRenderingEnabled()
{
	LogUnimplemented("RootWindow.IsRenderingEnabled");
	return false;
}

void URootWindow::LockMouse(BitfieldBool* bLockMove, BitfieldBool* bLockButton)
{
	LogUnimplemented("RootWindow.LockMouse");
}

void URootWindow::ResetRenderViewport()
{
	LogUnimplemented("RootWindow.ResetRenderViewport");
}

void URootWindow::SetDefaultEditCursor(UObject** newEditCursor)
{
	LogUnimplemented("RootWindow.SetDefaultEditCursor");
}

void URootWindow::SetDefaultMovementCursors(UObject** newMovementCursor, UObject** newHorizontalMovementCursor, UObject** newVerticalMovementCursor, UObject** newTopLeftMovementCursor, UObject** newTopRightMovementCursor)
{
	LogUnimplemented("RootWindow.SetDefaultMovementCursors");
}

void URootWindow::SetRawBackground(UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("RootWindow.SetRawBackground");
}

void URootWindow::SetRawBackgroundSize(float newWidth, float NewHeight)
{
	LogUnimplemented("RootWindow.SetRawBackgroundSize");
}

void URootWindow::SetRenderViewport(float newX, float newY, float newWidth, float NewHeight)
{
	LogUnimplemented("RootWindow.SetRenderViewport");
}

void URootWindow::SetSnapshotSize(float newWidth, float NewHeight)
{
	LogUnimplemented("RootWindow.SetSnapshotSize");
}

void URootWindow::ShowCursor(BitfieldBool* bShow)
{
	LogUnimplemented("RootWindow.ShowCursor");
}

void URootWindow::StretchRawBackground(BitfieldBool* bStretch)
{
	LogUnimplemented("RootWindow.StretchRawBackground");
}

/////////////////////////////////////////////////////////////////////////////

void UScrollAreaWindow::AutoHideScrollbars(BitfieldBool* bHide)
{
	LogUnimplemented("ScrollAreaWindow.AutoHideScrollbars");
}

void UScrollAreaWindow::EnableScrolling(BitfieldBool* bHScrolling, BitfieldBool* bVScrolling)
{
	LogUnimplemented("ScrollAreaWindow.EnableScrolling");
}

void UScrollAreaWindow::SetAreaMargins(float newMarginWidth, float newMarginHeight)
{
	LogUnimplemented("ScrollAreaWindow.SetAreaMargins");
}

void UScrollAreaWindow::SetScrollbarDistance(float newDistance)
{
	LogUnimplemented("ScrollAreaWindow.SetScrollbarDistance");
}

/////////////////////////////////////////////////////////////////////////////

void UBorderWindow::BaseMarginsFromBorder(BitfieldBool* bBorder)
{
	LogUnimplemented("BorderWindow.BaseMarginsFromBorder");
}

void UBorderWindow::EnableResizing(BitfieldBool* bResize)
{
	LogUnimplemented("BorderWindow.EnableResizing");
}

void UBorderWindow::SetBorderMargins(float* NewLeft, float* NewRight, float* newTop, float* newBottom)
{
	LogUnimplemented("BorderWindow.SetBorderMargins");
}

void UBorderWindow::SetBorders(UObject** bordTL, UObject** bordTR, UObject** bordBL, UObject** bordBR, UObject** bordL, UObject** bordR, UObject** bordT, UObject** bordB, UObject** center)
{
	LogUnimplemented("BorderWindow.SetBorders");
}

void UBorderWindow::SetMoveCursors(UObject** Move, UObject** hMove, UObject** vMove, UObject** tlMove, UObject** trMove)
{
	LogUnimplemented("BorderWindow.SetMoveCursors");
}
