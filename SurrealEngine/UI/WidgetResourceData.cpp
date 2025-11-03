
#include "WidgetResourceData.h"
#include "Utils/File.h"
#include <miniz.h>
#include "Utils/Exception.h"
#include <cstdlib>
#include <string>

class ResourceLoaderPK3 : public ResourceLoader
{
public:
	ResourceLoaderPK3()
	{
		const auto pk3PathStr = (fs::path{ OS::executable_path() } / "SurrealEngine.pk3").string();
		mz_bool result = mz_zip_reader_init_file(&widgetResources, pk3PathStr.c_str(), 0);
#ifndef WIN32
		// On Linux, SurrealEngine.pk3 can additionally be put in some other folders given below.
		if (!result)
			result = mz_zip_reader_init_file(&widgetResources, "/usr/share/surrealengine/SurrealEngine.pk3", 0);
		if (!result)
		{
			char* home = std::getenv("HOME");
			std::string directory = fs::path(home) / ".local/share/surrealengine";
			char* xdg_data_home = std::getenv("XDG_DATA_HOME");
			if (xdg_data_home != nullptr) {
				directory = fs::path(xdg_data_home) / "surrealengine";
			}
			result = mz_zip_reader_init_file(&widgetResources, (fs::path(directory) / "SurrealEngine.pk3").c_str(), 0);
		}
		if (!result)
		{
			char* home = std::getenv("HOME");
			const std::string directory = fs::path(home) / ".surrealengine";
			result = mz_zip_reader_init_file(&widgetResources, (fs::path(directory) / "SurrealEngine.pk3").c_str(), 0);
		}
#endif
		if (!result)
			Exception::Throw("Could not open SurrealEngine.pk3");
	}

	~ResourceLoaderPK3()
	{
		mz_zip_reader_end(&widgetResources);
	}

	std::vector<SingleFontData> LoadFont(const std::string& name) override
	{
		std::vector<SingleFontData> fonts;
		fonts.resize(3);
		TryLoadWidgetFile("noto/notosans-regular.ttf", fonts[0].fontdata);
		TryLoadWidgetFile("noto/notosansarmenian-regular.ttf", fonts[1].fontdata);
		TryLoadWidgetFile("noto/notosansgeorgian-regular.ttf", fonts[2].fontdata);
		return fonts;
	}

	std::vector<uint8_t> ReadAllBytes(const std::string& filename) override
	{
		std::vector<uint8_t> buffer;
		if (!TryLoadWidgetFile(filename, buffer))
			Exception::Throw("Could not load " + filename);
		return buffer;
	}

private:
	bool TryLoadWidgetFile(const std::string& name, std::vector<uint8_t>& buffer)
	{
		mz_bool result;
		mz_zip_archive_file_stat stat;
		mz_uint32 fileIndex;

		buffer.clear();

		result = mz_zip_reader_locate_file_v2(&widgetResources, name.c_str(), nullptr, 0, &fileIndex);
		if (!result)
			return false;

		result = mz_zip_reader_file_stat(&widgetResources, fileIndex, &stat);
		if (!result)
			return false;

		buffer.resize(stat.m_uncomp_size);
		result = mz_zip_reader_extract_file_to_mem(&widgetResources, name.c_str(), buffer.data(), buffer.size(), 0);
		if (!result)
			return false;

		return true;
	}

	mz_zip_archive widgetResources = {};
};


void InitWidgetResources()
{
	ResourceLoader::Set(std::make_unique<ResourceLoaderPK3>());
}

void DeinitWidgetResources()
{
	ResourceLoader::Set(nullptr);
}
