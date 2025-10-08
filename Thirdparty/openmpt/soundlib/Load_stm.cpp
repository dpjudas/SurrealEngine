/*
 * Load_stm.cpp
 * ------------
 * Purpose: STM (Scream Tracker 2) and STX (Scream Tracker Music Interface Kit - a mixture of STM and S3M) module loaders
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "S3MTools.h"

OPENMPT_NAMESPACE_BEGIN

// STM sample header struct
struct STMSampleHeader
{
	char     filename[12];  // Can't have long comments - just filename comments :)
	uint8le  zero;
	uint8le  disk;       // A blast from the past
	uint16le offset;     // 20-bit offset in file (lower 4 bits are zero)
	uint16le length;     // Sample length
	uint16le loopStart;  // Loop start point
	uint16le loopEnd;    // Loop end point
	uint8le  volume;     // Volume
	uint8le  reserved2;
	uint16le sampleRate;
	uint8le  reserved3[6];

	// Convert an STM sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.filename = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, filename);

		mptSmp.nC5Speed = sampleRate;
		mptSmp.nVolume = std::min(volume.get(), uint8(64)) * 4;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;

		if(mptSmp.nLength < 2) mptSmp.nLength = 0;

		if(mptSmp.nLoopStart < mptSmp.nLength
			&& mptSmp.nLoopEnd > mptSmp.nLoopStart
			&& mptSmp.nLoopEnd != 0xFFFF)
		{
			mptSmp.uFlags = CHN_LOOP;
			mptSmp.nLength = std::max(mptSmp.nLoopEnd, mptSmp.nLength);  // ST2 does not sanitize loop end, allow it to overflow into the next sample's data
		}
	}
};

MPT_BINARY_STRUCT(STMSampleHeader, 32)


// STM file header
struct STMFileHeader
{
	char  songname[20];
	char  trackerName[8];  // !Scream! for ST 2.xx
	uint8 dosEof;          // 0x1A
	uint8 filetype;        // 1=song, 2=module (only 2 is supported, of course) :)
	uint8 verMajor;
	uint8 verMinor;
	uint8 initTempo;
	uint8 numPatterns;
	uint8 globalVolume;
	uint8 reserved[13];

	bool Validate() const
	{
		if(filetype != 2
		   || (dosEof != 0x1A && dosEof != 2)  // ST2 ignores this, ST3 doesn't. Broken versions of putup10.stm / putup11.stm have dosEof = 2.
		   || verMajor != 2
		   || (verMinor != 0 && verMinor != 10 && verMinor != 20 && verMinor != 21)
		   || numPatterns > 64
		   || (globalVolume > 64 && globalVolume != 0x58))  // 0x58 may be a placeholder value in earlier ST2 versions.
		{
			return false;
		}
		return ValidateTrackerName(trackerName);
	}

	static bool ValidateTrackerName(const char (&trackerName)[8])
	{
		// Tracker string can be anything really (ST2 and ST3 won't check it),
		// but we do not want to generate too many false positives here, as
		// STM already has very few magic bytes anyway.
		// Magic bytes that have been found in the wild are !Scream!, BMOD2STM, WUZAMOD! and SWavePro.
		for(uint8 c : trackerName)
		{
			if(c < 0x20 || c >= 0x7F)
				return false;
		}
		return true;
	}

	uint64 GetHeaderMinimumAdditionalSize() const
	{
		return 31 * sizeof(STMSampleHeader) + (verMinor == 0 ? 64 : 128) + numPatterns * 64 * 4;
	}
};

MPT_BINARY_STRUCT(STMFileHeader, 48)


static bool ValidateSTMOrderList(ModSequence &order)
{
	for(auto &pat : order)
	{
		if(pat == 99 || pat == 255)  // 99 is regular, sometimes a single 255 entry can be found too
			pat = PATTERNINDEX_INVALID;
		else if(pat > 63)
			return false;
	}
	return true;
}


static void ConvertSTMCommand(ModCommand &m, const uint8 command, const ROWINDEX row, const uint8 fileVerMinor, uint8 &newTempo, ORDERINDEX &breakPos, ROWINDEX &breakRow)
{
	static constexpr EffectCommand stmEffects[] =
	{
		CMD_NONE,        CMD_SPEED,          CMD_POSITIONJUMP, CMD_PATTERNBREAK,   // .ABC
		CMD_VOLUMESLIDE, CMD_PORTAMENTODOWN, CMD_PORTAMENTOUP, CMD_TONEPORTAMENTO, // DEFG
		CMD_VIBRATO,     CMD_TREMOR,         CMD_ARPEGGIO,     CMD_NONE,           // HIJK
		CMD_NONE,        CMD_NONE,           CMD_NONE,         CMD_NONE,           // LMNO
		// KLMNO can be entered in the editor but don't do anything
	};

	m.command = stmEffects[command & 0x0F];

	switch(m.command)
	{
	case CMD_VOLUMESLIDE:
		// Lower nibble always has precedence, and there are no fine slides.
		if(m.param & 0x0F)
			m.param &= 0x0F;
		else
			m.param &= 0xF0;
		break;

	case CMD_PATTERNBREAK:
		m.param = static_cast<ModCommand::PARAM>((m.param & 0xF0) * 10 + (m.param & 0x0F));
		if(breakPos != ORDERINDEX_INVALID && m.param == 0)
		{
			// Merge Bxx + C00 into just Bxx
			m.command = CMD_POSITIONJUMP;
			m.param = static_cast<ModCommand::PARAM>(breakPos);
			breakPos = ORDERINDEX_INVALID;
		}
		LimitMax(breakRow, row);
		break;

	case CMD_POSITIONJUMP:
		// This effect is also very weird.
		// Bxx doesn't appear to cause an immediate break -- it merely
		// sets the next order for when the pattern ends (either by
		// playing it all the way through, or via Cxx effect)
		breakPos = m.param;
		breakRow = 63;
		m.command = CMD_NONE;
		break;

	case CMD_TREMOR:
		// this actually does something with zero values, and has no
		// effect memory. which makes SENSE for old-effects tremor,
		// but ST3 went and screwed it all up by adding an effect
		// memory and IT followed that, and those are much more popular
		// than STM so we kind of have to live with this effect being
		// broken... oh well. not a big loss.
		break;

	case CMD_SPEED:
		if(fileVerMinor < 21)
			m.param = static_cast<ModCommand::PARAM>(((m.param / 10u) << 4u) + m.param % 10u);

		if(!m.param)
		{
			m.command = CMD_NONE;
			break;
		}

#ifdef MODPLUG_TRACKER
		// ST2 has a very weird tempo mode where the length of a tick depends both
		// on the ticks per row and a scaling factor. Try to write the tempo into a separate command.
		newTempo = m.param;
		m.param >>= 4;
#else
		MPT_UNREFERENCED_PARAMETER(newTempo);
#endif // MODPLUG_TRACKER
		break;

	default:
		// Anything not listed above is a no-op if there's no value, as ST2 doesn't have effect memory.
		if(!m.param)
			m.command = CMD_NONE;
		break;
	}
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderSTM(MemoryFileReader file, const uint64 *pfilesize)
{
	STMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.Validate())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadSTM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	STMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.Validate())
		return false;
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(fileHeader.GetHeaderMinimumAdditionalSize())))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_STM, 4);

	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songname);

	m_modFormat.formatName = UL_("Scream Tracker 2");
	m_modFormat.type = UL_("stm");
	m_modFormat.charset = mpt::Charset::CP437;

	if(!std::memcmp(fileHeader.trackerName, "!Scream!", 8))
	{
		if(fileHeader.verMinor >= 21)
			m_modFormat.madeWithTracker = UL_("Scream Tracker 2.2 - 2.3 or compatible");
		else
			m_modFormat.madeWithTracker = MPT_UFORMAT("Scream Tracker {}.{} or compatible")(fileHeader.verMajor, mpt::ufmt::dec0<2>(fileHeader.verMinor));
	}
	else if(!std::memcmp(fileHeader.trackerName, "BMOD2STM", 8))
		m_modFormat.madeWithTracker = UL_("BMOD2STM");
	else if(!std::memcmp(fileHeader.trackerName, "WUZAMOD!", 8))
		m_modFormat.madeWithTracker = UL_("Wuzamod");
	else if(!std::memcmp(fileHeader.trackerName, "SWavePro", 8))
		m_modFormat.madeWithTracker = UL_("SoundWave Pro");
	else
		m_modFormat.madeWithTracker = UL_("Unknown");

	m_playBehaviour.set(kST3SampleSwap);

	m_nSamples = 31;
	m_nMinPeriod = 64;
	m_nMaxPeriod = 0x7FFF;
	
	m_playBehaviour.set(kST3SampleSwap);
	
	uint8 initTempo = fileHeader.initTempo;
	if(fileHeader.verMinor < 21)
		initTempo = static_cast<uint8>(((initTempo / 10u) << 4u) + initTempo % 10u);
	if(initTempo == 0)
		initTempo = 0x60;

	Order().SetDefaultTempo(ConvertST2Tempo(initTempo));
	Order().SetDefaultSpeed(initTempo >> 4);
	if(fileHeader.verMinor > 10)
		m_nDefaultGlobalVolume = std::min(fileHeader.globalVolume, uint8(64)) * 4u;

	// Read samples
	uint16 sampleOffsets[31];
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		STMSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		if(sampleHeader.zero != 0 && sampleHeader.zero != 46)  // putup10.stm has zero = 46
			return false;
		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.filename);
		sampleOffsets[smp - 1] = sampleHeader.offset;
	}

	// Read order list
	ReadOrderFromFile<uint8>(Order(), file, fileHeader.verMinor == 0 ? 64 : 128);
	if(!ValidateSTMOrderList(Order()))
		return false;

	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.numPatterns);
	for(PATTERNINDEX pat = 0; pat < fileHeader.numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			for(int i = 0; i < 64 * 4; i++)
			{
				uint8 note = file.ReadUint8();
				if(note < 0xFB || note > 0xFD)
					file.Skip(3);
			}
			continue;
		}

		auto m = Patterns[pat].begin();
		ORDERINDEX breakPos = ORDERINDEX_INVALID;
		ROWINDEX breakRow = 63;  // Candidate row for inserting pattern break

		for(ROWINDEX row = 0; row < 64; row++)
		{
			uint8 newTempo = 0;
			for(CHANNELINDEX chn = 0; chn < 4; chn++, m++)
			{
				uint8 note = file.ReadUint8(), insVol, volCmd, cmdInf;
				switch(note)
				{
				case 0xFB:
					note = insVol = volCmd = cmdInf = 0x00;
					break;
				case 0xFC:
					continue;
				case 0xFD:
					m->note = NOTE_NOTECUT;
					continue;
				default:
					{
					const auto patData = file.ReadArray<uint8, 3>();
					insVol = patData[0];
					volCmd = patData[1];
					cmdInf = patData[2];
					}
					break;
				}

				if(note == 0xFE)
					m->note = NOTE_NOTECUT;
				else if(note < 0x60)
					m->note = static_cast<ModCommand::NOTE>((note >> 4) * 12 + (note & 0x0F) + 36 + NOTE_MIN);

				m->instr = insVol >> 3;
				if(m->instr > 31)
				{
					m->instr = 0;
				}
			
				uint8 vol = (insVol & 0x07) | ((volCmd & 0xF0) >> 1);
				if(vol <= 64)
				{
					m->volcmd = VOLCMD_VOLUME;
					m->vol = vol;
				}

				m->param = cmdInf;
				ConvertSTMCommand(*m, volCmd & 0x0F, row, fileHeader.verMinor, newTempo, breakPos, breakRow);
			}
			if(newTempo != 0)
			{
				Patterns[pat].WriteEffect(EffectWriter(CMD_TEMPO, mpt::saturate_round<ModCommand::PARAM>(ConvertST2Tempo(newTempo).ToDouble())).Row(row).RetryPreviousRow());
			}
		}

		if(breakPos != ORDERINDEX_INVALID)
		{
			Patterns[pat].WriteEffect(EffectWriter(CMD_POSITIONJUMP, static_cast<ModCommand::PARAM>(breakPos)).Row(breakRow).RetryPreviousRow());
		}
	}

	// Reading Samples
	if(loadFlags & loadSampleData)
	{
		const SampleIO sampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM);

		for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
		{
			ModSample &sample = Samples[smp];
			// ST2 just plays random noise for samples with a default volume of 0
			if(sample.nLength && sample.nVolume > 0)
			{
				FileReader::pos_type sampleOffset = sampleOffsets[smp - 1] << 4;
				// acidlamb.stm has some bogus samples with sample offsets past EOF
				if(sampleOffset > sizeof(STMFileHeader) && file.Seek(sampleOffset))
				{
					sampleIO.ReadSample(sample, file);
				}
			}
		}
	}

	return true;
}


// STX file header
struct STXFileHeader
{
	char     songName[20];
	char     trackerName[8];  // Typically !Scream! but mustn't be relied upon, like for STM
	uint16le patternSize;     // or EOF in newer file version (except for future brain.stx?!)
	uint16le unknown1;
	uint16le patTableOffset;
	uint16le smpTableOffset;
	uint16le chnTableOffset;
	uint32le unknown2;
	uint8    globalVolume;
	uint8    initTempo;
	uint32le unknown3;
	uint16le numPatterns;
	uint16le numSamples;
	uint16le numOrders;
	char     unknown4[6];
	char     magic[4];

	bool Validate() const
	{
		if(std::memcmp(magic, "SCRM", 4)
		   || (patternSize < 64 && patternSize != 0x1A)
		   || patternSize > 0x840
		   || (globalVolume > 64 && globalVolume != 0x58)  // 0x58 may be a placeholder value in earlier ST2 versions.
		   || numPatterns > 64
		   || numSamples > 96  // Some STX files have more sample slots than their STM counterpart for mysterious reasons
		   || (numOrders > 0x81 && numOrders != 0x101)
		   || unknown1 != 0 || unknown2 != 0 || unknown3 != 1)
		{
			return false;
		}
		return STMFileHeader::ValidateTrackerName(trackerName);
	}

	uint64 GetHeaderMinimumAdditionalSize() const
	{
		return std::max({(patTableOffset << 4) + numPatterns * 2, (smpTableOffset << 4) + numSamples * 2, (chnTableOffset << 4) + 32 + numOrders * 5 });
	}
};

MPT_BINARY_STRUCT(STXFileHeader, 64)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderSTX(MemoryFileReader file, const uint64 *pfilesize)
{
	STXFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.Validate())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadSTX(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	STXFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.Validate())
		return false;
	if (!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(fileHeader.GetHeaderMinimumAdditionalSize())))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_STM, 4);

	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songName);

	m_nSamples = fileHeader.numSamples;
	m_nMinPeriod = 64;
	m_nMaxPeriod = 0x7FFF;

	m_playBehaviour.set(kST3SampleSwap);

	uint8 initTempo = fileHeader.initTempo;
	if(initTempo == 0)
		initTempo = 0x60;

	Order().SetDefaultTempo(ConvertST2Tempo(initTempo));
	Order().SetDefaultSpeed(initTempo >> 4);
	m_nDefaultGlobalVolume = std::min(fileHeader.globalVolume, uint8(64)) * 4u;

	std::vector<uint16le> patternOffsets, sampleOffsets;
	file.Seek(fileHeader.patTableOffset << 4);
	file.ReadVector(patternOffsets, fileHeader.numPatterns);
	file.Seek(fileHeader.smpTableOffset << 4);
	file.ReadVector(sampleOffsets, fileHeader.numSamples);

	// Read order list
	file.Seek((fileHeader.chnTableOffset << 4) + 32);
	Order().resize(fileHeader.numOrders);
	for(auto &pat : Order())
	{
		pat = file.ReadUint8();
		file.Skip(4);
	}
	if(!ValidateSTMOrderList(Order()))
		return false;

	// Read samples
	for(SAMPLEINDEX smp = 1; smp <= fileHeader.numSamples; smp++)
	{
		if(!file.Seek(sampleOffsets[smp - 1] << 4))
			return false;
		S3MSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.filename);
		const uint32 sampleOffset = sampleHeader.GetSampleOffset();
		if((loadFlags & loadSampleData) && sampleHeader.length != 0 && file.Seek(sampleOffset))
		{
			sampleHeader.GetSampleFormat(true).ReadSample(Samples[smp], file);
		}
	}

	// Read patterns
	uint8 formatVersion = 1;
	if(!patternOffsets.empty() && fileHeader.patternSize != 0x1A)
	{
		if(!file.Seek(patternOffsets.front() << 4))
			return false;
		// First two bytes describe pattern size, like in S3M
		if(file.ReadUint16LE() == fileHeader.patternSize)
			formatVersion = 0;
	}

	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.numPatterns);
	for(PATTERNINDEX pat = 0; pat < fileHeader.numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
			break;
		if(!file.Seek(patternOffsets[pat] << 4))
			return false;
		if(formatVersion == 0 && file.ReadUint16LE() > 0x840)
			return false;

		ORDERINDEX breakPos = ORDERINDEX_INVALID;
		ROWINDEX breakRow = 63;  // Candidate row for inserting pattern break

		auto rowBase = Patterns[pat].GetRow(0);
		ROWINDEX row = 0;
		uint8 newTempo = 0;
		while(row < 64)
		{
			uint8 info = file.ReadUint8();

			if(info == s3mEndOfRow)
			{
				// End of row
				if(newTempo != 0)
				{
					Patterns[pat].WriteEffect(EffectWriter(CMD_TEMPO, mpt::saturate_round<ModCommand::PARAM>(ConvertST2Tempo(newTempo).ToDouble())).Row(row).RetryPreviousRow());
					newTempo = 0;
				}

				if(++row < 64)
				{
					rowBase = Patterns[pat].GetRow(row);
				}
				continue;
			}

			CHANNELINDEX channel = (info & s3mChannelMask);
			ModCommand dummy;
			ModCommand &m = (channel < GetNumChannels()) ? rowBase[channel] : dummy;

			if(info & s3mNotePresent)
			{
				const auto [note, instr] = file.ReadArray<uint8, 2>();
				if(note < 0xF0)
					m.note = static_cast<ModCommand::NOTE>(Clamp((note & 0x0F) + 12 * (note >> 4) + 36 + NOTE_MIN, NOTE_MIN, NOTE_MAX));
				else if(note == s3mNoteOff)
					m.note = NOTE_NOTECUT;
				else if(note == s3mNoteNone)
					m.note = NOTE_NONE;
				m.instr = instr;
			}

			if(info & s3mVolumePresent)
			{
				uint8 volume = file.ReadUint8();
				m.volcmd = VOLCMD_VOLUME;
				m.vol = std::min(volume, uint8(64));
			}

			if(info & s3mEffectPresent)
			{
				const auto [command, param] = file.ReadArray<uint8, 2>();
				m.param = param;
				ConvertSTMCommand(m, command, row, 0xFF, newTempo, breakPos, breakRow);
			}
		}

		if(breakPos != ORDERINDEX_INVALID)
		{
			Patterns[pat].WriteEffect(EffectWriter(CMD_POSITIONJUMP, static_cast<ModCommand::PARAM>(breakPos)).Row(breakRow).RetryPreviousRow());
		}
	}

	m_modFormat.formatName = UL_("Scream Tracker Music Interface Kit");
	m_modFormat.type = UL_("stx");
	m_modFormat.charset = mpt::Charset::CP437;
	m_modFormat.madeWithTracker = MPT_UFORMAT("STM2STX 1.{}")(formatVersion);

	return true;
}


OPENMPT_NAMESPACE_END
