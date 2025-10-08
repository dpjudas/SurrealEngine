/*
 * ModInstrument.cpp
 * -----------------
 * Purpose: Helper functions for Module Instrument handling
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "ModInstrument.h"
#include "Sndfile.h"


OPENMPT_NAMESPACE_BEGIN


// Convert envelope data between various formats.
void InstrumentEnvelope::Convert(MODTYPE fromType, MODTYPE toType)
{
	if(!(fromType & MOD_TYPE_XM) && (toType & MOD_TYPE_XM))
	{
		// IT / MPTM -> XM: Expand loop by one tick, convert sustain loops to sustain points, remove carry flag.
		nSustainStart = nSustainEnd;
		dwFlags.reset(ENV_CARRY);

		if(nLoopEnd > nLoopStart && dwFlags[ENV_LOOP])
		{
			for(uint32 node = nLoopEnd; node < size(); node++)
			{
				at(node).tick++;
			}
		}
	} else if((fromType & MOD_TYPE_XM) && !(toType & MOD_TYPE_XM))
	{
		if(nSustainStart > nLoopEnd && dwFlags[ENV_LOOP])
		{
			// In the IT format, the sustain loop is always considered before the envelope loop.
			// In the XM format, whichever of the two is encountered first is considered.
			// So we have to disable the sustain loop if it was behind the normal loop.
			dwFlags.reset(ENV_SUSTAIN);
		}
		if(!dwFlags[ENV_LOOP | ENV_SUSTAIN])
		{
			// XM has no automatic fade-out behaviour at the end of the envelope.
			dwFlags.set(ENV_SUSTAIN);
			nSustainStart = nSustainEnd = LastPoint();
		}

		// XM -> IT / MPTM: Shorten loop by one tick by inserting bogus point
		if(nLoopEnd > nLoopStart && dwFlags[ENV_LOOP] && nLoopEnd < size())
		{
			if(at(nLoopEnd).tick - 1 > at(nLoopEnd - 1).tick)
			{
				// Insert an interpolated point just before the loop point.
				EnvelopeNode::tick_t tick = static_cast<EnvelopeNode::tick_t>(at(nLoopEnd).tick - 1u);
				auto interpolatedValue = static_cast<EnvelopeNode::value_t>(GetValueFromPosition(tick, 64));
				insert(begin() + nLoopEnd, EnvelopeNode(tick, interpolatedValue));
			} else
			{
				// There is already a point before the loop point: Use it as new loop end.
				nLoopEnd--;
			}
		}
	}

	if(toType != MOD_TYPE_MPT)
	{
		nReleaseNode = ENV_RELEASE_NODE_UNSET;
	}
}


// Get envelope value at a given tick. Assumes that the envelope data is in rage [0, rangeIn],
// returns value in range [0, rangeOut].
int32 InstrumentEnvelope::GetValueFromPosition(int position, int32 rangeOut, int32 rangeIn) const
{
	if(empty())
		return 0;

	uint32 pt = LastPoint();
	const int32 ENV_PRECISION = 1 << 16;

	// Checking where current 'tick' is relative to the envelope points.
	for(uint32 i = 0; i < LastPoint(); i++)
	{
		if (position <= at(i).tick)
		{
			pt = i;
			break;
		}
	}

	int x2 = at(pt).tick;
	int32 value = 0;

	if(position >= x2)
	{
		// Case: current 'tick' is on a envelope point.
		value = at(pt).value * ENV_PRECISION / rangeIn;
	} else
	{
		// Case: current 'tick' is between two envelope points.
		int x1 = 0;

		if(pt)
		{
			// Get previous node's value and tick.
			value = at(pt - 1).value * ENV_PRECISION / rangeIn;
			x1 = at(pt - 1).tick;
		}

		if(x2 > x1 && position > x1)
		{
			// Linear approximation between the points;
			// f(x + d) ~ f(x) + f'(x) * d, where f'(x) = (y2 - y1) / (x2 - x1)
			value += Util::muldiv(position - x1, (at(pt).value * ENV_PRECISION / rangeIn - value), x2 - x1);
		}
	}

	Limit(value, int32(0), ENV_PRECISION);
	return (value * rangeOut + ENV_PRECISION / 2) / ENV_PRECISION;
}


void InstrumentEnvelope::Sanitize(uint8 maxValue)
{
	if(!empty())
	{
		front().tick = 0;
		LimitMax(front().value, maxValue);
		for(iterator it = begin() + 1; it != end(); it++)
		{
			it->tick = std::max(it->tick, (it - 1)->tick);
			LimitMax(it->value, maxValue);
		}
		LimitMax(nLoopEnd, LastPoint());
		LimitMax(nLoopStart, nLoopEnd);
		LimitMax(nSustainEnd, LastPoint());
		LimitMax(nSustainStart, nSustainEnd);
		if(nReleaseNode != ENV_RELEASE_NODE_UNSET)
			LimitMax(nReleaseNode, LastPoint());
	} else
	{
		nLoopStart = 0;
		nLoopEnd = 0;
		nSustainStart = 0;
		nSustainEnd = 0;
		nReleaseNode = ENV_RELEASE_NODE_UNSET;
	}
}


// Translate instrument properties between two given formats.
void ModInstrument::Convert(MODTYPE fromType, MODTYPE toType)
{
	MPT_UNREFERENCED_PARAMETER(fromType);
	synth.Clear();

	if(toType & MOD_TYPE_XM)
	{
		ResetNoteMap();

		PitchEnv.dwFlags.reset(ENV_ENABLED | ENV_FILTER);

		dwFlags.reset(INS_SETPANNING);
		SetCutoff(GetCutoff(), false);
		SetResonance(GetResonance(), false);
		filterMode = FilterMode::Unchanged;

		nCutSwing = nPanSwing = nResSwing = nVolSwing = 0;

		nPPC = NOTE_MIDDLEC - 1;
		nPPS = 0;

		nNNA = NewNoteAction::NoteCut;
		nDCT = DuplicateCheckType::None;
		nDNA = DuplicateNoteAction::NoteCut;

		if(nMidiChannel == MidiMappedChannel)
		{
			nMidiChannel = MidiFirstChannel;
		}

		// FT2 only has unsigned Pitch Wheel Depth, and it's limited to 0...36 (in the GUI, at least. As you would expect it from FT2, this value is actually not sanitized on load).
		midiPWD = static_cast<int8>(std::abs(midiPWD));
		Limit(midiPWD, int8(0), int8(36));

		nGlobalVol = 64;
		nPan = 128;

		LimitMax(nFadeOut, 32767u);
	}

	VolEnv.Convert(fromType, toType);
	PanEnv.Convert(fromType, toType);
	PitchEnv.Convert(fromType, toType);

	if(fromType == MOD_TYPE_XM && (toType & (MOD_TYPE_IT | MOD_TYPE_MPT)))
	{
		if(!VolEnv.dwFlags[ENV_ENABLED])
		{
			// Note-Off with no envelope cuts the note immediately in XM
			VolEnv.resize(2);
			VolEnv[0].tick = 0;
			VolEnv[0].value =  ENVELOPE_MAX;
			VolEnv[1].tick = 1;
			VolEnv[1].value =  ENVELOPE_MIN;
			VolEnv.dwFlags.set(ENV_ENABLED | ENV_SUSTAIN);
			VolEnv.dwFlags.reset(ENV_LOOP);
			VolEnv.nSustainStart = VolEnv.nSustainEnd = 0;
		}
	}

	// Limit fadeout length and precision for IT
	if(toType & MOD_TYPE_IT)
	{
		LimitMax(nFadeOut, 8192u);
		nFadeOut = ((nFadeOut + 16) / 32) * 32;
	}

	// MPT-specific features - remove instrument tunings, Pitch/Tempo Lock, cutoff / resonance swing and filter mode for other formats
	if(!(toType & MOD_TYPE_MPT))
	{
		SetTuning(nullptr);
		pitchToTempoLock.Set(0);
		nCutSwing = nResSwing = 0;
		filterMode = FilterMode::Unchanged;
		nVolRampUp = 0;
	}
}


// Get a set of all samples referenced by this instrument
std::set<SAMPLEINDEX> ModInstrument::GetSamples() const
{
	std::set<SAMPLEINDEX> referencedSamples;

	for(const auto sample : Keyboard)
	{
		if(sample)
		{
			referencedSamples.insert(sample);
		}
	}

	return referencedSamples;
}


// Write sample references into a bool vector. If a sample is referenced by this instrument, true is written.
// The caller has to initialize the vector.
void ModInstrument::GetSamples(std::vector<bool> &referencedSamples) const
{
	for(const auto sample : Keyboard)
	{
		if(sample != 0 && sample < referencedSamples.size())
		{
			referencedSamples[sample] = true;
		}
	}
}


void ModInstrument::Sanitize(MODTYPE modType)
{
	LimitMax(nFadeOut, 65536u);
	LimitMax(nGlobalVol, 64u);
	LimitMax(nPan, 256u);

	LimitMax(wMidiBank, uint16(16384));
	LimitMax(nMidiProgram, uint8(128));
	LimitMax(nMidiChannel, uint8(17));

	if(nNNA > NewNoteAction::NoteFade) nNNA = NewNoteAction::NoteCut;
	if(nDCT > DuplicateCheckType::Plugin) nDCT = DuplicateCheckType::None;
	if(nDNA > DuplicateNoteAction::NoteFade) nDNA = DuplicateNoteAction::NoteCut;

	LimitMax(nPanSwing, uint8(64));
	LimitMax(nVolSwing, uint8(100));

	Limit(nPPS, int8(-32), int8(32));

	LimitMax(nCutSwing, uint8(64));
	LimitMax(nResSwing, uint8(64));
	
#ifdef MODPLUG_TRACKER
	MPT_UNREFERENCED_PARAMETER(modType);
	const uint8 range = ENVELOPE_MAX;
#else
	const uint8 range = modType == MOD_TYPE_AMS ? uint8_max : uint8(ENVELOPE_MAX);
#endif
	VolEnv.Sanitize();
	PanEnv.Sanitize();
	PitchEnv.Sanitize(range);
	synth.Sanitize();

	for(size_t i = 0; i < std::size(NoteMap); i++)
	{
		if(NoteMap[i] < NOTE_MIN || NoteMap[i] > NOTE_MAX)
			NoteMap[i] = static_cast<uint8>(i + NOTE_MIN);
	}

	if(!Resampling::IsKnownMode(resampling))
		resampling = SRCMODE_DEFAULT;

	if(nMixPlug > MAX_MIXPLUGINS)
		nMixPlug = 0;
}


std::map<SAMPLEINDEX, int8> ModInstrument::CanConvertToDefaultNoteMap() const
{
	std::map<SAMPLEINDEX, int8> transposeMap;
	for(size_t i = 0; i < std::size(NoteMap); i++)
	{
		if(Keyboard[i] == 0)
			continue;
		if(NoteMap[i] == NOTE_NONE)
			continue;

		const int8 relativeNote = static_cast<int8>(NoteMap[i] - (i + NOTE_MIN));
		if(transposeMap.count(Keyboard[i]) && transposeMap[Keyboard[i]] != relativeNote)
			return {};
		transposeMap[Keyboard[i]] = relativeNote;
	}
	// Remove all samples that wouldn't be transposed.
	// They were previously inserted into the map to catch the case where a specific sample's
	// map would start with a transpose value of 0 but end with a different value.
	for(auto it = transposeMap.begin(); it != transposeMap.end();)
	{
		if(it->second == 0)
			it = transposeMap.erase(it);
		else
			it++;
	}
	return transposeMap;
}


void ModInstrument::Transpose(int8 amount)
{
	for(auto &note : NoteMap)
	{
		note = static_cast<uint8>(Clamp(note + amount, NOTE_MIN, NOTE_MAX));
	}
}


uint8 ModInstrument::GetMIDIChannel(const ModChannel &channel, CHANNELINDEX chn) const
{
	// For mapped channels, return their pattern channel, modulo 16 (because there are only 16 MIDI channels)
	if(nMidiChannel == MidiMappedChannel)
		return static_cast<uint8>((channel.nMasterChn ? (channel.nMasterChn - 1u) : chn) % 16u);
	else if(HasValidMIDIChannel())
		return (nMidiChannel - MidiFirstChannel) % 16u;
	else
		return 0;

}


OPENMPT_NAMESPACE_END
