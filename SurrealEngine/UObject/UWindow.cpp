
#include "Precomp.h"
#include "UWindow.h"
#include "Audio/AudioDevice.h"
#include "UObject/UClass.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/UFont.h"
#include "UObject/USound.h"
#include "UObject/UClient.h"
#include "UObject/USubsystem.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "USubsystem.h"
#include "Render/RenderSubsystem.h"
#include "Package/PackageManager.h"

void UWindow::AddActorRef(UObject* refActor)
{
	UObject* playerPawn = GetPlayerPawn();
	if (!playerPawn) return;

	// TODO: actual meat of the code. AddActorRef seems to be an unexposed NPlayerPawnExt function.
	//RingQueue<ActorRef> ActorRefs{32};  
	
	LogUnimplemented("Window.AddActorRef");
}

int UWindow::AddTimer(float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName)
{
	LogUnimplemented("Window.AddTimer");
	return 0;
}

void UWindow::AskParentForReconfigure()
{
	UWindow* parent = parentOwner();
	if (parent)
	{
		CallEvent(parent, "ChildRequestedReconfiguration", { ExpressionValue::ObjectValue(this) });
	}
}

void UWindow::AskParentToShowArea(float* areaX, float* areaY, float* areaWidth, float* areaHeight)
{
	UWindow* parent = parentOwner();
	if (parent)
	{
		float showX = areaX ? *areaX : X();
		float showY = areaY ? *areaY : Y();
		float showWidth = areaWidth ? *areaWidth : Width();
		float showHeight = areaHeight ? *areaHeight : Height();
		CallEvent(parent, "ChildRequestedShowArea", {
			ExpressionValue::ObjectValue(this),
			ExpressionValue::FloatValue(showX),
			ExpressionValue::FloatValue(showY),
			ExpressionValue::FloatValue(showWidth),
			ExpressionValue::FloatValue(showHeight)
			});
	}
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
	X() = newX;
	Y() = newY;
	Width() = newWidth;
	Height() = NewHeight;

	CallEvent(this, "ConfigurationChanged");
}

void UWindow::ConvertCoordinates(UObject* fromWin, float fromX, float fromY, UObject* toWin, float& toX, float& toY)
{
	LogUnimplemented("Window.ConvertCoordinates");
}

std::string UWindow::ConvertScriptString(const std::string& oldStr)
{
	LogUnimplemented("Window.ConvertScriptString");
	return oldStr;
}

bool UWindow::ConvertVectorToCoordinates(const vec3& Location, float& relativeX, float& relativeY)
{
	LogUnimplemented("Window.ConvertVectorToCoordinates");
	return false;
}

void UWindow::Destroy()
{
	UWindow* parent = parentOwner();
	DetachFromParent();

	if (parent)
	{
		CallEvent(parent, "ChildRemoved", { ExpressionValue::ObjectValue(this) });
		for (UWindow* ancestor = parent->parentOwner(); ancestor; ancestor = ancestor->parentOwner())
			CallEvent(ancestor, "DescendantRemoved", { ExpressionValue::ObjectValue(this) });
	}
}

void UWindow::DetachFromParent()
{
	if (prevSibling())
		prevSibling()->nextSibling() = nextSibling();
	if (nextSibling())
		nextSibling()->prevSibling() = prevSibling();
	if (parentOwner())
	{
		if (parentOwner()->firstChild() == this)
			parentOwner()->firstChild() = nextSibling();
		if (parentOwner()->lastChild() == this)
			parentOwner()->lastChild() = prevSibling();
	}
	prevSibling() = nullptr;
	nextSibling() = nullptr;
	parentOwner() = nullptr;
}

void UWindow::DestroyAllChildren()
{
	while (firstChild())
		firstChild()->Destroy();
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
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* child = firstChild(); child; child = child->nextSibling())
	{
		if (!visibleOnly || bIsVisible())
			return child;
	}
	return nullptr;
}

UObject* UWindow::GetClientObject()
{
	return clientObject();
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
	return engine->dxgc;
}

UObject* UWindow::GetHigherSibling(BitfieldBool* bVisibleOnly)
{
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* cur = nextSibling(); cur; cur = cur->nextSibling())
	{
		if (!visibleOnly || bIsVisible())
			return cur;
	}
	return nullptr;
}

UObject* UWindow::GetLowerSibling(BitfieldBool* bVisibleOnly)
{
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* cur = prevSibling(); cur; cur = cur->prevSibling())
	{
		if (!visibleOnly || bIsVisible())
			return cur;
	}
	return nullptr;
}

UObject* UWindow::GetModalWindow()
{
	LogUnimplemented("Window.GetModalWindow");
	return nullptr;
}

UObject* UWindow::GetParent()
{
	return parentOwner();
}

UObject* UWindow::GetPlayerPawn()
{
	UWindow* cur = this;
	while (cur->parentOwner())
		cur = cur->parentOwner();
	URootWindow* root = UObject::TryCast<URootWindow>(cur);
	return root ? root->parentPawn() : nullptr;
}

UObject* UWindow::GetRootWindow()
{
	UWindow* cur = this;
	while (cur->parentOwner())
		cur = cur->parentOwner();
	return UObject::TryCast<URootWindow>(cur);
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
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* child = lastChild(); child; child = child->prevSibling())
	{
		if (!visibleOnly || bIsVisible())
			return child;
	}
	return nullptr;
}

void UWindow::GrabMouse()
{
	LogUnimplemented("Window.GrabMouse");
}

void UWindow::Hide()
{
	bIsVisible() = false;
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
	if (engine && engine->window)
		return engine->window->GetKeyState(static_cast<EInputKey>(Key));
	else
	 	return false;
}

bool UWindow::IsPointInWindow(float pointX, float pointY)
{
	LogUnimplemented("Window.IsPointInWindow");
	return false;
}

bool UWindow::IsSensitive(BitfieldBool* bRecurse)
{
	if (bIsSensitive())
		return true;
	if (bRecurse && *bRecurse)
	{
		for (UWindow* cur = firstChild(); cur; cur = cur->nextSibling())
		{
			if (cur->IsSensitive(bRecurse))
				return true;
		}
	}
	return false;
}

bool UWindow::IsVisible(BitfieldBool* bRecurse)
{
	if (bIsVisible())
		return true;
	if (bRecurse && *bRecurse)
	{
		for (UWindow* cur = firstChild(); cur; cur = cur->nextSibling())
		{
			if (cur->IsVisible(bRecurse))
				return true;
		}
	}
	return false;
}

