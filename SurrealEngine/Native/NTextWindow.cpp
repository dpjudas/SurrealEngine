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
	LogUnimplemented("NTextWindow::AppendText not implemented");
}

void NTextWindow::EnableTextAsAccelerator(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("NTextWindow::EnableTextAsAccelerator not implemented");
}

void NTextWindow::GetText(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("NTextWindow::GetText not implemented");
	ReturnValue = "";
}

void NTextWindow::GetTextLength(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NTextWindow::GetTextLength not implemented");
	ReturnValue = 0;
}

void NTextWindow::GetTextPart(UObject* Self, int startPos, int Count, std::string& OutText, int& ReturnValue)
{
	LogUnimplemented("NTextWindow::GetTextPart not implemented");
	ReturnValue = 0;
}

void NTextWindow::ResetLines(UObject* Self)
{
	LogUnimplemented("NTextWindow::ResetLines not implemented");
}

void NTextWindow::ResetMinWidth(UObject* Self)
{
	LogUnimplemented("NTextWindow::ResetMinWidth not implemented");
}

void NTextWindow::SetLines(UObject* Self, int newMinLines, int newMaxLines)
{
	LogUnimplemented("NTextWindow::SetLines not implemented");
}

void NTextWindow::SetMaxLines(UObject* Self, int newMaxLines)
{
	LogUnimplemented("NTextWindow::SetMaxLines not implemented");
}

void NTextWindow::SetMinLines(UObject* Self, int newMinLines)
{
	LogUnimplemented("NTextWindow::SetMinLines not implemented");
}

void NTextWindow::SetMinWidth(UObject* Self, float newMinWidth)
{
	LogUnimplemented("NTextWindow::SetMinWidth not implemented");
}

void NTextWindow::SetText(UObject* Self, const std::string& NewText)
{
	LogUnimplemented("NTextWindow::SetText not implemented");
}

void NTextWindow::SetTextAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	LogUnimplemented("NTextWindow::SetTextAlignments not implemented");
}

void NTextWindow::SetTextMargins(UObject* Self, float newHMargin, float newVMargin)
{
	LogUnimplemented("NTextWindow::SetTextMargins not implemented");
}

void NTextWindow::SetWordWrap(UObject* Self, bool bNewWordWrap)
{
	LogUnimplemented("NTextWindow::SetWordWrap not implemented");
}
