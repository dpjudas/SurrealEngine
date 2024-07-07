#pragma once 

#include "Utils/MemoryStreamWriter.h"
#include "UObject/UClass.h"
#include "UObject/UTexture.h"

class Exporter
{
public:
	static std::string ExportObject(UObject* obj, int tablevel, bool bInline);

	static MemoryStreamWriter ExportClass(UClass* cls);
	static MemoryStreamWriter ExportTexture(UTexture* tex, const std::string& ext);

private:
	static MemoryStreamWriter ExportFireTexture(UFireTexture* tex);
	static MemoryStreamWriter ExportWaveTexture(UWaveTexture* tex);
	static MemoryStreamWriter ExportWetTexture(UWetTexture* tex);
	static MemoryStreamWriter ExportIceTexture(UIceTexture* tex);

	static MemoryStreamWriter ExportBmpIndexed(UTexture* tex);
	static MemoryStreamWriter ExportPng(UTexture* tex);

	static MemoryStreamWriter GetImage(UTexture* tex);
	static MemoryStreamWriter GetImageP8(UTexture* tex);
};