void UWindow::Lower()
{
	if (!parentOwner())
		return;
	UWindow* owner = parentOwner();
	DetachFromParent();
	parentOwner() = owner;
	if (owner->firstChild())
	{
		nextSibling() = owner->firstChild();
		owner->firstChild()->prevSibling() = this;
		owner->firstChild() = this;
	}
	else
	{
		owner->firstChild() = this;
		owner->lastChild() = this;
	}
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
	bool show = !bShow || *bShow;
	//LogMessage("Window.NewChild(" + NewClass->Name.ToString() + ", " + (show ? "true" : "false") + ")");
	auto child = UObject::Cast<UWindow>(engine->packages->GetTransientPackage()->NewObject("dxRootWindow", UObject::Cast<UClass>(NewClass), ObjectFlags::Transient));
	child->parentOwner() = this;
	child->prevSibling() = lastChild();
	child->nextSibling() = nullptr;
	child->firstChild() = nullptr;
	child->lastChild() = nullptr;
	child->bIsVisible() = show;
	if (lastChild())
		lastChild()->nextSibling() = child;
	lastChild() = child;
	if (!firstChild())
		firstChild() = child;
	CallEvent(child, "InitWindow");
	CallEvent(this, "ChildAdded", { ExpressionValue::ObjectValue(child) });
	for (UWindow* ancestor = parentOwner(); ancestor; ancestor = ancestor->parentOwner())
		CallEvent(ancestor, "DescendantAdded", { ExpressionValue::ObjectValue(child) });
	return child;
}

void UWindow::PlaySound(UObject* newsound, float* Volume, float* Pitch, float* posX, float* posY)
{
	USound* s = UObject::Cast<USound>(newsound);
	UPlayerPawnExt* player = UObject::Cast<UPlayerPawnExt>(GetPlayerPawn());
	if (s && player)
	{
		int slot = SLOT_Misc;
		int id = ((((int)(ptrdiff_t)this) & 0xffffff) << 4) + (slot << 1);
		vec3 location = player->Location();
		if (posX)
			location.x = *posX;
		if (posY)
			location.y = *posY;
		engine->audiodev->PlaySound(player, id, s, location, Volume ? *Volume : 1.0f, player->WorldSoundRadius(), Pitch ? *Pitch : 1.0f);
	}
}

void UWindow::QueryGranularity(float& hGranularity, float& vGranularity)
{
	LogUnimplemented("Window.QueryGranularity");
}

float UWindow::QueryPreferredHeight(float queryWidth)
{
	float height = 0.0f;
	CallEvent(this, "ParentRequestedPreferredSize", {
		ExpressionValue::BoolValue(true),
		ExpressionValue::Variable(&queryWidth, engine->floatprop),
		ExpressionValue::BoolValue(false),
		ExpressionValue::Variable(&height, engine->floatprop)
		});
	return height;
}

void UWindow::QueryPreferredSize(float& preferredWidth, float& preferredHeight)
{
	CallEvent(this, "ParentRequestedPreferredSize", {
		ExpressionValue::BoolValue(true),
		ExpressionValue::Variable(&preferredWidth, engine->floatprop),
		ExpressionValue::BoolValue(true),
		ExpressionValue::Variable(&preferredHeight, engine->floatprop)
		});
}

float UWindow::QueryPreferredWidth(float queryHeight)
{
	float width = 0.0f;
	CallEvent(this, "ParentRequestedPreferredSize", {
		ExpressionValue::BoolValue(false),
		ExpressionValue::Variable(&width, engine->floatprop),
		ExpressionValue::BoolValue(true),
		ExpressionValue::Variable(&queryHeight, engine->floatprop)
		});
	return width;
}

void UWindow::Raise()
{
	if (!parentOwner())
		return;
	UWindow* owner = parentOwner();
	DetachFromParent();
	parentOwner() = owner;
	if (owner->lastChild())
	{
		prevSibling() = owner->lastChild();
		owner->lastChild()->nextSibling() = this;
		owner->lastChild() = this;
	}
	else
	{
		owner->firstChild() = this;
		owner->lastChild() = this;
	}
}

void UWindow::ReleaseGC(UObject* GC)
{
	// Do nothing here for now.
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
	Background() = UObject::Cast<UTexture>(newBackground);
}

void UWindow::SetBackgroundSmoothing(bool newSmoothing)
{
	bSmoothBackground() = newSmoothing;
}

void UWindow::SetBackgroundStretching(bool newStretching)
{
	bStretchBackground() = newStretching;
}

void UWindow::SetBackgroundStyle(uint8_t NewStyle)
{
	backgroundStyle() = NewStyle;
}

void UWindow::SetBaselineData(float* newBaselineOffset, float* newUnderlineHeight)
{
	LogUnimplemented("Window.SetBaselineData");
}

void UWindow::SetBoldFont(UObject* fn)
{
	boldFont() = UObject::Cast<UFont>(fn);
}

void UWindow::SetChildVisibility(bool bNewVisibility)
{
	LogUnimplemented("Window.SetChildVisibility");
}

void UWindow::SetClientObject(UObject* newClientObject)
{
	clientObject() = newClientObject;
}

void UWindow::SetConfiguration(float newX, float newY, float newWidth, float NewHeight)
{
	X() = newX;
	Y() = newY;
	Width() = newWidth;
	Height() = NewHeight;
}

void UWindow::SetCursorPos(float newMouseX, float newMouseY)
{
	LogUnimplemented("Window.SetCursorPos");
}

void UWindow::SetDefaultCursor(UObject* tX, UObject** shadowTexture, float* HotX, float* HotY, Color* cursorColor)
{
	defaultCursor() = UObject::Cast<UTexture>(tX);
	if (shadowTexture)
		defaultCursorShadow() = UObject::Cast<UTexture>(*shadowTexture);
	if (HotX)
		defaultHotX() = *HotX;
	if (HotY)
		defaultHotY() = *HotY;
	if (cursorColor)
		defaultCursorColor() = *cursorColor;
}

void UWindow::SetFocusSounds(UObject** newFocusSound, UObject** newUnfocusSound)
{
	if (newFocusSound)
		focusSound() = UObject::Cast<USound>(*newFocusSound);
	if (newUnfocusSound)
		unfocusSound() = UObject::Cast<USound>(*newUnfocusSound);
}

bool UWindow::SetFocusWindow(UObject* NewFocusWindow)
{
	LogUnimplemented("Window.SetFocusWindow");
	return false;
}

void UWindow::SetFont(UObject* fn)
{
	normalFont() = UObject::Cast<UFont>(fn);
	boldFont() = UObject::Cast<UFont>(fn);
}

void UWindow::SetFonts(UObject* nFont, UObject* bFont)
{
	normalFont() = UObject::Cast<UFont>(nFont);
	boldFont() = UObject::Cast<UFont>(bFont);
}

void UWindow::SetHeight(float NewHeight)
{
	Height() = NewHeight;
}

void UWindow::SetNormalFont(UObject* fn)
{
	normalFont() = UObject::Cast<UFont>(fn);
}

void UWindow::SetPos(float newX, float newY)
{
	X() = newX;
	Y() = newY;
}

void UWindow::SetSelectability(bool newSelectability)
{
	bIsSelectable() = newSelectability;
}

void UWindow::SetSensitivity(bool newSensitivity)
{
	bIsSensitive() = newSensitivity;
}

void UWindow::SetSize(float newWidth, float NewHeight)
{
	Width() = newWidth;
	Height() = NewHeight;
}

void UWindow::SetSoundVolume(float newVolume)
{
	SoundVolume() = newVolume;
}

void UWindow::SetTextColor(const Color& NewColor)
{
	TextColor() = NewColor;
}

void UWindow::SetTileColor(const Color& NewColor)
{
	tileColor() = NewColor;
}

