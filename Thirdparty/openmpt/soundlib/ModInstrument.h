/*
 * ModInstrument.h
 * ---------------
 * Purpose: Module Instrument header class and helpers
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "InstrumentSynth.h"
#include "modcommand.h"
#include "tuningbase.h"
#include "Snd_defs.h"
#include "openmpt/base/FlagSet.hpp"
#include "../common/misc_util.h"

#include <map>
#include <set>

OPENMPT_NAMESPACE_BEGIN

struct ModChannel;

// Instrument Nodes
struct EnvelopeNode
{
	using tick_t = uint16;
	using value_t = uint8;

	tick_t tick = 0;   // Envelope node position (x axis)
	value_t value = 0; // Envelope node value (y axis)

	constexpr EnvelopeNode() = default;
	constexpr EnvelopeNode(tick_t tick, value_t value) : tick{tick}, value{value} { }

	bool operator== (const EnvelopeNode &other) const { return tick == other.tick && value == other.value; }
};

// Instrument Envelopes
struct InstrumentEnvelope : public std::vector<EnvelopeNode>
{
	FlagSet<EnvelopeFlags> dwFlags; // Envelope flags
	uint8 nLoopStart = 0;           // Loop start node
	uint8 nLoopEnd = 0;             // Loop end node
	uint8 nSustainStart = 0;        // Sustain start node
	uint8 nSustainEnd = 0;          // Sustain end node
	uint8 nReleaseNode = ENV_RELEASE_NODE_UNSET; // Release node

	// Convert envelope data between various formats.
	void Convert(MODTYPE fromType, MODTYPE toType);

	// Get envelope value at a given tick. Assumes that the envelope data is in rage [0, rangeIn],
	// returns value in range [0, rangeOut].
	int32 GetValueFromPosition(int position, int32 rangeOut, int32 rangeIn = ENVELOPE_MAX) const;

	// Ensure that ticks are ordered in increasing order and values are within the allowed range.
	void Sanitize(uint8 maxValue = ENVELOPE_MAX);

	uint32 size() const { return static_cast<uint32>(std::vector<EnvelopeNode>::size()); }

	uint8 LastPoint() const { return static_cast<uint8>(std::max(size(), uint32(1)) - 1); }

	using std::vector<EnvelopeNode>::push_back;
	void push_back(EnvelopeNode::tick_t tick, EnvelopeNode::value_t value) { emplace_back(tick, value); }
};

// Instrument Struct
struct ModInstrument
{
	uint32 nFadeOut = 256;   // Instrument fadeout speed
	uint32 nGlobalVol = 64;  // Global volume (0...64, all sample volumes are multiplied with this - TODO: This is 0...128 in Impulse Tracker)
	uint32 nPan = 32 * 4;    // Default pan (0...256), if the appropriate flag is set. Sample panning overrides instrument panning.

	uint16 nVolRampUp = 0;  // Default sample ramping up, 0 = use global default

	ResamplingMode resampling = SRCMODE_DEFAULT;  // Resampling mode

	FlagSet<InstrumentFlags> dwFlags;                         // Instrument flags
	NewNoteAction nNNA = NewNoteAction::NoteCut;              // New note action
	DuplicateCheckType nDCT = DuplicateCheckType::None;       // Duplicate check type (i.e. which condition will trigger the duplicate note action)
	DuplicateNoteAction nDNA = DuplicateNoteAction::NoteCut;  // Duplicate note action

	uint8 nPanSwing = 0;  // Random panning factor (0...64)
	uint8 nVolSwing = 0;  // Random volume factor (0...100)

	uint8 nIFC = 0;                                 // Default filter cutoff (0...127). Used if the high bit is set
	uint8 nIFR = 0;                                 // Default filter resonance (0...127). Used if the high bit is set
	uint8 nCutSwing = 0;                            // Random cutoff factor (0...64)
	uint8 nResSwing = 0;                            // Random resonance factor (0...64)
	FilterMode filterMode = FilterMode::Unchanged;  // Default filter mode

	int8 nPPS = 0;                         // Pitch/Pan separation (i.e. how wide the panning spreads, -32...32)
	uint8 nPPC = NOTE_MIDDLEC - NOTE_MIN;  // Pitch/Pan centre (zero-based)

	uint16 wMidiBank = 0;    // MIDI Bank (1...16384). 0 = Don't send.
	uint8 nMidiProgram = 0;  // MIDI Program (1...128). 0 = Don't send.
	uint8 nMidiChannel = 0;  // MIDI Channel (1...16). 0 = Don't send. 17 = Mapped (Send to tracker channel modulo 16).
	uint8 nMidiDrumKey = 0;  // Drum set note mapping (currently only used by the .MID loader)
	int8 midiPWD = 2;        // MIDI Pitch Wheel Depth and CMD_FINETUNE depth in semitones
	PLUGINDEX nMixPlug = 0;  // Plugin assigned to this instrument (0 = no plugin, 1 = first plugin)

	PlugVelocityHandling pluginVelocityHandling = PLUGIN_VELOCITYHANDLING_CHANNEL;  // How to deal with plugin velocity
	PlugVolumeHandling pluginVolumeHandling = PLUGIN_VOLUMEHANDLING_IGNORE;         // How to deal with plugin volume

	TEMPO pitchToTempoLock;      // BPM at which the samples assigned to this instrument loop correctly (0 = unset)
	CTuning *pTuning = nullptr;  // sample tuning assigned to this instrument
	InstrumentSynth synth;       // Synth scripts for this instrument

	InstrumentEnvelope VolEnv;    // Volume envelope data
	InstrumentEnvelope PanEnv;    // Panning envelope data
	InstrumentEnvelope PitchEnv;  // Pitch / filter envelope data

	std::array<uint8, 128> NoteMap;         // Note mapping, e.g. C-5 => D-5
	std::array<SAMPLEINDEX, 128> Keyboard;  // Sample mapping, e.g. C-5 => Sample 1

	mpt::charbuf<MAX_INSTRUMENTNAME> name;
	mpt::charbuf<MAX_INSTRUMENTFILENAME> filename;

	std::string GetName() const { return name; }
	std::string GetFilename() const { return filename; }

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// WHEN adding new members here, ALSO update InstrumentExtensions.cpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	MPT_CONSTEXPR20_CONTAINER_FUN explicit ModInstrument(SAMPLEINDEX sample = 0)
		: NoteMap{mpt::generate_array<uint8, 128>([](std::size_t i){ return static_cast<uint8>(NOTE_MIN + i); })}
		, Keyboard{mpt::init_array<SAMPLEINDEX, 128>(sample)}
	{
		return;
	}

	// Assign all notes to a given sample.
	MPT_CONSTEXPR20_ALGORITHM_FUN void AssignSample(SAMPLEINDEX sample)
	{
		Keyboard.fill(sample);
	}

	// Reset note mapping (i.e. every note is mapped to itself)
	MPT_CONSTEXPR20_ALGORITHM_FUN void ResetNoteMap()
	{
		std::iota(NoteMap.begin(), NoteMap.end(), static_cast<uint8>(NOTE_MIN));
	}

	// If the instrument has a non-default note mapping and can be simplified to use the default note mapping by transposing samples,
	// the list of samples that would need to be transposed and the corresponding transpose values are returned - otherwise an empty map.
	std::map<SAMPLEINDEX, int8> CanConvertToDefaultNoteMap() const;

	// Transpose entire note mapping by given number of semitones
	void Transpose(int8 amount);

	MPT_CONSTEXPRINLINE bool IsCutoffEnabled() const { return (nIFC & 0x80) != 0; }
	MPT_CONSTEXPRINLINE bool IsResonanceEnabled() const { return (nIFR & 0x80) != 0; }
	MPT_CONSTEXPRINLINE uint8 GetCutoff() const { return (nIFC & 0x7F); }
	MPT_CONSTEXPRINLINE uint8 GetResonance() const { return (nIFR & 0x7F); }
	MPT_CONSTEXPRINLINE void SetCutoff(uint8 cutoff, bool enable) { nIFC = std::min(cutoff, uint8(0x7F)) | (enable ? 0x80 : 0x00); }
	MPT_CONSTEXPRINLINE void SetResonance(uint8 resonance, bool enable) { nIFR = std::min(resonance, uint8(0x7F)) | (enable ? 0x80 : 0x00); }

	MPT_CONSTEXPRINLINE bool HasValidMIDIChannel() const { return (nMidiChannel >= 1 && nMidiChannel <= 17); }
	uint8 GetMIDIChannel(const ModChannel &channel, CHANNELINDEX chn) const;

	MPT_CONSTEXPRINLINE void SetTuning(CTuning *pT)
	{
		pTuning = pT;
	}

	// Get a reference to a specific envelope of this instrument
	MPT_CONSTEXPRINLINE const InstrumentEnvelope &GetEnvelope(EnvelopeType envType) const
	{
		switch(envType)
		{
		case ENV_VOLUME:
		default:
			return VolEnv;
		case ENV_PANNING:
			return PanEnv;
		case ENV_PITCH:
			return PitchEnv;
		}
	}

	InstrumentEnvelope &GetEnvelope(EnvelopeType envType)
	{
		return const_cast<InstrumentEnvelope &>(static_cast<const ModInstrument &>(*this).GetEnvelope(envType));
	}

	// Get a set of all samples referenced by this instrument
	std::set<SAMPLEINDEX> GetSamples() const;

	// Write sample references into a bool vector. If a sample is referenced by this instrument, true is written.
	// The caller has to initialize the vector.
	void GetSamples(std::vector<bool> &referencedSamples) const;

	// Translate instrument properties between two given formats.
	void Convert(MODTYPE fromType, MODTYPE toType);

	// Sanitize all instrument data.
	void Sanitize(MODTYPE modType);

};

OPENMPT_NAMESPACE_END
