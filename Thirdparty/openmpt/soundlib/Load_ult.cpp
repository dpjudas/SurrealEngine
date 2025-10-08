/*
 * Load_ult.cpp
 * ------------
 * Purpose: ULT (UltraTracker) module loader
 * Notes  : (currently none)
 * Authors: Storlek (Original author - http://schismtracker.org/ - code ported with permission)
 *			Johannes Schultz (OpenMPT Port, tweaks)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct UltFileHeader
{
	char  signature[14];		// "MAS_UTrack_V00"
	uint8 version;				// '1'...'4'
	char  songName[32];			// Song Name, space-padded
	uint8 messageLength;		// Number of Lines
};

MPT_BINARY_STRUCT(UltFileHeader, 48)


struct UltSample
{
	enum UltSampleFlags
	{
		ULT_16BIT = 4,
		ULT_LOOP  = 8,
		ULT_PINGPONGLOOP = 16,
	};

	char     name[32];
	char     filename[12];
	uint32le loopStart;
	uint32le loopEnd;
	uint32le sizeStart;
	uint32le sizeEnd;
	uint8le  volume;	// 0-255, apparently prior to 1.4 this was logarithmic?
	uint8le  flags;		// above
	uint16le speed;		// only exists for 1.4+
	int16le  finetune;

	// Convert an ULT sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.Set16BitCuePoints();

		mptSmp.filename = mpt::String::ReadBuf(mpt::String::spacePadded, filename);

		if(sizeEnd <= sizeStart)
		{
			return;
		}

		mptSmp.nLength = sizeEnd - sizeStart;
		mptSmp.nSustainStart = loopStart;
		mptSmp.nSustainEnd = std::min(static_cast<SmpLength>(loopEnd), mptSmp.nLength);
		mptSmp.nVolume = volume;

		mptSmp.nC5Speed = speed * 2;  // Doubled to fit the note range
		if(finetune)
		{
			mptSmp.Transpose(finetune / (12.0 * 32768.0));
		}

		if(flags & ULT_LOOP)
			mptSmp.uFlags.set(CHN_SUSTAINLOOP);
		if(flags & ULT_PINGPONGLOOP)
			mptSmp.uFlags.set(CHN_PINGPONGSUSTAIN);
		if(flags & ULT_16BIT)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nSustainStart /= 2;
			mptSmp.nSustainEnd /= 2;
		}
		
	}
};

MPT_BINARY_STRUCT(UltSample, 66)


/* Unhandled effects:
5x1 - do not loop sample (x is unused)
E0x - set vibrato strength (2 is normal)

The logarithmic volume scale used in older format versions here, or pretty
much anywhere for that matter. I don't even think Ultra Tracker tries to
convert them. */


static std::pair<EffectCommand, uint8> TranslateULTCommands(const uint8 e, uint8 param, uint8 version)
{

	static constexpr EffectCommand ultEffTrans[] =
	{
		CMD_ARPEGGIO,
		CMD_PORTAMENTOUP,
		CMD_PORTAMENTODOWN,
		CMD_TONEPORTAMENTO,
		CMD_VIBRATO,
		CMD_NONE,
		CMD_NONE,
		CMD_TREMOLO,
		CMD_NONE,
		CMD_OFFSET,
		CMD_VOLUMESLIDE,
		CMD_PANNING8,
		CMD_VOLUME8,
		CMD_PATTERNBREAK,
		CMD_NONE,  // extended effects, processed separately
		CMD_SPEED,
	};

	EffectCommand effect = ultEffTrans[e & 0x0F];

	switch(e & 0x0F)
	{
	case 0x00:
		if(!param || version < '3')
			effect = CMD_NONE;
		break;
	case 0x05:
		// play backwards
		if((param & 0x0F) == 0x02 || (param & 0xF0) == 0x20)
		{
			effect = CMD_S3MCMDEX;
			param = 0x9F;
		}
		if(((param & 0x0F) == 0x0C || (param & 0xF0) == 0xC0) && version >= '3')
		{
			effect = CMD_KEYOFF;
			param = 0;
		}
		break;
	case 0x07:
		if(version < '4')
			effect = CMD_NONE;
		break;
	case 0x0A:
		if(param & 0xF0)
			param &= 0xF0;
		break;
	case 0x0B:
		param = (param & 0x0F) * 0x11;
		break;
	case 0x0D: // pattern break
		param = static_cast<uint8>(10 * (param >> 4) + (param & 0x0F));
		break;
	case 0x0E: // special
		switch(param >> 4)
		{
		case 0x01:
			effect = CMD_PORTAMENTOUP;
			param = 0xF0 | (param & 0x0F);
			break;
		case 0x02:
			effect = CMD_PORTAMENTODOWN;
			param = 0xF0 | (param & 0x0F);
			break;
		case 0x08:
			if(version >= '4')
			{
				effect = CMD_S3MCMDEX;
				param = 0x60 | (param & 0x0F);
			}
			break;
		case 0x09:
			effect = CMD_RETRIG;
			param &= 0x0F;
			break;
		case 0x0A:
			effect = CMD_VOLUMESLIDE;
			param = ((param & 0x0F) << 4) | 0x0F;
			break;
		case 0x0B:
			effect = CMD_VOLUMESLIDE;
			param = 0xF0 | (param & 0x0F);
			break;
		case 0x0C: case 0x0D:
			effect = CMD_S3MCMDEX;
			break;
		}
		break;
	case 0x0F:
		if(param > 0x2F)
			effect = CMD_TEMPO;
		break;
	}
	return {effect, param};
}


