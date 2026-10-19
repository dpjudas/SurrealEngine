
#include "Precomp.h"
#include "UButtonWindow.h"
#include "Packages/Extension/Windows/UGC.h"
#include "Packages/Engine/Resources/USound.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

void UButtonWindow::DrawWindow(UGC* gc)
{
	bool hasFocus = GetFocusWindow() == this;
	bool pressed = bButtonPressed();
	bool enabled = bIsSensitive();

	ButtonState* state;
	if (!enabled && pressed)
		state = &PressedInsensitiveState;
	else if (!enabled)
		state = &NormalInsensitiveState;
	else if (hasFocus && pressed)
		state = &PressedFocusState;
	else if (hasFocus)
		state = &NormalFocusState;
	else
		state = &NormalState;

	if (state->Texture)
	{
		gc->SetTileColor(state->ButtonColor);
		gc->DrawStretchedTexture(
			0.0f, 0.0f, Width(), Height(),
			0.0f, 0.0f, (float)state->Texture->USize(), (float)state->Texture->VSize(),
			state->Texture);
	}

	if (normalFont())
	{
		float xMargin = hMargin();
		float yMargin = vMargin();
		float w = Width() - 2.0f * xMargin;
		float h = Height() - 2.0f * yMargin;
		if (w > 0.0f && h > 0.0f)
		{
			gc->SetTextColor(state->TextColor);
			gc->SetAlignments(HAlign(), VAlign());
			gc->DrawText(xMargin, yMargin, w, h, Text());
		}
		// DrawDebugBox(gc);
	}

	// Note: we are intentionally not calling UTextWindow::DrawWindow
	UWindow::DrawWindow(gc);
}

void UButtonWindow::ActivateButton(EInputKey key)
{
	for (UWindow* cur = this; cur; cur = cur->parentOwner())
	{
		if (cur->ButtonActivated(this))
			break;
	}
}

void UButtonWindow::EnableAutoRepeat(std::optional<bool> bEnable, std::optional<float> newInitialDelay, std::optional<float> newRepeatRate)
{
	if (bEnable)
		bAutoRepeat() = *bEnable;
	if (newInitialDelay)
		initialDelay() = *newInitialDelay;
	if (newRepeatRate)
		repeatRate() = *newRepeatRate;

	repeatTime() = 0.0f; // To do: this field is probably used to figure out when its time to repeat
}

void UButtonWindow::EnableRightMouseClick(std::optional<bool> bEnable)
{
	bEnableRightMouseClick() = !bEnable || *bEnable;
}

void UButtonWindow::PressButton(std::optional<uint8_t> Key)
{
	// How does this differ from activate? It animates first maybe? use activateTimer() for that?
	ActivateButton(Key ? (EInputKey)(*Key) : IK_LeftMouse);
}

void UButtonWindow::SetActivateDelay(std::optional<float> newDelay)
{
	if (newDelay)
		activateDelay() = *newDelay;
}

void UButtonWindow::SetButtonColors(std::optional<Color> Normal, std::optional<Color> pressed, std::optional<Color> normalFocus, std::optional<Color> pressedFocus, std::optional<Color> normalInsensitive, std::optional<Color> pressedInsensitive)
{
	if (Normal)
		NormalState.ButtonColor = *Normal;
	if (pressed)
		PressedState.ButtonColor = *pressed;
	if (normalFocus)
		NormalFocusState.ButtonColor = *normalFocus;
	if (pressedFocus)
		PressedFocusState.ButtonColor = *pressedFocus;
	if (normalInsensitive)
		NormalInsensitiveState.ButtonColor = *normalInsensitive;
	if (pressedInsensitive)
		PressedInsensitiveState.ButtonColor = *pressedInsensitive;
}

void UButtonWindow::SetButtonSounds(std::optional<UObject*> newPressSound, std::optional<UObject*> newClickSound)
{
	if (newPressSound)
		pressSound() = UObject::Cast<USound>(*newPressSound);
	if (newClickSound)
		clickSound() = UObject::Cast<USound>(*newClickSound);
}

void UButtonWindow::SetButtonTextures(std::optional<UObject*> Normal, std::optional<UObject*> pressed, std::optional<UObject*> normalFocus, std::optional<UObject*> pressedFocus, std::optional<UObject*> normalInsensitive, std::optional<UObject*> pressedInsensitive)
{
	if (Normal)
		NormalState.Texture = UObject::Cast<UTexture>(*Normal);
	if (pressed)
		PressedState.Texture = UObject::Cast<UTexture>(*pressed);
	if (normalFocus)
		NormalFocusState.Texture = UObject::Cast<UTexture>(*normalFocus);
	if (pressedFocus)
		PressedFocusState.Texture = UObject::Cast<UTexture>(*pressedFocus);
	if (normalInsensitive)
		NormalInsensitiveState.Texture = UObject::Cast<UTexture>(*normalInsensitive);
	if (pressedInsensitive)
		PressedInsensitiveState.Texture = UObject::Cast<UTexture>(*pressedInsensitive);
}

void UButtonWindow::SetTextColors(std::optional<Color> Normal, std::optional<Color> pressed, std::optional<Color> normalFocus, std::optional<Color> pressedFocus, std::optional<Color> normalInsensitive, std::optional<Color> pressedInsensitive)
{
	if (Normal)
		NormalState.TextColor = *Normal;
	if (pressed)
		PressedState.TextColor = *pressed;
	if (normalFocus)
		NormalFocusState.TextColor = *normalFocus;
	if (pressedFocus)
		PressedFocusState.TextColor = *pressedFocus;
	if (normalInsensitive)
		NormalInsensitiveState.TextColor = *normalInsensitive;
	if (pressedInsensitive)
		PressedInsensitiveState.TextColor = *pressedInsensitive;
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
	SetFocusWindow(this);

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

void UButtonWindow::MouseEnteredWindow()
{
	UTextWindow::MouseEnteredWindow();
	SetFocusWindow(this);
}
