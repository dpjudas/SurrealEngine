
#include "Precomp.h"
#include "UFont.h"
#include "UTexture.h"

UFont::UFont(ObjectStream* stream) : UObject(stream)
{
	pages.resize(stream->ReadIndex());
	for (FontPage& page : pages)
	{
		page.Texture = Cast<UTexture>(stream->ReadUObject());
		page.Characters.resize(stream->ReadIndex());
		for (FontCharacter& character : page.Characters)
		{
			character.StartU = stream->ReadInt32();
			character.StartV = stream->ReadInt32();
			character.USize = stream->ReadInt32();
			character.VSize = stream->ReadInt32();
		}
	}
}

FontCharacter UFont::GetGlyph(char c) const
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
