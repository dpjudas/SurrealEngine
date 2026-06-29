
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
	/*
		Tokenizer function reading pseudo HTML text ala this:

		<DC=255,255,255>
		<P>We'll make this one easy for you.  To open the door, use the code:
		<P>
		<P><JC><B>0012</B>
		<P>
		<P>Got it?
		<P>
		<P>Jaime
	*/

	if (!textObject)
		return false;

	const std::string& text = textObject->Text();
	size_t pos = (size_t)TextPos();
	size_t len = text.size();

	// Eat whitespace
	EatWhitespace(text, pos);

	if (pos >= len) // EOF
	{
		TextPos() = (int)pos;
		return false;
	}

	size_t tagstart = pos;
	if (!ReadChars(text, pos, "<"))
	{
		if (ReadText(text, pos, LastText()))
		{
			LastTag() = DeusExTextTags::TT_Text;
			TextPos() = (int)pos;
			return true;
		}
	}
	else
	{
		std::string tagname;
		if (ReadTagName(text, pos, tagname))
		{
			if (tagname == "DC" && ReadTagColor(text, pos, DefaultColor()) && ReadChars(text, pos, ">"))
			{
				LastColor() = DefaultColor();
				LastTag() = DeusExTextTags::TT_DefaultColor;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "P" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_NewParagraph;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "B" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_Bold;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "/B" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_EndBold;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "I" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_Italics;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "/I" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_EndItalics;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "U" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_Underline;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "/U" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_EndUnderline;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "JC" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_None; // What does <JC> mean? Seems to be used for headlines
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "JR" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_None; // What does <JR> mean? Seems to be used for signatures
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "PLAYERNAME" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_PlayerName;
				LastText() = PlayerName();
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "PLAYERFIRSTNAME" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_PlayerFirstName;
				LastText() = PlayerFirstName();
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "COMMENT" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_Comment;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "/COMMENT" && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_EndComment;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "FILE" && ReadTagFile(text, pos, LastFileName(), LastFileDescription()) && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_File;
				TextPos() = (int)pos;
				return true;
			}
			else if (tagname == "EMAIL" && ReadTagEmail(text, pos, LastEmailName(), LastEmailSubject(), LastEmailFrom(), LastEmailTo(), LastEmailCC()) && ReadChars(text, pos, ">"))
			{
				LastTag() = DeusExTextTags::TT_File;
				TextPos() = (int)pos;
				return true;
			}
		}
	}

	// Parse error. Dump what we got as text for debugging
	LastTag() = DeusExTextTags::TT_Text;
	LastText() = text.substr(tagstart);
	TextPos() = (int)len;
	return true;
}

void UDXTextParser::EatWhitespace(const std::string& text, size_t& pos)
{
	size_t len = text.size();
	while (pos < len && (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\r' || text[pos] == '\n'))
		pos++;
}

bool UDXTextParser::ReadTagName(const std::string& text, size_t& pos, std::string& tagname)
{
	tagname.clear();

	size_t len = text.size();
	while (pos < len && ((text[pos] >= 'a' && text[pos] <= 'z') || (text[pos] >= 'A' && text[pos] <= 'Z') || text[pos] == '/'))
	{
		tagname += text[pos];
		pos++;
	}

	if (tagname.empty() || pos == len)
		return false;
	return true;
}

bool UDXTextParser::ReadChars(const std::string& text, size_t& pos, const std::string& chars)
{
	if (pos + chars.size() >= text.size() || text.substr(pos, chars.size()) != chars)
		return false;
	pos += chars.size();
	return true;
}

bool UDXTextParser::ReadTagColor(const std::string& text, size_t& pos, Color& color)
{
	if (!ReadChars(text, pos, "="))
		return false;
	if (!ReadInteger(text, pos, color.R))
		return false;
	if (!ReadChars(text, pos, ","))
		return false;
	if (!ReadInteger(text, pos, color.G))
		return false;
	if (!ReadChars(text, pos, ","))
		return false;
	if (!ReadInteger(text, pos, color.B))
		return false;
	color.A = 255;
	return true;
}

bool UDXTextParser::ReadInteger(const std::string& text, size_t& pos, int value)
{
	std::string v;
	size_t len = text.size();
	while (pos < len && text[pos] >= '0' && text[pos] <= '9')
	{
		v += text[pos];
		pos++;
	}
	if (v.empty())
		return false;
	value = std::atoi(v.c_str());
	return true;
}

bool UDXTextParser::ReadTextUntil(const std::string& text, size_t& pos, std::string& value, char endChar)
{
	value.clear();
	size_t len = text.size();
	while (pos < len && text[pos] != endChar)
	{
		value += text[pos];
		pos++;
	}
	return pos != len;
}

bool UDXTextParser::ReadTagFile(const std::string& text, size_t& pos, std::string& filename, std::string& filedescription)
{
	if (!ReadTextUntil(text, pos, filename, ',') || !ReadChars(text, pos, ","))
		return false;
	return ReadTextUntil(text, pos, filedescription, '>');
}

bool UDXTextParser::ReadTagEmail(const std::string& text, size_t& pos, std::string& emailName, std::string& emailSubject, std::string& emailFrom, std::string& emailTo, std::string& emailCC)
{
	if (!ReadTextUntil(text, pos, emailName, ',') || !ReadChars(text, pos, ","))
		return false;
	if (!ReadTextUntil(text, pos, emailSubject, ',') || !ReadChars(text, pos, ","))
		return false;
	if (!ReadTextUntil(text, pos, emailFrom, ',') || !ReadChars(text, pos, ","))
		return false;
	if (!ReadTextUntil(text, pos, emailTo, ',') || !ReadChars(text, pos, ","))
		return false;
	return ReadTextUntil(text, pos, emailCC, '>');
}

bool UDXTextParser::ReadText(const std::string& text, size_t& pos, std::string& value)
{
	EatWhitespace(text, pos);

	value.clear();
	size_t len = text.size();
	while (pos < len && text[pos] != '<')
	{
		value += text[pos];
		pos++;
	}

	// Eat any whitespace at the end
	while (!value.empty() && (value.back() == ' ' || value.back() == '\r' || value.back() == '\n'))
		value.pop_back();

	if (value.empty() || pos == len)
		return false;
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
