/*
 * Load_mt2.cpp
 * ------------
 * Purpose: MT2 (MadTracker 2) module loader
 * Notes  : A couple of things are not handled properly or not at all, such as internal effects and automation envelopes
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#ifdef MPT_EXTERNAL_SAMPLES
// For loading external samples
#include "../common/mptPathString.h"
#endif // MPT_EXTERNAL_SAMPLES
#ifdef MPT_WITH_VST
#include "../mptrack/Vstplug.h"
#endif // MPT_WITH_VST

OPENMPT_NAMESPACE_BEGIN

struct MT2FileHeader
{
	enum MT2HeaderFlags
	{
		packedPatterns		= 0x01,
		automation			= 0x02,
		drumsAutomation		= 0x08,
		masterAutomation	= 0x10,
	};

	char     signature[4];		// "MT20"
	uint32le userID;
	uint16le version;
	char     trackerName[32];	// "MadTracker 2.0"
	char     songName[64];
	uint16le numOrders;
	uint16le restartPos;
	uint16le numPatterns;
	uint16le numChannels;
	uint16le samplesPerTick;
	uint8le  ticksPerLine;
	uint8le  linesPerBeat;
	uint32le flags;				// See HeaderFlags
	uint16le numInstruments;
	uint16le numSamples;
};

MPT_BINARY_STRUCT(MT2FileHeader, 126)


struct MT2DrumsData
{
	uint16le numDrumPatterns;
	uint16le DrumSamples[8];
	uint8le  DrumPatternOrder[256];
};

MPT_BINARY_STRUCT(MT2DrumsData, 274)


struct MT2TrackSettings
{
	uint16le volume;
	uint8le  trackfx;	// Built-in effect type is used
	uint8le  output;
	uint16le fxID;
	uint16le trackEffectParam[64][8];
};

MPT_BINARY_STRUCT(MT2TrackSettings, 1030)


struct MT2Command
{
	uint8 note;	// 0=nothing, 97=note off
	uint8 instr;
	uint8 vol;
	uint8 pan;
	uint8 fxcmd;
	uint8 fxparam1;
	uint8 fxparam2;
};

MPT_BINARY_STRUCT(MT2Command, 7)


struct MT2EnvPoint
{
	uint16le x;
	uint16le y;
};

MPT_BINARY_STRUCT(MT2EnvPoint, 4)


struct MT2Instrument
{
	enum EnvTypes
	{
		VolumeEnv	= 1,
		PanningEnv	= 2,
		PitchEnv	= 4,
		FilterEnv	= 8,
	};

	uint16le numSamples;
	uint8le  groupMap[96];
	uint8le  vibtype, vibsweep, vibdepth, vibrate;
	uint16le fadeout;
	uint16le nna;
};

MPT_BINARY_STRUCT(MT2Instrument, 106)


struct MT2IEnvelope
{
	uint8le flags;
	uint8le numPoints;
	uint8le sustainPos;
	uint8le loopStart;
	uint8le loopEnd;
	uint8le reserved[3];
	MT2EnvPoint points[16];
};

MPT_BINARY_STRUCT(MT2IEnvelope, 72)


// Note: The order of these fields differs a bit in MTIOModule_MT2.cpp - maybe just typos, I'm not sure.
// This struct follows the save format of MadTracker 2.6.1.
struct MT2InstrSynth
{
	uint8le  synthID;
	uint8le  effectID;		// 0 = Lowpass filter, 1 = Highpass filter
	uint16le cutoff;		// 100...11000 Hz
	uint8le  resonance;		// 0...128
	uint8le  attack;		// 0...128
	uint8le  decay;			// 0...128
	uint8le  midiChannel;	// 0...15
	int8le   device;		// VST slot (positive) or MIDI device (negative)
	int8le   unknown1;		// Missing in MTIOModule_MT2.cpp
	uint8le  volume;		// 0...255
	int8le   finetune;		// -96...96
	int8le   transpose;		// -48...48
	uint8le  unknown2;		// Seems to be equal to instrument number.
	uint8le  unknown3;
	uint8le  midiProgram;
	uint8le  reserved[16];
};

MPT_BINARY_STRUCT(MT2InstrSynth, 32)


struct MT2Sample
{
	uint32le length;
	uint32le frequency;
	uint8le  depth;
	uint8le  channels;
	uint8le  flags;
	uint8le  loopType;
	uint32le loopStart;
	uint32le loopEnd;
	uint16le volume;
	int8le   panning;
	int8le   note;
	int16le  spb;
};

MPT_BINARY_STRUCT(MT2Sample, 26)


struct MT2Group
{
	uint8le sample;
	uint8le vol;		// 0...128
	int8le  pitch;		// -128...127
	uint8le reserved[5];
};

MPT_BINARY_STRUCT(MT2Group, 8)


struct MT2VST
{
	char     dll[64];
	char     programName[28];
	uint32le fxID;
	uint32le fxVersion;
	uint32le programNr;
	uint8le  useChunks;
	uint8le  track;
	int8le   pan;				// Not imported - could use pan mix mode for D/W ratio, but this is not implemented for instrument plugins!
	char     reserved[17];
	uint32le n;
};

MPT_BINARY_STRUCT(MT2VST, 128)


static bool ConvertMT2Command(CSoundFile *that, ModCommand &m, MT2Command &p)
{
	bool hasLegacyTempo = false;

	// Note
	m.note = NOTE_NONE;
	if(p.note) m.note = (p.note > 96) ? NOTE_KEYOFF : (p.note + NOTE_MIN + 11);
	// Instrument
	m.instr = p.instr;
	// Volume Column
	if(p.vol >= 0x10 && p.vol <= 0x90)
	{
		m.SetVolumeCommand(VOLCMD_VOLUME, static_cast<ModCommand::VOL>((p.vol - 0x10) / 2));
	} else if(p.vol >= 0xA0 && p.vol <= 0xAF)
	{
		m.volcmd = VOLCMD_VOLSLIDEDOWN;
		m.vol = (p.vol & 0x0F);
	} else if(p.vol >= 0xB0 && p.vol <= 0xBF)
	{
		m.volcmd = VOLCMD_VOLSLIDEUP;
		m.vol = (p.vol & 0x0F);
	} else if(p.vol >= 0xC0 && p.vol <= 0xCF)
		{
		m.volcmd = VOLCMD_FINEVOLDOWN;
		m.vol = (p.vol & 0x0F);
	} else if(p.vol >= 0xD0 && p.vol <= 0xDF)
	{
		m.volcmd = VOLCMD_FINEVOLUP;
		m.vol = (p.vol & 0x0F);
	}

	// Effects
	if(p.fxcmd || p.fxparam1 || p.fxparam2)
	{
		switch(p.fxcmd)
		{
		case 0x00:	// FastTracker effect
			CSoundFile::ConvertModCommand(m, p.fxparam2, p.fxparam1);
#ifdef MODPLUG_TRACKER
			m.Convert(MOD_TYPE_XM, MOD_TYPE_IT, *that);
#else
			MPT_UNREFERENCED_PARAMETER(that);
#endif // MODPLUG_TRACKER
			if(p.fxparam2 == 0x0F)
				hasLegacyTempo = true;
			break;

		case 0x01:	// Portamento up (on every tick)
			m.command = CMD_PORTAMENTOUP;
			m.param = mpt::saturate_cast<ModCommand::PARAM>((p.fxparam2 << 4) | (p.fxparam1 >> 4));
			break;

		case 0x02:	// Portamento down (on every tick)
			m.command = CMD_PORTAMENTODOWN;
			m.param = mpt::saturate_cast<ModCommand::PARAM>((p.fxparam2 << 4) | (p.fxparam1 >> 4));
			break;

		case 0x03:	// Tone Portamento (on every tick)
			m.command = CMD_TONEPORTAMENTO;
			m.param = mpt::saturate_cast<ModCommand::PARAM>((p.fxparam2 << 4) | (p.fxparam1 >> 4));
			break;

		case 0x04:	// Vibrato
			m.command = CMD_VIBRATO;
			m.param = (p.fxparam2 & 0xF0) | (p.fxparam1 >> 4);
			break;

		case 0x08:	// Panning + Polarity (we can only import panning for now)
			if(p.fxparam1)
			{
				m.command = CMD_PANNING8;
				m.param = p.fxparam1;
			} else if(p.fxparam2 == 1 || p.fxparam2 == 2)
			{
				// Invert left or right channel
				m.command = CMD_S3MCMDEX;
				m.param = 0x91;
			}
			break;

		case 0x0C:	// Set volume (0x80 = 100%)
			m.command = CMD_VOLUME;
			m.param = p.fxparam2 / 2;
			break;

		case 0x0F:	// Set tempo, LPB and ticks (we can only import tempo for now)
			if(p.fxparam2 != 0)
			{
				m.command = CMD_TEMPO;
				m.param = p.fxparam2;
			} else
			{
				m.command = CMD_SPEED;
				m.param = (p.fxparam1 & 0x0F);
			}
			break;

		case 0x10:	// Impulse Tracker effect
			CSoundFile::S3MConvert(m, p.fxparam2, p.fxparam1, true);
			if(m.command == CMD_TEMPO || m.command == CMD_SPEED)
				hasLegacyTempo = true;
			break;

		case 0x1D:	// Gapper (like IT Tremor with old FX, i.e. 1D 00 XY = ontime X + 1 ticks, offtime Y + 1 ticks)
			m.command = CMD_TREMOR;
			m.param = p.fxparam1;
			break;

		case 0x20:	// Cutoff + Resonance (we can only import cutoff for now)
			m.command = CMD_MIDI;
			m.param = p.fxparam2 >> 1;
			break;
				
		case 0x22:	// Cutoff + Resonance + Attack + Decay (we can only import cutoff for now)
			m.command = CMD_MIDI;
			m.param = (p.fxparam2 & 0xF0) >> 1;
			break;

		case 0x24:	// Reverse
			m.command = CMD_S3MCMDEX;
			m.param = 0x9F;
			break;

		case 0x80:	// Track volume
			m.command = CMD_CHANNELVOLUME;
			m.param = p.fxparam2 / 4u;
			break;

		case 0x9D:	// Offset + delay
			m.volcmd = VOLCMD_OFFSET;
			m.vol = p.fxparam2 >> 3;
			m.command = CMD_S3MCMDEX;
			m.param = 0xD0 | std::min(p.fxparam1, uint8(0x0F));
			break;

		case 0xCC:	// MIDI CC
			//m.command = CMD_MIDI;
			break;

			// TODO: More MT2 Effects
		}
	}

	if(p.pan)
	{
		if(m.command == CMD_NONE)
		{
			m.command = CMD_PANNING8;
			m.param = p.pan;
		} else if(m.volcmd == VOLCMD_NONE)
		{
			m.volcmd = VOLCMD_PANNING;
			m.vol = p.pan / 4;
		}
	}

	return hasLegacyTempo;
}


// This doesn't really do anything but skipping the envelope chunk at the moment.
static void ReadMT2Automation(uint16 version, FileReader &file)
{
	uint32 flags;
	uint32 trkfxid;
	if(version >= 0x203)
	{
		flags = file.ReadUint32LE();
		trkfxid = file.ReadUint32LE();
	} else
	{
		flags = file.ReadUint16LE();
		trkfxid = file.ReadUint16LE();
	}
	MPT_UNREFERENCED_PARAMETER(trkfxid);
	while(flags != 0)
	{
		if(flags & 1)
		{
			file.Skip(4 + sizeof(MT2EnvPoint) * 64);
		}
		flags >>= 1;
	}
}


static bool ValidateHeader(const MT2FileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.signature, "MT20", 4)
		|| fileHeader.version < 0x200 || fileHeader.version >= 0x300
		|| fileHeader.numChannels < 1 || fileHeader.numChannels > 64
		|| fileHeader.numOrders > 256
		|| fileHeader.numInstruments >= MAX_INSTRUMENTS
		|| fileHeader.numSamples >= MAX_SAMPLES
		)
	{
		return false;
	}
	return true;
}


static uint64 GetHeaderMinimumAdditionalSize(const MT2FileHeader &fileHeader)
{
	MPT_UNREFERENCED_PARAMETER(fileHeader);
	return 256;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMT2(MemoryFileReader file, const uint64 *pfilesize)
{
	MT2FileHeader fileHeader;
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


bool CSoundFile::ReadMT2(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	MT2FileHeader fileHeader;
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

	InitializeGlobals(MOD_TYPE_MT2, fileHeader.numChannels);

	m_modFormat.formatName = MPT_UFORMAT("MadTracker {}.{}")(fileHeader.version >> 8, mpt::ufmt::hex0<2>(fileHeader.version & 0xFF));
	m_modFormat.type = UL_("mt2");
	m_modFormat.madeWithTracker = mpt::ToUnicode(mpt::Charset::Windows1252, mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.trackerName));
	m_modFormat.charset = mpt::Charset::Windows1252;

	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songName);
	Order().SetDefaultSpeed(Clamp<uint8, uint8>(fileHeader.ticksPerLine, 1, 31));
	Order().SetDefaultTempoInt(125);
	m_SongFlags = SONG_LINEARSLIDES | SONG_ITCOMPATGXX | SONG_EXFILTERRANGE;
	m_nInstruments = fileHeader.numInstruments;
	m_nSamples = fileHeader.numSamples;
	m_nDefaultRowsPerBeat = Clamp<uint8, uint8>(fileHeader.linesPerBeat, 1, 32);
	m_nDefaultRowsPerMeasure = m_nDefaultRowsPerBeat * 4;
	m_nVSTiVolume = 48;
	m_nSamplePreAmp = 48 * 2;	// Double pre-amp because we will halve the volume of all non-drum instruments, because the volume of drum samples can exceed that of normal samples

	uint8 orders[256];
	file.ReadArray(orders);
	ReadOrderFromArray(Order(), orders, fileHeader.numOrders);
	Order().SetRestartPos(fileHeader.restartPos);

	// This value is supposed to be the size of the drums data, but in old MT2.0 files it's 8 bytes too small.
	// MadTracker itself unconditionally reads 274 bytes here if the value is != 0, so we do the same.
	const bool hasDrumChannels = file.ReadUint16LE() != 0;
	FileReader drumData = file.ReadChunk(hasDrumChannels ? sizeof(MT2DrumsData) : 0);
	FileReader extraData = file.ReadChunk(file.ReadUint32LE());

	const CHANNELINDEX channelsWithoutDrums = GetNumChannels();
	static_assert(MAX_BASECHANNELS >= 64 + 8);
	if(hasDrumChannels)
	{
		ChnSettings.resize(GetNumChannels() + 8);
	}

	bool hasLegacyTempo = false;

	// Read patterns
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.numPatterns);
	for(PATTERNINDEX pat = 0; pat < fileHeader.numPatterns; pat++)
	{
		ROWINDEX numRows = file.ReadUint16LE();
		FileReader chunk = file.ReadChunk((file.ReadUint32LE() + 1) & ~1);

		LimitMax(numRows, MAX_PATTERN_ROWS);
		if(!numRows
			|| !(loadFlags & loadPatternData)
			|| !Patterns.Insert(pat, numRows))
		{
			continue;
		}

		if(fileHeader.flags & MT2FileHeader::packedPatterns)
		{
			ROWINDEX row = 0;
			CHANNELINDEX chn = 0;
			while(chunk.CanRead(1))
			{
				MT2Command cmd;

				uint8 infobyte = chunk.ReadUint8();
				uint8 repeatCount = 0;
				if(infobyte == 0xFF)
				{
					repeatCount = chunk.ReadUint8();
					infobyte = chunk.ReadUint8();
				}
				if(infobyte & 0x7F)
				{
					ModCommand *m = Patterns[pat].GetpModCommand(row, chn);
					MemsetZero(cmd);
					if(infobyte & 0x01) cmd.note = chunk.ReadUint8();
					if(infobyte & 0x02) cmd.instr = chunk.ReadUint8();
					if(infobyte & 0x04) cmd.vol = chunk.ReadUint8();
					if(infobyte & 0x08) cmd.pan = chunk.ReadUint8();
					if(infobyte & 0x10) cmd.fxcmd = chunk.ReadUint8();
					if(infobyte & 0x20) cmd.fxparam1 = chunk.ReadUint8();
					if(infobyte & 0x40) cmd.fxparam2 = chunk.ReadUint8();
					hasLegacyTempo |= ConvertMT2Command(this, *m, cmd);
					const ModCommand &orig = *m;
					const ROWINDEX fillRows = std::min((uint32)repeatCount, (uint32)numRows - (row + 1));
					for(ROWINDEX r = 0; r < fillRows; r++)
					{
						m += GetNumChannels();
						// cppcheck false-positive
						// cppcheck-suppress selfAssignment
						*m = orig;
					}
				}
				row += repeatCount + 1;
				while(row >= numRows) { row -= numRows; chn++; }
				if(chn >= channelsWithoutDrums) break;
			}
		} else
		{
			for(ROWINDEX row = 0; row < numRows; row++)
			{
				auto rowData = Patterns[pat].GetRow(row);
				for(CHANNELINDEX chn = 0; chn < channelsWithoutDrums; chn++)
				{
					MT2Command cmd;
					chunk.ReadStruct(cmd);
					hasLegacyTempo |= ConvertMT2Command(this, rowData[chn], cmd);
				}
			}
		}
	}

	if(fileHeader.samplesPerTick > 1 && fileHeader.samplesPerTick < 5000)
	{
		if(hasLegacyTempo)
		{
			Order().SetDefaultTempo(TEMPO{}.SetRaw(Util::muldivr(110250, TEMPO::fractFact, fileHeader.samplesPerTick)));
			m_nTempoMode = TempoMode::Classic;
		} else
		{
			Order().SetDefaultTempo(TEMPO(44100.0 * 60.0 / (Order().GetDefaultSpeed() * m_nDefaultRowsPerBeat * fileHeader.samplesPerTick)));
			m_nTempoMode = TempoMode::Modern;
		}
	}

	// Read extra data
	uint32 numVST = 0;
	std::vector<int8> trackRouting(GetNumChannels(), 0);
	while(extraData.CanRead(8))
	{
		uint32 id = extraData.ReadUint32LE();
		FileReader chunk = extraData.ReadChunk(extraData.ReadUint32LE());

		switch(id)
		{
		case MagicLE("BPM+"):
			if(!hasLegacyTempo)
			{
				m_nTempoMode = TempoMode::Modern;
				double d = chunk.ReadDoubleLE();
				if(d > 0.00000001)
				{
					Order().SetDefaultTempo(TEMPO(44100.0 * 60.0 / (Order().GetDefaultSpeed() * m_nDefaultRowsPerBeat * d)));
				}
			}
			break;

		case MagicLE("TFXM"):
			break;

		case MagicLE("TRKO"):
			break;

		case MagicLE("TRKS"):
			m_nSamplePreAmp = chunk.ReadUint16LE() / 256u;  // 131072 is 0dB... I think (that's how MTIOModule_MT2.cpp reads)
			// Dirty workaround for modules that use track automation for a fade-in at the song start (e.g. Rock.mt2)
			if(!m_nSamplePreAmp)
				m_nSamplePreAmp = 48;
			m_nVSTiVolume = m_nSamplePreAmp / 2u;
			for(CHANNELINDEX c = 0; c < GetNumChannels(); c++)
			{
				MT2TrackSettings trackSettings;
				if(chunk.ReadStruct(trackSettings))
				{
					ChnSettings[c].nVolume = static_cast<uint8>(trackSettings.volume >> 10);	// 32768 is 0dB
					trackRouting[c] = trackSettings.output;
				}
			}
			break;

		case MagicLE("TRKL"):
			for(CHANNELINDEX i = 0; i < GetNumChannels() && chunk.CanRead(1); i++)
			{
				std::string name;
				chunk.ReadNullString(name);
				ChnSettings[i].szName = mpt::String::ReadBuf(mpt::String::spacePadded, name.c_str(), name.length());
			}
			break;

		case MagicLE("PATN"):
			for(PATTERNINDEX i = 0; i < fileHeader.numPatterns && chunk.CanRead(1) && Patterns.IsValidIndex(i); i++)
			{
				std::string name;
				chunk.ReadNullString(name);
				Patterns[i].SetName(name);
			}
			break;

		case MagicLE("MSG\0"):
			chunk.Skip(1);	// Show message on startup
			m_songMessage.Read(chunk, chunk.BytesLeft(), SongMessage::leCRLF);
			break;

		case MagicLE("PICT"):
			break;

		case MagicLE("SUM\0"):
			{
				uint8 summaryMask[6];
				chunk.ReadArray(summaryMask);
				std::string artist;
				chunk.ReadNullString(artist);
				if(artist != "Unregistered")
				{
					m_songArtist = mpt::ToUnicode(mpt::Charset::Windows1252, artist);
				}
			}
			break;

		case MagicLE("TMAP"):
			break;
		case MagicLE("MIDI"):
			break;
		case MagicLE("TREQ"):
			break;

		case MagicLE("VST2"):
			numVST = chunk.ReadUint32LE();
#ifdef MPT_WITH_VST
			if(!(loadFlags & loadPluginData))
			{
				break;
			}
			for(uint32 i = 0; i < std::min(numVST, uint32(MAX_MIXPLUGINS)); i++)
			{
				MT2VST vstHeader;
				if(chunk.ReadStruct(vstHeader))
				{
					if(fileHeader.version >= 0x0250)
						chunk.Skip(16 * 4);	// Parameter automation map for 16 parameters

					SNDMIXPLUGIN &mixPlug = m_MixPlugins[i];
					mixPlug.Destroy();
					std::string libraryName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, vstHeader.dll);
					mixPlug.Info.szName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, vstHeader.programName);
					if(libraryName.length() > 4 && libraryName[libraryName.length() - 4] == '.')
					{
						// Remove ".dll" from library name
						libraryName.resize(libraryName.length() - 4);
					}
					mixPlug.Info.szLibraryName = libraryName;
					mixPlug.Info.dwPluginId1 = Vst::kEffectMagic;
					mixPlug.Info.dwPluginId2 = vstHeader.fxID;
					if(vstHeader.track >= GetNumChannels())
					{
						mixPlug.SetMasterEffect(true);
					} else
					{
						if(!ChnSettings[vstHeader.track].nMixPlugin)
						{
							ChnSettings[vstHeader.track].nMixPlugin = static_cast<PLUGINDEX>(i + 1);
						} else
						{
							// Channel already has plugin assignment - chain the plugins
							PLUGINDEX outPlug = ChnSettings[vstHeader.track].nMixPlugin - 1;
							while(true)
							{
								if(m_MixPlugins[outPlug].GetOutputPlugin() == PLUGINDEX_INVALID)
								{
									m_MixPlugins[outPlug].SetOutputPlugin(static_cast<PLUGINDEX>(i));
									break;
								}
								outPlug = m_MixPlugins[outPlug].GetOutputPlugin();
							}
						}
					}

					// Read plugin settings
					uint32 dataSize;
					if(vstHeader.useChunks)
					{
						// MT2 only ever calls effGetChunk for programs, and OpenMPT uses the defaultProgram value to determine
						// whether it should use effSetChunk for programs or banks...
						mixPlug.defaultProgram = -1;
						LimitMax(vstHeader.n, std::numeric_limits<decltype(dataSize)>::max() - 4);
						dataSize = vstHeader.n + 4;
					} else
					{
						mixPlug.defaultProgram = vstHeader.programNr;
						LimitMax(vstHeader.n, (std::numeric_limits<decltype(dataSize)>::max() / 4u) - 1);
						dataSize = vstHeader.n * 4 + 4;
					}
					mixPlug.pluginData.resize(dataSize);
					if(vstHeader.useChunks)
					{
						std::memcpy(mixPlug.pluginData.data(), "fEvN", 4);	// 'NvEf' plugin data type
						chunk.ReadRaw(mpt::span(mixPlug.pluginData.data() + 4, vstHeader.n));
					} else
					{
						auto memFile = std::make_pair(mpt::as_span(mixPlug.pluginData), mpt::IO::Offset(0));
						mpt::IO::WriteIntLE<uint32>(memFile, 0);  // Plugin data type
						for(uint32 param = 0; param < vstHeader.n; param++)
						{
							mpt::IO::Write(memFile, IEEE754binary32LE{chunk.ReadFloatLE()});
						}
					}
				} else
				{
					break;
				}
			}
#endif // MPT_WITH_VST
			break;
		}
	}

#ifndef NO_PLUGINS
	// Now that we have both the track settings and plugins, establish the track routing by applying the same plugins to the source track as to the target track:
	for(CHANNELINDEX c = 0; c < GetNumChannels(); c++)
	{
		int8 outTrack = trackRouting[c];
		if(outTrack > c && outTrack < GetNumChannels() && ChnSettings[outTrack].nMixPlugin != 0)
		{
			if(ChnSettings[c].nMixPlugin == 0)
			{
				ChnSettings[c].nMixPlugin = ChnSettings[outTrack].nMixPlugin;
			} else
			{
				PLUGINDEX outPlug = ChnSettings[c].nMixPlugin - 1;
				for(;;)
				{
					if(m_MixPlugins[outPlug].GetOutputPlugin() == PLUGINDEX_INVALID)
					{
						m_MixPlugins[outPlug].SetOutputPlugin(ChnSettings[outTrack].nMixPlugin - 1);
						break;
					}
					outPlug = m_MixPlugins[outPlug].GetOutputPlugin();
				}
			}
		}
	}
#endif // NO_PLUGINS

	// Read drum channels
	INSTRUMENTINDEX drumMap[8] = { 0 };
	uint16 drumSample[8] = { 0 };
	if(hasDrumChannels)
	{
		MT2DrumsData drumHeader;
		drumData.ReadStruct(drumHeader);

		// Allocate some instruments to handle the drum samples
		for(INSTRUMENTINDEX i = 0; i < 8; i++)
		{
			drumMap[i] = GetNextFreeInstrument(m_nInstruments + 1);
			drumSample[i] = drumHeader.DrumSamples[i];
			if(drumMap[i] != INSTRUMENTINDEX_INVALID)
			{
				ModInstrument *mptIns = AllocateInstrument(drumMap[i], drumHeader.DrumSamples[i] + 1);
				if(mptIns != nullptr)
				{
					mptIns->name = MPT_AFORMAT("Drum #{}")(i+1);
				}
			} else
			{
				drumMap[i] = 0;
			}
		}

		// Get all the drum pattern chunks
		std::vector<FileReader> patternChunks(drumHeader.numDrumPatterns);
		for(uint32 pat = 0; pat < drumHeader.numDrumPatterns; pat++)
		{
			uint16 numRows = file.ReadUint16LE();
			patternChunks[pat] = file.ReadChunk(numRows * 32);
		}

		std::vector<PATTERNINDEX> patMapping(fileHeader.numPatterns, PATTERNINDEX_INVALID);
		for(uint32 ord = 0; ord < fileHeader.numOrders; ord++)
		{
			if(drumHeader.DrumPatternOrder[ord] >= drumHeader.numDrumPatterns || Order()[ord] >= fileHeader.numPatterns)
				continue;

			// Figure out where to write this drum pattern
			PATTERNINDEX writePat = Order()[ord];
			if(patMapping[writePat] == PATTERNINDEX_INVALID)
			{
				patMapping[writePat] = drumHeader.DrumPatternOrder[ord];
			} else if(patMapping[writePat] != drumHeader.DrumPatternOrder[ord])
			{
				// Damn, this pattern has previously used a different drum pattern. Duplicate it...
				PATTERNINDEX newPat = Patterns.Duplicate(writePat);
				if(newPat != PATTERNINDEX_INVALID)
				{
					writePat = newPat;
					Order()[ord] = writePat;
				}
			}
			if(!Patterns.IsValidPat(writePat))
				continue;

			FileReader &chunk = patternChunks[drumHeader.DrumPatternOrder[ord]];
			chunk.Rewind();
			const ROWINDEX numRows = static_cast<ROWINDEX>(chunk.GetLength() / 32u);
			for(ROWINDEX row = 0; row < Patterns[writePat].GetNumRows(); row++)
			{
				ModCommand *m = Patterns[writePat].GetpModCommand(row, GetNumChannels() - 8);
				for(CHANNELINDEX chn = 0; chn < 8; chn++, m++)
				{
					*m = ModCommand{};
					if(row >= numRows)
						continue;

					uint8 drums[4];
					chunk.ReadArray(drums);
					if(drums[0] & 0x80)
					{
						m->note = NOTE_MIDDLEC;
						m->instr = static_cast<ModCommand::INSTR>(drumMap[chn]);
						uint8 delay = drums[0] & 0x1F;
						if(delay)
						{
							LimitMax(delay, uint8(0x0F));
							m->command = CMD_S3MCMDEX;
							m->param = 0xD0 | delay;
						}
						m->volcmd = VOLCMD_VOLUME;
						// Volume is 0...255, but 128 is equivalent to v64 - we compensate this by halving the global volume of all non-drum instruments
						m->vol = static_cast<ModCommand::VOL>((static_cast<uint16>(drums[1]) + 3) / 4u);
					}
				}
			}
		}
	}

	// Read automation envelopes
	if(fileHeader.flags & MT2FileHeader::automation)
	{
		const uint32 numEnvelopes = ((fileHeader.flags & MT2FileHeader::drumsAutomation) ? GetNumChannels() : channelsWithoutDrums)
			+ ((fileHeader.version >= 0x0250) ? numVST : 0)
			+ ((fileHeader.flags & MT2FileHeader::masterAutomation) ? 1 : 0);

		for(uint32 pat = 0; pat < fileHeader.numPatterns; pat++)
		{
			for(uint32 env = 0; env < numEnvelopes && file.CanRead(4); env++)
			{
				// TODO
				ReadMT2Automation(fileHeader.version, file);
			}
		}
	}

	// Read instruments
	std::vector<FileReader> instrChunks(255);
	for(INSTRUMENTINDEX i = 0; i < 255; i++)
	{
		char instrName[32];
		file.ReadArray(instrName);
		uint32 dataLength = file.ReadUint32LE();
		if(dataLength == 32) dataLength += 108 + sizeof(MT2IEnvelope) * 4;
		if(fileHeader.version > 0x0201 && dataLength) dataLength += 4;

		FileReader instrChunk = instrChunks[i] = file.ReadChunk(dataLength);

		ModInstrument *mptIns = nullptr;
		if(i < fileHeader.numInstruments)
		{
			// Default sample assignment if there is no data chunk? Fixes e.g. instrument 33 in Destiny - Dream Alone.mt2
			mptIns = AllocateInstrument(i + 1, i + 1);
		}
		if(mptIns == nullptr)
			continue;

		mptIns->name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, instrName);

		if(!dataLength)
			continue;

		MT2Instrument insHeader;
		instrChunk.ReadStruct(insHeader);
		uint16 flags = 0;
		if(fileHeader.version >= 0x0201) flags = instrChunk.ReadUint16LE();
		uint32 envMask = MT2Instrument::VolumeEnv | MT2Instrument::PanningEnv;
		if(fileHeader.version >= 0x0202) envMask = instrChunk.ReadUint32LE();

		mptIns->nFadeOut = insHeader.fadeout;
		const NewNoteAction NNA[4]       = { NewNoteAction::NoteCut, NewNoteAction::Continue, NewNoteAction::NoteOff, NewNoteAction::NoteFade };
		const DuplicateCheckType DCT[4]  = { DuplicateCheckType::None, DuplicateCheckType::Note, DuplicateCheckType::Sample, DuplicateCheckType::Instrument };
		const DuplicateNoteAction DNA[4] = { DuplicateNoteAction::NoteCut, DuplicateNoteAction::NoteFade /* actually continue, but IT doesn't have that */, DuplicateNoteAction::NoteOff, DuplicateNoteAction::NoteFade };
		mptIns->nNNA = NNA[insHeader.nna & 3];
		mptIns->nDCT = DCT[(insHeader.nna >> 8) & 3];
		mptIns->nDNA = DNA[(insHeader.nna >> 12) & 3];

		// Load envelopes
		for(uint32 env = 0; env < 4; env++)
		{
			if(envMask & 1)
			{
				MT2IEnvelope mt2Env;
				instrChunk.ReadStruct(mt2Env);

				const EnvelopeType envType[4] = { ENV_VOLUME, ENV_PANNING, ENV_PITCH, ENV_PITCH };
				InstrumentEnvelope &mptEnv = mptIns->GetEnvelope(envType[env]);

				mptEnv.dwFlags.set(ENV_FILTER, (env == 3) && (mt2Env.flags & 1) != 0);
				mptEnv.dwFlags.set(ENV_ENABLED, (mt2Env.flags & 1) != 0);
				mptEnv.dwFlags.set(ENV_SUSTAIN, (mt2Env.flags & 2) != 0);
				mptEnv.dwFlags.set(ENV_LOOP, (mt2Env.flags & 4) != 0);
				mptEnv.resize(std::min(mt2Env.numPoints.get(), uint8(16)));
				mptEnv.nSustainStart = mptEnv.nSustainEnd = mt2Env.sustainPos;
				mptEnv.nLoopStart = mt2Env.loopStart;
				mptEnv.nLoopEnd = mt2Env.loopEnd;

				for(uint32 p = 0; p < mptEnv.size(); p++)
				{
					mptEnv[p].tick = mt2Env.points[p].x;
					mptEnv[p].value = static_cast<uint8>(Clamp<uint16, uint16>(mt2Env.points[p].y, 0, 64));
				}
			}
			envMask >>= 1;
		}
		if(!mptIns->VolEnv.dwFlags[ENV_ENABLED] && mptIns->nNNA != NewNoteAction::NoteFade)
		{
			mptIns->nFadeOut = int16_max;
		}

		mptIns->SetCutoff(0x7F, true);
		mptIns->SetResonance(0, true);

		if(flags)
		{
			MT2InstrSynth synthData;
			instrChunk.ReadStruct(synthData);

			if(flags & 2)
			{
				mptIns->SetCutoff(FrequencyToCutOff(synthData.cutoff), true);
				mptIns->SetResonance(synthData.resonance, true);
			}
			mptIns->filterMode = synthData.effectID == 1 ? FilterMode::HighPass : FilterMode::LowPass;
			if(flags & 4)
			{
				// VSTi / MIDI synth enabled
				mptIns->nMidiChannel = synthData.midiChannel + 1;
				mptIns->nMixPlug = static_cast<PLUGINDEX>(synthData.device + 1);
				if(synthData.device < 0)
				{
					// TODO: This is a MIDI device - maybe use MIDI I/O plugin to emulate those?
					mptIns->nMidiProgram = synthData.midiProgram + 1;	// MT2 only seems to use this for MIDI devices, not VSTis!
				}
				if(synthData.transpose)
				{
					for(uint32 n = 0; n < std::size(mptIns->NoteMap); n++)
					{
						int note = NOTE_MIN + n + synthData.transpose;
						Limit(note, NOTE_MIN, NOTE_MAX);
						mptIns->NoteMap[n] = static_cast<uint8>(note);
					}
				}
				// Instruments with plugin assignments never play samples at the same time!
				mptIns->AssignSample(0);
			}
		}
	}

	// Read sample headers
	std::bitset<256> sampleNoInterpolation;
	std::bitset<256> sampleSynchronized;
	for(SAMPLEINDEX i = 0; i < 256; i++)
	{
		char sampleName[32];
		file.ReadArray(sampleName);
		uint32 dataLength = file.ReadUint32LE();

		FileReader sampleChunk = file.ReadChunk(dataLength);

		if(i < fileHeader.numSamples)
		{
			m_szNames[i + 1] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleName);
		}

		if(dataLength && i < fileHeader.numSamples)
		{
			ModSample &mptSmp = Samples[i + 1];
			mptSmp.Initialize(MOD_TYPE_IT);
			mptSmp.SetDefaultCuePoints();
			MT2Sample sampleHeader;
			sampleChunk.ReadStruct(sampleHeader);

			mptSmp.nLength = sampleHeader.length;
			mptSmp.nC5Speed = sampleHeader.frequency;
			if(sampleHeader.depth > 1) { mptSmp.uFlags.set(CHN_16BIT); mptSmp.nLength /= 2u; }
			if(sampleHeader.channels > 1) { mptSmp.uFlags.set(CHN_STEREO); mptSmp.nLength /= 2u; }
			if(sampleHeader.loopType == 1) mptSmp.uFlags.set(CHN_LOOP);
			else if(sampleHeader.loopType == 2) mptSmp.uFlags.set(CHN_LOOP | CHN_PINGPONGLOOP);
			mptSmp.nLoopStart = sampleHeader.loopStart;
			mptSmp.nLoopEnd = sampleHeader.loopEnd;
			mptSmp.nVolume = sampleHeader.volume >> 7;
			if(sampleHeader.panning == -128)
				mptSmp.uFlags.set(CHN_SURROUND);
			else
				mptSmp.nPan = static_cast<uint16>(sampleHeader.panning + 128);
			mptSmp.uFlags.set(CHN_PANNING);
			mptSmp.RelativeTone = sampleHeader.note;

			if(sampleHeader.flags & 2)
			{
				// Sample is synchronized to beat
				// The synchronization part is not supported in OpenMPT, but synchronized samples also always play at the same pitch as C-5, which we CAN do!
				sampleSynchronized[i] = true;
				//mptSmp.nC5Speed = Util::muldiv(mptSmp.nC5Speed, sampleHeader.spb, 22050);
			}
			if(sampleHeader.flags & 5)
			{
				// External sample
				mptSmp.uFlags.set(SMP_KEEPONDISK);
			}
			if(sampleHeader.flags & 8)
			{
				sampleNoInterpolation[i] = true;
				for(INSTRUMENTINDEX drum = 0; drum < 8; drum++)
				{
					if(drumSample[drum] == i && Instruments[drumMap[drum]] != nullptr)
					{
						Instruments[drumMap[drum]]->resampling = SRCMODE_NEAREST;
					}
				}
			}
		}
	}

	// Read sample groups
	for(INSTRUMENTINDEX ins = 0; ins < fileHeader.numInstruments; ins++)
	{
		if(instrChunks[ins].GetLength())
		{
			FileReader &chunk = instrChunks[ins];
			MT2Instrument insHeader;
			chunk.Rewind();
			chunk.ReadStruct(insHeader);

			std::vector<MT2Group> groups;
			file.ReadVector(groups, insHeader.numSamples);

			ModInstrument *mptIns = Instruments[ins + 1];
			// Instruments with plugin assignments never play samples at the same time!
			if(mptIns == nullptr || mptIns->nMixPlug != 0)
				continue;

			mptIns->nGlobalVol = 32;	// Compensate for extended dynamic range of drum instruments
			mptIns->AssignSample(0);
			for(uint32 note = 0; note < 96; note++)
			{
				if(insHeader.groupMap[note] < insHeader.numSamples)
				{
					const MT2Group &group = groups[insHeader.groupMap[note]];
					SAMPLEINDEX sample = group.sample + 1;
					mptIns->Keyboard[note + 11 + NOTE_MIN] = sample;
					if(sample > 0 && sample <= m_nSamples)
					{
						ModSample &mptSmp = Samples[sample];
						mptSmp.nVibType = static_cast<VibratoType>(insHeader.vibtype & 3); // In fact, MT2 only implements sine vibrato
						mptSmp.nVibSweep = insHeader.vibsweep;
						mptSmp.nVibDepth = insHeader.vibdepth;
						mptSmp.nVibRate = insHeader.vibrate;
						mptSmp.nGlobalVol = uint16(group.vol) * 2;
						mptSmp.nFineTune = group.pitch;
						if(sampleNoInterpolation[sample - 1])
						{
							mptIns->resampling = SRCMODE_NEAREST;
						}
						if(sampleSynchronized[sample - 1])
						{
							mptIns->NoteMap[note + 11 + NOTE_MIN] = NOTE_MIDDLEC;
						}
					}
					// TODO: volume, finetune for duplicated samples
				}
			}
		}
	}

	if(!(loadFlags & loadSampleData))
		return true;

	// Read sample data
	for(SAMPLEINDEX i = 0; i < m_nSamples; i++)
	{
		ModSample &mptSmp = Samples[i + 1];
		mptSmp.Transpose(-(mptSmp.RelativeTone - 49 - (mptSmp.nFineTune / 128.0)) / 12.0);
		mptSmp.nFineTune = 0;
		mptSmp.RelativeTone = 0;

		if(!mptSmp.uFlags[SMP_KEEPONDISK])
		{
			SampleIO(
				mptSmp.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				mptSmp.uFlags[CHN_STEREO] ? SampleIO::stereoSplit : SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::MT2)
				.ReadSample(mptSmp, file);
		} else
		{
			// External sample
			const uint32 filenameSize = file.ReadUint32LE();
			file.Skip(12); // Reserved
			std::string filename;
			file.ReadString<mpt::String::maybeNullTerminated>(filename, filenameSize);
			mptSmp.filename = filename;

#if defined(MPT_EXTERNAL_SAMPLES)
			SetSamplePath(i + 1, mpt::PathString::FromLocale(filename));
#elif !defined(LIBOPENMPT_BUILD_TEST)
			AddToLog(LogWarning, MPT_UFORMAT("Loading external sample {} ('{}') failed: External samples are not supported.")(i + 1, mpt::ToUnicode(GetCharsetFile(), filename)));
#endif // MPT_EXTERNAL_SAMPLES
		}
	}

	return true;
}

OPENMPT_NAMESPACE_END
