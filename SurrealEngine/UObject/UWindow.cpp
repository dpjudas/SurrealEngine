
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
#include "Render/RenderSubsystem.h"
#include "Package/PackageManager.h"

void UWindow::UpdateLayout()
{
	if (!bIsVisible())
		return;

	UWindow* parent = parentOwner();
	if (parent)
	{
		if (!bConfigured())
			ResizeChild();

		EHAlign halign = (EHAlign)winHAlign();
		EVAlign valign = (EVAlign)winVAlign();
		float leftMargin = hMargin0();
		float rightMargin = hMargin1();
		float topMargin = vMargin0();
		float bottomMargin = vMargin1();

		float pWidth = parent->Width();
		float pHeight = parent->Height();
		float width = Width();
		float height = Height();

		float x = 0.0f, y = 0.0f;
		if (halign == EHAlign::Left || halign == EHAlign::Full)
			x = X();
		else if (halign == EHAlign::Center)
			x = (pWidth - width) * 0.5f + X();
		else if (halign == EHAlign::Right)
			x = pWidth - width - X();

		if (valign == EVAlign::Top || valign == EVAlign::Full)
			y = Y();
		else if (valign == EVAlign::Center)
			y = (pHeight - height) * 0.5f + Y();
		else if (valign == EVAlign::Bottom)
			y = pHeight - height - Y();

		UsedX = x;
		UsedY = y;
	}
	else
	{
		float virtualWidth = GetVirtualWidth();
		float virtualHeight = GetVirtualHeight();
		float virtualScale = GetVirtualScale();
		if (Width() != virtualWidth || Height() != virtualHeight)
		{
			ConfigureChild(0.0f, 0.0f, virtualWidth, virtualHeight);
		}

		// Center the virtual viewbox
		UsedX = std::round((engine->ViewportWidth - virtualWidth * virtualScale) * 0.5f) / virtualScale;
		UsedY = 0.0f;
	}

	for (UWindow* child = firstChild(); child; child = child->nextSibling())
	{
		child->UpdateLayout();
	}

	if (FirstDraw)
	{
		FirstDraw = false;
		WindowReady();
	}
}

float UWindow::GetVirtualScale()
{
	return engine->ViewportHeight != 0 ? engine->ViewportHeight / GetVirtualHeight() : 1.0f;
}

void UWindow::AddActorRef(UObject* refActor)
{
	UPlayerPawnExt* playerPawn = UObject::Cast<UPlayerPawnExt>(GetPlayerPawn());
	if (!playerPawn) return;
	UActor* target = UObject::Cast<UActor>(refActor);  
	if (!target) return;

	int& count = playerPawn->actorCount();
	ActorRef* refs = playerPawn->actorList();

	int foundIndex = -1;
	for (int i = 0; i < count; ++i)
	{
		UActor* actor = refs[i].Actor;
		if (actor && actor->bDeleteMe()) actor = nullptr;
		if (actor == target) { foundIndex = i; break; }
	}

	if (foundIndex >= 0)
	{
		++refs[foundIndex].RefCount;
		return;
	}

	if (count >= 32)
	{
		for (int i = 1; i < count; ++i)
			refs[i - 1] = refs[i];
		--count;
	}

	refs[count].Actor = target;
	refs[count].RefCount = 1;
	++count;
}

int UWindow::AddTimer(float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName)
{
	LogUnimplemented("Window.AddTimer");
	return 0;
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
		parent->ChildRequestedShowArea(this, showX, showY, showWidth, showHeight);
	}
}

std::string UWindow::CarriageReturn()
{
	return "\r\n"; // Can't be doing this, can it?
}

static void ApplyStyleChange(UWindow* window)
{
	CallEvent(window, "StyleChanged", {});
	for (auto cur = window->firstChild(); cur; cur = cur->nextSibling())
		ApplyStyleChange(cur);
}

void UWindow::ChangeStyle()
{
	UWindow* cur = this;
	while (cur->parentOwner())
		cur = cur->parentOwner();
	ApplyStyleChange(cur);
}

void UWindow::ConvertCoordinates(UWindow* fromWin, float fromX, float fromY, UWindow* toWin, float& toX, float& toY)
{
	// Convert to global coordinates:
	float x = fromX;
	float y = fromY;
	for (UWindow* cur = fromWin; cur->parentOwner(); cur = cur->parentOwner())
	{
		x += cur->UsedX;
		y += cur->UsedY;
	}
	// Convert to local coordinates:
	for (UWindow* cur = toWin; cur->parentOwner(); cur = cur->parentOwner())
	{
		x -= cur->UsedX;
		y -= cur->UsedY;
	}
	toX = x;
	toY = y;
}

std::string UWindow::ConvertScriptString(const std::string& oldStr)
{
	LogUnimplemented("Window.ConvertScriptString");
	return oldStr;
}

bool UWindow::ConvertVectorToCoordinates(const vec3& Location, float& relativeX, float& relativeY)
{
	// Convert to view space
	vec4 viewSpaceLocation = engine->render->MainFrame.Frame.WorldToView * vec4(Location, 1.0f);
	if (viewSpaceLocation.z < 1.0f) // To do: should this be < 1.0 or > -1.0? (OpenGL vs Vulkan/D3D - what did we use?)
		return false;

	// Perform perspective projection
	vec4 projLocation = engine->render->MainFrame.Frame.Projection * viewSpaceLocation;
	float rcpW = 1.0f / projLocation.w;
	projLocation.x *= rcpW;
	projLocation.y *= rcpW;
	projLocation.z *= rcpW;

	// Scale to viewport
	vec2 viewportLocation = vec2(
		(projLocation.x + 1.0f) * engine->ViewportWidth * 0.5f,
		(1.0f - projLocation.y) * engine->ViewportHeight * 0.5f);

	// Convert to virtual coordinates
	vec2 rootLocation = vec2(
		viewportLocation.x * GetVirtualWidth() / engine->ViewportWidth,
		viewportLocation.y * GetVirtualHeight() / engine->ViewportHeight);

	// Convert from root window to our window
	UWindow* root = GetRootWindow();
	if (!root)
		return false;
	ConvertCoordinates(root, rootLocation.x, rootLocation.y, this, relativeX, relativeY);

	// Return true if the point is still inside the window
	return relativeX >= 0.0f && relativeX < Width() && relativeY >= 0.0f && relativeY < Height();
}

void UWindow::Destroy()
{
	UWindow* parent = parentOwner();
	DetachFromParent();
	if (parent)
	{
		parent->ChildRemoved(this);
		for (UWindow* ancestor = parent->parentOwner(); ancestor; ancestor = ancestor->parentOwner())
			ancestor->DescendantRemoved(this);
	}
	DestroyWindow();
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
	SetSensitivity(false);
}

void UWindow::EnableSpecialText(BitfieldBool* bEnable)
{
	bSpecialText() = !bEnable || *bEnable;
}

void UWindow::EnableTranslucentText(BitfieldBool* bEnable)
{
	bTextTranslucent() = !bEnable || *bEnable;
}

void UWindow::EnableWindow(BitfieldBool* bEnable)
{
	SetSensitivity(bEnable ? *bEnable : true);
}

UWindow* UWindow::FindWindow(float pointX, float pointY, float& relativeX, float& relativeY)
{
	// Convert to global coordinates and find root:
	float x = pointX;
	float y = pointY;
	UWindow* cur = this;
	while (cur->parentOwner())
	{
		x += cur->UsedX;
		y += cur->UsedY;
		cur = cur->parentOwner();
	}

	// Search the tree
	while (true)
	{
		UWindow* foundChild = nullptr;
		for (UWindow* child = cur->lastChild(); child; child = child->prevSibling())
		{
			if (child->bIsVisible() &&
				child->UsedX <= x &&
				child->UsedY <= y &&
				child->UsedX + child->Width() > x &&
				child->UsedY + child->Height() > y)
			{
				foundChild = child;
				break;
			}
		}
		if (!foundChild)
			break;
		cur = foundChild;
		x -= cur->UsedX;
		y -= cur->UsedY;
	}

	relativeX = x;
	relativeY = y;
	return cur;
}

UObject* UWindow::GetBottomChild(BitfieldBool* bVisibleOnly)
{
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* child = firstChild(); child; child = child->nextSibling())
	{
		if (!visibleOnly || child->bIsVisible())
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
	if (URootWindow* root = GetRootWindow())
	{
		ConvertCoordinates(root, root->MouseX(), root->MouseY(), this, MouseX, MouseY);
	}
	else
	{
		MouseX = 0.0f;
		MouseY = 0.0f;
	}
}

