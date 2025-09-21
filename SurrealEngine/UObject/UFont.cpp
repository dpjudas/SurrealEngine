
#include "Precomp.h"
#include "UFont.h"
#include "UTexture.h"

void UFont::Load(ObjectStream* stream)
{
	// Note: 63 package and older inherited from UTexture. Newer versions inherit from UObject
	// We must keep the old relationship intact to load the older games.
	// If the game is newer then this is not a valid texture and any properties on UTexture do not exist.

	if (stream->GetVersion() <= 63)
	{
		UTexture::Load(stream);

		pages.resize(1);
		FontPage& page = pages.front();
		page.Texture = this;
		page.Characters.resize(stream->ReadIndex());
		for (FontCharacter& character : page.Characters)
		{
			character.StartU = stream->ReadInt32();
			character.StartV = stream->ReadInt32();
			character.USize = stream->ReadInt32();
			character.VSize = stream->ReadInt32();
		}
	}
	else
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
}

void UFont::Save(PackageStreamWriter* stream)
{
	if (stream->GetVersion() <= 63)
	{
		UTexture::Save(stream);
	}
	else
	{
		UObject::Save(stream);
	}
	Exception::Throw("UFont::Save not implemented");
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
