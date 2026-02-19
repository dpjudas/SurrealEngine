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
	Exception::Throw("NDeusExTextParser::CloseText not implemented");
}

void NDeusExTextParser::GetColor(UObject* Self, Color& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::GetColor not implemented");
}

void NDeusExTextParser::GetEmailInfo(UObject* Self, std::string& emailName, std::string& emailSubject, std::string& emailFrom, std::string& emailTo, std::string& emailCC)
{
	Exception::Throw("NDeusExTextParser::GetEmailInfo not implemented");
}

void NDeusExTextParser::GetFileInfo(UObject* Self, std::string& FileName, std::string& fileDescription)
{
	Exception::Throw("NDeusExTextParser::GetFileInfo not implemented");
}

void NDeusExTextParser::GetName(UObject* Self, NameString& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::GetName not implemented");
}

void NDeusExTextParser::GetTag(UObject* Self, uint8_t& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::GetTag not implemented");
}

void NDeusExTextParser::GetText(UObject* Self, std::string& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::GetText not implemented");
}

void NDeusExTextParser::GotoLabel(UObject* Self, const std::string& Label)
{
	Exception::Throw("NDeusExTextParser::GotoLabel not implemented");
}

void NDeusExTextParser::IsEOF(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::IsEOF not implemented");
}

void NDeusExTextParser::OpenText(UObject* Self, const NameString& textName, std::string* TextPackage, BitfieldBool& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::OpenText not implemented");
}

void NDeusExTextParser::ProcessText(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NDeusExTextParser::ProcessText not implemented");
}

void NDeusExTextParser::SetPlayerName(UObject* Self, const std::string& newPlayerName)
{
	Exception::Throw("NDeusExTextParser::SetPlayerName not implemented");
}