UObject* UWindow::GetFocusWindow()
{
	if (URootWindow* root = GetRootWindow())
	{
		return root->FocusWindow();
	}
	else
	{
		return nullptr;
	}
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
		if (!visibleOnly || cur->bIsVisible())
			return cur;
	}
	return nullptr;
}

UObject* UWindow::GetLowerSibling(BitfieldBool* bVisibleOnly)
{
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* cur = prevSibling(); cur; cur = cur->prevSibling())
	{
		if (!visibleOnly || cur->bIsVisible())
			return cur;
	}
	return nullptr;
}

UObject* UWindow::GetModalWindow()
{
	UWindow* cur = this;
	do
	{
		if (UObject::TryCast<UModalWindow>(cur))
			return cur;
		cur = cur->parentOwner();
	} while (cur);
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

URootWindow* UWindow::GetRootWindow()
{
	UWindow* cur = this;
	while (cur->parentOwner())
		cur = cur->parentOwner();
	return UObject::TryCast<URootWindow>(cur);
}

UObject* UWindow::GetTabGroupWindow()
{
	// Not called directly by script
	LogUnimplemented("Window.GetTabGroupWindow");
	return nullptr;
}

float UWindow::GetTickOffset()
{
	// Not called directly by script
	LogUnimplemented("Window.GetTickOffset");
	return 0.0f;
}

UObject* UWindow::GetTopChild(BitfieldBool* bVisibleOnly)
{
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* child = lastChild(); child; child = child->prevSibling())
	{
		if (!visibleOnly || child->bIsVisible())
			return child;
	}
	return nullptr;
}

void UWindow::GrabMouse()
{
	if (URootWindow* root = GetRootWindow())
		root->grabbedWindow() = this;
}

void UWindow::Hide()
{
	if (bIsVisible())
	{
		bIsVisible() = false;
		VisibilityChanged(false);
	}
}

bool UWindow::IsActorValid(UObject* refActor)
{
	UPlayerPawnExt* playerPawn = UObject::Cast<UPlayerPawnExt>(GetPlayerPawn());
	if (!playerPawn) return false;
	UActor* target = UObject::Cast<UActor>(refActor);
	if (!target) return false;
	ActorRef* refs = playerPawn->actorList();
	for (int i = 0, count = playerPawn->actorCount(); i < count; i++)
	{
		if (refs[i].Actor == refActor)
			return true;
	}
	return false;
}

bool UWindow::IsFocusWindow()
{
	if (auto root = GetRootWindow())
		return root->FocusWindow() == this;
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
	return pointX >= 0.0f && pointX < Width() && pointY >= 0.0f && pointY < Height();
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
	auto child = UObject::Cast<UWindow>(engine->packages->GetTransientPackage()->NewObject(NewClass->Name.ToString(), UObject::Cast<UClass>(NewClass), ObjectFlags::Transient));
	child->parentOwner() = this;
	child->prevSibling() = lastChild();
	child->nextSibling() = nullptr;
	child->firstChild() = nullptr;
	child->lastChild() = nullptr;
	child->bIsVisible() = show;
	child->bIsSensitive() = true;
	if (lastChild())
		lastChild()->nextSibling() = child;
	lastChild() = child;
	if (!firstChild())
		firstChild() = child;
	child->InitWindow();
	ChildAdded(child);
	for (UWindow* ancestor = parentOwner(); ancestor; ancestor = ancestor->parentOwner())
		ancestor->DescendantAdded(child);
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
	ParentRequestedGranularity(hGranularity, vGranularity);
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
	UPlayerPawnExt* playerPawn = UObject::Cast<UPlayerPawnExt>(GetPlayerPawn());
	if (!playerPawn) return;
	UActor* target = UObject::Cast<UActor>(refActor);
	if (!target) return;

	int& count = playerPawn->actorCount();
	ActorRef* refs = playerPawn->actorList();

	int foundIndex = -1;
	for (int i = 0; i < count; ++i)
	{
		UActor* actor = refs[i].Actor;
		if (actor && actor->bDeleteMe()) actor = nullptr;
		if (actor == target) { foundIndex = i; break; }
	}

	if (foundIndex >= 0)
	{
		--refs[foundIndex].RefCount;
		if (refs[foundIndex].RefCount == 0)
		{
			for (int i = foundIndex + 1; i < count; i++)
				refs[i - 1] = refs[i];
			refs[count - 1].Actor = nullptr;
			refs[count - 1].RefCount = 0;
			count--;
		}
	}
}

void UWindow::RemoveTimer(int timerId)
{
	LogUnimplemented("Window.RemoveTimer");
}

