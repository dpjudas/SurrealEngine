
#include "WidgetResourceData.h"
#include "Utils/File.h"
#include <miniz.h>
#include "Utils/Exception.h"
#include <cstdlib>
#include <string>

static mz_zip_archive widgetResources;

void InitWidgetResources()
{
	mz_bool result = mz_zip_reader_init_file(&widgetResources, FilePath::combine(OS::executable_path(), "SurrealEngine.pk3").c_str(), 0);
#ifndef WIN32
	// On Linux, SurrealEngine.pk3 can additionally be put in some other folders given below.
	if (!result)
		result = mz_zip_reader_init_file(&widgetResources, FilePath::combine("/usr/share/surrealengine", "SurrealEngine.pk3").c_str(), 0);
	if (!result) {
		char * home = std::getenv("HOME");
		std::string directory = FilePath::combine(std::string(home) , ".local/share/surrealengine");
		char * xdg_data_home = std::getenv("XDG_DATA_HOME");
		if (xdg_data_home != NULL) {
			directory = FilePath::combine(std::string(xdg_data_home), "surrealengine");
		}
		result = mz_zip_reader_init_file(&widgetResources, FilePath::combine(directory, "SurrealEngine.pk3").c_str(), 0);
	}
	if (!result) {
		char * home = std::getenv("HOME");
		std::string directory = FilePath::combine(std::string(home) , ".surrealengine");
		result = mz_zip_reader_init_file(&widgetResources, FilePath::combine(directory, "SurrealEngine.pk3").c_str(), 0);
	}
#endif
	if (!result)
		Exception::Throw("Could not open SurrealEngine.pk3");
}

void DeinitWidgetResources()
{
	mz_zip_reader_end(&widgetResources);
}

static bool TryLoadWidgetFile(const std::string& name, std::vector<uint8_t>& buffer)
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

std::vector<SingleFontData> LoadWidgetFontData(const std::string& name)
{
	std::vector<SingleFontData> fonts;
	fonts.resize(3);
	TryLoadWidgetFile("noto/notosans-regular.ttf", fonts[0].fontdata);
	TryLoadWidgetFile("noto/notosansarmenian-regular.ttf", fonts[1].fontdata);
	TryLoadWidgetFile("noto/notosansgeorgian-regular.ttf", fonts[2].fontdata);
	return fonts;
}

std::vector<uint8_t> LoadWidgetData(const std::string& name)
{
	std::vector<uint8_t> buffer;
	if (!TryLoadWidgetFile(name, buffer))
		Exception::Throw("Could not load " + name);
	return buffer;
}
