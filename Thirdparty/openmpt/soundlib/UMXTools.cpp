/*
* UMXTools.h
* ------------
* Purpose: UMX/UAX (Unreal package) helper functions
* Notes  : (currently none)
* Authors: OpenMPT Devs (inspired by code from https://wiki.beyondunreal.com/Legacy:Package_File_Format)
* The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
*/


#include "stdafx.h"
#include "UMXTools.h"
#include "Loaders.h"


OPENMPT_NAMESPACE_BEGIN

namespace UMX
{

bool FileHeader::IsValid() const
{
	return !std::memcmp(magic, "\xC1\x83\x2A\x9E", 4)
		&& nameOffset >= sizeof(FileHeader)
		&& exportOffset >= sizeof(FileHeader)
		&& importOffset >= sizeof(FileHeader)
		&& nameCount > 0 && nameCount <= uint32_max / 5u
		&& exportCount > 0 && exportCount <= uint32_max / 8u
		&& importCount > 0 && importCount <= uint32_max / 4u
		&& uint32_max - nameCount * 5u >= nameOffset
		&& uint32_max - exportCount * 8u >= exportOffset
		&& uint32_max - importCount * 4u >= importOffset;
}


uint32 FileHeader::GetMinimumAdditionalFileSize() const
{
	return std::max({nameOffset + nameCount * 5u, exportOffset + exportCount * 8u, importOffset + importCount * 4u}) - static_cast<uint32>(sizeof(FileHeader));
}


CSoundFile::ProbeResult ProbeFileHeader(MemoryFileReader file, const uint64 *pfilesize, const char *requiredType)
{
	FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return CSoundFile::ProbeWantMoreData;
	}
	if(!fileHeader.IsValid())
	{
		return CSoundFile::ProbeFailure;
	}
	if(requiredType != nullptr && !FindNameTableEntryMemory(file, fileHeader, requiredType))
	{
		return CSoundFile::ProbeFailure;
	}
	return CSoundFile::ProbeAdditionalSize(file, pfilesize, fileHeader.GetMinimumAdditionalFileSize());
}


// Read compressed unreal integers - similar to MIDI integers, but signed values are possible.
template <typename Tfile>
static int32 ReadIndexImpl(Tfile &chunk)
{
	enum
	{
		signMask      = 0x80,  // Highest bit of first byte indicates if value is signed
		valueMask1    = 0x3F,  // Low 6 bits of first byte are actual value
		continueMask1 = 0x40,  // Second-highest bit of first byte indicates if further bytes follow
		valueMask     = 0x7F,  // Low 7 bits of following bytes are actual value
		continueMask  = 0x80,  // Highest bit of following bytes indicates if further bytes follow
	};

	// Read first byte
	uint8 b = chunk.ReadUint8();
	bool isSigned = (b & signMask) != 0;
	uint32 result = (b & valueMask1);
	int shift = 6;

	if(b & continueMask1)
	{
		// Read remaining bytes
		do
		{
			b = chunk.ReadUint8();
			uint32 data = static_cast<uint32>(b) & valueMask;
			data <<= shift;
			result |= data;
			shift += 7;
		} while((b & continueMask) != 0 && (shift < 32));
	}

	if(isSigned && result <= int32_max)
		return -static_cast<int32>(result);
	else if(isSigned)
		return int32_min;
	else
		return result;
}

int32 ReadIndex(FileReader &chunk)
{
	return ReadIndexImpl(chunk);
}


// Returns true if the given nme exists in the name table.
template <typename TFile>
static bool FindNameTableEntryImpl(TFile &file, const FileHeader &fileHeader, const char *name)
{
	if(!name)
	{
		return false;
	}
	const std::size_t nameLen = std::strlen(name);
	if(nameLen == 0)
	{
		return false;
	}
	bool result = false;
	const FileReader::pos_type oldpos = file.GetPosition();
	if(file.Seek(fileHeader.nameOffset))
	{
		for(uint32 i = 0; i < fileHeader.nameCount && file.CanRead(5); i++)
		{
			if(fileHeader.packageVersion >= 64)
			{
				int32 length = ReadIndexImpl(file);
				if(length <= 0)
				{
					continue;
				}
			}
			bool match = true;
			std::size_t pos = 0;
			char c = 0;
			while((c = file.ReadUint8()) != 0)
			{
				c = mpt::ToLowerCaseAscii(c);
				if(pos < nameLen)
				{
					match = match && (c == name[pos]);
				}
				pos++;
			}
			if(pos != nameLen)
			{
				match = false;
			}
			if(match)
			{
				result = true;
			}
			file.Skip(4);  // Object flags
		}
	}
	file.Seek(oldpos);
	return result;
}

bool FindNameTableEntry(FileReader &file, const FileHeader &fileHeader, const char *name)
{
	return FindNameTableEntryImpl(file, fileHeader, name);
}

bool FindNameTableEntryMemory(MemoryFileReader &file, const FileHeader &fileHeader, const char *name)
{
	return FindNameTableEntryImpl(file, fileHeader, name);
}


