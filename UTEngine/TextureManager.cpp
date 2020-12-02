
#include "Precomp.h"
#include "TextureManager.h"
#include "PackageManager.h"
#include "BinaryStream.h"

TextureManager::TextureManager(PackageManager* packages) : Packages(packages)
{
}

UnrealTexture* TextureManager::GetTexture(ExportTableEntry* entry)
{
	if (!entry)
		return nullptr;

	auto it = Textures.find(entry);
	if (it != Textures.end())
		return it->second.get();

	UnrealTexture* texture = nullptr;

	std::string clsname = entry->GetClsName();
	if (clsname == "Texture")
	{
		auto obj = entry->Open();
		if (obj)
		{
			auto& stream = obj->Stream;

			auto& textureslot = Textures[entry];
			textureslot = std::make_unique<UnrealTexture>();
			texture = textureslot.get();
			texture->Properties = obj->Properties;

			for (const UnrealProperty& prop : texture->Properties)
			{
				if (prop.Name == "CompFormat")
				{
					texture->Format = (ETextureFormat)prop.Scalar.ValueByte;
					break;
				}
			}

			for (const UnrealProperty& prop : texture->Properties)
			{
				if (prop.Name == "Format")
				{
					texture->Format = (ETextureFormat)prop.Scalar.ValueByte;
					break;
				}
			}

			int mipsCount = stream->ReadIndex();
			texture->Mipmaps.resize(mipsCount);

			for (UnrealMipmap& mipmap : texture->Mipmaps)
			{
				uint32_t unknown = 0;
				if (obj->Package->GetVersion() >= 68)
					unknown = stream->ReadInt32();
				int bytes = stream->ReadIndex();
				mipmap.Data.resize(bytes);
				stream->ReadBytes(mipmap.Data.data(), bytes);
				mipmap.Width = stream->ReadUInt32();
				mipmap.Height = stream->ReadUInt32();
				uint8_t UBits = stream->ReadUInt8();
				uint8_t VBits = stream->ReadUInt8();
			}

			for (auto& prop : texture->Properties)
			{
				if (prop.Name == "Palette")
				{
					texture->Palette = GetPalette(entry->Owner->FindExportObject(prop.Scalar.ValueInt));
				}
				else if (prop.Name == "DrawScale")
				{
					texture->DrawScale = prop.Scalar.ValueFloat;
				}
				else if (prop.Name == "DetailTexture")
				{
					texture->DetailTexture = GetTexture(obj->Package->FindExportObject(prop.Scalar.ValueObject));
				}
				else if (prop.Name == "MacroTexture")
				{
					texture->MacroTexture = GetTexture(obj->Package->FindExportObject(prop.Scalar.ValueObject));
				}
			}
		}
	}

	return texture;
}

UnrealPalette* TextureManager::GetPalette(ExportTableEntry* entry)
{
	auto it = Palettes.find(entry);
	if (it != Palettes.end())
		return it->second.get();

	auto& palette = Palettes[entry];

	if (entry->GetClsName() == "Palette")
	{
		auto obj = entry->Open();
		auto& stream = obj->Stream;

		palette = std::make_unique<UnrealPalette>();
		int count = stream->ReadIndex();
		palette->Colors.resize(count);
		stream->ReadBytes(palette->Colors.data(), count * 4);
	}

	return palette.get();
}
