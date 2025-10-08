/*
 * PlayState.cpp
 * -------------
 * Purpose: This class represents all of the playback state of a module.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "PlayState.h"
#include "MIDIMacros.h"
#include "Mixer.h"
#include "Sndfile.h"


OPENMPT_NAMESPACE_BEGIN


PlayState::PlayState()
{
	Chn.fill({});
	m_midiMacroScratchSpace.reserve(kMacroLength);  // Note: If macros ever become variable-length, the scratch space needs to be at least one byte longer than the longest macro in the file for end-of-SysEx insertion to stay allocation-free in the mixer!
}


void PlayState::ResetGlobalVolumeRamping() noexcept
{
	m_lHighResRampingGlobalVolume = m_nGlobalVolume << VOLUMERAMPPRECISION;
	m_nGlobalVolumeDestination = m_nGlobalVolume;
	m_nSamplesToGlobalVolRampDest = 0;
	m_nGlobalVolumeRampAmount = 0;
}


void PlayState::UpdateTimeSignature(const CSoundFile &sndFile) noexcept
{
	if(!sndFile.Patterns.IsValidIndex(m_nPattern) || !sndFile.Patterns[m_nPattern].GetOverrideSignature())
	{
		m_nCurrentRowsPerBeat = sndFile.m_nDefaultRowsPerBeat;
		m_nCurrentRowsPerMeasure = sndFile.m_nDefaultRowsPerMeasure;
	} else
	{
		m_nCurrentRowsPerBeat = sndFile.Patterns[m_nPattern].GetRowsPerBeat();
		m_nCurrentRowsPerMeasure = sndFile.Patterns[m_nPattern].GetRowsPerMeasure();
	}
}


void PlayState::UpdatePPQ(bool patternTransition) noexcept
{
	ROWINDEX rpm = m_nCurrentRowsPerMeasure ? m_nCurrentRowsPerMeasure : DEFAULT_ROWS_PER_MEASURE;
	ROWINDEX rpb = m_nCurrentRowsPerBeat ? m_nCurrentRowsPerBeat : DEFAULT_ROWS_PER_BEAT;
	if(m_lTotalSampleCount > 0 && (patternTransition || !(m_nRow % rpm)))
	{
		// Pattern end = end of measure, so round up PPQ to the next full measure
		m_ppqPosBeat += (rpm + (rpb - 1)) / rpb;
		m_ppqPosFract = 0;
	}
}


mpt::span<ModChannel> PlayState::PatternChannels(const CSoundFile &sndFile) noexcept
{
	return mpt::as_span(Chn).subspan(0, std::min(Chn.size(), static_cast<size_t>(sndFile.GetNumChannels())));
}


mpt::span<ModChannel> PlayState::BackgroundChannels(const CSoundFile &sndFile) noexcept
{
	return mpt::as_span(Chn).subspan(std::min(Chn.size(), static_cast<size_t>(sndFile.GetNumChannels())));
}


OPENMPT_NAMESPACE_END
