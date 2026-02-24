#include "Precomp.h"
#include "NExtString.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NExtString::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ExtString", "AppendText", &NExtString::AppendText, 1141);
	RegisterVMNativeFunc_2("ExtString", "GetFirstTextPart", &NExtString::GetFirstTextPart, 1145);
	RegisterVMNativeFunc_2("ExtString", "GetNextTextPart", &NExtString::GetNextTextPart, 1146);
	RegisterVMNativeFunc_1("ExtString", "GetText", &NExtString::GetText, 1142);
	RegisterVMNativeFunc_1("ExtString", "GetTextLength", &NExtString::GetTextLength, 1143);
	RegisterVMNativeFunc_4("ExtString", "GetTextPart", &NExtString::GetTextPart, 1144);
	RegisterVMNativeFunc_1("ExtString", "SetText", &NExtString::SetText, 1140);
}

void NExtString::AppendText(UObject* Self, const std::string& NewText)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ESSelf->AppendText(NewText);
}

void NExtString::GetFirstTextPart(UObject* Self, std::string& OutText, int& ReturnValue)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ReturnValue = ESSelf->GetFirstTextPart(OutText);
}

void NExtString::GetNextTextPart(UObject* Self, std::string& OutText, int& ReturnValue)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ReturnValue = ESSelf->GetNextTextPart(OutText);
}

void NExtString::GetText(UObject* Self, std::string& ReturnValue)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ReturnValue = ESSelf->GetText();
}

void NExtString::GetTextLength(UObject* Self, int& ReturnValue)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ReturnValue = ESSelf->GetTextLength();
}

void NExtString::GetTextPart(UObject* Self, int startPos, int Count, std::string& OutText, int& ReturnValue)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ReturnValue = ESSelf->GetTextPart(startPos, Count, OutText);
}

void NExtString::SetText(UObject* Self, const std::string& NewText)
{
	auto ESSelf = UObject::Cast<UDXExtString>(Self);
	ESSelf->SetText(NewText);
}
