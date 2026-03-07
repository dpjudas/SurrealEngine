#include "UDXTextParser.h"
#include "UDXExtString.h"
#include "UObject.h"
#include "../Package/Package.h"
#include "../Package/PackageManager.h"
#include "../Engine.h"
#include "Utils/Logger.h"

bool UDXTextParser::OpenText(NameString textName, std::string textPackage)
{
    // Unfinished and probably won't work.
    LogUnimplemented("DeusExTextParser.OpenText()");
    CloseText();
    if (textName.IsNone())
        textName = NameString("DeusExQuotes"); // Not correct, but it handles empty strings just fine.
    NameString fullPath(textPackage + textName.ToString());
    UDXExtString* textObject = UObject::Cast<UDXExtString>(engine->packages->GetPackage(textPackage)->GetUObject(NameString("ExtString"), textName));
    if (textObject)
    {
        const std::string sourceText = (textObject->Text() == "") ? "BOTTOM TEXT" : textObject->Text();
        LastText() = sourceText;
        TextPos() = 0;
        Text() = 1;
    }
    return textObject != nullptr;
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
    return static_cast<uint8_t>(LastTag());
}

NameString UDXTextParser::GetName()
{
    NameString nameToReturn("");
    if (LastTag() == DeusExTextTags::TT_Font)
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
