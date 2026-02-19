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
	LogUnimplemented("NDeusExTextParser::CloseText not implemented");
}

void NDeusExTextParser::GetColor(UObject* Self, Color& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::GetColor not implemented");
	ReturnValue = { 0, 0, 0, 0 };
}

void NDeusExTextParser::GetEmailInfo(UObject* Self, std::string& emailName, std::string& emailSubject, std::string& emailFrom, std::string& emailTo, std::string& emailCC)
{
	LogUnimplemented("NDeusExTextParser::GetEmailInfo not implemented");
}

void NDeusExTextParser::GetFileInfo(UObject* Self, std::string& FileName, std::string& fileDescription)
{
	LogUnimplemented("NDeusExTextParser::GetFileInfo not implemented");
}

void NDeusExTextParser::GetName(UObject* Self, NameString& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::GetName not implemented");
	ReturnValue = {};
}

void NDeusExTextParser::GetTag(UObject* Self, uint8_t& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::GetTag not implemented");
	ReturnValue = 0;
}

void NDeusExTextParser::GetText(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::GetText not implemented");
	ReturnValue = "";
}

void NDeusExTextParser::GotoLabel(UObject* Self, const std::string& Label)
{
	LogUnimplemented("NDeusExTextParser::GotoLabel not implemented");
}

void NDeusExTextParser::IsEOF(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::IsEOF not implemented");
	ReturnValue = false;
}

void NDeusExTextParser::OpenText(UObject* Self, const NameString& textName, std::string* TextPackage, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::OpenText not implemented");
	ReturnValue = false;
}

void NDeusExTextParser::ProcessText(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("NDeusExTextParser::ProcessText not implemented");
	ReturnValue = false;
}

void NDeusExTextParser::SetPlayerName(UObject* Self, const std::string& newPlayerName)
{
	LogUnimplemented("NDeusExTextParser::SetPlayerName not implemented");
}