void UWindow::SetVisibilitySounds(UObject** visSound, UObject** invisSound)
{
	if (visSound)
		visibleSound() = UObject::Cast<USound>(*visSound);
	if (invisSound)
		invisibleSound() = UObject::Cast<USound>(*invisSound);
}

void UWindow::SetWidth(float newWidth)
{
	Width() = newWidth;
}

void UWindow::SetWindowAlignments(uint8_t HAlign, uint8_t VAlign, float* hMargin0, float* vMargin0, float* hMargin1, float* vMargin1)
{
	LogUnimplemented("Window.SetWindowAlignments");
}

void UWindow::Show(BitfieldBool* bShow)
{
	bool show = !bShow || *bShow;
	bIsVisible() = show;
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
	if (NewTexture)
		DefaultTexture() = UObject::Cast<UTexture>(*NewTexture);
	if (NewColor)
		DefaultColor() = *NewColor;
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
	if (newWatchActor)
		watchActor() = UObject::Cast<UActor>(*newWatchActor);
	if (bEyeLevel)
		bWatchEyeHeight() = *bEyeLevel;
}

void UViewportWindow::ShowViewportActor(BitfieldBool* bShow)
{
	bShowActor() = !bShow || *bShow;
}

void UViewportWindow::ShowWeapons(BitfieldBool* bShow)
{
	bShowWeapons() = !bShow || *bShow;
}

/////////////////////////////////////////////////////////////////////////////

void UTileWindow::EnableWrapping(bool bWrapOn)
{
	bWrap() = bWrapOn;
}

void UTileWindow::FillParent(bool FillParent)
{
	bFillParent() = FillParent;
}

void UTileWindow::MakeHeightsEqual(bool bEqual)
{
	bEqualHeight() = bEqual;
}

void UTileWindow::MakeWidthsEqual(bool bEqual)
{
	bEqualWidth() = bEqual;
}

void UTileWindow::SetChildAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	hChildAlign() = newHAlign;
	vChildAlign() = newVAlign;
}

void UTileWindow::SetDirections(uint8_t newHDir, uint8_t newVDir)
{
	hDirection() = newHDir;
	vDirection() = newVDir;
}

void UTileWindow::SetMajorSpacing(float newSpacing)
{
	majorSpacing() = newSpacing;
}

void UTileWindow::SetMargins(float newHMargin, float newVMargin)
{
	hMargin() = newHMargin;
	vMargin() = newVMargin;
}

void UTileWindow::SetMinorSpacing(float newSpacing)
{
	minorSpacing() = newSpacing;
}

void UTileWindow::SetOrder(uint8_t newOrder)
{
	LogUnimplemented("TileWindow.SetOrder");
}

void UTileWindow::SetOrientation(uint8_t newOrientation)
{
	orientation() = newOrientation;
}

/////////////////////////////////////////////////////////////////////////////

void UTextWindow::AppendText(const std::string& NewText)
{
	Text() += NewText;
}

void UTextWindow::EnableTextAsAccelerator(BitfieldBool* bEnable)
{
	LogUnimplemented("TextWindow.EnableTextAsAccelerator");
}

std::string UTextWindow::GetText()
{
	return Text();
}

int UTextWindow::GetTextLength()
{
	return (int)Text().size();
}

int UTextWindow::GetTextPart(int startPos, int Count, std::string& OutText)
{
	int start = std::max(startPos, 0);
	int end = std::min(startPos + Count, (int)Text().size());
	OutText = Text().substr(start, end - start);
	return (int)OutText.size();
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
	minLines() = newMinLines;
	MaxLines() = newMaxLines;
}

void UTextWindow::SetMaxLines(int newMaxLines)
{
	MaxLines() = newMaxLines;
}

void UTextWindow::SetMinLines(int newMinLines)
{
	minLines() = newMinLines;
}

void UTextWindow::SetMinWidth(float newMinWidth)
{
	MinWidth() = newMinWidth;
}

void UTextWindow::SetText(const std::string& NewText)
{
	Text() = NewText;
}

void UTextWindow::SetTextAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	HAlign() = newHAlign;
	VAlign() = newVAlign;
}

void UTextWindow::SetTextMargins(float newHMargin, float newVMargin)
{
	hMargin() = newHMargin;
	vMargin() = newVMargin;
}

