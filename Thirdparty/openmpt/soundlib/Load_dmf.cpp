/*
 * load_dmf.cpp
 * ------------
 * Purpose: DMF module loader (X-Tracker by D-LUSiON).
 * Notes  : If it wasn't already outdated when the tracker left beta state, this would be a rather interesting
 *          and in some parts even sophisticated format - effect columns are separated by effect type, an easy to
 *          understand BPM tempo mode, effect durations are always divided into a 256th row, vibrato effects are
 *          specified by period length and the same 8-Bit granularity is used for both volume and panning.
 *          Unluckily, this format does not offer any envelopes or multi-sample instruments, and bidi sample loops
 *          are missing as well, so it was already well behind FT2 back then.
 * Authors: Johannes Schultz (mostly based on DMF.TXT, DMF_EFFC.TXT, trial and error and some invaluable hints by Zatzen)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "BitReader.h"

OPENMPT_NAMESPACE_BEGIN

// DMF header
struct DMFFileHeader
{
	char   signature[4];  // "DDMF"
	uint8  version;       // 1 - 7 are beta versions, 8 is the official thing, 10 is xtracker32
	char   tracker[8];    // "XTRACKER", or "SCREAM 3" when converting from S3M, etc.
	char   songname[30];
	char   composer[20];
	uint8  creationDay;
	uint8  creationMonth;
	uint8  creationYear;
};

MPT_BINARY_STRUCT(DMFFileHeader, 66)

struct DMFChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idCMSG = MagicLE("CMSG"),  // Song message
		idSEQU = MagicLE("SEQU"),  // Order list
		idPATT = MagicLE("PATT"),  // Patterns
		idSMPI = MagicLE("SMPI"),  // Sample headers
		idSMPD = MagicLE("SMPD"),  // Sample data
		idSMPJ = MagicLE("SMPJ"),  // Sample jump table (XTracker 32 only)
		idENDE = MagicLE("ENDE"),  // Last four bytes of DMF file
		idSETT = MagicLE("SETT"),  // Probably contains GUI settings
	};

	uint32le id;
	uint32le length;

	size_t GetLength() const
	{
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(DMFChunk, 8)

// Pattern header (global)
struct DMFPatterns
{
	uint16le numPatterns;  // 1..1024 patterns
	uint8le  numTracks;    // 1..32 channels
};

MPT_BINARY_STRUCT(DMFPatterns, 3)

// Pattern header (for each pattern)
struct DMFPatternHeader
{
	uint8le  numTracks;  // 1..32 channels
	uint8le  beat;       // [hi|lo] -> hi = rows per beat, lo = reserved
	uint16le numRows;
	uint32le patternLength;
	// patttern data follows here ...
};

MPT_BINARY_STRUCT(DMFPatternHeader, 8)

// Sample header
struct DMFSampleHeader
{
	enum SampleFlags
	{
		// Sample flags
		smpLoop     = 0x01,
		smp16Bit    = 0x02,
		smpCompMask = 0x0C,
		smpComp1    = 0x04,  // Compression type 1
		smpComp2    = 0x08,  // Compression type 2 (unused)
		smpComp3    = 0x0C,  // Compression type 3 (ditto)
		smpLibrary  = 0x80,  // Sample is stored in a library
	};

	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
	uint16le c3freq;  // 1000..45000hz
	uint8le  volume;  // 0 = ignore
	uint8le  flags;

	// Convert an DMFSampleHeader to OpenMPT's internal sample representation.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nLength = length;
		mptSmp.nSustainStart = loopStart;
		mptSmp.nSustainEnd = loopEnd;

		mptSmp.nC5Speed = c3freq;
		mptSmp.nGlobalVol = 64;
		if(volume)
			mptSmp.nVolume = volume + 1;
		else
			mptSmp.nVolume = 256;
		mptSmp.uFlags.set(SMP_NODEFAULTVOLUME, volume == 0);

		if((flags & smpLoop) != 0 && mptSmp.nSustainEnd > mptSmp.nSustainStart)
		{
			mptSmp.uFlags.set(CHN_SUSTAINLOOP);
		}
		if((flags & smp16Bit) != 0)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nLength /= 2;
			mptSmp.nSustainStart /= 2;
			mptSmp.nSustainEnd /= 2;
		}
	}
};

MPT_BINARY_STRUCT(DMFSampleHeader, 16)


// Pattern translation memory
struct DMFPatternSettings
{
	struct ChannelState
	{
		ModCommand::NOTE noteBuffer = NOTE_NONE; // Note buffer
		ModCommand::NOTE lastNote = NOTE_NONE;   // Last played note on channel
		uint8 vibratoType = 8;                   // Last used vibrato type on channel
		uint8 tremoloType = 4;                   // Last used tremolo type on channel
		uint8 highOffset = 6;                    // Last used high offset on channel
		bool playDir = false;                    // Sample play direction... false = forward (default)
	};

	std::vector<ChannelState> channels; // Memory for each channel's state
	bool realBPMmode = false;           // true = BPM mode
	uint8 beat = 0;                     // Rows per beat
	uint8 tempoTicks = 32;              // Tick mode param
	uint8 tempoBPM = 120;               // BPM mode param
	uint8 internalTicks = 6;            // Ticks per row in final pattern

	DMFPatternSettings(CHANNELINDEX numChannels)
	    : channels(numChannels)
	{ }
};


// Convert portamento value (not very accurate due to X-Tracker's higher granularity, to say the least)
static uint8 DMFporta2MPT(uint8 val, const uint8 internalTicks, const bool hasFine)
{
	if(val == 0)
		return 0;
	else if((val <= 0x0F && hasFine) || internalTicks < 2)
		return (val | 0xF0);
	else
		return std::max(uint8(1), static_cast<uint8>((val / (internalTicks - 1))));	// no porta on first tick!
}


// Convert portamento / volume slide value (not very accurate due to X-Tracker's higher granularity, to say the least)
static uint8 DMFslide2MPT(uint8 val, const uint8 internalTicks, const bool up)
{
	val = std::max(uint8(1), static_cast<uint8>(val / 4));
	const bool isFine = (val < 0x0F) || (internalTicks < 2);
	if(!isFine)
		val = std::max(uint8(1), static_cast<uint8>((val + internalTicks - 2) / (internalTicks - 1)));	// no slides on first tick! "+ internalTicks - 2" for rounding precision

	if(up)
		return (isFine ? 0x0F : 0x00) | (val << 4);
	else
		return (isFine ? 0xF0 : 0x00) | (val & 0x0F);

}


// Calculate tremor on/off param
static uint8 DMFtremor2MPT(uint8 val, const uint8 internalTicks)
{
	uint8 ontime = (val >> 4);
	uint8 offtime = (val & 0x0F);
	ontime = static_cast<uint8>(Clamp(ontime * internalTicks / 15, 1, 15));
	offtime = static_cast<uint8>(Clamp(offtime * internalTicks / 15, 1, 15));
	return (ontime << 4) | offtime;
}


// Calculate delay parameter for note cuts / delays
static uint8 DMFdelay2MPT(uint8 val, const uint8 internalTicks)
{
	int newval = (int)val * (int)internalTicks / 255;
	Limit(newval, 0, 15);
	return (uint8)newval;
}


// Convert vibrato-style command parameters
static uint8 DMFvibrato2MPT(uint8 val, const uint8 internalTicks)
{
	// MPT: 1 vibrato period == 64 ticks... we have internalTicks ticks per row.
	// X-Tracker: Period length specified in rows!
	const int periodInTicks = std::max(1, (val >> 4)) * internalTicks;
	const uint8 matchingPeriod = static_cast<uint8>(Clamp((128 / periodInTicks), 1, 15));
	return (matchingPeriod << 4) | std::max(uint8(1), static_cast<uint8>(val & 0x0F));
}


// Try using effect memory (zero paramer) to give the effect swapper some optimization hints.
static void ApplyEffectMemory(const ModCommand *m, ROWINDEX row, CHANNELINDEX numChannels, EffectCommand effect, uint8 &param)
{
	if(effect == CMD_NONE || param == 0)
		return;

	const bool isTonePortaEffect = (effect == CMD_PORTAMENTOUP || effect == CMD_PORTAMENTODOWN || effect == CMD_TONEPORTAMENTO);
	const bool isVolSlideEffect = (effect == CMD_VOLUMESLIDE || effect == CMD_TONEPORTAVOL || effect == CMD_VIBRATOVOL);

	while(row > 0)
	{
		m -= numChannels;
		row--;

		// First, keep some extra rules in mind for portamento, where effect memory is shared between various commands.
		bool isSame = (effect == m->command);
		if(isTonePortaEffect && (m->command == CMD_PORTAMENTOUP || m->command == CMD_PORTAMENTODOWN || m->command == CMD_TONEPORTAMENTO))
		{
			if(m->param < 0xE0)
			{
				// Avoid effect param for fine slides, or else we could accidentally put this command in the volume column, where fine slides won't work!
				isSame = true;
			} else
			{
				return;
			}
		} else if(isVolSlideEffect && (m->command == CMD_VOLUMESLIDE || m->command == CMD_TONEPORTAVOL || m->command == CMD_VIBRATOVOL))
		{
			isSame = true;
		}
		if(isTonePortaEffect
			&& (m->volcmd == VOLCMD_PORTAUP || m->volcmd == VOLCMD_PORTADOWN || m->volcmd == VOLCMD_TONEPORTAMENTO)
			&& m->vol != 0)
		{
			// Uuh... Don't even try
			return;
		} else if(isVolSlideEffect
			&& (m->volcmd == VOLCMD_FINEVOLUP || m->volcmd == VOLCMD_FINEVOLDOWN || m->volcmd == VOLCMD_VOLSLIDEUP || m->volcmd == VOLCMD_VOLSLIDEDOWN)
			&& m->vol != 0)
		{
			// Same!
			return;
		}

		if(isSame)
		{
			if(param != m->param && m->param != 0)
			{
				// No way to optimize this
				return;
			} else if(param == m->param)
			{
				// Yay!
				param = 0;
				return;
			}
		}
	}
}


static PATTERNINDEX ConvertDMFPattern(FileReader &file, const uint8 fileVersion, DMFPatternSettings &settings, CSoundFile &sndFile)
{
	// Pattern flags
	enum PatternFlags
	{
		// Global Track
		patGlobPack = 0x80,  // Pack information for global track follows
		patGlobMask = 0x3F,  // Mask for global effects
		// Note tracks
		patCounter = 0x80,  // Pack information for current channel follows
		patInstr   = 0x40,  // Instrument number present
		patNote    = 0x20,  // Note present
		patVolume  = 0x10,  // Volume present
		patInsEff  = 0x08,  // Instrument effect present
		patNoteEff = 0x04,  // Note effect present
		patVolEff  = 0x02,  // Volume effect stored
	};

	file.Rewind();
	
	DMFPatternHeader patHead;
	if(fileVersion < 3)
	{
		patHead.numTracks = file.ReadUint8();
		file.Skip(2);  // not sure what this is, later X-Tracker versions just skip over it
		patHead.numRows = file.ReadUint16LE();
		patHead.patternLength = file.ReadUint32LE();
	} else
	{
		file.ReadStruct(patHead);
	}
	if(fileVersion < 6)
		patHead.beat = 0;

	const ROWINDEX numRows = Clamp(ROWINDEX(patHead.numRows), ROWINDEX(1), MAX_PATTERN_ROWS);
	const PATTERNINDEX pat = sndFile.Patterns.InsertAny(numRows);
	if(pat == PATTERNINDEX_INVALID)
	{
		return pat;
	}

	ModCommand *m = sndFile.Patterns[pat].GetpModCommand(0, 0);
	const CHANNELINDEX numChannels = std::min(static_cast<CHANNELINDEX>(sndFile.GetNumChannels() - 1), static_cast<CHANNELINDEX>(patHead.numTracks));

	// When breaking to a pattern with less channels that the previous pattern,
	// all voices in the now unused channels are killed:
	for(CHANNELINDEX chn = numChannels + 1; chn < sndFile.GetNumChannels(); chn++)
	{
		m[chn].note = NOTE_NOTECUT;
	}

	// Initialize tempo stuff
	settings.beat = (patHead.beat >> 4);
	bool tempoChange = settings.realBPMmode;
	uint8 writeDelay = 0;

	// Counters for channel packing (including global track)
	std::vector<uint8> channelCounter(numChannels + 1, 0);

	for(ROWINDEX row = 0; row < numRows && file.CanRead(1); row++)
	{
		// Global track info counter reached 0 => read global track data
		if(channelCounter[0] == 0)
		{
			uint8 globalInfo = file.ReadUint8();
			// 0x80: Packing counter (if not present, counter stays at 0)
			if((globalInfo & patGlobPack) != 0)
			{
				channelCounter[0] = file.ReadUint8();
			}

			globalInfo &= patGlobMask;

			uint8 globalData = 0;
			if(globalInfo != 0)
			{
				globalData = file.ReadUint8();
			}

			switch(globalInfo)
			{
			case 1:  // Set Tick Frame Speed
				settings.realBPMmode = false;
				settings.tempoTicks = std::max(uint8(1), globalData);  // Tempo in 1/4 rows per second
				settings.tempoBPM = 0;                                 // Automatically updated by X-Tracker
				tempoChange = true;
				break;
			case 2:             // Set BPM Speed (real BPM mode)
				if(globalData)  // DATA = 0 doesn't do anything
				{
					settings.realBPMmode = true;
					settings.tempoBPM = globalData;  // Tempo in real BPM (depends on rows per beat)
					if(settings.beat != 0)
					{
						settings.tempoTicks = static_cast<uint8>(globalData * settings.beat * 15);  // Automatically updated by X-Tracker
					}
					tempoChange = true;
				}
				break;
			case 3:  // Set Beat
				settings.beat = (globalData >> 4);
				if(settings.beat != 0)
				{
					// Tempo changes only if we're in real BPM mode
					tempoChange = settings.realBPMmode;
				} else
				{
					// If beat is 0, change to tick speed mode, but keep current tempo
					settings.realBPMmode = false;
				}
				break;
			case 4:  // Tick Delay
				writeDelay = globalData;
				break;
			case 5:  // Set External Flag
				break;
			case 6:  // Slide Speed Up
				if(globalData > 0)
				{
					uint8 &tempoData = (settings.realBPMmode) ? settings.tempoBPM : settings.tempoTicks;
					if(tempoData < 256 - globalData)
					{
						tempoData += globalData;
					} else
					{
						tempoData = 255;
					}
					tempoChange = true;
				}
				break;
			case 7:  // Slide Speed Down
				if(globalData > 0)
				{
					uint8 &tempoData = (settings.realBPMmode) ? settings.tempoBPM : settings.tempoTicks;
					if(tempoData > 1 + globalData)
					{
						tempoData -= globalData;
					} else
					{
						tempoData = 1;
					}
					tempoChange = true;
				}
				break;
			}
		} else
		{
			channelCounter[0]--;
		}

		// These will eventually be written to the pattern
		int speed = 0, tempo = 0;

		if(tempoChange)
		{
			// Can't do anything if we're in BPM mode and there's no rows per beat set...
			if(!settings.realBPMmode || settings.beat)
			{
				// My approach to convert X-Tracker's "tick speed" (1/4 rows per second):
				// Tempo * 6 / Speed = Beats per Minute
				// => Tempo * 6 / (Speed * 60) = Beats per Second
				// => Tempo * 24 / (Speed * 60) = Rows per Second (4 rows per beat at tempo 6)
				// => Tempo = 60 * Rows per Second * Speed / 24
				// For some reason, using settings.tempoTicks + 1 gives more accurate results than just settings.tempoTicks... (same problem in the old libmodplug DMF loader)
				// Original unoptimized formula:
				//const int tickspeed = (tempoRealBPMmode) ? std::max(1, (tempoData * beat * 4) / 60) : tempoData;
				const int tickspeed = (settings.realBPMmode) ? std::max(1, settings.tempoBPM * settings.beat * 2) : ((settings.tempoTicks + 1) * 30);
				// Try to find matching speed - try higher speeds first, so that effects like arpeggio and tremor work better.
				for(speed = 255; speed >= 1; speed--)
				{
					// Original unoptimized formula:
					// tempo = 30 * tickspeed * speed / 48;
					tempo = tickspeed * speed / 48;
					if(tempo >= 32 && tempo <= 255)
						break;
				}
				Limit(tempo, 32, 255);
				settings.internalTicks = static_cast<uint8>(std::max(1, speed));
			} else
			{
				tempoChange = false;
			}
		}

		m = sndFile.Patterns[pat].GetpModCommand(row, 1);  // Reserve first channel for global effects

		for(CHANNELINDEX chn = 1; chn <= numChannels; chn++, m++)
		{
			// Track info counter reached 0 => read track data
			if(channelCounter[chn] == 0)
			{
				const uint8 channelInfo = file.ReadUint8();
				////////////////////////////////////////////////////////////////
				// 0x80: Packing counter (if not present, counter stays at 0)
				if((channelInfo & patCounter) != 0)
				{
					channelCounter[chn] = file.ReadUint8();
				}

				////////////////////////////////////////////////////////////////
				// 0x40: Instrument
				bool slideNote = true;  // If there is no instrument number next to a note, the note is not retriggered!
				if((channelInfo & patInstr) != 0)
				{
					m->instr = file.ReadUint8();
					if(m->instr != 0)
					{
						slideNote = false;
					}
				}

				////////////////////////////////////////////////////////////////
				// 0x20: Note
				if((channelInfo & patNote) != 0)
				{
					m->note = file.ReadUint8();
					if(m->note >= 1 && m->note <= 108)
					{
						m->note = Clamp(static_cast<ModCommand::NOTE>(m->note + 24), NOTE_MIN, NOTE_MAX);
						settings.channels[chn].lastNote = m->note;
					} else if(m->note >= 129 && m->note <= 236)
					{
						// "Buffer notes" for portamento (and other effects?) that are actually not played, but just "queued"...
						m->note = Clamp(static_cast<ModCommand::NOTE>((m->note & 0x7F) + 24), NOTE_MIN, NOTE_MAX);
						settings.channels[chn].noteBuffer = m->note;
						m->note = NOTE_NONE;
					} else if(m->note == 255)
					{
						m->note = NOTE_NOTECUT;
					}
				}

				// If there's just an instrument number, but no note, retrigger sample.
				if(m->note == NOTE_NONE && m->instr > 0)
				{
					m->note = settings.channels[chn].lastNote;
					m->instr = 0;
				}

				if(m->IsNote())
				{
					settings.channels[chn].playDir = false;
				}

				EffectCommand effect1 = CMD_NONE, effect2 = CMD_NONE, effect3 = CMD_NONE;
				uint8 effectParam1 = 0, effectParam2 = 0, effectParam3 = 0;
				bool useMem2 = false, useMem3 = false;	// Effect can use memory if necessary

				////////////////////////////////////////////////////////////////
				// 0x10: Volume
				if((channelInfo & patVolume) != 0)
				{
					m->volcmd = VOLCMD_VOLUME;
					m->vol = static_cast<ModCommand::VOL>((file.ReadUint8() + 2) / 4);  // Should be + 3 instead of + 2, but volume 1 is silent in X-Tracker.
				}

				////////////////////////////////////////////////////////////////
				// 0x08: Instrument effect
				if((channelInfo & patInsEff) != 0)
				{
					const uint8 command = file.ReadUint8();
					effectParam1 = file.ReadUint8();

					switch(command)
					{
					case 1:  // Stop Sample
						m->note = NOTE_NOTECUT;
						effect1 = CMD_NONE;
						break;
					case 2:  // Stop Sample Loop
						m->note = NOTE_KEYOFF;
						effect1 = CMD_NONE;
						break;
					case 3:  // Instrument Volume Override (aka "Restart")
						m->note = settings.channels[chn].lastNote;
						settings.channels[chn].playDir = false;
						effect1 = CMD_NONE;
						break;
					case 4:  // Sample Delay
						effectParam1 = DMFdelay2MPT(effectParam1, settings.internalTicks);
						if(effectParam1)
						{
							effect1 = CMD_S3MCMDEX;
							effectParam1 = 0xD0 | (effectParam1);
						} else
						{
							effect1 = CMD_NONE;
						}
						if(m->note == NOTE_NONE)
						{
							m->note = settings.channels[chn].lastNote;
							settings.channels[chn].playDir = false;
						}
						break;
					case 5:  // Tremolo Retrig Sample (who invented those stupid effect names?)
						effectParam1 = std::max(uint8(1), DMFdelay2MPT(effectParam1, settings.internalTicks));
						effect1 = CMD_RETRIG;
						settings.channels[chn].playDir = false;
						break;
					case 6:  // Offset
					case 7:  // Offset + 64k
					case 8:  // Offset + 128k
					case 9:  // Offset + 192k
						// Put high offset on previous row
						if(row > 0 && command != settings.channels[chn].highOffset)
						{
							if(sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_S3MCMDEX, (0xA0 | (command - 6))).Row(row - 1).Channel(chn).RetryPreviousRow()))
							{
								settings.channels[chn].highOffset = command;
							}
						}
						effect1 = CMD_OFFSET;
						if(m->note == NOTE_NONE)
						{
							// Offset without note does also work in DMF.
							m->note = settings.channels[chn].lastNote;
						}
						settings.channels[chn].playDir = false;
						break;
					case 10:	// Invert Sample play direction ("Tekkno Invert")
						effect1 = CMD_S3MCMDEX;
						if(settings.channels[chn].playDir == false)
							effectParam1 = 0x9F;
						else
							effectParam1 = 0x9E;
						settings.channels[chn].playDir = !settings.channels[chn].playDir;
						break;
					default:
						effect1 = CMD_NONE;
						break;
					}
				}

				////////////////////////////////////////////////////////////////
				// 0x04: Note effect
				if((channelInfo & patNoteEff) != 0)
				{
					const uint8 command = file.ReadUint8();
					effectParam2 = file.ReadUint8();

					switch(command)
					{
					case 1:  // Note Finetune (1/16th of a semitone signed 8-bit value, not 1/128th as the interface claims)
						{
							const auto fine = std::div(static_cast<int8>(effectParam2) * 8, 128);
							if(m->IsNote())
								m->note = static_cast<ModCommand::NOTE>(Clamp(m->note + fine.quot, NOTE_MIN, NOTE_MAX));
							effect2 = CMD_FINETUNE;
							effectParam2 = static_cast<uint8>(fine.rem) ^ 0x80;
						}
						break;
					case 2:  // Note Delay (wtf is the difference to Sample Delay?)
						effectParam2 = DMFdelay2MPT(effectParam2, settings.internalTicks);
						if(effectParam2)
						{
							effect2 = CMD_S3MCMDEX;
							effectParam2 = 0xD0 | (effectParam2);
						} else
						{
							effect2 = CMD_NONE;
						}
						useMem2 = true;
						break;
					case 3:  // Arpeggio
						effect2 = CMD_ARPEGGIO;
						useMem2 = true;
						break;
					case 4:  // Portamento Up
					case 5:  // Portamento Down
						effectParam2 = DMFporta2MPT(effectParam2, settings.internalTicks, true);
						effect2 = (command == 4) ? CMD_PORTAMENTOUP : CMD_PORTAMENTODOWN;
						useMem2 = true;
						break;
					case 6:  // Portamento to Note
						if(m->note == NOTE_NONE)
						{
							m->note = settings.channels[chn].noteBuffer;
						}
						effectParam2 = DMFporta2MPT(effectParam2, settings.internalTicks, false);
						effect2 = CMD_TONEPORTAMENTO;
						useMem2 = true;
						break;
					case 7:  // Scratch to Note (neat! but we don't have such an effect...)
						m->note = static_cast<ModCommand::NOTE>(Clamp(effectParam2 + 25, NOTE_MIN, NOTE_MAX));
						effect2 = CMD_TONEPORTAMENTO;
						effectParam2 = 0xFF;
						useMem2 = true;
						break;
					case 8:   // Vibrato Sine
					case 9:   // Vibrato Triangle (ramp down should be close enough)
					case 10:  // Vibrato Square
						// Put vibrato type on previous row
						if(row > 0 && command != settings.channels[chn].vibratoType)
						{
							if(sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_S3MCMDEX, (0x30 | (command - 8))).Row(row - 1).Channel(chn).RetryPreviousRow()))
							{
								settings.channels[chn].vibratoType = command;
							}
						}
						effect2 = CMD_VIBRATO;
						effectParam2 = DMFvibrato2MPT(effectParam2, settings.internalTicks);
						useMem2 = true;
						break;
					case 11:  // Note Tremolo
						effectParam2 = DMFtremor2MPT(effectParam2, settings.internalTicks);
						effect2 = CMD_TREMOR;
						useMem2 = true;
						break;
					case 12:  // Note Cut
						effectParam2 = DMFdelay2MPT(effectParam2, settings.internalTicks);
						if(effectParam2)
						{
							effect2 = CMD_S3MCMDEX;
							effectParam2 = 0xC0 | (effectParam2);
						} else
						{
							effect2 = CMD_NONE;
							m->note = NOTE_NOTECUT;
						}
						useMem2 = true;
						break;
					default:
						effect2 = CMD_NONE;
						break;
					}
				}

				////////////////////////////////////////////////////////////////
				// 0x02: Volume effect
				if((channelInfo & patVolEff) != 0)
				{
					const uint8 command = file.ReadUint8();
					effectParam3 = file.ReadUint8();

					switch(command)
					{
					case 1:  // Volume Slide Up
					case 2:  // Volume Slide Down
						effectParam3 = DMFslide2MPT(effectParam3, settings.internalTicks, (command == 1));
						effect3 = CMD_VOLUMESLIDE;
						useMem3 = true;
						break;
					case 3:  // Volume Tremolo (actually this is Tremor)
						effectParam3 = DMFtremor2MPT(effectParam3, settings.internalTicks);
						effect3 = CMD_TREMOR;
						useMem3 = true;
						break;
					case 4:  // Tremolo Sine
					case 5:  // Tremolo Triangle (ramp down should be close enough)
					case 6:  // Tremolo Square
						// Put tremolo type on previous row
						if(row > 0 && command != settings.channels[chn].tremoloType)
						{
							if(sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_S3MCMDEX, (0x40 | (command - 4))).Row(row - 1).Channel(chn).RetryPreviousRow()))
							{
								settings.channels[chn].tremoloType = command;
							}
						}
						effect3 = CMD_TREMOLO;
						effectParam3 = DMFvibrato2MPT(effectParam3, settings.internalTicks);
						useMem3 = true;
						break;
					case 7:  // Set Balance
						effect3 = CMD_PANNING8;
						break;
					case 8:  // Slide Balance Left
					case 9:  // Slide Balance Right
						effectParam3 = DMFslide2MPT(effectParam3, settings.internalTicks, (command == 8));
						effect3 = CMD_PANNINGSLIDE;
						useMem3 = true;
						break;
					case 10:  // Balance Vibrato Left/Right (always sine modulated)
						effect3 = CMD_PANBRELLO;
						effectParam3 = DMFvibrato2MPT(effectParam3, settings.internalTicks);
						useMem3 = true;
						break;
					default:
						effect3 = CMD_NONE;
						break;
					}
				}

				// Let's see if we can help the effect swapper by reducing some effect parameters to "continue" parameters.
				if(useMem2)
					ApplyEffectMemory(m, row, sndFile.GetNumChannels(), effect2, effectParam2);
				if(useMem3)
					ApplyEffectMemory(m, row, sndFile.GetNumChannels(), effect3, effectParam3);

				// I guess this is close enough to "not retriggering the note"
				if(slideNote && m->IsNote())
				{
					if(effect2 == CMD_NONE)
					{
						effect2 = CMD_TONEPORTAMENTO;
						effectParam2 = 0xFF;
					} else if(effect3 == CMD_NONE && effect2 != CMD_TONEPORTAMENTO)	// Tone portamentos normally go in effect #2
					{
						effect3 = CMD_TONEPORTAMENTO;
						effectParam3 = 0xFF;
					}
				}
				// If one of the effects is unused, temporarily put volume commands in there
				if(m->volcmd == VOLCMD_VOLUME)
				{
					if(effect2 == CMD_NONE)
					{
						effect2 = CMD_VOLUME;
						effectParam2 = m->vol;
						m->volcmd = VOLCMD_NONE;
					} else if(effect3 == CMD_NONE)
					{
						effect3 = CMD_VOLUME;
						effectParam3 = m->vol;
						m->volcmd = VOLCMD_NONE;
					}
				}

				ModCommand combinedCmd;
				combinedCmd.FillInTwoCommands(effect2, effectParam2, effect3, effectParam3);

				if(m->volcmd == VOLCMD_NONE && combinedCmd.volcmd != VOLCMD_NONE)
				{
					m->SetVolumeCommand(combinedCmd);
				}
				// Prefer instrument effects over any other effects
				if(effect1 != CMD_NONE)
				{
					combinedCmd.FillInTwoCommands(combinedCmd.command, combinedCmd.param, effect1, effectParam1);
					if(m->volcmd == VOLCMD_NONE && combinedCmd.volcmd != VOLCMD_NONE)
					{
						m->SetVolumeCommand(combinedCmd);
					}
					m->SetEffectCommand(combinedCmd);
				} else if(combinedCmd.command != CMD_NONE)
				{
					m->SetEffectCommand(combinedCmd);
				}

			} else
			{
				channelCounter[chn]--;
			}
		}  // End for all channels

		// Now we can try to write tempo information.
		if(tempoChange)
		{
			tempoChange = false;
			
			sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_TEMPO, static_cast<ModCommand::PARAM>(tempo)).Row(row).Channel(0).RetryNextRow());
			sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_SPEED, static_cast<ModCommand::PARAM>(speed)).Row(row).RetryNextRow());
		}
		// Try to put delay effects somewhere as well
		if(writeDelay & 0xF0)
		{
			sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_S3MCMDEX, 0xE0 | (writeDelay >> 4)).Row(row).AllowMultiple());
		}
		if(writeDelay & 0x0F)
		{
			const uint8 param = static_cast<uint8>((writeDelay & 0x0F) * settings.internalTicks / 15);
			sndFile.Patterns[pat].WriteEffect(EffectWriter(CMD_S3MCMDEX, 0x60u | Clamp(param, uint8(1), uint8(15))).Row(row).AllowMultiple());
		}
		writeDelay = 0;
	}  // End for all rows

	return pat;
}


static bool ValidateHeader(const DMFFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.signature, "DDMF", 4)
		|| !fileHeader.version || fileHeader.version > 10)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderDMF(MemoryFileReader file, const uint64 *pfilesize)
{
	DMFFileHeader fileHeader;
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


bool CSoundFile::ReadDMF(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	DMFFileHeader fileHeader;
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

	// Go through all chunks now... cannot use our standard IFF chunk reader here because early X-Tracker versions write some malformed chunk headers... fun code ahead!
	ChunkReader::ChunkList<DMFChunk> chunks;
	while(file.CanRead(sizeof(DMFChunk)))
	{
		DMFChunk chunkHeader;
		file.Read(chunkHeader);
		uint32 chunkLength = chunkHeader.length, chunkSkip = 0;
		// When loop start was added to version 3, the chunk size was not updated...
		if(fileHeader.version == 3 && chunkHeader.GetID() == DMFChunk::idSEQU)
			chunkSkip = 2;
		// ...and when the loop end was added to version 4, it was also note updated! Luckily they fixed it in version 5.
		else if(fileHeader.version == 4 && chunkHeader.GetID() == DMFChunk::idSEQU)
			chunkSkip = 4;
		// Earlier X-Tracker versions also write a garbage length for the SMPD chunk if samples are compressed.
		// I don't know when exactly this stopped, but I have no version 5-7 files to check (and no X-Tracker version that writes those versions).
		// Since this is practically always the last chunk in the file, the following code is safe for those versions, though.
		else if(fileHeader.version < 8 && chunkHeader.GetID() == DMFChunk::idSMPD)
			chunkLength = mpt::saturate_cast<uint32>(file.BytesLeft());
		chunks.chunks.push_back(ChunkReader::Item<DMFChunk>{chunkHeader, file.ReadChunk(chunkLength)});
		file.Skip(chunkSkip);
	}
	FileReader chunk;

	// Read pattern chunk first so that we know how many channels there are
	chunk = chunks.GetChunk(DMFChunk::idPATT);
	if(!chunk.IsValid())
		return false;

	DMFPatterns patHeader;
	chunk.ReadStruct(patHeader);
	// First, find out where all of our patterns are...
	std::vector<FileReader> patternChunks;
	if(loadFlags & loadPatternData)
	{
		patternChunks.resize(patHeader.numPatterns);
		const uint8 headerSize = fileHeader.version < 3 ? 9 : 8;
		for(auto &patternChunk : patternChunks)
		{
			chunk.Skip(headerSize - sizeof(uint32le));
			const uint32 patLength = chunk.ReadUint32LE();
			if(!chunk.CanRead(patLength))
				return false;
			chunk.SkipBack(headerSize);
			patternChunk = chunk.ReadChunk(headerSize + patLength);
		}
	}

	InitializeGlobals(MOD_TYPE_DMF, Clamp<uint8, uint8>(patHeader.numTracks, 1, 32) + 1);  // + 1 for global track (used for tempo stuff)

	m_modFormat.formatName = MPT_UFORMAT("Delusion Digital Music Format v{}")(fileHeader.version);
	m_modFormat.madeWithTracker = fileHeader.version == 10 ? UL_("X-Tracker 32") : UL_("X-Tracker");
	m_modFormat.type = UL_("dmf");
	m_modFormat.charset = mpt::Charset::CP437;

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.songname);
	m_songArtist = mpt::ToUnicode(mpt::Charset::CP437, mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.composer));

	FileHistory mptHistory;
	mptHistory.loadDate.day = Clamp(fileHeader.creationDay, uint8(1), uint8(31));
	mptHistory.loadDate.month = Clamp(fileHeader.creationMonth, uint8(1), uint8(12));
	mptHistory.loadDate.year = 1900 + fileHeader.creationYear;
	m_FileHistory.clear();
	m_FileHistory.push_back(mptHistory);

	// Read order list
	chunk = chunks.GetChunk(DMFChunk::idSEQU);
	ORDERINDEX seqLoopStart = 0, seqLoopEnd = ORDERINDEX_MAX;
	if(fileHeader.version >= 3)
		seqLoopStart = chunk.ReadUint16LE();
	if(fileHeader.version >= 4)
		seqLoopEnd = chunk.ReadUint16LE();
	// HIPOMATK.DMF has a loop end of 0, other v4 files have proper loop ends. Later X-Tracker versions import it as-is but it cannot be intentional.
	// We just assume that this feature might have been buggy in early v4 versions and ignore the loop end in that case.
	if(fileHeader.version == 4 && seqLoopEnd == 0)
		seqLoopEnd = ORDERINDEX_MAX;
	ReadOrderFromFile<uint16le>(Order(), chunk, chunk.BytesLeft() / 2);
	LimitMax(seqLoopStart, Order().GetLastIndex());
	LimitMax(seqLoopEnd, Order().GetLastIndex());

	if(loadFlags & loadPatternData)
	{
		// Now go through the order list and load them.
		DMFPatternSettings settings(GetNumChannels());

		Patterns.ResizeArray(Order().GetLength());
		for(PATTERNINDEX &pat : Order())
		{
			// Create one pattern for each order item, as the same pattern can be played with different settings
			if(pat < patternChunks.size())
			{
				pat = ConvertDMFPattern(patternChunks[pat], fileHeader.version, settings, *this);
			}
		}
		// Write loop end if necessary
		if(Order().IsValidPat(seqLoopEnd) && (seqLoopStart > 0 || seqLoopEnd < Order().GetLastIndex()))
		{
			PATTERNINDEX pat = Order()[seqLoopEnd];
			Patterns[pat].WriteEffect(EffectWriter(CMD_POSITIONJUMP, static_cast<ModCommand::PARAM>(seqLoopStart)).Row(Patterns[pat].GetNumRows() - 1).RetryPreviousRow());
		}
	}

	// Read song message
	chunk = chunks.GetChunk(DMFChunk::idCMSG);
	if(chunk.IsValid())
	{
		// The song message seems to start at a 1 byte offset.
		// The skipped byte seems to always be 0.
		// This also matches how XT 1.03 itself displays the song message.
		chunk.Skip(1);
		m_songMessage.ReadFixedLineLength(chunk, chunk.BytesLeft(), 40, 0);
	}
	
	// Read sample headers + data
	FileReader sampleDataChunk = chunks.GetChunk(DMFChunk::idSMPD);
	chunk = chunks.GetChunk(DMFChunk::idSMPI);
	m_nSamples = chunk.ReadUint8();

	for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
	{
		const uint8 nameLength = (fileHeader.version < 2) ? 30 : chunk.ReadUint8();
		chunk.ReadString<mpt::String::spacePadded>(m_szNames[smp], nameLength);
		DMFSampleHeader sampleHeader;
		ModSample &sample = Samples[smp];
		chunk.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(sample);

		// Read library name in version 8 files
		if(fileHeader.version >= 8)
			chunk.ReadString<mpt::String::spacePadded>(sample.filename, 8);

		// Filler + CRC
		chunk.Skip(fileHeader.version > 1 ? 6 : 2);

		// Now read the sample data from the data chunk
		FileReader sampleData = sampleDataChunk.ReadChunk(sampleDataChunk.ReadUint32LE());
		if(sampleData.IsValid() && (loadFlags & loadSampleData))
		{
			SampleIO(
				sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				(sampleHeader.flags & DMFSampleHeader::smpCompMask) == DMFSampleHeader::smpComp1 ? SampleIO::DMF : SampleIO::signedPCM)
				.ReadSample(sample, sampleData);
		}
	}

	m_SongFlags = SONG_LINEARSLIDES | SONG_ITCOMPATGXX;  // this will be converted to IT format by MPT. SONG_ITOLDEFFECTS is not set because of tremor and vibrato.
	Order().SetDefaultSpeed(6);
	Order().SetDefaultTempoInt(120);
	m_nDefaultGlobalVolume = 256;
	m_nSamplePreAmp = m_nVSTiVolume = 48;
	m_playBehaviour.set(kApplyOffsetWithoutNote);

	return true;
}


///////////////////////////////////////////////////////////////////////
// DMF Compression

struct DMFHNode
{
	int16 left, right;
	uint8 value;
};

struct DMFHTree
{
	BitReader file;
	int lastnode = 0, nodecount = 0;
	DMFHNode nodes[256]{};

	DMFHTree(FileReader &file)
	    : file(file)
	{
	}
	
	//
	// tree: [8-bit value][12-bit index][12-bit index] = 32-bit
	//

	void DMFNewNode()
	{
		int actnode = nodecount;
		if(actnode > 255) return;
		nodes[actnode].value = static_cast<uint8>(file.ReadBits(7));
		bool isLeft = file.ReadBits(1) != 0;
		bool isRight = file.ReadBits(1) != 0;
		actnode = lastnode;
		if(actnode > 255) return;
		nodecount++;
		lastnode = nodecount;
		if(isLeft)
		{
			nodes[actnode].left = (int16)lastnode;
			DMFNewNode();
		} else
		{
			nodes[actnode].left = -1;
		}
		lastnode = nodecount;
		if(isRight)
		{
			nodes[actnode].right = (int16)lastnode;
			DMFNewNode();
		} else
		{
			nodes[actnode].right = -1;
		}
	}
};


uintptr_t DMFUnpack(FileReader &file, uint8 *psample, uint32 maxlen)
{
	DMFHTree tree(file);
	uint8 value = 0, delta = 0;

	try
	{
		tree.DMFNewNode();
		if(tree.nodes[0].left < 0 || tree.nodes[0].right < 0)
			return tree.file.GetPosition();
		for(uint32 i = 0; i < maxlen; i++)
		{
			int actnode = 0;
			bool sign = tree.file.ReadBits(1) != 0;
			do
			{
				if(tree.file.ReadBits(1))
					actnode = tree.nodes[actnode].right;
				else
					actnode = tree.nodes[actnode].left;
				if(actnode > 255) break;
				delta = tree.nodes[actnode].value;
			} while((tree.nodes[actnode].left >= 0) && (tree.nodes[actnode].right >= 0));
			if(sign) delta ^= 0xFF;
			value += delta;
			psample[i] = value;
		}
	} catch(const BitReader::eof &)
	{
		//AddToLog(LogWarning, "Truncated DMF sample block");
	}
	return tree.file.GetPosition();
}


OPENMPT_NAMESPACE_END