struct ULTEventResult { uint8 repeat = 0; ModCommand::COMMAND lostCommand = CMD_NONE; ModCommand::PARAM lostParam = 0; };

static ULTEventResult ReadULTEvent(ModCommand &m, FileReader &file, uint8 version)
{
	uint8 repeat = 1;
	uint8 b = file.ReadUint8();
	if(b == 0xFC)	// repeat event
	{
		repeat = file.ReadUint8();
		b = file.ReadUint8();
	}

	m.note = (b > 0 && b < 97) ? (b + 23 + NOTE_MIN) : NOTE_NONE;

	const auto [instr, cmd, para1, para2] = file.ReadArray<uint8, 4>();
	
	m.instr = instr;
	auto [cmd1, param1] = TranslateULTCommands(cmd & 0x0F, para1, version);
	auto [cmd2, param2]= TranslateULTCommands(cmd >> 4, para2, version);

	// sample offset -- this is even more special than digitrakker's
	if(cmd1 == CMD_OFFSET && cmd2 == CMD_OFFSET)
	{
		uint32 offset = ((param2 << 8) | param1) >> 6;
		m.SetEffectCommand(CMD_OFFSET, static_cast<ModCommand::PARAM>(offset));
		if(offset > 0xFF)
			m.SetVolumeCommand(VOLCMD_OFFSET, static_cast<ModCommand::VOL>(offset >> 8));
		return {repeat};
	} else if(cmd1 == CMD_OFFSET)
	{
		uint32 offset = param1 * 4;
		param1 = mpt::saturate_cast<uint8>(offset);
		if(offset > 0xFF && ModCommand::GetEffectWeight(cmd2) < ModCommand::GetEffectWeight(CMD_OFFSET))
		{
			m.SetEffectCommand(CMD_OFFSET, static_cast<ModCommand::PARAM>(offset));
			m.SetVolumeCommand(VOLCMD_OFFSET, static_cast<ModCommand::VOL>(offset >> 8));
			return {repeat};
		}
	} else if(cmd2 == CMD_OFFSET)
	{
		uint32 offset = param2 * 4;
		param2 = mpt::saturate_cast<uint8>(offset);
		if(offset > 0xFF && ModCommand::GetEffectWeight(cmd1) < ModCommand::GetEffectWeight(CMD_OFFSET))
		{
			m.SetEffectCommand(CMD_OFFSET, static_cast<ModCommand::PARAM>(offset));
			m.SetVolumeCommand(VOLCMD_OFFSET, static_cast<ModCommand::VOL>(offset >> 8));
			return {repeat};
		}
	} else if(cmd1 == cmd2)
	{
		// don't try to figure out how ultratracker does this, it's quite random
		cmd2 = CMD_NONE;
	}
	if(cmd2 == CMD_VOLUME || (cmd2 == CMD_NONE && cmd1 != CMD_VOLUME))
	{
		// swap commands
		std::swap(cmd1, cmd2);
		std::swap(param1, param2);
	}

	// Combine slide commands, if possible
	ModCommand::CombineEffects(cmd2, param2, cmd1, param1);
	const auto lostCommand = m.FillInTwoCommands(cmd1, param1, cmd2, param2);
	return {repeat, lostCommand.first, lostCommand.second};
}


static bool ValidateHeader(const UltFileHeader &fileHeader)
{
	if(fileHeader.version < '1' || fileHeader.version > '4'
	   || std::memcmp(fileHeader.signature, "MAS_UTrack_V00", sizeof(fileHeader.signature)))
	{
		return false;
	}
	return true;
}