void UTextWindow::SetWordWrap(bool bNewWordWrap)
{
	bWordWrap() = bNewWordWrap;
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
	if (Normal)
		curTexture() = UObject::Cast<UTexture>(*Normal);
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

void UToggleWindow::SetToggleSounds(UObject** newEnableSound, UObject** newDisableSound)
{
	if (newEnableSound)
		enableSound() = UObject::Cast<USound>(*newEnableSound);
	if (newDisableSound)
		disableSound() = UObject::Cast<USound>(*newDisableSound);
}

/////////////////////////////////////////////////////////////////////////////

void UCheckboxWindow::SetCheckboxColor(const Color& NewColor)
{
	checkboxColor() = NewColor;
}

void UCheckboxWindow::SetCheckboxSpacing(float newSpacing)
{
	checkboxSpacing() = newSpacing;
}

void UCheckboxWindow::SetCheckboxStyle(uint8_t NewStyle)
{
	checkboxStyle() = NewStyle;
}

void UCheckboxWindow::SetCheckboxTextures(UObject** newToggleOff, UObject** newToggleOn, float* newTextureWidth, float* newTextureHeight)
{
	if (newToggleOff)
		toggleOff() = UObject::Cast<UTexture>(*newToggleOff);
	if (newToggleOn)
		toggleOn() = UObject::Cast<UTexture>(*newToggleOn);
	if (newTextureWidth)
		textureWidth() = *newTextureWidth;
	if (newTextureHeight)
		textureHeight() = *newTextureHeight;
}

void UCheckboxWindow::ShowCheckboxOnRightSide(BitfieldBool* bRight)
{
	bRightSide() = !bRight || *bRight;
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
	focusMode() = newFocusMode;
}

/////////////////////////////////////////////////////////////////////////////

void URootWindow::EnablePositionalSound(BitfieldBool* bEnable)
{
	bPositionalSound() = !bEnable || *bEnable;
}

void URootWindow::EnableRendering(BitfieldBool* newRender)
{
	bRender() = !newRender || *newRender;
}

UObject* URootWindow::GenerateSnapshot(BitfieldBool* bFilter)
{
	LogUnimplemented("RootWindow.GenerateSnapshot");
	return nullptr;
}

bool URootWindow::IsPositionalSoundEnabled()
{
	return bPositionalSound();
}

bool URootWindow::IsRenderingEnabled()
{
	return bRender();
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
	if (newEditCursor)
		defaultEditCursor() = UObject::Cast<UTexture>(*newEditCursor);
}

void URootWindow::SetDefaultMovementCursors(UObject** newMovementCursor, UObject** newHorizontalMovementCursor, UObject** newVerticalMovementCursor, UObject** newTopLeftMovementCursor, UObject** newTopRightMovementCursor)
{
	if (newMovementCursor)
		DefaultMoveCursor() = UObject::Cast<UTexture>(*newMovementCursor);
	if (newHorizontalMovementCursor)
		defaultHorizontalMoveCursor() = UObject::Cast<UTexture>(*newHorizontalMovementCursor);
	if (newVerticalMovementCursor)
		defaultVerticalMoveCursor() = UObject::Cast<UTexture>(*newVerticalMovementCursor);
	if (newTopLeftMovementCursor)
		defaultTopLeftMoveCursor() = UObject::Cast<UTexture>(*newTopLeftMovementCursor);
	if (newTopRightMovementCursor)
		defaultTopRightMoveCursor() = UObject::Cast<UTexture>(*newTopRightMovementCursor);
}

void URootWindow::SetRawBackground(UObject** NewTexture, Color* NewColor)
{
	if (NewTexture)
		rawBackground() = UObject::Cast<UTexture>(*NewTexture);
	if (NewColor)
		rawColor() = *NewColor;
}

void URootWindow::SetRawBackgroundSize(float newWidth, float NewHeight)
{
	rawBackgroundWidth() = newWidth;
	rawBackgroundHeight() = NewHeight;
}

void URootWindow::SetRenderViewport(float newX, float newY, float newWidth, float NewHeight)
{
	renderX() = newX;
	renderY() = newY;
	renderWidth() = newWidth;
	renderHeight() = NewHeight;
}

void URootWindow::SetSnapshotSize(float newWidth, float NewHeight)
{
	LogUnimplemented("RootWindow.SetSnapshotSize");
}

void URootWindow::ShowCursor(BitfieldBool* bShow)
{
	bCursorVisible() = !bShow || *bShow;
}

void URootWindow::StretchRawBackground(BitfieldBool* bStretch)
{
	bStretchRawBackground() = !bStretch || *bStretch;
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
	marginWidth() = newMarginWidth;
	marginHeight() = newMarginHeight;
}

void UScrollAreaWindow::SetScrollbarDistance(float newDistance)
{
	scrollbarDistance() = newDistance;
}

/////////////////////////////////////////////////////////////////////////////

void UBorderWindow::BaseMarginsFromBorder(BitfieldBool* bBorder)
{
	bMarginsFromBorder() = !bBorder || *bBorder;
}

void UBorderWindow::EnableResizing(BitfieldBool* bResize)
{
	bResizeable() = !bResize || *bResize;
}

void UBorderWindow::SetBorderMargins(float* NewLeft, float* NewRight, float* newTop, float* newBottom)
{
	if (NewLeft)
		childLeftMargin() = *NewLeft;
	if (NewRight)
		childRightMargin() = *NewRight;
	if (newTop)
		childTopMargin() = *newTop;
	if (newBottom)
		childBottomMargin() = *newBottom;
}

void UBorderWindow::SetBorders(UObject** bordTL, UObject** bordTR, UObject** bordBL, UObject** bordBR, UObject** bordL, UObject** bordR, UObject** bordT, UObject** bordB, UObject** newCenter)
{
	if (bordTL)
		borderTopLeft() = UObject::Cast<UTexture>(*bordTL);
	if (bordTR)
		borderTopRight() = UObject::Cast<UTexture>(*bordTR);
	if (bordBL)
		borderBottomLeft() = UObject::Cast<UTexture>(*bordBL);
	if (bordBR)
		borderBottomRight() = UObject::Cast<UTexture>(*bordBR);
	if (bordL)
		borderLeft() = UObject::Cast<UTexture>(*bordL);
	if (bordR)
		borderRight() = UObject::Cast<UTexture>(*bordR);
	if (bordT)
		borderTop() = UObject::Cast<UTexture>(*bordT);
	if (bordB)
		borderBottom() = UObject::Cast<UTexture>(*bordB);
	if (newCenter)
		center() = UObject::Cast<UTexture>(*newCenter);
}

void UBorderWindow::SetMoveCursors(UObject** Move, UObject** hMove, UObject** vMove, UObject** tlMove, UObject** trMove)
{
	if (Move)
		MoveCursor() = UObject::Cast<UTexture>(*Move);
	if (hMove)
		hMoveCursor() = UObject::Cast<UTexture>(*hMove);
	if (vMove)
		vMoveCursor() = UObject::Cast<UTexture>(*vMove);
	if (tlMove)
		tlMoveCursor() = UObject::Cast<UTexture>(*tlMove);
	if (trMove)
		trMoveCursor() = UObject::Cast<UTexture>(*trMove);
}

/////////////////////////////////////////////////////////////////////////////

void UScaleWindow::ClearAllEnumerations()
{
	LogUnimplemented("ScaleWindow.ClearAllEnumerations");
}

void UScaleWindow::EnableStretchedScale(BitfieldBool* bNewStretch)
{
	LogUnimplemented("ScaleWindow.EnableStretchedScale");
}

int UScaleWindow::GetNumTicks()
{
	LogUnimplemented("ScaleWindow.GetNumTicks");
	return 0;
}

int UScaleWindow::GetThumbSpan()
{
	LogUnimplemented("ScaleWindow.GetThumbSpan");
	return 0;
}

int UScaleWindow::GetTickPosition()
{
	LogUnimplemented("ScaleWindow.GetTickPosition");
	return 0;
}

float UScaleWindow::GetValue()
{
	LogUnimplemented("ScaleWindow.GetValue");
	return 0.0f;
}

std::string UScaleWindow::GetValueString()
{
	LogUnimplemented("ScaleWindow.GetValueString");
	return "";
}

void UScaleWindow::GetValues(float& fromValue, float& toValue)
{
	LogUnimplemented("ScaleWindow.GetValues");
}

void UScaleWindow::MoveThumb(uint8_t MoveThumb)
{
	LogUnimplemented("ScaleWindow.MoveThumb");
}

void UScaleWindow::PlayScaleSound(UObject* newsound, float* Volume, float* Pitch)
{
	LogUnimplemented("ScaleWindow.PlayScaleSound");
}

void UScaleWindow::SetBorderPattern(UObject* NewTexture)
{
	LogUnimplemented("ScaleWindow.SetBorderPattern");
}

void UScaleWindow::SetEnumeration(int tickPos, const std::string& newStr)
{
	LogUnimplemented("ScaleWindow.SetEnumeration");
}

void UScaleWindow::SetNumTicks(int newNumTicks)
{
	LogUnimplemented("ScaleWindow.SetNumTicks");
}

void UScaleWindow::SetScaleBorder(float* newBorderSize, Color* NewColor)
{
	LogUnimplemented("ScaleWindow.SetScaleBorder");
}

void UScaleWindow::SetScaleColor(const Color& NewColor)
{
	LogUnimplemented("ScaleWindow.SetScaleColor");
}

void UScaleWindow::SetScaleMargins(float* marginWidth, float* marginHeight)
{
	LogUnimplemented("ScaleWindow.SetScaleMargins");
}

void UScaleWindow::SetScaleOrientation(uint8_t newOrientation)
{
	LogUnimplemented("ScaleWindow.SetScaleOrientation");
}

void UScaleWindow::SetScaleSounds(UObject** setSound, UObject** clickSound, UObject** dragSound)
{
	LogUnimplemented("ScaleWindow.SetScaleSounds");
}

void UScaleWindow::SetScaleStyle(uint8_t NewStyle)
{
	LogUnimplemented("ScaleWindow.SetScaleStyle");
}

void UScaleWindow::SetScaleTexture(UObject* NewTexture, float* newWidth, float* NewHeight, float* newStart, float* newEnd)
{
	LogUnimplemented("ScaleWindow.SetScaleTexture");
}

void UScaleWindow::SetThumbBorder(float* newBorderSize, Color* NewColor)
{
	LogUnimplemented("ScaleWindow.SetThumbBorder");
}

void UScaleWindow::SetThumbCaps(UObject* preCap, UObject* postCap, float* preCapWidth, float* preCapHeight, float* postCapWidth, float* postCapHeight)
{
	LogUnimplemented("ScaleWindow.SetThumbCaps");
}

void UScaleWindow::SetThumbColor(const Color& NewColor)
{
	LogUnimplemented("ScaleWindow.SetThumbColor");
}

void UScaleWindow::SetThumbSpan(int* newRange)
{
	LogUnimplemented("ScaleWindow.SetThumbSpan");
}

void UScaleWindow::SetThumbStep(int NewStep)
{
	LogUnimplemented("ScaleWindow.SetThumbStep");
}

void UScaleWindow::SetThumbStyle(uint8_t NewStyle)
{
	LogUnimplemented("ScaleWindow.SetThumbStyle");
}

void UScaleWindow::SetThumbTexture(UObject* NewTexture, float* newWidth, float* NewHeight)
{
	LogUnimplemented("ScaleWindow.SetThumbTexture");
}

void UScaleWindow::SetTickColor(const Color& NewColor)
{
	LogUnimplemented("ScaleWindow.SetTickColor");
}

void UScaleWindow::SetTickPosition(int newPosition)
{
	LogUnimplemented("ScaleWindow.SetTickPosition");
}

void UScaleWindow::SetTickStyle(uint8_t NewStyle)
{
	LogUnimplemented("ScaleWindow.SetTickStyle");
}

void UScaleWindow::SetTickTexture(UObject* tickTexture, BitfieldBool* bDrawEndTicks, float* newWidth, float* NewHeight)
{
	LogUnimplemented("ScaleWindow.SetTickTexture");
}

void UScaleWindow::SetValue(float NewValue)
{
	LogUnimplemented("ScaleWindow.SetValue");
}

void UScaleWindow::SetValueFormat(const std::string& newFmt)
{
	LogUnimplemented("ScaleWindow.SetValueFormat");
}

void UScaleWindow::SetValueRange(float newFrom, float newTo)
{
	LogUnimplemented("ScaleWindow.SetValueRange");
}

/////////////////////////////////////////////////////////////////////////////

void UScaleManagerWindow::SetManagerAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	childHAlign() = newHAlign;
	childVAlign() = newVAlign;
}

