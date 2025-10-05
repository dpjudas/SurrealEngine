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
	int32_t ClassPackage;   // name index
	int32_t ClassName;      // name index
	int32_t ObjOuter;       // objref
	int32_t ObjName;        // name index
};

class ExportTableEntry
{
public:
	int32_t ObjClass;       // objref
	int32_t ObjBase;        // objref
	int32_t ObjOuter;       // objref
	int32_t ObjName;        // name index
	ObjectFlags ObjFlags;
	int32_t ObjSize;
	int32_t ObjOffset;
};