// Read an entry from the name table.
std::string ReadNameTableEntry(FileReader &chunk, uint16 packageVersion)
{
	std::string name;
	if(packageVersion >= 64)
	{
		// String length
		int32 length = ReadIndex(chunk);
		if(length <= 0)
		{
			return "";
		}
		name.reserve(std::min(length, mpt::saturate_cast<int32>(chunk.BytesLeft())));
	}

	// Simple zero-terminated string
	uint8 chr;
	while((chr = chunk.ReadUint8()) != 0)
	{
		// Convert string to lower case
		if(chr >= 'A' && chr <= 'Z')
		{
			chr = chr - 'A' + 'a';
		}
		name.append(1, static_cast<char>(chr));
	}

	chunk.Skip(4);  // Object flags
	return name;
}


// Read complete name table.
std::vector<std::string> ReadNameTable(FileReader &file, const FileHeader &fileHeader)
{
	file.Seek(fileHeader.nameOffset);  // nameOffset and nameCount were validated when parsing header
	std::vector<std::string> names;
	names.reserve(fileHeader.nameCount);
	for(uint32 i = 0; i < fileHeader.nameCount && file.CanRead(5); i++)
	{
		names.push_back(ReadNameTableEntry(file, fileHeader.packageVersion));
	}
	return names;
}


// Read an entry from the import table.
int32 ReadImportTableEntry(FileReader &chunk, uint16 packageVersion)
{
	ReadIndex(chunk);  // Class package
	ReadIndex(chunk);  // Class name
	if(packageVersion >= 60)
	{
		chunk.Skip(4);  // Package
	} else
	{
		ReadIndex(chunk);  // ??
	}
	return ReadIndex(chunk);  // Object name (offset into the name table)
}


// Read import table.
std::vector<int32> ReadImportTable(FileReader &file, const FileHeader &fileHeader, const std::vector<std::string> &names)
{
	file.Seek(fileHeader.importOffset);  // importOffset and importCount were validated when parsing header
	std::vector<int32> classes;
	classes.reserve(fileHeader.importCount);
	for(uint32 i = 0; i < fileHeader.importCount && file.CanRead(4); i++)
	{
		int32 objName = ReadImportTableEntry(file, fileHeader.packageVersion);
		if(static_cast<size_t>(objName) < names.size())
		{
			classes.push_back(objName);
		}
	}
	return classes;
}


// Read an entry from the export table.
std::pair<FileReader, int32> ReadExportTableEntry(FileReader &file, const FileHeader &fileHeader, const std::vector<int32> &classes, const std::vector<std::string> &names, const char *filterType)
{
	const uint32 objClass = ~static_cast<uint32>(ReadIndex(file));  // Object class
	if(objClass >= classes.size())
		return {};

	ReadIndex(file);  // Object parent
	if(fileHeader.packageVersion >= 60)
	{
		file.Skip(4);  // Internal package / group of the object
	}
	int32 objName = ReadIndex(file);  // Object name (offset into the name table)
	file.Skip(4);                     // Object flags
	int32 objSize = ReadIndex(file);
	int32 objOffset = ReadIndex(file);
	if(objSize <= 0 || objOffset <= static_cast<int32>(sizeof(FileHeader)))
		return {};

	// If filterType is set, reject any objects not of that type
	if(filterType != nullptr && names[classes[objClass]] != filterType)
		return {};

	FileReader chunk = file.GetChunkAt(objOffset, objSize);
	if(!chunk.IsValid())
		return {};

	if(fileHeader.packageVersion < 40)
	{
		chunk.Skip(8);  // 00 00 00 00 00 00 00 00
	}
	if(fileHeader.packageVersion < 60)
	{
		chunk.Skip(16);  // 81 00 00 00 00 00 FF FF FF FF FF FF FF FF 00 00
	}
	// Read object properties
#if 0
	size_t propertyName = static_cast<size_t>(ReadIndex(chunk));
	if(propertyName >= names.size() || names[propertyName] != "none")
	{
		// Can't bother to implement property reading, as no UMX files I've seen so far use properties for the relevant objects,
		// and only the UAX files in the Unreal 1997/98 beta seem to use this and still load just fine when ignoring it.
		// If it should be necessary to implement this, check CUnProperty.cpp in http://ut-files.com/index.php?dir=Utilities/&file=utcms_source.zip
		MPT_ASSERT_NOTREACHED();
		continue;
	}
#else
	ReadIndex(chunk);
#endif

	if(fileHeader.packageVersion >= 120)
	{
		// UT2003 Packages
		ReadIndex(chunk);
		chunk.Skip(8);
	} else if(fileHeader.packageVersion >= 100)
	{
		// AAO Packages
		chunk.Skip(4);
		ReadIndex(chunk);
		chunk.Skip(4);
	} else if(fileHeader.packageVersion >= 62)
	{
		// UT Packages
		// Mech8.umx and a few other UT tunes have packageVersion = 62.
		// In CUnSound.cpp, the condition above reads "packageVersion >= 63" but if that is used, those tunes won't load properly.
		ReadIndex(chunk);
		chunk.Skip(4);
	} else
	{
		// Old Unreal Packagaes
		ReadIndex(chunk);
	}

	int32 size = ReadIndex(chunk);
	return {chunk.ReadChunk(size), objName};
}


}  // namespace UMX

OPENMPT_NAMESPACE_END
