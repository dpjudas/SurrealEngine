#pragma once

class PackageManager;
class TextureManager;
class UnrealTexture;

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
	UnrealTexture* Texture = nullptr;
	std::vector<FontCharacter> Characters;
};

class Font
{
public:
	Font(std::string fontname, PackageManager* packages, TextureManager* textures);

	FontCharacter GetGlyph(char c) const;

	std::vector<FontPage> pages;
};
