
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
	else if (clsname == "FractalTexture" || clsname == "FireTexture" || clsname == "IceTexture" || clsname == "WaterTexture" || clsname == "WaveTexture" || clsname == "ScriptedTexture")
	{
		auto obj = entry->Open();
		if (obj)
		{
			auto& stream = obj->Stream;

			auto& textureslot = Textures[entry];
			textureslot = std::make_unique<UnrealTexture>();
			texture = textureslot.get();
			texture->Properties = obj->Properties;
			texture->Format = TEXF_P8;
			texture->FireTexture = true;
			texture->Mipmaps.resize(1);

			int width = 128;
			int height = 128;

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
				else if (prop.Name == "UClamp")
				{
					width = prop.Scalar.ValueInt;
				}
				else if (prop.Name == "VClamp")
				{
					height = prop.Scalar.ValueInt;
				}
			}

			UnrealMipmap& mipmap = texture->Mipmaps.front();
			mipmap.Width = width;
			mipmap.Height = height;
			mipmap.Data.resize((size_t)mipmap.Width* mipmap.Height);
			uint8_t* pixels = (uint8_t*)mipmap.Data.data();
			memset(pixels, 0, (size_t)width* height);
		}

		return nullptr;
	}
	else
	{
		return nullptr;
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

void TextureManager::Update()
{
	for (auto& textureslot : Textures)
	{
		if (textureslot.second->FireTexture && !textureslot.second->TextureModified)
		{
			UnrealMipmap& mipmap = textureslot.second->Mipmaps.front();

			int width = mipmap.Width;
			int height = mipmap.Height;
			uint8_t* pixels = (uint8_t*)mipmap.Data.data();

			uint8_t* line = pixels + (size_t)width * (height - 1);
			for (int x = 0; x < width; x++)
			{
				line[x] = (uint8_t)clamp(std::max(std::cos(radians(90.0f + (x - width / 2) * 200.0f / width)), 0.0f) * 100 + rand() * 155 / RAND_MAX, 0.0f, 255.0f);
			}

			uint8_t* src = pixels;
			for (int y = 0; y < height - 1; y++)
			{
				uint8_t* dest = src;
				src += width;

				for (int x = 0; x < width; x++)
				{
					int value = src[x];
					for (int i = -3; i < 3; i++)
					{
						int xx = (x + i) % width;
						if (xx < 0) xx += width;
						value += src[xx];
					}
					value -= 8;
					value = value / 7;
					if (value < 0) value = 0;
					dest[x] = value;
				}
			}

			textureslot.second->TextureModified = true;
		}
	}
}