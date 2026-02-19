#include "Precomp.h"
#include "NComputerWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NComputerWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ComputerWindow", "ClearLine", &NComputerWindow::ClearLine, 1986);
	RegisterVMNativeFunc_0("ComputerWindow", "ClearScreen", &NComputerWindow::ClearScreen, 1985);
	RegisterVMNativeFunc_1("ComputerWindow", "EnableWordWrap", &NComputerWindow::EnableWordWrap, 2001);
	RegisterVMNativeFunc_1("ComputerWindow", "FadeOutText", &NComputerWindow::FadeOutText, 2002);
	RegisterVMNativeFunc_2("ComputerWindow", "GetChar", &NComputerWindow::GetChar, 1990);
	RegisterVMNativeFunc_4("ComputerWindow", "GetInput", &NComputerWindow::GetInput, 1989);
	RegisterVMNativeFunc_1("ComputerWindow", "GetThrottle", &NComputerWindow::GetThrottle, 1999);
	RegisterVMNativeFunc_1("ComputerWindow", "IsBufferFlushed", &NComputerWindow::IsBufferFlushed, 1994);
	RegisterVMNativeFunc_1("ComputerWindow", "IsPaused", &NComputerWindow::IsPaused, 1997);
	RegisterVMNativeFunc_1("ComputerWindow", "Pause", &NComputerWindow::Pause, 1995);
	RegisterVMNativeFunc_1("ComputerWindow", "PlaySoundLater", &NComputerWindow::PlaySoundLater, 1992);
	RegisterVMNativeFunc_2("ComputerWindow", "Print", &NComputerWindow::Print, 1987);
	RegisterVMNativeFunc_7("ComputerWindow", "PrintGraphic", &NComputerWindow::PrintGraphic, 1991);
	RegisterVMNativeFunc_0("ComputerWindow", "PrintLn", &NComputerWindow::PrintLn, 1988);
	RegisterVMNativeFunc_0("ComputerWindow", "ResetThrottle", &NComputerWindow::ResetThrottle, 2000);
	RegisterVMNativeFunc_0("ComputerWindow", "Resume", &NComputerWindow::Resume, 1996);
	RegisterVMNativeFunc_6("ComputerWindow", "SetBackgroundTextures", &NComputerWindow::SetBackgroundTextures, 1970);
	RegisterVMNativeFunc_1("ComputerWindow", "SetComputerSoundVolume", &NComputerWindow::SetComputerSoundVolume, 1983);
	RegisterVMNativeFunc_1("ComputerWindow", "SetCursorBlinkSpeed", &NComputerWindow::SetCursorBlinkSpeed, 1979);
	RegisterVMNativeFunc_1("ComputerWindow", "SetCursorColor", &NComputerWindow::SetCursorColor, 1978);
	RegisterVMNativeFunc_3("ComputerWindow", "SetCursorTexture", &NComputerWindow::SetCursorTexture, 1977);
	RegisterVMNativeFunc_1("ComputerWindow", "SetFadeSpeed", &NComputerWindow::SetFadeSpeed, 1976);
	RegisterVMNativeFunc_1("ComputerWindow", "SetFontColor", &NComputerWindow::SetFontColor, 1974);
	RegisterVMNativeFunc_4("ComputerWindow", "SetTextFont", &NComputerWindow::SetTextFont, 1973);
	RegisterVMNativeFunc_2("ComputerWindow", "SetTextPosition", &NComputerWindow::SetTextPosition, 1993);
	RegisterVMNativeFunc_2("ComputerWindow", "SetTextSize", &NComputerWindow::SetTextSize, 1971);
	RegisterVMNativeFunc_1("ComputerWindow", "SetTextSound", &NComputerWindow::SetTextSound, 1981);
	RegisterVMNativeFunc_1("ComputerWindow", "SetTextTiming", &NComputerWindow::SetTextTiming, 1975);
	RegisterVMNativeFunc_2("ComputerWindow", "SetTextWindowPosition", &NComputerWindow::SetTextWindowPosition, 1972);
	RegisterVMNativeFunc_1("ComputerWindow", "SetThrottle", &NComputerWindow::SetThrottle, 1998);
	RegisterVMNativeFunc_1("ComputerWindow", "SetTypingSound", &NComputerWindow::SetTypingSound, 1982);
	RegisterVMNativeFunc_1("ComputerWindow", "SetTypingSoundVolume", &NComputerWindow::SetTypingSoundVolume, 1984);
	RegisterVMNativeFunc_1("ComputerWindow", "ShowTextCursor", &NComputerWindow::ShowTextCursor, 1980);
}

void NComputerWindow::ClearLine(UObject* Self, int rowToClear)
{
	LogUnimplemented("NComputerWindow::ClearLine not implemented");
}

void NComputerWindow::ClearScreen(UObject* Self)
{
	LogUnimplemented("NComputerWindow::ClearScreen not implemented");
}

void NComputerWindow::EnableWordWrap(UObject* Self, BitfieldBool* bNewWordWrap)
{
	LogUnimplemented("NComputerWindow::EnableWordWrap not implemented");
}

void NComputerWindow::FadeOutText(UObject* Self, float* fadeDuration)
{
	LogUnimplemented("NComputerWindow::FadeOutText not implemented");
}

