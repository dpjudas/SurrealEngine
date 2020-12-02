
#include "Precomp.h"
#include "Font.h"
#include "PackageManager.h"
#include "Package.h"
#include "TextureManager.h"
#include "BinaryStream.h"

Font::Font(std::string fontname, PackageManager* packages, TextureManager* textures)
{
	Package* engine = packages->GetPackage("Engine");
	auto entry = engine->FindExportObject("Font", fontname);
	if (entry)
	{
		auto obj = entry->Open();

		pages.resize(obj->Stream->ReadIndex());
		for (FontPage& page : pages)
		{
			page.Texture = textures->GetTexture(engine->FindExportObject(obj->Stream->ReadIndex()));
			page.Characters.resize(obj->Stream->ReadIndex());
			for (FontCharacter& character : page.Characters)
			{
				character.StartU = obj->Stream->ReadInt32();
				character.StartV = obj->Stream->ReadInt32();
				character.USize = obj->Stream->ReadInt32();
				character.VSize = obj->Stream->ReadInt32();
			}
		}
	}
}

FontCharacter Font::GetGlyph(char c) const
{
	FontCharacter glyph = pages.front().Characters[(uint8_t)c];
	if (glyph.USize == 0)
	{
		if (c >= 'a' && c <= 'z')
		{
			c += 'A' - 'a';
			glyph = pages.front().Characters[(uint8_t)c];
		}

		if (glyph.USize == 0)
			glyph = pages.front().Characters[32];
	}
	return glyph;
}