static uint64 GetHeaderMinimumAdditionalSize(const UltFileHeader &fileHeader)
{
	return fileHeader.messageLength * 32u + 3u + 256u;
}

CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderULT(MemoryFileReader file, const uint64 *pfilesize)
{
	UltFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!ValidateHeader(fileHeader))
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, GetHeaderMinimumAdditionalSize(fileHeader));
}


bool CSoundFile::ReadULT(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	UltFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!ValidateHeader(fileHeader))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
		return false;

	InitializeGlobals(MOD_TYPE_ULT, 0);
	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.songName);

	const mpt::uchar *versions[] = {UL_("<1.4"), UL_("1.4"), UL_("1.5"), UL_("1.6")};
	m_modFormat.formatName = UL_("UltraTracker");
	m_modFormat.type = UL_("ult");
	m_modFormat.madeWithTracker = U_("UltraTracker ") + versions[fileHeader.version - '1'];
	m_modFormat.charset = mpt::Charset::CP437;

	m_SongFlags = SONG_AUTO_TONEPORTA | SONG_AUTO_TONEPORTA_CONT | SONG_ITCOMPATGXX | SONG_ITOLDEFFECTS;  // this will be converted to IT format by MPT.
	m_playBehaviour.reset(kITClearPortaTarget);
	m_playBehaviour.reset(kITPortaTargetReached);
	m_playBehaviour.set(kFT2PortaTargetNoReset);

	// Read "messageLength" lines, each containing 32 characters.
	m_songMessage.ReadFixedLineLength(file, fileHeader.messageLength * 32, 32, 0);

	if(SAMPLEINDEX numSamples = file.ReadUint8(); numSamples < MAX_SAMPLES)
		m_nSamples = numSamples;
	else
		return false;

	for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
	{
		UltSample sampleHeader;

		// Annoying: v4 added a field before the end of the struct
		if(fileHeader.version >= '4')
		{
			file.ReadStruct(sampleHeader);
		} else
		{
			file.ReadStructPartial(sampleHeader, 64);
			sampleHeader.finetune = sampleHeader.speed;
			sampleHeader.speed = 8363;
		}

		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);
	}

	ReadOrderFromFile<uint8>(Order(), file, 256, 0xFF, 0xFE);

	if(CHANNELINDEX numChannels = file.ReadUint8() + 1u; numChannels <= MAX_BASECHANNELS)
		ChnSettings.resize(numChannels);
	else
		return false;

	PATTERNINDEX numPats = file.ReadUint8() + 1;

	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		if(fileHeader.version >= '3')
			ChnSettings[chn].nPan = ((file.ReadUint8() & 0x0F) << 4) + 8;
		else
			ChnSettings[chn].nPan = (chn & 1) ? 192 : 64;
	}

	Patterns.ResizeArray(numPats);
	for(PATTERNINDEX pat = 0; pat < numPats; pat++)
	{
		if(!Patterns.Insert(pat, 64))
			return false;
	}

	bool postFixSpeedCommands = false;
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ModCommand evnote;
		for(PATTERNINDEX pat = 0; pat < numPats && file.CanRead(5); pat++)
		{
			ModCommand *note = Patterns[pat].GetpModCommand(0, chn);
			ROWINDEX row = 0;
			while(row < 64)
			{
				const ULTEventResult eventResult = ReadULTEvent(evnote, file, fileHeader.version);
				if(eventResult.lostCommand != CMD_NONE && ModCommand::IsGlobalCommand(eventResult.lostCommand, eventResult.lostParam))
					Patterns[pat].WriteEffect(EffectWriter(eventResult.lostCommand, eventResult.lostParam).Row(row).RetryNextRow());
				int repeat = eventResult.repeat;
				if(repeat + row > 64)
					repeat = 64 - row;
				if(repeat == 0)
					break;
				if(evnote.command == CMD_SPEED && evnote.param == 0)
					postFixSpeedCommands = true;
				while(repeat--)
				{
					*note = evnote;
					note += GetNumChannels();
					row++;
				}
			}
		}
	}
	if(postFixSpeedCommands)
	{
		for(CPattern &pat : Patterns)
		{
			for(ROWINDEX row = 0; row < pat.GetNumRows(); row++)
			{
				for(auto &m : pat.GetRow(row))
				{
					if(m.command == CMD_SPEED && m.param == 0)
					{
						m.param = 6;
						pat.WriteEffect(EffectWriter(CMD_TEMPO, 125).Row(row).RetryNextRow());
					}
				}
			}
		}
	}
	
	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
		{
			SampleIO(
				Samples[smp].uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(Samples[smp], file);
		}
	}
	return true;
}


OPENMPT_NAMESPACE_END
