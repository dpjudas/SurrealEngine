/*
 * Load_etx.cpp
 * ------------
 * Purpose: EasyTrax (ETX) module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct ETXFileHeader
{
	char     magic[14];  // 'EASYTRAX 1.0\x01\x00'
	uint8    tempo;
	uint8    lastPattern;
	uint32le orderlistOffset;
	uint32le patternsOffset;
	uint32le sampleHeadersOffset;
	uint32le sampleDataOffset;

	bool IsValid() const
	{
		return !memcmp(magic, "EASYTRAX 1.0\x01\x00", 14)
			&& tempo > 0
			&& lastPattern <= 127
			&& orderlistOffset >= 32 && orderlistOffset < 0x80'0000
			&& patternsOffset >= 32 && patternsOffset < 0x80'0000
			&& sampleHeadersOffset >= 32 && sampleHeadersOffset < 0x80'0000
			&& sampleDataOffset >= 32 && sampleDataOffset < 0x80'0000;
	}

	uint32 GetHeaderMinimumAdditionalSize() const
	{
		return 1024;  // Order list
	}
};

MPT_BINARY_STRUCT(ETXFileHeader, 32)


struct ETXSampleHeader
{
	char     name[13];
	uint32le offset;  // Relative to fileHeader.sampleDataOffset
	uint32le length;
	uint32le loopStart;  // 0xFFFFFFFF if no loop
	uint32le sampleRate;
	int8     transpose;
	int8     finetune;
	uint8    zero;

	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nC5Speed = sampleRate;
		mptSmp.nLength = length;
		if(loopStart != uint32_max)
		{
			mptSmp.nLoopStart = loopStart;
			mptSmp.nLoopEnd = mptSmp.nLength;
			mptSmp.uFlags.set(CHN_LOOP);
		}
		mptSmp.Transpose((transpose * 100 + finetune) / 1200.0);
	}
};

MPT_BINARY_STRUCT(ETXSampleHeader, 32)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderETX(MemoryFileReader file, const uint64 *pfilesize)
{
	ETXFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadETX(FileReader &file, ModLoadingFlags loadFlags)
{
	ETXFileHeader fileHeader;
	file.Rewind();
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;
	if(!file.CanRead(fileHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_S3M, 4);
	m_SongFlags.set(SONG_IMPORTED);
	m_playBehaviour.reset(kST3EffectMemory);
	m_nMinPeriod = 218;  // Highest possible sample playback rate appears to be 65535 Hz
	m_nSamples = 128;
	Order().SetDefaultTempoInt(fileHeader.tempo);
	Order().SetDefaultSpeed(6);

	if(!file.Seek(fileHeader.orderlistOffset))
		return false;
	ReadOrderFromFile<uint8>(Order(), file, 1024, 0xFF);
	for(ORDERINDEX ord = 0; ord < Order().size(); ord++)
	{
		if(Order()[ord] == PATTERNINDEX_INVALID)
			Order().resize(ord);
		else if(Order()[ord] > 127)
			return false;
	}

	if(!file.Seek(fileHeader.patternsOffset))
		return false;
	Patterns.ResizeArray(fileHeader.lastPattern + 1);
	for(PATTERNINDEX pat = 0; pat < Patterns.Size(); pat++)
	{
		if(!(loadFlags & loadPatternData) || !file.CanRead(1024) || !Patterns.Insert(pat, 64))
			break;

		auto m = Patterns[pat].begin();
		for(ROWINDEX row = 0; row < Patterns[pat].GetNumRows(); row++)
		{
			for(CHANNELINDEX chn = 0; chn < 4; chn++, m++)
			{
				const auto [note, vol, instr, unused] = file.ReadArray<uint8, 4>();
				MPT_UNUSED_VARIABLE(unused);
				if(note == 0xFF && !chn)
				{
					if(!Patterns[pat].WriteEffect(EffectWriter(CMD_PATTERNBREAK, 0).Row(std::max(row, ROWINDEX(1)) - 1)))
					{
						Patterns[pat].Resize(row, false);
						break;
					}
				} else if(note == 0xFE)
				{
					m->SetEffectCommand(CMD_VOLUMEDOWN_ETX, vol);
				} else if(note > 0 && note <= 96)
				{
					m->note = NOTE_MIDDLEC - 24 + note;
					m->instr = instr + 1;
					m->SetVolumeCommand(VOLCMD_VOLUME, static_cast<ModCommand::VOL>((std::min(vol, uint8(127)) + 1u) / 2u));
				}
			}
		}
	}

	if(!file.Seek(fileHeader.sampleHeadersOffset))
		return false;
	FileReader sampleHeaderChunk = file.ReadChunk(128 * sizeof(ETXSampleHeader));
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		ETXSampleHeader sampleHeader;
		sampleHeaderChunk.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);
		if(loadFlags & loadSampleData)
		{
			if(!file.Seek(fileHeader.sampleDataOffset + sampleHeader.offset))
				return false;
			SampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::unsignedPCM)
				.ReadSample(Samples[smp], file);
		}
	}

	m_modFormat.formatName = UL_("EasyTrax");
	m_modFormat.type = UL_("etx");
	m_modFormat.charset = mpt::Charset::CP437;

	return true;
}


OPENMPT_NAMESPACE_END
