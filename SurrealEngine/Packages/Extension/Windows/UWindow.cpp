
#include "Precomp.h"
#include "UWindow.h"
#include "UGC.h"
#include "TabGroup/URootWindow.h"
#include "Audio/AudioDevice.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"
#include "Package/PackageManager.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/Properties/UFloatProperty.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Extension/UPlayerPawnExt.h"

void UWindow::UpdateLayout()
{
	if (!bIsVisible())
		return;

	UWindow* parent = parentOwner();
	if (parent)
	{
		EHAlign halign = (EHAlign)winHAlign();
		EVAlign valign = (EVAlign)winVAlign();
		float leftMargin = hMargin0();
		float rightMargin = hMargin1();
		float topMargin = vMargin0();
		float bottomMargin = vMargin1();
		float pWidth = parent->Width();
		float pHeight = parent->Height();

		if (halign == EHAlign::Full || valign == EVAlign::Full)
		{
			float newX = X();
			float newY = Y();
			float newWidth = Width();
			float newHeight = Height();

			if (halign == EHAlign::Full)
			{
				newX = 0.0f;
				if (parent == engine->dxRootWindow)
					newWidth = std::max(GetExtendedVirtualWidth() - leftMargin - rightMargin, 0.0f);
				else
					newWidth = std::max(pWidth - leftMargin - rightMargin, 0.0f);
			}

			if (valign == EVAlign::Full)
			{
				newY = 0.0f;
				newHeight = std::max(pHeight - topMargin - bottomMargin, 0.0f);
			}

			X() = newX;
			Y() = newY;

			if (Width() != newWidth || Height() != newHeight)
			{
				Width() = newWidth;
				Height() = newHeight;
				bNeedsReconfigure() = true;
			}
		}

		float width = Width();
		float height = Height();
		float offsetX = 0.0f;

		if (parent == engine->dxRootWindow)
		{
			float extraWidth = std::max(GetExtendedVirtualWidth() - pWidth, 0.0f);
			offsetX = -std::round(extraWidth * 0.5f);
			pWidth += extraWidth;
		}

		float x = 0.0f, y = 0.0f;
		if (halign == EHAlign::Left || halign == EHAlign::Full)
			x = X() + leftMargin;
		else if (halign == EHAlign::Center)
			x = (pWidth - width) * 0.5f;
		else if (halign == EHAlign::Right)
			x = pWidth - rightMargin - width - X();

		if (valign == EVAlign::Top || valign == EVAlign::Full)
			y = Y() + topMargin;
		else if (valign == EVAlign::Center)
			y = (pHeight - height) * 0.5f;
		else if (valign == EVAlign::Bottom)
			y = pHeight - bottomMargin - height - Y();

		x += offsetX;

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
		UsedX = std::round((engine->viewport->ViewportWidth() - virtualWidth * virtualScale) * 0.5f) / virtualScale;
		UsedY = 0.0f;
	}

	bool wasReconfigured = bNeedsReconfigure();
	if (bNeedsReconfigure())
	{
		//LogMessage(GetUClassFullName(this).ToString() + ": UpdateLayout");

		bNeedsReconfigure() = false;
		ConfigurationChanged();
	}

	// Configure the windows that never got a ConfigureChild call:
	for (UWindow* child = firstChild(); child; child = child->nextSibling())
	{
		if ((wasReconfigured || child->FirstDraw) && !child->bConfigured())
		{
			float newWidth = child->hardcodedWidth();
			float newHeight = child->hardcodedHeight();
			if (!child->FixedWidth && !child->FixedHeight)
				child->QueryPreferredSize(newWidth, newHeight);
			else if (child->FixedWidth)
				newHeight = child->QueryPreferredHeight(child->FixedWidth);
			else if (child->FixedHeight)
				newWidth = child->QueryPreferredWidth(child->FixedHeight);
			child->Width() = newWidth;
			child->Height() = newHeight;
			child->bNeedsReconfigure() = true;
		}
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

float UWindow::GetVirtualWidth()
{
	// Force a 4:3 ratio for the root window as ConWindowActive.CalculateWindowSizes depends on it
	return std::round(GetVirtualHeight() * 4 / 3);
}

float UWindow::GetExtendedVirtualWidth()
{
	// Extend up to 16:9 instead of staying in the 4:3 box of the root window
	float scale = GetVirtualScale();
	float realWidth = std::ceil(engine->viewport->ViewportWidth() / scale);
	float maxWidth = GetVirtualHeight() * (16.0f / 9.0f);
	return std::min(realWidth, maxWidth);
}

float UWindow::GetVirtualHeight()
{
	float scale = GetVirtualScale();
	return std::ceil(engine->viewport->ViewportHeight() / scale);
}

float UWindow::GetVirtualScale()
{
	// Assume it was originally designed for 800x600. Find the closest clean integer scale factor.
	return std::max(std::round(engine->viewport->ViewportHeight() / 600.0f), 1.0f);
}

void UWindow::AddActorRef(UObject* refActor)
{
	UPlayerPawnExt* playerPawn = UObject::Cast<UPlayerPawnExt>(GetPlayerPawn());
	if (!playerPawn) return;
	UActor* target = UObject::Cast<UActor>(refActor);
	if (!target) return;

	int& count = playerPawn->actorCount();
	auto refs = playerPawn->actorList();

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

int UWindow::AddTimer(float TimeOut, std::optional<bool> bLoop, std::optional<int> clientData, std::optional<NameString> functionName)
{
	if (!functionName) // Script always specifies a function name
	{
		return -1;
	}

	int id = NextTimerId++;
	WTimer info;
	info.ClientData = clientData ? *clientData : 0;
	info.FunctionName = *functionName;
	info.Timeout = TimeOut;
	info.Loop = bLoop ? *bLoop : false;
	info.TimeLeft = info.Timeout;
	ActiveTimers[id] = info;
	return id;
}

void UWindow::RemoveTimer(int timerId)
{
	auto it = ActiveTimers.find(timerId);
	if (it != ActiveTimers.end())
		ActiveTimers.erase(it);
}

void UWindow::AskParentToShowArea(std::optional<float> areaX, std::optional<float> areaY, std::optional<float> areaWidth, std::optional<float> areaHeight)
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
	return oldStr;
}

bool UWindow::ConvertVectorToCoordinates(const vec3& Location, float& relativeX, float& relativeY)
{
	// Convert to view space
	vec4 viewSpaceLocation = engine->render->MainFrame.Frame.WorldToView * vec4(Location, 1.0f);
	if (viewSpaceLocation.z < 1.0f)
		return false;

	// Perform perspective projection
	vec4 projLocation = engine->render->MainFrame.Frame.Projection * viewSpaceLocation;
	float rcpW = 1.0f / projLocation.w;
	projLocation.x *= rcpW;
	projLocation.y *= rcpW;
	projLocation.z *= rcpW;

	// Scale to viewport
	vec2 viewportLocation = vec2(
		(projLocation.x + 1.0f) * engine->viewport->ViewportWidth() * 0.5f,
		(projLocation.y + 1.0f) * engine->viewport->ViewportHeight() * 0.5f);

	// Convert to virtual coordinates
	vec2 rootLocation = vec2(
		viewportLocation.x * GetVirtualWidth() / engine->viewport->ViewportWidth(),
		viewportLocation.y * GetVirtualHeight() / engine->viewport->ViewportHeight());

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
		for (UWindow* ancestor = parent; ancestor; ancestor = ancestor->parentOwner())
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

void UWindow::EnableSpecialText(std::optional<bool> bEnable)
{
	bSpecialText() = !bEnable || *bEnable;
}

void UWindow::EnableTranslucentText(std::optional<bool> bEnable)
{
	bTextTranslucent() = !bEnable || *bEnable;
}

void UWindow::EnableWindow(std::optional<bool> bEnable)
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

UObject* UWindow::GetBottomChild(std::optional<bool> bVisibleOnly)
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

UObject* UWindow::GetHigherSibling(std::optional<bool> bVisibleOnly)
{
	bool visibleOnly = (bVisibleOnly && *bVisibleOnly);
	for (UWindow* cur = nextSibling(); cur; cur = cur->nextSibling())
	{
		if (!visibleOnly || cur->bIsVisible())
			return cur;
	}
	return nullptr;
}

UObject* UWindow::GetLowerSibling(std::optional<bool> bVisibleOnly)
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

UObject* UWindow::GetTopChild(std::optional<bool> bVisibleOnly)
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
	auto refs = playerPawn->actorList();
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

bool UWindow::IsSensitive(std::optional<bool> bRecurse)
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

bool UWindow::IsVisible(std::optional<bool> bRecurse)
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

/*
UObject* UWindow::MoveFocus(EMove direction)
{
	auto root = GetRootWindow();
	if(root)
	{
		int hmref = root->handleMouseRef();
		if (!hmref)
			MoveTabGroup(dir);
		else {
			UTabGroupWindow* tabWindow = UObject::Cast<UTabGroupWindow*>(GetTabGroupWindow());
			if (tabWindow)
			{
				bool wrapFocus = bWrapFocus();
			}
		}
	}
}
*/

UObject* UWindow::MoveFocusDown()
{
	//MoveFocus(EMove::Down);
	return nullptr;
}

UObject* UWindow::MoveFocusLeft()
{
	//MoveFocus(EMove::Left);
	return nullptr;
}

UObject* UWindow::MoveFocusRight()
{
	//MoveFocus(EMove::Right);
	return nullptr;
}

UObject* UWindow::MoveFocusUp()
{
	//MoveFocus(EMove::Up);
	return nullptr;
}

/*UObject* UWindow::MoveTabGroup(EMove dir)
{
	// tbd
}*/

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

UObject* UWindow::NewChild(UObject* NewClass, std::optional<bool> bShow)
{
	bool show = !bShow || *bShow;
	//LogMessage(GetUClassFullName(this).ToString() + ": NewChild(" + NewClass->Name.ToString() + ", " + (show ? "true" : "false") + ")");
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

void UWindow::PlaySound(UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch, std::optional<float> posX, std::optional<float> posY)
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
		engine->audiodev->PlaySound(player, id, s, location, Volume ? *Volume : 1.0f, player->WorldSoundRadius(), Pitch ? *Pitch : 1.0f, false);
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
	auto refs = playerPawn->actorList();

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
	bDrawRawBackground() = true;
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

void UWindow::SetBaselineData(std::optional<float> newBaselineOffset, std::optional<float> newUnderlineHeight)
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

void UWindow::SetDefaultCursor(UObject* tX, std::optional<UObject*> shadowTexture, std::optional<float> HotX, std::optional<float> HotY, std::optional<Color> cursorColor)
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

void UWindow::SetFocusSounds(std::optional<UObject*> newFocusSound, std::optional<UObject*> newUnfocusSound)
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

void UWindow::SetVisibilitySounds(std::optional<UObject*> visSound, std::optional<UObject*> invisSound)
{
	if (visSound)
		visibleSound() = UObject::Cast<USound>(*visSound);
	if (invisSound)
		invisibleSound() = UObject::Cast<USound>(*invisSound);
}

void UWindow::Show(std::optional<bool> bShow)
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
	bNeedsReconfigure() = true;
}

void UWindow::ResetHeight()
{
	// Not called from script
	FixedHeight = false;
	bNeedsReconfigure() = true;
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
	bNeedsReconfigure() = true;
}

void UWindow::SetHeight(float NewHeight)
{
	hardcodedHeight() = NewHeight;
	FixedHeight = true;
	bNeedsReconfigure() = true;
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
	if (!FixedWidth || !FixedHeight)
	{
		if (Background())
		{
			preferredWidth = (float)Background()->USize();
			preferredHeight = (float)Background()->VSize();
		}
		ParentRequestedPreferredSize(false, preferredWidth, false, preferredHeight);
	}

	if (FixedWidth)
		preferredWidth = hardcodedWidth();

	if (FixedHeight)
		preferredHeight = hardcodedHeight();

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
		if (Background())
			width = (float)Background()->USize();
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
		if (Background())
			height = (float)Background()->VSize();
		ParentRequestedPreferredSize(true, queryWidth, false, height);
	}

	lastQueryWidth() = height;
	return height;
}

void UWindow::AskParentForReconfigure()
{
	//LogMessage(GetUClassFullName(this).ToString() + ": AskParentForReconfigure");
	UWindow* parent = parentOwner();
	if (parent)
	{
		bool result = parent->ChildRequestedReconfiguration(this);
		// To do: what is the result used for?
	}
}

void UWindow::ResizeChild()
{
	//LogMessage(GetUClassFullName(this).ToString() + ": ResizeChild");
	bNeedsReconfigure() = true;
}

void UWindow::ConfigureChild(float newX, float newY, float newWidth, float newHeight)
{
	//LogMessage(GetUClassFullName(this).ToString() + ": ConfigureChild(" + std::to_string(newX) + ", " + std::to_string(newY) + ", " + std::to_string(newWidth) + ", " + std::to_string(newHeight) + ")");

	if (FixedWidth)
		newWidth = hardcodedWidth();
	if (FixedHeight)
		newHeight = hardcodedHeight();

	if (UWindow* owner = parentOwner())
	{
		if ((EHAlign)winHAlign() == EHAlign::Full)
		{
			float leftMargin = hMargin0();
			float rightMargin = hMargin1();
			if (owner == engine->dxRootWindow)
			{
				newX = 0.0f;
				newWidth = std::max(GetExtendedVirtualWidth() - leftMargin - rightMargin, 0.0f);
			}
			else
			{
				newX = 0.0f;
				newWidth = std::max(owner->Width() - leftMargin - rightMargin, 0.0f);
			}
		}
		if ((EVAlign)winVAlign() == EVAlign::Full)
		{
			float topMargin = vMargin0();
			float bottomMargin = vMargin1();
			newY = 0.0f;
			newHeight = std::max(owner->Height() - topMargin - bottomMargin, 0.0f);
		}
	}

	X() = newX;
	Y() = newY;
	bConfigured() = true;
	//if (Width() != newWidth || Height() != newHeight)
	{
		Width() = newWidth;
		Height() = newHeight;
		bNeedsReconfigure() = true;
	}
}

void UWindow::SetWindowAlignments(uint8_t HAlign, uint8_t VAlign, std::optional<float> newHMargin0, std::optional<float> newVMargin0, std::optional<float> newHMargin1, std::optional<float> newVMargin1)
{
	winHAlign() = HAlign;
	winVAlign() = VAlign;

	if (newHMargin0)
		hMargin0() = *newHMargin0;
	if (newVMargin0)
		vMargin0() = *newVMargin0;

	if (newHMargin1)
		hMargin1() = *newHMargin1;
	else if (newHMargin0)
		hMargin1() = *newHMargin0;

	if (newVMargin1)
		vMargin1() = *newVMargin1;
	else if (newVMargin0)
		vMargin1() = *newVMargin0;
}

void UWindow::InitWindow()
{
	//LogMessage(GetUClassFullName(this).ToString() + ": InitWindow");

	TextColor() = { 255, 255, 255, 255 };
	//textPlane() = vec4(1.0f);
	tileColor() = { 255, 255, 255, 255 };
	//tilePlane() = vec4(1.0f);
	backgroundStyle() = (uint8_t)EDrawStyle::Translucent;

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
		ExpressionValue::BoolValue(bWidthSpecified),
		ExpressionValue::Variable(&preferredWidth, engine->floatprop),
		ExpressionValue::BoolValue(bHeightSpecified),
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
	if (bDrawRawBackground())
	{
		gc->SetStyle((EDrawStyle)backgroundStyle());
		gc->EnableSmoothing(bSmoothBackground());
		gc->SetTileColor(tileColor());
		if (bStretchBackground())
		{
			if (UTexture* tex = UObject::Cast<UTexture>(Background()))
			{
				float swidth = (float)tex->USize();
				float sheight = (float)tex->VSize();
				Rectf dest = Rectf::xywh(gc->offsetX, gc->offsetY, Width(), Height());
				Rectf src = Rectf::xywh(0.0f, 0.0f, swidth, sheight);
				gc->DrawTile(tex, gc->ScaleRect(dest), src, tileColor(), gc->EffectivePolyFlags());
			}
		}
		else
		{
			gc->DrawIcon(0.0f, 0.0f, Background());
		}
		// DrawDebugBox(gc);
	}

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

	std::vector<int> expiredList;
	for (auto& it : ActiveTimers)
	{
		it.second.TimeLeft -= timeElapsed;
		if (it.second.TimeLeft <= 0.0f)
			expiredList.push_back(it.first);
	}

	for (int timerId : expiredList)
	{
		// Keep calling the timer if its looping until we've called it the number of times total time elapsed
		while (true)
		{
			auto it = ActiveTimers.find(timerId);
			if (it == ActiveTimers.end())
				break; // script removed it via Window.RemoveTimer(id)

			NameString functionName = it->second.FunctionName;
			int clientData = it->second.ClientData;
			bool loop = it->second.Loop;

			if (loop)
			{
				it->second.TimeLeft += it->second.Timeout;
				if (it->second.TimeLeft > 0.0f || it->second.Timeout <= 0.00001f)
					loop = false;
			}
			else
			{
				// Single fire. Remove it.
				ActiveTimers.erase(it);
			}

			CallEvent(this, functionName, { ExpressionValue::IntValue(clientData) });

			if (!loop)
				break;
		}
	}

	for (auto cur = firstChild(); cur; cur = cur->nextSibling())
	{
		cur->Tick(timeElapsed);
	}
}

void UWindow::DrawDebugBox(UGC* gc)
{
	Rectf dest = gc->ScaleRect(Rectf::xywh(gc->offsetX, gc->offsetY, Width(), Height()));
	engine->render->Draw2DLine(vec4(1.0f), 0, vec3(dest.left, dest.top, 1.0f), vec3(dest.right, dest.top, 1.0f), false);
	engine->render->Draw2DLine(vec4(1.0f), 0, vec3(dest.left, dest.bottom, 1.0f), vec3(dest.right, dest.bottom, 1.0f), false);
	engine->render->Draw2DLine(vec4(1.0f), 0, vec3(dest.left, dest.top, 1.0f), vec3(dest.left, dest.bottom, 1.0f), false);
	engine->render->Draw2DLine(vec4(1.0f), 0, vec3(dest.right, dest.top, 1.0f), vec3(dest.right, dest.bottom, 1.0f), false);
}