void UScaleManagerWindow::SetManagerMargins(float* newMarginWidth, float* newMarginHeight)
{
	if (newMarginWidth)
		marginWidth() = *newMarginWidth;
	if (newMarginHeight)
		marginHeight() = *newMarginHeight;
}

void UScaleManagerWindow::SetManagerOrientation(uint8_t newOrientation)
{
	orientation() = newOrientation;
}

void UScaleManagerWindow::SetMarginSpacing(float* newSpacing)
{
	if (newSpacing)
		Spacing() = *newSpacing;
}

void UScaleManagerWindow::SetScale(UObject* NewScale)
{
	Scale() = UObject::Cast<UScaleWindow>(NewScale);
}

void UScaleManagerWindow::SetScaleButtons(UObject* newDecButton, UObject* newIncButton)
{
	if (newDecButton)
		decButton() = UObject::Cast<UButtonWindow>(newDecButton);
	if (newIncButton)
		incButton() = UObject::Cast<UButtonWindow>(newIncButton);
}

void UScaleManagerWindow::SetValueField(UObject* newValueField)
{
	valueField() = UObject::Cast<UTextWindow>(newValueField);
}

void UScaleManagerWindow::StretchScaleField(BitfieldBool* bNewStretch)
{
	bStretchScaleField() = !bNewStretch || *bNewStretch;
}

void UScaleManagerWindow::StretchValueField(BitfieldBool* bNewStretch)
{
	bStretchValueField() = !bNewStretch || *bNewStretch;
}

/////////////////////////////////////////////////////////////////////////////

int UListWindow::AddRow(const std::string& rowStr, int* clientData)
{
	LogUnimplemented("ListWindow.AddRow");
	return 0;
}

void UListWindow::AddSortColumn(int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive)
{
	LogUnimplemented("ListWindow.AddSortColumn");
}

void UListWindow::DeleteAllRows()
{
	LogUnimplemented("ListWindow.DeleteAllRows");
}

void UListWindow::DeleteRow(int rowId)
{
	LogUnimplemented("ListWindow.DeleteRow");
}

void UListWindow::EnableAutoExpandColumns(BitfieldBool* bAutoExpand)
{
	LogUnimplemented("ListWindow.EnableAutoExpandColumns");
}

void UListWindow::EnableAutoSort(BitfieldBool* bAutoSort)
{
	LogUnimplemented("ListWindow.EnableAutoSort");
}

void UListWindow::EnableHotKeys(BitfieldBool* bEnable)
{
	LogUnimplemented("ListWindow.EnableHotKeys");
}

void UListWindow::EnableMultiSelect(BitfieldBool* bEnableMultiSelect)
{
	LogUnimplemented("ListWindow.EnableMultiSelect");
}

uint8_t UListWindow::GetColumnAlignment(int colIndex)
{
	LogUnimplemented("ListWindow.GetColumnAlignment");
	return 0;
}

void UListWindow::GetColumnColor(int colIndex, Color& colColor)
{
	LogUnimplemented("ListWindow.GetColumnColor");
}

UObject* UListWindow::GetColumnFont(int colIndex)
{
	LogUnimplemented("ListWindow.GetColumnFont");
	return nullptr;
}

std::string UListWindow::GetColumnTitle(int colIndex)
{
	LogUnimplemented("ListWindow.GetColumnTitle");
	return "";
}

uint8_t UListWindow::GetColumnType(int colIndex)
{
	LogUnimplemented("ListWindow.GetColumnType");
	return 0;
}

float UListWindow::GetColumnWidth(int colIndex)
{
	LogUnimplemented("ListWindow.GetColumnWidth");
	return 0.0f;
}

std::string UListWindow::GetField(int rowId, int colIndex)
{
	LogUnimplemented("ListWindow.GetField");
	return "";
}

void UListWindow::GetFieldMargins(float& marginWidth, float& marginHeight)
{
	LogUnimplemented("ListWindow.GetFieldMargins");
}

float UListWindow::GetFieldValue(int rowId, int colIndex)
{
	LogUnimplemented("ListWindow.GetFieldValue");
	return 0.0f;
}

int UListWindow::GetFocusRow()
{
	LogUnimplemented("ListWindow.GetFocusRow");
	return 0;
}

int UListWindow::GetNumColumns()
{
	LogUnimplemented("ListWindow.GetNumColumns");
	return 0;
}

int UListWindow::GetNumRows()
{
	LogUnimplemented("ListWindow.GetNumRows");
	return 0;
}

int UListWindow::GetNumSelectedRows()
{
	LogUnimplemented("ListWindow.GetNumSelectedRows");
	return 0;
}

int UListWindow::GetPageSize()
{
	LogUnimplemented("ListWindow.GetPageSize");
	return 0;
}

