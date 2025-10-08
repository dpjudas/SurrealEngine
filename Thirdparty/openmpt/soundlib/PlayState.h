/*
 * PlayState.h
 * -----------
 * Purpose: This class represents all of the playback state of a module.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include "ModChannel.h"
#include "Snd_defs.h"

#include <map>
#include <optional>
#include <vector>

OPENMPT_NAMESPACE_BEGIN

struct PlayState
{
	friend class CSoundFile;

public:
	samplecount_t m_lTotalSampleCount = 0;  // Total number of rendered samples
protected:
	samplecount_t m_nBufferCount = 0;  // Remaining number samples to render for this tick
	double m_dBufferDiff = 0.0;        // Modern tempo rounding error compensation

public:
	double m_ppqPosFract = 0.0;  // Fractional PPQ position within current measure
	uint32 m_ppqPosBeat = 0;     // PPQ position of the last start of measure
	uint32 m_nTickCount = 0;     // Current tick being processed
protected:
	uint32 m_nPatternDelay = 0;  // Pattern delay (rows)
	uint32 m_nFrameDelay = 0;    // Fine pattern delay (ticks)
public:
	uint32 m_nSamplesPerTick = 0;
	ROWINDEX m_nCurrentRowsPerBeat = 0;     // Current time signature
	ROWINDEX m_nCurrentRowsPerMeasure = 0;  // Current time signature
	uint32 m_nMusicSpeed = 0;               // Current speed
	TEMPO m_nMusicTempo;                    // Current tempo

	// Playback position
	ROWINDEX m_nRow = 0;      // Current row being processed
	ROWINDEX m_nNextRow = 0;  // Next row to process
protected:
	ROWINDEX m_nextPatStartRow = 0;  // For FT2's E60 bug
	ROWINDEX m_breakRow = 0;         // Candidate target row for pattern break
	ROWINDEX m_patLoopRow = 0;       // Candidate target row for pattern loop
	ORDERINDEX m_posJump = 0;        // Candidate target order for position jump

public:
	PATTERNINDEX m_nPattern = 0;                     // Current pattern being processed
	ORDERINDEX m_nCurrentOrder = 0;                  // Current order being processed
	ORDERINDEX m_nNextOrder = 0;                     // Next order to process
	ORDERINDEX m_nSeqOverride = ORDERINDEX_INVALID;  // Queued order to be processed next, regardless of what order would normally follow
	OrderTransitionMode m_seqOverrideMode = OrderTransitionMode::AtPatternEnd;

	// Global volume
public:
	int32 m_nGlobalVolume = MAX_GLOBAL_VOLUME;  // Current global volume (0...MAX_GLOBAL_VOLUME)
protected:
	// Global volume ramping
	int32 m_nSamplesToGlobalVolRampDest = 0, m_nGlobalVolumeRampAmount = 0;
	int32 m_nGlobalVolumeDestination = 0, m_lHighResRampingGlobalVolume = 0;

public:
	FlagSet<PlayFlags> m_flags = SONG_POSITIONCHANGED;

	std::array<CHANNELINDEX, MAX_CHANNELS> ChnMix;  // Index of channels in Chn to be actually mixed
	std::array<ModChannel, MAX_CHANNELS> Chn;       // Mixing channels... First m_nChannels channels are directly mapped to pattern channels (i.e. they are never NNA channels)!
	GlobalScriptState m_globalScriptState;

	struct MIDIMacroEvaluationResults
	{
		std::map<PLUGINDEX, float> pluginDryWetRatio;
		std::map<std::pair<PLUGINDEX, PlugParamIndex>, PlugParamValue> pluginParameter;
	};

	std::vector<uint8> m_midiMacroScratchSpace;
	std::optional<MIDIMacroEvaluationResults> m_midiMacroEvaluationResults;

public:
	PlayState();

	void ResetGlobalVolumeRamping() noexcept;

	void UpdateTimeSignature(const CSoundFile &sndFile) noexcept;
	void UpdatePPQ(bool patternTransition) noexcept;

	constexpr uint32 TicksOnRow() const noexcept
	{
		return (m_nMusicSpeed + m_nFrameDelay) * std::max(m_nPatternDelay, uint32(1));
	}

	mpt::span<ModChannel> PatternChannels(const CSoundFile &sndFile) noexcept;
	mpt::span<const ModChannel> PatternChannels(const CSoundFile &sndFile) const noexcept
	{
		return const_cast<PlayState *>(this)->PatternChannels(sndFile);
	}

	mpt::span<ModChannel> BackgroundChannels(const CSoundFile &sndFile) noexcept;
	mpt::span<const ModChannel> BackgroundChannels(const CSoundFile &sndFile) const noexcept
	{
		return const_cast<PlayState *>(this)->PatternChannels(sndFile);
	}
};


OPENMPT_NAMESPACE_END
