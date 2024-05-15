#pragma once 

#include "UObject/UClass.h"

class Exporter
{
public:
	static std::string ExportObject(UObject* obj, int tablevel, bool bInline);
	static std::string ExportClass(UClass* cls);
};