int UListWindow::GetRowClientInt(int rowId)
{
	LogUnimplemented("ListWindow.GetRowClientInt");
	return 0;
}

UObject* UListWindow::GetRowClientObject(int rowId)
{
	LogUnimplemented("ListWindow.GetRowClientObject");
	return nullptr;
}

int UListWindow::GetSelectedRow()
{
	LogUnimplemented("ListWindow.GetSelectedRow");
	return 0;
}

void UListWindow::HideColumn(int colIndex, BitfieldBool* bHide)
{
	LogUnimplemented("ListWindow.HideColumn");
}

int UListWindow::IndexToRowId(int index)
{
	LogUnimplemented("ListWindow.IndexToRowId");
	return 0;
}

bool UListWindow::IsAutoExpandColumnsEnabled()
{
	LogUnimplemented("ListWindow.IsAutoExpandColumnsEnabled");
	return false;
}

bool UListWindow::IsAutoSortEnabled()
{
	LogUnimplemented("ListWindow.IsAutoSortEnabled");
	return false;
}

bool UListWindow::IsColumnHidden(int colIndex)
{
	LogUnimplemented("ListWindow.IsColumnHidden");
	return false;
}

bool UListWindow::IsMultiSelectEnabled()
{
	LogUnimplemented("ListWindow.IsMultiSelectEnabled");
	return false;
}

bool UListWindow::IsRowSelected(int rowId)
{
	LogUnimplemented("ListWindow.IsRowSelected");
	return false;
}

void UListWindow::ModifyRow(int rowId, const std::string& rowStr)
{
	LogUnimplemented("ListWindow.ModifyRow");
}

void UListWindow::MoveRow(uint8_t Move, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag)
{
	LogUnimplemented("ListWindow.MoveRow");
}

void UListWindow::PlayListSound(UObject* listSound, float* Volume, float* Pitch)
{
	LogUnimplemented("ListWindow.PlayListSound");
}

void UListWindow::RemoveSortColumn(int colIndex)
{
	LogUnimplemented("ListWindow.RemoveSortColumn");
}

void UListWindow::ResetSortColumns(BitfieldBool* bSort)
{
	LogUnimplemented("ListWindow.ResetSortColumns");
}

void UListWindow::ResizeColumns(BitfieldBool* bExpandOnly)
{
	LogUnimplemented("ListWindow.ResizeColumns");
}

int UListWindow::RowIdToIndex(int rowId)
{
	LogUnimplemented("ListWindow.RowIdToIndex");
	return 0;
}

void UListWindow::SelectAllRows(BitfieldBool* bSelect)
{
	LogUnimplemented("ListWindow.SelectAllRows");
}

void UListWindow::SelectRow(int rowId, BitfieldBool* bSelect)
{
	LogUnimplemented("ListWindow.SelectRow");
}

void UListWindow::SelectToRow(int rowId, BitfieldBool* bClearRows, BitfieldBool* bInvert, BitfieldBool* bSpanRows)
{
	LogUnimplemented("ListWindow.SelectToRow");
}

void UListWindow::SetColumnAlignment(int colIndex, uint8_t newAlign)
{
	LogUnimplemented("ListWindow.SetColumnAlignment");
}

void UListWindow::SetColumnColor(int colIndex, const Color& NewColor)
{
	LogUnimplemented("ListWindow.SetColumnColor");
}

void UListWindow::SetColumnFont(int colIndex, UObject* NewFont)
{
	LogUnimplemented("ListWindow.SetColumnFont");
}

void UListWindow::SetColumnTitle(int colIndex, const std::string& Title)
{
	LogUnimplemented("ListWindow.SetColumnTitle");
}

void UListWindow::SetColumnType(int colIndex, uint8_t newType, std::string* newFmt)
{
	LogUnimplemented("ListWindow.SetColumnType");
}

void UListWindow::SetColumnWidth(int colIndex, float newWidth)
{
	LogUnimplemented("ListWindow.SetColumnWidth");
}

void UListWindow::SetDelimiter(const std::string& newDelimiter)
{
	LogUnimplemented("ListWindow.SetDelimiter");
}

void UListWindow::SetField(int rowId, int colIndex, const std::string& fieldStr)
{
	LogUnimplemented("ListWindow.SetField");
}

void UListWindow::SetFieldMargins(float newMarginWidth, float newMarginHeight)
{
	LogUnimplemented("ListWindow.SetFieldMargins");
}

void UListWindow::SetFieldValue(int rowId, int colIndex, float NewValue)
{
	LogUnimplemented("ListWindow.SetFieldValue");
}

void UListWindow::SetFocusColor(const Color& NewColor)
{
	LogUnimplemented("ListWindow.SetFocusColor");
}

void UListWindow::SetFocusRow(int rowId, BitfieldBool* bMoveTo, BitfieldBool* bAnchor)
{
	LogUnimplemented("ListWindow.SetFocusRow");
}

void UListWindow::SetFocusTexture(UObject* NewTexture)
{
	LogUnimplemented("ListWindow.SetFocusTexture");
}

void UListWindow::SetFocusThickness(float newThickness)
{
	LogUnimplemented("ListWindow.SetFocusThickness");
}

void UListWindow::SetHighlightColor(const Color& NewColor)
{
	LogUnimplemented("ListWindow.SetHighlightColor");
}

void UListWindow::SetHighlightTextColor(const Color& NewColor)
{
	LogUnimplemented("ListWindow.SetHighlightTextColor");
}

void UListWindow::SetHighlightTexture(UObject* NewTexture)
{
	LogUnimplemented("ListWindow.SetHighlightTexture");
}

void UListWindow::SetHotKeyColumn(int colIndex)
{
	LogUnimplemented("ListWindow.SetHotKeyColumn");
}

void UListWindow::SetListSounds(UObject** ActivateSound, UObject** moveSound)
{
	LogUnimplemented("ListWindow.SetListSounds");
}

void UListWindow::SetNumColumns(int newCols)
{
	LogUnimplemented("ListWindow.SetNumColumns");
}

void UListWindow::SetRow(int rowId, BitfieldBool* bSelect, BitfieldBool* bClearRows, BitfieldBool* bDrag)
{
	LogUnimplemented("ListWindow.SetRow");
}

void UListWindow::SetRowClientInt(int rowId, int clientInt)
{
	LogUnimplemented("ListWindow.SetRowClientInt");
}

void UListWindow::SetRowClientObject(int rowId, UObject* clientObj)
{
	LogUnimplemented("ListWindow.SetRowClientObject");
}

void UListWindow::SetSortColumn(int colIndex, BitfieldBool* bReverse, BitfieldBool* bCaseSensitive)
{
	LogUnimplemented("ListWindow.SetSortColumn");
}

void UListWindow::ShowFocusRow()
{
	LogUnimplemented("ListWindow.ShowFocusRow");
}

void UListWindow::Sort()
{
	LogUnimplemented("ListWindow.Sort");
}

void UListWindow::ToggleRowSelection(int rowId)
{
	LogUnimplemented("ListWindow.ToggleRowSelection");
}

/////////////////////////////////////////////////////////////////////////////

