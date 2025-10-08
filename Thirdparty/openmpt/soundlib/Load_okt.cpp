/*
 * Load_okt.cpp
 * ------------
 * Purpose: OKT (Oktalyzer) module loader
 * Notes  : (currently none)
 * Authors: Storlek (Original author - http://schismtracker.org/ - code ported with permission)
 *          Johannes Schultz (OpenMPT Port, tweaks)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct OktIffChunk
{
	// IFF chunk names
	enum ChunkIdentifiers
	{
		idCMOD	= MagicBE("CMOD"),
		idSAMP	= MagicBE("SAMP"),
		idSPEE	= MagicBE("SPEE"),
		idSLEN	= MagicBE("SLEN"),
		idPLEN	= MagicBE("PLEN"),
		idPATT	= MagicBE("PATT"),
		idPBOD	= MagicBE("PBOD"),
		idSBOD	= MagicBE("SBOD"),
	};

	uint32be signature;  // IFF chunk name
	uint32be chunkSize;  // Chunk size without header
};

MPT_BINARY_STRUCT(OktIffChunk, 8)

struct OktSample
{
	char     name[20];
	uint32be length;      // Length in bytes
	uint16be loopStart;   // *2 for real value
	uint16be loopLength;  // ditto
	uint16be volume;      // Default volume
	uint16be type;        // 7-/8-bit sample (0: 7-bit, only usable on paired channels ["8" in GUI], 1: 8-bit, only usable on unpaired channels ["4" in GUI], 2: 7-bit, usable on all channels ["B" in GUI])
};

MPT_BINARY_STRUCT(OktSample, 32)


// Parse the sample header block
static void ReadOKTSamples(FileReader &chunk, CSoundFile &sndFile)
{
	static_assert(MAX_SAMPLES >= 72);  // For copies of type "B" samples
	sndFile.m_nSamples = std::min(static_cast<SAMPLEINDEX>(chunk.BytesLeft() / sizeof(OktSample)), SAMPLEINDEX(36));

	for(SAMPLEINDEX smp = 1; smp <= sndFile.GetNumSamples(); smp++)
	{
		ModSample &mptSmp = sndFile.GetSample(smp);
		OktSample oktSmp;
		chunk.ReadStruct(oktSmp);

		mptSmp.Initialize();
		sndFile.m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, oktSmp.name);

		mptSmp.nC5Speed = 8287;
		mptSmp.nVolume = std::min(oktSmp.volume.get(), uint16(64)) * 4u;
		mptSmp.nLength = oktSmp.length & ~1;
		mptSmp.cues[0] = oktSmp.type;  // Temporary storage for pattern reader, will be reset later
		mptSmp.cues[1] = 0;
		// Parse loops
		const SmpLength loopStart = oktSmp.loopStart * 2;
		const SmpLength loopLength = oktSmp.loopLength * 2;
		if(loopLength > 2 && loopStart + loopLength <= mptSmp.nLength)
		{
			mptSmp.uFlags.set(CHN_SUSTAINLOOP);
			mptSmp.nSustainStart = loopStart;
			mptSmp.nSustainEnd = loopStart + loopLength;
		}
	}
}


// Turn negative arpeggio offset into equivalent positive arpeggio offset
static uint8 InvertArpeggioParam(uint8 param)
{
	param &= 0x0F;
	if(!param)
		return param;
	else if(param <= 0x0C)
		return (0x0C - param);
	else
		return (0x18 - param);
}


// Parse a pattern block
static void ReadOKTPattern(FileReader &chunk, PATTERNINDEX pat, CSoundFile &sndFile, const std::array<int8, 8> pairedChn)
{
	if(!chunk.CanRead(2))
	{
		// Invent empty pattern
		sndFile.Patterns.Insert(pat, 64);
		return;
	}

	ROWINDEX rows = Clamp(static_cast<ROWINDEX>(chunk.ReadUint16BE()), ROWINDEX(1), MAX_PATTERN_ROWS);

	if(!sndFile.Patterns.Insert(pat, rows))
	{
		return;
	}

	const CHANNELINDEX chns = sndFile.GetNumChannels();

	for(ROWINDEX row = 0; row < rows; row++)
	{
		auto rowCmd = sndFile.Patterns[pat].GetRow(row);
		for(CHANNELINDEX chn = 0; chn < chns; chn++)
		{
			ModCommand &m = rowCmd[chn];
			const auto oldCmd = m.command;
			const auto oldParam = m.param;
			const auto [note, instr, effect, param] = chunk.ReadArray<uint8, 4>();

			if(note > 0 && note <= 36)
			{
				m.note = note + (NOTE_MIDDLEC - 13);
				if(pairedChn[chn] && m.note >= NOTE_MIDDLEC + 22)
					m.note = NOTE_MIDDLEC + 21;

				m.instr = instr + 1;
				if(m.instr > 0 && m.instr <= sndFile.GetNumSamples())
				{
					auto &sample = sndFile.GetSample(m.instr);
					// Default volume only works on raw Paula channels
					if(pairedChn[chn] && sample.nVolume < 256)
						m.SetVolumeCommand(VOLCMD_VOLUME, 64);

					// If channel and sample type don't match, stop this channel (add 100 to the instrument number to make it understandable what happened during import)
					if((sample.cues[0] == 1 && pairedChn[chn] != 0) || (sample.cues[0] == 0 && pairedChn[chn] == 0))
					{
						m.instr += 100;
					} else if(sample.cues[0] == 2 && pairedChn[chn] && sample.uFlags[CHN_SUSTAINLOOP])
					{
						// Type "B" sample: Loops only work on raw Paula channels
						sample.cues[1] = 1;
						m.instr += 36;
					}
				}
			}

			switch(effect)
			{
			case 0:  // Nothing
				break;

			case 1:  // 1 Portamento Down (Period)
				if(param)
				{
					m.SetEffectCommand(CMD_PORTAMENTOUP, param);
				}
				break;
			case 2:  // 2 Portamento Up (Period)
				if(param)
					m.SetEffectCommand(CMD_PORTAMENTODOWN, param);
				break;

			case 10:  // A Arpeggio 1 (down, orig, up)
				if(param)
					m.SetEffectCommand(CMD_ARPEGGIO, (param & 0x0F) | (InvertArpeggioParam(param >> 4) << 4));
				break;

			case 11:  // B Arpeggio 2 (orig, up, orig, down)
				if(param)
					m.SetEffectCommand(CMD_ARPEGGIO, (param & 0xF0) | InvertArpeggioParam(param & 0x0F));
				break;
		
			// This one is close enough to "standard" arpeggio -- I think!
			case 12:  // C Arpeggio 3 (up, up, orig)
				if(param)
					m.SetEffectCommand(CMD_ARPEGGIO, param);
				break;

			case 13:  // D Slide Down (Notes)
				if(param)
					m.SetEffectCommand(CMD_NOTESLIDEDOWN, 0x10 | std::min(uint8(0x0F), param));
				break;
			case 30:  // U Slide Up (Notes)
				if(param)
					m.SetEffectCommand(CMD_NOTESLIDEUP, 0x10 | std::min(uint8(0x0F), param));
				break;
			// Fine Slides are only implemented for libopenmpt. For OpenMPT,
			// sliding every 5 (non-note) ticks kind of works (at least at
			// speed 6), but implementing separate (format-agnostic) fine slide commands would of course be better.
			case 21:  // L Slide Down Once (Notes)
				if(param)
					m.SetEffectCommand(CMD_NOTESLIDEDOWN, 0x50 | std::min(uint8(0x0F), param));
				break;
			case 17:  // H Slide Up Once (Notes)
				if(param)
					m.SetEffectCommand(CMD_NOTESLIDEUP, 0x50 | std::min(uint8(0x0F), param));
				break;

			case 15:  // F Set Filter <>00:ON
				m.SetEffectCommand(CMD_MODCMDEX, !!param);
				break;

			case 25:  // P Pos Jump
				m.SetEffectCommand(CMD_POSITIONJUMP, param);
				break;

			case 27:  // R Release sample (apparently not listed in the help!)
				m.note = NOTE_KEYOFF;
				m.instr = 0;
				break;

			case 28:  // S Speed
				if(param < 0x20)
					m.SetEffectCommand(CMD_SPEED, param);
				break;

			case 31:  // V Volume
				// Volume on mixed channels is permanent, on hardware channels it behaves like in regular MODs
				if(param & 0x0F)
					m.SetEffectCommand(pairedChn[chn] ? CMD_CHANNELVOLSLIDE : CMD_VOLUMESLIDE, param & 0x0F);

				switch(param >> 4)
				{
				case 4:  // Normal slide down
					if(param != 0x40)
						break;
					// 0x40 is set volume -- fall through
					[[fallthrough]];
				case 0: case 1: case 2: case 3:
					if(pairedChn[chn])
					{
						m.SetEffectCommand(CMD_CHANNELVOLUME, param);
					} else
					{
						m.SetVolumeCommand(VOLCMD_VOLUME, param);
						m.SetEffectCommand(oldCmd, oldParam);
					}
					break;
				case 5:  // Normal slide up
					m.param <<= 4;
					break;
				case 6:  // Fine slide down
					m.param = 0xF0 | std::min(static_cast<uint8>(m.param), uint8(0x0E));
					break;
				case 7:  // Fine slide up
					m.param = (std::min(static_cast<uint8>(m.param), uint8(0x0E)) << 4) | 0x0F;
					break;
				default:
					// Junk.
					m.SetEffectCommand(oldCmd, oldParam);
					break;
				}

				// Volume is shared between two mixed channels, second channel has priority
				if(m.command == CMD_CHANNELVOLUME || m.command == CMD_CHANNELVOLSLIDE)
				{
					ModCommand &other = rowCmd[chn + pairedChn[chn]];
					// Try to preserve effect if there already was one
					if(auto volCmd = other.ConvertToVolCommand(other.command, other.param, true); volCmd.first != VOLCMD_NONE)
					{
						other.SetVolumeCommand(volCmd);
					}
					if(ModCommand::GetEffectWeight(other.command) < ModCommand::GetEffectWeight(m.command))
					{
						other.SetEffectCommand(m);
					} else if(row < rows - 1)
					{
						// Retry on next row
						sndFile.Patterns[pat].GetpModCommand(row + 1, static_cast<CHANNELINDEX>(chn + pairedChn[chn]))->SetEffectCommand(m);
					}
				}
				break;

#if 0
			case 24:  // O Old Volume (???)
				m.command = CMD_VOLUMESLIDE;
				m.param = 0;
				break;
#endif

			default:
				break;
			}

			// In case we overwrote the volume command from a mixed channel
			if(oldCmd != CMD_NONE && m.command != oldCmd)
			{
				m.FillInTwoCommands(m.command, m.param, oldCmd, oldParam);
			}
		}
	}
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderOKT(MemoryFileReader file, const uint64 *pfilesize)
{
	if(!file.CanRead(8))
	{
		return ProbeWantMoreData;
	}
	if(!file.ReadMagic("OKTASONG"))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadOKT(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	if(!file.ReadMagic("OKTASONG"))
	{
		return false;
	}

	std::vector<FileReader> patternChunks;
	std::vector<FileReader> sampleChunks;
	std::array<int8, 8> pairedChn{{}};
	ORDERINDEX numOrders = 0;

	InitializeGlobals(MOD_TYPE_OKT, 0);

	m_modFormat.formatName = UL_("Oktalyzer");
	m_modFormat.type = UL_("okt");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	// Go through IFF chunks...
	while(file.CanRead(sizeof(OktIffChunk)))
	{
		OktIffChunk iffHead;
		if(!file.ReadStruct(iffHead))
			break;

		FileReader chunk = file.ReadChunk(iffHead.chunkSize);
		if(!chunk.IsValid())
			continue;

		switch(iffHead.signature)
		{
		case OktIffChunk::idCMOD:
			// Channel setup table
			if(GetNumChannels() == 0 && chunk.CanRead(8))
			{
				const auto chnTable = chunk.ReadArray<uint16be, 4>();
				ChnSettings.reserve(8);
				CHANNELINDEX realChn = 0;
				for(CHANNELINDEX chn = 0; chn < 4; chn++)
				{
					if(chnTable[chn])
					{
						pairedChn[realChn++] = 1;
						pairedChn[realChn] = -1;
						ChnSettings.emplace_back().nPan = (((chn & 3) == 1) || ((chn & 3) == 2)) ? 0xC0 : 0x40;
					}
					realChn++;
					ChnSettings.emplace_back().nPan = (((chn & 3) == 1) || ((chn & 3) == 2)) ? 0xC0 : 0x40;
				}

				if(loadFlags == onlyVerifyHeader)
				{
					return true;
				}
			}
			break;

		case OktIffChunk::idSAMP:
			// Convert sample headers
			if(m_nSamples > 0)
			{
				break;
			}
			ReadOKTSamples(chunk, *this);
			break;

		case OktIffChunk::idSPEE:
			// Read default speed
			if(chunk.GetLength() >= 2)
			{
				Order().SetDefaultSpeed(Clamp(chunk.ReadUint16BE(), uint16(1), uint16(255)));
			}
			break;

		case OktIffChunk::idSLEN:
			// Number of patterns, we don't need this.
			break;

		case OktIffChunk::idPLEN:
			// Read number of valid orders
			if(chunk.GetLength() >= 2)
			{
				numOrders = chunk.ReadUint16BE();
			}
			break;

		case OktIffChunk::idPATT:
			// Read the orderlist
			ReadOrderFromFile<uint8>(Order(), chunk, chunk.GetLength(), 0xFF, 0xFE);
			break;

		case OktIffChunk::idPBOD:
			// Don't read patterns for now, as the number of channels might be unknown at this point.
			if(patternChunks.size() < 256)
			{
				patternChunks.push_back(chunk);
			}
			break;

		case OktIffChunk::idSBOD:
			// Sample data - same as with patterns, as we need to know the sample format / length
			if(sampleChunks.size() < MAX_SAMPLES - 1 && chunk.GetLength() > 0)
			{
				sampleChunks.push_back(chunk);
			}
			break;
		}
	}

	// If there wasn't even a CMOD chunk, we can't really load this.
	if(GetNumChannels() == 0)
		return false;

	Order().SetDefaultTempoInt(125);
	m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	m_nSamplePreAmp = m_nVSTiVolume = 48;
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 856 * 4;
	m_SongFlags.set(SONG_FASTPORTAS);

	// Fix orderlist
	Order().resize(numOrders);

	// Read patterns
	if(loadFlags & loadPatternData)
	{
		Patterns.ResizeArray(static_cast<PATTERNINDEX>(patternChunks.size()));
		for(PATTERNINDEX pat = 0; pat < patternChunks.size(); pat++)
		{
			ReadOKTPattern(patternChunks[pat], pat, *this, pairedChn);
		}
	}

	// Read samples
	size_t fileSmp = 0;
	const SAMPLEINDEX origSamples = m_nSamples;
	for(SAMPLEINDEX smp = 1; smp <= origSamples; smp++)
	{
		if(fileSmp >= sampleChunks.size() || !(loadFlags & loadSampleData))
			break;

		ModSample &mptSample = Samples[smp];
		const bool needCopy = mptSample.cues[1] != 0;
		if(mptSample.nLength == 0)
			continue;

		// Weird stuff?
		LimitMax(mptSample.nLength, mpt::saturate_cast<SmpLength>(sampleChunks[fileSmp].GetLength()));

		SampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::bigEndian,
			SampleIO::signedPCM)
			.ReadSample(mptSample, sampleChunks[fileSmp]);

		if(needCopy)
		{
			// Type "B" samples (can play on both paired and unpaired channels) can have loop information,
			// which can only be used on unpaired channels. So we need a looped and unlooped copy of the sample.
			m_nSamples = std::max(m_nSamples, static_cast<SAMPLEINDEX>(smp + 36));
			ModSample &copySample = Samples[smp + 36];
			copySample.Initialize();
			copySample.nC5Speed = mptSample.nC5Speed;
			copySample.nVolume = mptSample.nVolume;
			copySample.nLength = mptSample.nLength;
			copySample.CopyWaveform(mptSample);
			m_szNames[smp + 36] = m_szNames[smp];
		}

		fileSmp++;
	}

	return true;
}


OPENMPT_NAMESPACE_END
