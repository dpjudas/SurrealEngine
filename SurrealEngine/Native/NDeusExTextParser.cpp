#include "Precomp.h"
#include "NDeusExTextParser.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NDeusExTextParser::RegisterFunctions()
{
	RegisterVMNativeFunc_0("DeusExTextParser", "CloseText", &NDeusExTextParser::CloseText, 2211);
	RegisterVMNativeFunc_1("DeusExTextParser", "GetColor", &NDeusExTextParser::GetColor, 2218);
	RegisterVMNativeFunc_5("DeusExTextParser", "GetEmailInfo", &NDeusExTextParser::GetEmailInfo, 2219);
	RegisterVMNativeFunc_2("DeusExTextParser", "GetFileInfo", &NDeusExTextParser::GetFileInfo, 2220);
	RegisterVMNativeFunc_1("DeusExTextParser", "GetName", &NDeusExTextParser::GetName, 2217);
	RegisterVMNativeFunc_1("DeusExTextParser", "GetTag", &NDeusExTextParser::GetTag, 2216);
	RegisterVMNativeFunc_1("DeusExTextParser", "GetText", &NDeusExTextParser::GetText, 2214);
	RegisterVMNativeFunc_1("DeusExTextParser", "GotoLabel", &NDeusExTextParser::GotoLabel, 2215);
	RegisterVMNativeFunc_1("DeusExTextParser", "IsEOF", &NDeusExTextParser::IsEOF, 2213);
	RegisterVMNativeFunc_3("DeusExTextParser", "OpenText", &NDeusExTextParser::OpenText, 2210);
	RegisterVMNativeFunc_1("DeusExTextParser", "ProcessText", &NDeusExTextParser::ProcessText, 2212);
	RegisterVMNativeFunc_1("DeusExTextParser", "SetPlayerName", &NDeusExTextParser::SetPlayerName, 2221);
}

void NDeusExTextParser::CloseText(UObject* Self)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	DXTPSelf->CloseText();
}

void NDeusExTextParser::GetColor(UObject* Self, Color& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->GetColor();
}

void NDeusExTextParser::GetEmailInfo(UObject* Self, std::string& emailName, std::string& emailSubject, std::string& emailFrom, std::string& emailTo, std::string& emailCC)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	DXTPSelf->GetEmailInfo(emailName, emailSubject, emailFrom, emailTo, emailCC);
}

void NDeusExTextParser::GetFileInfo(UObject* Self, std::string& FileName, std::string& fileDescription)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	DXTPSelf->GetFileInfo(FileName, fileDescription);
}

void NDeusExTextParser::GetName(UObject* Self, NameString& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->GetName();
}

void NDeusExTextParser::GetTag(UObject* Self, uint8_t& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->GetTag();
}

void NDeusExTextParser::GetText(UObject* Self, std::string& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->GetText();
}

void NDeusExTextParser::GotoLabel(UObject* Self, const std::string& Label)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	DXTPSelf->GotoLabel(Label);
}

void NDeusExTextParser::IsEOF(UObject* Self, BitfieldBool& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->IsEOF();
}

void NDeusExTextParser::OpenText(UObject* Self, const NameString& textName, std::string* TextPackage, BitfieldBool& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->OpenText(textName, TextPackage ? *TextPackage : "");
}

void NDeusExTextParser::ProcessText(UObject* Self, BitfieldBool& ReturnValue)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	ReturnValue = DXTPSelf->ProcessText();
}

void NDeusExTextParser::SetPlayerName(UObject* Self, const std::string& newPlayerName)
{
	auto DXTPSelf = UObject::Cast<UDXTextParser>(Self);
	DXTPSelf->SetPlayerName(newPlayerName);
}
