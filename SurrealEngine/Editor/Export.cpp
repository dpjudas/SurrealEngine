#include "Editor/Export.h"
#include "UObject/UProperty.h"
#include "UObject/UTextBuffer.h"

static void UnpackRgba(uint32_t rgba, uint32_t& r, uint32_t& g, uint32_t& b, uint32_t& a)
{
	r = rgba & 0xff;
	g = (rgba >> 8) & 0xff;
	b = (rgba >> 16) & 0xff;
	a = (rgba >> 24) & 0xff;
}

/////////////////////////////////////////////////////////////////////////////

std::string Exporter::ExportObject(UObject* obj, int tablevel, bool bInline)
{
	std::string txt = "";
	for (UProperty* prop : obj->Class->Properties)
	{
		for (int i = 0; i < prop->ArrayDimension; i++)
		{
			if (AnyFlags(prop->Flags, ObjectFlags::TagExp))
			{
				// Get default property from super class
				UObject* defobj = nullptr;
				if (obj->Class == obj->Class->Class)
					defobj = obj->Class->BaseStruct;
				else
					defobj = obj->Class;

				if (prop->Name == "Tag")
				{
					if (obj->Class->GetPropertyAsString(prop->Name) == obj->Name.ToString())
						continue;
				}

				// TODO: implement inline declared objects here
				// not necessary until we support <= 227j

				std::string tabs(tablevel, '\t');
				prop->GetExportText(txt, tabs, obj, defobj, i);
			}
		}
	}

	return txt;
}

/////////////////////////////////////////////////////////////////////////////

MemoryStreamWriter Exporter::ExportClass(UClass* cls)
{
	MemoryStreamWriter text;
	if (!cls->ScriptText)
		return text;

	text << cls->ScriptText->Text;
	text << "\r\ndefaultproperties\r\n{\r\n";
	text << ExportObject(cls->GetDefaultObject(), 1, false);
	return text;
}

/////////////////////////////////////////////////////////////////////////////

MemoryStreamWriter Exporter::ExportTexture(UTexture* tex, const std::string& ext)
{
	if (!tex)
		return MemoryStreamWriter();

	const std::string& className = tex->Class->Name.ToString();
	if (className.compare("FireTexture") == 0)
	{
		return ExportFireTexture(reinterpret_cast<UFireTexture*>(tex));
	}
	else if (className.compare("WetTexture") == 0)
	{
		return ExportWetTexture(reinterpret_cast<UWetTexture*>(tex));
	}
	else if (className.compare("WaveTexture") == 0)
	{
		return ExportWaveTexture(reinterpret_cast<UWaveTexture*>(tex));
	}
	else if (className.compare("IceTexture") == 0)
	{
		return ExportIceTexture(reinterpret_cast<UIceTexture*>(tex));
	}

	if (tex->ActualFormat == TextureFormat::P8)
	{
		if (ext.compare("bmp") == 0)
			return ExportBmpIndexed(tex);

		Exception::Throw("Unknown texture export format: " + ext);
	}

	std::vector<Color> pixels;
	switch (tex->ActualFormat)
	{
	default:
		Exception::Throw("Unsupported texture format (" + std::to_string(static_cast<int>(tex->ActualFormat)) + ")");
	}
	
	Exception::Throw("Unknown texture export format: " + ext);
}

/////////////////////////////////////////////////////////////////////////////

MemoryStreamWriter Exporter::ExportFireTexture(UFireTexture* tex)
{
	MemoryStreamWriter data;
	data << "BEGIN OBJECT CLASS=FireTexture USIZE=" << std::to_string(tex->USize()) << " VSIZE=" << std::to_string(tex->VSize()) << "\r\n";

	data << ExportObject(tex, 1, false);

	// 5-color interpolated palette, this is what 227/469 UnrealEd recognizes
	UPalette* palette = tex->Palette();
	for (int i = 0; i < 5; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[(i * 64) - 1], r, g, b, a);

		data << "\tColor" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	// Export full color palette as well
	for (int i = 0; i < 256; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[i], r, g, b, a);

		data << "\tPaletteColor" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	data << "END OBJECT\r\n";
	return data;
}

MemoryStreamWriter Exporter::ExportWaveTexture(UWaveTexture* tex)
{
	MemoryStreamWriter data;
	data << "BEGIN OBJECT CLASS=WaveTexture USIZE=" + std::to_string(tex->USize()) + " VSIZE=" + std::to_string(tex->VSize()) + "\r\n";

	data << ExportObject(tex, 1, false);

	// 5-color interpolated palette, this is what 227/469 UnrealEd recognizes
	UPalette* palette = tex->Palette();
	for (int i = 0; i < 5; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[(i * 64) - 1], r, g, b, a);

		data << "\tColor" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	// Export full color palette as well
	for (int i = 0; i < 256; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[i], r, g, b, a);

		data << "\tPalette" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	data << "END OBJECT\r\n";
	return data;
}

