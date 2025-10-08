/*
 * ModChannel.h
 * ------------
 * Purpose: The ModChannel struct represents the state of one mixer channel.
 *          ModChannelSettings represents the default settings of one pattern channel.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "InstrumentSynth.h"
#include "modcommand.h"
#include "Paula.h"
#include "tuningbase.h"

#include <bitset>

OPENMPT_NAMESPACE_BEGIN

class CSoundFile;
struct ModSample;
struct ModInstrument;

// Mix Channel Struct
struct ModChannel
{
	// Envelope playback info
	struct EnvInfo
	{
		uint32 nEnvPosition = 0;
		int16 nEnvValueAtReleaseJump = NOT_YET_RELEASED;
		FlagSet<EnvelopeFlags> flags;

		void Reset()
		{
			nEnvPosition = 0;
			nEnvValueAtReleaseJump = NOT_YET_RELEASED;
		}
	};

	struct AutoSlideStatus
	{
		bool AnyActive() const noexcept { return m_set.any(); }
		bool IsActive(AutoSlideCommand cmd) const noexcept { return m_set[static_cast<size_t>(cmd)]; }
		void SetActive(AutoSlideCommand cmd, bool active = true) noexcept { m_set[static_cast<size_t>(cmd)] = active; }
		void Reset() noexcept { m_set.reset(); }

		bool AnyPitchSlideActive() const noexcept
		{
			return IsActive(AutoSlideCommand::TonePortamento)
				|| IsActive(AutoSlideCommand::PortamentoUp) || IsActive(AutoSlideCommand::PortamentoDown)
				|| IsActive(AutoSlideCommand::FinePortamentoUp) || IsActive(AutoSlideCommand::FinePortamentoDown)
				|| IsActive(AutoSlideCommand::PortamentoFC);
		}
	private:
		std::bitset<static_cast<size_t>(AutoSlideCommand::NumCommands)> m_set;
	};

	// Information used in the mixer (should be kept tight for better caching)
	SamplePosition position;     // Current play position (fixed point)
	SamplePosition increment;    // Sample speed relative to mixing frequency (fixed point)
	const void *pCurrentSample;  // Currently playing sample (nullptr if no sample is playing)
	int32 leftVol;               // 0...4096 (12 bits, since 16 bits + 12 bits = 28 bits = 0dB in integer mixer, see MIXING_ATTENUATION)
	int32 rightVol;              // Ditto
	int32 leftRamp;              // Ramping delta, 20.12 fixed point (see VOLUMERAMPPRECISION)
	int32 rightRamp;             // Ditto
	int32 rampLeftVol;           // Current ramping volume, 20.12 fixed point (see VOLUMERAMPPRECISION)
	int32 rampRightVol;          // Ditto
	mixsample_t nFilter_Y[2][2]; // Filter memory - two history items per sample channel
	mixsample_t nFilter_A0, nFilter_B0, nFilter_B1; // Filter coeffs
	mixsample_t nFilter_HP;

	SmpLength nLength;
	SmpLength nLoopStart;
	SmpLength nLoopEnd;
	FlagSet<ChannelFlags> dwFlags;
	mixsample_t nROfs, nLOfs;
	uint32 nRampLength;

	const ModSample *pModSample;  // Currently assigned sample slot (may already be stopped)
	Paula::State paulaState;
	InstrumentSynth::States synthState;

	// Information not used in the mixer
	const ModInstrument *pModInstrument;  // Currently assigned instrument slot
	SmpLength prevNoteOffset;             // Offset for instrument-less notes for ProTracker/ScreamTracker
	SmpLength oldOffset;                  // Offset command memory
	FlagSet<ChannelFlags> dwOldFlags;     // Flags from previous tick
	int32 newLeftVol, newRightVol;
	int32 nRealVolume, nRealPan;
	int32 nVolume, nPan, nFadeOutVol;
	int32 nPeriod;  // Frequency in Hz if CSoundFile::PeriodsAreFrequencies() or using custom tuning, 4x Amiga periods otherwise
	int32 nC5Speed, nPortamentoDest;
	int32 cachedPeriod, glissandoPeriod;
	int32 nCalcVolume;                 // Calculated channel volume, 14-Bit (without global volume, pre-amp etc applied) - for MIDI macros
	EnvInfo VolEnv, PanEnv, PitchEnv;  // Envelope playback info
	int32 nAutoVibDepth;
	uint32 nEFxOffset;  // Offset memory for Invert Loop (EFx, .MOD only)
	ROWINDEX nPatternLoop;
	AutoSlideStatus autoSlide;
	uint16 portamentoSlide;
	int16 nFineTune;
	int16 microTuning;  // Micro-tuning / MIDI pitch wheel command
	int16 nVolSwing, nPanSwing;
	int16 nCutSwing, nResSwing;
	uint16 volSlideDownRemain, volSlideDownTotal;
	union
	{
		uint16 nRestorePanOnNewNote;  // If > 0, nPan should be set to nRestorePanOnNewNote - 1 on new note. Used to recover from pan swing and IT sample / instrument panning. High bit set = surround
		uint16 nnaChannelAge;         // If channel is moved to background (NNA), this counts up how old it is
	};
	uint16 nnaGeneration;         // For PlaybackTest implementation
	CHANNELINDEX nMasterChn;
	SAMPLEINDEX swapSampleIndex;  // Sample to swap to when current sample (loop) has finished playing
	ModCommand rowCommand;
	// 8-bit members
	uint8 nGlobalVol;  // Channel volume (CV in ITTECH.TXT) 0...64
	uint8 nInsVol;     // Sample / Instrument volume (SV * IV in ITTECH.TXT) 0...64
	int8 nTranspose;
	ResamplingMode resamplingMode;
	uint8 nRestoreResonanceOnNewNote;  // See nRestorePanOnNewNote
	uint8 nRestoreCutoffOnNewNote;     // ditto
	uint8 nNote;
	NewNoteAction nNNA;
	uint8 nLastNote;  // Last note, ignoring note offs and cuts - for MIDI macros
	uint8 nArpeggioLastNote, lastMidiNoteWithoutArp;  // For plugin arpeggio and NNA handling
	uint8 nNewNote, nNewIns, nOldIns, nCommand, nArpeggio;
	uint8 nRetrigParam, nRetrigCount;
	uint8 nOldVolumeSlide, nOldFineVolUpDown;
	uint8 nOldPortaUp, nOldPortaDown, nOldFinePortaUpDown, nOldExtraFinePortaUpDown;
	uint8 nOldPanSlide, nOldChnVolSlide;
	uint8 nOldGlobalVolSlide;
	uint8 nAutoVibPos, nVibratoPos, nTremoloPos, nPanbrelloPos;
	uint8 nVibratoType, nVibratoSpeed, nVibratoDepth;
	uint8 nTremoloType, nTremoloSpeed, nTremoloDepth;
	uint8 nPanbrelloType, nPanbrelloSpeed, nPanbrelloDepth;
	int8  nPanbrelloOffset, nPanbrelloRandomMemory;
	uint8 nOldCmdEx, nOldVolParam, nOldTempo;
	uint8 nOldHiOffset;
	uint8 nCutOff, nResonance;
	uint8 nTremorCount, nTremorParam;
	uint8 nPatternLoopCount;
	uint8 nLeftVU, nRightVU;
	uint8 nActiveMacro;
	uint8 volSlideDownStart;
	FilterMode nFilterMode;
	uint8 nEFxSpeed, nEFxDelay;              // memory for Invert Loop (EFx, .MOD only)
	uint8 noteSlideParam, noteSlideCounter;  // IMF / PTM Note Slide
	uint8 lastZxxParam;                      // Memory for \xx slides
	bool isFirstTick : 1;                    // Execute tick-0 effects on this channel? (condition differs between formats due to Pattern Delay commands)
	bool triggerNote : 1;                    // Trigger note on this tick on this channel if there is one?
	bool isPreviewNote : 1;                  // Notes preview in editor
	bool isPaused : 1;                       // Don't mix or increment channel position, but keep the note alive
	bool portaTargetReached : 1;             // Tone portamento is finished
	bool fcPortaTick : 1;                    // Future Composer portamento state

	//-->Variables used to make user-definable tuning modes work with pattern effects.
	//If true, freq should be recalculated in ReadNote() on first tick.
	//Currently used only for vibrato things - using in other context might be
	//problematic.
	bool m_ReCalculateFreqOnFirstTick : 1;

	//To tell whether to calculate frequency.
	bool m_CalculateFreq : 1;

	int32 m_PortamentoFineSteps, m_PortamentoTickSlide;

	//NOTE_PCs memory.
	float m_plugParamValueStep, m_plugParamTargetValue;
	uint16 m_RowPlugParam;
	PLUGINDEX m_RowPlug;

	// Get a reference to a specific envelope of this channel
	const EnvInfo &GetEnvelope(EnvelopeType envType) const
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

	EnvInfo &GetEnvelope(EnvelopeType envType)
	{
		return const_cast<EnvInfo &>(static_cast<const ModChannel *>(this)->GetEnvelope(envType));
	}

	void ResetEnvelopes()
	{
		VolEnv.Reset();
		PanEnv.Reset();
		PitchEnv.Reset();
	}

	enum ResetFlags
	{
		resetChannelSettings = 1,  // Reload initial channel settings
		resetSetPosBasic     = 2,  // Reset basic runtime channel attributes
		resetSetPosAdvanced  = 4,  // Reset more runtime channel attributes
		resetSetPosFull      = resetSetPosBasic | resetSetPosAdvanced | resetChannelSettings,  // Reset all runtime channel attributes
		resetTotal           = resetSetPosFull,
	};

	void Reset(ResetFlags resetMask, const CSoundFile &sndFile, CHANNELINDEX sourceChannel, ChannelFlags muteFlag);
	void Stop();

	bool IsSamplePlaying() const noexcept { return !increment.IsZero(); }

	uint32 GetVSTVolume() const noexcept;

	ModCommand::NOTE GetPluginNote(bool ignoreArpeggio = false) const noexcept;

	// Check if the channel has a valid MIDI output. A return value of true implies that pModInstrument != nullptr.
	bool HasMIDIOutput() const noexcept;
	// Check if the channel uses custom tuning. A return value of true implies that pModInstrument != nullptr.
	bool HasCustomTuning() const noexcept;

	// Check if currently processed loop is a sustain loop. pModSample is not checked for validity!
	bool InSustainLoop() const noexcept;

	void UpdateInstrumentVolume(const ModSample *smp, const ModInstrument *ins);

	void SetInstrumentPan(int32 pan, const CSoundFile &sndFile);
	void RestorePanAndFilter();

	void RecalcTuningFreq(Tuning::RATIOTYPE vibratoFactor, Tuning::NOTEINDEXTYPE arpeggioSteps, const CSoundFile &sndFile);

	// IT command S73-S7E
	void InstrumentControl(uint8 param, const CSoundFile &sndFile);
	// Volume command :xx
	void PlayControl(uint8 param);

	int32 GetMIDIPitchBend() const noexcept { return (static_cast<int32>(microTuning) + 0x8000) / 4; }
	void SetMIDIPitchBend(const uint8 high, const uint8 low) noexcept
	{
		microTuning = static_cast<int16>(((high << 9) | (low << 2)) - 0x8000);
	}
};


// Default pattern channel settings
struct ModChannelSettings
{
#ifdef MODPLUG_TRACKER
	static constexpr uint32 INVALID_COLOR = 0xFFFFFFFF;
	uint32 color = INVALID_COLOR;   // For pattern editor
#endif                              // MODPLUG_TRACKER
	FlagSet<ChannelFlags> dwFlags;  // Channel flags
	uint16 nPan = 128;              // Initial pan (0...256)
	uint8 nVolume = 64;             // Initial channel volume (0...64)
	PLUGINDEX nMixPlugin = 0;       // Assigned plugin

	mpt::charbuf<MAX_CHANNELNAME> szName;  // Channel name
};

OPENMPT_NAMESPACE_END
