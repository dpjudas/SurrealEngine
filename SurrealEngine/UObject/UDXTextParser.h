#pragma once

#include "UObject.h"

enum class DeusExTextTags : uint8_t
{
    TT_Text,
    TT_File,
    TT_Email,
    TT_Note,
    TT_EndNote,
    TT_Goal,
    TT_EndGoal,
    TT_Comment,
    TT_EndComment,
    TT_PlayerName,
    TT_PlayerFirstName,
    TT_NewPage,
    TT_CenterText,
    TT_LeftJustify,
    TT_RightJustify,
    TT_DefaultColor,
    TT_TextColor,
    TT_RevertColor,
    TT_NewParagraph,
    TT_Bold,
    TT_EndBold,
    TT_Underline,
    TT_EndUnderline,
    TT_Italics,
    TT_EndItalics,
    TT_Graphic,
    TT_Font,
    TT_Label,
    TT_OpenBracket,
    TT_CloseBracket,
    TT_None
};

class UDXTextParser : public UObject
{
public:
    using UObject::UObject;

    int& Text() { return Value<int>(PropOffsets_DeusExTextParser.Text); }
    int& TextPos() { return Value<int>(PropOffsets_DeusExTextParser.textPos); }
    int& TagEndPos() { return Value<int>(PropOffsets_DeusExTextParser.tagEndPos); }

    std::string& LastText() { return Value<std::string>(PropOffsets_DeusExTextParser.lastText); }
    DeusExTextTags& LastTag() { return Value<DeusExTextTags>(PropOffsets_DeusExTextParser.lastTag); }
    NameString& LastName() { return Value<NameString>(PropOffsets_DeusExTextParser.lastName); }
    Color& LastColor() { return Value<Color>(PropOffsets_DeusExTextParser.lastColor); }
    Color& DefaultColor() { return Value<Color>(PropOffsets_DeusExTextParser.DefaultColor); }
    BitfieldBool bParagraphStarted() { return BoolValue(PropOffsets_DeusExTextParser.bParagraphStarted); }
    std::string& PlayerName() { return Value<std::string>(PropOffsets_DeusExTextParser.PlayerName); }
    std::string& PlayerFirstName() { return Value<std::string>(PropOffsets_DeusExTextParser.playerFirstName); }

    std::string& LastEmailName() { return Value<std::string>(PropOffsets_DeusExTextParser.lastEmailName); }
    std::string& LastEmailSubject() { return Value<std::string>(PropOffsets_DeusExTextParser.lastEmailSubject); }
    std::string& LastEmailFrom() { return Value<std::string>(PropOffsets_DeusExTextParser.lastEmailFrom); }
    std::string& LastEmailTo() { return Value<std::string>(PropOffsets_DeusExTextParser.lastEmailTo); }
    std::string& LastEmailCC() { return Value<std::string>(PropOffsets_DeusExTextParser.lastEmailCC); }
    std::string& LastFileName() { return Value<std::string>(PropOffsets_DeusExTextParser.lastFileName); }
    std::string& LastFileDescription() { return Value<std::string>(PropOffsets_DeusExTextParser.lastFileDescription); }

    bool OpenText(NameString textName, std::string textPackage = "");
    void CloseText();
    bool ProcessText();
    bool IsEOF();
    std::string GetText();
    void GotoLabel(const std::string& label);
    uint8_t GetTag();
    NameString GetName();
    Color GetColor();
    void GetEmailInfo(std::string& name, std::string& subject, std::string& from, std::string& to, std::string& cc);
    void GetFileInfo(std::string& fileName, std::string& fileDescription);
    void SetPlayerName(const std::string& newPlayerName);
};