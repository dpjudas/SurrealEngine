/*
 * XMTools.cpp
 * -----------
 * Purpose: Definition of XM file structures and helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "XMTools.h"
#include "Loaders.h"
#include "Sndfile.h"
#include "../common/version.h"

#include <algorithm>


OPENMPT_NAMESPACE_BEGIN


// Convert OpenMPT's internal envelope representation to XM envelope data.
void XMInstrument::ConvertEnvelopeToXM(const InstrumentEnvelope &mptEnv, uint8le &numPoints, uint8le &flags, uint8le &sustain, uint8le &loopStart, uint8le &loopEnd, EnvType env)
{
	numPoints = static_cast<uint8>(std::min(std::size_t(12), static_cast<std::size_t>(mptEnv.size())));

	// Envelope Data
	for(uint8 i = 0; i < numPoints; i++)
	{
		switch(env)
		{
		case EnvTypeVol:
			volEnv[i * 2] = std::min(mptEnv[i].tick, uint16_max);
			volEnv[i * 2 + 1] = std::min(mptEnv[i].value, uint8(64));
			break;
		case EnvTypePan:
			panEnv[i * 2] = std::min(mptEnv[i].tick, uint16_max);
			panEnv[i * 2 + 1] = std::min(mptEnv[i].value, uint8(63));
			break;
		}
	}

	// Envelope Flags
	if(mptEnv.dwFlags[ENV_ENABLED]) flags |= XMInstrument::envEnabled;
	if(mptEnv.dwFlags[ENV_SUSTAIN]) flags |= XMInstrument::envSustain;
	if(mptEnv.dwFlags[ENV_LOOP]) flags |= XMInstrument::envLoop;

	// Envelope Loops
	sustain = std::min(uint8(12), mptEnv.nSustainStart);
	loopStart = std::min(uint8(12), mptEnv.nLoopStart);
	loopEnd = std::min(uint8(12), mptEnv.nLoopEnd);

}


// Convert OpenMPT's internal sample representation to an XMInstrument.
XMInstrument::SampleList XMInstrument::ConvertToXM(const ModInstrument &mptIns, bool compatibilityExport)
{
	MemsetZero(*this);

	// FFF is maximum in the FT2 GUI, but it can also accept other values. MilkyTracker just allows 0...4095 and 32767 ("cut")
	volFade = static_cast<uint16>(std::min(mptIns.nFadeOut, uint32(32767)));

	// Convert envelopes
	ConvertEnvelopeToXM(mptIns.VolEnv, volPoints, volFlags, volSustain, volLoopStart, volLoopEnd, EnvTypeVol);
	ConvertEnvelopeToXM(mptIns.PanEnv, panPoints, panFlags, panSustain, panLoopStart, panLoopEnd, EnvTypePan);

	if(mptIns.nMidiChannel != MidiNoChannel)
	{
		midiEnabled = 1;
		midiChannel = (mptIns.nMidiChannel != MidiMappedChannel ? (mptIns.nMidiChannel - MidiFirstChannel) : 0);
	}
	midiProgram = (mptIns.nMidiProgram != 0 ? mptIns.nMidiProgram - 1 : 0);
	pitchWheelRange = std::min(mptIns.midiPWD, int8(36));

	// Create sample assignment table
	const auto sampleList = GetSampleList(mptIns, compatibilityExport);
	for(std::size_t i = 0; i < std::size(sampleMap); i++)
	{
		if(mptIns.Keyboard[i + 12] > 0)
		{
			auto sample = std::find(sampleList.samples.begin(), sampleList.samples.end(), mptIns.Keyboard[i + 12]);
			if(sample != sampleList.samples.end())
			{
				// Yep, we want to export this sample.
				sampleMap[i] = static_cast<uint8>(std::distance(sampleList.samples.begin(), sample));
			}
		}
	}

	return sampleList;
}


// Get a list of samples that should be written to the file.
XMInstrument::SampleList XMInstrument::GetSampleList(const ModInstrument &mptIns, bool compatibilityExport) const
{
	SampleList sampleList;          // List of samples associated with this instrument
	std::vector<bool> addedToList;  // Which samples did we already add to the sample list?

	for(std::size_t i = 0; i < std::size(sampleMap); i++)
	{
		const SAMPLEINDEX smp = mptIns.Keyboard[i + 12];
		if(smp == 0)
			continue;
		if(smp > addedToList.size())
			addedToList.resize(smp, false);
		if(addedToList[smp - 1])
			continue;
		// We haven't considered this sample yet.
		addedToList[smp - 1] = true;
		if(sampleList.samples.size() < (compatibilityExport ? 16u : 32u))
			sampleList.samples.push_back(smp);
		else
			sampleList.tooManySamples = true;
	}
	// FT2 completely ignores MIDI settings (and also the less important stuff) if not at least one (empty) sample is assigned to this instrument!
	if(sampleList.samples.empty() && compatibilityExport && midiEnabled)
		sampleList.samples.assign(1, 0);
	return sampleList;
}


// Convert XM envelope data to an OpenMPT's internal envelope representation.
void XMInstrument::ConvertEnvelopeToMPT(InstrumentEnvelope &mptEnv, uint8 numPoints, uint8 flags, uint8 sustain, uint8 loopStart, uint8 loopEnd, EnvType env) const
{
	mptEnv.resize(std::min(numPoints, uint8(12)));

	// Envelope Data
	for(uint32 i = 0; i < mptEnv.size(); i++)
	{
		switch(env)
		{
		case EnvTypeVol:
			mptEnv[i].tick = volEnv[i * 2];
			mptEnv[i].value = static_cast<EnvelopeNode::value_t>(volEnv[i * 2 + 1]);
			break;
		case EnvTypePan:
			mptEnv[i].tick = panEnv[i * 2];
			mptEnv[i].value = static_cast<EnvelopeNode::value_t>(panEnv[i * 2 + 1]);
			break;
		}

		if(i > 0 && mptEnv[i].tick < mptEnv[i - 1].tick && !(mptEnv[i].tick & 0xFF00))
		{
			// libmikmod code says: "Some broken XM editing program will only save the low byte of the position
			// value. Try to compensate by adding the missing high byte."
			// Note: MPT 1.07's XI instrument saver omitted the high byte of envelope nodes.
			// This might be the source for some broken envelopes in IT and XM files.
			mptEnv[i].tick |= static_cast<uint16>(mptEnv[i - 1].tick & 0xFF00u);
			if(mptEnv[i].tick < mptEnv[i - 1].tick)
				mptEnv[i].tick += 0x100;
		}
	}

	// Envelope Flags
	mptEnv.dwFlags.reset();
	if((flags & XMInstrument::envEnabled) != 0 && !mptEnv.empty()) mptEnv.dwFlags.set(ENV_ENABLED);

	// Envelope Loops
	if(sustain < 12)
	{
		if((flags & XMInstrument::envSustain) != 0) mptEnv.dwFlags.set(ENV_SUSTAIN);
		mptEnv.nSustainStart = mptEnv.nSustainEnd = sustain;
	}

	if(loopEnd < 12 && loopEnd >= loopStart)
	{
		if((flags & XMInstrument::envLoop) != 0) mptEnv.dwFlags.set(ENV_LOOP);
		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;
	}
}


// Convert an XMInstrument to OpenMPT's internal instrument representation.
void XMInstrument::ConvertToMPT(ModInstrument &mptIns) const
{
	mptIns.nFadeOut = volFade;

	// Convert envelopes
	ConvertEnvelopeToMPT(mptIns.VolEnv, volPoints, volFlags, volSustain, volLoopStart, volLoopEnd, EnvTypeVol);
	ConvertEnvelopeToMPT(mptIns.PanEnv, panPoints, panFlags, panSustain, panLoopStart, panLoopEnd, EnvTypePan);

	// Create sample assignment table
	for(std::size_t i = 0; i < std::size(sampleMap); i++)
	{
		mptIns.Keyboard[i + 12] = sampleMap[i];
	}

	if(midiEnabled)
	{
		mptIns.nMidiChannel = midiChannel + MidiFirstChannel;
		Limit(mptIns.nMidiChannel, uint8(MidiFirstChannel), uint8(MidiLastChannel));
		mptIns.nMidiProgram = static_cast<uint8>(std::min(static_cast<uint16>(midiProgram), uint16(127)) + 1);
	}
	mptIns.midiPWD = static_cast<int8>(pitchWheelRange);
}


// Apply auto-vibrato settings from sample to file.
void XMInstrument::ApplyAutoVibratoToXM(const ModSample &mptSmp, MODTYPE fromType)
{
	vibType = mptSmp.nVibType;
	vibSweep = mptSmp.nVibSweep;
	vibDepth = mptSmp.nVibDepth;
	vibRate = mptSmp.nVibRate;

	if((vibDepth | vibRate) != 0 && !(fromType & MOD_TYPE_XM))
	{
		if(mptSmp.nVibSweep != 0)
			vibSweep = mpt::saturate_cast<decltype(vibSweep)::base_type>(Util::muldivr_unsigned(mptSmp.nVibDepth, 256, mptSmp.nVibSweep));
		else
			vibSweep = 255;
	}
}


// Apply auto-vibrato settings from file to a sample.
void XMInstrument::ApplyAutoVibratoToMPT(ModSample &mptSmp) const
{
	mptSmp.nVibType = static_cast<VibratoType>(vibType.get());
	mptSmp.nVibSweep = vibSweep;
	mptSmp.nVibDepth = vibDepth;
	mptSmp.nVibRate = vibRate;
}


// Write stuff to the header that's always necessary (also for empty instruments)
void XMInstrumentHeader::Finalise()
{
	size = sizeof(XMInstrumentHeader);
	if(numSamples > 0)
	{
		sampleHeaderSize = sizeof(XMSample);
	} else
	{
		if(!instrument.midiEnabled)
			size -= sizeof(XMInstrument);
		sampleHeaderSize = 0;
	}
}


// Convert OpenMPT's internal sample representation to an XMInstrumentHeader.
XMInstrument::SampleList XMInstrumentHeader::ConvertToXM(const ModInstrument &mptIns, bool compatibilityExport)
{
	const auto sampleList = instrument.ConvertToXM(mptIns, compatibilityExport);
	numSamples = static_cast<uint16>(sampleList.samples.size());
	mpt::String::WriteBuf(mpt::String::spacePadded, name) = mptIns.name;

	type = mptIns.nMidiProgram;	// If FT2 writes crap here, we can do so, too! (we probably shouldn't, though. This is just for backwards compatibility with old MPT versions.)
	return sampleList;
}


// Convert an XMInstrumentHeader to OpenMPT's internal instrument representation.
void XMInstrumentHeader::ConvertToMPT(ModInstrument &mptIns) const
{
	instrument.ConvertToMPT(mptIns);

	// Create sample assignment table
	for(std::size_t i = 0; i < std::size(instrument.sampleMap); i++)
	{
		if(instrument.sampleMap[i] < numSamples)
		{
			mptIns.Keyboard[i + 12] = instrument.sampleMap[i];
		} else
		{
			mptIns.Keyboard[i + 12] = 0;
		}
	}

	mptIns.name = mpt::String::ReadBuf(mpt::String::spacePadded, name);

	// Old MPT backwards compatibility
	if(!instrument.midiEnabled)
	{
		mptIns.nMidiProgram = type;
	}
}


// Convert OpenMPT's internal sample representation to an XIInstrumentHeader.
XMInstrument::SampleList XIInstrumentHeader::ConvertToXM(const ModInstrument &mptIns, bool compatibilityExport)
{
	const auto sampleList = instrument.ConvertToXM(mptIns, compatibilityExport);
	numSamples = static_cast<uint16>(sampleList.samples.size());

	memcpy(signature, "Extended Instrument: ", 21);
	mpt::String::WriteBuf(mpt::String::spacePadded, name) = mptIns.name;
	eof = 0x1A;

	const std::string openMptTrackerName = mpt::ToCharset(mpt::Charset::CP437, Version::Current().GetOpenMPTVersionString());
	mpt::String::WriteBuf(mpt::String::spacePadded, trackerName) = openMptTrackerName;

	version = 0x102;
	return sampleList;
}


// Convert an XIInstrumentHeader to OpenMPT's internal instrument representation.
void XIInstrumentHeader::ConvertToMPT(ModInstrument &mptIns) const
{
	instrument.ConvertToMPT(mptIns);

	// Fix sample assignment table
	for(std::size_t i = 12; i < std::size(instrument.sampleMap) + 12; i++)
	{
		if(mptIns.Keyboard[i] >= numSamples)
		{
			mptIns.Keyboard[i] = 0;
		}
	}

	mptIns.name = mpt::String::ReadBuf(mpt::String::spacePadded, name);
}


// Convert OpenMPT's internal sample representation to an XMSample.
void XMSample::ConvertToXM(const ModSample &mptSmp, MODTYPE fromType, bool compatibilityExport)
{
	MemsetZero(*this);

	// Volume / Panning
	vol = static_cast<uint8>(std::min(mptSmp.nVolume / 4u, 64u));
	pan = static_cast<uint8>(std::min(mptSmp.nPan, uint16(255)));

	// Sample Frequency
	if((fromType & (MOD_TYPE_MOD | MOD_TYPE_XM)))
	{
		finetune = mptSmp.nFineTune;
		relnote = mptSmp.RelativeTone;
	} else
	{
		std::tie(relnote, finetune) = ModSample::FrequencyToTranspose(mptSmp.nC5Speed);
	}

	flags = 0;
	if(mptSmp.uFlags[CHN_PINGPONGLOOP])
		flags |= XMSample::sampleBidiLoop;
	else if(mptSmp.uFlags[CHN_LOOP])
		flags |= XMSample::sampleLoop;

	// Sample Length and Loops
	length = mpt::saturate_cast<uint32>(mptSmp.nLength);
	loopStart = mpt::saturate_cast<uint32>(mptSmp.nLoopStart);
	loopLength = mpt::saturate_cast<uint32>(mptSmp.nLoopEnd - mptSmp.nLoopStart);

	if(mptSmp.uFlags[CHN_16BIT])
	{
		flags |= XMSample::sample16Bit;
		length *= 2;
		loopStart *= 2;
		loopLength *= 2;
	}

	if(mptSmp.uFlags[CHN_STEREO] && !compatibilityExport)
	{
		flags |= XMSample::sampleStereo;
		length *= 2;
		loopStart *= 2;
		loopLength *= 2;
	}
}


// Convert an XMSample to OpenMPT's internal sample representation.
void XMSample::ConvertToMPT(ModSample &mptSmp) const
{
	mptSmp.Initialize(MOD_TYPE_XM);

	// Volume
	mptSmp.nVolume = vol * 4;
	LimitMax(mptSmp.nVolume, uint16(256));

	// Panning
	mptSmp.nPan = pan;
	mptSmp.uFlags = CHN_PANNING;

	// Sample Frequency
	mptSmp.nFineTune = finetune;
	mptSmp.RelativeTone = relnote;

	// Sample Length and Loops
	mptSmp.nLength = length;
	mptSmp.nLoopStart = loopStart;
	mptSmp.nLoopEnd = mptSmp.nLoopStart + loopLength;

	if((flags & XMSample::sample16Bit))
	{
		mptSmp.nLength /= 2;
		mptSmp.nLoopStart /= 2;
		mptSmp.nLoopEnd /= 2;
	}

	if((flags & XMSample::sampleStereo))
	{
		mptSmp.nLength /= 2;
		mptSmp.nLoopStart /= 2;
		mptSmp.nLoopEnd /= 2;
	}

	if((flags & (XMSample::sampleLoop | XMSample::sampleBidiLoop)) && mptSmp.nLoopEnd > mptSmp.nLoopStart)
	{
		mptSmp.uFlags.set(CHN_LOOP);
		if((flags & XMSample::sampleBidiLoop))
		{
			mptSmp.uFlags.set(CHN_PINGPONGLOOP);
		}
	}

	mptSmp.filename = "";
}


// Retrieve the internal sample format flags for this instrument.
SampleIO XMSample::GetSampleFormat() const
{
	if(reserved == sampleADPCM && !(flags & (XMSample::sample16Bit | XMSample::sampleStereo)))
	{
		// MODPlugin :(
		return SampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::ADPCM);
	}

	return SampleIO(
		(flags & XMSample::sample16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
		(flags & XMSample::sampleStereo) ? SampleIO::stereoSplit : SampleIO::mono,
		SampleIO::littleEndian,
		SampleIO::deltaPCM);
}


OPENMPT_NAMESPACE_END