void UWindow::SetAcceleratorText(const std::string& newStr)
{
	char accelerator = 0;
	size_t pos = newStr.find("|&");
	if (pos != std::string::npos && pos + 2 < newStr.size())
	{
		accelerator = newStr[pos + 2];
	}
	int previousKey = acceleratorKey();
	int newKey = static_cast<int>(accelerator);
	acceleratorKey() = newKey;
	if (previousKey != newKey)
	{
		UModalWindow* modal = UObject::Cast<UModalWindow>(GetModalWindow());
		modal->bDirtyAccelerators() = true;
	}
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
	if (newBaselineOffset)
		BaselineOffset = *newBaselineOffset;
	if (newUnderlineHeight)
		UnderlineHeight = *newUnderlineHeight;
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

void UWindow::SetCursorPos(float newMouseX, float newMouseY)
{
	if (URootWindow* root = GetRootWindow())
	{
		float x = 0.0f, y = 0.0f;
		ConvertCoordinates(this, newMouseX, newMouseY, root, x, y);
		root->SetRootCursorPos(x, y);
	}
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
	if (auto root = GetRootWindow())
		return root->SetRootFocusWindow(UObject::Cast<UWindow>(NewFocusWindow));
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

void UWindow::SetNormalFont(UObject* fn)
{
	normalFont() = UObject::Cast<UFont>(fn);
}

void UWindow::SetSelectability(bool newSelectability)
{
	bIsSelectable() = newSelectability;
}

void UWindow::SetSensitivity(bool newSensitivity)
{
	if (bIsSensitive() != newSensitivity)
	{
		bIsSensitive() = newSensitivity;
		SensitivityChanged(newSensitivity);
	}
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

void UWindow::Show(BitfieldBool* bShow)
{
	bool show = !bShow || *bShow;
	if (bIsVisible() != show)
	{
		bIsVisible() = show;
		VisibilityChanged(show);
	}
}

void UWindow::UngrabMouse()
{
	if (URootWindow* root = GetRootWindow())
		root->grabbedWindow() = nullptr;
}

void UWindow::ResetSize()
{
	// Not called from script
	ResetWidth();
	ResetHeight();
}

void UWindow::ResetWidth()
{
	// Not called from script
	FixedWidth = false;
	bConfigured() = false;
}

void UWindow::ResetHeight()
{
	// Not called from script
	FixedHeight = false;
	bConfigured() = false;
}

void UWindow::SetSize(float newWidth, float NewHeight)
{
	SetWidth(newWidth);
	SetHeight(NewHeight);
}

void UWindow::SetWidth(float newWidth)
{
	hardcodedWidth() = newWidth;
	FixedWidth = true;
	bConfigured() = false;
}

void UWindow::SetHeight(float NewHeight)
{
	hardcodedHeight() = NewHeight;
	FixedHeight = true;
	bConfigured() = false;
}

void UWindow::SetConfiguration(float newX, float newY, float newWidth, float NewHeight)
{
	SetPos(newX, newY);
	SetSize(newWidth, NewHeight);
}

void UWindow::SetPos(float newX, float newY)
{
	X() = newX;
	Y() = newY;
}

void UWindow::QueryPreferredSize(float& preferredWidth, float& preferredHeight)
{
	bool widthSet = false;
	bool heightSet = false;

	if (FixedWidth)
	{
		preferredWidth = hardcodedWidth();
		widthSet = true;
	}

	if (FixedHeight)
	{
		preferredHeight = hardcodedHeight();
		heightSet = true;
	}

	if (!widthSet || !heightSet)
	{
		ParentRequestedPreferredSize(widthSet, preferredWidth, heightSet, preferredHeight);
	}

	lastQueryWidth() = preferredWidth;
	lastQueryHeight() = preferredHeight;
}

float UWindow::QueryPreferredWidth(float queryHeight)
{
	float width = 0.0f;

	if (FixedWidth)
	{
		width = hardcodedWidth();
	}
	else
	{
		ParentRequestedPreferredSize(false, width, true, queryHeight);
	}

	lastQueryWidth() = width;
	return width;
}

float UWindow::QueryPreferredHeight(float queryWidth)
{
	float height = 0.0f;

	if (FixedHeight)
	{
		height = hardcodedHeight();
	}
	else
	{
		ParentRequestedPreferredSize(true, queryWidth, false, height);
	}

	lastQueryWidth() = height;
	return height;
}

void UWindow::AskParentForReconfigure()
{
	UWindow* parent = parentOwner();
	if (parent)
	{
		parent->ChildRequestedReconfiguration(this);
	}
}

void UWindow::ResizeChild()
{
	float width = 0.0f, height = 0.0f;
	QueryPreferredSize(width, height);
	ConfigureChild(X(), Y(), width, height);
}

void UWindow::ConfigureChild(float newX, float newY, float newWidth, float newHeight)
{
	if (UWindow* owner = parentOwner())
	{
		if ((EHAlign)winHAlign() == EHAlign::Full)
			newWidth = owner->Width();
		if ((EVAlign)winVAlign() == EVAlign::Full)
			newHeight = owner->Height();
	}

	if (!bConfigured() || X() != newX || Y() != newY || Width() != newWidth || Height() != newHeight)
	{
		X() = newX;
		Y() = newY;
		Width() = newWidth;
		Height() = newHeight;
		bConfigured() = true;
		for (UWindow* child = firstChild(); child; child = child->nextSibling())
			child->bConfigured() = false;
		ConfigurationChanged();
	}
}

void UWindow::SetWindowAlignments(uint8_t HAlign, uint8_t VAlign, float* newHMargin0, float* newVMargin0, float* newHMargin1, float* newVMargin1)
{
	winHAlign() = HAlign;
	winVAlign() = VAlign;
	if (newHMargin0)
		hMargin0() = *newHMargin0;
	if (newVMargin0)
		vMargin0() = *newVMargin0;
	if (newHMargin1)
		hMargin1() = *newHMargin1;
	if (newVMargin1)
		vMargin1() = *newVMargin1;
}

void UWindow::InitWindow()
{
	CallEvent(this, "InitWindow");
}

void UWindow::DestroyWindow()
{
	CallEvent(this, "DestroyWindow");
}

void UWindow::WindowReady()
{
	CallEvent(this, "WindowReady");
}

void UWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	CallEvent(this, "ParentRequestedPreferredSize", {
		ExpressionValue::BoolValue(true),
		ExpressionValue::Variable(&preferredWidth, engine->floatprop),
		ExpressionValue::BoolValue(true),
		ExpressionValue::Variable(&preferredHeight, engine->floatprop)
		});
}

void UWindow::ParentRequestedGranularity(float& hGranularity, float& vGranularity)
{
	CallEvent(this, "ParentRequestedGranularity", {
		ExpressionValue::Variable(&hGranularity, engine->floatprop),
		ExpressionValue::Variable(&vGranularity, engine->floatprop)
		});
}

void UWindow::ChildRequestedVisibilityChange(UWindow* childWin, bool bNewVisibility)
{
	CallEvent(this, "ChildRequestedVisibilityChange", {
		ExpressionValue::ObjectValue(childWin),
		ExpressionValue::BoolValue(bNewVisibility)
		});
}

bool UWindow::ChildRequestedReconfiguration(UWindow* childWin)
{
	return CallEvent(this, "ChildRequestedReconfiguration", { ExpressionValue::ObjectValue(childWin) }).ToBool();
}

void UWindow::ChildRequestedShowArea(UWindow* child, float showX, float showY, float showWidth, float showHeight)
{
	CallEvent(this, "ChildRequestedShowArea", {
		ExpressionValue::ObjectValue(child),
		ExpressionValue::FloatValue(showX),
		ExpressionValue::FloatValue(showY),
		ExpressionValue::FloatValue(showWidth),
		ExpressionValue::FloatValue(showHeight)
		});
}

void UWindow::ConfigurationChanged()
{
	CallEvent(this, "ConfigurationChanged");
}

void UWindow::VisibilityChanged(bool bNewVisibility)
{
	CallEvent(this, "VisibilityChanged", { ExpressionValue::BoolValue(bNewVisibility) });
}

void UWindow::SensitivityChanged(bool bNewSensitivity)
{
	CallEvent(this, "SensitivityChanged", { ExpressionValue::BoolValue(bNewSensitivity) });
}

void UWindow::MouseMoved(float newX, float newY)
{
	CallEvent(this, "MouseMoved", {
		ExpressionValue::FloatValue(newX),
		ExpressionValue::FloatValue(newY)
		});
}

bool UWindow::RawMouseButtonPressed(float pointX, float pointY, EInputKey button, EInputType iState)
{
	return CallEvent(this, "RawMouseButtonPressed", {
		ExpressionValue::FloatValue(pointX),
		ExpressionValue::FloatValue(pointY),
		ExpressionValue::ByteValue(button),
		ExpressionValue::ByteValue(iState)
		}).ToBool();
}

bool UWindow::RawKeyPressed(EInputKey key, EInputType iState, bool bRepeat)
{
	return CallEvent(this, "RawKeyPressed", {
		ExpressionValue::ByteValue(key),
		ExpressionValue::ByteValue(iState),
		ExpressionValue::BoolValue(bRepeat)
		}).ToBool();
}

bool UWindow::MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks)
{
	return CallEvent(this, "MouseButtonPressed", {
		ExpressionValue::FloatValue(pointX),
		ExpressionValue::FloatValue(pointY),
		ExpressionValue::ByteValue(button),
		ExpressionValue::IntValue(numClicks)
		}).ToBool();
}

bool UWindow::MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks)
{
	return CallEvent(this, "MouseButtonReleased", {
		ExpressionValue::FloatValue(pointX),
		ExpressionValue::FloatValue(pointY),
		ExpressionValue::ByteValue(button),
		ExpressionValue::IntValue(numClicks)
		}).ToBool();
}

bool UWindow::KeyPressed(std::string key)
{
	return CallEvent(this, "KeyPressed", { ExpressionValue::StringValue(key) }).ToBool();
}

bool UWindow::AcceleratorKeyPressed(std::string key)
{
	return CallEvent(this, "AcceleratorKeyPressed", { ExpressionValue::StringValue(key) }).ToBool();
}

bool UWindow::VirtualKeyPressed(EInputKey key, bool bRepeat)
{
	return CallEvent(this, "VirtualKeyPressed", {
		ExpressionValue::ByteValue(key),
		ExpressionValue::BoolValue(bRepeat)
		}).ToBool();
}

void UWindow::MouseEnteredWindow()
{
	CallEvent(this, "MouseEnteredWindow");
}

void UWindow::MouseLeftWindow()
{
	CallEvent(this, "MouseLeftWindow");
}

void UWindow::FocusEnteredWindow()
{
	CallEvent(this, "FocusEnteredWindow");
}

void UWindow::FocusLeftWindow()
{
	CallEvent(this, "FocusLeftWindow");
}

void UWindow::FocusEnteredDescendant(UWindow* enterWindow)
{
	CallEvent(this, "FocusEnteredDescendant", { ExpressionValue::ObjectValue(enterWindow) });
}

void UWindow::FocusLeftDescendant(UWindow* leaveWindow)
{
	CallEvent(this, "FocusLeftDescendant", { ExpressionValue::ObjectValue(leaveWindow) });
}

bool UWindow::ButtonActivated(UWindow* button)
{
	return CallEvent(this, "ButtonActivated", { ExpressionValue::ObjectValue(button) }).ToBool();
}

bool UWindow::ToggleChanged(UWindow* button, bool bNewToggle)
{
	return CallEvent(this, "ToggleChanged", {
		ExpressionValue::ObjectValue(button),
		ExpressionValue::BoolValue(bNewToggle)
		}).ToBool();
}

bool UWindow::BoxOptionSelected(UWindow* box, int buttonNumber)
{
	return CallEvent(this, "BoxOptionSelected", {
		ExpressionValue::ObjectValue(box),
		ExpressionValue::IntValue(buttonNumber)
		}).ToBool();
}

bool UWindow::ScalePositionChanged(UWindow* scale, int newTickPosition, float newValue, bool bFinal)
{
	return CallEvent(this, "ScalePositionChanged", {
		ExpressionValue::ObjectValue(scale),
		ExpressionValue::IntValue(newTickPosition),
		ExpressionValue::FloatValue(newValue),
		ExpressionValue::BoolValue(bFinal)
		}).ToBool();
}

