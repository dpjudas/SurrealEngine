
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
	CloseText();
	if (textName.IsNone())
		textName = "DeusExQuotes"; // Not correct, but it handles empty strings just fine.
	if (textPackage.empty())
		textPackage = "DeusExText";
	textObject = UObject::Cast<UDXExtString>(engine->packages->GetPackage(textPackage)->GetUObject("ExtString", textName));
	Text() = textObject ? 1 : 0;
	return textObject;
}

void UDXTextParser::CloseText()
{
	textObject = nullptr;
	Text() = 0;
	TextPos() = 0;
	TagEndPos() = 0;
}

bool UDXTextParser::ProcessText()
{
	if (!textObject)
		return false;

	const std::string& sourceText = textObject->Text();
	if (TextPos() >= (int)sourceText.size())
		return false;

	// To do: this is a tokenizer function. It reads one tag at a time and sets the LastXX properties to what it found

	/*
		<DC=255,255,255>
		<P>We'll make this one easy for you.  To open the door, use the code:
		<P>
		<P><JC><B>0012</B>
		<P>
		<P>Got it?
		<P>
		<P>Jaime
	*/

	LastTag() = DeusExTextTags::TT_Text;
	LastText() = sourceText;
	TextPos() = (int)sourceText.size();
	return true;
}

bool UDXTextParser::IsEOF()
{
	return !textObject || TextPos() == textObject->Text().size();
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
	PlayerName() = newPlayerName;
}
