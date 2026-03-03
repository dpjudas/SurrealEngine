#include "UDXTextParser.h"

#include "Utils/Logger.h"

bool UDXTextParser::OpenText(NameString textName, std::string textPackage)
{
    LogUnimplemented("DeusExTextParser.OpenText()");
    return false;
}

void UDXTextParser::CloseText()
{
    LogUnimplemented("DeusExTextParser.CloseText()");
}

bool UDXTextParser::ProcessText()
{
    LogUnimplemented("DeusExTextParser.ProcessText()");
    return false;
}

bool UDXTextParser::IsEOF()
{
    LogUnimplemented("DeusExTextParser.IsEOF()");
    return false;
}

std::string UDXTextParser::GetText()
{
    LogUnimplemented("DeusExTextParser.GetText()");
    return LastText();
}

void UDXTextParser::GotoLabel(const std::string& label)
{
    LogUnimplemented("DeusExTextParser.GotoLabel()");
}

uint8_t UDXTextParser::GetTag()
{
    LogUnimplemented("DeusExTextParser.GetTag()");
    return static_cast<uint8_t>(LastTag());
}

NameString UDXTextParser::GetName()
{
    return LastName();
}

Color UDXTextParser::GetColor()
{
    LogUnimplemented("DeusExTextParser.GetColor()");
    return LastColor();
}

void UDXTextParser::GetEmailInfo(std::string& name, std::string& subject, std::string& from, std::string& to, std::string& cc)
{
    LogUnimplemented("DeusExTextParser.GetEmailInfo()");
    name = LastEmailName();
    subject = LastEmailSubject();
    from = LastEmailFrom();
    to = LastEmailTo();
    cc = LastEmailCC();
}

void UDXTextParser::GetFileInfo(std::string& fileName, std::string& fileDescription)
{
    LogUnimplemented("DeusExTextParser.GetFileInfo()");
    fileName = LastFileName();
    fileDescription = LastFileDescription();
}

void UDXTextParser::SetPlayerName(const std::string& newPlayerName)
{
    LogUnimplemented("DeusExTextParser.SetPlayerName()");
}
