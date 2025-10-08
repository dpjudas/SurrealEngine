/*
 * Load_dtm.cpp
 * ------------
 * Purpose: Digital Tracker / Digital Home Studio module Loader (DTM)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

enum PatternFormats : uint32
{
	DTM_PT_PATTERN_FORMAT = 0,
	DTM_204_PATTERN_FORMAT = MagicBE("2.04"),
	DTM_206_PATTERN_FORMAT = MagicBE("2.06"),
};


struct DTMFileHeader
{
	char     magic[4];
	uint32be headerSize;
	uint16be type;        // 0 = module
	uint8be  stereoMode;  // FF = panoramic stereo, 00 = old stereo
	uint8be  bitDepth;    // Typically 8, sometimes 16, but is not actually used anywhere?
	uint16be reserved;    // Usually 0, but not in unknown title 1.dtm and unknown title 2.dtm
	uint16be speed;
	uint16be tempo;
	uint32be forcedSampleRate; // Seems to be ignored in newer files
};

MPT_BINARY_STRUCT(DTMFileHeader, 22)


// IFF-style Chunk
struct DTMChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idS_Q_ = MagicBE("S.Q."),
		idPATT = MagicBE("PATT"),
		idINST = MagicBE("INST"),
		idIENV = MagicBE("IENV"),
		idDAPT = MagicBE("DAPT"),
		idDAIT = MagicBE("DAIT"),
		idTEXT = MagicBE("TEXT"),
		idPATN = MagicBE("PATN"),
		idTRKN = MagicBE("TRKN"),
		idVERS = MagicBE("VERS"),
		idSV19 = MagicBE("SV19"),
	};

	uint32be id;
	uint32be length;

	size_t GetLength() const
	{
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(DTMChunk, 8)


struct DTMSample
{
	uint32be reserved;   // 0x204 for first sample, 0x208 for second, etc...
	uint32be length;     // in bytes
	uint8be  finetune;   // -8....7
	uint8be  volume;     // 0...64
	uint32be loopStart;  // in bytes
	uint32be loopLength; // ditto
	char     name[22];
	uint8be  stereo;
	uint8be  bitDepth;
	uint16be transpose;
	uint16be unknown;
	uint32be sampleRate;

	void ConvertToMPT(ModSample &mptSmp, uint32 forcedSampleRate, uint32 formatVersion) const
	{
		mptSmp.Initialize(MOD_TYPE_IT);
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = mptSmp.nLoopStart + loopLength;
		// In revolution to come.dtm, the file header says samples rate is 24512 Hz, but samples say it's 50000 Hz
		// Digital Home Studio ignores the header setting in 2.04-/2.06-style modules
		mptSmp.nC5Speed = (formatVersion == DTM_PT_PATTERN_FORMAT && forcedSampleRate > 0) ? forcedSampleRate : sampleRate;
		int32 transposeAmount = 0;
#ifdef MODPLUG_TRACKER
		transposeAmount = MOD2XMFineTune(finetune);
#else
		mptSmp.nFineTune = MOD2XMFineTune(finetune);
#endif
		if(formatVersion == DTM_206_PATTERN_FORMAT && transpose > 0 && transpose != 48)
		{
			// Digital Home Studio applies this unconditionally, but some old songs sound wrong then (delirium.dtm).
			// Digital Tracker 2.03 ignores the setting.
			// Maybe this should not be applied for "real" Digital Tracker modules?
			transposeAmount += (48 - transpose) * 128;
		}
		mptSmp.Transpose(transposeAmount * (1.0 / (12.0 * 128.0)));
		mptSmp.nVolume = std::min(volume.get(), uint8(64)) * 4u;
		if(stereo & 1)
		{
			mptSmp.uFlags.set(CHN_STEREO);
			mptSmp.nLength /= 2u;
			mptSmp.nLoopStart /= 2u;
			mptSmp.nLoopEnd /= 2u;
		}
		if(bitDepth > 8)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nLength /= 2u;
			mptSmp.nLoopStart /= 2u;
			mptSmp.nLoopEnd /= 2u;
		}
		if(mptSmp.nLoopEnd > mptSmp.nLoopStart + 1)
		{
			mptSmp.uFlags.set(CHN_LOOP);
		} else
		{
			mptSmp.nLoopStart = mptSmp.nLoopEnd = 0;
		}
	}
};

MPT_BINARY_STRUCT(DTMSample, 50)


struct DTMInstrument
{
	uint16be insNum;
	uint8be  unknown1;
	uint8be  envelope; // 0xFF = none
	uint8be  sustain;  // 0xFF = no sustain point
	uint16be fadeout;
	uint8be  vibRate;
	uint8be  vibDepth;
	uint8be  modulationRate;
	uint8be  modulationDepth;
	uint8be  breathRate;
	uint8be  breathDepth;
	uint8be  volumeRate;
	uint8be  volumeDepth;
};

MPT_BINARY_STRUCT(DTMInstrument, 15)


struct DTMEnvelope
{
	struct DTMEnvPoint
	{
		uint8be value;
		uint8be tick;
	};
	uint16be numPoints;
	DTMEnvPoint points[16];
};

MPT_BINARY_STRUCT(DTMEnvelope::DTMEnvPoint, 2)
MPT_BINARY_STRUCT(DTMEnvelope, 34)


struct DTMText
{
	uint16be textType;	// 0 = pattern, 1 = free, 2 = song
	uint32be textLength;
	uint16be tabWidth;
	uint16be reserved;
	uint16be oddLength;
};

MPT_BINARY_STRUCT(DTMText, 12)


static bool ValidateHeader(const DTMFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.magic, "D.T.", 4)
		|| fileHeader.headerSize < sizeof(fileHeader) - 8u
		|| fileHeader.headerSize > 256 // Excessively long song title?
		|| fileHeader.type != 0)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderDTM(MemoryFileReader file, const uint64 *pfilesize)
{
	DTMFileHeader fileHeader;
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


bool CSoundFile::ReadDTM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	DTMFileHeader fileHeader;
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

	std::string songName;
	file.ReadString<mpt::String::maybeNullTerminated>(songName, fileHeader.headerSize - (sizeof(fileHeader) - 8u));

	auto chunks = ChunkReader(file).ReadChunks<DTMChunk>(1);

	// Read pattern properties
	uint32 patternFormat;
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idPATT))
	{
		const uint16 numChannels = chunk.ReadUint16BE();
		if(numChannels < 1 || numChannels > 32)
			return false;

		InitializeGlobals(MOD_TYPE_DTM, numChannels);

		Patterns.ResizeArray(chunk.ReadUint16BE());  // Number of stored patterns, may be lower than highest pattern number
		patternFormat = chunk.ReadUint32BE();
		if(patternFormat != DTM_PT_PATTERN_FORMAT && patternFormat != DTM_204_PATTERN_FORMAT && patternFormat != DTM_206_PATTERN_FORMAT)
			return false;
	} else
	{
		return false;
	}

	m_SongFlags.set(SONG_ITCOMPATGXX | SONG_ITOLDEFFECTS | SONG_FASTPORTAS);
	m_playBehaviour.reset(kPeriodsAreHertz);
	m_playBehaviour.reset(kITVibratoTremoloPanbrello);
	// Various files have a default speed or tempo of 0
	if(fileHeader.tempo)
		Order().SetDefaultTempoInt(fileHeader.tempo);
	if(fileHeader.speed)
		Order().SetDefaultSpeed(fileHeader.speed);
	if(fileHeader.stereoMode == 0)
		SetupMODPanning(true);
	m_songName = std::move(songName);

	// Read order list
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idS_Q_))
	{
		uint16 ordLen = chunk.ReadUint16BE();
		uint16 restartPos = chunk.ReadUint16BE();
		chunk.Skip(4);	// Reserved
		ReadOrderFromFile<uint8>(Order(), chunk, ordLen);
		Order().SetRestartPos(restartPos);
	} else
	{
		return false;
	}

	// Read global info
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idSV19))
	{
		chunk.Skip(2);	// Ticks per quarter note, typically 24
		uint32 fractionalTempo = chunk.ReadUint32BE();
		Order().SetDefaultTempo(TEMPO(Order().GetDefaultTempo().GetInt() + fractionalTempo / 4294967296.0));

		uint16be panning[32];
		chunk.ReadArray(panning);
		for(CHANNELINDEX chn = 0; chn < 32 && chn < GetNumChannels(); chn++)
		{
			// Panning is in range 0...180, 90 = center
			ChnSettings[chn].nPan = static_cast<uint16>(128 + Util::muldivr(std::min(static_cast<int>(panning[chn]), int(180)) - 90, 128, 90));
		}

		chunk.Skip(16);
		// Chunk ends here for old DTM modules
		if(chunk.CanRead(2))
		{
			m_nDefaultGlobalVolume = std::min(chunk.ReadUint16BE(), static_cast<uint16>(MAX_GLOBAL_VOLUME));
		}
		chunk.Skip(128);
		uint16be volume[32];
		if(chunk.ReadArray(volume))
		{
			for(CHANNELINDEX chn = 0; chn < 32 && chn < GetNumChannels(); chn++)
			{
				// Volume is in range 0...128, 64 = normal
				ChnSettings[chn].nVolume = static_cast<uint8>(std::min(static_cast<int>(volume[chn]), int(128)) / 2);
			}
			m_nSamplePreAmp *= 2;	// Compensate for channel volume range
		}
	}

	// Read song message
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idTEXT))
	{
		DTMText text;
		chunk.ReadStruct(text);
		if(text.oddLength == 0xFFFF)
		{
			chunk.Skip(1);
		}
		m_songMessage.Read(chunk, chunk.BytesLeft(), SongMessage::leCRLF);
	}

	// Read sample headers
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idINST))
	{
		uint16 numSamples = chunk.ReadUint16BE();
		bool newSamples = (numSamples >= 0x8000);
		numSamples &= 0x7FFF;
		if(numSamples >= MAX_SAMPLES || !chunk.CanRead(numSamples * (sizeof(DTMSample) + (newSamples ? 2u : 0u))))
		{
			return false;
		}
		
		m_nSamples = numSamples;
		for(SAMPLEINDEX smp = 1; smp <= numSamples; smp++)
		{
			SAMPLEINDEX realSample = newSamples ? (chunk.ReadUint16BE() + 1u) : smp;
			DTMSample dtmSample;
			chunk.ReadStruct(dtmSample);
			if(realSample < 1 || realSample >= MAX_SAMPLES)
			{
				continue;
			}
			m_nSamples = std::max(m_nSamples, realSample);
			ModSample &mptSmp = Samples[realSample];
			dtmSample.ConvertToMPT(mptSmp, fileHeader.forcedSampleRate, patternFormat);
			m_szNames[realSample] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, dtmSample.name);
		}
	
		if(chunk.ReadUint16BE() == 0x0004)
		{
			// Digital Home Studio instruments
			m_nInstruments = std::min(static_cast<INSTRUMENTINDEX>(m_nSamples), static_cast<INSTRUMENTINDEX>(MAX_INSTRUMENTS - 1));

			FileReader envChunk = chunks.GetChunk(DTMChunk::idIENV);
			while(chunk.CanRead(sizeof(DTMInstrument)))
			{
				DTMInstrument instr;
				chunk.ReadStruct(instr);
				if(instr.insNum < GetNumInstruments())
				{
					ModSample &sample = Samples[instr.insNum + 1];
					sample.nVibDepth = instr.vibDepth;
					sample.nVibRate = instr.vibRate;
					sample.nVibSweep = 255;

					ModInstrument *mptIns = AllocateInstrument(instr.insNum + 1, instr.insNum + 1);
					if(mptIns != nullptr)
					{
						InstrumentEnvelope &mptEnv = mptIns->VolEnv;
						mptIns->nFadeOut = std::min(static_cast<uint16>(instr.fadeout), uint16(0xFFF));
						if(instr.envelope != 0xFF && envChunk.Seek(2 + sizeof(DTMEnvelope) * instr.envelope))
						{
							DTMEnvelope env;
							envChunk.ReadStruct(env);
							mptEnv.dwFlags.set(ENV_ENABLED);
							mptEnv.resize(std::min({ static_cast<std::size_t>(env.numPoints), std::size(env.points), static_cast<std::size_t>(MAX_ENVPOINTS) }));
							for(size_t i = 0; i < mptEnv.size(); i++)
							{
								mptEnv[i].value = std::min(uint8(64), static_cast<uint8>(env.points[i].value));
								mptEnv[i].tick = env.points[i].tick;
							}

							if(instr.sustain != 0xFF)
							{
								mptEnv.dwFlags.set(ENV_SUSTAIN);
								mptEnv.nSustainStart = mptEnv.nSustainEnd = instr.sustain;
							}
							if(!mptEnv.empty())
							{
								mptEnv.dwFlags.set(ENV_LOOP);
								mptEnv.nLoopStart = mptEnv.nLoopEnd = static_cast<uint8>(mptEnv.size() - 1);
							}
						}
					}
				}
			}
		}
	}

	// Read pattern data
	for(auto &chunk : chunks.GetAllChunks(DTMChunk::idDAPT))
	{
		chunk.Skip(4);	// FF FF FF FF
		PATTERNINDEX patNum = chunk.ReadUint16BE();
		ROWINDEX numRows = chunk.ReadUint16BE();
		if(patternFormat == DTM_206_PATTERN_FORMAT)
		{
			// The stored data is actually not row-based, but tick-based.
			numRows /= Order().GetDefaultSpeed();
		}
		if(!(loadFlags & loadPatternData) || patNum > 255 || !Patterns.Insert(patNum, numRows))
		{
			continue;
		}

		if(patternFormat == DTM_206_PATTERN_FORMAT)
		{
			chunk.Skip(4);
			for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
			{
				uint16 length = chunk.ReadUint16BE();
				if(length % 2u) length++;
				FileReader rowChunk = chunk.ReadChunk(length);
				int tick = 0;
				std::div_t position = { 0, 0 };
				while(rowChunk.CanRead(6) && static_cast<ROWINDEX>(position.quot) < numRows)
				{
					ModCommand *m = Patterns[patNum].GetpModCommand(position.quot, chn);

					const auto [note, volume, instr, command, param, delay] = rowChunk.ReadArray<uint8, 6>();
					if(note > 0 && note <= 96)
					{
						m->note = note + NOTE_MIN + 12;
						if(position.rem)
							m->SetEffectCommand(CMD_MODCMDEX, static_cast<ModCommand::PARAM>(0xD0 | std::min(position.rem, 15)));
					} else if(note & 0x80)
					{
						// Lower 7 bits contain note, probably intended for MIDI-like note-on/note-off events
						if(position.rem)
							m->SetEffectCommand(CMD_MODCMDEX, static_cast<ModCommand::PARAM>(0xC0 |std::min(position.rem, 15)));
						else
							m->note = NOTE_NOTECUT;
					}
					if(volume)
					{
						m->SetVolumeCommand(VOLCMD_VOLUME, std::min(volume, uint8(64)));  // Volume can go up to 255, but we do not support over-amplification at the moment.
					}
					if(instr)
					{
						m->instr = instr;
					}
					if(command || param)
					{
						ConvertModCommand(*m, command, param);
#ifdef MODPLUG_TRACKER
						m->Convert(MOD_TYPE_MOD, MOD_TYPE_IT, *this);
#endif
						// G is 8-bit volume
						// P is tremor (need to disable oldfx)
					}
					if(delay & 0x80)
						tick += (delay & 0x7F) * 0x100 + rowChunk.ReadUint8();
					else
						tick += delay;
					position = std::div(tick, Order().GetDefaultSpeed());
				}
			}
		} else
		{
			for(ModCommand &m : Patterns[patNum])
			{
				const auto data = chunk.ReadArray<uint8, 4>();
				uint8 command = 0;
				if(patternFormat == DTM_204_PATTERN_FORMAT)
				{
					const auto [note, instrVol, instrCmd, param] = data;
					if(note > 0 && note < 0x80)
					{
						m.note = static_cast<ModCommand::NOTE>((note >> 4) * 12 + (note & 0x0F) + NOTE_MIN + 11);
					}
					uint8 vol = instrVol >> 2;
					if(vol)
					{
						m.SetVolumeCommand(VOLCMD_VOLUME, static_cast<ModCommand::VOL>(vol - 1u));
					}
					m.instr = ((instrVol & 0x03) << 4) | (instrCmd >> 4);
					command = instrCmd & 0x0F;
					m.param = param;
				} else
				{
					std::tie(command, m.param) = ReadMODPatternEntry(data, m);
					m.instr |= static_cast<ModCommand::INSTR>(data[0] & 0x30u);  // Allow more than 31 instruments
				}
				ConvertModCommand(m, command, m.param);
				// Fix commands without memory and slide nibble precedence
				switch(m.command)
				{
				case CMD_PORTAMENTOUP:
				case CMD_PORTAMENTODOWN:
					if(!m.param)
						m.command = CMD_NONE;
					break;
				case CMD_VOLUMESLIDE:
				case CMD_TONEPORTAVOL:
				case CMD_VIBRATOVOL:
					if(m.param & 0xF0)
						m.param &= 0xF0;
					else if(!m.param)
						m.command = CMD_NONE;
					break;
				default:
					break;
				}
#ifdef MODPLUG_TRACKER
				m.Convert(MOD_TYPE_MOD, MOD_TYPE_IT, *this);
#endif
			}
		}
	}

	// Read pattern names
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idPATN))
	{
		PATTERNINDEX pat = 0;
		std::string name;
		while(chunk.CanRead(1) && pat < Patterns.Size())
		{
			chunk.ReadNullString(name, 32);
			Patterns[pat].SetName(name);
			pat++;
		}
	}

	// Read channel names
	if(FileReader chunk = chunks.GetChunk(DTMChunk::idTRKN))
	{
		CHANNELINDEX chn = 0;
		std::string name;
		while(chunk.CanRead(1) && chn < GetNumChannels())
		{
			chunk.ReadNullString(name, 32);
			ChnSettings[chn].szName = name;
			chn++;
		}
	}

	// Read sample data
	for(auto &chunk : chunks.GetAllChunks(DTMChunk::idDAIT))
	{
		SAMPLEINDEX smp = chunk.ReadUint16BE();
		if(smp >= GetNumSamples() || !(loadFlags & loadSampleData))
		{
			continue;
		}
		ModSample &mptSmp = Samples[smp + 1];
		SampleIO(
			mptSmp.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
			mptSmp.uFlags[CHN_STEREO] ? SampleIO::stereoInterleaved: SampleIO::mono,
			SampleIO::bigEndian,
			SampleIO::signedPCM).ReadSample(mptSmp, chunk);
	}

	// Is this accurate?
	mpt::ustring tracker;
	if(patternFormat == DTM_206_PATTERN_FORMAT)
	{
		tracker = UL_("Digital Home Studio");
	} else if(patternFormat == DTM_PT_PATTERN_FORMAT)
	{
		tracker = UL_("Digital Tracker 2.3");
	} else if(FileReader chunk = chunks.GetChunk(DTMChunk::idVERS))
	{
		uint32 version = chunk.ReadUint32BE();
		tracker = MPT_UFORMAT("Digital Tracker {}.{}")(version >> 4, version & 0x0F);
	} else
	{
		tracker = UL_("Digital Tracker");
	}
	m_modFormat.formatName = UL_("Digital Tracker");
	m_modFormat.type = UL_("dtm");
	m_modFormat.madeWithTracker = std::move(tracker);
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	return true;
}

OPENMPT_NAMESPACE_END