bool UWindow::ScaleRangeChanged(UWindow* scale, int fromTick, int toTick, float fromValue, float toValue, bool bFinal)
{
	return CallEvent(this, "ScaleRangeChanged", {
		ExpressionValue::ObjectValue(scale),
		ExpressionValue::IntValue(fromTick),
		ExpressionValue::IntValue(toTick),
		ExpressionValue::FloatValue(fromValue),
		ExpressionValue::FloatValue(toValue),
		ExpressionValue::BoolValue(bFinal)
		}).ToBool();
}

bool UWindow::ScaleAttributesChanged(UWindow* scale, int tickPosition, int tickSpan, int numTicks)
{
	return CallEvent(this, "ScaleRangeChanged", {
		ExpressionValue::ObjectValue(scale),
		ExpressionValue::IntValue(tickPosition),
		ExpressionValue::IntValue(tickSpan),
		ExpressionValue::IntValue(numTicks)
		}).ToBool();
}

bool UWindow::ClipAttributesChanged(UWindow* scale, int newClipWidth, int newClipHeight, int newChildWidth, int newChildHeight)
{
	return CallEvent(this, "ClipAttributesChanged", {
		ExpressionValue::ObjectValue(scale),
		ExpressionValue::IntValue(newClipWidth),
		ExpressionValue::IntValue(newClipHeight),
		ExpressionValue::IntValue(newChildWidth),
		ExpressionValue::IntValue(newChildHeight)
		}).ToBool();
}

bool UWindow::ListRowActivated(UWindow* list, int rowId)
{
	return CallEvent(this, "ListRowActivated", {
		ExpressionValue::ObjectValue(list),
		ExpressionValue::IntValue(rowId)
		}).ToBool();
}

bool UWindow::ListSelectionChanged(UWindow* list, int numSelections, int focusRowId)
{
	return CallEvent(this, "ListSelectionChanged", {
		ExpressionValue::ObjectValue(list),
		ExpressionValue::IntValue(numSelections),
		ExpressionValue::IntValue(focusRowId)
		}).ToBool();
}

bool UWindow::TextChanged(UWindow* edit, bool bModified)
{
	return CallEvent(this, "TextChanged", {
		ExpressionValue::ObjectValue(edit),
		ExpressionValue::BoolValue(bModified)
		}).ToBool();
}

bool UWindow::EditActivated(UWindow* edit, bool bModified)
{
	return CallEvent(this, "EditActivated", {
		ExpressionValue::ObjectValue(edit),
		ExpressionValue::BoolValue(bModified)
		}).ToBool();
}

void UWindow::DrawWindow(UGC* gc)
{
	CallEvent(this, "DrawWindow", { ExpressionValue::ObjectValue(gc) });
}

void UWindow::PostDrawWindow(UGC* gc)
{
	CallEvent(this, "PostDrawWindow", { ExpressionValue::ObjectValue(gc) });
}

void UWindow::ChildAdded(UWindow* child)
{
	CallEvent(this, "ChildAdded", { ExpressionValue::ObjectValue(child) });
}

void UWindow::ChildRemoved(UWindow* child)
{
	CallEvent(this, "ChildRemoved", { ExpressionValue::ObjectValue(child) });
}

void UWindow::DescendantAdded(UWindow* descendant)
{
	CallEvent(this, "DescendantAdded", { ExpressionValue::ObjectValue(descendant) });
}

void UWindow::DescendantRemoved(UWindow* descendant)
{
	CallEvent(this, "DescendantRemoved", { ExpressionValue::ObjectValue(descendant) });
}

void UWindow::Tick(float timeElapsed)
{
	if (bTickEnabled())
		CallEvent(this, "Tick", { ExpressionValue::FloatValue(timeElapsed) });
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		cur->Tick(timeElapsed);
	}
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

void UTileWindow::SetOrder(EOrder newOrder)
{
	bWrap() = true;
	uint8_t horizontal = (uint8_t)EOrientation::Horizontal;
	uint8_t vertical = (uint8_t)EOrientation::Vertical;
	uint8_t leftToRight = (uint8_t)EHDirection::LeftToRight;
	uint8_t rightToLeft = (uint8_t)EHDirection::RightToLeft;
	uint8_t topToBottom = (uint8_t)EVDirection::TopToBottom;
	uint8_t bottomToTop = (uint8_t)EVDirection::BottomToTop;
	switch (newOrder)
	{
		//Offsetreminder:orientation,hdirection,vdirection,bwrap
		case EOrder::Right:
			orientation() = horizontal;
			hDirection() = leftToRight;
			vDirection() = topToBottom;
			bWrap() = false;
			break;
		case EOrder::Left:
			orientation() = horizontal;
			hDirection() = rightToLeft;
			vDirection() = topToBottom;
			bWrap() = false;
			break;
		case EOrder::Down:
			orientation() = vertical;
			hDirection() = leftToRight;
			vDirection() = topToBottom;
			bWrap() = false;
			break;
		case EOrder::Up:
			orientation() = vertical;
			hDirection() = leftToRight;
			vDirection() = topToBottom;
			bWrap() = false;
			break;
		case EOrder::RightThenDown:
			orientation() = horizontal;
			hDirection() = leftToRight;
			vDirection() = topToBottom;
			break;
		case EOrder::RightThenUp:
			orientation() = horizontal;
			hDirection() = leftToRight;
			vDirection() = bottomToTop;
			break;
		case EOrder::LeftThenDown:
			orientation() = horizontal;
			hDirection() = rightToLeft;
			vDirection() = topToBottom;
			break;
		case EOrder::LeftThenUp:
			orientation() = horizontal;
			hDirection() = rightToLeft;
			vDirection() = bottomToTop;
			break;
		case EOrder::DownThenRight:
			orientation() = vertical;
			hDirection() = leftToRight;
			vDirection() = topToBottom;
			break;
		case EOrder::DownThenLeft:
			orientation() = vertical;
			hDirection() = rightToLeft;
			vDirection() = topToBottom;
			break;
		case EOrder::UpThenRight:
			orientation() = vertical;
			hDirection() = leftToRight;
			vDirection() = bottomToTop;
			break;
		case EOrder::UpThenLeft:
			orientation() = vertical;
			hDirection() = rightToLeft;
			vDirection() = topToBottom;
			break;
	}
	AskParentForReconfigure();
}

void UTileWindow::SetOrientation(uint8_t newOrientation)
{
	orientation() = newOrientation;
}

void UTileWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	EOrientation orient = (EOrientation)orientation();
	EHDirection hdir = (EHDirection)hDirection();
	EVDirection vdir = (EVDirection)vDirection();
	bool wrap = bWrap();

	// To do: implement more of this

	if (orient == EOrientation::Horizontal)
	{
		preferredWidth = 0.0f;
		preferredHeight = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = 0.0f, h = 0.0f;
			cur->QueryPreferredSize(w, h);
			preferredHeight = std::max(preferredHeight, h);
			preferredWidth += h;
		}
	}
	else
	{
		preferredWidth = 0.0f;
		preferredHeight = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = 0.0f, h = 0.0f;
			cur->QueryPreferredSize(w, h);
			preferredWidth = std::max(preferredWidth, w);
			preferredHeight += h;
		}
	}
}

void UTileWindow::ConfigurationChanged()
{
	EOrientation orient = (EOrientation)orientation();
	EHDirection hdir = (EHDirection)hDirection();
	EVDirection vdir = (EVDirection)vDirection();
	bool wrap = bWrap();

	// To do: implement more of this

	if (orient == EOrientation::Horizontal)
	{
		float x = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = cur->QueryPreferredWidth(Height());
			float h = cur->QueryPreferredHeight(w);
			cur->ConfigureChild(x, 0.0f, w, std::min(h, Height()));
			x += w;
		}
	}
	else
	{
		float y = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float h = cur->QueryPreferredHeight(Width());
			float w = cur->QueryPreferredWidth(h);
			cur->ConfigureChild(0.0f, y, std::min(w, Width()), h);
			y += h;
		}
	}

	UWindow::ConfigurationChanged();
}

/////////////////////////////////////////////////////////////////////////////

void UTextWindow::AppendText(const std::string& NewText)
{
	Text() += NewText;
}

