#pragma once

#include "UWindow.h"

class UEditWindow;

class UComputerWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void ClearLine(int rowToClear);
	void ClearScreen();
	void EnableWordWrap(std::optional<bool> bNewWordWrap);
	void FadeOutText(std::optional<float> fadeDuration);
	void GetChar(const std::string& inputKey, std::optional<bool> bEcho);
	void GetInput(int MaxLength, const std::string& inputKey, std::optional<std::string> defaultInputString, std::optional<std::string> inputMask);
	float GetThrottle();
	bool IsBufferFlushed();
	bool IsPaused();
	void Pause(std::optional<float> pauseLength);
	void PlaySoundLater(UObject* newsound);
	void Print(const std::string& printText, std::optional<bool> bNewLine);
	void PrintGraphic(UObject* Graphic, int Width, int Height, std::optional<int> posX, std::optional<int> posY, std::optional<bool> bStatic, std::optional<bool> bPixelPos);
	void PrintLn();
	void ResetThrottle();
	void Resume();
	void SetBackgroundTextures(UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6);
	void SetComputerSoundVolume(float newSoundVolume);
	void SetCursorBlinkSpeed(float newBlinkSpeed);
	void SetCursorColor(const Color& newCursorColor);
	void SetCursorTexture(UObject* newCursorTexture, std::optional<int> newCursorWidth, std::optional<int> newCursorHeight);
	void SetFadeSpeed(float newFadeSpeed);
	void SetFontColor(const Color& newFontColor);
	void SetTextFont(UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor);
	void SetTextPosition(int posX, int posY);
	void SetTextSize(int newCols, int newRows);
	void SetTextSound(UObject* newTextSound);
	void SetTextTiming(float newTiming);
	void SetTextWindowPosition(int newX, int newY);
	void SetThrottle(float throttleModifier);
	void SetTypingSound(UObject* newTypingSound);
	void SetTypingSoundVolume(float newSoundVolume);
	void ShowTextCursor(std::optional<bool> bShow);

	void InitWindow() override;
	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;
	void DrawWindow(UGC* gc) override;

	Color& FontColor() { return Value<Color>(PropOffsets_ComputerWindow.FontColor); }
	UPlayerPawnExt*& Player() { return Value<UPlayerPawnExt*>(PropOffsets_ComputerWindow.Player); }
	UWindow*& TextWindow() { return Value<UWindow*>(PropOffsets_ComputerWindow.TextWindow); }
	int& TextX() { return Value<int>(PropOffsets_ComputerWindow.TextX); }
	int& TextY() { return Value<int>(PropOffsets_ComputerWindow.TextY); }
	BitfieldBool bComputerStartInvoked() { return BoolValue(PropOffsets_ComputerWindow.bComputerStartInvoked); }
	BitfieldBool bCursorVisible() { return BoolValue(PropOffsets_ComputerWindow.bCursorVisible); }
	BitfieldBool bEchoKey() { return BoolValue(PropOffsets_ComputerWindow.bEchoKey); }
	BitfieldBool bFirstTick() { return BoolValue(PropOffsets_ComputerWindow.bFirstTick); }
	BitfieldBool bGamePaused() { return BoolValue(PropOffsets_ComputerWindow.bGamePaused); }
	BitfieldBool bIgnoreGamePaused() { return BoolValue(PropOffsets_ComputerWindow.bIgnoreGamePaused); }
	BitfieldBool bIgnoreTick() { return BoolValue(PropOffsets_ComputerWindow.bIgnoreTick); }
	BitfieldBool bInvokeComputerStart() { return BoolValue(PropOffsets_ComputerWindow.bInvokeComputerStart); }
	BitfieldBool bLastLineWrapped() { return BoolValue(PropOffsets_ComputerWindow.bLastLineWrapped); }
	BitfieldBool bPauseProcessing() { return BoolValue(PropOffsets_ComputerWindow.bPauseProcessing); }
	BitfieldBool bShowCursor() { return BoolValue(PropOffsets_ComputerWindow.bShowCursor); }
	BitfieldBool bWaitingForKey() { return BoolValue(PropOffsets_ComputerWindow.bWaitingForKey); }
	BitfieldBool bWordWrap() { return BoolValue(PropOffsets_ComputerWindow.bWordWrap); }
	UTexture*& backgroundTextures() { return Value<UTexture*>(PropOffsets_ComputerWindow.backgroundTextures); }
	Color& colGraphicTile() { return Value<Color>(PropOffsets_ComputerWindow.colGraphicTile); }
	float& computerSoundVolume() { return Value<float>(PropOffsets_ComputerWindow.computerSoundVolume); }
	float& cursorBlinkSpeed() { return Value<float>(PropOffsets_ComputerWindow.cursorBlinkSpeed); }
	Color& cursorColor() { return Value<Color>(PropOffsets_ComputerWindow.cursorColor); }
	int& cursorHeight() { return Value<int>(PropOffsets_ComputerWindow.cursorHeight); }
	float& cursorNextEvent() { return Value<float>(PropOffsets_ComputerWindow.cursorNextEvent); }
	UTexture*& cursorTexture() { return Value<UTexture*>(PropOffsets_ComputerWindow.cursorTexture); }
	int& cursorWidth() { return Value<int>(PropOffsets_ComputerWindow.cursorWidth); }
	//DynamicArray& displayBuffer() { return Value<DynamicArray>(PropOffsets_ComputerWindow.displayBuffer); }
	float& eventTimeInterval() { return Value<float>(PropOffsets_ComputerWindow.eventTimeInterval); }
	float& fadeOutStart() { return Value<float>(PropOffsets_ComputerWindow.fadeOutStart); }
	float& fadeOutTimer() { return Value<float>(PropOffsets_ComputerWindow.fadeOutTimer); }
	float& fadeSpeed() { return Value<float>(PropOffsets_ComputerWindow.fadeSpeed); }
	int& fontHeight() { return Value<int>(PropOffsets_ComputerWindow.fontHeight); }
	int& fontWidth() { return Value<int>(PropOffsets_ComputerWindow.fontWidth); }
	std::string& inputKey() { return Value<std::string>(PropOffsets_ComputerWindow.inputKey); }
	std::string& inputMask() { return Value<std::string>(PropOffsets_ComputerWindow.inputMask); }
	UEditWindow*& inputWindow() { return Value<UEditWindow*>(PropOffsets_ComputerWindow.inputWindow); }
	//DynamicArray& queuedBuffer() { return Value<DynamicArray>(PropOffsets_ComputerWindow.queuedBuffer); }
	int& queuedBufferStart() { return Value<int>(PropOffsets_ComputerWindow.queuedBufferStart); }
	int& textCols() { return Value<int>(PropOffsets_ComputerWindow.textCols); }
	UFont*& textFont() { return Value<UFont*>(PropOffsets_ComputerWindow.textFont); }
	int& textRows() { return Value<int>(PropOffsets_ComputerWindow.textRows); }
	USound*& textSound() { return Value<USound*>(PropOffsets_ComputerWindow.textSound); }
	float& throttle() { return Value<float>(PropOffsets_ComputerWindow.throttle); }
	float& timeCurrent() { return Value<float>(PropOffsets_ComputerWindow.timeCurrent); }
	float& timeLastEvent() { return Value<float>(PropOffsets_ComputerWindow.timeLastEvent); }
	float& timeNextEvent() { return Value<float>(PropOffsets_ComputerWindow.timeNextEvent); }
	USound*& typingSound() { return Value<USound*>(PropOffsets_ComputerWindow.typingSound); }

private:
	std::string text;
};
