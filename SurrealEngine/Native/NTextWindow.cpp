#include "Precomp.h"
#include "NTextWindow.h"
#include "VM/NativeFunc.h"
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
	LogUnimplemented("TextWindow.AppendText");
}

void NTextWindow::EnableTextAsAccelerator(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("TextWindow.EnableTextAsAccelerator");
}

void NTextWindow::GetText(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("TextWindow.GetText");
	ReturnValue = "";
}

void NTextWindow::GetTextLength(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("TextWindow.GetTextLength");
	ReturnValue = 0;
}

void NTextWindow::GetTextPart(UObject* Self, int startPos, int Count, std::string& OutText, int& ReturnValue)
{
	LogUnimplemented("TextWindow.GetTextPart");
	ReturnValue = 0;
}

void NTextWindow::ResetLines(UObject* Self)
{
	LogUnimplemented("TextWindow.ResetLines");
}

void NTextWindow::ResetMinWidth(UObject* Self)
{
	LogUnimplemented("TextWindow.ResetMinWidth");
}

void NTextWindow::SetLines(UObject* Self, int newMinLines, int newMaxLines)
{
	LogUnimplemented("TextWindow.SetLines");
}

void NTextWindow::SetMaxLines(UObject* Self, int newMaxLines)
{
	LogUnimplemented("TextWindow.SetMaxLines");
}

void NTextWindow::SetMinLines(UObject* Self, int newMinLines)
{
	LogUnimplemented("TextWindow.SetMinLines");
}

void NTextWindow::SetMinWidth(UObject* Self, float newMinWidth)
{
	LogUnimplemented("TextWindow.SetMinWidth");
}

void NTextWindow::SetText(UObject* Self, const std::string& NewText)
{
	LogUnimplemented("TextWindow.SetText");
}

void NTextWindow::SetTextAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	LogUnimplemented("TextWindow.SetTextAlignments");
}

void NTextWindow::SetTextMargins(UObject* Self, float newHMargin, float newVMargin)
{
	LogUnimplemented("TextWindow.SetTextMargins");
}

void NTextWindow::SetWordWrap(UObject* Self, bool bNewWordWrap)
{
	LogUnimplemented("TextWindow.SetWordWrap");
}