void NComputerWindow::GetChar(UObject* Self, const std::string& inputKey, BitfieldBool* bEcho)
{
	LogUnimplemented("NComputerWindow::GetChar not implemented");
}

void NComputerWindow::GetInput(UObject* Self, int MaxLength, const std::string& inputKey, std::string* defaultInputString, std::string* inputMask)
{
	LogUnimplemented("NComputerWindow::GetInput not implemented");
}

void NComputerWindow::GetThrottle(UObject* Self, float& ReturnValue)
{
	LogUnimplemented("NComputerWindow::GetThrottle not implemented");
	ReturnValue = 0.0f;
}

void NComputerWindow::IsBufferFlushed(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NComputerWindow::IsBufferFlushed not implemented");
	ReturnValue = false;
}

void NComputerWindow::IsPaused(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NComputerWindow::IsPaused not implemented");
	ReturnValue = false;
}

void NComputerWindow::Pause(UObject* Self, float* pauseLength)
{
	LogUnimplemented("NComputerWindow::Pause not implemented");
}

void NComputerWindow::PlaySoundLater(UObject* Self, UObject* newsound)
{
	LogUnimplemented("NComputerWindow::PlaySoundLater not implemented");
}

void NComputerWindow::Print(UObject* Self, const std::string& printText, BitfieldBool* bNewLine)
{
	LogUnimplemented("NComputerWindow::Print not implemented");
}

void NComputerWindow::PrintGraphic(UObject* Self, UObject* Graphic, int Width, int Height, int* posX, int* posY, BitfieldBool* bStatic, BitfieldBool* bPixelPos)
{
	LogUnimplemented("NComputerWindow::PrintGraphic not implemented");
}

void NComputerWindow::PrintLn(UObject* Self)
{
	LogUnimplemented("NComputerWindow::PrintLn not implemented");
}

void NComputerWindow::ResetThrottle(UObject* Self)
{
	LogUnimplemented("NComputerWindow::ResetThrottle not implemented");
}

void NComputerWindow::Resume(UObject* Self)
{
	LogUnimplemented("NComputerWindow::Resume not implemented");
}

void NComputerWindow::SetBackgroundTextures(UObject* Self, UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6)
{
	LogUnimplemented("NComputerWindow::SetBackgroundTextures not implemented");
}

void NComputerWindow::SetComputerSoundVolume(UObject* Self, float newSoundVolume)
{
	LogUnimplemented("NComputerWindow::SetComputerSoundVolume not implemented");
}

void NComputerWindow::SetCursorBlinkSpeed(UObject* Self, float newBlinkSpeed)
{
	LogUnimplemented("NComputerWindow::SetCursorBlinkSpeed not implemented");
}

void NComputerWindow::SetCursorColor(UObject* Self, const Color& newCursorColor)
{
	LogUnimplemented("NComputerWindow::SetCursorColor not implemented");
}

void NComputerWindow::SetCursorTexture(UObject* Self, UObject* newCursorTexture, int* newCursorWidth, int* newCursorHeight)
{
	LogUnimplemented("NComputerWindow::SetCursorTexture not implemented");
}

void NComputerWindow::SetFadeSpeed(UObject* Self, float fadeSpeed)
{
	LogUnimplemented("NComputerWindow::SetFadeSpeed not implemented");
}

void NComputerWindow::SetFontColor(UObject* Self, const Color& newFontColor)
{
	LogUnimplemented("NComputerWindow::SetFontColor not implemented");
}

void NComputerWindow::SetTextFont(UObject* Self, UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor)
{
	LogUnimplemented("NComputerWindow::SetTextFont not implemented");
}

void NComputerWindow::SetTextPosition(UObject* Self, int posX, int posY)
{
	LogUnimplemented("NComputerWindow::SetTextPosition not implemented");
}

void NComputerWindow::SetTextSize(UObject* Self, int newCols, int newRows)
{
	LogUnimplemented("NComputerWindow::SetTextSize not implemented");
}

void NComputerWindow::SetTextSound(UObject* Self, UObject* newTextSound)
{
	LogUnimplemented("NComputerWindow::SetTextSound not implemented");
}

void NComputerWindow::SetTextTiming(UObject* Self, float newTiming)
{
	LogUnimplemented("NComputerWindow::SetTextTiming not implemented");
}

void NComputerWindow::SetTextWindowPosition(UObject* Self, int newX, int newY)
{
	LogUnimplemented("NComputerWindow::SetTextWindowPosition not implemented");
}

void NComputerWindow::SetThrottle(UObject* Self, float throttleModifier)
{
	LogUnimplemented("NComputerWindow::SetThrottle not implemented");
}

void NComputerWindow::SetTypingSound(UObject* Self, UObject* newTypingSound)
{
	LogUnimplemented("NComputerWindow::SetTypingSound not implemented");
}

void NComputerWindow::SetTypingSoundVolume(UObject* Self, float newSoundVolume)
{
	LogUnimplemented("NComputerWindow::SetTypingSoundVolume not implemented");
}

void NComputerWindow::ShowTextCursor(UObject* Self, BitfieldBool* bShow)
{
	LogUnimplemented("NComputerWindow::ShowTextCursor not implemented");
}