void UComputerWindow::ClearLine(int rowToClear)
{
	LogUnimplemented("ComputerWindow.ClearLine");
}

void UComputerWindow::ClearScreen()
{
	LogUnimplemented("ComputerWindow.ClearScreen");
}

void UComputerWindow::EnableWordWrap(BitfieldBool* bNewWordWrap)
{
	LogUnimplemented("ComputerWindow.EnableWordWrap");
}

void UComputerWindow::FadeOutText(float* fadeDuration)
{
	LogUnimplemented("ComputerWindow.FadeOutText");
}

void UComputerWindow::GetChar(const std::string& inputKey, BitfieldBool* bEcho)
{
	LogUnimplemented("ComputerWindow.GetChar");
}

void UComputerWindow::GetInput(int MaxLength, const std::string& inputKey, std::string* defaultInputString, std::string* inputMask)
{
	LogUnimplemented("ComputerWindow.GetInput");
}

float UComputerWindow::GetThrottle()
{
	LogUnimplemented("ComputerWindow.GetThrottle");
	return 0.0f;
}

bool UComputerWindow::IsBufferFlushed()
{
	LogUnimplemented("ComputerWindow.IsBufferFlushed");
	return false;
}

bool UComputerWindow::IsPaused()
{
	LogUnimplemented("ComputerWindow.IsPaused");
	return false;
}

void UComputerWindow::Pause(float* pauseLength)
{
	LogUnimplemented("ComputerWindow.Pause");
}

void UComputerWindow::PlaySoundLater(UObject* newsound)
{
	LogUnimplemented("ComputerWindow.PlaySoundLater");
}

void UComputerWindow::Print(const std::string& printText, BitfieldBool* bNewLine)
{
	LogUnimplemented("ComputerWindow.Print");
}

void UComputerWindow::PrintGraphic(UObject* Graphic, int Width, int Height, int* posX, int* posY, BitfieldBool* bStatic, BitfieldBool* bPixelPos)
{
	LogUnimplemented("ComputerWindow.PrintGraphic");
}

void UComputerWindow::PrintLn()
{
	LogUnimplemented("ComputerWindow.PrintLn");
}

void UComputerWindow::ResetThrottle()
{
	LogUnimplemented("ComputerWindow.ResetThrottle");
}

void UComputerWindow::Resume()
{
	LogUnimplemented("ComputerWindow.Resume");
}

void UComputerWindow::SetBackgroundTextures(UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6)
{
	LogUnimplemented("ComputerWindow.SetBackgroundTextures");
}

void UComputerWindow::SetComputerSoundVolume(float newSoundVolume)
{
	LogUnimplemented("ComputerWindow.SetComputerSoundVolume");
}

void UComputerWindow::SetCursorBlinkSpeed(float newBlinkSpeed)
{
	LogUnimplemented("ComputerWindow.SetCursorBlinkSpeed");
}

void UComputerWindow::SetCursorColor(const Color& newCursorColor)
{
	LogUnimplemented("ComputerWindow.SetCursorColor");
}

void UComputerWindow::SetCursorTexture(UObject* newCursorTexture, int* newCursorWidth, int* newCursorHeight)
{
	LogUnimplemented("ComputerWindow.SetCursorTexture");
}

void UComputerWindow::SetFadeSpeed(float fadeSpeed)
{
	LogUnimplemented("ComputerWindow.SetFadeSpeed");
}

void UComputerWindow::SetFontColor(const Color& newFontColor)
{
	LogUnimplemented("ComputerWindow.SetFontColor");
}

void UComputerWindow::SetTextFont(UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor)
{
	LogUnimplemented("ComputerWindow.SetTextFont");
}

void UComputerWindow::SetTextPosition(int posX, int posY)
{
	LogUnimplemented("ComputerWindow.SetTextPosition");
}

void UComputerWindow::SetTextSize(int newCols, int newRows)
{
	LogUnimplemented("ComputerWindow.SetTextSize");
}

void UComputerWindow::SetTextSound(UObject* newTextSound)
{
	LogUnimplemented("ComputerWindow.SetTextSound");
}

void UComputerWindow::SetTextTiming(float newTiming)
{
	LogUnimplemented("ComputerWindow.SetTextTiming");
}

void UComputerWindow::SetTextWindowPosition(int newX, int newY)
{
	LogUnimplemented("ComputerWindow.SetTextWindowPosition");
}

void UComputerWindow::SetThrottle(float throttleModifier)
{
	LogUnimplemented("ComputerWindow.SetThrottle");
}

void UComputerWindow::SetTypingSound(UObject* newTypingSound)
{
	LogUnimplemented("ComputerWindow.SetTypingSound");
}

void UComputerWindow::SetTypingSoundVolume(float newSoundVolume)
{
	LogUnimplemented("ComputerWindow.SetTypingSoundVolume");
}

void UComputerWindow::ShowTextCursor(BitfieldBool* bShow)
{
	LogUnimplemented("ComputerWindow.ShowTextCursor");
}

/////////////////////////////////////////////////////////////////////////////

void UGC::ClearZ()
{
	// Only used by ActorDisplayWindow.DrawWindow
	engine->render->Device->ClearZ();
}

void UGC::CopyGC(UObject* Copy)
{
	// Not used directly by scripts
	LogUnimplemented("GC.CopyGC");
}

void UGC::DrawActor(UObject* Actor, BitfieldBool* bClearZ, BitfieldBool* bConstrain, BitfieldBool* bUnlit, float* DrawScale, float* ScaleGlow, UObject** Skin)
{
	// Only used by ActorDisplayWindow.DrawWindow, AugmentationDisplayWindow.DrawWindow
	LogUnimplemented("GC.DrawActor");
}

void UGC::DrawBorders(float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject* borders, BitfieldBool* bStretchHorizontally, BitfieldBool* bStretchVertically)
{
	LogUnimplemented("GC.DrawBorders");
}

void UGC::DrawBox(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX)
{
	LogUnimplemented("GC.DrawBox");
}

void UGC::DrawIcon(float DestX, float DestY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		vec4 color(1.0f);
		uint32_t polyflags = 0;
		if (bMasked())
			polyflags |= PF_Masked;
		if (bModulated())
			polyflags |= PF_Modulated;
		if (!bSmoothed())
			polyflags |= PF_NoSmooth;

		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		engine->render->DrawTile(tex, offsetX + DestX * scale.x, offsetY + DestY * scale.y, swidth * scale.x, sheight * scale.y, 0.0f, 0.0f, swidth, sheight, 1.0f, color, vec4(0.0), polyflags);
	}
}

void UGC::DrawPattern(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		vec4 color(1.0f);
		uint32_t polyflags = 0;
		if (bMasked())
			polyflags |= PF_Masked;
		if (bModulated())
			polyflags |= PF_Modulated;
		if (!bSmoothed())
			polyflags |= PF_NoSmooth;

		engine->render->DrawTile(tex, offsetX + DestX * scale.x, offsetY + DestY * scale.y, destWidth * scale.x, destHeight * scale.y, OrgX, OrgY, destWidth, destHeight, 1.0f, color, vec4(0.0), polyflags);
	}
}

