/*
 * Load_uax.cpp
 * ------------
 * Purpose: UAX (Unreal Sounds) module ripper
 * Notes  : The sounds are read into module sample slots.
 * Authors: Johannes Schultz (inspired by code from http://wiki.beyondunreal.com/Legacy:Package_File_Format)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "UMXTools.h"


OPENMPT_NAMESPACE_BEGIN

CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderUAX(MemoryFileReader file, const uint64 *pfilesize)
{
	return UMX::ProbeFileHeader(file, pfilesize, "sound");
}


bool CSoundFile::ReadUAX(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	UMX::FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;

	// Note that this can be a false positive, e.g. Unreal maps will have music and sound
	// in their name table because they usually import such files. However, it spares us
	// from wildly seeking through the file, as the name table is usually right at the
	// start of the file, so it is hopefully a good enough heuristic for our purposes.
	if(!UMX::FindNameTableEntry(file, fileHeader, "sound"))
		return false;
	else if(!file.CanRead(fileHeader.GetMinimumAdditionalFileSize()))
		return false;
	else if(loadFlags == onlyVerifyHeader)
		return true;

	const std::vector<std::string> names = UMX::ReadNameTable(file, fileHeader);
	const std::vector<int32> classes = UMX::ReadImportTable(file, fileHeader, names);

	InitializeGlobals(MOD_TYPE_MPT, 4);
	m_modFormat.formatName = MPT_UFORMAT("Unreal Package v{}")(fileHeader.packageVersion);
	m_modFormat.type = UL_("uax");
	m_modFormat.charset = mpt::Charset::Windows1252;
	
	// Read export table
	file.Seek(fileHeader.exportOffset);
	for(uint32 i = 0; i < fileHeader.exportCount && file.CanRead(8); i++)
	{
		auto [fileChunk, objName] = UMX::ReadExportTableEntry(file, fileHeader, classes, names, "sound");
		if(!fileChunk.IsValid())
			continue;

		if(CanAddMoreSamples())
		{
			// Read as sample
			if(ReadSampleFromFile(GetNumSamples() + 1, fileChunk, true))
			{
				if(objName > 0 && static_cast<size_t>(objName) < names.size())
				{
					m_szNames[GetNumSamples()] = names[objName];
				}
			}
		}
	}

	if(m_nSamples != 0)
	{
		m_ContainerType = ModContainerType::UAX;
		Patterns.Insert(0, 64);
		Order().assign(1, 0);
		return true;
	} else
	{
		return false;
	}
}


OPENMPT_NAMESPACE_END
