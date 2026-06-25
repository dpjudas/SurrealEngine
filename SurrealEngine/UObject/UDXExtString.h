#pragma once

#include <string>

#include "UObject.h"

/// Deus Ex's String extension.
/// Used by DeusExTextParser
class UDXExtString : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;

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