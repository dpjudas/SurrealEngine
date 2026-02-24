#pragma once

#include <string>

#include "UObject.h"

/// Deus Ex's String extension.
/// Doesn't seem to be used anywhere.
class UDXExtString : public UObject
{
public:
    using UObject::UObject;

    std::string& Text() { return Value<std::string>(PropOffsets_ExtString.Text); }
    int& SpeechPage() { return Value<int>(PropOffsets_ExtString.speechPage); }

    void AppendText(const std::string& newText);
    int GetFirstTextPart(std::string& outText);
    int GetNextTextPart(std::string& outText);
    std::string& GetText();
    int GetTextLength();
    int GetTextPart(int startPos, int count, std::string& outText);
    void SetText(const std::string& newText);
};