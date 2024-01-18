
#include "WidgetResourceData.h"
#include "File.h"
#include <miniz.h>
#include <stdexcept>

static mz_zip_archive widgetResources;

void InitWidgetResources()
{
	mz_bool result = mz_zip_reader_init_file(&widgetResources, FilePath::combine(OS::executable_path(), "SurrealEngine.pk3").c_str(), 0);
	if (!result)
		throw std::runtime_error("Could not open SurrealEngine.pk3");
}

void DeinitWidgetResources()
{
	mz_zip_reader_end(&widgetResources);
}

static std::vector<uint8_t> TryLoadWidgetFile(const std::string& name)
{
	mz_bool result;
	mz_zip_archive_file_stat stat;
	mz_uint32 fileIndex;
	
	result = mz_zip_reader_locate_file_v2(&widgetResources, name.c_str(), nullptr, 0, &fileIndex);
	if (!result)
		return {};

	result = mz_zip_reader_file_stat(&widgetResources, fileIndex, &stat);
	if (!result)
		return {};

	std::vector<uint8_t> buffer(stat.m_uncomp_size);
	result = mz_zip_reader_extract_file_to_mem(&widgetResources, name.c_str(), buffer.data(), buffer.size(), 0);
	if (!result)
		return {};

	return buffer;
}

std::vector<SingleFontData> LoadWidgetFontData(const std::string& name)
{
	std::vector<SingleFontData> fonts;
	fonts.resize(3);
	fonts[0].fontdata = TryLoadWidgetFile("noto/notosans-regular.ttf");
	fonts[1].fontdata = TryLoadWidgetFile("noto/notosansarmenian-regular.ttf");
	fonts[2].fontdata = TryLoadWidgetFile("noto/notosansgeorgian-regular.ttf");
	return fonts;
}

std::vector<uint8_t> LoadWidgetData(const std::string& name)
{
	return TryLoadWidgetFile(name);
}