void UTextWindow::EnableTextAsAccelerator(BitfieldBool* bEnable)
{
	std::string accelText = bEnable ? Text() : "";
	SetAcceleratorText(accelText.length() > 0 ? accelText : "");
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

	if (normalFont()) // How does TextWindow get its intrinsic size!?
	{
		float xExtent = 0.0f, yExtent = 0.0f;
		engine->dxgc->SetFont(normalFont());
		engine->dxgc->GetTextExtent(100000.0f, xExtent, yExtent, Text());
		SetSize(xExtent, yExtent);
	}
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

void UTextWindow::InitWindow()
{
	SetFont(engine->canvas->SmallFont());
	UWindow::InitWindow();
}

void UTextWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	if (!Text().empty()) // Is this needed?
	{
		float xExtent = 0.0f, yExtent = 0.0f;
		engine->dxgc->GetTextExtent(bWidthSpecified ? preferredWidth : 100000.0f, xExtent, yExtent, Text());
		if (!bWidthSpecified)
			preferredWidth = xExtent;
		if (!bHeightSpecified)
			preferredHeight = yExtent;
	}

	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UTextWindow::DrawWindow(UGC* gc)
{
	if (normalFont()) // When should text windows draw their text? They are used for buttons, which sometimes draw themselves via UI
	{
		gc->DrawText(0.0f, 0.0f, Width(), Height(), Text());
	}
	UWindow::DrawWindow(gc);
}

/////////////////////////////////////////////////////////////////////////////

void UButtonWindow::ActivateButton(EInputKey key)
{
	for (UWindow* cur = this; cur; cur = cur->parentOwner())
	{
		if (cur->ButtonActivated(this))
			break;
	}
}

void UButtonWindow::EnableAutoRepeat(BitfieldBool* bEnable, float* newInitialDelay, float* newRepeatRate)
{
	if (bEnable)
		bAutoRepeat() = *bEnable;
	if (newInitialDelay)
		initialDelay() = *newInitialDelay;
	if (newRepeatRate)
		repeatRate() = *newRepeatRate;

	repeatTime() = 0.0f; // To do: this field is probably used to figure out when its time to repeat
}

void UButtonWindow::EnableRightMouseClick(BitfieldBool* bEnable)
{
	bEnableRightMouseClick() = !bEnable || *bEnable;
}

void UButtonWindow::PressButton(uint8_t* Key)
{
	// How does this differ from activate? It animates first maybe? use activateTimer() for that?
	ActivateButton(Key ? (EInputKey)(*Key) : IK_LeftMouse);
}

void UButtonWindow::SetActivateDelay(float* newDelay)
{
	if (newDelay)
		activateDelay() = *newDelay;
}

void UButtonWindow::SetButtonColors(Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	if (Normal)
		ButtonColors.Normal = *Normal;
	if (pressed)
		ButtonColors.Pressed = *pressed;
	if (normalFocus)
		ButtonColors.NormalFocus = *normalFocus;
	if (pressedFocus)
		ButtonColors.PressedFocus = *pressedFocus;
	if (normalInsensitive)
		ButtonColors.NormalInsensitive = *normalInsensitive;
	if (pressedInsensitive)
		ButtonColors.PressedInsensitive = *pressedInsensitive;
}

void UButtonWindow::SetButtonSounds(UObject** newPressSound, UObject** newClickSound)
{
	if (newPressSound)
		pressSound() = UObject::Cast<USound>(*newPressSound);
	if (newClickSound)
		clickSound() = UObject::Cast<USound>(*newClickSound);
}

void UButtonWindow::SetButtonTextures(UObject** Normal, UObject** pressed, UObject** normalFocus, UObject** pressedFocus, UObject** normalInsensitive, UObject** pressedInsensitive)
{
	if (Normal)
		ButtonTextures.Normal = UObject::Cast<UTexture>(*Normal);
	if (pressed)
		ButtonTextures.Pressed = UObject::Cast<UTexture>(*pressed);
	if (normalFocus)
		ButtonTextures.NormalFocus = UObject::Cast<UTexture>(*normalFocus);
	if (pressedFocus)
		ButtonTextures.PressedFocus = UObject::Cast<UTexture>(*pressedFocus);
	if (normalInsensitive)
		ButtonTextures.NormalInsensitive = UObject::Cast<UTexture>(*normalInsensitive);
	if (pressedInsensitive)
		ButtonTextures.PressedInsensitive = UObject::Cast<UTexture>(*pressedInsensitive);
}

void UButtonWindow::SetTextColors(Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive)
{
	if (Normal)
		TextColors.Normal = *Normal;
	if (pressed)
		TextColors.Pressed = *pressed;
	if (normalFocus)
		TextColors.NormalFocus = *normalFocus;
	if (pressedFocus)
		TextColors.PressedFocus = *pressedFocus;
	if (normalInsensitive)
		TextColors.NormalInsensitive = *normalInsensitive;
	if (pressedInsensitive)
		TextColors.PressedInsensitive = *pressedInsensitive;
}

void UButtonWindow::MouseMoved(float newX, float newY)
{
	if (bMousePressed())
	{
		bButtonPressed() = newX >= 0.0f && newX < Width() && newY >= 0.0f && newY < Height();
	}
	else
	{
		UTextWindow::MouseMoved(newX, newY);
	}
}

bool UButtonWindow::MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks)
{
	if (UTextWindow::MouseButtonPressed(pointX, pointY, button, numClicks))
		return true;

	bMousePressed() = true;
	bButtonPressed() = true;
	GrabMouse();
	return true;
}

bool UButtonWindow::MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks)
{
	if (bMousePressed())
	{
		UngrabMouse();
		bMousePressed() = false;
		bButtonPressed() = false;
		if (pointX >= 0.0f && pointX < Width() && pointY >= 0.0f && pointY < Height())
			ActivateButton(button);
		return true;
	}

	return UTextWindow::MouseButtonReleased(pointX, pointY, button, numClicks);
}

/////////////////////////////////////////////////////////////////////////////

void UToggleWindow::ChangeToggle()
{
	SetToggle(!GetToggle());
}

bool UToggleWindow::GetToggle()
{
	return isToggled;
}

void UToggleWindow::SetToggle(bool bNewToggle)
{
	if (isToggled != bNewToggle)
	{
		isToggled = bNewToggle;
		for (UWindow* cur = this; cur; cur = cur->parentOwner())
		{
			if (cur->ToggleChanged(this, bNewToggle))
				break;
		}
	}
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
	textTimeout() = newTimeout;
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

void URadioBoxWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	preferredWidth = 0.0f;
	preferredHeight = 0.0f;
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		float w = 0.0f, h = 0.0f;
		cur->QueryPreferredSize(w, h);
		preferredHeight = std::max(preferredHeight, h);
		preferredWidth += h;
	}
}

void URadioBoxWindow::ConfigurationChanged()
{
	if (bSizeChildrenToParent())
	{
		int childCount = 0;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			if (cur->bIsVisible())
				childCount++;
		}
		if (childCount == 0)
			return;

		float totalWidth = Width();
		float h = Height();
		float x = 0.0f;
		int idx = 0;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			if (cur->bIsVisible())
			{
				float w = totalWidth * idx / childCount;
				cur->ConfigureChild(x, 0.0f, w, h);
				x += w;
				idx++;
			}
		}
	}
	else if (!bSizeParentToChildren())
	{
		float x = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = 0.0f, h = 0.0f;
			cur->QueryPreferredSize(w, h);
			cur->ConfigureChild(x, 0.0f, w, h);
			x += w;
		}
	}

	UTabGroupWindow::ConfigurationChanged();
}

/////////////////////////////////////////////////////////////////////////////

void UClipWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	// To do: how does this work?
	UTabGroupWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UClipWindow::ConfigurationChanged()
{
	// To do: how does this work?
	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		cur->ConfigureChild(0.0f, 0.0f, Width(), Height());
	}

	UTabGroupWindow::ConfigurationChanged();
}

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
	// Not called by script
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
	RenderViewportSet = false;
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
	RenderViewportSet = true;
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

void URootWindow::WindowReady()
{
	SetRootCursorPos(GetVirtualWidth() * 0.5f, GetVirtualHeight() * 0.5f);
	UModalWindow::WindowReady();
}

bool URootWindow::IsCursorVisible()
{
	// To do: is this correct? There is also URootWindow::ShowCursor - it isn't called by unrealscript when a modal is shown
	for (UWindow* cur = firstChild(); cur; cur = cur->nextSibling())
	{
		if (UObject::TryCast<UModalWindow>(cur))
		{
			return true;
		}
	}
	return false;
}

