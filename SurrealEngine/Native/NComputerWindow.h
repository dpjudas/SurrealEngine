#pragma once

#include "UObject/UObject.h"

class NComputerWindow
{
public:
	static void RegisterFunctions();

	static void ClearLine(UObject* Self, int rowToClear);
	static void ClearScreen(UObject* Self);
	static void EnableWordWrap(UObject* Self, BitfieldBool* bNewWordWrap);
	static void FadeOutText(UObject* Self, float* fadeDuration);
	static void GetChar(UObject* Self, const std::string& inputKey, BitfieldBool* bEcho);
	static void GetInput(UObject* Self, int MaxLength, const std::string& inputKey, std::string* defaultInputString, std::string* inputMask);
	static void GetThrottle(UObject* Self, float& ReturnValue);
	static void IsBufferFlushed(UObject* Self, BitfieldBool& ReturnValue);
	static void IsPaused(UObject* Self, BitfieldBool& ReturnValue);
	static void Pause(UObject* Self, float* pauseLength);
	static void PlaySoundLater(UObject* Self, UObject* newsound);
	static void Print(UObject* Self, const std::string& printText, BitfieldBool* bNewLine);
	static void PrintGraphic(UObject* Self, UObject* Graphic, int Width, int Height, int* posX, int* posY, BitfieldBool* bStatic, BitfieldBool* bPixelPos);
	static void PrintLn(UObject* Self);
	static void ResetThrottle(UObject* Self);
	static void Resume(UObject* Self);
	static void SetBackgroundTextures(UObject* Self, UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6);
	static void SetComputerSoundVolume(UObject* Self, float newSoundVolume);
	static void SetCursorBlinkSpeed(UObject* Self, float newBlinkSpeed);
	static void SetCursorColor(UObject* Self, const Color& newCursorColor);
	static void SetCursorTexture(UObject* Self, UObject* newCursorTexture, int* newCursorWidth, int* newCursorHeight);
	static void SetFadeSpeed(UObject* Self, float fadeSpeed);
	static void SetFontColor(UObject* Self, const Color& newFontColor);
	static void SetTextFont(UObject* Self, UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor);
	static void SetTextPosition(UObject* Self, int posX, int posY);
	static void SetTextSize(UObject* Self, int newCols, int newRows);
	static void SetTextSound(UObject* Self, UObject* newTextSound);
	static void SetTextTiming(UObject* Self, float newTiming);
	static void SetTextWindowPosition(UObject* Self, int newX, int newY);
	static void SetThrottle(UObject* Self, float throttleModifier);
	static void SetTypingSound(UObject* Self, UObject* newTypingSound);
	static void SetTypingSoundVolume(UObject* Self, float newSoundVolume);
	static void ShowTextCursor(UObject* Self, BitfieldBool* bShow);
};
