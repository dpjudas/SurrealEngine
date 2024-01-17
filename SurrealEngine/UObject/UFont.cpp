
#include "Precomp.h"
#include "UFont.h"
#include "UTexture.h"

void UFont::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	if (stream->GetVersion() <= 63)
	{
		pages.resize(1);

		// To do: how does the old font format look like?
	}
	else
	{
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
}

FontGlyph UFont::GetGlyph(char c) const
{
	FontGlyph glyph = FindGlyph(c);
	if (glyph.USize == 0)
	{
		if (c >= 'a' && c <= 'z')
		{
			c += 'A' - 'a';
			glyph = FindGlyph(c);
		}

		if (glyph.USize == 0)
			glyph = FindGlyph(32);
	}
	return glyph;
}

FontGlyph UFont::FindGlyph(char c) const
{
	size_t index = c;
	for (auto& page : pages)
	{
		if (index < page.Characters.size())
		{
			return { page.Texture, page.Characters[index] };
		}
		index -= page.Characters.size();
	}
	return {};
}