void URootWindow::PostDrawWindow(UGC* gc)
{
	UModalWindow::PostDrawWindow(gc);
	if (IsCursorVisible())
	{
		// To do: how is current cursor picked?
		float relativeX = 0.0f, relativeY = 0.0f;
		if (UWindow* focus = this/*GetCursorFocus(relativeX, relativeY)*/)
		{
			if (UTexture* cursor = UObject::Cast<UTexture>(focus->defaultCursor()))
			{
				Color white = { 255, 255, 255, 255 };
				gc->SetStyle(EDrawStyle::Masked);
				gc->SetTileColor(white);
				float hotspotX = cursor->USize() * 0.5f;
				float hotspotY = cursor->VSize() * 0.5f;
				gc->DrawIcon(MouseX() - hotspotX, MouseY() - hotspotY, cursor);
			}
		}
	}
}

static UWindow* CommonAncestor(UWindow* a, UWindow* b)
{
	if (a == b)
		return a;

	std::vector<UWindow*> list1;
	std::vector<UWindow*> list2;
	list1.reserve(16);
	list2.reserve(16);
	for (UWindow* w = a; w != nullptr; w = w->parentOwner())
		list1.push_back(w);
	for (UWindow* w = b; w != nullptr; w = w->parentOwner())
		list2.push_back(w);

	if (list1.empty() || list2.empty() || list1.back() != list2.back())
		return nullptr;

	auto it1 = list1.rbegin();
	auto it2 = list2.rbegin();
	while (it1 != list1.rend() && it2 != list2.rend())
	{
		if (*it1 != *it2)
		{
			return *(--it1);
		}
		++it1;
		++it2;
	}

	if (it1 == list1.rend())
		return *(--it1);
	else if (it2 == list2.rend())
		return *(--it2);

	return nullptr;
}

bool URootWindow::SetRootFocusWindow(UWindow* newFocusWindow)
{
	UWindow* oldFocusWindow = FocusWindow();
	if (oldFocusWindow != newFocusWindow)
	{
		UWindow* ancestor = CommonAncestor(oldFocusWindow, newFocusWindow);
		if (oldFocusWindow)
		{
			if (oldFocusWindow->unfocusSound())
				PlaySound(oldFocusWindow->unfocusSound(), nullptr, nullptr, nullptr, nullptr);

			oldFocusWindow->FocusLeftWindow();
			for (UWindow* w = oldFocusWindow->parentOwner(); w != ancestor; w = w->parentOwner())
			{
				w->FocusLeftDescendant(oldFocusWindow);
			}
		}
		FocusWindow() = newFocusWindow;
		if (newFocusWindow)
		{
			// Note: this order is in reverse. Hopefully it doesn't matter.
			newFocusWindow->FocusEnteredWindow();
			for (UWindow* w = newFocusWindow->parentOwner(); w != ancestor; w = w->parentOwner())
			{
				w->FocusEnteredDescendant(newFocusWindow);
			}

			if (newFocusWindow->focusSound())
				PlaySound(newFocusWindow->focusSound(), nullptr, nullptr, nullptr, nullptr);
		}
	}
	return true;
}

void URootWindow::SetRootCursorPos(float newMouseX, float newMouseY)
{
	newMouseX = std::max(newMouseX, 0.0f);
	newMouseY = std::max(newMouseY, 0.0f);
	newMouseX = std::min(newMouseX, GetVirtualWidth());
	newMouseY = std::min(newMouseY, GetVirtualHeight());

	prevMouseX() = MouseX();
	prevMouseY() = MouseY();
	MouseX() = newMouseX;
	MouseY() = newMouseY;

	float relativeX = 0.0f, relativeY = 0.0f;
	UWindow* focus = GetCursorFocus(relativeX, relativeY);

	auto lastFocus = UObject::Cast<UWindow>(lastMouseWindow());
	if (lastFocus != focus)
	{
		if (lastFocus)
		{
			if (UWindow* ancestor = CommonAncestor(lastFocus, focus))
			{
				for (UWindow* w = lastFocus; w != ancestor; w = w->parentOwner())
				{
					w->MouseLeftWindow();
				}
			}
		}
		lastMouseWindow() = focus;
		focus->MouseEnteredWindow();
	}

	focus->MouseMoved(relativeX, relativeY);
}

UWindow* URootWindow::GetCursorFocus(float& relativeX, float& relativeY)
{
	if (UWindow* grab = grabbedWindow())
	{
		ConvertCoordinates(this, MouseX(), MouseY(), grab, relativeX, relativeY);
		return grab;
	}

	if (UWindow* cursor = FindWindow(MouseX(), MouseY(), relativeX, relativeY))
		return cursor;

	relativeX = MouseX();
	relativeY = MouseY();
	return this;
}

void URootWindow::OnWindowMouseMove(const Point& pos)
{
#if 0 // We currently handle this in OnWindowRawMouseMove
	if (IsCursorVisible())
		SetRootCursorPos((float)pos.x / scale, (float)pos.y / scale);
#endif
}

void URootWindow::OnWindowMouseDown(const Point& pos, EInputKey key)
{
	float relativeX = 0.0f, relativeY = 0.0f;
	UWindow* focus = GetCursorFocus(relativeX, relativeY);

	if (!focus->bIsSensitive())
		return;

	if (focus->RawMouseButtonPressed(relativeX, relativeY, key, EInputType::IST_Press))
		return;

	if (focus->bIsSelectable())
		SetRootFocusWindow(focus);

	int numClicks = 1; // What is this?
	if (focus->MouseButtonPressed(relativeX, relativeY, key, numClicks))
		return;
}

void URootWindow::OnWindowMouseDoubleclick(const Point& pos, EInputKey key)
{
	// Is this numClicks = 2?
}

void URootWindow::OnWindowMouseUp(const Point& pos, EInputKey key)
{
	float relativeX = 0.0f, relativeY = 0.0f;
	UWindow* focus = GetCursorFocus(relativeX, relativeY);

	if (focus->RawMouseButtonPressed(relativeX, relativeY, key, EInputType::IST_Release))
		return;

	if (!focus->bIsSensitive())
		return;

	int numClicks = 1; // What is this?
	if (focus->MouseButtonReleased(relativeX, relativeY, key, numClicks))
		return;
}

void URootWindow::OnWindowMouseWheel(const Point& pos, EInputKey key)
{
	// How does Deus handle the mouse wheel?
}

void URootWindow::OnWindowRawMouseMove(int dx, int dy)
{
	if (IsCursorVisible())
	{
		float mouseSpeed = 1.0f / GetVirtualScale();
		SetRootCursorPos(MouseX() + dx * mouseSpeed, MouseY() + dy * mouseSpeed);
	}
}

void URootWindow::OnWindowKeyChar(std::string chars)
{
	UWindow* focus = FocusWindow();
	if (!focus)
		return;

	if (focus->KeyPressed(chars))
		return;

	// To do: fire these for edit windows
	// event bool TextChanged(window edit, bool bModified)
	// event bool EditActivated(window edit, bool bModified)
}

void URootWindow::OnWindowKeyDown(EInputKey key)
{
	UWindow* focus = FocusWindow();
	if (!focus)
		return;

	// To do: this shouldn't just check the focus window. It needs to build an accelerator table for all windows
	if (engine->window->GetKeyState(IK_Alt) && focus->acceleratorKey() != 0)
	{
		std::string chars(1, (char)focus->acceleratorKey());
		EInputKey accelKey = (EInputKey)(uint8_t)chars.front();
		if (focus->AcceleratorKeyPressed(chars))
			return;
	}

	bool repeat = false; // To do: can zwidget tell us this?

	if (focus->RawKeyPressed(key, EInputType::IST_Press, repeat))
		return;

	if (focus->VirtualKeyPressed(key, repeat))
		return;
}

void URootWindow::OnWindowKeyUp(EInputKey key)
{
	UWindow* focus = FocusWindow();
	if (!focus)
		return;

	if (focus->RawKeyPressed(key, EInputType::IST_Release, false))
		return;

	// To do: fire these for specific window types
	// event bool ButtonActivated(Window button)
	// event bool ToggleChanged(Window button, bool bNewToggle)
	// event bool BoxOptionSelected(Window box, int buttonNumber)
	// event bool ListRowActivated(window list, int rowId)
	// event bool ListSelectionChanged(window list, int numSelections, int focusRowId)
}

/////////////////////////////////////////////////////////////////////////////

