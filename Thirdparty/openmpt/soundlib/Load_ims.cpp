/*
 * Load_ims.cpp
 * ------------
 * Purpose: Images Music System (IMS) module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "MODTools.h"

OPENMPT_NAMESPACE_BEGIN

struct IMSFileHeader
{
	std::array<char, 20> songName;
	std::array<MODSampleHeader, 31> sampleHeaders;
	MODFileHeader order;
	uint32be sampleDataOffset;

	bool IsValid() const
	{
		if(CountInvalidChars(songName))
			return false;

		uint16 hasSampleLength = 0;
		for(const auto &sampleHeader : sampleHeaders)
		{
			if(sampleHeader.length > 0x8000 || sampleHeader.finetune || sampleHeader.GetInvalidByteScore())
				return false;
			hasSampleLength |= sampleHeader.length;
		}
		if(!hasSampleLength)
			return false;

		if(!order.numOrders || order.numOrders > 128 || order.restartPos > order.numOrders)
			return false;

		if(sampleDataOffset <= 1084)
			return false;
		const auto patternDiv = std::div(sampleDataOffset - 1084, 768);
		if(patternDiv.rem)
			return false;
		const uint8 numPatterns = mpt::saturate_cast<uint8>(patternDiv.quot);
		if(numPatterns > 128)
			return false;

		for(const auto pat : order.orderList)
		{
			if(pat >= numPatterns)
				return false;
		}
		
		return true;
	}
};

MPT_BINARY_STRUCT(IMSFileHeader, 1084)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderIMS(MemoryFileReader file, const uint64 *pfilesize)
{
	IMSFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(pfilesize && *pfilesize < fileHeader.sampleDataOffset)
		return ProbeFailure;
	if(!fileHeader.IsValid())
		return ProbeFailure;

	return ProbeSuccess;
}


bool CSoundFile::ReadIMS(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	IMSFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.IsValid())
		return false;
	if(!file.LengthIsAtLeast(fileHeader.sampleDataOffset))
		return false;

	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, 4);
	m_SongFlags.set(SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	Order().SetDefaultTempoInt(125);
	Order().SetDefaultSpeed(6);
	Order().SetRestartPos(fileHeader.order.restartPos);
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 1712 * 4;
	m_nSamples = 31;
	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songName);

	ReadOrderFromArray(Order(), fileHeader.order.orderList, fileHeader.order.numOrders);

	file.Seek(20);
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		const auto &sampleHeader = fileHeader.sampleHeaders[smp - 1];
		sampleHeader.ConvertToMPT(Samples[smp], true);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);
	}

	if(loadFlags & loadPatternData)
	{
		file.Seek(1084);
		const PATTERNINDEX numPatterns = static_cast<PATTERNINDEX>((fileHeader.sampleDataOffset - 1084u) / 768u);
		for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
		{
			if(!Patterns.Insert(pat, 64))
				return false;
			for(ModCommand &m : Patterns[pat])
			{
				const auto data = file.ReadArray<uint8, 3>();
				if(const uint8 note = (data[0] & 0x3F); note < 48)
					m.note = NOTE_MIDDLEC - 24 + note;
				else if(note != 63)
					return false;
				m.instr = ((data[0] & 0xC0) >> 2) | (data[1] >> 4);
				if(m.instr > 31)
					return false;
				ConvertModCommand(m, data[1] & 0x0F, data[2]);
			}
		}
	}

	if(loadFlags & loadSampleData)
	{
		file.Seek(fileHeader.sampleDataOffset);
		for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
		{
			if(!Samples[smp].nLength)
				continue;
			SampleIO(
				SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(Samples[smp], file);
		}
	}

	m_modFormat.formatName = UL_("Images Music System");
	m_modFormat.type = UL_("ims");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	return true;
}

OPENMPT_NAMESPACE_END
