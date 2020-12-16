
#include "Precomp.h"
#include "UFont.h"
#include "UTexture.h"

void UFont::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	pages.resize(stream->ReadIndex());
	for (FontPage& page : pages)
	{
		page.Texture = stream->ReadObject<UTexture>();
		page.Characters.resize(stream->ReadIndex());
		for (FontCharacter& character : page.Characters)
		{
			character.StartU = stream->ReadInt32();
			character.StartV = stream->ReadInt32();
			character.USize = stream->ReadInt32();
			character.VSize = stream->ReadInt32();
		}
	}

	charactersPerPage = stream->ReadUInt32();
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
