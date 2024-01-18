
#include "WidgetResourceData.h"
#include "File.h"

std::vector<SingleFontData> LoadWidgetFontData(const std::string& name)
{
	std::vector<SingleFontData> fonts;
	SingleFontData font;
	font.fontdata = File::read_all_bytes("c:\\windows\\fonts\\segoeui.ttf");
	fonts.push_back(std::move(font));
	return fonts;
}

std::vector<uint8_t> LoadWidgetData(const std::string& name)
{
	return File::read_all_bytes(name);
}
