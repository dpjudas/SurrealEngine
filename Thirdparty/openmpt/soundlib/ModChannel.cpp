/*
 * ModChannel.cpp
 * --------------
 * Purpose: The ModChannel struct represents the state of one mixer channel.
 *          ModChannelSettings represents the default settings of one pattern channel.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "ModChannel.h"
#include "Sndfile.h"
#include "tuning.h"

OPENMPT_NAMESPACE_BEGIN

void ModChannel::Reset(ResetFlags resetMask, const CSoundFile &sndFile, CHANNELINDEX sourceChannel, ChannelFlags muteFlag)
{
	// For "the ultimate beeper.mod"
	const ModSample *defaultSample = (sndFile.GetType() == MOD_TYPE_MOD && sndFile.GetSample(0).HasSampleData()) ? &sndFile.GetSample(0) : nullptr;
	if(resetMask & resetSetPosBasic)
	{
		// IT compatibility: Initial "last note memory" of channel is C-0 (so a lonely instrument number without note will play that note).
		// Test case: InitialNoteMemory.it
		nNote = nNewNote = (sndFile.m_playBehaviour[kITInitialNoteMemory] ? NOTE_MIN : NOTE_NONE);
		nArpeggioLastNote = lastMidiNoteWithoutArp = NOTE_NONE;
		nNewIns = nOldIns = 0;
		swapSampleIndex = 0;
		pModSample = defaultSample;
		pModInstrument = nullptr;
		nPortamentoDest = 0;
		nCommand = CMD_NONE;
		nPatternLoopCount = 0;
		nPatternLoop = 0;
		nFadeOutVol = 0;
		dwFlags.set(CHN_KEYOFF | CHN_NOTEFADE);
		dwOldFlags.reset();
		autoSlide.Reset();
		nInsVol = 64;
		nnaGeneration = 0;
		//IT compatibility 15. Retrigger
		if(sndFile.m_playBehaviour[kITRetrigger])
		{
			nRetrigParam = 1;
			nRetrigCount = 0;
		}
		microTuning = 0;
		nTremorCount = 0;
		nEFxSpeed = 0;
		prevNoteOffset = 0;
		lastZxxParam = 0xFF;
		isFirstTick = false;
		triggerNote = false;
		isPreviewNote = false;
		isPaused = false;
		portaTargetReached = false;
		rowCommand.Clear();
		mpt::reconstruct(synthState);
	}

	if(resetMask & resetSetPosAdvanced)
	{
		increment = SamplePosition(0);
		nPeriod = 0;
		position.Set(0);
		nLength = 0;
		nLoopStart = 0;
		nLoopEnd = 0;
		nROfs = nLOfs = 0;
		pModSample = defaultSample;
		pModInstrument = nullptr;
		nCutOff = 0x7F;
		nResonance = 0;
		nFilterMode = FilterMode::LowPass;
		rightVol = leftVol = 0;
		newRightVol = newLeftVol = 0;
		rightRamp = leftRamp = 0;
		nVolume = 0;  // Needs to be 0 for SMP_NODEFAULTVOLUME flag
		nVibratoPos = nTremoloPos = nPanbrelloPos = 0;
		nOldHiOffset = 0;
		nLeftVU = nRightVU = 0;
		nOldExtraFinePortaUpDown = nOldFinePortaUpDown = nOldPortaDown = nOldPortaUp = 0;
		portamentoSlide = 0;
		nMasterChn = 0;

		// Custom tuning related
		m_ReCalculateFreqOnFirstTick = false;
		m_CalculateFreq = false;
		m_PortamentoFineSteps = 0;
		m_PortamentoTickSlide = 0;
	}

	if(resetMask & resetChannelSettings)
	{
		if(sourceChannel < sndFile.ChnSettings.size())
		{
			dwFlags = sndFile.ChnSettings[sourceChannel].dwFlags;
			nPan = sndFile.ChnSettings[sourceChannel].nPan;
			nGlobalVol = sndFile.ChnSettings[sourceChannel].nVolume;
			if(dwFlags[CHN_MUTE])
			{
				dwFlags.reset(CHN_MUTE);
				dwFlags.set(muteFlag);
			}
		} else
		{
			dwFlags.reset();
			nPan = 128;
			nGlobalVol = 64;
		}
		nRestorePanOnNewNote = 0;
		nRestoreCutoffOnNewNote = 0;
		nRestoreResonanceOnNewNote = 0;
	}
}


void ModChannel::Stop()
{
	nPeriod = 0;
	increment.Set(0);
	position.Set(0);
	nLeftVU = nRightVU = 0;
	nVolume = 0;
	pCurrentSample = nullptr;
}


void ModChannel::UpdateInstrumentVolume(const ModSample *smp, const ModInstrument *ins)
{
	nInsVol = 64;
	if(smp != nullptr)
		nInsVol = static_cast<uint8>(smp->nGlobalVol);
	if(ins != nullptr)
		nInsVol = static_cast<uint8>((nInsVol * ins->nGlobalVol) / 64);
}


uint32 ModChannel::GetVSTVolume() const noexcept
{
	return pModInstrument ? pModInstrument->nGlobalVol * 4 : nVolume;
}


ModCommand::NOTE ModChannel::GetPluginNote(bool ignoreArpeggio) const noexcept
{
	if(nArpeggioLastNote != NOTE_NONE && !ignoreArpeggio)
	{
		// If an arpeggio is playing, this definitely the last playing note, which may be different from the arpeggio base note stored in nLastNote.
		return nArpeggioLastNote;
	}
	ModCommand::NOTE plugNote = nLastNote;
	if(pModInstrument != nullptr && plugNote >= NOTE_MIN && plugNote < (std::size(pModInstrument->NoteMap) + NOTE_MIN))
	{
		plugNote = pModInstrument->NoteMap[plugNote - NOTE_MIN];
	}
	return plugNote;
}


bool ModChannel::HasMIDIOutput() const noexcept
{
	return pModInstrument != nullptr && pModInstrument->HasValidMIDIChannel();
}


bool ModChannel::HasCustomTuning() const noexcept
{
	return pModInstrument != nullptr && pModInstrument->pTuning != nullptr;
}


bool ModChannel::InSustainLoop() const noexcept
{
	return (dwFlags & (CHN_LOOP | CHN_KEYOFF)) == CHN_LOOP && pModSample->uFlags[CHN_SUSTAINLOOP];
}


void ModChannel::SetInstrumentPan(int32 pan, const CSoundFile &sndFile)
{
	// IT compatibility: Instrument and sample panning does not override channel panning
	// Test case: PanResetInstr.it
	if(sndFile.m_playBehaviour[kITDoNotOverrideChannelPan])
	{
		nRestorePanOnNewNote = static_cast<uint16>(nPan + 1);
		if(dwFlags[CHN_SURROUND])
			nRestorePanOnNewNote |= 0x8000;
	}
	nPan = pan;
}


void ModChannel::RestorePanAndFilter()
{
	if(nRestorePanOnNewNote > 0)
	{
		nPan = (nRestorePanOnNewNote & 0x7FFF) - 1;
		if(nRestorePanOnNewNote & 0x8000)
			dwFlags.set(CHN_SURROUND);
		nRestorePanOnNewNote = 0;
	}
	if(nRestoreResonanceOnNewNote > 0)
	{
		nResonance = nRestoreResonanceOnNewNote - 1;
		nRestoreResonanceOnNewNote = 0;
	}
	if(nRestoreCutoffOnNewNote > 0)
	{
		nCutOff = nRestoreCutoffOnNewNote - 1;
		nRestoreCutoffOnNewNote = 0;
	}
}


void ModChannel::RecalcTuningFreq(Tuning::RATIOTYPE vibratoFactor, Tuning::NOTEINDEXTYPE arpeggioSteps, const CSoundFile &sndFile)
{
	if(!HasCustomTuning())
		return;

	ModCommand::NOTE note = ModCommand::IsNote(nNote) ? nNote : nLastNote;

	if(sndFile.m_playBehaviour[kITRealNoteMapping] && note >= NOTE_MIN && note <= NOTE_MAX)
		note = pModInstrument->NoteMap[note - NOTE_MIN];

	nPeriod = mpt::saturate_round<uint32>(static_cast<float>(nC5Speed) * vibratoFactor * pModInstrument->pTuning->GetRatio(static_cast<Tuning::NOTEINDEXTYPE>(note - NOTE_MIDDLEC + arpeggioSteps), nFineTune + m_PortamentoFineSteps) * (1 << FREQ_FRACBITS));
}


// IT command S73-S7E
void ModChannel::InstrumentControl(uint8 param, const CSoundFile &sndFile)
{
	param &= 0x0F;
	switch(param)
	{
		case 0x3: nNNA = NewNoteAction::NoteCut; break;
		case 0x4: nNNA = NewNoteAction::Continue; break;
		case 0x5: nNNA = NewNoteAction::NoteOff; break;
		case 0x6: nNNA = NewNoteAction::NoteFade; break;
		case 0x7: VolEnv.flags.reset(ENV_ENABLED); break;
		case 0x8: VolEnv.flags.set(ENV_ENABLED); break;
		case 0x9: PanEnv.flags.reset(ENV_ENABLED); break;
		case 0xA: PanEnv.flags.set(ENV_ENABLED); break;
		case 0xB: PitchEnv.flags.reset(ENV_ENABLED); break;
		case 0xC: PitchEnv.flags.set(ENV_ENABLED); break;
		case 0xD:  // S7D: Enable pitch envelope, force to play as pitch envelope
		case 0xE:  // S7E: Enable pitch envelope, force to play as filter envelope
			if(sndFile.GetType() == MOD_TYPE_MPT)
			{
				PitchEnv.flags.set(ENV_ENABLED);
				PitchEnv.flags.set(ENV_FILTER, param != 0xD);
			}
			break;
	}
}


// Volume command :xx
void ModChannel::PlayControl(uint8 param)
{
	switch(param)
	{
	case 0: isPaused = true; break;
	case 1: isPaused = false; break;
	case 2: dwFlags.set(CHN_PINGPONGFLAG, false); break;
	case 3: dwFlags.set(CHN_PINGPONGFLAG, true); break;
	case 4: dwFlags.flip(CHN_PINGPONGFLAG); break;
	case 5: oldOffset = position.GetUInt(); break;
	case 6: position.Set(oldOffset); break;
	}
}


OPENMPT_NAMESPACE_END
