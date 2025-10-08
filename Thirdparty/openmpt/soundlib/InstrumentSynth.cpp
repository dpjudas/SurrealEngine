/*
 * InstrumentSynth.cpp
 * -------------------
 * Purpose: "Script" / "Synth" processor for various file formats (MED, GT2, Puma, His Master's Noise, Face The Music, Future Composer)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "InstrumentSynth.h"
#include "ModChannel.h"
#include "Sndfile.h"
#include "Tables.h"

OPENMPT_NAMESPACE_BEGIN

struct InstrumentSynth::States::State
{
	static constexpr uint16 STOP_ROW = uint16_max;

	enum Flags
	{
		kJumpConditionSet,
		kGTKTremorEnabled,
		kGTKTremorMute,
		kGTKTremoloEnabled,
		kGTKVibratoEnabled,
		kFCVibratoDelaySet,
		kFCVibratoStep,
		kFCPitchBendStep,
		kFCVolumeBendStep,
		kNumFlags,
	};

	std::bitset<kNumFlags> m_flags;

	uint16 m_currentRow = STOP_ROW;
	uint16 m_nextRow = 0;
	uint16 m_ticksRemain = 0;
	uint8 m_stepSpeed = 1;
	uint8 m_stepsRemain = 0;

	uint16 m_volumeFactor = 16384;
	int16 m_volumeAdd = int16_min;
	uint16 m_panning = 2048;
	int16 m_linearPitchFactor = 0;
	int16 m_periodFreqSlide = 0;
	int16 m_periodAdd = 0;
	uint16 m_loopCount = 0;

	uint16 m_gtkKeyOffOffset = STOP_ROW;
	int16 m_gtkVolumeStep = 0, m_gtkPitchStep = 0, m_gtkPanningStep = 0;
	uint16 m_gtkPitch = 4096;
	uint8 m_gtkSpeed = 1, m_gtkSpeedRemain = 1;
	uint8 m_gtkTremorOnTime = 3, m_gtkTremorOffTime = 3, m_gtkTremorPos = 0;
	uint8 m_gtkVibratoWidth = 0, m_gtkVibratoSpeed = 0, m_gtkVibratoPos = 0;

	uint8 m_pumaStartWaveform = 0, m_pumaEndWaveform = 0, m_pumaWaveform = 0;
	int8 m_pumaWaveformStep = 0;

	uint8 m_medVibratoEnvelope = uint8_max, m_medVibratoSpeed = 0, m_medVibratoDepth = 0;
	int8 m_medVibratoValue = 0;
	uint16 m_medVibratoPos = 0;
	int16 m_medVolumeStep = 0;
	int16 m_medPeriodStep = 0;
	uint16 m_medArpOffset = STOP_ROW;
	uint8 m_medArpPos = 0;
	uint8 m_medHold = uint8_max;
	uint16 m_medDecay = STOP_ROW;
	uint8 m_medVolumeEnv = uint8_max, m_medVolumeEnvPos = 0;

	uint32 m_ftmSampleStart = 0;
	int16 m_ftmDetune = 1;
	uint16 m_ftmVolumeChangeJump = STOP_ROW;
	uint16 m_ftmPitchChangeJump = STOP_ROW;
	uint16 m_ftmSampleChangeJump = STOP_ROW;
	uint16 m_ftmReleaseJump = STOP_ROW;
	uint16 m_ftmVolumeDownJump = STOP_ROW;
	uint16 m_ftmPortamentoJump = STOP_ROW;
	struct LFO
	{
		uint8 targetWaveform = 0, speed = 0, depth = 0, position = 0;
	};
	std::array<LFO, 4> m_ftmLFO;
	uint8 m_ftmWorkTrack = 0;

	int8 m_fcPitch = 0;
	int16 m_fcVibratoValue = 0;
	uint8 m_fcVibratoDelay = 0, m_fcVibratoSpeed = 0, m_fcVibratoDepth = 0;
	int8 m_fcVolumeBendSpeed = 0, m_fcPitchBendSpeed = 0;
	uint8 m_fcVolumeBendRemain = 0, m_fcPitchBendRemain = 0;

	void JumpToPosition(const Events &events, uint16 position);
	void NextTick(const Events &events, PlayState &playState, CHANNELINDEX channel, const CSoundFile &sndFile, States &states);
	void ApplyChannelState(ModChannel &chn, int32 &period, const CSoundFile &sndFile);
	bool EvaluateEvent(const Event &event, PlayState &playState, CHANNELINDEX channel, const CSoundFile &sndFile, States &states);
	void EvaluateRunningEvent(const Event &event);
	void HandleFTMInterrupt(uint16 &target, const bool condition);
	bool HandleFCVolumeBend(bool forceRun = false);

	CHANNELINDEX FTMRealChannel(CHANNELINDEX channel, const CSoundFile &sndFile) const noexcept
	{
		if(m_ftmWorkTrack)
			return static_cast<CHANNELINDEX>(m_ftmWorkTrack - 1) % sndFile.GetNumChannels();
		else
			return channel;
	}
};


static int32 ApplyLinearPitchSlide(int32 target, const int32 totalAmount, const bool periodsAreFrequencies)
{
	const auto &table = (periodsAreFrequencies ^ (totalAmount < 0)) ? LinearSlideUpTable : LinearSlideDownTable;
	size_t value = std::abs(totalAmount);
	while(value > 0)
	{
		const size_t amount = std::min(value, std::size(table) - size_t(1));
		target = Util::muldivr(target, table[amount], 65536);
		value -= amount;
	}
	return target;
}


static int16 TranslateGT2Pitch(uint16 pitch)
{
	// 4096 = normal, 8192 = one octave up
	return mpt::saturate_round<int16>((mpt::log2(8192.0 / std::max(pitch, uint16(1))) - 1.0) * (16 * 12));
}


static int32 TranslateFTMPitch(uint16 pitch, ModChannel &chn, const CSoundFile &sndFile)
{
	int32 period = sndFile.GetPeriodFromNote(NOTE_MIDDLEC - 12 + pitch / 16, chn.nFineTune, chn.nC5Speed);
	sndFile.DoFreqSlide(chn, period, (pitch % 16) * 4);
	return period;
}


static int8 MEDEnvelopeFromSample(const ModInstrument &instr, const CSoundFile &sndFile, uint8 envelope, uint16 envelopePos)
{
	SAMPLEINDEX smp = instr.Keyboard[NOTE_MIDDLEC - NOTE_MIN] + envelope;
	if(smp < 1 || smp > sndFile.GetNumSamples())
		return 0;
	
	const auto &mptSmp = sndFile.GetSample(smp);
	if(envelopePos >= mptSmp.nLength || mptSmp.uFlags[CHN_16BIT] || !mptSmp.sample8())
		return 0;

	return mptSmp.sample8()[envelopePos];
}


static void ChannelSetSample(ModChannel &chn, const CSoundFile &sndFile, SAMPLEINDEX smp, bool swapAtEnd = true)
{
	if(smp < 1 || smp > sndFile.GetNumSamples())
		return;
	const bool channelIsActive = chn.pCurrentSample && chn.nLength;
	if(sndFile.m_playBehaviour[kMODSampleSwap] && smp <= uint8_max && swapAtEnd && channelIsActive)
	{
		chn.swapSampleIndex = smp;
		return;
	}
	const ModSample &sample = sndFile.GetSample(smp);
	if(chn.pModSample == &sample && channelIsActive)
		return;
	if(chn.increment.IsZero() && chn.nLength == 0 && chn.nVolume == 0)
		chn.nVolume = 256;
	chn.pModSample = &sample;
	chn.pCurrentSample = sample.samplev();
	chn.dwFlags = (chn.dwFlags & CHN_CHANNELFLAGS) | sample.uFlags;
	chn.nLength = sample.uFlags[CHN_LOOP] ? sample.nLoopEnd : sample.nLength;
	chn.nLoopStart = sample.nLoopStart;
	chn.nLoopEnd = sample.nLoopEnd;
	if(chn.position.GetUInt() >= chn.nLength)
		chn.position.Set(0);
}


// To allow State to be forward-declared
InstrumentSynth::States::States() = default;
InstrumentSynth::States::States(const States &other) = default;
InstrumentSynth::States::States(States &&other) noexcept = default;
InstrumentSynth::States::~States() = default;
InstrumentSynth::States &InstrumentSynth::States::operator=(const States &other) = default;
InstrumentSynth::States &InstrumentSynth::States::operator=(States &&other) noexcept = default;


void InstrumentSynth::States::Stop()
{
	for(auto &state : states)
		state.m_currentRow = state.m_nextRow = State::STOP_ROW;
}


void InstrumentSynth::States::NextTick(PlayState &playState, CHANNELINDEX channel, const CSoundFile &sndFile)
{
	ModChannel &chn = playState.Chn[channel];
	if(!chn.pModInstrument || !chn.pModInstrument->synth.HasScripts())
		return;

	const auto &scripts = chn.pModInstrument->synth.m_scripts;
	states.resize(scripts.size());
	for(size_t i = 0; i < scripts.size(); i++)
	{
		auto &script = scripts[i];
		auto &state = states[i];
		if(chn.triggerNote)
			mpt::reconstruct(state);

		if(i == 1 && chn.rowCommand.command == CMD_MED_SYNTH_JUMP && chn.isFirstTick)
		{
			// Ugly special case: If the script didn't run yet (not triggered on same row), we need to run at least the first SetStepSpeed command.
			if(state.m_nextRow == 0 && !script.empty() && script[0].type == Event::Type::SetStepSpeed)
			{
				state.EvaluateEvent(script[0], playState, channel, sndFile, *this);
				state.m_stepsRemain = 0;
			}
			state.JumpToPosition(script, chn.rowCommand.param);
		}

		state.NextTick(script, playState, channel, sndFile, *this);
	}
}


void InstrumentSynth::States::ApplyChannelState(ModChannel &chn, int32 &period, const CSoundFile &sndFile)
{
	if(!chn.pModInstrument || !chn.pModInstrument->synth.HasScripts())
		return;

	for(auto &state : states)
	{
		state.ApplyChannelState(chn, period, sndFile);
	}
}


void InstrumentSynth::States::State::JumpToPosition(const Events &events, uint16 position)
{
	for(size_t pos = 0; pos < events.size(); pos++)
	{
		if(events[pos].type == Event::Type::JumpMarker && events[pos].u16 >= position)
		{
			m_nextRow = static_cast<uint16>(pos);
			m_ticksRemain = 0;
			return;
		}
	}
}


void InstrumentSynth::States::State::NextTick(const Events &events, PlayState &playState, CHANNELINDEX channel, const CSoundFile &sndFile, States &states)
{
	if(events.empty())
		return;

	const CHANNELINDEX origChannel = channel;
	channel = FTMRealChannel(channel, sndFile);
	ModChannel *chn = &playState.Chn[channel];

	if(m_gtkKeyOffOffset != STOP_ROW && chn->dwFlags[CHN_KEYOFF])
	{
		m_nextRow = m_gtkKeyOffOffset;
		m_ticksRemain = 0;
		m_gtkKeyOffOffset = STOP_ROW;
	}
	if(m_pumaWaveformStep)
	{
		m_pumaWaveform = static_cast<uint8>(Clamp(m_pumaWaveform + m_pumaWaveformStep, m_pumaStartWaveform, m_pumaEndWaveform));
		if(m_pumaWaveform <= m_pumaStartWaveform || m_pumaWaveform >= m_pumaEndWaveform)
			m_pumaWaveformStep = -m_pumaWaveformStep;
		ChannelSetSample(*chn, sndFile, m_pumaWaveform);
	}

	if(m_medHold != uint8_max)
	{
		if(!m_medHold--)
			m_nextRow = m_medDecay;
	}

	const ModCommand &m = chn->rowCommand;
	HandleFTMInterrupt(m_ftmPitchChangeJump, m.IsNote());
	HandleFTMInterrupt(m_ftmVolumeChangeJump, m.command == CMD_CHANNELVOLUME);
	HandleFTMInterrupt(m_ftmSampleChangeJump, m.instr != 0);
	HandleFTMInterrupt(m_ftmReleaseJump, m.note == NOTE_KEYOFF);
	HandleFTMInterrupt(m_ftmVolumeDownJump, m.command == CMD_VOLUMEDOWN_DURATION);
	HandleFTMInterrupt(m_ftmPortamentoJump, m.command == CMD_TONEPORTA_DURATION);

	if(!HandleFCVolumeBend() && m_stepSpeed && !m_stepsRemain--)
	{
		// Yep, MED executes this before a potential SPD command may change the step speed on this very row...
		m_stepsRemain = m_stepSpeed - 1;

		if(m_medVolumeStep)
			m_volumeFactor = static_cast<int16>(std::clamp(m_volumeFactor + m_medVolumeStep, 0, 16384));
		if(m_medPeriodStep)
			m_periodAdd = mpt::saturate_cast<int16>(m_periodAdd - m_medPeriodStep);
		if(m_medVolumeEnv != uint8_max && chn->pModInstrument)
		{
			m_volumeFactor = static_cast<uint16>(std::clamp((MEDEnvelopeFromSample(*chn->pModInstrument, sndFile, m_medVolumeEnv & 0x7F, m_medVolumeEnvPos) + 128) * 64, 0, 16384));
			if(m_medVolumeEnvPos < 127)
				m_medVolumeEnvPos++;
			else if(m_medVolumeEnv & 0x80)
				m_medVolumeEnvPos = 0;
		}

		if(m_ticksRemain)
		{
			if(m_currentRow < events.size())
				EvaluateRunningEvent(events[m_currentRow]);
			m_ticksRemain--;
		} else
		{
			uint8 jumpCount = 0;
			while(!m_ticksRemain)
			{
				m_currentRow = m_nextRow;
				if(m_currentRow >= std::min(events.size(), static_cast<size_t>(STOP_ROW)))
					break;
				m_nextRow++;
				if(EvaluateEvent(events[m_currentRow], playState, channel, sndFile, states))
					break;

				MPT_ASSERT(m_nextRow == STOP_ROW || m_nextRow == m_currentRow + 1 || events[m_currentRow].IsJumpEvent());
				if(events[m_currentRow].IsJumpEvent())
				{
					// This smells like an infinite loop
					if(jumpCount++ > 10)
						break;
				}

				channel = FTMRealChannel(origChannel, sndFile);
				chn = &playState.Chn[channel];
			}
		}
	}

	// MED stuff
	if(m_medArpOffset < events.size() && events[m_medArpOffset].u16)
	{
		m_linearPitchFactor = 16 * events[m_medArpOffset + m_medArpPos].u8;
		m_medArpPos = static_cast<uint8>((m_medArpPos + 1) % events[m_medArpOffset].u16);
	}
	if(m_medVibratoDepth)
	{
		static_assert(std::size(ModSinusTable) == 64);
		uint16 offset = m_medVibratoPos / 16u;
		if(m_medVibratoEnvelope == uint8_max)
			m_medVibratoValue = ModSinusTable[(offset * 2) % std::size(ModSinusTable)];
		else if(chn->pModInstrument)
			m_medVibratoValue = MEDEnvelopeFromSample(*chn->pModInstrument, sndFile, m_medVibratoEnvelope, offset);
		m_medVibratoPos = (m_medVibratoPos + m_medVibratoSpeed) % (32u * 16u);
	}

	// GTK stuff
	if(m_currentRow < events.size() && m_gtkSpeed && !--m_gtkSpeedRemain)
	{
		m_gtkSpeedRemain = m_gtkSpeed;
		if(m_gtkVolumeStep)
			m_volumeFactor = static_cast<uint16>(std::clamp(m_volumeFactor + m_gtkVolumeStep, 0, 16384));
		if(m_gtkPanningStep)
			m_panning = static_cast<uint16>(std::clamp(m_panning + m_gtkPanningStep, 0, 4096));
		if(m_gtkPitchStep)
		{
			m_gtkPitch = static_cast<uint16>(std::clamp(m_gtkPitch + m_gtkPitchStep, 0, 32768));
			m_linearPitchFactor = TranslateGT2Pitch(m_gtkPitch);
		}
	}
	if(m_flags[kGTKTremorEnabled])
	{
		if(m_gtkTremorPos >= m_gtkTremorOnTime + m_gtkTremorOffTime)
			m_gtkTremorPos = 0;
		m_flags.set(kGTKTremorMute, m_gtkTremorPos >= m_gtkTremorOnTime);
		m_gtkTremorPos++;
	}
	if(m_flags[kGTKTremoloEnabled])
	{
		m_volumeAdd = static_cast<int16>(ModSinusTable[(m_gtkVibratoPos / 4u) % std::size(ModSinusTable)] * m_gtkVibratoWidth / 2);
		m_gtkVibratoPos += m_gtkVibratoSpeed;
	}
	if(m_flags[kGTKVibratoEnabled])
	{
		m_periodFreqSlide = static_cast<int16>(-ModSinusTable[(m_gtkVibratoPos / 4u) % std::size(ModSinusTable)] * m_gtkVibratoWidth / 96);
		m_gtkVibratoPos += m_gtkVibratoSpeed;
	}

	// FTM LFOs
	for(auto &lfo : m_ftmLFO)
	{
		if(!lfo.speed && !lfo.depth)
			continue;

		const uint8 lutPos = static_cast<uint8>(Util::muldivr_unsigned(lfo.position, 256, 192));
		int32 value = 0;
		switch(lfo.targetWaveform & 0x07)
		{
		case 0: value = ITSinusTable[lutPos]; break;
		case 1: value = lutPos < 128 ? 64 : -64; break;
		case 2: value = 64 - std::abs(((lutPos + 64) % 256) - 128); break;
		case 3: value = 64 - lutPos / 2; break;
		case 4: value = lutPos / 2 - 64; break;
		}
		if((lfo.targetWaveform & 0xF0) < 0xA0)
			value += 64;
		value *= lfo.depth;  // -8192...+8192 or 0...16384 for LFO targets

		switch(lfo.targetWaveform & 0xF0)
		{
		case 0x10: m_ftmLFO[0].speed = static_cast<uint8>(value / 64); break;
		case 0x20: m_ftmLFO[1].speed = static_cast<uint8>(value / 64); break;
		case 0x30: m_ftmLFO[2].speed = static_cast<uint8>(value / 64); break;
		case 0x40: m_ftmLFO[3].speed = static_cast<uint8>(value / 64); break;
		case 0x50: m_ftmLFO[0].depth = static_cast<uint8>(value / 64); break;
		case 0x60: m_ftmLFO[1].depth = static_cast<uint8>(value / 64); break;
		case 0x70: m_ftmLFO[2].depth = static_cast<uint8>(value / 64); break;
		case 0x80: m_ftmLFO[3].depth = static_cast<uint8>(value / 64); break;
		case 0xA0: m_volumeAdd = mpt::saturate_cast<int16>(value * 4); break;
		case 0xF0: m_periodFreqSlide = static_cast<int16>(value / 8); break;
		}
		
		uint16 newPos = lfo.position + lfo.speed;
		if(newPos >= 192)
		{
			newPos -= 192;
			if(lfo.targetWaveform & 0x08)
			{
				lfo.speed = 0;
				newPos = 191;
			}
		}
		lfo.position = static_cast<uint8>(newPos);
	}

	// Future Composer stuff
	if(m_flags[kFCVibratoDelaySet] && m_fcVibratoDelay > 0)
	{
		m_fcVibratoDelay--;
	} else if(m_fcVibratoDepth)
	{
		if(m_flags[kFCVibratoStep])
		{
			int16 delta = m_fcVibratoDepth * 2;
			m_fcVibratoValue += m_fcVibratoSpeed;
			if(m_fcVibratoValue > delta)
			{
				m_fcVibratoValue = delta;
				m_flags.flip(kFCVibratoStep);
			}
		} else
		{
			m_fcVibratoValue -= m_fcVibratoSpeed;
			if(m_fcVibratoValue < 0)
			{
				m_fcVibratoValue = 0;
				m_flags.flip(kFCVibratoStep);
			}
		}
	}
	if(m_fcPitchBendRemain)
	{
		m_flags.flip(kFCPitchBendStep);
		if(m_flags[kFCPitchBendStep])
		{
			m_fcPitchBendRemain--;
			m_periodAdd -= static_cast<int16>(m_fcPitchBendSpeed * 4);
		}
	}
}


void InstrumentSynth::States::State::ApplyChannelState(ModChannel &chn, int32 &period, const CSoundFile &sndFile)
{
	if(m_volumeFactor != 16384)
		chn.nRealVolume = Util::muldivr(chn.nRealVolume, m_volumeFactor, 16384);
	if(m_volumeAdd != int16_min)
		chn.nRealVolume = std::clamp(chn.nRealVolume + m_volumeAdd, int32(0), int32(16384));
	if(m_flags[kGTKTremorEnabled] && m_flags[kGTKTremorMute])
		chn.nRealVolume = 0;

	if(m_panning != 2048)
	{
		if(chn.nRealPan >= 128)
			chn.nRealPan += ((m_panning - 2048) * (256 - chn.nRealPan)) / 2048;
		else
			chn.nRealPan += ((m_panning - 2048) * (chn.nRealPan)) / 2048;
	}

	const bool periodsAreFrequencies = sndFile.PeriodsAreFrequencies();
	if(m_linearPitchFactor != 0)
		period = ApplyLinearPitchSlide(period, m_linearPitchFactor, periodsAreFrequencies);
	if(m_periodFreqSlide != 0)
		sndFile.DoFreqSlide(chn, period, m_periodFreqSlide);
	if(periodsAreFrequencies)
		period -= m_periodAdd;
	else
		period += m_periodAdd;
	if(m_medVibratoDepth)
		period += m_medVibratoValue * m_medVibratoDepth / 64;

	int16 vibratoFC = m_fcVibratoValue - m_fcVibratoDepth;
	const bool doVibratoFC = vibratoFC != 0 && m_fcVibratoDelay < 1;
	if(m_fcPitch || doVibratoFC)
	{
		uint8 fcNote = static_cast<uint8>(m_fcPitch >= 0 ? m_fcPitch + chn.nLastNote - NOTE_MIN : m_fcPitch) & 0x7F;
		static_assert(mpt::array_size<decltype(chn.pModInstrument->NoteMap)>::size > 0x7F);
		if(m_fcPitch && ModCommand::IsNote(chn.nLastNote))
			period += (sndFile.GetPeriodFromNote(chn.pModInstrument->NoteMap[fcNote], chn.nFineTune, chn.nC5Speed) - sndFile.GetPeriodFromNote(chn.pModInstrument->NoteMap[chn.nLastNote - NOTE_MIN], chn.nFineTune, chn.nC5Speed));

		if(doVibratoFC)
		{
			int note = (fcNote * 2) + 160;
			while(note < 256)
			{
				vibratoFC *= 2;
				note += 24;
			}
			period += vibratoFC * 4;
		}
	}

	if((m_linearPitchFactor || m_periodFreqSlide || m_periodAdd) && !sndFile.PeriodsAreFrequencies())
	{
		if(period < sndFile.m_nMinPeriod)
			period = sndFile.m_nMinPeriod;
		else if(period > sndFile.m_nMaxPeriod && sndFile.m_playBehaviour[kApplyUpperPeriodLimit])
			period = sndFile.m_nMaxPeriod;
	}
	if(period < 1)
		period = 1;

	if(m_ftmDetune != 1)
		chn.microTuning = m_ftmDetune;
}


bool InstrumentSynth::States::State::EvaluateEvent(const Event &event, PlayState &playState, CHANNELINDEX channel, const CSoundFile &sndFile, States &states)
{
	// Return true to indicate end of processing for this tick
	ModChannel &chn = playState.Chn[channel];
	switch(event.type)
	{
	case Event::Type::StopScript:
		m_nextRow = STOP_ROW;
		return true;
	case Event::Type::Jump:
		m_nextRow = event.u16;
		return false;
	case Event::Type::JumpIfTrue:
		if(m_flags[kJumpConditionSet])
			m_nextRow = event.u16;
		return false;
	case Event::Type::Delay:
		m_ticksRemain = event.u16;
		return true;
	case Event::Type::SetStepSpeed:
		m_stepSpeed = event.u8;
		if(event.Byte1())
			m_stepsRemain = m_stepSpeed - 1;
		return false;
	case Event::Type::JumpMarker:
		return false;
	case Event::Type::SampleOffset:
	case Event::Type::SampleOffsetAdd:
	case Event::Type::SampleOffsetSub:
		{
			int64 pos = event.Value24Bit();
			if(event.type == Event::Type::SampleOffsetAdd)
				pos += chn.position.GetInt();
			else if(event.type == Event::Type::SampleOffsetSub)
				pos = chn.position.GetInt() - pos;
			else
				pos += m_ftmSampleStart;
			chn.position.Set(std::min(chn.nLength, mpt::saturate_cast<SmpLength>(pos)), 0);
		}
		return false;
	case Event::Type::SetLoopCounter:
		if(!m_loopCount || event.u8)
			m_loopCount = 1 + std::min(event.u16, uint16(0xFFFE));
		return false;
	case Event::Type::EvaluateLoopCounter:
		if(m_loopCount > 1)
			m_nextRow = event.u16;
		if(m_loopCount)
			m_loopCount--;
		return false;
	case Event::Type::NoteCut:
		chn.nFadeOutVol = 0;
		chn.dwFlags.set(CHN_NOTEFADE);
		return false;

	case Event::Type::GTK_KeyOff:
		m_gtkKeyOffOffset = event.u16;
		return false;
	case Event::Type::GTK_SetVolume:
		m_volumeFactor = event.u16;
		chn.dwFlags.set(CHN_FASTVOLRAMP);
		return false;
	case Event::Type::GTK_SetPitch:
		m_gtkPitch = event.u16;
		m_linearPitchFactor = TranslateGT2Pitch(event.u16);
		m_periodAdd = 0;
		return false;
	case Event::Type::GTK_SetPanning:
		m_panning = event.u16;
		return false;
	case Event::Type::GTK_SetVolumeStep:
		m_gtkVolumeStep = event.i16;
		return false;
	case Event::Type::GTK_SetPitchStep:
		m_gtkPitchStep = event.i16;
		return false;
	case Event::Type::GTK_SetPanningStep:
		m_gtkPanningStep = event.i16;
		return false;
	case Event::Type::GTK_SetSpeed:
		m_gtkSpeed = m_gtkSpeedRemain = event.u8;
		return false;
	case Event::Type::GTK_EnableTremor:
		m_flags.set(kGTKTremorEnabled, event.u8 != 0);
		return false;
	case Event::Type::GTK_SetTremorTime:
		if(event.Byte0())
			m_gtkTremorOnTime = event.Byte0();
		if(event.Byte1())
			m_gtkTremorOffTime = event.Byte1();
		m_gtkTremorPos = 0;
		return false;
	case Event::Type::GTK_EnableTremolo:
		m_flags.set(kGTKTremoloEnabled, event.u8 != 0);
		m_gtkVibratoPos = 0;
		if(!m_gtkVibratoWidth)
			m_gtkVibratoWidth = 8;
		if(!m_gtkVibratoSpeed)
			m_gtkVibratoSpeed = 16;
		return false;
	case Event::Type::GTK_EnableVibrato:
		m_flags.set(kGTKVibratoEnabled, event.u8 != 0);
		m_periodFreqSlide = 0;
		m_gtkVibratoPos = 0;
		if(!m_gtkVibratoWidth)
			m_gtkVibratoWidth = 3;
		if(!m_gtkVibratoSpeed)
			m_gtkVibratoSpeed = 8;
		return false;
	case Event::Type::GTK_SetVibratoParams:
		if(event.Byte0())
			m_gtkVibratoWidth = event.Byte0();
		if(event.Byte1())
			m_gtkVibratoSpeed = event.Byte1();
		return false;

	case Event::Type::Puma_SetWaveform:
		m_pumaWaveform = m_pumaStartWaveform = event.Byte0() + 1;
		if(event.Byte0() < 10)
		{
			m_pumaWaveformStep = 0;
		} else
		{
			m_pumaWaveformStep = static_cast<int8>(event.Byte1());
			m_pumaEndWaveform = event.Byte2() + m_pumaStartWaveform;
		}
		ChannelSetSample(chn, sndFile, m_pumaWaveform);
		return false;
	case Event::Type::Puma_VolumeRamp:
		m_ticksRemain = event.Byte2();
		m_volumeAdd = static_cast<int16>(event.Byte0() * 256 - 16384);
		chn.dwFlags.set(CHN_FASTVOLRAMP);
		return true;
	case Event::Type::Puma_StopVoice:
		chn.Stop();
		m_nextRow = STOP_ROW;
		return true;
	case Event::Type::Puma_SetPitch:
		m_linearPitchFactor = event.i8 * 8;
		m_periodAdd = 0;
		m_ticksRemain = std::max(event.Byte2(), uint8(1)) - 1;
		return true;
	case Event::Type::Puma_PitchRamp:
		m_linearPitchFactor = 0;
		m_periodAdd = event.i8 * 4;
		m_ticksRemain = std::max(event.Byte2(), uint8(1)) - 1;
		return true;

	case Event::Type::Mupp_SetWaveform:
		ChannelSetSample(chn, sndFile, static_cast<SAMPLEINDEX>(32 + event.Byte0() * 28 + event.Byte1()));
		m_volumeFactor = static_cast<uint16>(std::min(event.Byte2() & 0x7F, 64) * 256u);
		chn.dwFlags.set(CHN_FASTVOLRAMP);
		return true;

	case Event::Type::MED_DefineArpeggio:
		if(!event.u16)
			return false;
		m_nextRow = m_currentRow + event.u16;
		m_medArpOffset = m_currentRow;
		m_medArpPos = 0;
		return true;
	case Event::Type::MED_JumpScript:
		if(event.u8 < chn.synthState.states.size() && chn.pModInstrument && event.u8 < chn.pModInstrument->synth.m_scripts.size())
		{
			chn.synthState.states[event.u8].JumpToPosition(chn.pModInstrument->synth.m_scripts[event.u8], event.u16);
			chn.synthState.states[event.u8].m_stepsRemain = 0;
		}
		return false;
	case Event::Type::MED_SetEnvelope:
		if(event.Byte2())
			m_medVolumeEnv = (event.Byte0() & 0x3F) | (event.Byte1() ? 0x80 : 0x00);
		else
			m_medVibratoEnvelope = event.Byte0();
		m_medVolumeEnvPos = 0;
		return false;
	case Event::Type::MED_SetVolume:
		m_volumeFactor = event.u8 * 256u;
		chn.dwFlags.set(CHN_FASTVOLRAMP);
		return true;
	case Event::Type::MED_SetWaveform:
		if(chn.pModInstrument)
			ChannelSetSample(chn, sndFile, chn.pModInstrument->Keyboard[NOTE_MIDDLEC - NOTE_MIN] + event.u8);
		return true;
	case Event::Type::MED_SetVibratoSpeed:
		m_medVibratoSpeed = event.u8;
		return false;
	case Event::Type::MED_SetVibratoDepth:
		m_medVibratoDepth = event.u8;
		return false;
	case Event::Type::MED_SetVolumeStep:
		m_medVolumeStep = static_cast<int16>(event.i16 * 256);
		return false;
	case Event::Type::MED_SetPeriodStep:
		m_medPeriodStep = static_cast<int16>(event.i16 * 4);
		return false;
	case Event::Type::MED_HoldDecay:
		m_medHold = event.u8;
		m_medDecay = event.u16;
		return false;

	case Event::Type::FTM_SetCondition:
 		{
			MPT_ASSERT(!sndFile.PeriodsAreFrequencies());
 			const int32 threshold = (event.u8 < 3) ? int32_max - TranslateFTMPitch(event.u16, chn, sndFile) : event.u16;
			const int32 compare = (event.u8 < 3) ? int32_max - chn.nPeriod : chn.nGlobalVol;
			switch(event.u8 % 3u)
			{
			case 0: m_flags.set(kJumpConditionSet, compare == threshold); break;
			case 1: m_flags.set(kJumpConditionSet, compare < threshold); break;
			case 2: m_flags.set(kJumpConditionSet, compare > threshold); break;
			}
		}
		return false;
	case Event::Type::FTM_SetInterrupt:
		if(event.u8 & 0x01) m_ftmPitchChangeJump = event.u16;
		if(event.u8 & 0x02) m_ftmVolumeChangeJump = event.u16;
		if(event.u8 & 0x04) m_ftmSampleChangeJump = event.u16;
		if(event.u8 & 0x08) m_ftmReleaseJump = event.u16;
		if(event.u8 & 0x10) m_ftmPortamentoJump = event.u16;
		if(event.u8 & 0x20) m_ftmVolumeDownJump = event.u16;
		return false;
	case Event::Type::FTM_PlaySample:
		if(chn.nNewIns > 0 && chn.nNewIns <= sndFile.GetNumSamples())
			chn.pModSample = &sndFile.GetSample(chn.nNewIns);
		if(chn.pModSample)
		{
			const ModSample &sample = *chn.pModSample;
			chn.nVolume = sample.nVolume;
			chn.UpdateInstrumentVolume(&sample, nullptr);
			chn.nC5Speed = sample.nC5Speed;
			chn.dwFlags = (chn.dwFlags & (CHN_CHANNELFLAGS ^ CHN_NOTEFADE)) | sample.uFlags;
			chn.nLength = chn.pModSample->uFlags[CHN_LOOP] ? sample.nLoopEnd : sample.nLength;
			chn.nLoopStart = sample.nLoopStart;
			chn.nLoopEnd = sample.nLoopEnd;
		}
		chn.position.Set(0);
		return false;
	case Event::Type::FTM_SetPitch:
		chn.nPeriod = TranslateFTMPitch(event.u16 * 2, chn, sndFile);
		return false;
	case Event::Type::FTM_SetDetune:
		// Detune always applies to the first channel of a channel pair (and only if the other channel is playing a sample)
		states.states[channel & ~1].m_ftmDetune = static_cast<int16>(event.u16 * -8);
		return false;
	case Event::Type::FTM_AddDetune:
		states.states[channel & ~1].m_ftmDetune -= static_cast<int16>(event.i16 * 8);
		return false;
	case Event::Type::FTM_AddPitch:
		if(event.i16)
		{
			sndFile.DoFreqSlide(chn, chn.nPeriod, event.i16 * 8);
			const int32 limit = TranslateFTMPitch((event.i16 < 0) ? 0 : 0x21E, chn, sndFile);
			if((event.i16 > 0) == sndFile.PeriodsAreFrequencies())
				chn.nPeriod = std::min(chn.nPeriod, limit);
			else
				chn.nPeriod = std::max(chn.nPeriod, limit);
		}
		return false;
	case Event::Type::FTM_SetVolume:
		chn.nGlobalVol = std::min(event.u8, uint8(64));
		chn.dwFlags.set(CHN_FASTVOLRAMP);
		return false;
	case Event::Type::FTM_AddVolume:
		chn.nGlobalVol = static_cast<uint8>(std::clamp(chn.nGlobalVol + event.i16, 0, 64));
		return false;
	case Event::Type::FTM_SetSample:
		chn.swapSampleIndex = event.u8 + 1;
		return false;
	case Event::Type::FTM_SetSampleStart:
		// Documentation says this should be in words, but it really appears to work with bytes.
		// The relative variants appear to be completely broken.
		if(event.u8 == 1)
			m_ftmSampleStart += std::min(static_cast<uint32>(event.u16), Util::MaxValueOfType(m_ftmSampleStart) - m_ftmSampleStart);
		else if(event.u8 == 2)
			m_ftmSampleStart -= std::min(static_cast<uint32>(event.u16), m_ftmSampleStart);
		else
			m_ftmSampleStart = event.u16 * 2u;
		return false;
	case Event::Type::FTM_SetOneshotLength:
		if(chn.pModSample)
		{
			const SmpLength loopLength = chn.nLoopEnd - chn.nLoopStart;
			int64 loopStart = event.u16 * 2;
			if(event.u8 == 1)
				loopStart += chn.nLoopStart;
			else if(event.u8 == 2)
				loopStart = chn.nLoopStart - loopStart;
			loopStart = std::clamp(loopStart, int64(0), static_cast<int64>(chn.pModSample->nLength));
			chn.nLoopStart = static_cast<SmpLength>(loopStart);
			chn.nLoopEnd = chn.nLoopStart + loopLength;
			LimitMax(chn.nLoopEnd, chn.pModSample->nLength);
			chn.nLength = chn.nLoopEnd;
			chn.dwFlags.set(CHN_LOOP, chn.nLoopEnd > chn.nLoopStart);
			if(chn.position.GetUInt() >= chn.nLength && chn.dwFlags[CHN_LOOP])
				chn.position.SetInt(chn.nLoopStart);
		}
		return false;
	case Event::Type::FTM_SetRepeatLength:
		if(chn.pModSample)
		{
			int64 loopEnd = chn.nLoopStart + event.u16 * 2;
			if(event.u8 == 1)
				loopEnd = chn.nLoopEnd + event.u16 * 2;
			else if(event.u8 == 2)
				loopEnd = chn.nLoopEnd - event.u16 * 2;
			loopEnd = std::clamp(loopEnd, static_cast<int64>(chn.nLoopStart), static_cast<int64>(chn.pModSample->nLength));
			chn.nLoopEnd = static_cast<SmpLength>(loopEnd);
			chn.nLength = chn.nLoopEnd;
			chn.dwFlags.set(CHN_LOOP, chn.nLoopEnd > chn.nLoopStart);
			if(chn.position.GetUInt() >= chn.nLength && chn.dwFlags[CHN_LOOP])
				chn.position.SetInt(chn.nLoopStart);
		}
		return false;
	case Event::Type::FTM_CloneTrack:
		if(event.Byte0() < sndFile.GetNumChannels())
		{
			const ModChannel &srcChn = playState.Chn[event.Byte0()];
			if(event.Byte1() & (0x01 | 0x08))
				chn.nPeriod = srcChn.nPeriod;
			if(event.Byte1() & (0x02 | 0x08))
				chn.nGlobalVol = srcChn.nGlobalVol;
			if(event.Byte1() & (0x04 | 0x08))
			{
				chn.nNewIns = srcChn.nNewIns;
				chn.swapSampleIndex = srcChn.swapSampleIndex;
				chn.pModSample = srcChn.pModSample;
				chn.position = srcChn.position;
				chn.dwFlags = (chn.dwFlags & CHN_CHANNELFLAGS) | (srcChn.dwFlags & CHN_SAMPLEFLAGS);
				chn.nLength = srcChn.nLength;
				chn.nLoopStart = srcChn.nLoopStart;
				chn.nLoopEnd = srcChn.nLoopEnd;
			}
			if(event.Byte1() & 0x08)
			{
				// Note: This does not appear to behave entirely as documented.
				// When the command is triggered, it copies frequency, volume, sample and the state of running slide commands.
				// Running LFOs are not copied. But any notes and effects (including newly triggered LFOs) on the source track on following rows are copied.
				// There appears to be no way to stop this cloning once it has started.
				// As no FTM in the wild makes use of this command, we will glance over this ugly detail.
				chn.position = srcChn.position;
				chn.portamentoSlide = srcChn.portamentoSlide;
				chn.nPortamentoDest = srcChn.nPortamentoDest;
				chn.volSlideDownStart = srcChn.volSlideDownStart;
				chn.volSlideDownTotal = srcChn.volSlideDownTotal;
				chn.volSlideDownRemain = srcChn.volSlideDownRemain;
				chn.autoSlide.SetActive(AutoSlideCommand::TonePortamentoWithDuration, srcChn.autoSlide.IsActive(AutoSlideCommand::TonePortamentoWithDuration));
				chn.autoSlide.SetActive(AutoSlideCommand::VolumeDownWithDuration, srcChn.autoSlide.IsActive(AutoSlideCommand::VolumeDownWithDuration));
			}
		}
		return false;
	case Event::Type::FTM_StartLFO:
		{
			auto &lfo = m_ftmLFO[event.Byte0() & 3];
			lfo.targetWaveform = event.Byte1();
			lfo.speed = lfo.depth = lfo.position = 0;
		}
		return false;
	case Event::Type::FTM_LFOAddSub:
		{
			auto &lfo = m_ftmLFO[event.Byte0() & 3];
			int factor = (event.Byte0() & 4) ? -1 : 1;
			lfo.speed = std::min(mpt::saturate_cast<uint8>(lfo.speed + event.Byte1() * factor), uint8(0xBF));
			lfo.depth = std::min(mpt::saturate_cast<uint8>(lfo.depth + event.Byte2() * factor), uint8(0x7F));
		}
		return false;
	case Event::Type::FTM_SetWorkTrack:
		if(event.Byte0() == uint8_max)
		{
			m_ftmWorkTrack = 0;
		} else if(const bool isRelative = event.Byte1() != 0; isRelative && event.Byte0())
		{
			if(!m_ftmWorkTrack)
				m_ftmWorkTrack = static_cast<uint8>(channel + 1);
			m_ftmWorkTrack = static_cast<uint8>((m_ftmWorkTrack - 1u + event.Byte0()) % sndFile.GetNumChannels() + 1);
		} else if(!isRelative)
		{
			m_ftmWorkTrack = event.Byte0() + 1;
		}
		return false;
	case Event::Type::FTM_SetGlobalVolume:
		playState.m_nGlobalVolume = event.u16;
		return false;
	case Event::Type::FTM_SetTempo:
		playState.m_nMusicTempo = TEMPO(1777517.482 / std::clamp(event.u16, uint16(0x1000), uint16(0x4FFF)));
		return false;
	case Event::Type::FTM_SetSpeed:
		if(event.u16)
			playState.m_nMusicSpeed = event.u16;
		else
			playState.m_nMusicSpeed = uint16_max;
		return false;
	case Event::Type::FTM_SetPlayPosition:
		if(ORDERINDEX playPos = sndFile.Order().FindOrder(event.u16, event.u16); playPos != ORDERINDEX_INVALID)
		{
			playState.m_nNextOrder = playPos;
			playState.m_nNextRow = event.u8;
		}
		return false;

	case Event::Type::FC_SetWaveform:
		{
			uint8 waveform = event.Byte1() + 1;
			if(event.Byte0() == 0xE9)
				waveform += static_cast<uint8>(event.Byte2() * 10 + 90);
			ChannelSetSample(chn, sndFile, waveform, event.Byte0() == 0xE4);
		}
		return false;
	case Event::Type::FC_SetPitch:
		m_fcPitch = event.i8;
		return true;
	case Event::Type::FC_SetVibrato:
		m_fcVibratoSpeed = event.Byte0();
		m_fcVibratoDepth = event.Byte1();
		if(!m_flags[kFCVibratoDelaySet])
		{
			m_flags.set(kFCVibratoDelaySet);
			m_fcVibratoDelay = event.Byte2();
			m_fcVibratoValue = m_fcVibratoDepth;
		}
		return false;
	case Event::Type::FC_PitchSlide:
		m_fcPitchBendSpeed = event.Byte0();
		m_fcPitchBendRemain = event.Byte1();
		return false;
	case Event::Type::FC_VolumeSlide:
		m_fcVolumeBendSpeed = event.Byte0();
		m_fcVolumeBendRemain = event.Byte1();
		HandleFCVolumeBend(true);
		return true;
	}

	MPT_ASSERT_NOTREACHED();
	return false;
}


void InstrumentSynth::States::State::EvaluateRunningEvent(const Event &event)
{
	switch(event.type)
	{
	case Event::Type::Puma_VolumeRamp:
		if(event.Byte2() > 0)
			m_volumeAdd = static_cast<int16>((event.Byte1() + Util::muldivr(event.Byte0() - event.Byte1(), m_ticksRemain, event.Byte2())) * 256 - 16384);
		break;
	case Event::Type::Puma_PitchRamp:
		if(event.Byte2() > 0)
			m_periodAdd = static_cast<int16>((static_cast<int8>(event.Byte1()) + Util::muldivr(static_cast<int8>(event.Byte0()) - static_cast<int8>(event.Byte1()), m_ticksRemain, event.Byte2())) * 4);
		break;
	default:
		break;
	}
}


void InstrumentSynth::States::State::HandleFTMInterrupt(uint16 &target, const bool condition)
{
	if(target == STOP_ROW || !condition)
		return;
	m_nextRow = target;
	m_ticksRemain = 0;
	m_stepsRemain = 0;
	target = STOP_ROW;
}


bool InstrumentSynth::States::State::HandleFCVolumeBend(bool forceRun)
{
	if(!m_fcVolumeBendRemain && !forceRun)
		return false;

	m_flags.flip(kFCVolumeBendStep);
	if(m_flags[kFCVolumeBendStep])
	{
		m_fcVolumeBendRemain--;
		int32 target = m_volumeFactor + m_fcVolumeBendSpeed * 256;
		if(target < 0 || target >= 32768)
			m_fcVolumeBendRemain = 0;
		m_volumeFactor = static_cast<uint16>(std::clamp(target, int32(0), int32(16384)));
	}
	return true;
}


void GlobalScriptState::Initialize(const CSoundFile &sndFile)
{
	if(!sndFile.m_globalScript.empty())
		states.assign(sndFile.GetNumChannels(), {});
}


void GlobalScriptState::NextTick(PlayState &playState, const CSoundFile &sndFile)
{
	if(sndFile.m_globalScript.empty())
		return;
	states.resize(sndFile.GetNumChannels());
	for(CHANNELINDEX chn = 0; chn < sndFile.GetNumChannels(); chn++)
	{
		auto &state = states[chn];
		auto &modChn = playState.Chn[chn];
		if(modChn.rowCommand.command == CMD_MED_SYNTH_JUMP && !playState.m_nTickCount)
			states[chn].JumpToPosition(sndFile.m_globalScript, modChn.rowCommand.param);
		state.NextTick(sndFile.m_globalScript, playState, chn, sndFile, *this);
	}
}


void GlobalScriptState::ApplyChannelState(PlayState &playState, CHANNELINDEX chn, int32 &period, const CSoundFile &sndFile)
{
	if(sndFile.m_globalScript.empty())
		return;
	for(CHANNELINDEX s = 0; s < states.size(); s++)
	{
		if(states[s].FTMRealChannel(s, sndFile) == chn)
		{
			states[s].ApplyChannelState(playState.Chn[chn], period, sndFile);
		}
	}
}


void InstrumentSynth::Sanitize()
{
	for(auto &script : m_scripts)
	{
		if(script.size() >= States::State::STOP_ROW)
			script.resize(States::State::STOP_ROW - 1);
	}
}


OPENMPT_NAMESPACE_END
