#include "Precomp.h"
#include "NTextWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NTextWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("TextWindow", "AppendText", &NTextWindow::AppendText, 1551);
	RegisterVMNativeFunc_1("TextWindow", "EnableTextAsAccelerator", &NTextWindow::EnableTextAsAccelerator, 1564);
	RegisterVMNativeFunc_1("TextWindow", "GetText", &NTextWindow::GetText, 1552);
	RegisterVMNativeFunc_1("TextWindow", "GetTextLength", &NTextWindow::GetTextLength, 1553);
	RegisterVMNativeFunc_4("TextWindow", "GetTextPart", &NTextWindow::GetTextPart, 1554);
	RegisterVMNativeFunc_0("TextWindow", "ResetLines", &NTextWindow::ResetLines, 1561);
	RegisterVMNativeFunc_0("TextWindow", "ResetMinWidth", &NTextWindow::ResetMinWidth, 1563);
	RegisterVMNativeFunc_2("TextWindow", "SetLines", &NTextWindow::SetLines, 1558);
	RegisterVMNativeFunc_1("TextWindow", "SetMaxLines", &NTextWindow::SetMaxLines, 1560);
	RegisterVMNativeFunc_1("TextWindow", "SetMinLines", &NTextWindow::SetMinLines, 1559);
	RegisterVMNativeFunc_1("TextWindow", "SetMinWidth", &NTextWindow::SetMinWidth, 1562);
	RegisterVMNativeFunc_1("TextWindow", "SetText", &NTextWindow::SetText, 1550);
	RegisterVMNativeFunc_2("TextWindow", "SetTextAlignments", &NTextWindow::SetTextAlignments, 1556);
	RegisterVMNativeFunc_2("TextWindow", "SetTextMargins", &NTextWindow::SetTextMargins, 1557);
	RegisterVMNativeFunc_1("TextWindow", "SetWordWrap", &NTextWindow::SetWordWrap, 1555);
}

void NTextWindow::AppendText(UObject* Self, const std::string& NewText)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->AppendText(NewText);
}

void NTextWindow::EnableTextAsAccelerator(UObject* Self, BitfieldBool* bEnable)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->EnableTextAsAccelerator(bEnable);
}

void NTextWindow::GetText(UObject* Self, std::string& ReturnValue)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	ReturnValue = textWindow->GetText();
}

void NTextWindow::GetTextLength(UObject* Self, int& ReturnValue)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	ReturnValue = textWindow->GetTextLength();
}

void NTextWindow::GetTextPart(UObject* Self, int startPos, int Count, std::string& OutText, int& ReturnValue)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	ReturnValue = textWindow->GetTextPart(startPos, Count, OutText);
}

void NTextWindow::ResetLines(UObject* Self)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->ResetLines();
}

void NTextWindow::ResetMinWidth(UObject* Self)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->ResetMinWidth();
}

void NTextWindow::SetLines(UObject* Self, int newMinLines, int newMaxLines)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetLines(newMinLines, newMaxLines);
}

void NTextWindow::SetMaxLines(UObject* Self, int newMaxLines)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetMaxLines(newMaxLines);
}

void NTextWindow::SetMinLines(UObject* Self, int newMinLines)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetMinLines(newMinLines);
}

void NTextWindow::SetMinWidth(UObject* Self, float newMinWidth)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetMinWidth(newMinWidth);
}

void NTextWindow::SetText(UObject* Self, const std::string& NewText)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetText(NewText);
}

void NTextWindow::SetTextAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetTextAlignments(newHAlign, newVAlign);
}

void NTextWindow::SetTextMargins(UObject* Self, float newHMargin, float newVMargin)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetTextMargins(newHMargin, newVMargin);
}

void NTextWindow::SetWordWrap(UObject* Self, bool bNewWordWrap)
{
	UTextWindow* textWindow = UObject::Cast<UTextWindow>(Self);
	textWindow->SetWordWrap(bNewWordWrap);
}
