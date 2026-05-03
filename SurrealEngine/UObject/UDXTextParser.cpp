
#include "Precomp.h"
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
	LogUnimplemented("DeusExTextParser.OpenText('" + textName.ToString() + "', '" + textPackage + "')");
	CloseText();
	if (textName.IsNone())
		textName = "DeusExQuotes"; // Not correct, but it handles empty strings just fine.
	if (textPackage.empty())
		textPackage = "DeusExText";
	UDXExtString* textObject = UObject::Cast<UDXExtString>(engine->packages->GetPackage(textPackage)->GetUObject("ExtString", textName));
	if (textObject)
	{
		const std::string sourceText = (textObject->Text() == "") ? "BOTTOM TEXT" : textObject->Text();
		LastText() = sourceText;
		TextPos() = 0;
		Text() = 1;
	}
	return textObject;
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
	if (LastTag() == DeusExTextTags::TT_Font)
		return {};
	return LastName();
}

Color UDXTextParser::GetColor()
{
	if (LastTag() < DeusExTextTags::TT_DefaultColor)
	{
		Color color;  
		color.A = 255;
		color.R = 255;
		color.G = 127;
		color.B = 127; 
		return color;
	}
	else if (LastTag() == DeusExTextTags::TT_RevertColor)
	{
		return DefaultColor();
	}
	else if (LastTag() > DeusExTextTags::TT_TextColor)
	{
		Color color;  
		color.A = 255;
		color.R = 127;
		color.G = 127;
		color.B = 255; 
		return color;
	}
	else
	{
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
