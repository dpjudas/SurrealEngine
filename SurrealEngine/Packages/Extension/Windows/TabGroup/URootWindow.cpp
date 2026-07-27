
#include "Precomp.h"
#include "URootWindow.h"
#include "Utils/Logger.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Resources/USound.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"
#include "Packages/Extension/Windows/UGC.h"
#include "Engine.h"

void URootWindow::EnablePositionalSound(std::optional<bool> bEnable)
{
	bPositionalSound() = !bEnable || *bEnable;
}

void URootWindow::EnableRendering(std::optional<bool> newRender)
{
	bRender() = !newRender || *newRender;
}

UObject* URootWindow::GenerateSnapshot(std::optional<bool> bFilter)
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

void URootWindow::LockMouse(std::optional<bool> bLockMove, std::optional<bool> bLockButton)
{
	LogUnimplemented("RootWindow.LockMouse");
}

void URootWindow::SetDefaultEditCursor(std::optional<UObject*> newEditCursor)
{
	if (newEditCursor)
		defaultEditCursor() = UObject::Cast<UTexture>(*newEditCursor);
}

void URootWindow::SetDefaultMovementCursors(std::optional<UObject*> newMovementCursor, std::optional<UObject*> newHorizontalMovementCursor, std::optional<UObject*> newVerticalMovementCursor, std::optional<UObject*> newTopLeftMovementCursor, std::optional<UObject*> newTopRightMovementCursor)
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

void URootWindow::SetRawBackground(std::optional<UObject*> NewTexture, std::optional<Color> NewColor)
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

void URootWindow::ResetRenderViewport()
{
	RenderViewportSet = false;
}

void URootWindow::SetSnapshotSize(float newWidth, float NewHeight)
{
	LogUnimplemented("RootWindow.SetSnapshotSize");
}

void URootWindow::ShowCursor(std::optional<bool> bShow)
{
	bCursorVisible() = !bShow || *bShow;
}

void URootWindow::StretchRawBackground(std::optional<bool> bStretch)
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
		// Find the cursor based on where the mouse is hovering:
		UTexture* cursor = nullptr;
		float relativeX = 0.0f, relativeY = 0.0f;
		UWindow* focus = GetCursorFocus(relativeX, relativeY);
		if (!focus)
			focus = this;
		while (focus)
		{
			if (UTexture* tex = UObject::Cast<UTexture>(focus->defaultCursor()))
			{
				cursor = tex;
				break;
			}
			focus = focus->parentOwner();
		}

		// Draw the cursor if we found one
		if (cursor)
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
				PlaySound(oldFocusWindow->unfocusSound(), {}, {}, {}, {});

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
				PlaySound(newFocusWindow->focusSound(), {}, {}, {}, {});
		}
	}
	return true;
}

