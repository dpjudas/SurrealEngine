/*
 * ContainerUMX.cpp
 * ----------------
 * Purpose: UMX (Unreal Music) module ripper
 * Notes  : Obviously, this code only rips modules from older Unreal Engine games, such as Unreal 1, Unreal Tournament 1 and Deus Ex.
 * Authors: OpenMPT Devs (inspired by code from http://wiki.beyondunreal.com/Legacy:Package_File_Format)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "UMXTools.h"
#include "Container.h"
#include "Sndfile.h"


OPENMPT_NAMESPACE_BEGIN

CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderUMX(MemoryFileReader file, const uint64 *pfilesize)
{
	return UMX::ProbeFileHeader(file, pfilesize, "music");
}


bool UnpackUMX(std::vector<ContainerItem> &containerItems, FileReader &file, ContainerLoadingFlags loadFlags)
{
	file.Rewind();
	containerItems.clear();

	UMX::FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;

	// Note that this can be a false positive, e.g. Unreal maps will have music and sound
	// in their name table because they usually import such files. However, it spares us
	// from wildly seeking through the file, as the name table is usually right at the
	// start of the file, so it is hopefully a good enough heuristic for our purposes.
	if(!UMX::FindNameTableEntry(file, fileHeader, "music"))
		return false;
	else if(!file.CanRead(fileHeader.GetMinimumAdditionalFileSize()))
		return false;
	else if(loadFlags == ContainerOnlyVerifyHeader)
		return true;

	const std::vector<std::string> names = UMX::ReadNameTable(file, fileHeader);
	const std::vector<int32> classes = UMX::ReadImportTable(file, fileHeader, names);

	// Read export table
	file.Seek(fileHeader.exportOffset);
	for(uint32 i = 0; i < fileHeader.exportCount && file.CanRead(8); i++)
	{
		auto [fileChunk, objName] = UMX::ReadExportTableEntry(file, fileHeader, classes, names, "music");
		if(!fileChunk.IsValid())
			continue;

		ContainerItem item;

		if(objName >= 0 && static_cast<std::size_t>(objName) < names.size())
		{
			item.name = mpt::ToUnicode(mpt::Charset::Windows1252, names[objName]);
		}

		item.file = fileChunk;

		containerItems.push_back(std::move(item));
	}

	return !containerItems.empty();
}


OPENMPT_NAMESPACE_END
