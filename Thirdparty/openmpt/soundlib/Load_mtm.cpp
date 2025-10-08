/*
 * Load_mtm.cpp
 * ------------
 * Purpose: MTM (MultiTracker) module loader
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

// File Header
struct MTMFileHeader
{
	char     id[3];          // MTM file marker
	uint8le  version;        // Tracker version
	char     songName[20];   // ASCIIZ songname
	uint16le numTracks;      // Number of tracks saved
	uint8le  lastPattern;    // Last pattern number saved
	uint8le  lastOrder;      // Last order number to play (songlength-1)
	uint16le commentSize;    // Length of comment field
	uint8le  numSamples;     // Number of samples saved
	uint8le  attribute;      // Attribute byte (unused)
	uint8le  beatsPerTrack;  // Numbers of rows in every pattern (MultiTracker itself does not seem to support values != 64)
	uint8le  numChannels;    // Number of channels used
	uint8le  panPos[32];     // Channel pan positions
};

MPT_BINARY_STRUCT(MTMFileHeader, 66)


// Sample Header
struct MTMSampleHeader
{
	char     samplename[22];
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
	int8le   finetune;
	uint8le  volume;
	uint8le  attribute;

	// Convert an MTM sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nVolume = std::min(uint16(volume * 4), uint16(256));
		if(length > 2)
		{
			mptSmp.nLength = length;
			mptSmp.nLoopStart = loopStart;
			mptSmp.nLoopEnd = std::max(loopEnd.get(), uint32(1)) - 1;
			LimitMax(mptSmp.nLoopEnd, mptSmp.nLength);
			if(mptSmp.nLoopStart + 4 >= mptSmp.nLoopEnd)
				mptSmp.nLoopStart = mptSmp.nLoopEnd = 0;
			if(mptSmp.nLoopEnd > 2)
				mptSmp.uFlags.set(CHN_LOOP);
			mptSmp.nFineTune = finetune; // Uses MOD units but allows the full int8 range rather than just -8...+7 so we keep the value as-is and convert it during playback
			mptSmp.nC5Speed = ModSample::TransposeToFrequency(0, finetune * 16);

			if(attribute & 0x01)
			{
				mptSmp.uFlags.set(CHN_16BIT);
				mptSmp.nLength /= 2;
				mptSmp.nLoopStart /= 2;
				mptSmp.nLoopEnd /= 2;
			}
		}
	}
};

MPT_BINARY_STRUCT(MTMSampleHeader, 37)


static bool ValidateHeader(const MTMFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.id, "MTM", 3)
		|| fileHeader.version >= 0x20
		|| fileHeader.lastOrder > 127
		|| fileHeader.beatsPerTrack > 64
		|| fileHeader.numChannels > 32
		|| fileHeader.numChannels == 0
		)
	{
		return false;
	}
	return true;
}


static uint64 GetHeaderMinimumAdditionalSize(const MTMFileHeader &fileHeader)
{
	return sizeof(MTMSampleHeader) * fileHeader.numSamples + 128 + 192 * fileHeader.numTracks + 64 * (fileHeader.lastPattern + 1) + fileHeader.commentSize;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMTM(MemoryFileReader file, const uint64 *pfilesize)
{
	MTMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	return ProbeAdditionalSize(file, pfilesize, GetHeaderMinimumAdditionalSize(fileHeader));
}


bool CSoundFile::ReadMTM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	MTMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_MTM, fileHeader.numChannels);
	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songName);
	m_nSamples = fileHeader.numSamples;
	
	m_modFormat.formatName = UL_("MultiTracker");
	m_modFormat.type = UL_("mtm");
	m_modFormat.madeWithTracker = MPT_UFORMAT("MultiTracker {}.{}")(fileHeader.version >> 4, fileHeader.version & 0x0F);
	m_modFormat.charset = mpt::Charset::CP437;

	// Reading instruments
	for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
	{
		MTMSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.samplename);
	}

	// Setting Channel Pan Position
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = ((fileHeader.panPos[chn] & 0x0F) << 4) + 8;
	}

	// Reading pattern order
	uint8 orders[128];
	file.ReadArray(orders);
	ReadOrderFromArray(Order(), orders, fileHeader.lastOrder + 1, 0xFF, 0xFE);

	// Reading Patterns
	const ROWINDEX rowsPerPat = fileHeader.beatsPerTrack ? fileHeader.beatsPerTrack : 64;
	FileReader tracks = file.ReadChunk(192 * fileHeader.numTracks);

	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.lastPattern + 1);

	bool hasSpeed = false, hasTempo = false;
	for(PATTERNINDEX pat = 0; pat <= fileHeader.lastPattern; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, rowsPerPat))
		{
			file.Skip(64);
			continue;
		}

		for(CHANNELINDEX chn = 0; chn < 32; chn++)
		{
			uint16 track = file.ReadUint16LE();
			if(track == 0 || track > fileHeader.numTracks || chn >= GetNumChannels())
			{
				continue;
			}

			tracks.Seek(192 * (track - 1));

			ModCommand *m = Patterns[pat].GetpModCommand(0, chn);
			for(ROWINDEX row = 0; row < rowsPerPat; row++, m += GetNumChannels())
			{
				const auto [noteInstr, instrCmd, par] = tracks.ReadArray<uint8, 3>();

				if(noteInstr & 0xFC)
					m->note = (noteInstr >> 2) + 36 + NOTE_MIN;
				m->instr = ((noteInstr & 0x03) << 4) | (instrCmd >> 4);
				uint8 cmd = instrCmd & 0x0F;
				uint8 param = par;
				if(cmd == 0x0A)
				{
					if(param & 0xF0) param &= 0xF0; else param &= 0x0F;
				} else if(cmd == 0x08)
				{
					// No 8xx panning in MultiTracker, only E8x
					cmd = param = 0;
				} else if(cmd == 0x0E)
				{
					// MultiTracker does not support these commands
					switch(param & 0xF0)
					{
					case 0x00:
					case 0x30:
					case 0x40:
					case 0x60:
					case 0x70:
					case 0xF0:
						cmd = param = 0;
						break;
					}
				}
				if(cmd != 0 || param != 0)
				{
					ConvertModCommand(*m, cmd, param);
#ifdef MODPLUG_TRACKER
					m->Convert(MOD_TYPE_MTM, MOD_TYPE_S3M, *this);
#endif
					if(m->command == CMD_SPEED)
						hasSpeed = true;
					else if(m->command == CMD_TEMPO)
						hasTempo = true;
				}
			}
		}
	}

	// Curiously, speed commands reset the tempo to 125 in MultiTracker, and tempo commands reset the speed to 6.
	// External players of the time (e.g. DMP) did not implement this quirk and assumed a more ProTracker-like interpretation of speed and tempo.
	// Quite a few musicians created MTMs that make use DMP's speed and tempo interpretation, which in return means that they will play too
	// fast or too slow in MultiTracker. On the other hand there are also a few MTMs that break when using ProTracker-like speed and tempo.
	// As a way to support as many modules of both types as possible, we will assume a ProTracker-like interpretation if both speed and tempo
	// commands are found on the same line, and a MultiTracker-like interpretation when they are never found on the same line.
	if(hasSpeed && hasTempo)
	{
		bool hasSpeedAndTempoOnSameRow = false;
		for(const auto &pattern : Patterns)
		{
			for(ROWINDEX row = 0; row < pattern.GetNumRows(); row++)
			{
				bool hasSpeedOnRow = false, hasTempoOnRow = false;
				for(const ModCommand &m : pattern.GetRow(row))
				{
					if(m.command == CMD_SPEED)
						hasSpeedOnRow = true;
					else if(m.command == CMD_TEMPO)
						hasTempoOnRow = true;
				}
				if(hasSpeedOnRow && hasTempoOnRow)
				{
					hasSpeedAndTempoOnSameRow = true;
					break;
				}
			}
			if(hasSpeedAndTempoOnSameRow)
				break;
		}

		if(!hasSpeedAndTempoOnSameRow)
		{
			for(auto &pattern : Patterns)
			{
				for(ROWINDEX row = 0; row < pattern.GetNumRows(); row++)
				{
					for(const ModCommand &m : pattern.GetRow(row))
					{
						if(m.command == CMD_SPEED || m.command == CMD_TEMPO)
						{
							const bool writeTempo = m.command == CMD_SPEED;
							pattern.WriteEffect(EffectWriter(writeTempo ? CMD_TEMPO : CMD_SPEED, writeTempo ? 125 : 6).Row(row));
							break;
						}
					}
				}
			}
		}
	}

	if(fileHeader.commentSize != 0)
	{
		// Read message with a fixed line length of 40 characters
		// (actually the last character is always null, so make that 39 + 1 padding byte)
		m_songMessage.ReadFixedLineLength(file, fileHeader.commentSize, 39, 1);
	}

	// Reading Samples
	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
		{
			SampleIO(
				Samples[smp].uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::unsignedPCM)
				.ReadSample(Samples[smp], file);
		}
	}

	m_nMinPeriod = 64;
	m_nMaxPeriod = 32767;
	return true;
}


OPENMPT_NAMESPACE_END