MemoryStreamWriter Exporter::ExportWetTexture(UWetTexture* tex)
{
	MemoryStreamWriter data;
	data << "BEGIN OBJECT CLASS=WetTexture USIZE=" + std::to_string(tex->USize()) + " VSIZE=" + std::to_string(tex->VSize()) + "\r\n";

	data << ExportObject(tex, 1, false);

	// 5-color interpolated palette, this is what 227/469 UnrealEd recognizes
	UPalette* palette = tex->Palette();
	for (int i = 0; i < 5; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[(i * 64) - 1], r, g, b, a);

		data << "\tColor" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	// Export full color palette as well
	for (int i = 0; i < 256; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[i], r, g, b, a);

		data << "\tPalette" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	data << "END OBJECT\r\n";
	return data;
}

MemoryStreamWriter Exporter::ExportIceTexture(UIceTexture* tex)
{
	MemoryStreamWriter data;
	data << "BEGIN OBJECT CLASS=IceTexture USIZE=" << std::to_string(tex->USize()) << " VSIZE=" << std::to_string(tex->VSize()) << "\r\n";

	data << ExportObject(tex, 1, false);

	// 5-color interpolated palette, this is what 227/469 UnrealEd recognizes
	UPalette* palette = tex->Palette();
	for (int i = 0; i < 5; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[(i * 64) - 1], r, g, b, a);

		data << "\tColor" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	// Export full color palette as well
	for (int i = 0; i < 256; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[i], r, g, b, a);

		data << "\tPalette" << std::to_string(i + 1);
		data << "=(R=" << std::to_string(r);
		data << ",G=" << std::to_string(g);
		data << ",B=" << std::to_string(b);
		data << ",A=" << std::to_string(a) + ")\r\n";
	}

	data << "END OBJECT\r\n";
	return data;
}

/////////////////////////////////////////////////////////////////////////////

struct BmpHeader
{
	uint16_t signature;
	uint32_t fileSize;
	uint32_t reserved;
	uint32_t pixelOffset;

	uint32_t dibHeaderSize;
	uint32_t imageWidth;
	uint32_t imageHeight;
	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression;
	uint32_t imageSize;
	uint32_t pixelsPerMeterX;
	uint32_t pixelsPerMeterY;
	uint32_t colorsUsed;
	uint32_t colorsImportant;
};

MemoryStreamWriter& operator<<(MemoryStreamWriter& s, BmpHeader& bmp)
{
	s << bmp.signature;
	s << bmp.fileSize;
	s << bmp.reserved;
	s << bmp.pixelOffset;

	s << bmp.dibHeaderSize;
	s << bmp.imageWidth;
	s << bmp.imageHeight;
	s << bmp.planes;
	s << bmp.bitCount;
	s << bmp.compression;
	s << bmp.imageSize;
	s << bmp.pixelsPerMeterX;
	s << bmp.pixelsPerMeterY;
	s << bmp.colorsUsed;
	s << bmp.colorsImportant;
	return s;
}

MemoryStreamWriter Exporter::ExportBmpIndexed(UTexture* tex)
{
	MemoryStreamWriter data;
	BmpHeader hdr = { 0 };
	int usize = tex->USize();
	int vsize = tex->VSize();

	if (usize > 8192 || vsize > 8192)
	{
		Exception::Throw("Abnormally large indexed texture: " + std::to_string(usize) + "x" + std::to_string(vsize));
	}

	data.Reserve(sizeof(hdr) + (4 * 256) + (usize * vsize));

	hdr.signature = 0x4d42;
	hdr.fileSize = 0; // re-fill later
	hdr.pixelOffset = sizeof(BmpHeader) + (4 * 256);
	hdr.dibHeaderSize = 40;
	hdr.imageWidth = tex->USize();
	hdr.imageHeight = tex->VSize();
	hdr.planes = 1;
	hdr.bitCount = 8;

	data << hdr;

	UPalette* palette = tex->Palette();

	// Assuming 8-bit color palette
	for (int i = 0; i < 256; i++)
	{
		uint32_t r, g, b, a;
		UnpackRgba(palette->Colors[i], r, g, b, a);

		uint32_t bgra = (b) | (g << 8) | (r << 16) | (a << 24);
		data << bgra;
	}

	hdr.pixelOffset = data.Tell();

	uint8_t *pixels = tex->Mipmaps[0].Data.data();
	for (int y = vsize; y > 0; y--)
	{
		for (int x = 0; x < usize; x++)
		{
			data << pixels[((y - 1) * usize) + x];
		}
	}

	hdr.fileSize = static_cast<uint32_t>(data.Tell());
	data.Seek(0, SEEK_SET);
	data << hdr;

	return data;
}