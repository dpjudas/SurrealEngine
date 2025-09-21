#pragma once

#include "UTexture.h"

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
	Array<FontCharacter> Characters;
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

class UFont : public UTexture
{
public:
	using UTexture::UTexture;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	FontGlyph GetGlyph(char c) const;

private:
	FontGlyph FindGlyph(char c) const;

	Array<FontPage> pages;
	int charactersPerPage;
};
