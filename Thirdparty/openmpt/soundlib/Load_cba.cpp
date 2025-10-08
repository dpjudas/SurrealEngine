/*
 * Load_cba.cpp
 * ------------
 * Purpose: Chuck Biscuits / Black Artist (CBA) module loader
 * Notes  : This format appears to have been used only for the Expoze musicdisk by Heretics.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct CBAFileHeader
{
	char     magic[4];  // 'CBA\xF9'
	char     title[32];
	uint8    eof;
	uint16le messageLength;
	uint8    numChannels;
	uint8    lastPattern;
	uint8    numOrders;
	uint8    numSamples;
	uint8    speed;
	uint8    tempo;
	uint8    panPos[32];
	uint8    orders[255];

	bool IsValid() const
	{
		return !memcmp(magic, "CBA\xF9", 4)
			&& eof == 0x1A
			&& numChannels > 0 && numChannels <= 32
			&& speed > 0
			&& tempo >= 32;
	}

	uint32 GetHeaderMinimumAdditionalSize() const
	{
		return numSamples * 48 + messageLength;
	}
};

MPT_BINARY_STRUCT(CBAFileHeader, 332)


struct CBASampleHeader
{
	char     name[32];
	uint8    flags;
	uint8    volume;
	uint16le sampleRate;
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;

	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nVolume = volume * 4;
		mptSmp.nC5Speed = sampleRate;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.uFlags.set(CHN_LOOP, flags & 0x08);
	}
};

MPT_BINARY_STRUCT(CBASampleHeader, 48)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderCBA(MemoryFileReader file, const uint64 *pfilesize)
{
	CBAFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadCBA(FileReader &file, ModLoadingFlags loadFlags)
{
	CBAFileHeader fileHeader;

	file.Rewind();
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;
	if(!file.CanRead(fileHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_S3M, fileHeader.numChannels);
	m_SongFlags.set(SONG_IMPORTED);
	m_playBehaviour.set(kST3SampleSwap);  // AFTERMIX.CBA, pattern 53
	m_nMixLevels = MixLevels::CompatibleFT2;
	m_nSamples = fileHeader.numSamples;
	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.title);
	Order().SetDefaultTempoInt(fileHeader.tempo);
	Order().SetDefaultSpeed(fileHeader.speed);
	ReadOrderFromArray(Order(), fileHeader.orders, fileHeader.numOrders, 0xFF, 0xFE);

	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = fileHeader.panPos[chn] * 2;
	}

	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		CBASampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);
	}

	Patterns.ResizeArray(fileHeader.lastPattern + 1);
	for(PATTERNINDEX pat = 0; pat < Patterns.Size(); pat++)
	{
		if(!(loadFlags & loadPatternData) || !file.CanRead(64 * 5 * fileHeader.numChannels) || !Patterns.Insert(pat, 64))
		{
			file.Skip(64 * 5 * fileHeader.numChannels);
			continue;
		}

		for(ModCommand &m : Patterns[pat])
		{
			const auto [instr, note, vol, command, param] = file.ReadArray<uint8, 5>();
			m.instr = instr;
			if(note == 255)
				m.note = NOTE_NOTECUT;
			else if(note > 0 && note <= 96)
				m.note = NOTE_MIDDLEC - 49 + note;
			
			if(vol)
				m.SetVolumeCommand(VOLCMD_VOLUME, std::min(vol, uint8(65)) - 1);

			if(command > 0 && command < 0x0F)
				ConvertModCommand(m, command - 1, param);
			else if(command == 0x0F)  // "Funky sync"
				m.SetEffectCommand(CMD_DUMMY, param);
			else if(command == 0x18)
				m.SetEffectCommand(CMD_RETRIG, param);
			else if(command >= 0x10 && command <= 0x1E)
				m.SetEffectCommand(CMD_MODCMDEX, static_cast<ModCommand::PARAM>(((command << 4) + 0x10) | std::min(param, uint8(0x0F))));
			else if(command == 0x1F)
				m.SetEffectCommand(CMD_SPEED, param);
			else if(command == 0x20)
				m.SetEffectCommand(CMD_TEMPO, param);
			m.ExtendedMODtoS3MEffect();
		}
	}

	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		if(loadFlags & loadSampleData)
		{
			SampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::deltaPCM)
				.ReadSample(Samples[smp], file);
		} else
		{
			file.Skip(Samples[smp].nLength);
		}
	}

	m_songMessage.Read(file, fileHeader.messageLength, SongMessage::leCRLF);

	m_modFormat.formatName = UL_("Chuck Biscuits / Black Artist");
	m_modFormat.type = UL_("cba");
	m_modFormat.charset = mpt::Charset::CP437;

	return true;
}


OPENMPT_NAMESPACE_END
