/*
 * Load_symmod.cpp
 * ---------------
 * Purpose: SymMOD (Symphonie / Symphonie Pro) module loader
 * Notes  : Based in part on Patrick Meng's Java-based Symphonie player and its source.
 *          Some effect behaviour and other things are based on the original Amiga assembly source.
 *          Symphonie is an interesting beast, with a surprising combination of features and lack thereof.
 *          It offers advanced DSPs (for its time) but has a fixed track tempo. It can handle stereo samples
 *          but free panning support was only added in one of the very last versions. Still, a good number
 *          of high-quality modules were made with it despite (or because of) its lack of features.
 * Authors: Devin Acker
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "Loaders.h"
#include "Mixer.h"
#include "MixFuncTable.h"
#include "modsmp_ctrl.h"
#include "openmpt/soundbase/SampleConvert.hpp"
#include "openmpt/soundbase/SampleConvertFixedPoint.hpp"
#include "openmpt/soundbase/SampleDecode.hpp"
#include "SampleCopy.h"
#ifdef MPT_EXTERNAL_SAMPLES
#include "../common/mptPathString.h"
#endif  // MPT_EXTERNAL_SAMPLES
#include "mpt/base/numbers.hpp"

#include <map>

OPENMPT_NAMESPACE_BEGIN

struct SymFileHeader
{
	char     magic[4];  // "SymM"
	uint32be version;
	// Technically this is already the first chunk; for simplicity we always assume that the channel count comes first (which in practice it does)
	int32be  firstChunkID;
	uint32be numChannels;

	bool Validate() const
	{
		return !std::memcmp(magic, "SymM", 4)
			&& version == 1
			&& firstChunkID == -1
			&& numChannels > 0 && numChannels <= 256;
	}
};

MPT_BINARY_STRUCT(SymFileHeader, 16)


struct SymEvent
{
	enum Command : uint8
	{
		KeyOn = 0,
		VolSlideUp,
		VolSlideDown,
		PitchSlideUp,
		PitchSlideDown,
		ReplayFrom,
		FromAndPitch,
		SetFromAdd,
		FromAdd,
		SetSpeed,
		AddPitch,
		AddVolume,
		Tremolo,
		Vibrato,
		SampleVib,
		PitchSlideTo,
		Retrig,
		Emphasis,
		AddHalfTone,
		CV,
		CVAdd,

		Filter = 23,
		DSPEcho,
		DSPDelay,
	};

	enum Volume : uint8
	{
		VolCommand  = 200,
		StopSample  = 254,
		ContSample  = 253,
		StartSample = 252,  // unused
		KeyOff      = 251,
		SpeedDown   = 250,
		SpeedUp     = 249,
		SetPitch    = 248,
		PitchUp     = 247,
		PitchDown   = 246,
		PitchUp2    = 245,
		PitchDown2  = 244,
		PitchUp3    = 243,
		PitchDown3  = 242
	};

	uint8be command;  // See Command enum
	int8be note;
	uint8be param;  // Volume if <= 100, see Volume enum otherwise
	uint8be inst;
	
	bool IsGlobal() const
	{
		if(command == SymEvent::SetSpeed || command == SymEvent::DSPEcho || command == SymEvent::DSPDelay)
			return true;
		if(command == SymEvent::KeyOn && (param == SymEvent::SpeedUp || param == SymEvent::SpeedDown))
			return true;
		return false;
	}

	// used to compare DSP events for mapping them to MIDI macro numbers
	bool operator<(const SymEvent &other) const
	{
		return std::tie(command, note, param, inst) < std::tie(other.command, other.note, other.param, other.inst);
	}
};

MPT_BINARY_STRUCT(SymEvent, 4)


struct SymVirtualHeader
{
	char     id[4];  // "ViRT"
	uint8be  zero;
	uint8be  filler1;
	uint16be version;  // 0 = regular, 1 = transwave
	uint16be mixInfo;  // unused, but not 0 in all modules
	uint16be filler2;
	uint16be eos;      // 0

	uint16be numEvents;
	uint16be maxEvents;  // always 20
	uint16be eventSize;  // 4 for virtual instruments, 10 for transwave instruments (number of cycles, not used)

	bool IsValid() const
	{
		return !memcmp(id, "ViRT", 4) && zero == 0 && version <= 1 && eos == 0 && maxEvents == 20;
	}

	bool IsVirtual() const
	{
		return IsValid() && version == 0 && numEvents <= 20 && eventSize == sizeof(SymEvent);
	}

	bool IsTranswave() const
	{
		return IsValid() && version == 1 && numEvents == 2 && eventSize == 10;
	}
};

MPT_BINARY_STRUCT(SymVirtualHeader, 20)


// Virtual instrument info
// This allows instruments to be created based on a mix of other instruments.
// The sample mixing is done at load time.
struct SymVirtualInst
{
	SymVirtualHeader header;
	SymEvent noteEvents[20];
	char padding[28];

	bool Render(CSoundFile &sndFile, const bool asQueue, ModSample &target, uint16 sampleBoost) const
	{
		if(header.numEvents < 1 || header.numEvents > std::size(noteEvents) || noteEvents[0].inst >= sndFile.GetNumSamples())
			return false;

		target.Initialize(MOD_TYPE_IT);
		target.uFlags = CHN_16BIT;

		const auto events = mpt::as_span(noteEvents).subspan(0, header.numEvents);
		const double rateFactor = 1.0 / std::max(sndFile.GetSample(events[0].inst + 1).nC5Speed, uint32(1));

		for(const auto &event : events.subspan(0, asQueue ? events.size() : 1u))
		{
			if(event.inst >= sndFile.GetNumSamples() || event.note < 0)
				continue;
			const ModSample &sourceSmp = sndFile.GetSample(event.inst + 1);
			const double length = sourceSmp.nLength * std::pow(2.0, (event.note - events[0].note) / -12.0) * sourceSmp.nC5Speed * rateFactor;
			target.nLength += mpt::saturate_round<SmpLength>(length);
		}
		if(!target.AllocateSample())
			return false;

		std::vector<ModChannel> channels(events.size());
		SmpLength lastSampleOffset = 0;
		for(size_t ev = 0; ev < events.size(); ev++)
		{
			const SymEvent &event = events[ev];
			ModChannel &chn = channels[ev];

			if(event.inst >= sndFile.GetNumSamples() || event.note < 0)
				continue;

			int8 finetune = 0;
			if(event.param >= SymEvent::PitchDown3 && event.param <= SymEvent::PitchUp)
			{
				static constexpr int8 PitchTable[] = {-4, 4, -2, 2, -1, 1};
				static_assert(mpt::array_size<decltype(PitchTable)>::size == SymEvent::PitchUp - SymEvent::PitchDown3 + 1);
				finetune = PitchTable[event.param - SymEvent::PitchDown3];
			}

			const ModSample &sourceSmp = sndFile.GetSample(event.inst + 1);
			const double increment = std::pow(2.0, (event.note - events[0].note) / 12.0 + finetune / 96.0) * sourceSmp.nC5Speed * rateFactor;
			if(increment <= 0)
				continue;

			chn.increment = SamplePosition::FromDouble(increment);
			chn.pCurrentSample = sourceSmp.samplev();
			chn.nLength = sourceSmp.nLength;
			chn.dwFlags = sourceSmp.uFlags & CHN_SAMPLEFLAGS;
			if(asQueue)
			{
				// This determines when the queued sample will be played
				chn.oldOffset = lastSampleOffset;
				lastSampleOffset += mpt::saturate_round<SmpLength>(chn.nLength / chn.increment.ToDouble());
			}
			int32 volume = 4096 * sampleBoost / 10000;  // avoid clipping the filters if the virtual sample is later also filtered (see e.g. 303 emulator.symmod)
			if(!asQueue)
				volume /= header.numEvents;
			chn.leftVol = chn.rightVol = volume;
		}

		SmpLength writeOffset = 0;
		while(writeOffset < target.nLength)
		{
			std::array<mixsample_t, MIXBUFFERSIZE * 2> buffer{};
			const SmpLength writeCount = std::min(static_cast<SmpLength>(MIXBUFFERSIZE), target.nLength - writeOffset);

			for(auto &chn : channels)
			{
				if(!chn.pCurrentSample)
					continue;
				// Should queued sample be played yet?
				if(chn.oldOffset >= writeCount)
				{
					chn.oldOffset -= writeCount;
					continue;
				}

				uint32 functionNdx = MixFuncTable::ndxLinear;
				if(chn.dwFlags[CHN_16BIT])
					functionNdx |= MixFuncTable::ndx16Bit;
				if(chn.dwFlags[CHN_STEREO])
					functionNdx |= MixFuncTable::ndxStereo;

				const SmpLength procCount = std::min(writeCount - chn.oldOffset, mpt::saturate_round<SmpLength>((chn.nLength - chn.position.ToDouble()) / chn.increment.ToDouble()));
				MixFuncTable::Functions[functionNdx](chn, sndFile.m_Resampler, buffer.data() + chn.oldOffset * 2, procCount);
				chn.oldOffset = 0;
				if(chn.position.GetUInt() >= chn.nLength)
					chn.pCurrentSample = nullptr;
			}
			CopySample<SC::ConversionChain<SC::ConvertFixedPoint<int16, mixsample_t, 27>, SC::DecodeIdentity<mixsample_t>>>(target.sample16() + writeOffset, writeCount, 1, buffer.data(), sizeof(buffer), 2);
			writeOffset += writeCount;
		}

		return true;
	}
};

MPT_BINARY_STRUCT(SymVirtualInst, 128)


// Transwave instrument info
// Similar to virtual instruments, allows blending between two sample loops
struct SymTranswaveInst
{
	struct Transwave
	{
		uint16be sourceIns;
		uint16be volume;  // According to source label - but appears to be unused
		uint32be loopStart;
		uint32be loopLen;
		uint32be padding;

		std::pair<SmpLength, SmpLength> ConvertLoop(const ModSample &mptSmp) const
		{
			const double loopScale = static_cast<double>(mptSmp.nLength) / (100 << 16);
			const SmpLength start  = mpt::saturate_trunc<SmpLength>(loopScale * std::min(uint32(100 << 16), loopStart.get()));
			const SmpLength length = mpt::saturate_trunc<SmpLength>(loopScale * std::min(uint32(100 << 16), loopLen.get()));
			return {start, std::min(mptSmp.nLength - start, length)};
		}
	};

	SymVirtualHeader header;
	Transwave points[2];
	char padding[76];

	// Morph between two sample loops
	bool Render(const ModSample &smp1, const ModSample &smp2, ModSample &target) const
	{
		target.Initialize(MOD_TYPE_IT);

		const auto [loop1Start, loop1Len] = points[0].ConvertLoop(smp1);
		const auto [loop2Start, loop2Len] = points[1].ConvertLoop(smp2);

		if(loop1Len < 1 || loop1Len > MAX_SAMPLE_LENGTH / (4u * 80u))
			return false;

		const SmpLength cycleLength = loop1Len * 4u;
		const double cycleFactor1 = loop1Len / static_cast<double>(cycleLength);
		const double cycleFactor2 = loop2Len / static_cast<double>(cycleLength);

		target.uFlags = CHN_16BIT;
		target.nLength = cycleLength * 80u;
		if(!target.AllocateSample())
			return false;

		const double ampFactor = 1.0 / target.nLength;
		for(SmpLength i = 0; i < cycleLength; i++)
		{
			const double v1 = TranswaveInterpolate(smp1, loop1Start + i * cycleFactor1);
			const double v2 = TranswaveInterpolate(smp2, loop2Start + i * cycleFactor2);
			SmpLength writeOffset = i;
			for(int cycle = 0; cycle < 80; cycle++, writeOffset += cycleLength)
			{
				const double amp = writeOffset * ampFactor;
				target.sample16()[writeOffset] = mpt::saturate_round<int16>(v1 * (1.0 - amp) + v2 * amp);
			}
		}

		return true;
	}

	static MPT_FORCEINLINE double TranswaveInterpolate(const ModSample &smp, double offset)
	{
		if(!smp.HasSampleData())
			return 0.0;

		SmpLength intOffset = static_cast<SmpLength>(offset);
		const double fractOffset = offset - intOffset;
		const uint8 numChannels = smp.GetNumChannels();
		intOffset *= numChannels;

		int16 v1, v2;
		if(smp.uFlags[CHN_16BIT])
		{
			v1 = smp.sample16()[intOffset];
			v2 = smp.sample16()[intOffset + numChannels];
		} else
		{
			v1 = smp.sample8()[intOffset] * 256;
			v2 = smp.sample8()[intOffset + numChannels] * 256;
		}
		return (v1 * (1.0 - fractOffset) + v2 * fractOffset);
	}
};

MPT_BINARY_STRUCT(SymTranswaveInst, 128)


// Instrument definition
struct SymInstrument
{
	using SymInstrumentName = std::array<char, 128>;

	SymVirtualInst virt;  // or SymInstrumentName, or SymTranswaveInst

	enum Type : int8
	{
		Silent  = -8,
		Kill    = -4,
		Normal  = 0,
		Loop    = 4,
		Sustain = 8
	};

	enum Channel : uint8
	{
		Mono,
		StereoL,
		StereoR,
		LineSrc  // virtual mix instrument
	};

	enum SampleFlags : uint8
	{
		PlayReverse   = 1,   // reverse sample
		AsQueue       = 2,   // "queue" virtual instrument (rendereds samples one after another rather than simultaneously)
		MirrorX       = 4,   // invert sample phase
		Is16Bit       = 8,   // not used, we already know the bit depth of the samples
		NewLoopSystem = 16,  // use fine loop start/len values

		MakeNewSample = (PlayReverse | MirrorX)
	};

	enum InstFlags : uint8
	{
		NoTranspose = 1,  // don't apply sequence/position transpose
		NoDSP       = 2,  // don't apply DSP effects
		SyncPlay    = 4   // play a stereo instrument pair (or two copies of the same mono instrument) on consecutive channels
	};

	int8be   type;  // see Type enum
	uint8be  loopStartHigh;
	uint8be  loopLenHigh;
	uint8be  numRepetitions;  // for "sustain" instruments
	uint8be  channel;         // see Channel enum
	uint8be  dummy1;          // called "automaximize" (normalize?) in Amiga source, but unused
	uint8be  volume;          // 0-199
	uint8be  dummy2[3];       // info about "parent/child" and sample format
	int8be   finetune;        // -128..127 ~= 2 semitones
	int8be   transpose;
	uint8be  sampleFlags;  // see SampleFlags enum
	int8be   filter;       // negative: highpass, positive: lowpass
	uint8be  instFlags;    // see InstFlags enum
	uint8be  downsample;   // downsample factor; affects sample tuning
	uint8be  dummy3[2];    // resonance, "loadflags" (both unused)
	uint8be  info;         // bit 0 should indicate that rangeStart/rangeLen are valid, but they appear to be unused
	uint8be  rangeStart;   // ditto
	uint8be  rangeLen;     // ditto
	uint8be  dummy4;
	uint16be loopStartFine;
	uint16be loopLenFine;
	uint8be  dummy5[6];

	uint8be filterFlags;      // bit 0 = enable, bit 1 = highpass
	uint8be numFilterPoints;  // # of filter envelope points (up to 4, possibly only 1-2 ever actually used)
	struct SymFilterSetting
	{
		uint8be cutoff;
		uint8be resonance;
	} filterPoint[4];

	uint8be volFadeFlag;
	uint8be volFadeFrom;
	uint8be volFadeTo;
	
	uint8be padding[83];

	bool IsVirtual() const
	{
		return virt.header.IsValid();
	}

	// Valid instrument either is virtual or has a name
	bool IsEmpty() const
	{
		return virt.header.id[0] == 0 || type < 0;
	}
	
	std::string GetName() const
	{
		return mpt::String::ReadBuf(mpt::String::maybeNullTerminated, mpt::bit_cast<SymInstrumentName>(virt));
	}

	SymTranswaveInst GetTranswave() const
	{
		return mpt::bit_cast<SymTranswaveInst>(virt);
	}

	void ConvertToMPT(ModInstrument &mptIns, ModSample &mptSmp, CSoundFile &sndFile) const
	{
		if(!IsVirtual())
			mptIns.name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, mpt::bit_cast<SymInstrumentName>(virt));

		mptSmp.uFlags.reset(CHN_LOOP | CHN_PINGPONGLOOP | CHN_SUSTAINLOOP | CHN_PANNING);  // Avoid these coming in from sample files

		const auto [loopStart, loopLen] = GetSampleLoop(mptSmp);
		if(type == Loop && loopLen > 0)
		{
			mptSmp.uFlags.set(CHN_LOOP);
			mptSmp.nLoopStart = loopStart;
			mptSmp.nLoopEnd   = loopStart + loopLen;
		}

		// volume (0-199, default 100)
		// Symphonie actually compresses the sample data if the volume is above 100 (see end of function)
		// We spread the volume between sample and instrument global volume if it's below 100 for the best possible resolution.
		// This can be simplified if instrument volume ever gets adjusted to 0...128 range like in IT.
		uint8 effectiveVolume = (volume > 0 && volume < 200) ? static_cast<uint8>(std::min(volume.get(), uint8(100)) * 128u / 100) : 128;
		mptSmp.nGlobalVol     = std::max(effectiveVolume, uint8(64)) / 2u;
		mptIns.nGlobalVol     = std::min(effectiveVolume, uint8(64));

		// Tuning info (we'll let our own mixer take care of the downsampling instead of doing it at load time)
		mptSmp.nC5Speed = 40460;
		mptSmp.Transpose(-downsample + (transpose / 12.0) + (finetune / (128.0 * 12.0)));

		// DSP settings
		mptIns.nMixPlug = (instFlags & NoDSP) ? 2 : 1;
		if(instFlags & NoDSP)
		{
			// This is not 100% correct: An instrument playing after this one should pick up previous filter settings.
			mptIns.SetCutoff(127, true);
			mptIns.SetResonance(0, true);
		}

		// Various sample processing follows
		if(!mptSmp.HasSampleData())
			return;

		if(sampleFlags & PlayReverse)
			ctrlSmp::ReverseSample(mptSmp, 0, 0, sndFile);
		if(sampleFlags & MirrorX)
			ctrlSmp::InvertSample(mptSmp, 0, 0, sndFile);

		// Always use 16-bit data to help with heavily filtered 8-bit samples (like in Future_Dream.SymMOD)
		const bool doVolFade = (volFadeFlag == 2) && (volFadeFrom <= 100) && (volFadeTo <= 100);
		if(!mptSmp.uFlags[CHN_16BIT] && (filterFlags || doVolFade || filter))
		{
			int16 *newSample = static_cast<int16 *>(ModSample::AllocateSample(mptSmp.nLength, 2 * mptSmp.GetNumChannels()));
			if(!newSample)
				return;
			CopySample<SC::ConversionChain<SC::Convert<int16, int8>, SC::DecodeIdentity<int8>>>(newSample, mptSmp.nLength * mptSmp.GetNumChannels(), 1, mptSmp.sample8(), mptSmp.GetSampleSizeInBytes(), 1);
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.ReplaceWaveform(newSample, mptSmp.nLength, sndFile);
		}

		// Highpass
		if(filter < 0)
		{
			auto sampleData = mpt::as_span(mptSmp.sample16(), mptSmp.nLength * mptSmp.GetNumChannels());
			for(int i = 0; i < -filter; i++)
			{
				int32 mix = sampleData[0];
				for(auto &sample : sampleData)
				{
					mix    = mpt::rshift_signed(sample - mpt::rshift_signed(mix, 1), 1);
					sample = static_cast<int16>(mix);
				}
			}
		}

		// Volume Fade
		if(doVolFade)
		{
			auto sampleData = mpt::as_span(mptSmp.sample16(), mptSmp.nLength * mptSmp.GetNumChannels());
			int32 amp = volFadeFrom << 24, inc = Util::muldivr(volFadeTo - volFadeFrom, 1 << 24, static_cast<SmpLength>(sampleData.size()));
			for(auto &sample : sampleData)
			{
				sample = static_cast<int16>(Util::muldivr(sample, amp, 100 << 24));
				amp += inc;
			}
		}

		// Resonant Filter Sweep
		if(filterFlags != 0)
		{
			auto sampleData = mpt::as_span(mptSmp.sample16(), mptSmp.nLength * mptSmp.GetNumChannels());
			int32 cutoff = filterPoint[0].cutoff << 23, resonance = filterPoint[0].resonance << 23;
			const int32 cutoffStep = numFilterPoints > 1 ? Util::muldivr(filterPoint[1].cutoff - filterPoint[0].cutoff, 1 << 23, static_cast<SmpLength>(sampleData.size())) : 0;
			const int32 resoStep   = numFilterPoints > 1 ? Util::muldivr(filterPoint[1].resonance - filterPoint[0].resonance, 1 << 23, static_cast<SmpLength>(sampleData.size())) : 0;
			const uint8 highpass   = filterFlags & 2;

			int32 filterState[3]{};
			for(auto &sample : sampleData)
			{
				const int32 currentCutoff = cutoff / (1 << 23), currentReso = resonance / (1 << 23);
				cutoff += cutoffStep;
				resonance += resoStep;

				filterState[2] = mpt::rshift_signed(sample, 1) - filterState[0];
				filterState[1] += mpt::rshift_signed(currentCutoff * filterState[2], 8);
				filterState[0] += mpt::rshift_signed(currentCutoff * filterState[1], 6);
				filterState[0] += mpt::rshift_signed(currentReso * filterState[0], 6);
				filterState[0] = mpt::rshift_signed(filterState[0], 2);
				sample = mpt::saturate_cast<int16>(filterState[highpass]);
			}
		}

		// Lowpass
		if(filter > 0)
		{
			auto sampleData = mpt::as_span(mptSmp.sample16(), mptSmp.nLength * mptSmp.GetNumChannels());
			for(int i = 0; i < filter; i++)
			{
				int32 mix = sampleData[0];
				for(auto &sample : sampleData)
				{
					mix = (sample + sample + mix) / 3;
					sample = static_cast<int16>(mix);
				}
			}
		}

		// Symphonie normalizes samples at load time (it normalizes them to the sample boost value - but we will use the full 16-bit range)
		// Indeed, the left and right channel instruments are normalized separately.
		const auto Normalize = [](auto sampleData)
		{
			const auto scale              = Util::MaxValueOfType(sampleData[0]);
			const auto [minElem, maxElem] = std::minmax_element(sampleData.begin(), sampleData.end());
			const int max                 = std::max(-*minElem, +*maxElem);
			if(max >= scale || max == 0)
				return;

			for(auto &v : sampleData)
			{
				v = static_cast<typename std::remove_reference<decltype(v)>::type>(static_cast<int>(v) * scale / max);
			}
		};
		if(mptSmp.uFlags[CHN_16BIT])
			Normalize(mpt::as_span(mptSmp.sample16(), mptSmp.nLength * mptSmp.GetNumChannels()));
		else
			Normalize(mpt::as_span(mptSmp.sample8(), mptSmp.nLength * mptSmp.GetNumChannels()));

		// "Non-destructive" over-amplification with hard knee compression
		if(volume > 100 && volume < 200)
		{
			const auto Amplify = [](auto sampleData, const uint8 gain)
			{
				const int32 knee = 16384 * (200 - gain) / 100, kneeInv = 32768 - knee;
				constexpr int32 scale = 1 << (16 - (sizeof(sampleData[0]) * 8));
				for(auto &sample : sampleData)
				{
					int32 v = sample * scale;
					if(v > knee)
						v = (v - knee) * knee / kneeInv + kneeInv;
					else if(v < -knee)
						v = (v + knee) * knee / kneeInv - kneeInv;
					else
						v = v * kneeInv / knee;
					sample = mpt::saturate_cast<typename std::remove_reference<decltype(sample)>::type>(v / scale);
				}
			};

			const auto length = mptSmp.nLength * mptSmp.GetNumChannels();
			if(mptSmp.uFlags[CHN_16BIT])
				Amplify(mpt::span(mptSmp.sample16(), mptSmp.sample16() + length), volume);
			else
				Amplify(mpt::span(mptSmp.sample8(), mptSmp.sample8() + length), volume);
		}

		// This must be applied last because some sample processors are time-dependent and Symphonie would be doing this during playback instead
		mptSmp.RemoveAllCuePoints();
		if(type == Sustain && numRepetitions > 0 && loopLen > 0)
		{
			mptSmp.cues[0] = loopStart + loopLen * (numRepetitions + 1u);
			mptSmp.nSustainStart = loopStart;  // This is of purely informative value and not used for playback
			mptSmp.nSustainEnd   = loopStart + loopLen;

			if(MAX_SAMPLE_LENGTH / numRepetitions < loopLen)
				return;
			if(MAX_SAMPLE_LENGTH - numRepetitions * loopLen < mptSmp.nLength)
				return;

			const uint8 bps      = mptSmp.GetBytesPerSample();
			SmpLength loopEnd    = loopStart + loopLen * (numRepetitions + 1);
			SmpLength newLength  = mptSmp.nLength + loopLen * numRepetitions;
			std::byte *newSample = static_cast<std::byte *>(ModSample::AllocateSample(newLength, bps));
			if(!newSample)
				return;

			std::memcpy(newSample, mptSmp.sampleb(), (loopStart + loopLen) * bps);
			for(uint8 i = 0; i < numRepetitions; i++)
			{
				std::memcpy(newSample + (loopStart + loopLen * (i + 1)) * bps, mptSmp.sampleb() + loopStart * bps, loopLen * bps);
			}
			std::memcpy(newSample + loopEnd * bps, mptSmp.sampleb() + (loopStart + loopLen) * bps, (newLength - loopEnd) * bps);
			
			mptSmp.ReplaceWaveform(newSample, newLength, sndFile);
		}
	}

	std::pair<SmpLength, SmpLength> GetSampleLoop(const ModSample &mptSmp) const
	{
		if(type != Loop && type != Sustain)
			return {0, 0};

		SmpLength loopStart = std::min(loopStartHigh.get(), uint8(100));
		SmpLength loopLen = std::min(loopLenHigh.get(), uint8(100));
		if(sampleFlags & NewLoopSystem)
		{
			loopStart = (loopStart << 16) + loopStartFine;
			loopLen = (loopLen << 16) + loopLenFine;

			const double loopScale = static_cast<double>(mptSmp.nLength) / (100 << 16);
			loopStart = std::min(mptSmp.nLength, mpt::saturate_trunc<SmpLength>(loopStart * loopScale));
			loopLen = std::min(mptSmp.nLength - loopStart, mpt::saturate_trunc<SmpLength>(loopLen * loopScale));
		} else if(mptSmp.HasSampleData())
		{
			// The order of operations here may seem weird as it reduces precision, but it's taken directly from the original assembly source (UpdateRecalcLoop)
			loopStart = ((loopStart << 7) / 100u) * (mptSmp.nLength >> 7);
			loopLen = std::min(mptSmp.nLength - loopStart, ((loopLen << 7) / 100u) * (mptSmp.nLength >> 7));

			const auto FindLoopEnd = [](auto sampleData, const uint8 numChannels, SmpLength loopStart, SmpLength loopLen, const int threshold)
			{
				const auto valAtStart = sampleData.data()[loopStart * numChannels];
				auto *endPtr = sampleData.data() + (loopStart + loopLen) * numChannels;
				while(loopLen)
				{
					if(std::abs(*endPtr - valAtStart) < threshold)
						return loopLen;
					endPtr -= numChannels;
					loopLen--;
				}
				return loopLen;
			};
			if(mptSmp.uFlags[CHN_16BIT])
				loopLen = FindLoopEnd(mpt::as_span(mptSmp.sample16(), mptSmp.nLength * mptSmp.GetNumChannels()), mptSmp.GetNumChannels(), loopStart, loopLen, 6 * 256);
			else
				loopLen = FindLoopEnd(mpt::as_span(mptSmp.sample8(), mptSmp.nLength * mptSmp.GetNumChannels()), mptSmp.GetNumChannels(), loopStart, loopLen, 6);
		}

		return {loopStart, loopLen};
	}
};

MPT_BINARY_STRUCT(SymInstrument, 256)


struct SymSequence
{
	uint16be start;
	uint16be length;
	uint16be loop;
	int16be  info;
	int16be  transpose;

	uint8be  padding[6];
};

MPT_BINARY_STRUCT(SymSequence, 16)


struct SymPosition
{
	uint8be  dummy[4];
	uint16be loopNum;
	uint16be loopCount;  // Only used during playback
	uint16be pattern;
	uint16be start;
	uint16be length;
	uint16be speed;
	int16be  transpose;
	uint16be eventsPerLine;  // Unused

	uint8be  padding[12];

	// Used to compare position entries for mapping them to OpenMPT patterns
	bool operator<(const SymPosition &other) const
	{
		return std::tie(pattern, start, length, transpose, speed) < std::tie(other.pattern, other.start, other.length, other.transpose, other.speed);
	}
};

MPT_BINARY_STRUCT(SymPosition, 32)


static std::vector<std::byte> DecodeSymChunk(FileReader &file)
{
	std::vector<std::byte> data;
	const uint32 packedLength = file.ReadUint32BE();
	if(!file.CanRead(packedLength))
	{
		file.Skip(file.BytesLeft());
		return data;
	}

	FileReader chunk = file.ReadChunk(packedLength);
	if(packedLength >= 10 && chunk.ReadMagic("PACK\xFF\xFF"))
	{
		// RLE-compressed chunk
		uint32 unpackedLength = chunk.ReadUint32BE();
		// The best compression ratio can be achieved with type 1, where six bytes turn into up to 255*4 bytes, a ratio of 1:170.
		uint32 maxLength = packedLength - 10;
		if(Util::MaxValueOfType(maxLength) / 170 >= maxLength)
			maxLength *= 170;
		else
			maxLength = Util::MaxValueOfType(maxLength);
		LimitMax(unpackedLength, maxLength);
		data.resize(unpackedLength);

		bool done = false;
		uint32 offset = 0, remain = unpackedLength;

		while(!done && !chunk.EndOfFile())
		{
			uint8 len;
			std::array<std::byte, 4> dword;

			const int8 type = chunk.ReadInt8();
			switch(type)
			{
			case 0:
				// Copy raw bytes
				len = chunk.ReadUint8();
				if(remain >= len && chunk.CanRead(len))
				{
					chunk.ReadRaw(mpt::as_span(data).subspan(offset, len));
					offset += len;
					remain -= len;
				} else
				{
					done = true;
				}
				break;

			case 1:
				// Copy a dword multiple times
				len = chunk.ReadUint8();
				if(remain >= (len * 4u) && chunk.ReadArray(dword))
				{
					remain -= len * 4u;
					while(len--)
					{
						std::copy(dword.begin(), dword.end(), data.begin() + offset);
						offset += 4;
					}
				} else
				{
					done = true;
				}
				break;

			case 2:
				// Copy a dword twice
				if(remain >= 8 && chunk.ReadArray(dword))
				{
					std::copy(dword.begin(), dword.end(), data.begin() + offset);
					std::copy(dword.begin(), dword.end(), data.begin() + offset + 4);
					offset += 8;
					remain -= 8;
				} else
				{
					done = true;
				}
				break;

			case 3:
				// Zero bytes
				len = chunk.ReadUint8();
				if(remain >= len)
				{
					// vector is already initialized to zero
					offset += len;
					remain -= len;
				} else
				{
					done = true;
				}
				break;

			case -1:
				done = true;
				break;

			default:
				// error
				done = true;
				break;
			}
		}

#ifndef MPT_BUILD_FUZZER
		// When using a fuzzer, we should not care if the decompressed buffer has the correct size.
		// This makes finding new interesting test cases much easier.
		if(remain)
			mpt::reconstruct(data);
#endif
	} else
	{
		// Uncompressed chunk
		chunk.ReadVector(data, packedLength);
	}

	return data;
}


template<typename T>
static std::vector<T> DecodeSymArray(FileReader &file)
{
	const auto data = DecodeSymChunk(file);
	FileReader chunk(mpt::as_span(data));
	std::vector<T> retVal;
	chunk.ReadVector(retVal, data.size() / sizeof(T));
	return retVal;
}


static bool ReadRawSymSample(ModSample &sample, FileReader &file)
{
	SampleIO sampleIO(SampleIO::_16bit, SampleIO::mono, SampleIO::bigEndian, SampleIO::signedPCM);
	SmpLength nullBytes = 0;
	sample.Initialize();

	file.Rewind();
	if(file.ReadMagic("MAESTRO"))
	{
		file.Seek(12);
		if(file.ReadUint32BE() == 0)
			sampleIO |= SampleIO::stereoInterleaved;
		file.Seek(24);
	} else if(file.ReadMagic("16BT"))
	{
		file.Rewind();
		nullBytes = 4;  // In Symphonie, the anti-click would take care of those...
	} else
	{
		sampleIO |= SampleIO::_8bit;
	}

	sample.nLength = mpt::saturate_cast<SmpLength>(file.BytesLeft() / (sampleIO.GetNumChannels() * sampleIO.GetBitDepth() / 8u));
	const bool ok = sampleIO.ReadSample(sample, file) > 0;

	if(ok && nullBytes)
		std::memset(sample.samplev(), 0, std::min(nullBytes, sample.GetSampleSizeInBytes()));

	return ok;
}


static std::vector<std::byte> DecodeSample8(FileReader &file)
{
	auto data = DecodeSymChunk(file);
	uint8 lastVal = 0;
	for(auto &val : data)
	{
		lastVal += mpt::byte_cast<uint8>(val);
		val = mpt::byte_cast<std::byte>(lastVal);
	}

	return data;
}


static std::vector<std::byte> DecodeSample16(FileReader &file)
{
	auto data = DecodeSymChunk(file);
	std::array<std::byte, 4096> buf;
	constexpr size_t blockSize = buf.size() / 2;  // Size of block in 16-bit samples

	for(size_t block = 0; block < data.size() / buf.size(); block++)
	{
		const size_t offset = block * sizeof(buf);
		uint8 lastVal = 0;

		// Decode LSBs
		for(size_t i = 0; i < blockSize; i++)
		{
			lastVal += mpt::byte_cast<uint8>(data[offset + i]);
			buf[i * 2 + 1] = mpt::byte_cast<std::byte>(lastVal);
		}
		// Decode MSBs
		for(size_t i = 0; i < blockSize; i++)
		{
			lastVal += mpt::byte_cast<uint8>(data[offset + i + blockSize]);
			buf[i * 2] = mpt::byte_cast<std::byte>(lastVal);
		}

		std::copy(buf.begin(), buf.end(), data.begin() + offset);
	}

	return data;
}


static bool ConvertDSP(const SymEvent event, MIDIMacroConfigData::Macro &macro, const CSoundFile &sndFile)
{
	if(event.command == SymEvent::Filter)
	{
		// Symphonie practically uses the same filter for this as for the sample processing.
		// The cutoff and resonance are an approximation.
		const uint8 type = static_cast<uint8>(event.note % 5u);
		const uint8 cutoff = sndFile.FrequencyToCutOff(event.param * 10000.0 / 240.0);
		const uint8 reso = static_cast<uint8>(std::min(127, event.inst * 127 / 185));

		if(type == 1)  // lowpass filter
			macro = MPT_AFORMAT("F0F000{} F0F001{} F0F00200")(mpt::afmt::HEX0<2>(cutoff), mpt::afmt::HEX0<2>(reso));
		else if(type == 2)  // highpass filter
			macro = MPT_AFORMAT("F0F000{} F0F001{} F0F00210")(mpt::afmt::HEX0<2>(cutoff), mpt::afmt::HEX0<2>(reso));
		else  // no filter or unsupported filter type
			macro = "F0F0007F F0F00100";
		return true;
	} else if(event.command == SymEvent::DSPEcho)
	{
		const uint8 type = (event.note < 5) ? event.note : 0;
		const uint8 length = (event.param < 128) ? event.param : 127;
		const uint8 feedback = (event.inst < 128) ? event.inst : 127;
		macro = MPT_AFORMAT("F0F080{} F0F081{} F0F082{}")(mpt::afmt::HEX0<2>(type), mpt::afmt::HEX0<2>(length), mpt::afmt::HEX0<2>(feedback));
		return true;
	} else if(event.command == SymEvent::DSPDelay)
	{
		// DSP first has to be turned on from the Symphonie GUI before it can be used in a track (unlike Echo),
		// so it's not implemented for now.
		return false;
	}
	return false;
}


static uint8 MapToClosestMidiMacro(const SymEvent event, std::map<SymEvent, uint8> &macroMap)
{
	if(event.command == SymEvent::DSPDelay)
		return 0;
	uint8 bestMatch = 0;
	uint32 bestDistance = uint32_max;
	for(const auto &m : macroMap)
	{
		const auto &mapEvent = m.first;
		if(event.command != mapEvent.command || event.note != mapEvent.note)
			continue;
		const uint32 diff1 = static_cast<uint32>(event.param) - mapEvent.param, diff2 = static_cast<uint32>(event.inst) - mapEvent.inst;
		const uint32 distance = diff1 * diff1 + diff2 * diff2;
		if(distance >= bestDistance)
			continue;

		bestMatch = m.second;
		bestDistance = distance;
	}
	macroMap[event] = bestMatch;
	return bestMatch;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderSymMOD(MemoryFileReader file, const uint64 *pfilesize)
{
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	SymFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.Validate())
		return ProbeFailure;
	return ProbeSuccess;
}


bool CSoundFile::ReadSymMOD(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	SymFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader) || !fileHeader.Validate())
		return false;
	else if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MPT, std::min(MAX_BASECHANNELS, static_cast<CHANNELINDEX>(fileHeader.numChannels)));

	m_SongFlags.set(SONG_LINEARSLIDES | SONG_EXFILTERRANGE | SONG_AUTO_VIBRATO | SONG_AUTO_TREMOLO | SONG_IMPORTED);
	m_playBehaviour = GetDefaultPlaybackBehaviour(MOD_TYPE_IT);
	m_playBehaviour.reset(kITShortSampleRetrig);
	m_nSamplePreAmp = Clamp(512 / GetNumChannels(), 16, 128);

	enum class ChunkType : int32
	{
		NumChannels     = -1,
		TrackLength     = -2,
		PatternSize     = -3,
		NumInstruments  = -4,
		EventSize       = -5,
		Tempo           = -6,
		ExternalSamples = -7,
		PositionList    = -10,
		SampleFile      = -11,
		EmptySample     = -12,
		PatternEvents   = -13,
		InstrumentList  = -14,
		Sequences       = -15,
		InfoText        = -16,
		SamplePacked    = -17,
		SamplePacked16  = -18,
		InfoType        = -19,
		InfoBinary      = -20,
		InfoString      = -21,

		SampleBoost     = 10,  // All samples will be normalized to this value
		StereoDetune    = 11,  // Note: Not affected by no-DSP flag in instrument! So this would need to have its own plugin...
		StereoPhase     = 12,
	};

	uint32 trackLen      = 0;
	uint16 sampleBoost   = 2500;
	bool isSymphoniePro  = false;
	bool externalSamples = false;
	bool unknownHunks    = false;
	std::vector<SymPosition> positions;
	std::vector<SymSequence> sequences;
	std::vector<SymEvent> patternData;
	std::vector<SymInstrument> instruments;

	while(file.CanRead(sizeof(int32)))
	{
		const ChunkType chunkType = static_cast<ChunkType>(file.ReadInt32BE());
		switch(chunkType)
		{
		// Simple values
		case ChunkType::NumChannels:
			file.Skip(sizeof(uint32be));  // Already handled
			break;

		case ChunkType::TrackLength:
			trackLen = file.ReadUint32BE();
			if(trackLen > 1024)
				return false;
			break;

		case ChunkType::EventSize:
			if(auto eventSize = (file.ReadUint32BE() & 0xFFFF); eventSize != sizeof(SymEvent))
				return false;
			break;

		case ChunkType::Tempo:
			Order().SetDefaultTempo(TEMPO(1.24 * std::min(file.ReadUint32BE(), uint32(800))));
			break;

		// Unused values
		case ChunkType::NumInstruments:  // determined from # of instrument headers instead
		case ChunkType::PatternSize:
			file.Skip(4);
			break;

		case ChunkType::SampleBoost:
			sampleBoost    = static_cast<uint16>(Clamp(file.ReadUint32BE(), 0u, 10000u));
			isSymphoniePro = true;
			break;

		case ChunkType::StereoDetune:
		case ChunkType::StereoPhase:
			isSymphoniePro = true;
			if(uint32 val = file.ReadUint32BE(); val != 0)
				AddToLog(LogWarning, U_("Stereo Detune / Stereo Phase is not supported"));
			break;

		case ChunkType::ExternalSamples:
			file.Skip(4);
			if(!m_nSamples)
				externalSamples = true;
			break;

		// Binary chunk types
		case ChunkType::PositionList:
			if((loadFlags & loadPatternData) && positions.empty())
				positions = DecodeSymArray<SymPosition>(file);
			else
				file.Skip(file.ReadUint32BE());
			break;

		case ChunkType::SampleFile:
		case ChunkType::SamplePacked:
		case ChunkType::SamplePacked16:
			if(m_nSamples >= instruments.size())
				break;
			if(!externalSamples && (loadFlags & loadSampleData) && CanAddMoreSamples())
			{
				const SAMPLEINDEX sample = ++m_nSamples;

				std::vector<std::byte> unpackedSample;
				FileReader chunk;
				if(chunkType == ChunkType::SampleFile)
				{
					chunk = file.ReadChunk(file.ReadUint32BE());
				} else if(chunkType == ChunkType::SamplePacked)
				{
					unpackedSample = DecodeSample8(file);
					chunk = FileReader(mpt::as_span(unpackedSample));
				} else  // SamplePacked16
				{
					unpackedSample = DecodeSample16(file);
					chunk = FileReader(mpt::as_span(unpackedSample));
				}

				if(!ReadIFFSample(sample, chunk, false)
				   && !ReadWAVSample(sample, chunk)
				   && !ReadAIFFSample(sample, chunk)
				   && !ReadRawSymSample(Samples[sample], chunk))
				{
					AddToLog(LogWarning, U_("Unknown sample format."));
				}

				// Symphonie represents stereo instruments as two consecutive mono instruments which are
				// automatically played at the same time. If this one uses a stereo sample, split it
				// and map two OpenMPT instruments to the stereo halves to ensure correct playback
				if(Samples[sample].uFlags[CHN_STEREO] && CanAddMoreSamples())
				{
					const SAMPLEINDEX sampleL = ++m_nSamples;
					ctrlSmp::SplitStereo(Samples[sample], Samples[sampleL], Samples[sample], *this);
					Samples[sampleL].filename = "Left";
					Samples[sample].filename = "Right";
				} else if(sample < instruments.size() && instruments[sample].channel == SymInstrument::StereoR && CanAddMoreSamples())
				{
					// Prevent misalignment of samples in exit.symmod (see condition in MoveNextMonoInstrument in Symphonie source)
					m_nSamples++;
				}
			} else
			{
				// Skip sample
				file.Skip(file.ReadUint32BE());
			}
			break;

		case ChunkType::EmptySample:
			if(CanAddMoreSamples())
				m_nSamples++;
			break;

		case ChunkType::PatternEvents:
			if((loadFlags & loadPatternData) && patternData.empty())
				patternData = DecodeSymArray<SymEvent>(file);
			else
				file.Skip(file.ReadUint32BE());
			break;

		case ChunkType::InstrumentList:
			if(instruments.empty())
				instruments = DecodeSymArray<SymInstrument>(file);
			else
				file.Skip(file.ReadUint32BE());
			break;

		case ChunkType::Sequences:
			if((loadFlags & loadPatternData) && sequences.empty())
				sequences = DecodeSymArray<SymSequence>(file);
			else
				file.Skip(file.ReadUint32BE());
			break;

		case ChunkType::InfoText:
			if(const auto text = DecodeSymChunk(file); !text.empty())
				m_songMessage.Read(text.data(), text.size(), SongMessage::leLF);
			break;

		// Unused binary chunks
		case ChunkType::InfoType:
		case ChunkType::InfoBinary:
		case ChunkType::InfoString:
			file.Skip(file.ReadUint32BE());
			break;

		// Unrecognized chunk/value type (e.g. garbage at the end of Natsh1.SymMOD)
		default:
			unknownHunks = true;
			break;
		}
	}

	if(!trackLen || instruments.empty())
		return false;
	if((loadFlags & loadPatternData) && (positions.empty() || patternData.empty() || sequences.empty()))
		return false;
	if(unknownHunks)
		AddToLog(LogWarning, U_("Unknown hunks were found and ignored."));

	// Let's hope noone is going to use the 256th instrument ;)
	if(instruments.size() >= MAX_INSTRUMENTS)
		instruments.resize(MAX_INSTRUMENTS - 1u);
	m_nInstruments = static_cast<INSTRUMENTINDEX>(instruments.size());
	static_assert(MAX_SAMPLES >= MAX_INSTRUMENTS);
	m_nSamples = std::max(m_nSamples, m_nInstruments);

	// Supporting this is probably rather useless, as the paths will always be full Amiga paths. We just take the filename without path for now.
	if(externalSamples)
	{
#ifdef MPT_EXTERNAL_SAMPLES
		m_nSamples = m_nInstruments;
		for(SAMPLEINDEX sample = 1; sample <= m_nSamples; sample++)
		{
			const SymInstrument &symInst = instruments[sample - 1];
			if(symInst.IsEmpty() || symInst.IsVirtual())
				continue;
			
			auto filename = mpt::PathString::FromUnicode(mpt::ToUnicode(mpt::Charset::Amiga_no_C1, symInst.GetName()));
			if(file.GetOptionalFileName())
				filename = file.GetOptionalFileName()->GetDirectoryWithDrive() + filename.GetFilename();
			
			if(!LoadExternalSample(sample, filename))
				AddToLog(LogError, MPT_UFORMAT("Unable to load sample {}: {}")(sample, filename));
			else
				ResetSamplePath(sample);

			if(Samples[sample].uFlags[CHN_STEREO] && sample < m_nSamples)
			{
				const SAMPLEINDEX sampleL = sample + 1;
				ctrlSmp::SplitStereo(Samples[sample], Samples[sampleL], Samples[sample], *this);
				Samples[sampleL].filename = "Left";
				Samples[sample].filename = "Right";
				sample++;
			}
		}
#else
		AddToLog(LogWarning, U_("External samples are not supported."));
#endif  // MPT_EXTERNAL_SAMPLES
	}

	// Convert instruments
	for(int pass = 0; pass < 2; pass++)
	{
		for(INSTRUMENTINDEX ins = 1; ins <= m_nInstruments; ins++)
		{
			SymInstrument &symInst = instruments[ins - 1];
			if(symInst.IsEmpty())
				continue;
			// First load all regular instruments, and when we have the required information, render the virtual ones
			if(symInst.IsVirtual() != (pass == 1))
				continue;

			SAMPLEINDEX sample = ins;
			if(symInst.virt.header.IsVirtual())
			{
				const uint8 firstSource = symInst.virt.noteEvents[0].inst;
				ModSample &target = Samples[sample];
				if(symInst.virt.Render(*this, symInst.sampleFlags & SymInstrument::AsQueue, target, sampleBoost))
				{
					m_szNames[sample] = "Virtual";
					if(firstSource < instruments.size())
						symInst.downsample += instruments[firstSource].downsample;
				} else
				{
					sample = firstSource + 1;
				}
			} else if(symInst.virt.header.IsTranswave())
			{
				const SymTranswaveInst transwaveInst = symInst.GetTranswave();
				const auto &trans1 = transwaveInst.points[0], &trans2 = transwaveInst.points[1];
				if(trans1.sourceIns < m_nSamples)
				{
					const ModSample emptySample;
					const ModSample &smp1 = Samples[trans1.sourceIns + 1];
					const ModSample &smp2 = trans2.sourceIns < m_nSamples ? Samples[trans2.sourceIns + 1] : emptySample;
					ModSample &target     = Samples[sample];
					if(transwaveInst.Render(smp1, smp2, target))
					{
						m_szNames[sample] = "Transwave";
						// Transwave instruments play an octave lower than the original source sample, but are 4x oversampled,
						// so effectively they play an octave higher
						symInst.transpose += 12;
					}
				}
			}

			if(ModInstrument *instr = AllocateInstrument(ins, sample); instr != nullptr && sample <= m_nSamples)
				symInst.ConvertToMPT(*instr, Samples[sample], *this);
		}
	}

	// Convert patterns
	// map Symphonie positions to converted patterns
	std::map<SymPosition, PATTERNINDEX> patternMap;
	// map DSP commands to MIDI macro numbers
	std::map<SymEvent, uint8> macroMap;

	bool useDSP = false;
	const uint32 patternSize = fileHeader.numChannels * trackLen;
	const PATTERNINDEX numPatterns = mpt::saturate_cast<PATTERNINDEX>(patternData.size() / patternSize);

	Patterns.ResizeArray(numPatterns);
	Order().clear();

	struct ChnState
	{
		float curVolSlide      = 0;      // Current volume slide factor of a channel
		float curVolSlideAmt   = 0;      // Cumulative volume slide amount
		float curPitchSlide    = 0;      // Current pitch slide factor of a channel
		float curPitchSlideAmt = 0;      // Cumulative pitch slide amount
		bool stopped           = false;  // Sample paused or not (affects volume and pitch slides)
		uint8 lastNote         = 0;      // Last note played on a channel
		uint8 lastInst         = 0;      // Last instrument played on a channel
		uint8 lastVol          = 64;     // Last specified volume of a channel (to avoid excessive Mxx commands)
		uint8 channelVol       = 100;    // Volume multiplier, 0...100
		uint8 calculatedVol    = 64;     // Final channel volume
		uint8 fromAdd          = 0;      // Base sample offset for FROM and FR&P effects
		bool  retrigVibrato    = false;
		uint8 curVibrato       = 0;
		bool  retrigTremolo    = false;
		uint8 curTremolo       = 0;
		uint8 sampleVibSpeed   = 0;
		uint8 sampleVibDepth   = 0;
		uint8 tonePortaAmt     = 0;
		uint16 sampleVibPhase  = 0;
		uint16 retriggerRemain = 0;
		uint16 tonePortaRemain = 0;
	};
	std::vector<ChnState> chnStates(GetNumChannels());

	// In Symphonie, sequences represent the structure of a song, and not separate songs like in OpenMPT. Hence they will all be loaded into the same ModSequence.
	for(SymSequence &seq : sequences)
	{
		if(seq.info == 1)
			continue;
		if(seq.info == -1)
			break;

		if(seq.start >= positions.size()
		   || seq.length > positions.size()
		   || seq.length == 0
		   || positions.size() - seq.length < seq.start)
			continue;
		auto seqPositions = mpt::as_span(positions).subspan(seq.start, seq.length);

		// Sequences are all part of the same song, just add a skip index as a divider
		ModSequence &order = Order();
		if(!order.empty())
			order.push_back(PATTERNINDEX_SKIP);

		for(auto &pos : seqPositions)
		{
			// before checking the map, apply the sequence transpose value
			pos.transpose += seq.transpose;

			// pattern already converted?
			PATTERNINDEX patternIndex = 0;
			if(patternMap.count(pos))
			{
				patternIndex = patternMap[pos];
			} else if(loadFlags & loadPatternData)
			{
				// Convert pattern now
				patternIndex = Patterns.InsertAny(pos.length);
				if(patternIndex == PATTERNINDEX_INVALID)
					break;

				patternMap[pos] = patternIndex;

				if(pos.pattern >= numPatterns || pos.start >= trackLen)
					continue;

				uint8 patternSpeed = static_cast<uint8>(pos.speed);

				// This may intentionally read into the next pattern
				auto srcEvent = patternData.cbegin() + (pos.pattern * patternSize) + (pos.start * fileHeader.numChannels);
				const SymEvent emptyEvent{};
				ModCommand syncPlayCommand;
				for(ROWINDEX row = 0; row < pos.length; row++)
				{
					ModCommand *rowBase = Patterns[patternIndex].GetpModCommand(row, 0);
					bool applySyncPlay = false;
					for(CHANNELINDEX chn = 0; chn < fileHeader.numChannels; chn++)
					{
						const SymEvent &event = (srcEvent != patternData.cend()) ? *srcEvent : emptyEvent;
						if(srcEvent != patternData.cend())
							srcEvent++;
						if(chn >= GetNumChannels())
							continue;
						ModCommand &m = rowBase[chn];

						int8 note = (event.note >= 0 && event.note <= 84) ? event.note + 25 : -1;
						uint8 origInst = event.inst;
						uint8 mappedInst = 0;
						if(origInst < instruments.size())
						{
							mappedInst = static_cast<uint8>(origInst + 1);
							if(!(instruments[origInst].instFlags & SymInstrument::NoTranspose) && note >= 0)
								note = Clamp(static_cast<int8>(note + pos.transpose), NOTE_MIN, NOTE_MAX);
						}

						// If we duplicated a stereo channel to this cell but the event is non-empty, remove it again.
						if(m.note != NOTE_NONE && (event.command != SymEvent::KeyOn || event.note != -1 || event.inst != 0 || event.param != 0)
							&& m.instr > 0 && m.instr <= instruments.size() && instruments[m.instr - 1].channel == SymInstrument::StereoR)
						{
							m.Clear();
						}

						auto &chnState = chnStates[chn];

						if(applySyncPlay)
						{
							applySyncPlay = false;
							m = syncPlayCommand;
							if(m.command == CMD_NONE && chnState.calculatedVol != chnStates[chn - 1].calculatedVol)
							{
								chnState.calculatedVol = chnStates[chn - 1].calculatedVol;
								m.SetEffectCommand(CMD_CHANNELVOLUME, chnState.calculatedVol);
							}
							if(!event.IsGlobal())
								continue;
						}

						bool applyVolume = false;
						switch(static_cast<SymEvent::Command>(event.command.get()))
						{
						case SymEvent::KeyOn:
							if(event.param > SymEvent::VolCommand)
							{
								switch(event.param)
								{
								case SymEvent::StopSample:
									m.SetVolumeCommand(VOLCMD_PLAYCONTROL, 0);
									chnState.stopped = true;
									break;

								case SymEvent::ContSample:
									m.SetVolumeCommand(VOLCMD_PLAYCONTROL, 1);
									chnState.stopped = false;
									break;

								case SymEvent::KeyOff:
									if(m.note == NOTE_NONE)
										m.note = chnState.lastNote;
									m.SetVolumeCommand(VOLCMD_OFFSET, 1);
									break;

								case SymEvent::SpeedDown:
									if(patternSpeed > 1)
									{
										m.SetEffectCommand(CMD_SPEED, --patternSpeed);
									}
									break;

								case SymEvent::SpeedUp:
									if(patternSpeed < 0xFF)
									{
										m.SetEffectCommand(CMD_SPEED, ++patternSpeed);
									}
									break;

								case SymEvent::SetPitch:
									chnState.lastNote = note;
									if(mappedInst != chnState.lastInst)
										break;
									m.note = note;
									m.SetEffectCommand(CMD_TONEPORTA_DURATION, 0);
									chnState.curPitchSlide = 0;
									chnState.tonePortaRemain = 0;
									chnState.retrigVibrato = chnState.retrigTremolo = true;
									break;

								// fine portamentos with range up to half a semitone
								case SymEvent::PitchUp:
									m.SetEffectCommand(CMD_PORTAMENTOUP, 0xF2);
									break;
								case SymEvent::PitchDown:
									m.SetEffectCommand(CMD_PORTAMENTODOWN, 0xF2);
									break;
								case SymEvent::PitchUp2:
									m.SetEffectCommand(CMD_PORTAMENTOUP, 0xF4);
									break;
								case SymEvent::PitchDown2:
									m.SetEffectCommand(CMD_PORTAMENTODOWN, 0xF4);
									break;
								case SymEvent::PitchUp3:
									m.SetEffectCommand(CMD_PORTAMENTOUP, 0xF8);
									break;
								case SymEvent::PitchDown3:
									m.SetEffectCommand(CMD_PORTAMENTODOWN, 0xF8);
									break;
								}
							} else
							{
								if(event.note >= 0 || event.param < 100)
								{
									if(event.note >= 0)
									{
										m.note = chnState.lastNote = note;
										m.instr = chnState.lastInst = mappedInst;
										chnState.curPitchSlide = 0;
										chnState.tonePortaRemain = 0;
										chnState.retrigVibrato = chnState.retrigTremolo = true;
									}

									if(event.param > 0)
									{
										chnState.lastVol = mpt::saturate_round<uint8>(event.param * 0.64);
										if(chnState.curVolSlide != 0)
											applyVolume = true;
										chnState.curVolSlide = 0;
									}
								}
							}

							if(const uint8 newVol = static_cast<uint8>(Util::muldivr_unsigned(chnState.lastVol, chnState.channelVol, 100));
							   applyVolume || chnState.calculatedVol != newVol)
							{
								chnState.calculatedVol = newVol;
								m.SetEffectCommand(CMD_CHANNELVOLUME, newVol);
							}

							// Key-On commands with stereo instruments are played on both channels - unless there's already some sort of event
							if(event.note > 0 && (chn < GetNumChannels() - 1) && !(chn % 2u)
							   && origInst < instruments.size() && instruments[origInst].channel == SymInstrument::StereoL)
							{
								ModCommand &next = rowBase[chn + 1];
								next = m;
								next.instr++;

								chnStates[chn + 1].lastVol = chnState.lastVol;
								chnStates[chn + 1].curVolSlide = chnState.curVolSlide;
								chnStates[chn + 1].curVolSlideAmt = chnState.curVolSlideAmt;
								chnStates[chn + 1].curPitchSlide = chnState.curPitchSlide;
								chnStates[chn + 1].curPitchSlideAmt = chnState.curPitchSlideAmt;
								chnStates[chn + 1].retriggerRemain = chnState.retriggerRemain;
							}

							break;

							// volume effects
							// Symphonie has very fine fractional volume slides which are applied at the output sample rate,
							// rather than per tick or per row, so instead let's simulate it based on the pattern speed
							// by keeping track of the volume and using normal volume commands
							// the math here is an approximation which works fine for most songs
						case SymEvent::VolSlideUp:
							chnState.curVolSlideAmt = 0;
							chnState.curVolSlide = event.param * 0.0333f;
							break;
						case SymEvent::VolSlideDown:
							chnState.curVolSlideAmt = 0;
							chnState.curVolSlide = event.param * -0.0333f;
							break;

						case SymEvent::AddVolume:
							m.command = CMD_NONE;
							break;
						case SymEvent::Tremolo:
							{
								// both tremolo speed and depth can go much higher than OpenMPT supports,
								// but modules will probably use pretty sane, supportable values anyway
								// TODO: handle very small nonzero params
								uint8 speed = std::min<uint8>(15, event.inst >> 3);
								uint8 depth = std::min<uint8>(15, event.param >> 3);
								chnState.curTremolo = (speed << 4) | depth;
								if(chnState.curTremolo)
									chnState.retrigTremolo = true;
								else
									m.SetEffectCommand(CMD_TREMOLO, 0);
							}
							break;

							// pitch effects
							// Pitch slides have a similar granularity to volume slides, and are approximated
							// the same way here based on a rough comparison against Exx/Fxx slides
						case SymEvent::PitchSlideUp:
							chnState.curPitchSlideAmt = 0;
							chnState.curPitchSlide = event.param * 0.0333f;
							chnState.tonePortaRemain = 0;
							break;
						case SymEvent::PitchSlideDown:
							chnState.curPitchSlideAmt = 0;
							chnState.curPitchSlide = event.param * -0.0333f;
							chnState.tonePortaRemain = 0;
							break;

						case SymEvent::PitchSlideTo:
							if(note >= 0 && event.param > 0)
							{
								const int distance = std::abs((note - chnState.lastNote) * 32);
								chnState.curPitchSlide = 0;
								m.note = chnState.lastNote = note;
								chnState.tonePortaAmt = mpt::saturate_cast<ModCommand::PARAM>(distance / (2 * event.param));
								m.SetEffectCommand(CMD_TONEPORTAMENTO, chnState.tonePortaAmt);
								chnState.tonePortaRemain = static_cast<uint16>(distance - std::min(distance, chnState.tonePortaAmt * (patternSpeed - 1)));
							}
							break;
						case SymEvent::AddPitch:
							// "The range (-128...127) is about 4 half notes."
							m.command = CMD_NONE;
							break;
						case SymEvent::Vibrato:
							{
								// both vibrato speed and depth can go much higher than OpenMPT supports,
								// but modules will probably use pretty sane, supportable values anyway
								// TODO: handle very small nonzero params
								uint8 speed = std::min<uint8>(15, event.inst >> 3);
								uint8 depth = std::min<uint8>(15, event.param);
								chnState.curVibrato = (speed << 4) | depth;
								if(chnState.curVibrato)
									chnState.retrigVibrato = true;
								else
									m.SetEffectCommand(CMD_VIBRATO, 0);
							}
							break;
						case SymEvent::AddHalfTone:
							m.note = chnState.lastNote = Clamp(static_cast<uint8>(chnState.lastNote + event.param), NOTE_MIN, NOTE_MAX);
							m.SetEffectCommand(CMD_TONEPORTA_DURATION, 0);
							chnState.tonePortaRemain = 0;
							break;

							// DSP effects
						case SymEvent::Filter:
#ifndef NO_PLUGINS
						case SymEvent::DSPEcho:
						case SymEvent::DSPDelay:
#endif
							if(auto it = macroMap.find(event); it != macroMap.end() && it->second != 0)
							{
								m.SetEffectCommand(CMD_MIDI, it->second);
							} else if(macroMap.size() < m_MidiCfg.Zxx.size())
							{
								uint8 param = static_cast<uint8>(macroMap.size());
								if(ConvertDSP(event, m_MidiCfg.Zxx[param], *this))
								{
									m.SetEffectCommand(CMD_MIDI, macroMap[event] = 0x80 | param);

									if(event.command == SymEvent::DSPEcho || event.command == SymEvent::DSPDelay)
										useDSP = true;
								}
							} else if(uint8 param = MapToClosestMidiMacro(event, macroMap))
							{
								m.SetEffectCommand(CMD_MIDI, param);
							}
							break;

							// other effects
						case SymEvent::Retrig:
							// This plays the note <param> times every <inst>+1 ticks.
							// The effect continues on the following rows until the correct amount is reached.
							if(event.param < 1)
								break;
							m.SetEffectCommand(CMD_RETRIG, static_cast<ModCommand::PARAM>(std::min(15, event.inst + 1)));
							chnState.retriggerRemain = static_cast<uint16>(event.param * (event.inst + 1u));
							break;

						case SymEvent::SetSpeed:
							patternSpeed = event.param ? event.param : 4u;
							m.SetEffectCommand(CMD_SPEED, patternSpeed);
							break;

							// TODO this applies a fade on the sample level
						case SymEvent::Emphasis:
							m.command = CMD_NONE;
							break;
						case SymEvent::CV:
							if(event.note == 0 || event.note == 4)
							{
								uint8 pan  = (event.note == 4) ? event.inst : 128;
								uint8 vol  = std::min<uint8>(event.param, 100);
								uint8 volL = static_cast<uint8>(vol * std::min(128, 256 - pan) / 128);
								uint8 volR = static_cast<uint8>(vol * std::min(uint8(128), pan) / 128);

								if(volL != chnState.channelVol)
								{
									chnState.channelVol = volL;
									chnState.calculatedVol = static_cast<uint8>(Util::muldivr_unsigned(chnState.lastVol, chnState.channelVol, 100));
									m.SetEffectCommand(CMD_CHANNELVOLUME, chnState.calculatedVol);
								}
								if(event.note == 4 && chn < (GetNumChannels() - 1) && chnStates[chn + 1].channelVol != volR)
								{
									chnStates[chn + 1].channelVol = volR;
									chnState.calculatedVol = static_cast<uint8>(Util::muldivr_unsigned(chnState.lastVol, chnState.channelVol, 100));
									rowBase[chn + 1].SetEffectCommand(CMD_CHANNELVOLUME, chnState.calculatedVol);
								}
							}
							break;
						case SymEvent::CVAdd:
							// Effect doesn't seem to exist in UI and code looks like a no-op
							m.command = CMD_NONE;
							break;

						case SymEvent::SetFromAdd:
							chnState.fromAdd = event.param;
							chnState.sampleVibSpeed = 0;
							chnState.sampleVibDepth = 0;
							break;
						case SymEvent::FromAdd:
							// TODO need to verify how signedness of this value is treated
							// C = -128...+127
							//FORMEL: Neuer FADD := alter FADD + C* Samplelaenge/16384
							chnState.fromAdd += event.param;
							break;

						case SymEvent::SampleVib:
							chnState.sampleVibSpeed = event.inst;
							chnState.sampleVibDepth = event.param;
							break;

							// sample effects
						case SymEvent::FromAndPitch:
							chnState.lastNote = note;
							m.instr = chnState.lastInst = mappedInst;
							[[fallthrough]];
						case SymEvent::ReplayFrom:
							m.note = chnState.lastNote;
							if(note >= 0)
								m.instr = chnState.lastInst = mappedInst;
							if(event.command == SymEvent::ReplayFrom)
							{
								m.SetVolumeCommand(VOLCMD_TONEPORTAMENTO, 1);
							}
							// don't always add the command, because often FromAndPitch is used with offset 0
							// to act as a key-on which doesn't cancel volume slides, etc
							if(event.param || chnState.fromAdd || chnState.sampleVibDepth)
							{
								double sampleVib = 0.0;
								if(chnState.sampleVibDepth)
									sampleVib = chnState.sampleVibDepth * (std::sin(chnState.sampleVibPhase * (mpt::numbers::pi * 2.0 / 1024.0) + 1.5 * mpt::numbers::pi) - 1.0) / 4.0;
								m.SetEffectCommand(CMD_OFFSETPERCENTAGE, mpt::saturate_round<ModCommand::PARAM>(event.param + chnState.fromAdd + sampleVib));
							}
							chnState.tonePortaRemain = 0;
							break;
						}

						// Any event which plays a note should re-enable continuous effects
						if(m.note != NOTE_NONE)
							chnState.stopped = false;
						else if(chnState.stopped)
							continue;

						if(chnState.retriggerRemain)
						{
							chnState.retriggerRemain = std::max(chnState.retriggerRemain, static_cast<uint16>(patternSpeed)) - patternSpeed;
							if(m.command == CMD_NONE)
								m.SetEffectCommand(CMD_RETRIG, 0);
						}

						// Handle fractional volume slides
						if(chnState.curVolSlide != 0)
						{
							chnState.curVolSlideAmt += chnState.curVolSlide * patternSpeed;
							if(m.command == CMD_NONE)
							{
								if(patternSpeed > 1 && chnState.curVolSlideAmt >= (patternSpeed - 1))
								{
									uint8 slideAmt = std::min<uint8>(15, mpt::saturate_round<uint8>(chnState.curVolSlideAmt / (patternSpeed - 1)));
									chnState.curVolSlideAmt -= static_cast<float>(slideAmt * (patternSpeed - 1));
									// normal slide up
									m.SetEffectCommand(CMD_CHANNELVOLSLIDE, slideAmt << 4);
								} else if(chnState.curVolSlideAmt >= 1.0f)
								{
									uint8 slideAmt = std::min<uint8>(15, mpt::saturate_round<uint8>(chnState.curVolSlideAmt));
									chnState.curVolSlideAmt -= static_cast<float>(slideAmt);
									// fine slide up
									m.SetEffectCommand(CMD_CHANNELVOLSLIDE, (slideAmt << 4) | 0x0F);
								} else if(patternSpeed > 1 && chnState.curVolSlideAmt <= -(patternSpeed - 1))
								{
									uint8 slideAmt = std::min<uint8>(15, mpt::saturate_round<uint8>(-chnState.curVolSlideAmt / (patternSpeed - 1)));
									chnState.curVolSlideAmt += static_cast<float>(slideAmt * (patternSpeed - 1));
									// normal slide down
									m.SetEffectCommand(CMD_CHANNELVOLSLIDE, slideAmt);
								} else if(chnState.curVolSlideAmt <= -1.0f)
								{
									uint8 slideAmt = std::min<uint8>(14, mpt::saturate_round<uint8>(-chnState.curVolSlideAmt));
									chnState.curVolSlideAmt += static_cast<float>(slideAmt);
									// fine slide down
									m.SetEffectCommand(CMD_CHANNELVOLSLIDE, slideAmt | 0xF0);
								}
							}
						}
						// Handle fractional pitch slides
						if(chnState.curPitchSlide != 0)
						{
							chnState.curPitchSlideAmt += chnState.curPitchSlide * patternSpeed;
							if(m.command == CMD_NONE)
							{
								if(patternSpeed > 1 && chnState.curPitchSlideAmt >= (patternSpeed - 1))
								{
									uint8 slideAmt = std::min<uint8>(0xDF, mpt::saturate_round<uint8>(chnState.curPitchSlideAmt / (patternSpeed - 1)));
									chnState.curPitchSlideAmt -= static_cast<float>(slideAmt * (patternSpeed - 1));
									// normal slide up
									m.SetEffectCommand(CMD_PORTAMENTOUP, slideAmt);
								} else if(chnState.curPitchSlideAmt >= 1.0f)
								{
									uint8 slideAmt = std::min<uint8>(15, mpt::saturate_round<uint8>(chnState.curPitchSlideAmt));
									chnState.curPitchSlideAmt -= static_cast<float>(slideAmt);
									// fine slide up
									m.SetEffectCommand(CMD_PORTAMENTOUP, slideAmt | 0xF0);
								} else if(patternSpeed > 1 && chnState.curPitchSlideAmt <= -(patternSpeed - 1))
								{
									uint8 slideAmt = std::min<uint8>(0xDF, mpt::saturate_round<uint8>(-chnState.curPitchSlideAmt / (patternSpeed - 1)));
									chnState.curPitchSlideAmt += static_cast<float>(slideAmt * (patternSpeed - 1));
									// normal slide down
									m.SetEffectCommand(CMD_PORTAMENTODOWN, slideAmt);
								} else if(chnState.curPitchSlideAmt <= -1.0f)
								{
									uint8 slideAmt = std::min<uint8>(14, mpt::saturate_round<uint8>(-chnState.curPitchSlideAmt));
									chnState.curPitchSlideAmt += static_cast<float>(slideAmt);
									// fine slide down
									m.SetEffectCommand(CMD_PORTAMENTODOWN, slideAmt | 0xF0);
								}
							}
							// TODO: use volume column if effect column is occupied
							else if(m.volcmd == VOLCMD_NONE)
							{
								if(patternSpeed > 1 && chnState.curPitchSlideAmt / 4 >= (patternSpeed - 1))
								{
									uint8 slideAmt = std::min<uint8>(9, mpt::saturate_round<uint8>(chnState.curPitchSlideAmt / (patternSpeed - 1)) / 4);
									chnState.curPitchSlideAmt -= static_cast<float>(slideAmt * (patternSpeed - 1) * 4);
									m.SetVolumeCommand(VOLCMD_PORTAUP, slideAmt);
								} else if(patternSpeed > 1 && chnState.curPitchSlideAmt / 4 <= -(patternSpeed - 1))
								{
									uint8 slideAmt = std::min<uint8>(9, mpt::saturate_round<uint8>(-chnState.curPitchSlideAmt / (patternSpeed - 1)) / 4);
									chnState.curPitchSlideAmt += static_cast<float>(slideAmt * (patternSpeed - 1) * 4);
									m.SetVolumeCommand(VOLCMD_PORTADOWN, slideAmt);
								}
							}
						}
						// Vibrato and Tremolo
						if(m.command == CMD_NONE && chnState.curVibrato && chnState.retrigVibrato)
						{
							m.SetEffectCommand(CMD_VIBRATO, chnState.curVibrato);
							chnState.retrigVibrato = false;
						}
						if(m.command == CMD_NONE && chnState.curTremolo && chnState.retrigTremolo)
						{
							m.SetEffectCommand(CMD_TREMOLO, chnState.curTremolo);
							chnState.retrigTremolo = false;
						}
						// Tone Portamento
						if(m.command != CMD_TONEPORTAMENTO && chnState.tonePortaRemain)
						{
							if(m.command == CMD_NONE)
								m.command = CMD_TONEPORTAMENTO;
							else
								m.volcmd = VOLCMD_TONEPORTAMENTO;
							chnState.tonePortaRemain -= std::min(chnState.tonePortaRemain, static_cast<uint16>(chnState.tonePortaAmt * (patternSpeed - 1)));
						}

						chnState.sampleVibPhase = (chnState.sampleVibPhase + chnState.sampleVibSpeed * patternSpeed) & 1023;

						if(!(chn % 2u) && chnState.lastInst && chnState.lastInst <= instruments.size()
						   && (instruments[chnState.lastInst - 1].instFlags & SymInstrument::SyncPlay))
						{
							syncPlayCommand = m;
							applySyncPlay = true;
							if(syncPlayCommand.instr && instruments[chnState.lastInst - 1].channel == SymInstrument::StereoL)
								syncPlayCommand.instr++;
						}
					}
				}

				Patterns[patternIndex].WriteEffect(EffectWriter(CMD_SPEED, static_cast<uint8>(pos.speed)).Row(0).RetryNextRow());
			}
			order.insert(order.GetLength(), std::max(pos.loopNum.get(), uint16(1)), patternIndex);
			// Undo transpose tweak
			pos.transpose -= seq.transpose;
		}
	}

#ifndef NO_PLUGINS
	if(useDSP)
	{
		SNDMIXPLUGIN &plugin = m_MixPlugins[0];
		mpt::reconstruct(plugin);
		memcpy(&plugin.Info.dwPluginId1, "SymM", 4);
		memcpy(&plugin.Info.dwPluginId2, "Echo", 4);
		plugin.Info.routingFlags    = SNDMIXPLUGININFO::irAutoSuspend;
		plugin.Info.mixMode         = 0;
		plugin.Info.gain            = 10;
		plugin.Info.reserved        = 0;
		plugin.Info.dwOutputRouting = 0;
		std::fill(plugin.Info.dwReserved, plugin.Info.dwReserved + std::size(plugin.Info.dwReserved), 0);
		plugin.Info.szName        = "Echo";
		plugin.Info.szLibraryName = "SymMOD Echo";

		m_MixPlugins[1].Info.szName = "No Echo";
	}
#endif // NO_PLUGINS

	// Channel panning
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = (chn & 1) ? 256 : 0;
		ChnSettings[chn].nMixPlugin = useDSP ? 1 : 0;  // For MIDI macros controlling the echo DSP
	}

	m_modFormat.formatName = UL_("Symphonie");
	m_modFormat.type = UL_("symmod");
	if(!isSymphoniePro)
		m_modFormat.madeWithTracker = UL_("Symphonie");  // or Symphonie Jr
	else if(instruments.size() <= 128)
		m_modFormat.madeWithTracker = UL_("Symphonie Pro");
	else
		m_modFormat.madeWithTracker = UL_("Symphonie Pro 256");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	return true;
}

OPENMPT_NAMESPACE_END
