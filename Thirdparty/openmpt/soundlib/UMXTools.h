/*
 * UMXTools.h
 * ------------
 * Purpose: UMX/UAX (Unreal) helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs (inspired by code from http://wiki.beyondunreal.com/Legacy:Package_File_Format)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include "Sndfile.h"


OPENMPT_NAMESPACE_BEGIN

namespace UMX
{

// UMX File Header
struct FileHeader
{
	char magic[4];  // C1 83 2A 9E
	uint16le packageVersion;
	uint16le licenseMode;
	uint32le flags;
	uint32le nameCount;
	uint32le nameOffset;
	uint32le exportCount;
	uint32le exportOffset;
	uint32le importCount;
	uint32le importOffset;

	bool IsValid() const;
	uint32 GetMinimumAdditionalFileSize() const;
};

MPT_BINARY_STRUCT(FileHeader, 36)


// Check validity of file header
CSoundFile::ProbeResult ProbeFileHeader(MemoryFileReader file, const uint64* pfilesize, const char *requiredType);

// Read compressed unreal integers - similar to MIDI integers, but signed values are possible.
int32 ReadIndex(FileReader &chunk);

// Returns true if the given nme exists in the name table.
bool FindNameTableEntry(FileReader &file, const FileHeader &fileHeader, const char *name);

// Returns true if the given nme exists in the name table.
bool FindNameTableEntryMemory(MemoryFileReader &file, const FileHeader &fileHeader, const char *name);

// Read an entry from the name table.
std::string ReadNameTableEntry(FileReader &chunk, uint16 packageVersion);

// Read complete name table.
std::vector<std::string> ReadNameTable(FileReader &file, const FileHeader &fileHeader);

// Read import table.
std::vector<int32> ReadImportTable(FileReader &file, const FileHeader &fileHeader, const std::vector<std::string> &names);

// Read an entry from the import table.
int32 ReadImportTableEntry(FileReader &chunk, uint16 packageVersion);

// Read an entry from the export table.
std::pair<FileReader, int32> ReadExportTableEntry(FileReader &file, const FileHeader &fileHeader, const std::vector<int32> &classes, const std::vector<std::string> &names, const char *filterType);

}  // namespace UMX

OPENMPT_NAMESPACE_END