void URootWindow::SetRootCursorPos(float newMouseX, float newMouseY)
{
	// Clip cursor to the entire screen, not the root window box:

	newMouseX += UsedX;
	newMouseY += UsedY;

	float scale = GetVirtualScale();
	float realWidth = std::ceil(engine->viewport->ViewportWidth() / scale);
	float realHeight = std::ceil(engine->viewport->ViewportHeight() / scale);

	newMouseX = std::max(newMouseX, 0.0f);
	newMouseY = std::max(newMouseY, 0.0f);
	newMouseX = std::min(newMouseX, realWidth);
	newMouseY = std::min(newMouseY, realHeight);

	newMouseX -= UsedX;
	newMouseY -= UsedY;

	// Apply the new cursor pos:

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

bool URootWindow::OnWindowMouseMove(const Point& pos)
{
#if 0 // We currently handle this in OnWindowRawMouseMove
	if (IsCursorVisible())
		SetRootCursorPos((float)pos.x / scale, (float)pos.y / scale);
#endif
	return IsModalOpen();
}

bool URootWindow::OnWindowMouseDown(const Point& pos, EInputKey key)
{
	float relativeX = 0.0f, relativeY = 0.0f;
	UWindow* focus = GetCursorFocus(relativeX, relativeY);

	if (!focus->bIsSensitive())
		return IsModalOpen();

	if (focus->RawMouseButtonPressed(relativeX, relativeY, key, EInputType::IST_Press))
		return true;

	if (focus->bIsSelectable())
		SetRootFocusWindow(focus);

	int numClicks = 1; // What is this?
	for (UWindow* cur = focus; cur; cur = cur->parentOwner())
	{
		if (cur->MouseButtonPressed(relativeX, relativeY, key, numClicks))
			return true;
	}

	return IsModalOpen();
}

bool URootWindow::OnWindowMouseDoubleclick(const Point& pos, EInputKey key)
{
	// Is this numClicks = 2?
	return IsModalOpen();
}

bool URootWindow::OnWindowMouseUp(const Point& pos, EInputKey key)
{
	float relativeX = 0.0f, relativeY = 0.0f;
	UWindow* focus = GetCursorFocus(relativeX, relativeY);

	if (focus->RawMouseButtonPressed(relativeX, relativeY, key, EInputType::IST_Release))
		return true;

	if (!focus->bIsSensitive())
		return IsModalOpen();

	int numClicks = 1; // What is this?
	for (UWindow* cur = focus; cur; cur = cur->parentOwner())
	{
		if (cur->MouseButtonReleased(relativeX, relativeY, key, numClicks))
			return true;
	}

	return IsModalOpen();
}

bool URootWindow::OnWindowMouseWheel(const Point& pos, EInputKey key)
{
	if (!OnWindowMouseDown(pos, key))
		return false;

	OnWindowMouseUp(pos, key);
	return true;
}

bool URootWindow::OnWindowRawMouseMove(int dx, int dy)
{
	if (IsCursorVisible())
	{
		float mouseSpeed = 1.0f / GetVirtualScale();
		SetRootCursorPos(MouseX() + dx * mouseSpeed, MouseY() + dy * mouseSpeed);
	}
	return IsModalOpen();
}

bool URootWindow::OnWindowKeyChar(std::string chars)
{
	UWindow* focus = FocusWindow();
	if (!focus)
		return IsModalOpen();

	if (focus->KeyPressed(chars))
		return true;

	// To do: fire these for edit windows
	// event bool TextChanged(window edit, bool bModified)
	// event bool EditActivated(window edit, bool bModified)

	return IsModalOpen();
}

bool URootWindow::OnWindowKeyDown(EInputKey key)
{
	UWindow* focus = FocusWindow();
	if (!focus)
		return IsModalOpen();

	// To do: this shouldn't just check the focus window. It needs to build an accelerator table for all windows
	if (engine->window->GetKeyState(IK_Alt) && focus->acceleratorKey() != 0)
	{
		std::string chars(1, (char)focus->acceleratorKey());
		EInputKey accelKey = (EInputKey)(uint8_t)chars.front();
		if (focus->AcceleratorKeyPressed(chars))
			return true;
	}

	bool repeat = false; // To do: can surrealwidgets tell us this?

	if (focus->RawKeyPressed(key, EInputType::IST_Press, repeat))
		return true;

	if (focus->VirtualKeyPressed(key, repeat))
		return true;

	return IsModalOpen();
}

bool URootWindow::OnWindowKeyUp(EInputKey key)
{
	UWindow* focus = FocusWindow();
	if (!focus)
		return IsModalOpen();

	if (focus->RawKeyPressed(key, EInputType::IST_Release, false))
		return true;

	// To do: fire these for specific window types
	// event bool ButtonActivated(Window button)
	// event bool ToggleChanged(Window button, bool bNewToggle)
	// event bool BoxOptionSelected(Window box, int buttonNumber)
	// event bool ListRowActivated(window list, int rowId)
	// event bool ListSelectionChanged(window list, int numSelections, int focusRowId)

	return IsModalOpen();
}

bool URootWindow::IsModalOpen()
{
	for (UWindow* child = lastChild(); child; child = child->prevSibling())
	{
		if (UObject::TryCast<UModalWindow>(child))
			return true;
	}
	return false;
}
