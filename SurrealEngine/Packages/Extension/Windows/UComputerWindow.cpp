
#include "Precomp.h"
#include "UComputerWindow.h"
#include "Engine.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Extension/Windows/UGC.h"

void UComputerWindow::ClearLine(int rowToClear)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.ClearLine");
}

void UComputerWindow::FadeOutText(std::optional<float> fadeDuration)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.FadeOutText");
}

void UComputerWindow::GetChar(const std::string& inputKey, std::optional<bool> bEcho)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.GetChar");
}

void UComputerWindow::GetInput(int MaxLength, const std::string& inputKey, std::optional<std::string> defaultInputString, std::optional<std::string> inputMask)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.GetInput");
}

float UComputerWindow::GetThrottle()
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.GetThrottle");
	return 0.0f;
}

bool UComputerWindow::IsBufferFlushed()
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.IsBufferFlushed");
	return false;
}

bool UComputerWindow::IsPaused()
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.IsPaused");
	return false;
}

void UComputerWindow::Pause(std::optional<float> pauseLength)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.Pause");
}

void UComputerWindow::PlaySoundLater(UObject* newsound)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.PlaySoundLater");
}

void UComputerWindow::PrintGraphic(UObject* Graphic, int Width, int Height, std::optional<int> posX, std::optional<int> posY, std::optional<bool> bStatic, std::optional<bool> bPixelPos)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.PrintGraphic");
}

void UComputerWindow::PrintLn()
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.PrintLn");
}

void UComputerWindow::ResetThrottle()
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.ResetThrottle");
}

void UComputerWindow::Resume()
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.Resume");
}

void UComputerWindow::SetBackgroundTextures(UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetBackgroundTextures");
}

void UComputerWindow::SetComputerSoundVolume(float newSoundVolume)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetComputerSoundVolume");
}

void UComputerWindow::SetCursorBlinkSpeed(float newBlinkSpeed)
{
	// UNUSED from scripts.
	cursorBlinkSpeed() = newBlinkSpeed;
}

void UComputerWindow::SetCursorTexture(UObject* newCursorTexture, std::optional<int> newCursorWidth, std::optional<int> newCursorHeight)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetCursorTexture");
}

void UComputerWindow::SetFontColor(const Color& newFontColor)
{
	// UNUSED from scripts.
	FontColor() = newFontColor;
}

void UComputerWindow::SetTextPosition(int posX, int posY)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetTextPosition");
}

void UComputerWindow::SetTextSound(UObject* newTextSound)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetTextSound");
}

void UComputerWindow::SetTextWindowPosition(int newX, int newY)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetTextWindowPosition");
}

void UComputerWindow::SetThrottle(float throttleModifier)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetThrottle");
}

void UComputerWindow::SetTypingSound(UObject* newTypingSound)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetTypingSound");
}

void UComputerWindow::SetTypingSoundVolume(float newSoundVolume)
{
	// UNUSED from scripts.
	LogUnimplemented("ComputerWindow.SetTypingSoundVolume");
}

void UComputerWindow::ClearScreen()
{
	text.clear();
}

void UComputerWindow::EnableWordWrap(std::optional<bool> bNewWordWrap)
{
	// Script always calls this and calls it with wordwrap enabled
}

void UComputerWindow::Print(const std::string& printText, std::optional<bool> bNewLine)
{
	text += printText;
	if (bNewLine.has_value() && bNewLine.value())
		text += '\n';
}

void UComputerWindow::SetCursorColor(const Color& newCursorColor)
{
	cursorColor() = newCursorColor;
}

void UComputerWindow::SetFadeSpeed(float newFadeSpeed)
{
	fadeSpeed() = newFadeSpeed;
}

void UComputerWindow::SetTextFont(UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor)
{
	textFont() = UObject::Cast<UFont>(NewFont);
}

void UComputerWindow::SetTextSize(int newCols, int newRows)
{
	textCols() = newCols;
	textRows() = newRows;
}

void UComputerWindow::SetTextTiming(float newTiming)
{
	eventTimeInterval() = newTiming;
}

void UComputerWindow::ShowTextCursor(std::optional<bool> bShow)
{
	bShowCursor() = !bShow.has_value() ? true : bShow.value();
}

void UComputerWindow::InitWindow()
{
	UWindow::InitWindow();
}

void UComputerWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	if (UFont* font = textFont())
	{
		if (!bWidthSpecified)
			preferredWidth = (float)font->GetGlyph('X').USize * textCols();
		if (!bHeightSpecified)
			preferredHeight = (float)font->GetGlyph('X').VSize * textRows();
	}

	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UComputerWindow::DrawWindow(UGC* gc)
{
	// How does a computer window look like in DX?
	// Seems to be some kind of text scrolling effect that scrolls the written text as well

	if (UFont* font = textFont())
	{
		float w = Width();
		float h = Height();
		if (w > 0.0f && h > 0.0f)
		{
			gc->SetFonts(font, font);
			gc->bWordWrap() = true;
			gc->DrawText(0.0f, 0.0f, w, h, text);
		}
	}

	if (bShowCursor())
	{
		// To do: blink the cursor
	}

	// DrawDebugBox(gc);

	UWindow::DrawWindow(gc);
}