void UScrollAreaWindow::InitWindow()
{
	ClipWindow() = UObject::Cast<UClipWindow>(NewChild(engine->packages->FindClass("Extension.ClipWindow"), nullptr));
	hScale() = UObject::Cast<UScaleWindow>(NewChild(engine->packages->FindClass("Extension.ScaleWindow"), nullptr));
	vScale() = UObject::Cast<UScaleWindow>(NewChild(engine->packages->FindClass("Extension.ScaleWindow"), nullptr));
	hScaleMgr() = UObject::Cast<UScaleManagerWindow>(NewChild(engine->packages->FindClass("Extension.ScaleManagerWindow"), nullptr));
	vScaleMgr() = UObject::Cast<UScaleManagerWindow>(NewChild(engine->packages->FindClass("Extension.ScaleManagerWindow"), nullptr));
	DownButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow"), nullptr));
	LeftButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow"), nullptr));
	RightButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow"), nullptr));
	UpButton() = UObject::Cast<UButtonWindow>(NewChild(engine->packages->FindClass("Extension.ButtonWindow"), nullptr));

	ClipWindow()->bIsVisible() = true;
	hScale()->bIsVisible() = false;
	vScale()->bIsVisible() = false;
	hScaleMgr()->bIsVisible() = false;
	vScaleMgr()->bIsVisible() = false;
	DownButton()->bIsVisible() = false;
	LeftButton()->bIsVisible() = false;
	RightButton()->bIsVisible() = false;
	UpButton()->bIsVisible() = false;

	UWindow::InitWindow();
}

void UScrollAreaWindow::ConfigurationChanged()
{
	// To do: position the scrollbars (is that scalemgr or scale?), the scroll buttons and clip window properly
	// To do: how is the size of the clip window determined?

	ClipWindow()->ConfigureChild(0.0f, 0.0f, Width(), Height());
	UWindow::ConfigurationChanged();
}

void UScrollAreaWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	// To do: have to take the scrollbars into account

	if (!bWidthSpecified && !bHeightSpecified)
	{
		ClipWindow()->QueryPreferredSize(preferredWidth, preferredHeight);
	}
	else if (!bWidthSpecified)
	{
		ClipWindow()->QueryPreferredWidth(preferredHeight);
	}
	else // if (!bHeightSpecified)
	{
		ClipWindow()->QueryPreferredHeight(preferredWidth);
	}
}

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
	FontColor() = newFontColor;
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

void UGC::DrawText(float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr)
{
	if (!bDrawEnabled())
		return;

	UFont* font = normalFont();
	if (font)
	{
		float x = offsetX + DestX;
		float y = offsetY + DestY;
		uint32_t polyflags = EffectiveTextPolyFlags();

		auto valign = (EVAlign)VAlign();
		if (valign == EVAlign::Top)
		{
			DrawText(font, x, y, destWidth, textStr, TextColor(), polyflags);
		}
		else if (valign == EVAlign::Center || valign == EVAlign::Full)
		{
			Sizef extents = DrawText(font, x, y, destWidth, textStr, TextColor(), polyflags, true);
			DrawText(font, x, y + (destHeight - extents.height) * 0.5f, destWidth, textStr, TextColor(), polyflags);
		}
		else if (valign == EVAlign::Bottom)
		{
			Sizef extents = DrawText(font, x, y, destWidth, textStr, TextColor(), polyflags, true);
			DrawText(font, x, y + destHeight - extents.height, destWidth, textStr, TextColor(), polyflags);
		}
	}
}

void UGC::DrawBorders(float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject* borders, BitfieldBool* bStretchHorizontally, BitfieldBool* bStretchVertically)
{
	if (!bDrawEnabled())
		return;

	if (leftMargin != 0.0f || rightMargin != 0.0f || TopMargin != 0.0f || BottomMargin != 0.0f || bStretchHorizontally || bStretchVertically)
	{
		// margins are always zero from script. Was this supposed to be a CSS border-image style draw function?
		LogUnimplemented("GC.DrawBorders");
	}
	else
	{
		UTexture* tex = UObject::Cast<UTexture>(borders);
		if (tex)
		{
			float swidth = (float)tex->USize();
			float sheight = (float)tex->VSize();
			Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
			Rectf src = Rectf::xywh(0.0f, 0.0f, swidth, sheight);
			DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
		}
	}
}

void UGC::DrawBox(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf src = Rectf::xywh(0.0f, 0.0f, swidth, sheight);
		uint32_t polyflags = EffectivePolyFlags();
		Color color = tileColor();

		Rectf top = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, boxThickness);
		Rectf bottom = Rectf::xywh(offsetX + DestX, offsetY + DestY - boxThickness, destWidth, boxThickness);
		Rectf left = Rectf::xywh(offsetX + DestX, offsetY + DestY, boxThickness, destHeight);
		Rectf right = Rectf::xywh(offsetX + DestX - boxThickness, offsetY + DestY, boxThickness, destHeight);

		DrawTile(tex, ScaleRect(top), src, color, polyflags);
		DrawTile(tex, ScaleRect(bottom), src, color, polyflags);
		DrawTile(tex, ScaleRect(left), src, color, polyflags);
		DrawTile(tex, ScaleRect(right), src, color, polyflags);
	}
}

