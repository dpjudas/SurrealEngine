/*
 * Load_mid.cpp
 * ------------
 * Purpose: MIDI file loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "Dlsbank.h"
#include "MIDIEvents.h"
#include "mod_specifications.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/TrackerSettings.h"
#include "../mptrack/Moddoc.h"
#include "../mptrack/Mptrack.h"
#include "mpt/io_file/inputfile.hpp"
#include "mpt/io_file_read/inputfile_filecursor.hpp"
#include "../common/mptFileIO.h"
#include "mpt/fs/fs.hpp"
#endif // MODPLUG_TRACKER

OPENMPT_NAMESPACE_BEGIN

#if defined(MODPLUG_TRACKER) || defined(MPT_FUZZ_TRACKER)

#ifdef LIBOPENMPT_BUILD
struct CDLSBank { static int32 DLSMidiVolumeToLinear(uint32) { return 256; } };
#endif // LIBOPENMPT_BUILD

#define MIDI_DRUMCHANNEL	10

const char *szMidiGroupNames[17] =
{
	"Piano",
	"Chromatic Percussion",
	"Organ",
	"Guitar",
	"Bass",
	"Strings",
	"Ensemble",
	"Brass",
	"Reed",
	"Pipe",
	"Synth Lead",
	"Synth Pad",
	"Synth Effects",
	"Ethnic",
	"Percussive",
	"Sound Effects",
	"Percussions"
};


const char *szMidiProgramNames[128] =
{
	// 1-8: Piano
	"Acoustic Grand Piano",
	"Bright Acoustic Piano",
	"Electric Grand Piano",
	"Honky-tonk Piano",
	"Electric Piano 1",
	"Electric Piano 2",
	"Harpsichord",
	"Clavi",
	// 9-16: Chromatic Percussion
	"Celesta",
	"Glockenspiel",
	"Music Box",
	"Vibraphone",
	"Marimba",
	"Xylophone",
	"Tubular Bells",
	"Dulcimer",
	// 17-24: Organ
	"Drawbar Organ",
	"Percussive Organ",
	"Rock Organ",
	"Church Organ",
	"Reed Organ",
	"Accordion",
	"Harmonica",
	"Tango Accordion",
	// 25-32: Guitar
	"Acoustic Guitar (nylon)",
	"Acoustic Guitar (steel)",
	"Electric Guitar (jazz)",
	"Electric Guitar (clean)",
	"Electric Guitar (muted)",
	"Overdriven Guitar",
	"Distortion Guitar",
	"Guitar harmonics",
	// 33-40   Bass
	"Acoustic Bass",
	"Electric Bass (finger)",
	"Electric Bass (pick)",
	"Fretless Bass",
	"Slap Bass 1",
	"Slap Bass 2",
	"Synth Bass 1",
	"Synth Bass 2",
	// 41-48   Strings
	"Violin",
	"Viola",
	"Cello",
	"Contrabass",
	"Tremolo Strings",
	"Pizzicato Strings",
	"Orchestral Harp",
	"Timpani",
	// 49-56   Ensemble
	"String Ensemble 1",
	"String Ensemble 2",
	"SynthStrings 1",
	"SynthStrings 2",
	"Choir Aahs",
	"Voice Oohs",
	"Synth Voice",
	"Orchestra Hit",
	// 57-64   Brass
	"Trumpet",
	"Trombone",
	"Tuba",
	"Muted Trumpet",
	"French Horn",
	"Brass Section",
	"SynthBrass 1",
	"SynthBrass 2",
	// 65-72   Reed
	"Soprano Sax",
	"Alto Sax",
	"Tenor Sax",
	"Baritone Sax",
	"Oboe",
	"English Horn",
	"Bassoon",
	"Clarinet",
	// 73-80   Pipe
	"Piccolo",
	"Flute",
	"Recorder",
	"Pan Flute",
	"Blown Bottle",
	"Shakuhachi",
	"Whistle",
	"Ocarina",
	// 81-88   Synth Lead
	"Lead 1 (square)",
	"Lead 2 (sawtooth)",
	"Lead 3 (calliope)",
	"Lead 4 (chiff)",
	"Lead 5 (charang)",
	"Lead 6 (voice)",
	"Lead 7 (fifths)",
	"Lead 8 (bass + lead)",
	// 89-96   Synth Pad
	"Pad 1 (new age)",
	"Pad 2 (warm)",
	"Pad 3 (polysynth)",
	"Pad 4 (choir)",
	"Pad 5 (bowed)",
	"Pad 6 (metallic)",
	"Pad 7 (halo)",
	"Pad 8 (sweep)",
	// 97-104  Synth Effects
	"FX 1 (rain)",
	"FX 2 (soundtrack)",
	"FX 3 (crystal)",
	"FX 4 (atmosphere)",
	"FX 5 (brightness)",
	"FX 6 (goblins)",
	"FX 7 (echoes)",
	"FX 8 (sci-fi)",
	// 105-112 Ethnic
	"Sitar",
	"Banjo",
	"Shamisen",
	"Koto",
	"Kalimba",
	"Bag pipe",
	"Fiddle",
	"Shanai",
	// 113-120 Percussive
	"Tinkle Bell",
	"Agogo",
	"Steel Drums",
	"Woodblock",
	"Taiko Drum",
	"Melodic Tom",
	"Synth Drum",
	"Reverse Cymbal",
	// 121-128 Sound Effects
	"Guitar Fret Noise",
	"Breath Noise",
	"Seashore",
	"Bird Tweet",
	"Telephone Ring",
	"Helicopter",
	"Applause",
	"Gunshot"
};


// Notes 25-85
const char *szMidiPercussionNames[61] =
{
	"Seq Click",
	"Brush Tap",
	"Brush Swirl",
	"Brush Slap",
	"Brush Swirl W/Attack",
	"Snare Roll",
	"Castanet",
	"Snare Lo",
	"Sticks",
	"Bass Drum Lo",
	"Open Rim Shot",
	"Acoustic Bass Drum",
	"Bass Drum 1",
	"Side Stick",
	"Acoustic Snare",
	"Hand Clap",
	"Electric Snare",
	"Low Floor Tom",
	"Closed Hi-Hat",
	"High Floor Tom",
	"Pedal Hi-Hat",
	"Low Tom",
	"Open Hi-Hat",
	"Low-Mid Tom",
	"Hi Mid Tom",
	"Crash Cymbal 1",
	"High Tom",
	"Ride Cymbal 1",
	"Chinese Cymbal",
	"Ride Bell",
	"Tambourine",
	"Splash Cymbal",
	"Cowbell",
	"Crash Cymbal 2",
	"Vibraslap",
	"Ride Cymbal 2",
	"Hi Bongo",
	"Low Bongo",
	"Mute Hi Conga",
	"Open Hi Conga",
	"Low Conga",
	"High Timbale",
	"Low Timbale",
	"High Agogo",
	"Low Agogo",
	"Cabasa",
	"Maracas",
	"Short Whistle",
	"Long Whistle",
	"Short Guiro",
	"Long Guiro",
	"Claves",
	"Hi Wood Block",
	"Low Wood Block",
	"Mute Cuica",
	"Open Cuica",
	"Mute Triangle",
	"Open Triangle",
	"Shaker",
	"Jingle Bell",
	"Bell Tree",
};


static constexpr uint8 NUM_MIDI_CHANNELS = 32;


////////////////////////////////////////////////////////////////////////////////
// Maps a midi instrument - returns the instrument number in the file
uint32 CSoundFile::MapMidiInstrument(uint8 program, uint16 bank, uint8 midiChannel, uint8 note, bool isXG, std::bitset<NUM_MIDI_CHANNELS> drumChns)
{
	ModInstrument *pIns;
	program &= 0x7F;
	bank &= 0x3FFF;
	note &= 0x7F;

	// In XG mode, extra drums are on banks with MSB 7F
	const bool isDrum = drumChns[midiChannel - 1] || (bank >= 0x3F80 && isXG);

	for (uint32 i = 1; i <= m_nInstruments; i++) if (Instruments[i])
	{
		ModInstrument *p = Instruments[i];
		// Drum Kit?
		if (isDrum)
		{
			if (note == p->nMidiDrumKey && bank + 1 == p->wMidiBank) return i;
		} else
		// Melodic Instrument
		{
			if (program + 1 == p->nMidiProgram && bank + 1 == p->wMidiBank && p->nMidiDrumKey == 0) return i;
		}
	}
	if(!CanAddMoreInstruments() || !CanAddMoreSamples())
		return 0;

	pIns = AllocateInstrument(m_nInstruments + 1);
	if(pIns == nullptr)
	{
		return 0;
	}

	m_nSamples++;
	pIns->wMidiBank = bank + 1;
	pIns->nMidiProgram = program + 1;
	pIns->nFadeOut = 1024;
	pIns->nNNA = NewNoteAction::NoteOff;
	pIns->nDCT = isDrum ? DuplicateCheckType::Sample : DuplicateCheckType::Note;
	pIns->nDNA = DuplicateNoteAction::NoteFade;
	if(isDrum)
	{
		pIns->nMidiChannel = MIDI_DRUMCHANNEL;
		pIns->nMidiDrumKey = note;
		for(auto &key : pIns->NoteMap)
		{
			key = NOTE_MIDDLEC;
		}
	}
	pIns->VolEnv.dwFlags.set(ENV_ENABLED);
	if (!isDrum) pIns->VolEnv.dwFlags.set(ENV_SUSTAIN);
	pIns->VolEnv.reserve(4);
	pIns->VolEnv.push_back(EnvelopeNode(0, ENVELOPE_MAX));
	pIns->VolEnv.push_back(EnvelopeNode(10, ENVELOPE_MAX));
	pIns->VolEnv.push_back(EnvelopeNode(15, (ENVELOPE_MAX + ENVELOPE_MID) / 2));
	pIns->VolEnv.push_back(EnvelopeNode(20, ENVELOPE_MIN));
	pIns->VolEnv.nSustainStart = pIns->VolEnv.nSustainEnd = 1;
	// Set GM program / drum name
	if (!isDrum)
	{
		pIns->name = szMidiProgramNames[program];
	} else
	{
		if (note >= 24 && note <= 84)
			pIns->name = szMidiPercussionNames[note - 24];
		else
			pIns->name = "Percussions";
	}
	return m_nInstruments;
}


struct MThd
{
	uint32be headerLength;
	uint16be format;		// 0 = single-track, 1 = multi-track, 2 = multi-song
	uint16be numTracks;		// Number of track chunks
	uint16be division;		// Delta timing value: positive = units/beat; negative = smpte compatible units
};

MPT_BINARY_STRUCT(MThd, 10)


using tick_t = uint32;

struct TrackState
{
	FileReader track;
	tick_t nextEvent = 0;
	uint8 command = 0;
	uint8 midiBaseChannel = 0;
	bool finished = false;
};

struct ModChannelState
{
	static constexpr uint8 NOMIDI = 0xFF;  // No MIDI channel assigned.

	tick_t age = 0;                     // At which MIDI tick the channel was triggered
	int32 porta = 0;                    // Current portamento position in extra-fine slide units (1/64th of a semitone)
	uint8 vol = 100;                    // MIDI note volume (0...127)
	uint8 pan = 128;                    // MIDI channel panning (0...256)
	uint8 midiCh = NOMIDI;              // MIDI channel that was last played on this channel
	ModCommand::NOTE note = NOTE_NONE;  // MIDI note that was last played on this channel
	bool sustained = false;             // If true, the note was already released by a note-off event, but sustain pedal CC is still active
};

struct MidiChannelState
{
	int32  pitchbendMod = 0;  // Pre-computed pitchbend in extra-fine slide units (1/64th of a semitone)
	int16  pitchbend = MIDIEvents::pitchBendCentre; // 0...16383
	uint16 bank = 0;          // 0...16383
	uint8  program = 0;       // 0...127
	// -- Controllers ---------------- function ---------- CC# --- range  ---- init (midi) ---
	uint8 pan = 128;          // Channel Panning           10      [0-255]     128  (64)
	uint8 expression = 128;   // Channel Expression        11      0-128       128  (127)
	uint8 volume = 80;        // Channel Volume            7       0-128       80   (100)
	uint16 rpn = 0x3FFF;      // Currently selected RPN    100/101  n/a
	uint8 pitchBendRange = 2; // Pitch Bend Range                              2
	int8  transpose = 0;      // Channel transpose                             0
	bool  monoMode = false;   // Mono/Poly operation       126/127  n/a        Poly
	bool  sustain = false;    // Sustain pedal             64       on/off     off

	std::array<CHANNELINDEX, 128> noteOn;  // Value != CHANNELINDEX_INVALID: Note is active and mapped to mod channel in value

	MidiChannelState()
	{
		noteOn.fill(CHANNELINDEX_INVALID);
	}

	void SetPitchbend(uint16 value)
	{
		pitchbend = value;
		// Convert from arbitrary MIDI pitchbend to 64th of semitone
		pitchbendMod = Util::muldiv(pitchbend - MIDIEvents::pitchBendCentre, pitchBendRange * 64, MIDIEvents::pitchBendCentre);
	}

	void ResetAllControllers()
	{
		expression = 128;
		pitchBendRange = 2;
		SetPitchbend(MIDIEvents::pitchBendCentre);
		transpose = 0;
		rpn = 0x3FFF;
		monoMode = false;
		sustain = false;
		// Should also reset modulation, pedals (40h-43h), aftertouch
	}

	void SetRPN(uint8 value)
	{
		switch(rpn)
		{
		case 0: // Pitch Bend Range
			pitchBendRange = std::max(value, uint8(1));
			SetPitchbend(pitchbend);
			break;
		case 2: // Coarse Tune
			transpose = static_cast<int8>(value) - 64;
			break;
		}
	}

	void SetRPNRelative(int8 value)
	{
		switch(rpn)
		{
		case 0: // Pitch Bend Range
			pitchBendRange = static_cast<uint8>(std::clamp(pitchBendRange + value, 1, 0x7F));
			break;
		case 2: // Coarse Tune
			transpose = mpt::saturate_cast<int8>(transpose + value);
			break;
		}
	}
};


static CHANNELINDEX FindUnusedChannel(uint8 midiCh, ModCommand::NOTE note, const std::vector<ModChannelState> &channels, bool monoMode, mpt::span<ModCommand> patRow)
{
	for(size_t i = 0; i < channels.size(); i++)
	{
		// Check if this note is already playing, or find any note of the same MIDI channel in case of mono mode
		if(channels[i].midiCh == midiCh && (channels[i].note == note || (monoMode && channels[i].note != NOTE_NONE)))
		{
			return static_cast<CHANNELINDEX>(i);
		}
	}
	
	CHANNELINDEX anyUnusedChannel = CHANNELINDEX_INVALID;
	CHANNELINDEX anyFreeChannel = CHANNELINDEX_INVALID;

	CHANNELINDEX oldsetMidiCh = CHANNELINDEX_INVALID;
	tick_t oldestMidiChAge = std::numeric_limits<decltype(oldestMidiChAge)>::max();

	CHANNELINDEX oldestAnyCh = 0;
	tick_t oldestAnyChAge = std::numeric_limits<decltype(oldestAnyChAge)>::max();

	for(size_t i = 0; i < channels.size(); i++)
	{
		if(channels[i].note == NOTE_NONE && !patRow[i].IsNote())
		{
			// Recycle channel previously used by the same MIDI channel
			if(channels[i].midiCh == midiCh)
				return static_cast<CHANNELINDEX>(i);
			// If we cannot find a channel that was already used for the same MIDI channel, try a completely unused channel next
			else if(channels[i].midiCh == ModChannelState::NOMIDI && anyUnusedChannel == CHANNELINDEX_INVALID)
				anyUnusedChannel = static_cast<CHANNELINDEX>(i);
			// And if that fails, try any channel that currently doesn't play a note.
			if(anyFreeChannel == CHANNELINDEX_INVALID)
				anyFreeChannel = static_cast<CHANNELINDEX>(i);
		}

		// If we can't find any free channels, look for the oldest channels
		if(channels[i].midiCh == midiCh && channels[i].age < oldestMidiChAge)
		{
			// Oldest channel matching this MIDI channel
			oldestMidiChAge = channels[i].age;
			oldsetMidiCh = static_cast<CHANNELINDEX>(i);
		} else if(channels[i].age < oldestAnyChAge)
		{
			// Any oldest channel
			oldestAnyChAge = channels[i].age;
			oldestAnyCh = static_cast<CHANNELINDEX>(i);
		}
	}
	if(anyUnusedChannel != CHANNELINDEX_INVALID)
		return anyUnusedChannel;
	if(anyFreeChannel != CHANNELINDEX_INVALID)
		return anyFreeChannel;
	if(oldsetMidiCh != CHANNELINDEX_INVALID)
		return oldsetMidiCh;
	return oldestAnyCh;
}


static void MIDINoteOff(MidiChannelState &midiChn, std::vector<ModChannelState> &modChnStatus, uint8 note, uint8 delay, mpt::span<ModCommand> patRow, std::bitset<NUM_MIDI_CHANNELS> drumChns)
{
	CHANNELINDEX chn = midiChn.noteOn[note];
	if(chn == CHANNELINDEX_INVALID)
		return;

	if(midiChn.sustain)
	{
		// Turn this off later
		modChnStatus[chn].sustained = true;
		return;
	}

	uint8 midiCh = modChnStatus[chn].midiCh;
	modChnStatus[chn].note = NOTE_NONE;
	modChnStatus[chn].sustained = false;
	midiChn.noteOn[note] = CHANNELINDEX_INVALID;
	ModCommand &m = patRow[chn];
	if(m.note == NOTE_NONE)
	{
		m.note = NOTE_KEYOFF;
		if(delay != 0)
		{
			m.command = CMD_S3MCMDEX;
			m.param = 0xD0 | delay;
		}
	} else if(m.IsNote() && !drumChns[midiCh])
	{
		// Only do note cuts for melodic instruments - they sound weird on drums which should fade out naturally.
		if(m.command == CMD_S3MCMDEX && (m.param & 0xF0) == 0xD0)
		{
			// Already have a note delay
			m.command = CMD_DELAYCUT;
			m.param = (m.param << 4) | (delay - (m.param & 0x0F));
		} else if(m.command == CMD_NONE || m.command == CMD_PANNING8)
		{
			m.command = CMD_S3MCMDEX;
			m.param = 0xC0 | delay;
		}
	}
}


static void EnterMIDIVolume(ModCommand &m, ModChannelState &modChn, const MidiChannelState &midiChn)
{
	m.volcmd = VOLCMD_VOLUME;

	int32 vol = CDLSBank::DLSMidiVolumeToLinear(modChn.vol) >> 8;
	vol = (vol * midiChn.volume * midiChn.expression) >> 13;
	Limit(vol, 4, 256);
	m.vol = static_cast<ModCommand::VOL>(vol / 4);
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMID(MemoryFileReader file, const uint64 *pfilesize)
{
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	char magic[4];
	file.ReadArray(magic);
	if(!memcmp(magic, "MThd", 4))
		return ProbeSuccess;

	if(!memcmp(magic, "RIFF", 4) && file.Skip(4) && file.ReadMagic("RMID"))
		return ProbeSuccess;

	return ProbeFailure;
}


bool CSoundFile::ReadMID(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	// Microsoft MIDI files
	bool isRIFF = false;
	if(file.ReadMagic("RIFF"))
	{
		file.Skip(4);
		if(!file.ReadMagic("RMID"))
		{
			return false;
		} else if(loadFlags == onlyVerifyHeader)
		{
			return true;
		}
		do
		{
			char id[4];
			file.ReadArray(id);
			uint32 length = file.ReadUint32LE();
			if(memcmp(id, "data", 4))
			{
				file.Skip(length);
			} else
			{
				isRIFF = true;
				break;
			}
		} while(file.CanRead(8));
	}

	MThd fileHeader;
	if(!file.ReadMagic("MThd")
		|| !file.ReadStruct(fileHeader)
		|| fileHeader.numTracks == 0
		|| fileHeader.headerLength < 6
		|| !file.Skip(fileHeader.headerLength - 6))
	{
		return false;
	} else if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_MID, GetModSpecifications(MOD_TYPE_MPT).channelsMax);

#ifdef MODPLUG_TRACKER
	const uint32 quantize = Clamp(TrackerSettings::Instance().midiImportQuantize.Get(), 4u, 256u);
	const ROWINDEX patternLen = Clamp(TrackerSettings::Instance().midiImportPatternLen.Get(), GetModSpecifications().patternRowsMin, GetModSpecifications().patternRowsMax);
	const uint8 ticksPerRow = Clamp(TrackerSettings::Instance().midiImportTicks.Get(), uint8(2), uint8(16));
#else
	const uint32 quantize = 32;		// Must be 4 or higher
	const ROWINDEX patternLen = 128;
	const uint8 ticksPerRow = 16;	// Must be in range 2...16
#endif
#ifdef MPT_FUZZ_TRACKER
	// Avoid generating test cases that take overly long to evaluate
	const ORDERINDEX MPT_MIDI_IMPORT_MAX_ORDERS = 64;
#else
	const ORDERINDEX MPT_MIDI_IMPORT_MAX_ORDERS = MAX_ORDERS;
#endif

	m_songArtist = UL_("MIDI Conversion");
	m_modFormat.formatName = UL_("Standard MIDI File");
	m_modFormat.type = isRIFF ? UL_("rmi") : UL_("mid");
	m_modFormat.madeWithTracker = UL_("Standard MIDI File");
	m_modFormat.charset = mpt::Charset::ISO8859_1;

	SetMixLevels(MixLevels::v1_17RC3);
	m_nTempoMode = TempoMode::Modern;
	m_SongFlags = SONG_LINEARSLIDES;
	TEMPO tempo{120, 0};
	Order().SetDefaultTempo(tempo);
	Order().SetDefaultSpeed(ticksPerRow);
	m_nDefaultRowsPerBeat = quantize / 4;
	m_nDefaultRowsPerMeasure = 4 * m_nDefaultRowsPerBeat;
	m_nSamplePreAmp = m_nVSTiVolume = 32;
	uint16 ppqn = fileHeader.division;
	if(ppqn & 0x8000)
	{
		// SMPTE compatible units (approximation)
		int frames = 256 - (ppqn >> 8), subFrames = (ppqn & 0xFF);
		ppqn = static_cast<uint16>(frames * subFrames / 2);
	}
	if(!ppqn)
		ppqn = 96;
	Order().clear();

	std::array<MidiChannelState, NUM_MIDI_CHANNELS> midiChnStatus;
	const CHANNELINDEX tempoChannel = GetNumChannels() - 2, globalVolChannel = GetNumChannels() - 1;
	const uint16 numTracks = fileHeader.numTracks;
	std::vector<TrackState> tracks(numTracks);
	std::vector<ModChannelState> modChnStatus(GetNumChannels());
	std::bitset<NUM_MIDI_CHANNELS> drumChns;
	drumChns.set(MIDI_DRUMCHANNEL - 1);
	drumChns.set(MIDI_DRUMCHANNEL + 15);

	tick_t timeShift = 0;
	for(auto &track : tracks)
	{
		if(!file.ReadMagic("MTrk"))
			return false;
		track.track = file.ReadChunk(file.ReadUint32BE());
		tick_t delta = 0;
		track.track.ReadVarInt(delta);
		// Work-around for some MID files that assume that negative deltas exist (they don't according to the standard)
		if(delta > int32_max)
			timeShift = std::max(static_cast<tick_t>(~delta  + 1), timeShift);
		track.nextEvent = delta;
	}
	if(timeShift != 0)
	{
		for(auto &track : tracks)
		{
			if(track.nextEvent > int32_max)
				track.nextEvent = timeShift - static_cast<tick_t>(~track.nextEvent + 1);
			else
				track.nextEvent += timeShift;
		}
	}

	uint16 finishedTracks = 0;
	PATTERNINDEX emptyPattern = PATTERNINDEX_INVALID;
	ORDERINDEX lastOrd = 0, loopEndOrd = ORDERINDEX_INVALID;
	ROWINDEX lastRow = 0, loopEndRow = ROWINDEX_INVALID;
	ROWINDEX restartRow = ROWINDEX_INVALID;
	int8 masterTranspose = 0;
	bool isXG = false;
	bool isEMIDI = false;
	bool isEMIDILoop = false;
	const bool isType2 = (fileHeader.format == 2);

	const auto ModPositionFromTick = [&](const tick_t tick, const tick_t offset = 0)
	{
		tick_t modTicks = Util::muldivr_unsigned(tick, quantize * ticksPerRow, ppqn * 4u) - offset;

		ORDERINDEX ord = static_cast<ORDERINDEX>((modTicks / ticksPerRow) / patternLen);
		ROWINDEX row = (modTicks / ticksPerRow) % patternLen;
		uint8 delay = static_cast<uint8>(modTicks % ticksPerRow);

		return std::make_tuple(ord, row, delay);
	};

	while(finishedTracks < numTracks)
	{
		uint16 t = 0;
		tick_t tick = std::numeric_limits<decltype(tick)>::max();
		for(uint16 track = 0; track < numTracks; track++)
		{
			if(!tracks[track].finished && tracks[track].nextEvent < tick)
			{
				tick = tracks[track].nextEvent;
				t = track;
				if(isType2)
					break;
			}
		}
		FileReader &track = tracks[t].track;

		const auto [ord, row, delay] = ModPositionFromTick(tick);

		if(ord >= Order().GetLength())
		{
			if(ord > MPT_MIDI_IMPORT_MAX_ORDERS)
				break;
			ORDERINDEX curSize = Order().GetLength();
			// If we need to extend the order list by more than one pattern, this means that we
			// will be filling in empty patterns. Just recycle one empty pattern for this job.
			// We read events in chronological order, so it is never possible for the loader to
			// "jump back" to one of those empty patterns and write into it.
			if(ord > curSize && emptyPattern == PATTERNINDEX_INVALID)
			{
				if((emptyPattern = Patterns.InsertAny(patternLen)) == PATTERNINDEX_INVALID)
					break;
			}
			Order().resize(ord + 1, emptyPattern);

			if((Order()[ord] = Patterns.InsertAny(patternLen)) == PATTERNINDEX_INVALID)
				break;
		}

		// Keep track of position of last event for resizing the last pattern
		if(ord > lastOrd)
		{
			lastOrd = ord;
			lastRow = row;
		} else if(ord == lastOrd)
		{
			lastRow = std::max(lastRow, row);
		}

		PATTERNINDEX pat = Order()[ord];
		auto patRow = Patterns[pat].GetRow(row);

		uint8 data1 = track.ReadUint8();
		if(data1 == 0xFF)
		{
			// Meta events
			data1 = track.ReadUint8();
			size_t len = 0;
			track.ReadVarInt(len);
			FileReader chunk = track.ReadChunk(len);

			switch(data1)
			{
			case 1: // Text
			case 2: // Copyright
				m_songMessage.Read(chunk, len, SongMessage::leAutodetect);
				break;
			case 3: // Track Name
				if(len > 0)
				{
					std::string s;
					chunk.ReadString<mpt::String::maybeNullTerminated>(s, len);
					if(!m_songMessage.empty())
						m_songMessage.append(1, SongMessage::InternalLineEnding);
					m_songMessage += s;
					if(m_songName.empty())
						m_songName = s;
				}
				break;
			case 4: // Instrument
			case 5: // Lyric
				break;
			case 6: // Marker
			case 7: // Cue point
				{
					std::string s;
					chunk.ReadString<mpt::String::maybeNullTerminated>(s, len);
					Patterns[pat].SetName(s);
					if(!mpt::CompareNoCaseAscii(s, "loopStart"))
					{
						Order().SetRestartPos(ord);
						restartRow = row;
					} else if(!mpt::CompareNoCaseAscii(s, "loopEnd"))
					{
						std::tie(loopEndOrd, loopEndRow, std::ignore) = ModPositionFromTick(tick, 1);
					}
				}
				break;
			case 8: // Patch name
			case 9: // Port name
				break;
			case 0x21: // MIDI port
				tracks[t].midiBaseChannel = chunk.ReadUint8() * 16u;
				break;
			case 0x2F: // End Of Track
				tracks[t].finished = true;
				break;
			case 0x51: // Tempo
				{
					uint32 tempoInt = chunk.ReadUint24BE();
					if(tempoInt == 0)
						break;
					TEMPO newTempo(60000000.0 / tempoInt);
					if(!tick)
					{
						Order().SetDefaultTempo(newTempo);
					} else if(newTempo != tempo)
					{
						patRow[tempoChannel].command = CMD_TEMPO;
						patRow[tempoChannel].param = mpt::saturate_round<ModCommand::PARAM>(std::max(32.0, newTempo.ToDouble()));
					}
					tempo = newTempo;
				}
				break;
			case 0x7F: // Sequencer specific
				{
					// Yamaha MIDI port selection
					uint32 data = chunk.ReadUint32BE();
					if(chunk.LengthIs(4) && (data & 0xFFFFFF00) == 0x43000100)
						tracks[t].midiBaseChannel = static_cast<uint8>((data & 0xFF) * 16u);
				}
				break;

			default:
				break;
			}
		} else
		{
			uint8 command = tracks[t].command;
			if(data1 & 0x80)
			{
				// Command byte (if not present, use running status for channel messages)
				command = data1;
				if(data1 < 0xF0)
				{
					tracks[t].command = data1;
					data1 = track.ReadUint8();
				}
			}
			const uint8 midiCh = ((command & 0x0F) + tracks[t].midiBaseChannel) % NUM_MIDI_CHANNELS;

			switch(command & 0xF0)
			{
			case 0x80: // Note Off
			case 0x90: // Note On
				{
					data1 &= 0x7F;
					ModCommand::NOTE note = static_cast<ModCommand::NOTE>(Clamp(data1 + NOTE_MIN, NOTE_MIN, NOTE_MAX));
					uint8 data2 = track.ReadUint8();
					if(data2 > 0 && (command & 0xF0) == 0x90)
					{
						// Note On
						CHANNELINDEX chn = FindUnusedChannel(midiCh, note, modChnStatus, midiChnStatus[midiCh].monoMode, patRow);
						if(chn != CHANNELINDEX_INVALID)
						{
							modChnStatus[chn].age = tick;
							modChnStatus[chn].note = note;
							modChnStatus[chn].midiCh = midiCh;
							modChnStatus[chn].vol = data2;
							modChnStatus[chn].sustained = false;
							midiChnStatus[midiCh].noteOn[data1] = chn;
							int32 pitchOffset = 0;
							if(midiChnStatus[midiCh].pitchbendMod != 0)
							{
								pitchOffset = (midiChnStatus[midiCh].pitchbendMod + (midiChnStatus[midiCh].pitchbendMod > 0 ? 32 : -32)) / 64;
								modChnStatus[chn].porta = pitchOffset * 64;
							} else
							{
								modChnStatus[chn].porta = 0;
							}
							patRow[chn].note = static_cast<ModCommand::NOTE>(Clamp(note + pitchOffset + midiChnStatus[midiCh].transpose + masterTranspose, NOTE_MIN, NOTE_MAX));
							patRow[chn].instr = mpt::saturate_cast<ModCommand::INSTR>(MapMidiInstrument(midiChnStatus[midiCh].program, midiChnStatus[midiCh].bank, midiCh + 1, data1, isXG, drumChns));
							EnterMIDIVolume(patRow[chn], modChnStatus[chn], midiChnStatus[midiCh]);

							if(patRow[chn].command == CMD_PORTAMENTODOWN || patRow[chn].command == CMD_PORTAMENTOUP)
							{
								patRow[chn].command = CMD_NONE;
							}
							if(delay != 0)
							{
								patRow[chn].command = CMD_S3MCMDEX;
								patRow[chn].param = 0xD0 | delay;
							}
							if(modChnStatus[chn].pan != midiChnStatus[midiCh].pan && patRow[chn].command == CMD_NONE)
							{
								patRow[chn].command = CMD_PANNING8;
								patRow[chn].param = midiChnStatus[midiCh].pan;
								modChnStatus[chn].pan = midiChnStatus[midiCh].pan;
							}
						}
					} else
					{
						// Note Off
						MIDINoteOff(midiChnStatus[midiCh], modChnStatus, data1, delay, patRow, drumChns);
					}
				}
				break;
			case 0xA0: // Note Aftertouch
				{
					track.Skip(1);
				}
				break;
			case 0xB0: // Controller
				{
					uint8 data2 = track.ReadUint8();
					switch(data1)
					{
					case MIDIEvents::MIDICC_Panposition_Coarse:
						midiChnStatus[midiCh].pan = data2 * 2u;
						for(auto chn : midiChnStatus[midiCh].noteOn)
						{
							if(chn != CHANNELINDEX_INVALID && modChnStatus[chn].pan != midiChnStatus[midiCh].pan)
							{
								if(Patterns[pat].WriteEffect(EffectWriter(CMD_PANNING8, midiChnStatus[midiCh].pan).Channel(chn).Row(row)))
								{
									modChnStatus[chn].pan = midiChnStatus[midiCh].pan;
								}
							}
						}
						break;

					case MIDIEvents::MIDICC_DataEntry_Coarse:
						midiChnStatus[midiCh].SetRPN(data2);
						break;

					case MIDIEvents::MIDICC_Volume_Coarse:
						midiChnStatus[midiCh].volume = (uint8)(CDLSBank::DLSMidiVolumeToLinear(data2) >> 9);
						for(auto chn : midiChnStatus[midiCh].noteOn)
						{
							if(chn != CHANNELINDEX_INVALID)
							{
								EnterMIDIVolume(patRow[chn], modChnStatus[chn], midiChnStatus[midiCh]);
							}
						}
						break;

					case MIDIEvents::MIDICC_Expression_Coarse:
						midiChnStatus[midiCh].expression = (uint8)(CDLSBank::DLSMidiVolumeToLinear(data2) >> 9);
						for(auto chn : midiChnStatus[midiCh].noteOn)
						{
							if(chn != CHANNELINDEX_INVALID)
							{
								EnterMIDIVolume(patRow[chn], modChnStatus[chn], midiChnStatus[midiCh]);
							}
						}
						break;

					case MIDIEvents::MIDICC_BankSelect_Coarse:
						midiChnStatus[midiCh].bank &= 0x7F;
						midiChnStatus[midiCh].bank |= (data2 << 7);
						break;

					case MIDIEvents::MIDICC_BankSelect_Fine:
						midiChnStatus[midiCh].bank &= (0x7F << 7);
						midiChnStatus[midiCh].bank |= data2;
						break;

					case MIDIEvents::MIDICC_HoldPedal_OnOff:
						midiChnStatus[midiCh].sustain = (data2 >= 0x40);
						if(data2 < 0x40)
						{
							// Release notes that are still being held after note-off
							for(const auto &chnState : modChnStatus)
							{
								if(chnState.midiCh == midiCh && chnState.sustained && chnState.note != NOTE_NONE)
								{
									MIDINoteOff(midiChnStatus[midiCh], modChnStatus, chnState.note - NOTE_MIN, delay, patRow, drumChns);
								}
							}
						}
						break;

					case MIDIEvents::MIDICC_DataButtonincrement:
					case MIDIEvents::MIDICC_DataButtondecrement:
						midiChnStatus[midiCh].SetRPNRelative((data1 == MIDIEvents::MIDICC_DataButtonincrement) ? 1 : -1);
						break;

					case MIDIEvents::MIDICC_NonRegisteredParameter_Fine:
					case MIDIEvents::MIDICC_NonRegisteredParameter_Coarse:
						midiChnStatus[midiCh].rpn = 0x3FFF;
						break;

					case MIDIEvents::MIDICC_RegisteredParameter_Fine:
						midiChnStatus[midiCh].rpn &= (0x7F << 7);
						midiChnStatus[midiCh].rpn |= data2;
						break;
					case MIDIEvents::MIDICC_RegisteredParameter_Coarse:
						midiChnStatus[midiCh].rpn &= 0x7F;
						midiChnStatus[midiCh].rpn |= (data2 << 7);
						break;

					case 110:
						isEMIDI = true;
						break;

					case 111:
						// Non-standard MIDI loop point. May conflict with Apogee EMIDI CCs (110/111), which is why we also check if CC 110 is ever used.
						if(data2 == 0 && !isEMIDI)
						{
							Order().SetRestartPos(ord);
							restartRow = row;
						}
						break;

					case 118:
						// EMIDI Global Loop Start
						isEMIDI = true;
						isEMIDILoop = false;
						Order().SetRestartPos(ord);
						restartRow = row;
						break;

					case 119:
						// EMIDI Global Loop End
						if(data2 == 0x7F)
						{
							isEMIDILoop = true;
							isEMIDI = true;
							std::tie(loopEndOrd, loopEndRow, std::ignore) = ModPositionFromTick(tick, 1);
						}
						break;

					case MIDIEvents::MIDICC_AllControllersOff:
						midiChnStatus[midiCh].ResetAllControllers();
						break;

						// Bn.78.00: All Sound Off (GS)
						// Bn.7B.00: All Notes Off (GM)
					case MIDIEvents::MIDICC_AllSoundOff:
					case MIDIEvents::MIDICC_AllNotesOff:
						// All Notes Off
						midiChnStatus[midiCh].sustain = false;
						for(uint8 note = 0; note < 128; note++)
						{
							MIDINoteOff(midiChnStatus[midiCh], modChnStatus, note, delay, patRow, drumChns);
						}
						break;
					case MIDIEvents::MIDICC_MonoOperation:
						if(data2 == 0)
						{
							midiChnStatus[midiCh].monoMode = true;
						}
						break;
					case MIDIEvents::MIDICC_PolyOperation:
						if(data2 == 0)
						{
							midiChnStatus[midiCh].monoMode = false;
						}
						break;
					}
				}
				break;
			case 0xC0: // Program Change
				midiChnStatus[midiCh].program = data1 & 0x7F;
				break;
			case 0xD0: // Channel aftertouch
				break;
			case 0xE0: // Pitch bend
				midiChnStatus[midiCh].SetPitchbend(data1 | (track.ReadUint8() << 7));
				break;
			case 0xF0: // General / Immediate
				switch(command & 0x0F)
				{
				case MIDIEvents::sysExStart: // SysEx
				case MIDIEvents::sysExEnd: // SysEx (continued)
					{
						uint32 len;
						track.ReadVarInt(len);
						FileReader sysex = track.ReadChunk(len);
						if(midiCh == MIDIEvents::sysExEnd)
							break;

						if(sysex.ReadMagic("\x7F\x7F\x04\x01"))
						{
							// Master volume
							uint8 volumeRaw[2];
							sysex.ReadArray(volumeRaw);
							uint16 globalVol = volumeRaw[0] | (volumeRaw[1] << 7);
							if(tick == 0)
							{
								m_nDefaultGlobalVolume = Util::muldivr_unsigned(globalVol, MAX_GLOBAL_VOLUME, 16383);
							} else
							{
								patRow[globalVolChannel].command = CMD_GLOBALVOLUME;
								patRow[globalVolChannel].param = static_cast<ModCommand::PARAM>(Util::muldivr_unsigned(globalVol, 128, 16383));
							}
						} else
						{
							uint8 xg[7];
							sysex.ReadArray(xg);
							if(!memcmp(xg, "\x43\x10\x4C\x00\x00\x7E\x00", 7))
							{
								// XG System On
								isXG = true;
							} else if(!memcmp(xg, "\x43\x10\x4C\x00\x00\x06", 6))
							{
								// XG Master Transpose
								masterTranspose = static_cast<int8>(xg[6]) - 64;
							} else if(!memcmp(xg, "\x41\x10\x42\x12\x40", 5) && (xg[5] & 0xF0) == 0x10 && xg[6] == 0x15)
							{
								// GS Drum Kit
								uint8 chn = xg[5] & 0x0F;
								if(chn == 0)
									chn = 9;
								else if(chn < 10)
									chn--;
								drumChns.set(chn, sysex.ReadUint8() != 0);
							}
						}
					}
					break;
				case MIDIEvents::sysQuarterFrame:
					track.Skip(1);
					break;
				case MIDIEvents::sysPositionPointer:
					track.Skip(2);
					break;
				case MIDIEvents::sysSongSelect:
					track.Skip(1);
					break;
				case MIDIEvents::sysTuneRequest:
				case MIDIEvents::sysMIDIClock:
				case MIDIEvents::sysMIDITick:
				case MIDIEvents::sysStart:
				case MIDIEvents::sysContinue:
				case MIDIEvents::sysStop:
				case MIDIEvents::sysActiveSense:
				case MIDIEvents::sysReset:
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
		}

		// Pitch bend any channels that haven't reached their target yet
		// TODO: This is currently not called on any rows without events!
		for(size_t chn = 0; chn < modChnStatus.size(); chn++)
		{
			ModChannelState &chnState = modChnStatus[chn];
			ModCommand &m = patRow[chn];
			uint8 midiCh = chnState.midiCh;
			if(chnState.note == NOTE_NONE || m.command == CMD_S3MCMDEX || m.command == CMD_DELAYCUT || midiCh == ModChannelState::NOMIDI)
				continue;

			int32 diff = midiChnStatus[midiCh].pitchbendMod - chnState.porta;
			if(diff == 0)
				continue;

			if(m.command == CMD_PORTAMENTODOWN || m.command == CMD_PORTAMENTOUP)
			{
				// First, undo the effect of an existing portamento command
				int32 porta = 0;
				if(m.param < 0xE0)
					porta = m.param * 4 * (ticksPerRow - 1);
				else if(m.param < 0xF0)
					porta = (m.param & 0x0F);
				else
					porta = (m.param & 0x0F) * 4;

				if(m.command == CMD_PORTAMENTODOWN)
					porta = -porta;

				diff += porta;
				chnState.porta -= porta;

				if(diff == 0)
				{
					m.command = CMD_NONE;
					continue;
				}
			}

			m.command = (diff < 0) ? CMD_PORTAMENTODOWN : CMD_PORTAMENTOUP;
			int32 absDiff = std::abs(diff);
			int32 realDiff = 0;
			if(absDiff < 16)
			{
				// Extra-fine slides can do this.
				m.param = 0xE0 | static_cast<uint8>(absDiff);
				realDiff = absDiff;
			} else if(absDiff < 64)
			{
				// Fine slides can do this.
				absDiff = std::min((absDiff + 3) / 4, 0x0F);
				m.param = 0xF0 | static_cast<uint8>(absDiff);
				realDiff = absDiff * 4;
			} else
			{
				// Need a normal slide.
				absDiff /= 4 * (ticksPerRow - 1);
				LimitMax(absDiff, 0xDF);
				m.param = static_cast<ModCommand::PARAM>(absDiff);
				realDiff = absDiff * 4 * (ticksPerRow - 1);
			}
			chnState.porta += realDiff * mpt::signum(diff);
		}

		tick_t delta = 0;
		if(track.ReadVarInt(delta) && track.CanRead(1))
		{
			tracks[t].nextEvent += delta;
		} else
		{
			finishedTracks++;
			tracks[t].nextEvent = Util::MaxValueOfType(delta);
			tracks[t].finished = true;
			// Add another sub-song for type-2 files
			if(isType2 && finishedTracks < numTracks)
			{
				if(Order.AddSequence() == SEQUENCEINDEX_INVALID)
					break;
				Order().clear();
			}
		}
	}

	if(isEMIDILoop)
		isEMIDI = false;

	if(isEMIDI)
	{
		Order().SetRestartPos(0);
	}

	if(loopEndOrd == ORDERINDEX_INVALID)
		loopEndOrd = lastOrd;
	if(loopEndRow == ROWINDEX_INVALID)
		loopEndRow = lastRow;

	if(Order().IsValidPat(loopEndOrd))
	{
		PATTERNINDEX lastPat = Order()[loopEndOrd];
		if(loopEndOrd == lastOrd)
			Patterns[lastPat].Resize(loopEndRow + 1);
		if(restartRow != ROWINDEX_INVALID && !isEMIDI)
		{
			Patterns[lastPat].WriteEffect(EffectWriter(CMD_PATTERNBREAK, mpt::saturate_cast<ModCommand::PARAM>(restartRow)).Row(loopEndRow));
			if(ORDERINDEX restartPos = Order().GetRestartPos(); loopEndOrd != lastOrd || restartPos <= std::numeric_limits<ModCommand::PARAM>::max())
				Patterns[lastPat].WriteEffect(EffectWriter(CMD_POSITIONJUMP, mpt::saturate_cast<ModCommand::PARAM>(restartPos)).Row(loopEndRow));
		}
	}
	Order.SetSequence(0);

	std::vector<CHANNELINDEX> channels;
	channels.reserve(GetNumChannels());
	for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
	{
		if(modChnStatus[i].midiCh != ModChannelState::NOMIDI
#ifdef MODPLUG_TRACKER
			|| (GetpModDoc() != nullptr && !GetpModDoc()->IsChannelUnused(i))
#endif // MODPLUG_TRACKER
			)
		{
			channels.push_back(i);
			if(modChnStatus[i].midiCh != ModChannelState::NOMIDI)
				ChnSettings[i].szName = MPT_AFORMAT("MIDI Ch {}")(1 + modChnStatus[i].midiCh);
			else if(i == tempoChannel)
				ChnSettings[i].szName = "Tempo";
			else if(i == globalVolChannel)
				ChnSettings[i].szName = "Global Volume";
		}
	}
	if(channels.empty())
		return false;

#ifdef MODPLUG_TRACKER
	if(GetpModDoc() != nullptr)
	{
		// Keep MIDI channels in patterns neatly grouped
		std::sort(channels.begin(), channels.end(), [&modChnStatus] (CHANNELINDEX c1, CHANNELINDEX c2)
		{
			if(modChnStatus[c1].midiCh == modChnStatus[c2].midiCh)
				return c1 < c2;
			return modChnStatus[c1].midiCh < modChnStatus[c2].midiCh;
		});
		GetpModDoc()->ReArrangeChannels(channels, false);
		GetpModDoc()->m_ShowSavedialog = true;
	}

	std::unique_ptr<CDLSBank> cachedBank, embeddedBank;
	FileReader *bankFile = nullptr;

	if(CDLSBank::IsDLSBank(file))
	{
		// Soundfont embedded in MIDI file
		embeddedBank = std::make_unique<CDLSBank>();
		embeddedBank->Open(file);
		bankFile = &file;
	} else
	{
		// Soundfont with same name as MIDI file
		for(const auto &ext : { P_(".sf2"), P_(".sf3"), P_(".sf4"), P_(".sbk"), P_(".dls") })
		{
			mpt::PathString filename = file.GetOptionalFileName().value_or(P_("")).ReplaceExtension(ext);
			if(mpt::native_fs{}.is_file(filename))
			{
				embeddedBank = std::make_unique<CDLSBank>();
				if(embeddedBank->Open(filename))
					break;
			}
		}
	}
	ChangeModTypeTo(MOD_TYPE_MPT);
	const MidiLibrary &midiLib = CTrackApp::GetMidiLibrary();
	mpt::PathString cachedBankName;
	// Load Instruments
	for (INSTRUMENTINDEX ins = 1; ins <= m_nInstruments; ins++) if (Instruments[ins])
	{
		ModInstrument *pIns = Instruments[ins];
		uint32 midiCode = 0;
		if(pIns->nMidiChannel == MIDI_DRUMCHANNEL)
			midiCode = 0x80 | (pIns->nMidiDrumKey & 0x7F);
		else if(pIns->nMidiProgram)
			midiCode = (pIns->nMidiProgram - 1) & 0x7F;

		if(embeddedBank && embeddedBank->FindAndExtract(*this, ins, midiCode >= 0x80, bankFile))
		{
			continue;
		}

		const mpt::PathString &midiMapName = midiLib[midiCode].value_or(P_(""));
		if(!midiMapName.empty())
		{
			// Load from DLS/SF2 Bank
			if(CDLSBank::IsDLSBank(midiMapName))
			{
				CDLSBank *dlsBank = nullptr;
				if(cachedBank != nullptr && !mpt::PathCompareNoCase(cachedBankName, midiMapName))
				{
					dlsBank = cachedBank.get();
				} else
				{
					cachedBank = std::make_unique<CDLSBank>();
					cachedBankName = midiMapName;
					if(cachedBank->Open(midiMapName)) dlsBank = cachedBank.get();
				}
				if(dlsBank)
				{
					dlsBank->FindAndExtract(*this, ins, midiCode >= 0x80);
				}
			} else
			{
				// Load from Instrument or Sample file
				mpt::IO::InputFile f(midiMapName, SettingCacheCompleteFileBeforeLoading());
				if(f.IsValid())
				{
					FileReader insFile = GetFileReader(f);
					if(ReadInstrumentFromFile(ins, insFile, false))
					{
						mpt::PathString filename = midiMapName.GetFilename();
						pIns = Instruments[ins];
						if(!pIns->filename[0]) pIns->filename = filename.ToLocale();
						if(!pIns->name[0])
						{
							if(midiCode < 0x80)
							{
								pIns->name = szMidiProgramNames[midiCode];
							} else
							{
								uint32 key = midiCode & 0x7F;
								if((key >= 24) && (key < 24 + std::size(szMidiPercussionNames)))
									pIns->name = szMidiPercussionNames[key - 24];
							}
						}
					}
				}
			}
		}
	}
#endif // MODPLUG_TRACKER
	return true;
}


#else // !MODPLUG_TRACKER && !MPT_FUZZ_TRACKER

bool CSoundFile::ReadMID(FileReader &/*file*/, ModLoadingFlags /*loadFlags*/)
{
	return false;
}

#endif

OPENMPT_NAMESPACE_END
