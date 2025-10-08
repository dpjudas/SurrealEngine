/*
 * Load_dsm.cpp
 * ------------
 * Purpose: - Digisound Interface Kit (DSIK) Internal Format (DSM v2 / RIFF) module loader
 *          - Dynamic Studio (DSM) module loader
 * Notes  : 1. There is also another fundamentally different DSIK DSM v1 module format, not handled here.
 *          MilkyTracker can load it, but the only files of this format seen in the wild are also
 *          available in their original format, so I did not bother implementing it so far.
 *
 *          2. S3M-style retrigger does not seem to exist - it is translated to volume slides by CONV.EXE,
 *          and J00 in S3M files is not converted either. S3M pattern loops (SBx) are not converted
 *          properly by CONV.EXE and completely ignored by PLAY.EXE.
 *          Command 8 (set panning) uses 00-80 for regular panning and A4 for surround, probably
 *          making DSIK one of the first applications to use this convention established by DSMI's AMF format.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN


/////////////////////////////////////////////////////////////////////
// DMS (DSIK) loader

struct DSMChunk
{
	char     magic[4];
	uint32le size;
};

MPT_BINARY_STRUCT(DSMChunk, 8)


struct DSMSongHeader
{
	char     songName[28];
	uint16le fileVersion;
	uint16le flags;
	uint16le orderPos;
	uint16le restartPos;
	uint16le numOrders;
	uint16le numSamples;
	uint16le numPatterns;
	uint16le numChannels;
	uint8le  globalVol;
	uint8le  mastervol;
	uint8le  speed;
	uint8le  bpm;
	uint8le  panPos[16];
	uint8le  orders[128];
};

MPT_BINARY_STRUCT(DSMSongHeader, 192)


struct DSMSampleHeader
{
	char     filename[13];
	uint16le flags;
	uint8le  volume;
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
	uint32le dataPtr;  // Interal sample pointer during playback in DSIK
	uint32le sampleRate;
	char     sampleName[28];

	// Convert a DSM sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.filename = mpt::String::ReadBuf(mpt::String::nullTerminated, filename);

		mptSmp.nC5Speed = sampleRate;
		mptSmp.uFlags.set(CHN_LOOP, (flags & 1) != 0);
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nVolume = std::min(volume.get(), uint8(64)) * 4;
	}

	// Retrieve the internal sample format flags for this sample.
	SampleIO GetSampleFormat() const
	{
		SampleIO sampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::unsignedPCM);
		if(flags & 0x40)
			sampleIO |= SampleIO::deltaPCM;	// fairlight.dsm by Comrade J
		else if(flags & 0x02)
			sampleIO |= SampleIO::signedPCM;
		if(flags & 0x04)
			sampleIO |= SampleIO::_16bit;
		return sampleIO;
	}
};

MPT_BINARY_STRUCT(DSMSampleHeader, 64)


struct DSMHeader
{
	char fileMagic0[4];
	char fileMagic1[4];
	char fileMagic2[4];
};

MPT_BINARY_STRUCT(DSMHeader, 12)


static bool ValidateHeader(const DSMHeader &fileHeader)
{
	if(!std::memcmp(fileHeader.fileMagic0, "RIFF", 4)
		&& !std::memcmp(fileHeader.fileMagic2, "DSMF", 4))
	{
		// "Normal" DSM files with RIFF header
		// <RIFF> <file size> <DSMF>
		return true;
	} else if(!std::memcmp(fileHeader.fileMagic0, "DSMF", 4))
	{
		// DSM files with alternative header
		// <DSMF> <4 bytes, usually 4x NUL or RIFF> <file size> <4 bytes, usually DSMF but not always>
		return true;
	} else
	{
		return false;
	}
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderDSM(MemoryFileReader file, const uint64 *pfilesize)
{
	DSMHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	if(std::memcmp(fileHeader.fileMagic0, "DSMF", 4) == 0)
	{
		if(!file.Skip(4))
		{
			return ProbeWantMoreData;
		}
	}
	DSMChunk chunkHeader;
	if(!file.ReadStruct(chunkHeader))
	{
		return ProbeWantMoreData;
	}
	if(std::memcmp(chunkHeader.magic, "SONG", 4))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadDSM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	DSMHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(std::memcmp(fileHeader.fileMagic0, "DSMF", 4) == 0)
	{
		file.Skip(4);
	}
	DSMChunk chunkHeader;
	if(!file.ReadStruct(chunkHeader))
	{
		return false;
	}
	// Technically, the song chunk could be anywhere in the file, but we're going to simplify
	// things by not using a chunk header here and just expect it to be right at the beginning.
	if(std::memcmp(chunkHeader.magic, "SONG", 4))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	DSMSongHeader songHeader;
	file.ReadStructPartial(songHeader, chunkHeader.size);
	if(songHeader.numOrders > 128 || songHeader.numChannels > 16 || songHeader.numPatterns > 256 || songHeader.restartPos > 128)
	{
		return false;
	}

	InitializeGlobals(MOD_TYPE_DSM, std::max(songHeader.numChannels.get(), uint16(1)));

	m_modFormat.formatName = UL_("DSIK Format");
	m_modFormat.type = UL_("dsm");
	m_modFormat.charset = mpt::Charset::CP437;

	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, songHeader.songName);
	Order().SetDefaultSpeed(songHeader.speed);
	Order().SetDefaultTempoInt(songHeader.bpm);
	m_nDefaultGlobalVolume = std::min(songHeader.globalVol.get(), uint8(64)) * 4u;
	if(!m_nDefaultGlobalVolume) m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	if(songHeader.mastervol == 0x80)
		m_nSamplePreAmp = std::min(256u / GetNumChannels(), 128u);
	else
		m_nSamplePreAmp = songHeader.mastervol & 0x7F;

	// Read channel panning
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		if(songHeader.panPos[chn] <= 0x80)
		{
			ChnSettings[chn].nPan = songHeader.panPos[chn] * 2;
		}
	}

	ReadOrderFromArray(Order(), songHeader.orders, songHeader.numOrders, 0xFF, 0xFE);
	if(songHeader.restartPos < songHeader.numOrders)
		Order().SetRestartPos(songHeader.restartPos);

	// Read pattern and sample chunks
	PATTERNINDEX patNum = 0;
	while(file.ReadStruct(chunkHeader))
	{
		FileReader chunk = file.ReadChunk(chunkHeader.size);

		if(!memcmp(chunkHeader.magic, "PATT", 4) && (loadFlags & loadPatternData))
		{
			// Read pattern
			if(!Patterns.Insert(patNum, 64))
			{
				continue;
			}
			chunk.Skip(2);

			ModCommand dummy{};
			ROWINDEX row = 0;
			while(chunk.CanRead(1) && row < 64)
			{
				uint8 flag = chunk.ReadUint8();
				if(!flag)
				{
					row++;
					continue;
				}

				CHANNELINDEX chn = (flag & 0x0F);
				ModCommand &m = (chn < GetNumChannels() ? *Patterns[patNum].GetpModCommand(row, chn) : dummy);

				if(flag & 0x80)
				{
					uint8 note = chunk.ReadUint8();
					if(note)
					{
						if(note <= 12 * 9) note += 11 + NOTE_MIN;
						m.note = note;
					}
				}
				if(flag & 0x40)
				{
					m.instr = chunk.ReadUint8();
				}
				if (flag & 0x20)
				{
					m.volcmd = VOLCMD_VOLUME;
					m.vol = std::min(chunk.ReadUint8(), uint8(64));
				}
				if(flag & 0x10)
				{
					auto [command, param] = chunk.ReadArray<uint8, 2>();
					ConvertModCommand(m, command, param);
				}
			}
			patNum++;
		} else if(!memcmp(chunkHeader.magic, "INST", 4) && CanAddMoreSamples())
		{
			// Read sample
			m_nSamples++;
			ModSample &sample = Samples[m_nSamples];

			DSMSampleHeader sampleHeader;
			chunk.ReadStruct(sampleHeader);
			sampleHeader.ConvertToMPT(sample);

			m_szNames[m_nSamples] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.sampleName);

			if(loadFlags & loadSampleData)
			{
				sampleHeader.GetSampleFormat().ReadSample(sample, chunk);
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////
// DSM (Dynamic Studio) loader


struct DSmSampleHeader
{
	char     name[22];
	uint8    type;
	uint16le length;
	uint8    finetune;
	uint8    volume;
	uint16le loopStart;
	uint16le loopLength;
	uint8    padding;

	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.nVolume = std::min(volume, uint8(64)) * 4u;
		mptSmp.nFineTune = MOD2XMFineTune(finetune);
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopStart + loopLength;
		mptSmp.uFlags.set(CHN_LOOP, loopLength > 2);
		mptSmp.uFlags.set(CHN_16BIT, type == 16);
	}
};

MPT_BINARY_STRUCT(DSmSampleHeader, 32)


struct DSmFileHeader
{
	char  magic[4];  // "DSm\x1A"
	uint8 version;
	char  title[20];
	char  artist[20];
	uint8 numChannels;
	uint8 numSamples;
	uint8 numOrders;
	uint8 packInformation;
	uint8 globalVol;  // 0...100
	char  padding[14];

	bool IsValid() const noexcept
	{
		return !memcmp(magic, "DSm\x1A", 4)
			&& version == 0x20
			&& numChannels >= 1 && numChannels <= 16
			&& numSamples > 0
			&& numOrders > 0
			&& globalVol <= 100;
	}

	uint32 GetHeaderMinimumAdditionalSize() const noexcept
	{
		return static_cast<uint32>(numChannels + numOrders + numSamples * sizeof(DSmSampleHeader));
	}
};

MPT_BINARY_STRUCT(DSmFileHeader, 64)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderDSm(MemoryFileReader file, const uint64 *pfilesize)
{
	DSmFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadDSm(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	DSmFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;
	if(!file.CanRead(fileHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, fileHeader.numChannels);
	m_SongFlags = SONG_IMPORTED;
	static_assert(MAX_BASECHANNELS >= 32 && MAX_SAMPLES > 255);
	m_nSamples = fileHeader.numSamples;
	m_nDefaultGlobalVolume = Util::muldivr_unsigned(fileHeader.globalVol, MAX_GLOBAL_VOLUME, 100);

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.title);
	m_songArtist = mpt::ToUnicode(mpt::Charset::CP437, mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.artist));

	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = (file.ReadUint8() & 0x0F) * 0x11;
	}
	
	ReadOrderFromFile<uint8>(Order(), file, fileHeader.numOrders);
	PATTERNINDEX numPatterns = 0;
	for(PATTERNINDEX pat : Order())
	{
		numPatterns = std::max(pat, numPatterns);
	}
	numPatterns++;

	if(!file.CanRead((numPatterns * GetNumChannels() * 8) + (m_nSamples * sizeof(DSmSampleHeader)) + (numPatterns * GetNumChannels() * 64 * 4)))
		return false;

	// Track names for each pattern - we only read the track names of the first pattern
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].szName = mpt::String::ReadBuf(mpt::String::spacePadded, file.ReadArray<char, 8>());
	}
	file.Skip((numPatterns - 1) * GetNumChannels() * 8);

	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		DSmSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(Samples[smp]);
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);
	}

	Patterns.ResizeArray(numPatterns);
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			file.Skip(GetNumChannels() * 64 * 4);
			continue;
		}
		for(ModCommand &m : Patterns[pat])
		{
			const auto data = file.ReadArray<uint8, 4>();
			if(data[1] > 0 && data[1] <= 84 * 2)
				m.note = (data[1] >> 1) + NOTE_MIN + 35;
			m.instr = data[0];
			m.param = data[3];
			if(data[2] == 0x08)
			{
				switch(m.param & 0xF0)
				{
				case 0x00:  // 4-bit panning
					m.command = CMD_MODCMDEX;
					m.param |= 0x80;
					break;
				case 0x10:  // Default volume slide Up (should stop at sample's default volume)
					m.command = CMD_VOLUMESLIDE;
					m.param <<= 4;
					break;
				case 0x20:  // Default fine volume slide Up (should stop at sample's default volume)
					m.command = CMD_MODCMDEX;
					m.param |= 0xA0;
					break;
				case 0x30:  // Fine porta up (support all 5 octaves)
				case 0x40:  // Fine porta down (support all 5 octaves)
					m.command = CMD_MODCMDEX;
					m.param -= 0x20;
					break;
				default:
					break;
				}
			} else if(data[2] == 0x13)
			{
				// 3D Simulate
				m.command = CMD_PANNING8;
				uint32 param = (m.param & 0x7F) * 2u;
				if(m.param <= 0x40)  // 00 Front -> 40 Right
					param += 0x80;
				else if(m.param < 0x80)  // 40 Right -> 80 Back
					param = 0x180 - param;
				else if(m.param < 0xC0)  // 80 Back -> C0 Left
					param = 0x80 - param;
				else  // C0 Left -> FF Front
					param -= 0x80;
				m.param = mpt::saturate_cast<ModCommand::PARAM>(param);
			} else if((data[2] & 0xF0) == 0x20)
			{
				// Offset + volume
				m.command = CMD_OFFSET;
				m.SetVolumeCommand(VOLCMD_VOLUME, static_cast<ModCommand::VOL>((data[2] & 0x0F) * 4 + 4));
			} else if(data[2] <= 0x0F || data[2] == 0x11 || data[2] == 0x12)
			{
				// 0x11 and 0x12 support the full 5-octave range, 0x01 and 0x02 presumably only the ProTracker 3-octave range
				ConvertModCommand(m, data[2] & 0x0F, data[3]);
			}
		}
	}

	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
		{
			SampleIO(Samples[smp].uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM).ReadSample(Samples[smp], file);
		}
	}

	m_modFormat.formatName = UL_("Dynamic Studio");
	m_modFormat.type = UL_("dsm");
	m_modFormat.charset = mpt::Charset::CP437;

	return true;
}

OPENMPT_NAMESPACE_END
