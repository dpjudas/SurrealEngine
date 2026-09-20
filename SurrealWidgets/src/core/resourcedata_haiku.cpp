#include "surrealwidgets/core/resourcedata.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>

// Haiku has no GSettings/GTK (used by resourcedata_unix.cpp to find the desktop's
// UI font) and no fontconfig guaranteed to be installed. Rather than depend on
// either, just look for one of Haiku's own bundled fonts directly.

static std::vector<uint8_t> ReadAllBytes(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file)
		throw std::runtime_error("Could not open: " + filename);

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
		throw std::runtime_error("Could not read: " + filename);

	return buffer;
}

static std::vector<SingleFontData> LoadFirstAvailableFont(const std::vector<std::string>& candidateNames)
{
	static const char* fontDirs[] = {
		"/boot/system/data/fonts",
		"/boot/system/non-packaged/data/fonts",
		"/boot/home/config/non-packaged/data/fonts"
	};

	for (const char* dir : fontDirs)
	{
		for (const std::string& name : candidateNames)
		{
			std::string path = std::string(dir) + "/" + name;
			std::ifstream test(path, std::ios::binary);
			if (test)
				return { SingleFontData{ ReadAllBytes(path), "" } };
		}
	}

	throw std::runtime_error("Could not find a usable system font on this Haiku install");
}

std::vector<SingleFontData> ResourceData::LoadSystemFont()
{
	return LoadFirstAvailableFont({ "NotoSans-Regular.ttf", "DejaVuSans.ttf" });
}

std::vector<SingleFontData> ResourceData::LoadMonospaceSystemFont()
{
	return LoadFirstAvailableFont({ "NotoSansMono-Regular.ttf", "DejaVuSansMono.ttf" });
}

double ResourceData::GetSystemFontSize()
{
	return 11.0;
}

class ResourceLoaderHaiku : public ResourceLoader
{
public:
	std::vector<SingleFontData> LoadFont(const std::string& name) override
	{
		if (name == "system")
			return ResourceData::LoadSystemFont();
		else if (name == "monospace")
			return ResourceData::LoadMonospaceSystemFont();
		else
			return { SingleFontData{ReadAllBytes(name + ".ttf"), ""} };
	}

	std::vector<uint8_t> ReadAllBytes(const std::string& filename) override
	{
		return ::ReadAllBytes(filename);
	}
};

struct ResourceDefaultLoader
{
	ResourceDefaultLoader() { loader = std::make_unique<ResourceLoaderHaiku>(); }
	std::unique_ptr<ResourceLoader> loader;
};

static std::unique_ptr<ResourceLoader>& GetLoader()
{
	static ResourceDefaultLoader loader;
	return loader.loader;
}

ResourceLoader* ResourceLoader::Get()
{
	return GetLoader().get();
}

void ResourceLoader::Set(std::unique_ptr<ResourceLoader> instance)
{
	GetLoader() = std::move(instance);
}
