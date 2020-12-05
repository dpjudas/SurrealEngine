#pragma once

#include "UObject.h"

class UTexture;

class FontCharacter
{
public:
	int StartU = 0;
	int StartV = 0;
	int USize = 0;
	int VSize = 0;
};

class FontPage
{
public:
	UTexture* Texture = nullptr;
	std::vector<FontCharacter> Characters;
};

class UFont : public UObject
{
public:
	UFont(ObjectStream* stream);

	FontCharacter GetGlyph(char c) const;

	std::vector<FontPage> pages;
};
