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

class FontGlyph
{
public:
	FontGlyph() = default;
	FontGlyph(UTexture* texture, const FontCharacter& c) : Texture(texture), StartU(c.StartU), StartV(c.StartV), USize(c.USize), VSize(c.VSize) { }

	UTexture* Texture = nullptr;
	int StartU = 0;
	int StartV = 0;
	int USize = 0;
	int VSize = 0;
};

class UFont : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	FontGlyph GetGlyph(char c) const;

private:
	FontGlyph FindGlyph(char c) const;

	std::vector<FontPage> pages;
	int charactersPerPage;
};
