#include "Precomp.h"
#include "NComputerWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->ClearLine(rowToClear);
}

void NComputerWindow::ClearScreen(UObject* Self)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->ClearScreen();
}

void NComputerWindow::EnableWordWrap(UObject* Self, BitfieldBool* bNewWordWrap)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->EnableWordWrap(bNewWordWrap);
}

void NComputerWindow::FadeOutText(UObject* Self, float* fadeDuration)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->FadeOutText(fadeDuration);
}

void NComputerWindow::GetChar(UObject* Self, const std::string& inputKey, BitfieldBool* bEcho)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->GetChar(inputKey, bEcho);
}

void NComputerWindow::GetInput(UObject* Self, int MaxLength, const std::string& inputKey, std::string* defaultInputString, std::string* inputMask)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->GetInput(MaxLength, inputKey, defaultInputString, inputMask);
}

void NComputerWindow::GetThrottle(UObject* Self, float& ReturnValue)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	ReturnValue = window->GetThrottle();
}

void NComputerWindow::IsBufferFlushed(UObject* Self, BitfieldBool& ReturnValue)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	ReturnValue = window->IsBufferFlushed();
}

void NComputerWindow::IsPaused(UObject* Self, BitfieldBool& ReturnValue)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	ReturnValue = window->IsPaused();
}

void NComputerWindow::Pause(UObject* Self, float* pauseLength)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->Pause(pauseLength);
}

void NComputerWindow::PlaySoundLater(UObject* Self, UObject* newsound)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->PlaySoundLater(newsound);
}

void NComputerWindow::Print(UObject* Self, const std::string& printText, BitfieldBool* bNewLine)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->Print(printText, bNewLine);
}

void NComputerWindow::PrintGraphic(UObject* Self, UObject* Graphic, int Width, int Height, int* posX, int* posY, BitfieldBool* bStatic, BitfieldBool* bPixelPos)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->PrintGraphic(Graphic, Width, Height, posX, posY, bStatic, bPixelPos);
}

void NComputerWindow::PrintLn(UObject* Self)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->PrintLn();
}

void NComputerWindow::ResetThrottle(UObject* Self)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->ResetThrottle();
}

void NComputerWindow::Resume(UObject* Self)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->Resume();
}

void NComputerWindow::SetBackgroundTextures(UObject* Self, UObject* backTexture1, UObject* backTexture2, UObject* backTexture3, UObject* backTexture4, UObject* backTexture5, UObject* backTexture6)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetBackgroundTextures(backTexture1, backTexture2, backTexture3, backTexture4, backTexture5, backTexture6);
}

void NComputerWindow::SetComputerSoundVolume(UObject* Self, float newSoundVolume)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetComputerSoundVolume(newSoundVolume);
}

void NComputerWindow::SetCursorBlinkSpeed(UObject* Self, float newBlinkSpeed)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetCursorBlinkSpeed(newBlinkSpeed);
}

void NComputerWindow::SetCursorColor(UObject* Self, const Color& newCursorColor)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetCursorColor(newCursorColor);
}

void NComputerWindow::SetCursorTexture(UObject* Self, UObject* newCursorTexture, int* newCursorWidth, int* newCursorHeight)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetCursorTexture(newCursorTexture, newCursorWidth, newCursorHeight);
}

void NComputerWindow::SetFadeSpeed(UObject* Self, float fadeSpeed)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetFadeSpeed(fadeSpeed);
}

void NComputerWindow::SetFontColor(UObject* Self, const Color& newFontColor)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetFontColor(newFontColor);
}

void NComputerWindow::SetTextFont(UObject* Self, UObject* NewFont, int newFontWidth, int newFontHeight, const Color& newFontColor)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTextFont(NewFont, newFontWidth, newFontHeight, newFontColor);
}

void NComputerWindow::SetTextPosition(UObject* Self, int posX, int posY)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTextPosition(posX, posY);
}

void NComputerWindow::SetTextSize(UObject* Self, int newCols, int newRows)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTextSize(newCols, newRows);
}

void NComputerWindow::SetTextSound(UObject* Self, UObject* newTextSound)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTextSound(newTextSound);
}

void NComputerWindow::SetTextTiming(UObject* Self, float newTiming)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTextTiming(newTiming);
}

void NComputerWindow::SetTextWindowPosition(UObject* Self, int newX, int newY)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTextWindowPosition(newX, newY);
}

void NComputerWindow::SetThrottle(UObject* Self, float throttleModifier)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetThrottle(throttleModifier);
}

void NComputerWindow::SetTypingSound(UObject* Self, UObject* newTypingSound)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTypingSound(newTypingSound);
}

void NComputerWindow::SetTypingSoundVolume(UObject* Self, float newSoundVolume)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->SetTypingSoundVolume(newSoundVolume);
}

void NComputerWindow::ShowTextCursor(UObject* Self, BitfieldBool* bShow)
{
	UComputerWindow* window = UObject::Cast<UComputerWindow>(Self);
	window->ShowTextCursor(bShow);
}
