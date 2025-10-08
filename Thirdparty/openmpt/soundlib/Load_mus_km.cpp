/*
 * Load_mus_km.cpp
 * ---------------
 * Purpose: Karl Morton Music Format module loader
 * Notes  : This is probably not the official name of this format.
 *          Karl Morton's engine has been used in Psycho Pinball and Micro Machines 2 and also Back To Baghdad
 *          but the latter game only uses its sound effect format, not the music format.
 *          So there are only two known games using this music format, and no official tools or documentation are available.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct KMChunkHeader
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idSONG = MagicLE("SONG"),
		idSMPL = MagicLE("SMPL"),
	};

	uint32le id;      // See ChunkIdentifiers
	uint32le length;  // Chunk size including header

	size_t GetLength() const
	{
		return length <= 8 ? 0 : (length - 8);
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}

};

MPT_BINARY_STRUCT(KMChunkHeader, 8)


struct KMSampleHeader
{
	char name[32];
	uint32le loopStart;
	uint32le size;
};

MPT_BINARY_STRUCT(KMSampleHeader, 40)


struct KMSampleReference
{
	char name[32];
	uint8 finetune;
	uint8 volume;
};

MPT_BINARY_STRUCT(KMSampleReference, 34)


struct KMSongHeader
{
	char name[32];
	KMSampleReference samples[31];

	uint16le unknown;  // always 0
	uint32le numChannels;
	uint32le restartPos;
	uint32le musicSize;
};

MPT_BINARY_STRUCT(KMSongHeader, 32 + 31 * 34 + 14)


struct KMFileHeader
{
	KMChunkHeader chunkHeader;
	KMSongHeader songHeader;
};

MPT_BINARY_STRUCT(KMFileHeader, sizeof(KMChunkHeader) + sizeof(KMSongHeader))


static uint64 GetHeaderMinimumAdditionalSize(const KMFileHeader &fileHeader)
{
	// Require room for at least one more sample chunk header
	return static_cast<uint64>(fileHeader.songHeader.musicSize) + sizeof(KMChunkHeader);
}


// Check if string only contains printable characters and doesn't contain any garbage after the required terminating null
static bool IsValidKMString(const char (&str)[32])
{
	bool nullFound = false;
	for(char c : str)
	{
		if(c > 0x00 && c < 0x20)
			return false;
		else if(c == 0x00)
			nullFound = true;
		else if(nullFound)
			return false;
	}
	return nullFound;
}


static bool ValidateHeader(const KMFileHeader &fileHeader)
{
	if(fileHeader.chunkHeader.id != KMChunkHeader::idSONG
	   || fileHeader.chunkHeader.length < sizeof(fileHeader)
	   || fileHeader.chunkHeader.length - sizeof(fileHeader) != fileHeader.songHeader.musicSize
	   || fileHeader.chunkHeader.length > 0x40000  // That's enough space for 256 crammed 64-row patterns ;)
	   || fileHeader.songHeader.unknown != 0
	   || fileHeader.songHeader.numChannels < 1
	   || fileHeader.songHeader.numChannels > 4  // Engine rejects anything above 32, channels 5 to 32 are simply ignored
	   || !IsValidKMString(fileHeader.songHeader.name))
	{
		return false;
	}

	for(const auto &sample : fileHeader.songHeader.samples)
	{
		if(sample.finetune > 15 || sample.volume > 64 || !IsValidKMString(sample.name))
			return false;
	}

	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMUS_KM(MemoryFileReader file, const uint64 *pfilesize)
{
	KMFileHeader fileHeader;
	if(!file.Read(fileHeader))
		return ProbeWantMoreData;
	if(!ValidateHeader(fileHeader))
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, GetHeaderMinimumAdditionalSize(fileHeader));
}


bool CSoundFile::ReadMUS_KM(FileReader &file, ModLoadingFlags loadFlags)
{
	{
		file.Rewind();
		KMFileHeader fileHeader;
		if(!file.Read(fileHeader))
			return false;
		if(!ValidateHeader(fileHeader))
			return false;
		if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
			return false;
		if(loadFlags == onlyVerifyHeader)
			return true;
	}

	file.Rewind();

	const auto chunks = ChunkReader(file).ReadChunks<KMChunkHeader>(1);
	auto songChunks = chunks.GetAllChunks(KMChunkHeader::idSONG);
	auto sampleChunks = chunks.GetAllChunks(KMChunkHeader::idSMPL);

	if(songChunks.empty() || sampleChunks.empty())
		return false;

	InitializeGlobals(MOD_TYPE_MOD, 4);
	m_SongFlags = SONG_AMIGALIMITS | SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL | SONG_ISAMIGA;  // Yes, those were not Amiga games but the format fully conforms to Amiga limits, so allow the Amiga Resampler to be used.
	m_nSamples = 0;

	static constexpr uint16 MUS_SAMPLE_UNUSED = 255;  // Sentinel value to check if a sample needs to be duplicated
	for(auto &chunk : sampleChunks)
	{
		if(!CanAddMoreSamples())
			break;
		m_nSamples++;
		ModSample &mptSample = Samples[m_nSamples];
		mptSample.Initialize(MOD_TYPE_MOD);

		KMSampleHeader sampleHeader;
		if(!chunk.Read(sampleHeader)
		   || !IsValidKMString(sampleHeader.name))
			return false;

		m_szNames[m_nSamples] = sampleHeader.name;
		mptSample.nLoopEnd = mptSample.nLength = sampleHeader.size;
		mptSample.nLoopStart = sampleHeader.loopStart;
		mptSample.uFlags.set(CHN_LOOP);
		mptSample.nVolume = MUS_SAMPLE_UNUSED;

		if(!(loadFlags & loadSampleData))
			continue;

		SampleIO(SampleIO::_8bit,
		         SampleIO::mono,
		         SampleIO::littleEndian,
		         SampleIO::signedPCM)
		    .ReadSample(mptSample, chunk);
	}
	
	bool firstSong = true;
	for(auto &chunk : songChunks)
	{
		if(!firstSong && !Order.AddSequence())
			break;
		firstSong = false;
		Order().clear();

		KMSongHeader songHeader;
		if(!chunk.Read(songHeader)
		   || songHeader.unknown != 0
		   || songHeader.numChannels < 1
		   || songHeader.numChannels > 4)
			return false;
		
		Order().SetName(mpt::ToUnicode(mpt::Charset::CP437, songHeader.name));

		FileReader musicData = (loadFlags & loadPatternData) ? chunk.ReadChunk(songHeader.musicSize) : FileReader{};

		// Map the samples for this subsong
		std::array<SAMPLEINDEX, 32> sampleMap{};
		for(uint8 smp = 1; smp <= 31; smp++)
		{
			const auto &srcSample = songHeader.samples[smp - 1];
			const auto srcName = mpt::String::ReadAutoBuf(srcSample.name);
			if(srcName.empty())
				continue;
			if(srcSample.finetune > 15 || srcSample.volume > 64 || !IsValidKMString(srcSample.name))
				return false;

			const auto finetune = MOD2XMFineTune(srcSample.finetune);
			const uint16 volume = srcSample.volume * 4u;

			SAMPLEINDEX copyFrom = 0;
			for(SAMPLEINDEX srcSmp = 1; srcSmp <= m_nSamples; srcSmp++)
			{
				if(srcName != m_szNames[srcSmp])
					continue;

				auto &mptSample = Samples[srcSmp];
				sampleMap[smp] = srcSmp;
				if(mptSample.nVolume == MUS_SAMPLE_UNUSED
				   || (mptSample.nFineTune == finetune && mptSample.nVolume == volume))
				{
					// Sample was not used yet, or it uses the same finetune and volume
					mptSample.nFineTune = finetune;
					mptSample.nVolume = volume;
					copyFrom = 0;
					break;
				} else
				{
					copyFrom = srcSmp;
				}
			}
			if(copyFrom && CanAddMoreSamples())
			{
				m_nSamples++;
				sampleMap[smp] = m_nSamples;
				const auto &smpFrom = Samples[copyFrom];
				auto &newSample = Samples[m_nSamples];
				newSample.FreeSample();
				newSample = smpFrom;
				newSample.nFineTune = finetune;
				newSample.nVolume = volume;
				newSample.CopyWaveform(smpFrom);
				m_szNames[m_nSamples] = m_szNames[copyFrom];
			}
		}

		struct ChannelState
		{
			ModCommand prevCommand;
			uint8 repeat = 0;
		};
		std::array<ChannelState, 4> chnStates{};

		static constexpr ROWINDEX MUS_PATTERN_LENGTH = 64;
		const CHANNELINDEX numChannels = static_cast<CHANNELINDEX>(songHeader.numChannels);
		PATTERNINDEX pat = PATTERNINDEX_INVALID;
		ROWINDEX row = MUS_PATTERN_LENGTH;
		ROWINDEX restartRow = 0;
		uint32 repeatsLeft = 0;
		while(repeatsLeft || musicData.CanRead(1))
		{
			row++;
			if(row >= MUS_PATTERN_LENGTH)
			{
				pat = Patterns.InsertAny(MUS_PATTERN_LENGTH);
				if(pat == PATTERNINDEX_INVALID)
					break;

				Order().push_back(pat);
				row = 0;
			}

			ModCommand *m = Patterns[pat].GetpModCommand(row, 0);
			for(CHANNELINDEX chn = 0; chn < numChannels; chn++, m++)
			{
				auto &chnState = chnStates[chn];
				if(chnState.repeat)
				{
					chnState.repeat--;
					repeatsLeft--;
					*m = chnState.prevCommand;
					continue;
				}

				if(!musicData.CanRead(1))
					continue;

				if(musicData.GetPosition() == songHeader.restartPos)
				{
					Order().SetRestartPos(Order().GetLastIndex());
					restartRow = row;
				}

				const uint8 note = musicData.ReadUint8();
				if(note & 0x80)
				{
					chnState.repeat = note & 0x7F;
					repeatsLeft += chnState.repeat;
					*m = chnState.prevCommand;
					continue;
				}

				if(note > 0 && note <= 3 * 12)
					m->note = note + NOTE_MIDDLEC - 13;

				const auto instr = musicData.ReadUint8();
				m->instr = static_cast<ModCommand::INSTR>(sampleMap[instr & 0x1F]);

				if(instr & 0x80)
				{
					m->command = chnState.prevCommand.command;
					m->param = chnState.prevCommand.param;
				} else
				{
					static constexpr struct { ModCommand::COMMAND command; uint8 mask; } effTrans[] =
					{
						{CMD_VOLUME,         0x00}, {CMD_MODCMDEX,       0xA0}, {CMD_MODCMDEX,    0xB0}, {CMD_MODCMDEX,       0x10},
						{CMD_MODCMDEX,       0x20}, {CMD_MODCMDEX,       0x50}, {CMD_OFFSET,      0x00}, {CMD_TONEPORTAMENTO, 0x00},
						{CMD_TONEPORTAVOL,   0x00}, {CMD_VIBRATO,        0x00}, {CMD_VIBRATOVOL,  0x00}, {CMD_ARPEGGIO,       0x00},
						{CMD_PORTAMENTOUP,   0x00}, {CMD_PORTAMENTODOWN, 0x00}, {CMD_VOLUMESLIDE, 0x00}, {CMD_MODCMDEX,       0x90},
						{CMD_TONEPORTAMENTO, 0xFF}, {CMD_MODCMDEX,       0xC0}, {CMD_SPEED,       0x00}, {CMD_TREMOLO,        0x00},
					};
					
					const auto [command, param] = musicData.ReadArray<uint8, 2>();
					if(command < std::size(effTrans))
					{
						m->command = effTrans[command].command;
						m->param = param;
						if(m->command == CMD_SPEED && m->param >= 0x20)
							m->command = CMD_TEMPO;
						else if(effTrans[command].mask)
							m->param = effTrans[command].mask | (m->param & 0x0F);
					}
				}

				chnState.prevCommand = *m;
			}
		}

		if((restartRow != 0 || row < (MUS_PATTERN_LENGTH - 1u)) && pat != PATTERNINDEX_INVALID)
		{
			Patterns[pat].WriteEffect(EffectWriter(CMD_PATTERNBREAK, static_cast<ModCommand::PARAM>(restartRow)).Row(row).RetryNextRow());
		}
	}

	Order.SetSequence(0);

	m_modFormat.formatName = UL_("Karl Morton Music Format");
	m_modFormat.type = UL_("mus");
	m_modFormat.charset = mpt::Charset::CP437;

	return true;
}

OPENMPT_NAMESPACE_END
