/*
 * Load_mod.cpp
 * ------------
 * Purpose: MOD / NST (ProTracker / NoiseTracker / Startrekker) module loader / saver
 * Notes  : "1100 LOC for processing MOD files?!" you say?
 *          Well, extensive heuristics for more or less broken MOD files and files saved with tons of different trackers, to allow for the most optimal playback,
 *          do take up some space... and then there's also Startrekker synthesized instruments support, of course. It all adds up.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "MODTools.h"
#include "Tables.h"
#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "../common/mptFileIO.h"
#endif
#ifdef MPT_EXTERNAL_SAMPLES
// For loading external data in Startrekker files
#include "mpt/fs/fs.hpp"
#include "mpt/io_file/inputfile.hpp"
#include "mpt/io_file_read/inputfile_filecursor.hpp"
#include "../common/mptPathString.h"
#endif  // MPT_EXTERNAL_SAMPLES

OPENMPT_NAMESPACE_BEGIN

// Synthesized StarTrekker instruments
struct AMInstrument
{
	char     am[2];        // "AM"
	char     zero[4];
	uint16be startLevel;   // Start level
	uint16be attack1Level; // Attack 1 level
	uint16be attack1Speed; // Attack 1 speed
	uint16be attack2Level; // Attack 2 level
	uint16be attack2Speed; // Attack 2 speed
	uint16be sustainLevel; // Sustain level
	uint16be decaySpeed;   // Decay speed
	uint16be sustainTime;  // Sustain time
	uint16be nt;           // ?
	uint16be releaseSpeed; // Release speed
	uint16be waveform;     // Waveform
	int16be  pitchFall;    // Pitch fall
	uint16be vibAmp;       // Vibrato amplitude
	uint16be vibSpeed;     // Vibrato speed
	uint16be octave;       // Base frequency

	void ConvertToMPT(ModSample &sample, ModInstrument &ins, mpt::fast_prng &rng) const
	{
		sample.nLength = waveform == 3 ? 1024 : 32;
		sample.nLoopStart = 0;
		sample.nLoopEnd = sample.nLength;
		sample.uFlags.set(CHN_LOOP);
		sample.nVolume = 256;  // prelude.mod has volume 0 in sample header
		sample.nVibDepth = mpt::saturate_cast<uint8>(vibAmp * 2);
		sample.nVibRate = static_cast<uint8>(vibSpeed);
		sample.nVibType = VIB_SINE;
		sample.RelativeTone = static_cast<int8>(-12 * octave);
		if(sample.AllocateSample())
		{
			int8 *p = sample.sample8();
			for(SmpLength i = 0; i < sample.nLength; i++)
			{
				switch(waveform)
				{
				default:
				case 0: p[i] = ModSinusTable[i * 2];            break; // Sine
				case 1: p[i] = static_cast<int8>(-128 + i * 8); break; // Saw
				case 2: p[i] = i < 16 ? -128 : 127;             break; // Square
				case 3: p[i] = mpt::random<int8>(rng);          break; // Noise
				}
			}
		}

		InstrumentEnvelope &volEnv = ins.VolEnv;
		volEnv.dwFlags.set(ENV_ENABLED);
		volEnv.reserve(6);
		volEnv.push_back(0, static_cast<EnvelopeNode::value_t>(startLevel / 4));

		const struct
		{
			uint16 level, speed;
		} points[] = {{startLevel, 0}, {attack1Level, attack1Speed}, {attack2Level, attack2Speed}, {sustainLevel, decaySpeed}, {sustainLevel, sustainTime}, {0, releaseSpeed}};

		for(uint8 i = 1; i < std::size(points); i++)
		{
			int duration = std::min(points[i].speed, uint16(256));
			// Sustain time is already in ticks, no need to compute the segment duration.
			if(i != 4)
			{
				if(duration == 0)
				{
					volEnv.dwFlags.set(ENV_LOOP);
					volEnv.nLoopStart = volEnv.nLoopEnd = static_cast<uint8>(volEnv.size() - 1);
					break;
				}

				// Startrekker increments / decrements the envelope level by the stage speed
				// until it reaches the next stage level.
				int a, b;
				if(points[i].level > points[i - 1].level)
				{
					a = points[i].level - points[i - 1].level;
					b = 256 - points[i - 1].level;
				} else
				{
					a = points[i - 1].level - points[i].level;
					b = points[i - 1].level;
				}
				// Release time is again special.
				if(i == 5)
					b = 256;
				else if(b == 0)
					b = 1;
				duration = std::max((256 * a) / (duration * b), 1);
			}
			if(duration > 0)
			{
				volEnv.push_back(volEnv.back().tick + static_cast<EnvelopeNode::tick_t>(duration), static_cast<EnvelopeNode::value_t>(points[i].level / 4));
			}
		}

		if(pitchFall)
		{
			InstrumentEnvelope &pitchEnv = ins.PitchEnv;
			pitchEnv.dwFlags.set(ENV_ENABLED);
			pitchEnv.reserve(2);
			pitchEnv.push_back(0, ENVELOPE_MID);
			// cppcheck false-positive
			// cppcheck-suppress zerodiv
			pitchEnv.push_back(static_cast<EnvelopeNode::tick_t>(1024 / abs(pitchFall)), pitchFall > 0 ? ENVELOPE_MIN : ENVELOPE_MAX);
		}
	}
};

MPT_BINARY_STRUCT(AMInstrument, 36)


struct MODMagicResult
{
	const mpt::uchar *madeWithTracker = nullptr;
	uint32 invalidByteThreshold = MODSampleHeader::INVALID_BYTE_THRESHOLD;
	uint16 patternDataOffset    = 1084;
	CHANNELINDEX numChannels    = 0;
	bool isNoiseTracker         = false;
	bool isStartrekker          = false;
	bool isGenericMultiChannel  = false;
	bool setMODVBlankTiming     = false;
	bool swapBytes              = false;
};


static bool CheckMODMagic(const char magic[4], MODMagicResult &result)
{
	if(IsMagic(magic, "M.K.")      // ProTracker and compatible
	   || IsMagic(magic, "M!K!")   // ProTracker (>64 patterns)
	   || IsMagic(magic, "PATT")   // ProTracker 3.6
	   || IsMagic(magic, "NSMS")   // kingdomofpleasure.mod by bee hunter
	   || IsMagic(magic, "LARD"))  // judgement_day_gvine.mod by 4-mat
	{
		result.madeWithTracker = UL_("Generic ProTracker or compatible");
		result.numChannels = 4;
	} else if(IsMagic(magic, "M&K!")     // "His Master's Noise" musicdisk
	          || IsMagic(magic, "FEST")  // "His Master's Noise" musicdisk
	          || IsMagic(magic, "N.T."))
	{
		result.madeWithTracker = IsMagic(magic, "N.T.") ? UL_("NoiseTracker") : UL_("His Master's NoiseTracker");
		result.isNoiseTracker = true;
		result.setMODVBlankTiming = true;
		result.numChannels = 4;
	} else if(IsMagic(magic, "OKTA")
	          || IsMagic(magic, "OCTA"))
	{
		// Oktalyzer
		result.madeWithTracker = UL_("Oktalyzer");
		result.numChannels = 8;
	} else if(IsMagic(magic, "CD81")
	          || IsMagic(magic, "CD61"))
	{
		// Octalyser on Atari STe/Falcon
		result.madeWithTracker = UL_("Octalyser (Atari)");
		result.numChannels = static_cast<CHANNELINDEX>(magic[2] - '0');
	} else if(IsMagic(magic, "M\0\0\0") || IsMagic(magic, "8\0\0\0"))
	{
		// Inconexia demo by Iguana, delta samples (https://www.pouet.net/prod.php?which=830)
		result.madeWithTracker = UL_("Inconexia demo (delta samples)");
		result.invalidByteThreshold = MODSampleHeader::INVALID_BYTE_FRAGILE_THRESHOLD;
		result.numChannels = (magic[0] == '8') ? 8 : 4;
	} else if(!memcmp(magic, "FA0", 3) && magic[3] >= '4' && magic[3] <= '8')
	{
		// Digital Tracker on Atari Falcon
		result.madeWithTracker = UL_("Digital Tracker");
		result.numChannels = static_cast<CHANNELINDEX>(magic[3] - '0');
		// Digital Tracker MODs contain four bytes (00 40 00 00) right after the magic bytes which don't seem to do anything special.
		result.patternDataOffset = 1088;
	} else if((!memcmp(magic, "FLT", 3) || !memcmp(magic, "EXO", 3)) && (magic[3] == '4' || magic[3] == '8'))
	{
		// FLTx / EXOx - Startrekker by Exolon / Fairlight
		result.madeWithTracker = UL_("Startrekker");
		result.isStartrekker = true;
		result.setMODVBlankTiming = true;
		result.numChannels = static_cast<CHANNELINDEX>(magic[3] - '0');
	} else if(magic[0] >= '1' && magic[0] <= '9' && !memcmp(magic + 1, "CHN", 3))
	{
		// xCHN - Many trackers
		result.madeWithTracker = UL_("Generic MOD-compatible Tracker");
		result.isGenericMultiChannel = true;
		result.numChannels = static_cast<CHANNELINDEX>(magic[0] - '0');
	} else if(magic[0] >= '1' && magic[0] <= '9' && magic[1] >= '0' && magic[1] <= '9'
	          && (!memcmp(magic + 2, "CH", 2) || !memcmp(magic + 2, "CN", 2)))
	{
		// xxCN / xxCH - Many trackers
		result.madeWithTracker = UL_("Generic MOD-compatible Tracker");
		result.isGenericMultiChannel = true;
		result.numChannels = static_cast<CHANNELINDEX>((magic[0] - '0') * 10 + magic[1] - '0');
	} else if(!memcmp(magic, "TDZ", 3) && magic[3] >= '1' && magic[3] <= '9')
	{
		// TDZx - TakeTracker (only TDZ1-TDZ3 should exist, but historically this code only supported 4-9 channels, so we keep those for the unlikely case that they were actually used for something)
		result.madeWithTracker = UL_("TakeTracker");
		result.numChannels = static_cast<CHANNELINDEX>(magic[3] - '0');
	} else if(IsMagic(magic, ".M.K"))
	{
		// Hacked .DMF files from the game "Apocalypse Abyss"
		result.numChannels = 4;
		result.swapBytes = true;
	} else if(IsMagic(magic, "WARD"))
	{
		// MUSIC*.DTA files from the DOS game Aleshar - The World Of Ice
		result.madeWithTracker = UL_("Generic MOD-compatible Tracker");
		result.isGenericMultiChannel = true;
		result.numChannels = 8;
	} else
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMOD(MemoryFileReader file, const uint64 *pfilesize)
{
	if(!file.LengthIsAtLeast(1080 + 4))
	{
		return ProbeWantMoreData;
	}
	file.Seek(1080);
	char magic[4];
	file.ReadArray(magic);
	MODMagicResult modMagicResult;
	if(!CheckMODMagic(magic, modMagicResult))
	{
		return ProbeFailure;
	}

	file.Seek(20);
	uint32 invalidBytes = 0;
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		MODSampleHeader sampleHeader = ReadAndSwap<MODSampleHeader>(file, modMagicResult.swapBytes);
		invalidBytes += sampleHeader.GetInvalidByteScore();
	}
	if(invalidBytes > modMagicResult.invalidByteThreshold)
	{
		return ProbeFailure;
	}

	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadMOD(FileReader &file, ModLoadingFlags loadFlags)
{
	char magic[4];
	if(!file.Seek(1080) || !file.ReadArray(magic))
	{
		return false;
	}

	MODMagicResult modMagicResult;
	if(!CheckMODMagic(magic, modMagicResult)
	   || modMagicResult.numChannels < 1
	   || modMagicResult.numChannels > MAX_BASECHANNELS)
	{
		return false;
	}

	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_MOD, modMagicResult.numChannels);

	bool isNoiseTracker = modMagicResult.isNoiseTracker;
	bool isStartrekker = modMagicResult.isStartrekker;
	bool isGenericMultiChannel = modMagicResult.isGenericMultiChannel;
	bool isInconexia = IsMagic(magic, "M\0\0\0") || IsMagic(magic, "8\0\0\0");
	// A loop length of zero will freeze ProTracker, so assume that modules having such a value were not meant to be played on Amiga. Fixes LHS_MI.MOD
	bool hasRepLen0 = false;
	// Empty sample slots typically should have a default volume of 0 in ProTracker
	bool hasEmptySampleWithVolume = false;
	if(modMagicResult.setMODVBlankTiming)
	{
		m_playBehaviour.set(kMODVBlankTiming);
	}

	// Startrekker 8 channel mod (needs special treatment, see below)
	const bool isFLT8 = isStartrekker && GetNumChannels() == 8;
	const bool isMdKd = IsMagic(magic, "M.K.");
	// Adjust finetune values for modules saved with "His Master's Noisetracker"
	const bool isHMNT = IsMagic(magic, "M&K!") || IsMagic(magic, "FEST");
	bool maybeWOW = isMdKd;

	// Reading song title
	file.Seek(0);
	const auto songTitle = ReadAndSwap<std::array<char, 20>>(file, modMagicResult.swapBytes);
	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, songTitle);

	// Load Sample Headers
	SmpLength totalSampleLen = 0, wowSampleLen = 0;
	m_nSamples = 31;
	uint32 invalidBytes = 0;
	bool hasLongSamples = false;
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		MODSampleHeader sampleHeader = ReadAndSwap<MODSampleHeader>(file, modMagicResult.swapBytes);
		invalidBytes += ReadMODSample(sampleHeader, Samples[smp], m_szNames[smp], GetNumChannels() == 4);
		totalSampleLen += Samples[smp].nLength;

		if(isHMNT)
			Samples[smp].nFineTune = -static_cast<int8>(sampleHeader.finetune << 3);
		else if(Samples[smp].nLength > 65535)
			hasLongSamples = true;
		
		if(sampleHeader.length && !sampleHeader.loopLength)
			hasRepLen0 = true;
		else if(!sampleHeader.length && sampleHeader.volume == 64)
			hasEmptySampleWithVolume = true;

		if(maybeWOW)
		{
			// Some WOW files rely on sample length 1 being counted as well
			wowSampleLen += sampleHeader.length * 2;
			// WOW files are converted 669 files, which don't support finetune or default volume
			if(sampleHeader.finetune)
				maybeWOW = false;
			else if(sampleHeader.length > 0 && sampleHeader.volume != 64)
				maybeWOW = false;
		}
	}
	// If there is too much binary garbage in the sample headers, reject the file.
	if(invalidBytes > modMagicResult.invalidByteThreshold)
	{
		return false;
	}

	// Read order information
	const MODFileHeader fileHeader = ReadAndSwap<MODFileHeader>(file, modMagicResult.swapBytes);

	file.Seek(modMagicResult.patternDataOffset);

	if(fileHeader.restartPos > 0)
		maybeWOW = false;
	if(!maybeWOW)
		wowSampleLen = 0;

	ReadOrderFromArray(Order(), fileHeader.orderList);

	ORDERINDEX realOrders = fileHeader.numOrders;
	if(realOrders > 128)
	{
		// beatwave.mod by Sidewinder claims to have 129 orders. (MD5: 8a029ac498d453beb929db9a73c3c6b4, SHA1: f7b76fb9f477b07a2e78eb10d8624f0df262cde7 - the version from ModArchive, not ModLand)
		realOrders = 128;
	} else if(realOrders == 0)
	{
		// Is this necessary?
		realOrders = 128;
		while(realOrders > 1 && Order()[realOrders - 1] == 0)
		{
			realOrders--;
		}
	}

	// Get number of patterns (including some order list sanity checks)
	PATTERNINDEX numPatterns = GetNumPatterns(file, *this, realOrders, totalSampleLen, wowSampleLen, false);
	if(maybeWOW && GetNumChannels() == 8)
	{
		// M.K. with 8 channels = Mod's Grave
		modMagicResult.madeWithTracker = UL_("Mod's Grave");
		isGenericMultiChannel = true;
	}

	if(isFLT8)
	{
		// FLT8 has only even order items, so divide by two.
		for(auto &pat : Order())
		{
			pat /= 2u;
		}
	}

	// Restart position sanity checks
	realOrders--;
	Order().SetRestartPos(fileHeader.restartPos);

	// (Ultimate) Soundtracker didn't have a restart position, but instead stored a default tempo in this value.
	// The default value for this is 0x78 (120 BPM). This is probably the reason why some M.K. modules
	// have this weird restart position. I think I've read somewhere that NoiseTracker actually writes 0x78 there.
	// M.K. files that have restart pos == 0x78: action's batman by DJ Uno, VALLEY.MOD, WormsTDC.MOD, ZWARTZ.MOD
	// Files that have an order list longer than 0x78 with restart pos = 0x78: my_shoe_is_barking.mod, papermix.mod
	// - in both cases it does not appear like the restart position should be used.
	MPT_ASSERT(fileHeader.restartPos != 0x78 || fileHeader.restartPos + 1u >= realOrders);
	if(fileHeader.restartPos > realOrders || (fileHeader.restartPos == 0x78 && GetNumChannels() == 4))
	{
		Order().SetRestartPos(0);
	}

	Order().SetDefaultSpeed(6);
	Order().SetDefaultTempoInt(125);
	m_nMinPeriod = 14 * 4;
	m_nMaxPeriod = 3424 * 4;
	// Prevent clipping based on number of channels... If all channels are playing at full volume, "256 / #channels"
	// is the maximum possible sample pre-amp without getting distortion (Compatible mix levels given).
	// The more channels we have, the less likely it is that all of them are used at the same time, though, so cap at 32...
	m_nSamplePreAmp = Clamp(256 / GetNumChannels(), 32, 128);
	m_SongFlags = SONG_FORMAT_NO_VOLCOL;  // SONG_ISAMIGA will be set conditionally

	// Setup channel pan positions and volume
	SetupMODPanning();

	// Before loading patterns, apply some heuristics:
	// - Scan patterns to check if file could be a NoiseTracker file in disguise.
	//   In this case, the parameter of Dxx commands needs to be ignored (see 1.11song2.mod, 2-3song6.mod).
	// - Use the same code to find notes that would be out-of-range on Amiga.
	// - Detect 7-bit panning and whether 8xx / E8x commands should be interpreted as panning at all.
	bool onlyAmigaNotes = true;
	bool fix7BitPanning = false;
	uint8 maxPanning = 0;  // For detecting 8xx-as-sync
	const uint8 ENABLE_MOD_PANNING_THRESHOLD = 0x30;
	if(!isNoiseTracker)
	{
		const uint32 patternLength = GetNumChannels() * 64;
		bool leftPanning = false, extendedPanning = false;  // For detecting 800-880 panning
		isNoiseTracker = isMdKd && !hasEmptySampleWithVolume && !hasLongSamples;
		for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
		{
			uint16 patternBreaks = 0;
			for(uint32 i = 0; i < patternLength; i++)
			{
				ModCommand m;
				const auto data = ReadAndSwap<std::array<uint8, 4>>(file, modMagicResult.swapBytes && pat == 0);
				const auto [command, param] = ReadMODPatternEntry(data, m);
				if(!m.IsAmigaNote())
				{
					isNoiseTracker = onlyAmigaNotes = false;
				}
				if((command > 0x06 && command < 0x0A)
					|| (command == 0x0E && param > 0x01)
					|| (command == 0x0F && param > 0x1F)
					|| (command == 0x0D && ++patternBreaks > 1))
				{
					isNoiseTracker = false;
				}
				if(command == 0x08)
				{
					// Note: commands 880...88F are not considered for determining the panning style, as some modules use 7-bit panning but slightly overshoot:
					// LOOKATME.MOD (MD5: dedcec1a2a135aeb1a311841cea2c60c, SHA1: 42bf92bf824ef9fb904704b8ee7e3a30df60038d) has an 88A command as its rightmost panning.
					maxPanning = std::max(maxPanning, param);
					if(param < 0x80)
						leftPanning = true;
					else if(param > 0x8F && param != 0xA4)
						extendedPanning = true;
				} else if(command == 0x0E && (param & 0xF0) == 0x80)
				{
					maxPanning = std::max(maxPanning, static_cast<uint8>((param & 0x0F) << 4));
				}
			}
		}
		fix7BitPanning = leftPanning && !extendedPanning && maxPanning >= ENABLE_MOD_PANNING_THRESHOLD;
	}
	file.Seek(modMagicResult.patternDataOffset);

	const CHANNELINDEX readChannels = (isFLT8 ? 4 : GetNumChannels());  // 4 channels per pattern in FLT8 format.
	if(isFLT8)
		numPatterns++;                                              // as one logical pattern consists of two real patterns in FLT8 format, the highest pattern number has to be increased by one.
	bool hasTempoCommands = false, definitelyCIA = hasLongSamples;  // for detecting VBlank MODs
	// Heuristic for rejecting E0x commands that are most likely not intended to actually toggle the Amiga LED filter, like in naen_leijasi_ptk.mod by ilmarque
	bool filterState = false;
	int filterTransitions = 0;

	// Reading patterns
	Patterns.ResizeArray(numPatterns);
	std::bitset<32> referencedSamples;
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		ModCommand *rowBase = nullptr;

		if(isFLT8)
		{
			// FLT8: Only create "even" patterns and either write to channel 1 to 4 (even patterns) or 5 to 8 (odd patterns).
			PATTERNINDEX actualPattern = pat / 2u;
			if((pat % 2u) == 0 && !Patterns.Insert(actualPattern, 64))
			{
				break;
			}
			rowBase = Patterns[actualPattern].GetpModCommand(0, (pat % 2u) == 0 ? 0 : 4);
		} else
		{
			if(!Patterns.Insert(pat, 64))
			{
				break;
			}
			rowBase = Patterns[pat].GetpModCommand(0, 0);
		}

		if(rowBase == nullptr || !(loadFlags & loadPatternData))
		{
			break;
		}

		// For detecting PT1x mode
		std::vector<ModCommand::INSTR> lastInstrument(GetNumChannels(), 0);
		std::vector<uint8> instrWithoutNoteCount(GetNumChannels(), 0);

		for(ROWINDEX row = 0; row < 64; row++, rowBase += GetNumChannels())
		{
			// If we have more than one Fxx command on this row and one can be interpreted as speed
			// and the other as tempo, we can be rather sure that it is not a VBlank mod.
			bool hasSpeedOnRow = false, hasTempoOnRow = false;

			for(CHANNELINDEX chn = 0; chn < readChannels; chn++)
			{
				ModCommand &m = rowBase[chn];
				const auto data = ReadAndSwap<std::array<uint8, 4>>(file, modMagicResult.swapBytes && pat == 0);
				auto [command, param] = ReadMODPatternEntry(data, m);

				if(command || param)
				{
					if(isStartrekker && command == 0x0E)
					{
						// No support for Startrekker assembly macros
						command = param = 0;
					} else if(isStartrekker && command == 0x0F && param > 0x1F)
					{
						// Startrekker caps speed at 31 ticks per row
						param = 0x1F;
					}
					ConvertModCommand(m, command, param);
				}

				// Perform some checks for our heuristics...
				if(m.command == CMD_TEMPO)
				{
					hasTempoOnRow = true;
					if(m.param < 100)
						hasTempoCommands = true;
				} else if(m.command == CMD_SPEED)
				{
					hasSpeedOnRow = true;
				} else if(m.command == CMD_PATTERNBREAK && isNoiseTracker)
				{
					m.param = 0;
				} else if(m.command == CMD_TREMOLO && isHMNT)
				{
					m.command = CMD_HMN_MEGA_ARP;
				} else if(m.command == CMD_PANNING8 && fix7BitPanning)
				{
					// Fix MODs with 7-bit + surround panning
					if(m.param == 0xA4)
					{
						m.command = CMD_S3MCMDEX;
						m.param = 0x91;
					} else
					{
						m.param = mpt::saturate_cast<ModCommand::PARAM>(m.param * 2);
					}
				} else if(m.command == CMD_MODCMDEX && m.param < 0x10)
				{
					// Count LED filter transitions
					bool newState = !(m.param & 0x01);
					if(newState != filterState)
					{
						filterState = newState;
						filterTransitions++;
					}
				}
				if(m.note == NOTE_NONE && m.instr > 0 && !isFLT8)
				{
					if(lastInstrument[chn] > 0 && lastInstrument[chn] != m.instr)
					{
						// Arbitrary threshold for enabling sample swapping: 4 consecutive "sample swaps" in one pattern.
						if(++instrWithoutNoteCount[chn] >= 4)
						{
							m_playBehaviour.set(kMODSampleSwap);
						}
					}
				} else if(m.note != NOTE_NONE)
				{
					instrWithoutNoteCount[chn] = 0;
				}
				if(m.instr != 0)
				{
					lastInstrument[chn] = m.instr;
					if(isStartrekker)
						referencedSamples.set(m.instr & 0x1F);
				}
			}
			if(hasSpeedOnRow && hasTempoOnRow)
				definitelyCIA = true;
		}
	}

	if(onlyAmigaNotes && !hasRepLen0 && (IsMagic(magic, "M.K.") || IsMagic(magic, "M!K!") || IsMagic(magic, "PATT")))
	{
		// M.K. files that don't exceed the Amiga note limit (fixes mod.mothergoose)
		m_SongFlags.set(SONG_AMIGALIMITS);
		// Need this for professionaltracker.mod by h0ffman (SHA1: 9a7c52cbad73ed2a198ee3fa18d3704ea9f546ff)
		m_SongFlags.set(SONG_PT_MODE);
		m_playBehaviour.set(kMODSampleSwap);
		m_playBehaviour.set(kMODOutOfRangeNoteDelay);
		m_playBehaviour.set(kMODTempoOnSecondTick);
		// Arbitrary threshold for deciding that 8xx effects are only used as sync markers
		if(maxPanning < ENABLE_MOD_PANNING_THRESHOLD)
		{
			m_playBehaviour.set(kMODIgnorePanning);
			if(fileHeader.restartPos != 0x7F)
			{
				// Don't enable these hacks for ScreamTracker modules (restart position = 0x7F), to fix e.g. sample 10 in BASIC001.MOD (SHA1: 11298a5620e677beaa50bd4ed00c3710b75c81af)
				// Note: restart position = 0x7F can also be found in ProTracker modules, e.g. professionaltracker.mod by h0ffman
				m_playBehaviour.set(kMODOneShotLoops);
			}
		}
	} else if(!onlyAmigaNotes && fileHeader.restartPos == 0x7F && isMdKd && fileHeader.restartPos + 1u >= realOrders)
	{
		modMagicResult.madeWithTracker = UL_("Scream Tracker");
	}

	if(onlyAmigaNotes && !isGenericMultiChannel && filterTransitions < 7)
	{
		m_SongFlags.set(SONG_ISAMIGA);
	}
	if(isGenericMultiChannel || isMdKd || IsMagic(magic, "M!K!"))
	{
		m_playBehaviour.set(kFT2MODTremoloRampWaveform);
	}
	if(isInconexia)
	{
		m_playBehaviour.set(kMODIgnorePanning);
	}

	// Reading samples
	bool anyADPCM = false;
	if(loadFlags & loadSampleData)
	{
		file.Seek(modMagicResult.patternDataOffset + (readChannels * 64 * 4) * numPatterns);
		for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
		{
			ModSample &sample = Samples[smp];
			if(sample.nLength)
			{
				SampleIO::Encoding encoding = SampleIO::signedPCM;
				if(isInconexia)
				{
					encoding = SampleIO::deltaPCM;
				} else if(file.ReadMagic("ADPCM"))
				{
					encoding = SampleIO::ADPCM;
					anyADPCM = true;
				}

				SampleIO sampleIO(
					SampleIO::_8bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					encoding);

				// Fix sample 6 in MOD.shorttune2, which has a replen longer than the sample itself.
				// ProTracker reads beyond the end of the sample when playing. Normally samples are
				// adjacent in PT's memory, so we simply read into the next sample in the file.
				// On the other hand, the loop points in Purple Motions's SOUL-O-M.MOD are completely broken and shouldn't be treated like this.
				// As it was most likely written in Scream Tracker, it has empty sample slots with a default volume of 64, which we use for
				// rejecting this quirk for that file.
				FileReader::pos_type nextSample = file.GetPosition() + sampleIO.CalculateEncodedSize(sample.nLength);
				if(isMdKd && onlyAmigaNotes && !hasEmptySampleWithVolume)
					sample.nLength = std::max(sample.nLength, sample.nLoopEnd);

				sampleIO.ReadSample(sample, file);
				file.Seek(nextSample);
			}
		}
		// XOR with 0xDF gives the message "TakeTrackered with version 0.9E!!!!!"
		if(GetNumChannels() <= 16 && file.ReadMagic("\x8B\xBE\xB4\xBA\x8B\xAD\xBE\xBC\xB4\xBA\xAD\xBA\xBB\xFF\xA8\xB6\xAB\xB7\xFF\xA9\xBA\xAD\xAC\xB6\xB0\xB1\xFF\xEF\xF1\xE6\xBA\xFE\xFE\xFE\xFE\xFE"))
			modMagicResult.madeWithTracker = UL_("TakeTracker");
		else if(isMdKd && file.ReadArray<char, 6>() == std::array<char, 6>{0x00, 0x11, 0x55, 0x33, 0x22, 0x11} && file.CanRead(3))  // 3 more bytes that differ between modules and Tetramed version, purpose unknown
			modMagicResult.madeWithTracker = UL_("Tetramed");
	}

#if defined(MPT_EXTERNAL_SAMPLES) || defined(MPT_BUILD_FUZZER)
	// Detect Startrekker files with external synth instruments.
	// Note: Synthesized AM samples may overwrite existing samples (e.g. sample 1 in fa.worse face.mod),
	// hence they are loaded here after all regular samples have been loaded.
	if((loadFlags & loadSampleData) && isStartrekker)
	{
#ifdef MPT_EXTERNAL_SAMPLES
		std::optional<mpt::IO::InputFile> amFile;
		FileReader amData;
		if(file.GetOptionalFileName())
		{
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
			mpt::PathString filename = *(file.GetOptionalFileName());
#else
			mpt::PathString filename = file.GetOptionalFileName().value();
#endif
			// Find instrument definition file
			const mpt::PathString exts[] = {P_(".nt"), P_(".NT"), P_(".as"), P_(".AS")};
			for(const auto &ext : exts)
			{
				mpt::PathString infoName = filename + ext;
				char stMagic[16];
				if(mpt::native_fs{}.is_file(infoName))
				{
					amFile.emplace(infoName, SettingCacheCompleteFileBeforeLoading());
					if(amFile->IsValid() && (amData = GetFileReader(*amFile)).IsValid() && amData.ReadArray(stMagic))
					{
						if(!memcmp(stMagic, "ST1.2 ModuleINFO", 16))
							modMagicResult.madeWithTracker = UL_("Startrekker 1.2");
						else if(!memcmp(stMagic, "ST1.3 ModuleINFO", 16))
							modMagicResult.madeWithTracker = UL_("Startrekker 1.3");
						else if(!memcmp(stMagic, "AudioSculpture10", 16))
							modMagicResult.madeWithTracker = UL_("AudioSculpture 1.0");
						else
							continue;

						if(amData.Seek(144))
						{
							// Looks like a valid instrument definition file!
							m_nInstruments = 31;
							break;
						}
					}
				}
			}
		}
#elif defined(MPT_BUILD_FUZZER)
		// For fuzzing this part of the code, just take random data from patterns
		FileReader amData = file.GetChunkAt(1084, 31 * 120);
		m_nInstruments = 31;
#endif

		mpt::deterministic_random_device rd;
		auto prng = mpt::make_prng<mpt::deterministic_fast_engine>(rd);
		for(SAMPLEINDEX smp = 1; smp <= GetNumInstruments(); smp++)
		{
			// For Startrekker AM synthesis, we need instrument envelopes.
			ModInstrument *ins = AllocateInstrument(smp, smp);
			if(ins == nullptr)
			{
				break;
			}
			ins->name = m_szNames[smp];

			AMInstrument am;
			// Allow partial reads for fa.worse face.mod
			if(amData.ReadStructPartial(am) && !memcmp(am.am, "AM", 2) && am.waveform < 4)
			{
				am.ConvertToMPT(Samples[smp], *ins, prng);
			}

			// This extra padding is probably present to have identical block sizes for AM and FM instruments.
			amData.Skip(120 - sizeof(AMInstrument));
		}
	}
#endif  // MPT_EXTERNAL_SAMPLES || MPT_BUILD_FUZZER

	if((loadFlags & loadSampleData) && isStartrekker && !m_nInstruments)
	{
		uint8 emptySampleReferences = 0;
		for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
		{
			if(referencedSamples[smp] && !Samples[smp].nLength)
			{
				if(++emptySampleReferences > 1)
				{
#ifdef MPT_EXTERNAL_SAMPLES
					mpt::ustring filenameHint;
					if(file.GetOptionalFileName())
					{
						const auto filename = file.GetOptionalFileName()->GetFilename().ToUnicode();
						filenameHint = MPT_UFORMAT(" ({}.nt or {}.as)")(filename, filename);
					}
					AddToLog(LogWarning, MPT_UFORMAT("This Startrekker AM file is most likely missing its companion file{}. Synthesized instruments will not play.")(filenameHint));
#else
					AddToLog(LogWarning, U_("This appears to be a Startrekker AM file with external synthesizes instruments. External instruments are currently not supported."));
#endif  // MPT_EXTERNAL_SAMPLES
					break;
				}
			}
		}
	}

	// His Master's Noise "Mupp" instrument extensions
	if((loadFlags & loadSampleData) && isHMNT)
	{
		uint8 muppCount = 0;
		for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
		{
			file.Seek(20 + (smp - 1) * sizeof(MODSampleHeader));
			if(!file.ReadMagic("Mupp") || !CanAddMoreSamples(28))
				continue;

			if(!m_nInstruments)
			{
				m_playBehaviour.set(kMODSampleSwap);
				m_nInstruments = 31;
				for(INSTRUMENTINDEX ins = 1; ins <= 31; ins++)
				{
					if(ModInstrument *instr = AllocateInstrument(ins, ins); instr != nullptr)
						instr->name = m_szNames[ins];
				}
			}
			ModInstrument *instr = Instruments[smp];
			if(!instr)
				continue;

			const auto [muppPattern, loopStart, loopEnd] = file.ReadArray<uint8, 3>();
			file.Seek(1084 + 1024 * muppPattern);
			SAMPLEINDEX startSmp = m_nSamples + 1;
			m_nSamples += 28;
			instr->AssignSample(startSmp);

			SampleIO sampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::signedPCM);
			for(SAMPLEINDEX muppSmp = startSmp; muppSmp <= m_nSamples; muppSmp++)
			{
				ModSample &mptSmp = Samples[muppSmp];
				mptSmp.Initialize(MOD_TYPE_MOD);
				mptSmp.nLength = 32;
				mptSmp.nLoopStart = 0;
				mptSmp.nLoopEnd = 32;
				mptSmp.nFineTune = Samples[smp].nFineTune;
				mptSmp.nVolume = Samples[smp].nVolume;
				mptSmp.uFlags.set(CHN_LOOP);
				sampleIO.ReadSample(mptSmp, file);
			}

			auto &events = instr->synth.m_scripts.emplace_back();
			events.reserve(std::min(loopEnd + 2, 65));
			const auto waveforms = file.ReadArray<uint8, 64>();
			const auto volumes = file.ReadArray<uint8, 64>();
			for(uint8 i = 0; i < 64; i++)
			{
				events.push_back(InstrumentSynth::Event::Mupp_SetWaveform(muppCount, waveforms[i], volumes[i]));
				if(i == loopEnd && loopStart <= loopEnd)
				{
					events.push_back(InstrumentSynth::Event::Jump(loopStart));
					break;
				}
			}
			muppCount++;
		}
	}

	// For "the ultimate beeper.mod"
	{
		ModSample &sample = Samples[0];
		sample.Initialize(MOD_TYPE_MOD);
		sample.nLength = 2;
		sample.nLoopStart = 0;
		sample.nLoopEnd = 2;
		sample.nVolume = 0;
		sample.uFlags.set(CHN_LOOP);
		sample.AllocateSample();
	}

	// Fix VBlank MODs. Arbitrary threshold: 8 minutes (enough for "frame of mind" by Dascon...).
	// Basically, this just converts all tempo commands into speed commands
	// for MODs which are supposed to have VBlank timing (instead of CIA timing).
	// There is no perfect way to do this, since both MOD types look the same,
	// but the most reliable way is to simply check for extremely long songs
	// (as this would indicate that e.g. a F30 command was really meant to set
	// the ticks per row to 48, and not the tempo to 48 BPM).
	// In the pattern loader above, a second condition is used: Only tempo commands
	// below 100 BPM are taken into account. Furthermore, only ProTracker (M.K. / M!K!)
	// modules are checked.
	if((isMdKd || IsMagic(magic, "M!K!")) && hasTempoCommands && !definitelyCIA)
	{
		const double songTime = GetLength(eNoAdjust).front().duration;
		if(songTime >= 480.0)
		{
			m_playBehaviour.set(kMODVBlankTiming);
			if(GetLength(eNoAdjust, GetLengthTarget(songTime)).front().targetReached)
			{
				// This just makes things worse, song is at least as long as in CIA mode
				// Obviously we should keep using CIA timing then...
				m_playBehaviour.reset(kMODVBlankTiming);
			} else
			{
				modMagicResult.madeWithTracker = UL_("ProTracker (VBlank)");
			}
		}
	}

	std::transform(std::begin(magic), std::end(magic), std::begin(magic), [](unsigned char c) -> unsigned char { return (c < ' ') ? ' ' : c; });
	m_modFormat.formatName = MPT_UFORMAT("ProTracker MOD ({})")(mpt::ToUnicode(mpt::Charset::ASCII, std::string(std::begin(magic), std::end(magic))));
	m_modFormat.type = UL_("mod");
	if(modMagicResult.madeWithTracker)
		m_modFormat.madeWithTracker = modMagicResult.madeWithTracker;
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	if(anyADPCM)
		m_modFormat.madeWithTracker += UL_(" (ADPCM packed)");

	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveMod(std::ostream &f) const
{
	if(GetNumChannels() == 0)
	{
		return false;
	}

	// Write song title
	{
		char name[20];
		mpt::String::WriteBuf(mpt::String::maybeNullTerminated, name) = m_songName;
		mpt::IO::Write(f, name);
	}

	std::vector<SmpLength> sampleLength(32, 0);
	std::vector<SAMPLEINDEX> sampleSource(32, 0);

	if(GetNumInstruments())
	{
		INSTRUMENTINDEX lastIns = std::min(INSTRUMENTINDEX(31), GetNumInstruments());
		for(INSTRUMENTINDEX ins = 1; ins <= lastIns; ins++) if (Instruments[ins])
		{
			// Find some valid sample associated with this instrument.
			for(auto smp : Instruments[ins]->Keyboard)
			{
				if(smp > 0 && smp <= GetNumSamples())
				{
					sampleSource[ins] = smp;
					break;
				}
			}
		}
	} else
	{
		for(SAMPLEINDEX i = 1; i <= 31; i++)
		{
			sampleSource[i] = i;
		}
	}

	// Write sample headers
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		MODSampleHeader sampleHeader;
		mpt::String::WriteBuf(mpt::String::maybeNullTerminated, sampleHeader.name) = m_szNames[sampleSource[smp]];
		sampleLength[smp] = sampleHeader.ConvertToMOD(sampleSource[smp] <= GetNumSamples() ? GetSample(sampleSource[smp]) : ModSample(MOD_TYPE_MOD));
		mpt::IO::Write(f, sampleHeader);
	}

	// Write order list
	MODFileHeader fileHeader;
	MemsetZero(fileHeader);

	PATTERNINDEX writePatterns = 0;
	uint8 writtenOrders = 0;
	for(ORDERINDEX ord = 0; ord < Order().GetLength() && writtenOrders < 128; ord++)
	{
		// Ignore +++ and --- patterns in order list, as well as high patterns (MOD officially only supports up to 128 patterns)
		if(ord == Order().GetRestartPos())
		{
			fileHeader.restartPos = writtenOrders;
		}
		if(Order()[ord] < 128)
		{
			fileHeader.orderList[writtenOrders++] = static_cast<uint8>(Order()[ord]);
			if(writePatterns <= Order()[ord])
			{
				writePatterns = Order()[ord] + 1;
			}
		}
	}
	fileHeader.numOrders = writtenOrders;
	mpt::IO::Write(f, fileHeader);

	// Write magic bytes
	char modMagic[4];
	CHANNELINDEX writeChannels = std::min(CHANNELINDEX(99), GetNumChannels());
	if(writeChannels == 4)
	{
		// ProTracker may not load files with more than 64 patterns correctly if we do not specify the M!K! magic.
		if(writePatterns <= 64)
			memcpy(modMagic, "M.K.", 4);
		else
			memcpy(modMagic, "M!K!", 4);
	} else if(writeChannels < 10)
	{
		memcpy(modMagic, "0CHN", 4);
		modMagic[0] += static_cast<char>(writeChannels);
	} else
	{
		memcpy(modMagic, "00CH", 4);
		modMagic[0] += static_cast<char>(writeChannels / 10u);
		modMagic[1] += static_cast<char>(writeChannels % 10u);
	}
	mpt::IO::Write(f, modMagic);

	// Write patterns
	bool invalidInstruments = false;
	std::vector<uint8> events;
	for(PATTERNINDEX pat = 0; pat < writePatterns; pat++)
	{
		if(!Patterns.IsValidPat(pat))
		{
			// Invent empty pattern
			events.assign(writeChannels * 64 * 4, 0);
			mpt::IO::Write(f, events);
			continue;
		}

		for(ROWINDEX row = 0; row < 64; row++)
		{
			if(row >= Patterns[pat].GetNumRows())
			{
				// Invent empty row
				events.assign(writeChannels * 4, 0);
				mpt::IO::Write(f, events);
				continue;
			}
			const auto rowBase = Patterns[pat].GetRow(row);
			bool writePatternBreak = (Patterns[pat].GetNumRows() < 64 && row + 1 == Patterns[pat].GetNumRows() && !Patterns[pat].RowHasJump(row));

			events.resize(writeChannels * 4);
			size_t eventByte = 0;
			for(CHANNELINDEX chn = 0; chn < writeChannels; chn++, eventByte += 4)
			{
				const ModCommand &m = rowBase[chn];
				uint8 command = 0, param = 0;
				ModSaveCommand(m, command, param, false, true);

				if(m.volcmd == VOLCMD_VOLUME && !command && !param)
				{
					// Maybe we can save some volume commands...
					command = 0x0C;
					param = std::min(m.vol, uint8(64));
				}
				if(writePatternBreak && !command && !param)
				{
					command = 0x0D;
					writePatternBreak = false;
				}

				uint16 period = 0;
				// Convert note to period
				if(m.note >= 24 + NOTE_MIN && m.note < std::size(ProTrackerPeriodTable) + 24 + NOTE_MIN)
				{
					period = ProTrackerPeriodTable[m.note - 24 - NOTE_MIN];
				}

				const uint8 instr = (m.instr > 31) ? 0 : m.instr;
				if(m.instr > 31)
					invalidInstruments = true;

				events[eventByte + 0] = ((period >> 8) & 0x0F) | (instr & 0x10);
				events[eventByte + 1] = period & 0xFFu;
				events[eventByte + 2] = ((instr & 0x0F) << 4) | (command & 0x0F);
				events[eventByte + 3] = param;
			}
			mpt::IO::WriteRaw(f, mpt::as_span(events));
		}
	}

	if(invalidInstruments)
	{
		AddToLog(LogWarning, U_("Warning: This track references sample slots higher than 31. Such samples cannot be saved in the MOD format, and thus the notes will not sound correct. Use the Cleanup tool to rearrange and remove unused samples."));
	}

	//Check for unsaved patterns
	for(PATTERNINDEX pat = writePatterns; pat < Patterns.Size(); pat++)
	{
		if(Patterns.IsValidPat(pat))
		{
			AddToLog(LogWarning, U_("Warning: This track contains at least one pattern after the highest pattern number referred to in the sequence. Such patterns are not saved in the MOD format."));
			break;
		}
	}

	// Writing samples
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		if(sampleLength[smp] == 0)
		{
			continue;
		}
		const ModSample &sample = Samples[sampleSource[smp]];

		const mpt::IO::Offset sampleStart = mpt::IO::TellWrite(f);
		const size_t writtenBytes = MODSampleHeader::GetSampleFormat().WriteSample(f, sample, sampleLength[smp]);

		const int8 silence = 0;

		// Write padding byte if the sample size is odd.
		if((writtenBytes % 2u) != 0)
		{
			mpt::IO::Write(f, silence);
		}

		if(!sample.uFlags[CHN_LOOP] && writtenBytes >= 2)
		{
			// First two bytes of oneshot samples have to be 0 due to PT's one-shot loop
			const mpt::IO::Offset sampleEnd = mpt::IO::TellWrite(f);
			mpt::IO::SeekAbsolute(f, sampleStart);
			mpt::IO::Write(f, silence);
			mpt::IO::Write(f, silence);
			mpt::IO::SeekAbsolute(f, sampleEnd);
		}
	}

	return true;
}

#endif  // MODPLUG_NO_FILESAVE


OPENMPT_NAMESPACE_END