void UGC::DrawStretchedTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX)
{
	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		vec4 color(1.0f);
		uint32_t polyflags = 0;
		if (bMasked())
			polyflags |= PF_Masked;
		if (bModulated())
			polyflags |= PF_Modulated;
		if (!bSmoothed())
			polyflags |= PF_NoSmooth;

		engine->render->DrawTile(tex, offsetX + DestX * scale.x, offsetY + DestY * scale.y, destWidth * scale.x, destHeight * scale.y, srcX, srcY, srcWidth, srcHeight, 1.0f, color, vec4(0.0), polyflags);
	}
}

void UGC::DrawText(float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr)
{
	if (!bDrawEnabled())
		return;

	UFont* font = normalFont();
	if (font)
	{
		vec4 color(1.0f);
		float curX = 0.0f, curY = 0.0f, curXL = 0.0f, curYL = 0.0f;
		uint32_t polyflags = PF_Masked | PF_NoSmooth;
		engine->render->DrawText(font, color, offsetX + DestX * scale.x, offsetY + DestY * scale.y, curX, curY, curXL, curYL, false, textStr, polyflags, false, 0.0f, 0.0f, 10000.0f, 10000.0f, false);
	}
}

void UGC::DrawTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		vec4 color(1.0f);
		uint32_t polyflags = 0;
		if (bMasked())
			polyflags |= PF_Masked;
		if (bModulated())
			polyflags |= PF_Modulated;
		if (!bSmoothed())
			polyflags |= PF_NoSmooth;

		engine->render->DrawTile(tex, offsetX + DestX * scale.x, offsetY + DestY * scale.y, destWidth * scale.x, destHeight * scale.y, srcX, srcY, destWidth, destHeight, 1.0f, color, vec4(0.0), polyflags);
	}
}

void UGC::EnableDrawing(bool newDrawEnabled)
{
	// Only set by ActorDisplayWindow.DrawWindow and always set to true.
	bDrawEnabled() = newDrawEnabled;
}

void UGC::EnableMasking(bool bNewMasking)
{
	bMasked() = bNewMasking;
}

void UGC::EnableModulation(bool bNewModulation)
{
	bModulated() = bNewModulation;
}

void UGC::EnableSmoothing(bool bNewSmoothing)
{
	bSmoothed() = bNewSmoothing;
}

void UGC::EnableSpecialText(bool bNewSpecialText)
{
	// Not used directly by scripts
	LogUnimplemented("GC.EnableSpecialText");
}

void UGC::EnableTranslucency(bool bNewTranslucency)
{
	bTranslucent() = bNewTranslucency;
}

void UGC::EnableTranslucentText(bool bNewTranslucency)
{
	bTextTranslucent() = bNewTranslucency;
}

void UGC::EnableWordWrap(bool bNewWordWrap)
{
	bWordWrap() = bNewWordWrap;
}

void UGC::GetAlignments(uint8_t& outHAlign, uint8_t& outVAlign)
{
	outHAlign = HAlign();
	outVAlign = VAlign();
}

float UGC::GetFontHeight(BitfieldBool* bIncludeSpace)
{
	// Not used directly by scripts
	LogUnimplemented("GC.GetFontHeight");
	return 0.0f;
}

void UGC::GetFonts(UObject*& outNormalFont, UObject*& outBoldFont)
{
	outNormalFont = normalFont();
	outBoldFont = boldFont();
}

uint8_t UGC::GetHorizontalAlignment()
{
	return HAlign();
}

uint8_t UGC::GetStyle()
{
	return Style();
}

void UGC::GetTextColor(Color& outTextColor)
{
	outTextColor = TextColor();
}

void UGC::GetTextExtent(float destWidth, float& xExtent, float& yExtent, const std::string& textStr)
{
	UFont* font = normalFont();
	if (font)
	{
		vec4 color(1.0f);
		float curX = 0.0f, curY = 0.0f, curXL = 0.0f, curYL = 0.0f;
		engine->render->DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, textStr, PF_Masked | PF_NoSmooth, false, 0.0f, 0.0f, destWidth, 10000.0f, true);
		xExtent = curXL;
		yExtent = curYL;
	}
	else
	{
		xExtent = 0.0f;
		yExtent = 0.0f;
	}
}

float UGC::GetTextVSpacing()
{
	return textVSpacing();
}

void UGC::GetTileColor(Color& outTileColor)
{
	outTileColor = tileColor();
}

uint8_t UGC::GetVerticalAlignment()
{
	return VAlign();
}

void UGC::Intersect(float ClipX, float ClipY, float clipWidth, float clipHeight)
{
	LogUnimplemented("GC.Intersect");
}

bool UGC::IsDrawingEnabled()
{
	return bDrawEnabled();
}

bool UGC::IsMaskingEnabled()
{
	return bMasked();
}

bool UGC::IsModulationEnabled()
{
	return bModulated();
}

bool UGC::IsSmoothingEnabled()
{
	return bSmoothed();
}

bool UGC::IsSpecialTextEnabled()
{
	// Not used directly by scripts
	LogUnimplemented("GC.IsSpecialTextEnabled");
	return false;
}

bool UGC::IsTranslucencyEnabled()
{
	return bTranslucent();
}

bool UGC::IsTranslucentTextEnabled()
{
	return bTextTranslucent();
}

bool UGC::IsWordWrapEnabled()
{
	return bWordWrap();
}

void UGC::PopGC(int* gcNum)
{
	// Not used directly by scripts
	LogUnimplemented("GC.PopGC");
}

int UGC::PushGC()
{
	// Not used directly by scripts
	LogUnimplemented("GC.PushGC");
	return 0;
}

void UGC::SetAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	HAlign() = newHAlign;
	VAlign() = newVAlign;
}

void UGC::SetBaselineData(float* newBaselineOffset, float* newUnderlineHeight)
{
	if (newBaselineOffset)
		baselineOffset() = *newBaselineOffset;
	if (newUnderlineHeight)
		underlineHeight() = *newUnderlineHeight;
}

void UGC::SetBoldFont(UObject* newBoldFont)
{
	boldFont() = UObject::Cast<UFont>(newBoldFont);
}

void UGC::SetFont(UObject* NewFont)
{
	normalFont() = UObject::Cast<UFont>(NewFont);
	boldFont() = UObject::Cast<UFont>(NewFont);
}

void UGC::SetFonts(UObject* newNormalFont, UObject* newBoldFont)
{
	normalFont() = UObject::Cast<UFont>(newNormalFont);
	boldFont() = UObject::Cast<UFont>(newBoldFont);
}

void UGC::SetHorizontalAlignment(uint8_t newHAlign)
{
	HAlign() = newHAlign;
}

void UGC::SetNormalFont(UObject* newNormalFont)
{
	normalFont() = UObject::Cast<UFont>(newNormalFont);
}

void UGC::SetStyle(uint8_t NewStyle)
{
	Style() = NewStyle;
}

void UGC::SetTextColor(const Color& newTextColor)
{
	TextColor() = newTextColor;
}

void UGC::SetTextVSpacing(float newVSpacing)
{
	textVSpacing() = newVSpacing;
}

void UGC::SetTileColor(const Color& newTileColor)
{
	tileColor() = newTileColor;
}

void UGC::SetVerticalAlignment(uint8_t newVAlign)
{
	VAlign() = newVAlign;
}
