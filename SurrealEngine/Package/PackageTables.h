#pragma once

#include "NameString.h"
#include "ObjectFlags.h"

class NameTableEntry
{
public:
	NameString Name;
	uint32_t Flags;
};

class ImportTableEntry
{
public:
	int32_t ClassPackage;
	int32_t ClassName;
	int32_t ObjPackage;
	int32_t ObjName;
};

class ExportTableEntry
{
public:
	int32_t ObjClass;
	int32_t ObjBase;
	int32_t ObjPackage;
	int32_t ObjName;
	ObjectFlags ObjFlags;
	int32_t ObjSize;
	int32_t ObjOffset;
};
