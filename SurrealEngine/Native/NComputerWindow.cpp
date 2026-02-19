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
	LogUnimplemented("ComputerWindow.ClearLine");
}

void NComputerWindow::ClearScreen(UObject* Self)
{
	LogUnimplemented("ComputerWindow.ClearScreen");
}

void NComputerWindow::EnableWordWrap(UObject* Self, BitfieldBool* bNewWordWrap)
{
	LogUnimplemented("ComputerWindow.EnableWordWrap");
}

void NComputerWindow::FadeOutText(UObject* Self, float* fadeDuration)
{
	LogUnimplemented("ComputerWindow.FadeOutText");
}

void NComputerWindow::GetChar(UObject* Self, const std::string& inputKey, BitfieldBool* bEcho)
{
	LogUnimplemented("ComputerWindow.GetChar");
}

void NComputerWindow::GetInput(UObject* Self, int MaxLength, const std::string& inputKey, std::string* defaultInputString, std::string* inputMask)
{
	LogUnimplemented("ComputerWindow.GetInput");
}

void NComputerWindow::GetThrottle(UObject* Self, float& ReturnValue)
{
	LogUnimplemented("ComputerWindow.GetThrottle");
	ReturnValue = 0.0f;
}

void NComputerWindow::IsBufferFlushed(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("ComputerWindow.IsBufferFlushed");
	ReturnValue = false;
}

void NComputerWindow::IsPaused(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("ComputerWindow.IsPaused");
	ReturnValue = false;
}

void NComputerWindow::Pause(UObject* Self, float* pauseLength)
{
	LogUnimplemented("ComputerWindow.Pause");
}

void NComputerWindow::PlaySoundLater(UObject* Self, UObject* newsound)
{
	LogUnimplemented("ComputerWindow.PlaySoundLater");
}

void NComputerWindow::Print(UObject* Self, const std::string& printText, BitfieldBool* bNewLine)
{
	LogUnimplemented("ComputerWindow.Print");
}

void NComputerWindow::PrintGraphic(UObject* Self, UObject* Graphic, int Width, int Height, int* posX, int* posY, BitfieldBool* bStatic, BitfieldBool* bPixelPos)
{
	LogUnimplemented("ComputerWindow.PrintGraphic");
}

void NComputerWindow::PrintLn(UObject* Self)
{
	LogUnimplemented("ComputerWindow.PrintLn");
}

void NComputerWindow::ResetThrottle(UObject* Self)
{
	LogUnimplemented("ComputerWindow.ResetThrottle");
}

void NComputerWindow::Resume(UObject* Self)
{
	LogUnimplemented("ComputerWindow.Resume");
}

void NComputerWindow::SetBackgroundTextures(UObject* Self, UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6)
{
	LogUnimplemented("ComputerWindow.SetBackgroundTextures");
}

void NComputerWindow::SetComputerSoundVolume(UObject* Self, float newSoundVolume)
{
	LogUnimplemented("ComputerWindow.SetComputerSoundVolume");
}

void NComputerWindow::SetCursorBlinkSpeed(UObject* Self, float newBlinkSpeed)
{
	LogUnimplemented("ComputerWindow.SetCursorBlinkSpeed");
}

void NComputerWindow::SetCursorColor(UObject* Self, const Color& newCursorColor)
{
	LogUnimplemented("ComputerWindow.SetCursorColor");
}

void NComputerWindow::SetCursorTexture(UObject* Self, UObject* newCursorTexture, int* newCursorWidth, int* newCursorHeight)
{
	LogUnimplemented("ComputerWindow.SetCursorTexture");
}

void NComputerWindow::SetFadeSpeed(UObject* Self, float fadeSpeed)
{
	LogUnimplemented("ComputerWindow.SetFadeSpeed");
}

void NComputerWindow::SetFontColor(UObject* Self, const Color& newFontColor)
{
	LogUnimplemented("ComputerWindow.SetFontColor");
}

void NComputerWindow::SetTextFont(UObject* Self, UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor)
{
	LogUnimplemented("ComputerWindow.SetTextFont");
}

void NComputerWindow::SetTextPosition(UObject* Self, int posX, int posY)
{
	LogUnimplemented("ComputerWindow.SetTextPosition");
}

void NComputerWindow::SetTextSize(UObject* Self, int newCols, int newRows)
{
	LogUnimplemented("ComputerWindow.SetTextSize");
}

void NComputerWindow::SetTextSound(UObject* Self, UObject* newTextSound)
{
	LogUnimplemented("ComputerWindow.SetTextSound");
}

void NComputerWindow::SetTextTiming(UObject* Self, float newTiming)
{
	LogUnimplemented("ComputerWindow.SetTextTiming");
}

void NComputerWindow::SetTextWindowPosition(UObject* Self, int newX, int newY)
{
	LogUnimplemented("ComputerWindow.SetTextWindowPosition");
}

void NComputerWindow::SetThrottle(UObject* Self, float throttleModifier)
{
	LogUnimplemented("ComputerWindow.SetThrottle");
}

void NComputerWindow::SetTypingSound(UObject* Self, UObject* newTypingSound)
{
	LogUnimplemented("ComputerWindow.SetTypingSound");
}

void NComputerWindow::SetTypingSoundVolume(UObject* Self, float newSoundVolume)
{
	LogUnimplemented("ComputerWindow.SetTypingSoundVolume");
}

void NComputerWindow::ShowTextCursor(UObject* Self, BitfieldBool* bShow)
{
	LogUnimplemented("ComputerWindow.ShowTextCursor");
}
