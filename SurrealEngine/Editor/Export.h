#pragma once 

#include "Utils/MemoryStreamWriter.h"

class UObject;
class UClass;
class UTexture;
class UFireTexture;
class UWaveTexture;
class UWetTexture;
class UIceTexture;

class Exporter
{
public:
	static std::string ExportObject(UObject* obj, int tablevel, bool bInline);

	static MemoryStreamWriter ExportClass(UClass* cls);
	static MemoryStreamWriter ExportTexture(UTexture* tex, const std::string& ext);

private:
	static std::string ExportClassDefaultProperties(UClass* cls, int tablevel);

	static MemoryStreamWriter ExportFireTexture(UFireTexture* tex);
	static MemoryStreamWriter ExportWaveTexture(UWaveTexture* tex);
	static MemoryStreamWriter ExportWetTexture(UWetTexture* tex);
	static MemoryStreamWriter ExportIceTexture(UIceTexture* tex);

	static MemoryStreamWriter ExportBmpIndexed(UTexture* tex);
	static MemoryStreamWriter ExportPng(UTexture* tex);

	static MemoryStreamWriter GetImage(UTexture* tex);
	static MemoryStreamWriter GetImageP8(UTexture* tex);
};