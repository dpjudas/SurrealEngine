/*
 * Load_digi.cpp
 * -------------
 * Purpose: Digi Booster module loader
 * Notes  : Basically these are like ProTracker MODs with a few extra features such as more channels, longer samples and a few more effects.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

// DIGI File Header
struct DIGIFileHeader
{
	char     signature[20];
	char     versionStr[4];	// Supposed to be "V1.6" or similar, but other values like "TAP!" have been found as well.
	uint8be  versionInt;	// e.g. 0x16 = 1.6
	uint8be  numChannels;
	uint8be  packEnable;
	char     unknown[19];
	uint8be  lastPatIndex;
	uint8be  lastOrdIndex;
	uint8be  orders[128];
	uint32be smpLength[31];
	uint32be smpLoopStart[31];
	uint32be smpLoopLength[31];
	uint8be  smpVolume[31];
	uint8be  smpFinetune[31];
};

MPT_BINARY_STRUCT(DIGIFileHeader, 610)


static void ReadDIGIPatternEntry(FileReader &file, ModCommand &m)
{
	const auto [command, param] = CSoundFile::ReadMODPatternEntry(file, m);
	CSoundFile::ConvertModCommand(m, command, param);
	if(m.command == CMD_MODCMDEX)
	{
		switch(m.param & 0xF0)
		{
		case 0x30:
			// E3x: Play sample backwards (E30 stops sample when it reaches the beginning, any other value plays it from the beginning including regular loop)
			// The play direction is also reset if a new note is played on the other channel linked to this channel.
			// The behaviour is rather broken when there is no note next to the ommand.
			m.command = CMD_DIGIREVERSESAMPLE;
			m.param &= 0x0F;
			break;
		case 0x40:
			// E40: Stop playing sample
			if(m.param == 0x40)
			{
				m.note = NOTE_NOTECUT;
				m.command = CMD_NONE;
			}
			break;
		case 0x80:
			// E8x: High sample offset
			m.command = CMD_S3MCMDEX;
			m.param = 0xA0 | (m.param & 0x0F);
		}
	} else if(m.command == CMD_PANNING8)
	{
		// 8xx "Robot" effect (not supported)
		m.command = CMD_NONE;
	}
}


static bool ValidateHeader(const DIGIFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.signature, "DIGI Booster module\0", 20)
		|| !fileHeader.numChannels
		|| fileHeader.numChannels > 8
		|| fileHeader.lastOrdIndex > 127)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderDIGI(MemoryFileReader file, const uint64 *pfilesize)
{
	DIGIFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadDIGI(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	DIGIFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	// Globals
	InitializeGlobals(MOD_TYPE_DIGI, fileHeader.numChannels);
	m_nSamples = 31;
	m_nSamplePreAmp = 256 / GetNumChannels();

	m_modFormat.formatName = UL_("DigiBooster");
	m_modFormat.type = UL_("digi");
	m_modFormat.madeWithTracker = MPT_UFORMAT("Digi Booster {}.{}")(fileHeader.versionInt >> 4, fileHeader.versionInt & 0x0F);
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	ReadOrderFromArray(Order(), fileHeader.orders, fileHeader.lastOrdIndex + 1);

	// Read sample headers
	for(SAMPLEINDEX smp = 0; smp < 31; smp++)
	{
		ModSample &sample = Samples[smp + 1];
		sample.Initialize(MOD_TYPE_MOD);
		sample.nLength = fileHeader.smpLength[smp];
		sample.nLoopStart = fileHeader.smpLoopStart[smp];
		sample.nLoopEnd = sample.nLoopStart + fileHeader.smpLoopLength[smp];
		if(fileHeader.smpLoopLength[smp])
		{
			sample.uFlags.set(CHN_LOOP);
		}
		sample.SanitizeLoops();
	
		sample.nVolume = std::min(fileHeader.smpVolume[smp].get(), uint8(64)) * 4;
		sample.nFineTune = MOD2XMFineTune(fileHeader.smpFinetune[smp]);
	}

	// Read song + sample names
	file.ReadString<mpt::String::maybeNullTerminated>(m_songName, 32);
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		file.ReadString<mpt::String::maybeNullTerminated>(m_szNames[smp], 30);
	}


	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.lastPatIndex + 1);
	for(PATTERNINDEX pat = 0; pat <= fileHeader.lastPatIndex; pat++)
	{
		FileReader patternChunk;
		if(fileHeader.packEnable)
		{
			patternChunk = file.ReadChunk(file.ReadUint16BE());
		} else
		{
			patternChunk = file.ReadChunk(4 * 64 * GetNumChannels());
		}

		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			continue;
		}

		if(fileHeader.packEnable)
		{
			uint8 eventMask[64];
			patternChunk.ReadArray(eventMask);

			// Compressed patterns are stored in row-major order...
			for(ROWINDEX row = 0; row < 64; row++)
			{
				
				uint8 bit = 0x80;
				for(ModCommand &m : Patterns[pat].GetRow(row))
				{
					if(eventMask[row] & bit)
						ReadDIGIPatternEntry(patternChunk, m);
					bit >>= 1;
				}
			}
		} else
		{
			// ...but uncompressed patterns are stored in column-major order. WTF!
			for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
			{
				for(ROWINDEX row = 0; row < 64; row++)
				{
					ReadDIGIPatternEntry(patternChunk, *Patterns[pat].GetpModCommand(row, chn));
				}
			}
		}
	}

	if(loadFlags & loadSampleData)
	{
		// Reading Samples
		const SampleIO sampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::bigEndian,
			SampleIO::signedPCM);

		for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
		{
			sampleIO.ReadSample(Samples[smp], file);
		}
	}

	return true;
}


OPENMPT_NAMESPACE_END
