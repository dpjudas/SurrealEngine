#include "UDXTextParser.h"
#include "UObject.h"
#include "Utils/Logger.h"

bool UDXTextParser::OpenText(NameString textName, std::string textPackage)
{
    LogUnimplemented("DeusExTextParser.OpenText()");
    return false;
}

void UDXTextParser::CloseText()
{
    Text() = 0;
    TextPos() = 0;
}

bool UDXTextParser::ProcessText()
{
    LogUnimplemented("DeusExTextParser.ProcessText()");
    return false;
}

bool UDXTextParser::IsEOF()
{
    if (Text() != 0 && TextPos() != 0)
        return false;
    return true;
}

std::string UDXTextParser::GetText()
{
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
    if (LastTag() == DeusExTextTags::TT_Font)
        NameString nameToReturn("");
        return nameToReturn;
    return LastName();
}

Color UDXTextParser::GetColor()
{
    if (LastTag() < DeusExTextTags::TT_DefaultColor) {
        int weirdFallbackColor = ((int)(ptrdiff_t)this) & 0xff000000;
        Color color;  
        color.A = (weirdFallbackColor >> 24) & 0xff;  // 255  
        color.R = (weirdFallbackColor >> 16) & 0xff;  // 0    
        color.G = (weirdFallbackColor >> 8) & 0xff;   // 0  
        color.B = weirdFallbackColor & 0xff; 
        return color;
    }

    else if (LastTag() > DeusExTextTags::TT_TextColor) {
        if (LastTag() == DeusExTextTags::TT_RevertColor) {
            return DefaultColor();
        }
        int weirdFallbackColor = ((int)(ptrdiff_t)this) & 0xff000000;
        Color color;  
        color.A = (weirdFallbackColor >> 24) & 0xff;  // 255  
        color.R = (weirdFallbackColor >> 16) & 0xff;  // 0    
        color.G = (weirdFallbackColor >> 8) & 0xff;   // 0  
        color.B = weirdFallbackColor & 0xff; 
        return color;
    }
    else {
        return LastColor();
    }
}

void UDXTextParser::GetEmailInfo(std::string& name, std::string& subject, std::string& from, std::string& to, std::string& cc)
{
    name = "";
    subject = "";
    from = "";
    to = "";
    cc = "";
    if (LastTag() == DeusExTextTags::TT_Email)
    {
        name = LastEmailName();
        subject = LastEmailSubject();
        from = LastEmailFrom();
        to = LastEmailTo();
        cc = LastEmailCC();
    }
}

void UDXTextParser::GetFileInfo(std::string& fileName, std::string& fileDescription)
{
    fileName = "";
    fileDescription = "";
    if (LastTag() == DeusExTextTags::TT_File)
    {
        fileName = LastFileName();
        fileDescription = LastFileDescription();
    }
}

void UDXTextParser::SetPlayerName(const std::string& newPlayerName)
{
    LogUnimplemented("DeusExTextParser.SetPlayerName()");
}