void UGC::DrawIcon(float DestX, float DestY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, swidth, sheight);
		Rectf clip = dest;
		Rectf src = Rectf::xywh(0.0f, 0.0f, swidth, sheight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawPattern(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
		Rectf clip = dest;
		Rectf src = Rectf::xywh(OrgX, OrgY, destWidth, destHeight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawStretchedTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
		Rectf clip = dest;
		Rectf src = Rectf::xywh(srcX, srcY, srcWidth, srcHeight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
		Rectf clip = dest;
		Rectf src = Rectf::xywh(srcX, srcY, destWidth, destHeight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

Rectf UGC::ScaleRect(const Rectf& box)
{
	float scale = UWindow::GetVirtualScale();
	return Rectf(box.left * scale, box.top * scale, box.right * scale, box.bottom * scale);
}

uint32_t UGC::EffectivePolyFlags()
{
	uint32_t polyflags = PolyFlags();
	if (bMasked())
		polyflags |= PF_Masked;
	if (bModulated())
		polyflags |= PF_Modulated;
	if (!bSmoothed())
		polyflags |= PF_NoSmooth;
	if (bTranslucent())
		polyflags |= PF_Translucent;
	return polyflags;
}

uint32_t UGC::EffectiveTextPolyFlags()
{
	uint32_t polyflags = textPolyFlags();
	if (bTextTranslucent())
		polyflags |= PF_Translucent;
	else
		polyflags |= PF_Masked | PF_NoSmooth;
	return polyflags;
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
	SpecialTextEnabled = bNewSpecialText;
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
	if (!normalFont())
		return 0.0f;
	FontGlyph glyph = normalFont()->GetGlyph('X');
	return (float)glyph.VSize;
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
		Sizef extents = DrawText(font, 0.0f, 0.0f, destWidth, textStr, TextColor(), 0, true);
		xExtent = extents.width;
		yExtent = extents.height;
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
	// Not called directly by script. Seems we only clip by window then?
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
	return SpecialTextEnabled;
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

void UGC::SetStyle(EDrawStyle NewStyle)
{
	Style() = (uint8_t)NewStyle;

	// Is this what it is doing? So stupid to have the same states 3 times!
	switch (NewStyle)
	{
	case EDrawStyle::None:
		bDrawEnabled() = false;
		PolyFlags() = 0;
		textPolyFlags() = 0;
		break;
	case EDrawStyle::Normal:
		PolyFlags() = 0;
		textPolyFlags() = 0;
		bDrawEnabled() = true;
		bMasked() = false;
		bTranslucent() = false;
		bModulated() = false;
		bTextTranslucent() = false;
		break;
	case EDrawStyle::Masked:
		PolyFlags() = 0;
		textPolyFlags() = 0;
		bDrawEnabled() = true;
		bMasked() = true;
		bTranslucent() = false;
		bModulated() = false;
		bTextTranslucent() = false;
		break;
	case EDrawStyle::Translucent:
		PolyFlags() = 0;
		textPolyFlags() = 0;
		bDrawEnabled() = true;
		bMasked() = false;
		bTranslucent() = true;
		bModulated() = false;
		bTextTranslucent() = true;
		break;
	case EDrawStyle::Modulated:
		PolyFlags() = 0;
		textPolyFlags() = PF_Modulated;
		bDrawEnabled() = true;
		bMasked() = false;
		bTranslucent() = false;
		bModulated() = true;
		bTextTranslucent() = false;
		break;
	}
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

void UGC::DrawTile(UTexture* tex, const Rectf& dest, const Rectf& src, const Color& c, uint32_t flags)
{
	vec4 color(c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, c.A / 255.0f);
	float Z = 1.0f;
	vec4 fog(0.0f);

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)tex;
	texinfo.Texture = tex;
	texinfo.Format = texinfo.Texture->UsedFormat;
	texinfo.Mips = tex->UsedMipmaps.data();
	texinfo.NumMips = (int)tex->UsedMipmaps.size();
	texinfo.USize = tex->USize();
	texinfo.VSize = tex->VSize();
	if (tex->Palette())
		texinfo.Palette = (FColor*)tex->Palette()->Colors.data();

	if (dest.left > dest.right || dest.top > dest.bottom)
		return;

	if (dest.left >= clipBox.left && dest.top >= clipBox.top && dest.right <= clipBox.right && dest.bottom <= clipBox.bottom)
	{
		engine->render->DrawTile(texinfo, dest.left, dest.top, dest.right - dest.left, dest.bottom - dest.top, src.left, src.top, src.right - src.left, src.bottom - src.top, Z, color, fog, flags);
	}
	else
	{
		Rectf d = dest;
		Rectf s = src;

		float scaleX = (s.right - s.left) / (d.right - d.left);
		float scaleY = (s.bottom - s.top) / (d.bottom - d.top);

		if (d.left < clipBox.left)
		{
			s.left += scaleX * (clipBox.left - d.left);
			d.left = clipBox.left;
		}
		if (d.right > clipBox.right)
		{
			s.right += scaleX * (clipBox.right - d.right);
			d.right = clipBox.right;
		}
		if (d.top < clipBox.top)
		{
			s.top += scaleY * (clipBox.top - d.top);
			d.top = clipBox.top;
		}
		if (d.bottom > clipBox.bottom)
		{
			s.bottom += scaleY * (clipBox.bottom - d.bottom);
			d.bottom = clipBox.bottom;
		}

		if (d.left < d.right && d.top < d.bottom)
			engine->render->DrawTile(texinfo, d.left, d.top, d.right - d.left, d.bottom - d.top, s.left, s.top, s.right - s.left, s.bottom - s.top, Z, color, fog, flags);
	}
}

Sizef UGC::DrawText(UFont* font, float orgX, float orgY, float destWidth, const std::string& textStr, const Color& color, uint32_t polyflags, bool noDraw)
{
	if (!bWordWrap())
		destWidth = 100000.0f;


	// Remove the | and & escapes for now
	/*
	std::string text;
	text.reserve(textStr.size());
	for (char c : textStr)
	{
		if (c != '|' && c != '&')
			text += c;
	}
	*/

	auto halign = (EHAlign)HAlign();

	float totalWidth = 0.0f;
	float totalHeight = 0.0f;
	float curX = 0.0f;
	float curY = 0.0f;

	Array<TextBlock> textBlocks = FindTextBlocks(textStr);
	size_t lineBegin = 0;
	float lineWidth = 0.0f;
	float lineHeight = 0.0f;

	for (size_t pos = 0; pos < textBlocks.size(); pos++)
	{
		if (textBlocks[pos].text.front() == '\n')
		{
			if (pos != lineBegin)
			{
				float centerX = 0;
				if (halign == EHAlign::Center || halign == EHAlign::Full)
					centerX = std::round((destWidth - lineWidth) * 0.5f);
				else if (halign == EHAlign::Right)
					centerX = destWidth - lineWidth;

				if (!noDraw)
					DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, pos, font, color, polyflags);

				curY += lineHeight;
				totalHeight += lineHeight;
				totalWidth = std::max(totalWidth, lineWidth);
			}

			curX = 0;
			lineBegin = pos + 1;
			lineWidth = 0.0f;
			lineHeight = 0.0f;
		}
		else
		{
			vec2 blockSize = GetTextSize(font, textBlocks[pos].text);
			if (lineWidth + blockSize.x > destWidth)
			{
				float centerX = 0;
				if (halign == EHAlign::Center || halign == EHAlign::Full)
					centerX = std::round((destWidth - lineWidth) * 0.5f);
				else if (halign == EHAlign::Right)
					centerX = destWidth - lineWidth;

				if (!noDraw)
					DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, pos, font, color, polyflags);

				curX = 0;
				curY += lineHeight;
				totalHeight += lineHeight;
				totalWidth = std::max(totalWidth, lineWidth);

				if (textBlocks[pos].text.front() == ' ')
				{
					// Ignore whitespace at the beginning of a word wrapped line
					lineBegin = pos + 1;
					lineWidth = 0.0f;
					lineHeight = 0.0f;
				}
				else
				{
					lineBegin = pos;
					lineWidth = blockSize.x;
					lineHeight = blockSize.y;
				}
			}
			else
			{
				lineWidth += blockSize.x;
				lineHeight = std::max(lineHeight, blockSize.y);
			}
		}
	}

	if (lineBegin < textBlocks.size())
	{
		float centerX = 0;
		if (halign == EHAlign::Center || halign == EHAlign::Full)
			centerX = std::round((destWidth - lineWidth) * 0.5f);
		else if (halign == EHAlign::Right)
			centerX = destWidth - lineWidth;

		if (!noDraw)
			DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, textBlocks.size(), font, color, polyflags);

		curX += centerX + lineWidth;
		curY += lineHeight;
		totalHeight += lineHeight;
		totalWidth = std::max(totalWidth, lineWidth);
	}

	return Sizef(totalWidth, totalHeight);
}

vec2 UGC::GetTextSize(UFont* font, const std::string& text)
{
	float x = 0.0f;
	float y = 0.0f;
	for (char c : text)
	{
		FontGlyph glyph = font->GetGlyph(c);
		x += (float)glyph.USize;
		y = std::max(y, (float)glyph.VSize);
	}
	return { x, y };
}

Array<TextBlock> UGC::FindTextBlocks(const std::string& text)
{
	// Split text into words, whitespace or newline
	Array<TextBlock> textBlocks;
	size_t pos = 0;

	while (pos < text.size())
	{
		if (text[pos] == '\r')
        {
            textBlocks.push_back({"\r", Color{255,255,255, 255}, 0});
            pos++;
        }
		if (text[pos] == '\n')
		{
			textBlocks.push_back({"\n", Color{255,255,255, 255}, 0});
			pos++;
		}
		else if (text[pos] == ' ')
		{
			// Arbitrary-length whitespace
			size_t end = std::min(text.find_first_not_of(' ', pos + 1), text.size());

			std::string whitespaceText = text.substr(pos, end - pos);

			textBlocks.push_back({whitespaceText, Color{255, 255, 255, 255}, 0});
			pos = end;
		}
		else
		{
			size_t end = std::min(text.find_first_of(" \n", pos + 1), text.size());

			std::string foundText = text.substr(pos, end - pos);
			Color textColor = {255, 255, 255, 255};
			size_t accelPos = 0;

			if (foundText.starts_with("|p"))
			{
				try
				{
					int pColorIdx = std::stoi(foundText.substr(2, 1));
					textColor = s_PColors[pColorIdx];
					foundText = foundText.substr(3); // Strip |p and num out
				}
				catch (...)
				{
					foundText = foundText.substr(2); // Strip |p out
				}
			}

			auto accelFind = foundText.find("|&");

			if (accelFind != std::string::npos)
			{
				accelPos = accelFind;
				foundText = foundText.erase(accelFind, 2);
			}

			TextBlock block = {
				.text = foundText,
				.textColor = textColor,
				.accelPos = accelPos
			};

			textBlocks.push_back(block);
			pos = end;
		}
	}

	return textBlocks;
}

void UGC::DrawTextBlockRange(float x, float y, const Array<TextBlock>& textBlocks, size_t start, size_t end, UFont* font, const Color& color, uint32_t polyflags)
{
	for (size_t i = start; i < end; i++)
	{
		for (char c : textBlocks[i].text)
		{
			FontGlyph glyph = font->GetGlyph(c);

			if (!glyph.Texture)
				continue;

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)glyph.Texture;
			texinfo.Texture = glyph.Texture;
			texinfo.Format = texinfo.Texture->UsedFormat;
			texinfo.Mips = glyph.Texture->UsedMipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->UsedMipmaps.size();
			texinfo.USize = glyph.Texture->USize();
			texinfo.VSize = glyph.Texture->VSize();
			if (glyph.Texture->Palette())
				texinfo.Palette = (FColor*)glyph.Texture->Palette()->Colors.data();

			Rectf dest = Rectf::xywh(x, y, (float)glyph.USize, (float)glyph.VSize);
			Rectf src = Rectf::xywh((float)glyph.StartU, (float)glyph.StartV, (float)glyph.USize, (float)glyph.VSize);

			DrawTile(glyph.Texture, ScaleRect(dest), src, textBlocks[i].textColor, polyflags);

			x += (float)glyph.USize;
		}
	}
}
