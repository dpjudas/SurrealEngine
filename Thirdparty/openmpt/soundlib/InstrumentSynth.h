/*
 * InstrumentSynth.h
 * -----------------
 * Purpose: "Script" / "Synth" processor for various file formats (MED, GT2, Puma, His Master's Noise, Face The Music, Future Composer)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include "Snd_defs.h"

#include <vector>

OPENMPT_NAMESPACE_BEGIN

class CSoundFile;
struct ModChannel;
struct PlayState;

struct InstrumentSynth
{
	struct Event
	{
		enum class Type : uint8
		{
			StopScript,           // No parameter
			Jump,                 // Parameter: Event index (uint16)
			JumpIfTrue,           // Parameter: Event index (uint16)
			Delay,                // Parameter: Number of ticks (uint16)
			SetStepSpeed,         // Parameter: Speed (uint8), update speed now? (bool)
			JumpMarker,           // Parameter: Marker ID (uint16)
			SampleOffset,         // Parameter: Offset (uint32)
			SampleOffsetAdd,      // Parameter: Offset (uint32)
			SampleOffsetSub,      // Parameter: Offset (uint32)
			SetLoopCounter,       // Parameter: Count (uint16), force? (bool)
			EvaluateLoopCounter,  // Parameter: Event index (uint16)
			NoteCut,              // No parameter

			GTK_KeyOff,               // Parameter: Jump target once key is released (uint16)
			GTK_SetVolume,            // Parameter: Volume (uint16)
			GTK_SetPitch,             // Parameter: Pitch (uint16)
			GTK_SetPanning,           // Parameter: Panning (uint16)
			GTK_SetVolumeStep,        // Parameter: Step size (int16)
			GTK_SetPitchStep,         // Parameter: Step size (int16)
			GTK_SetPanningStep,       // Parameter: Step size (int16)
			GTK_SetSpeed,             // Parameter: Speed (uint8)
			GTK_EnableTremor,         // Parameter: Enable (uint8)
			GTK_SetTremorTime,        // Parameter: On time (uint8), off time (uint8)
			GTK_EnableTremolo,        // Parameter: Enable (uint8)
			GTK_EnableVibrato,        // Parameter: Enable (uint8)
			GTK_SetVibratoParams,     // Parameter: Width (uint8), speed (uint8)

			Puma_SetWaveform,  // Parameter: Waveform (uint8), wavestorm step (uint8), number of waveforms to cycle (uint8)
			Puma_VolumeRamp,   // Parameter: Start volume (uint8), end volume (uint8), number of ticks (uint8)
			Puma_StopVoice,    // No parameter
			Puma_SetPitch,     // Parameter: Pitch offset (int8), <unused> (uint8), number of ticks (uint8)
			Puma_PitchRamp,    // Parameter: Start pitch offset (int8), end pitch offset (int8), number of ticks (uint8)

			Mupp_SetWaveform,  // Parameter: Source instrument (uint8), waveform (uint8), volume (uint8)

			MED_DefineArpeggio,   // Parameter: Arpeggio note (uint8), arp length or 0 if it's not the first note (uint16)
			MED_JumpScript,       // Parameter: Script index (uint8), jump target (uint16 - JumpMarker ID, not event index!)
			MED_SetEnvelope,      // Parameter: Envelope index (uint8), loop on/off (uint8), is volume envelope (uint8)
			MED_SetVolume,        // Parameter: Volume (uint8)
			MED_SetWaveform,      // Parameter: Waveform (uint8)
			MED_SetVibratoSpeed,  // Parameter: Speed (uint8)
			MED_SetVibratoDepth,  // Parameter: Depth (uint8)
			MED_SetVolumeStep,    // Parameter: Volume step (int16)
			MED_SetPeriodStep,    // Parameter: Period step (int16)
			MED_HoldDecay,        // Parameter: Hold time (uint8), decay point (uint16)

			FTM_PlaySample,        // No parameter
			FTM_SetPitch,          // Parameter: New pitch (uint16)
			FTM_AddPitch,          // Parameter: Pitch amount (int16)
			FTM_SetDetune,         // Parameter: Detune amount (uint16)
			FTM_AddDetune,         // Parameter: Detune amount (int16)
			FTM_SetVolume,         // Parameter: Channel volume (uint8)
			FTM_AddVolume,         // Parameter: Volume amount (int16)
			FTM_SetSample,         // Parameter: New sample (uint8)
			FTM_SetCondition,      // Parameter: Pitch/volume threshold (uint16), condition type (uint8)
			FTM_SetInterrupt,      // Parameter: Jump target (uint16), interrupt type (uint8)
			FTM_SetSampleStart,    // Parameter: Offset (uint16), modification type (uint8)
			FTM_SetOneshotLength,  // Parameter: Length (uint16), modification type (uint8)
			FTM_SetRepeatLength,   // Parameter: Length (uint16), modification type (uint8)
			FTM_CloneTrack,        // Parameter: Track (uint8), properties (uint8)
			FTM_StartLFO,          // Parameter: LFO index (uint8), target/waveform (uint8)
			FTM_LFOAddSub,         // Parameter: LFO/addSub (uint8), speed (uint8), depth (uint8)
			FTM_SetWorkTrack,      // Parameter: Channel index (uint8), is relative? (bool)
			FTM_SetGlobalVolume,   // Parameter: Global volume (uint16)
			FTM_SetTempo,          // Parameter: Tempo (uint16)
			FTM_SetSpeed,          // Parameter: Speed (uint16)
			FTM_SetPlayPosition,   // Parameter: Pattern to play (uint16), row in pattern (uint8)

			FC_SetWaveform,  // Parameter: Command type (uint8), waveform (uint8), sample pack (uint8)
			FC_SetPitch,     // Parameter: Pitch (int8)
			FC_SetVibrato,   // Parameter: Speed (uint8), depth (uint8), delay (uint8)
			FC_PitchSlide,   // Parameter: Speed (uint8), time (uint8)
			FC_VolumeSlide,  // Parameter: Speed (uint8), time (uint8)
		};

		static constexpr Type JumpEvents[] =
		{
			Type::Jump, Type::JumpIfTrue, Type::EvaluateLoopCounter,
			Type::GTK_KeyOff,
			Type::MED_HoldDecay,
			Type::FTM_SetInterrupt,
		};

		Type type = Type::StopScript;
		union
		{
			uint8 u8 = 0;
			int8 i8;
		};
		union
		{
			uint16 u16;
			int16 i16;
			std::array<uint8, 2> bytes = {{}};
		};

		static constexpr Event StopScript() noexcept { return Event{Type::StopScript}; }
		static constexpr Event Jump(uint16 target) noexcept { return Event{Type::Jump, target}; }
		static constexpr Event JumpIfTrue(uint16 target) noexcept { return Event{Type::JumpIfTrue, target}; }
		static constexpr Event Delay(uint16 ticks) noexcept { return Event{Type::Delay, ticks}; }
		static constexpr Event SetStepSpeed(uint8 speed, bool updateNow) noexcept { return Event{Type::SetStepSpeed, speed, uint8(updateNow ? 1 : 0)}; }
		static constexpr Event JumpMarker(uint16 data) noexcept { return Event{Type::JumpMarker, data}; }
		static constexpr Event SampleOffset(uint32 offset) noexcept { return Event24Bit(Type::SampleOffset, offset); }
		static constexpr Event SampleOffsetAdd(uint32 offset) noexcept { return Event24Bit(Type::SampleOffsetAdd, offset); }
		static constexpr Event SampleOffsetSub(uint32 offset) noexcept { return Event24Bit(Type::SampleOffsetSub, offset); }
		static constexpr Event SetLoopCounter(uint16 count, bool force) noexcept { return Event{Type::SetLoopCounter, count, uint8(force ? 1 : 0)}; }
		static constexpr Event EvaluateLoopCounter(uint16 target) noexcept { return Event{Type::EvaluateLoopCounter, target}; }
		static constexpr Event NoteCut() noexcept { return Event{Type::NoteCut}; }

		static constexpr Event GTK_KeyOff(uint16 target) noexcept { return Event{Type::GTK_KeyOff, target}; }
		static constexpr Event GTK_SetVolume(uint16 volume) noexcept { return Event{Type::GTK_SetVolume, volume}; }
		static constexpr Event GTK_SetPitch(uint16 pitch) noexcept { return Event{Type::GTK_SetPitch, pitch}; }
		static constexpr Event GTK_SetPanning(uint16 panning) noexcept { return Event{Type::GTK_SetPanning, panning}; }
		static constexpr Event GTK_SetVolumeStep(int16 stepSize) noexcept { return Event{Type::GTK_SetVolumeStep, stepSize}; }
		static constexpr Event GTK_SetPitchStep(int16 stepSize) noexcept { return Event{Type::GTK_SetPitchStep, stepSize}; }
		static constexpr Event GTK_SetPanningStep(int16 stepSize) noexcept { return Event{Type::GTK_SetPanningStep, stepSize}; }
		static constexpr Event GTK_SetSpeed(uint8 speed) noexcept { return Event{Type::GTK_SetSpeed, speed}; }
		static constexpr Event GTK_EnableTremor(uint8 enable) noexcept { return Event{Type::GTK_EnableTremor, enable}; }
		static constexpr Event GTK_SetTremorTime(uint8 onTime, uint8 offTime) noexcept { return Event{Type::GTK_SetTremorTime, onTime, offTime}; }
		static constexpr Event GTK_EnableTremolo(uint8 enable) noexcept { return Event{Type::GTK_EnableTremolo, enable}; }
		static constexpr Event GTK_EnableVibrato(uint8 enable) noexcept { return Event{Type::GTK_EnableVibrato, enable}; }
		static constexpr Event GTK_SetVibratoParams(uint8 width, uint8 speed) noexcept { return Event{Type::GTK_SetVibratoParams, width, speed}; }

		static constexpr Event Puma_SetWaveform(uint8 waveform, uint8 step, uint8 count) noexcept { return Event{Type::Puma_SetWaveform, waveform, step, count}; }
		static constexpr Event Puma_VolumeRamp(uint8 startVol, uint8 endVol, uint8 ticks) noexcept { return Event{Type::Puma_VolumeRamp, startVol, endVol, ticks}; }
		static constexpr Event Puma_StopVoice() noexcept { return Event{Type::Puma_StopVoice}; }
		static constexpr Event Puma_SetPitch(int8 pitchOffset, uint8 ticks) noexcept { return Event{Type::Puma_SetPitch, pitchOffset, uint8(0), ticks}; }
		static constexpr Event Puma_PitchRamp(int8 startPitch, int8 endPitch, uint8 ticks) noexcept { return Event{Type::Puma_PitchRamp, startPitch, endPitch, ticks}; }

		static constexpr Event Mupp_SetWaveform(uint8 instr, uint8 waveform, uint8 volume) noexcept { return Event{Type::Mupp_SetWaveform, instr, waveform, volume}; }

		static constexpr Event MED_DefineArpeggio(uint8 note, uint16 noteCount) noexcept { return Event{Type::MED_DefineArpeggio, noteCount, note}; }
		static constexpr Event MED_JumpScript(uint8 scriptIndex, uint16 target) noexcept { return Event{Type::MED_JumpScript, target, scriptIndex}; }
		static constexpr Event MED_SetEnvelope(uint8 envelope, bool loop, bool volumeEnv) noexcept { return Event{Type::MED_SetEnvelope, envelope, uint8(loop ? 1 : 0), uint8(volumeEnv ? 1 : 0)}; }
		static constexpr Event MED_SetVolume(uint8 volume) noexcept { return Event{Type::MED_SetVolume, volume}; }
		static constexpr Event MED_SetWaveform(uint8 waveform) noexcept { return Event{Type::MED_SetWaveform, waveform}; }
		static constexpr Event MED_SetVibratoSpeed(uint8 depth) noexcept { return Event{Type::MED_SetVibratoSpeed, depth}; }
		static constexpr Event MED_SetVibratoDepth(uint8 depth) noexcept { return Event{Type::MED_SetVibratoDepth, depth}; }
		static constexpr Event MED_SetVolumeStep(int16 volumeStep) noexcept { return Event{Type::MED_SetVolumeStep, volumeStep}; }
		static constexpr Event MED_SetPeriodStep(int16 periodStep) noexcept { return Event{Type::MED_SetPeriodStep, periodStep}; }
		static constexpr Event MED_HoldDecay(uint8 hold, uint16 decay) noexcept { return Event{Type::MED_HoldDecay, decay, hold}; }

		static constexpr Event FTM_SetCondition(uint16 threshold, uint8 condition) noexcept { return Event{Type::FTM_SetCondition, threshold, condition}; }
		static constexpr Event FTM_SetInterrupt(uint16 target, uint8 type) noexcept { return Event{Type::FTM_SetInterrupt, target, type}; }
		static constexpr Event FTM_PlaySample() noexcept { return Event{Type::FTM_PlaySample}; }
		static constexpr Event FTM_SetPitch(uint16 pitch) noexcept { return Event{Type::FTM_SetPitch, pitch}; }
		static constexpr Event FTM_AddPitch(int16 pitch) noexcept { return Event{Type::FTM_AddPitch, pitch}; }
		static constexpr Event FTM_SetDetune(uint16 detune) noexcept { return Event{Type::FTM_SetDetune, detune}; }
		static constexpr Event FTM_AddDetune(int16 detune) noexcept { return Event{Type::FTM_AddDetune, detune}; }
		static constexpr Event FTM_SetVolume(uint8 volume) noexcept { return Event{Type::FTM_SetVolume, volume}; }
		static constexpr Event FTM_AddVolume(int16 volume) noexcept { return Event{Type::FTM_AddVolume, volume}; }
		static constexpr Event FTM_SetSample(uint8 sample) noexcept { return Event{Type::FTM_SetSample, sample}; }
		static constexpr Event FTM_SetSampleStart(uint16 offset, uint8 type) noexcept { return Event{Type::FTM_SetSampleStart, offset, type}; }
		static constexpr Event FTM_SetOneshotLength(uint16 length, uint8 type) noexcept { return Event{Type::FTM_SetOneshotLength, length, type}; }
		static constexpr Event FTM_SetRepeatLength(uint16 length, uint8 type) noexcept { return Event{ Type::FTM_SetRepeatLength, length, type }; }
		static constexpr Event FTM_CloneTrack(uint8 track, uint8 properties) noexcept { return Event{Type::FTM_CloneTrack, track, properties}; }
		static constexpr Event FTM_StartLFO(uint8 lfo, uint8 targetWaveform) noexcept { return Event{Type::FTM_StartLFO, lfo, targetWaveform, 0}; }
		static constexpr Event FTM_LFOAddSub(uint8 lfoAddSub, uint8 speed, uint8 depth) noexcept { return Event{Type::FTM_LFOAddSub, lfoAddSub, speed, depth}; }
		static constexpr Event FTM_SetWorkTrack(uint8 track, bool relative) noexcept { return Event{ Type::FTM_SetWorkTrack, track, uint8(relative ? 1 : 0), 0}; }
		static constexpr Event FTM_SetGlobalVolume(uint16 globalVolume) noexcept { return Event{Type::FTM_SetGlobalVolume, globalVolume}; }
		static constexpr Event FTM_SetTempo(uint16 tempo) noexcept { return Event{Type::FTM_SetTempo, tempo}; }
		static constexpr Event FTM_SetSpeed(uint16 speed) noexcept { return Event{Type::FTM_SetSpeed, speed}; }
		static constexpr Event FTM_SetPlayPosition(uint16 pattern, uint8 row) noexcept { return Event{Type::FTM_SetPlayPosition, pattern, row}; }

		static constexpr Event FC_SetWaveform(uint8 command, uint8 waveform, uint8 samplePack) noexcept { return Event{Type::FC_SetWaveform, command, waveform, samplePack}; }
		static constexpr Event FC_SetPitch(int8 pitch) noexcept { return Event{Type::FC_SetPitch, pitch}; }
		static constexpr Event FC_SetVibrato(uint8 speed, uint8 depth, uint8 delay) noexcept { return Event{Type::FC_SetVibrato, speed, depth, delay}; }
		static constexpr Event FC_PitchSlide(uint8 speed, uint8 time) noexcept { return Event{Type::FC_PitchSlide, speed, time}; }
		static constexpr Event FC_VolumeSlide(uint8 speed, uint8 time) noexcept { return Event{Type::FC_VolumeSlide, speed, time}; }

		constexpr Event() noexcept : u8{}, u16{} {}
		constexpr Event(const Event &other) noexcept = default;
		constexpr Event(Event &&other) noexcept = default;
		constexpr Event &operator=(const Event &other) noexcept = default;
		constexpr Event &operator=(Event &&other) noexcept = default;

		MPT_CONSTEXPR20_FUN bool IsJumpEvent() const noexcept
		{
			return mpt::contains(JumpEvents, type);
		}

		template <typename TMap>
		void FixupJumpTarget(TMap &offsetToIndexMap)
		{
			if(!IsJumpEvent())
				return;
			if(auto it = offsetToIndexMap.lower_bound(u16); it != offsetToIndexMap.end())
				u16 = it->second;
			else
				u16 = uint16_max;
		}

		constexpr uint8 Byte0() const noexcept { return u8; }
		constexpr uint8 Byte1() const noexcept { return bytes[0]; }
		constexpr uint8 Byte2() const noexcept { return bytes[1]; }
		constexpr uint32 Value24Bit() const noexcept { return Byte0() | (Byte1() << 8) | (Byte2() << 16); }

	protected:
		constexpr Event(Type type, uint8 b1, uint8 b2, uint8 b3) noexcept : type{type}, u8{b1}, bytes{b2, b3} {}
		constexpr Event(Type type, int8 b1, uint8 b2, uint8 b3) noexcept : type{type}, i8{b1}, bytes{b2, b3} {}
		constexpr Event(Type type, int8 b1, int8 b2, uint8 b3) noexcept : type{type}, i8{b1}, bytes{static_cast<uint8>(b2), b3} {}
		constexpr Event(Type type, uint8 b1, uint8 b2) noexcept : type{type}, u8{b1}, bytes{b2, 0} {}
		constexpr Event(Type type, uint16 u16, uint8 u8) noexcept : type{type}, u8{u8}, u16{u16} {}
		constexpr Event(Type type, uint16 u16) noexcept : type{type}, u8{0}, u16{u16} {}
		constexpr Event(Type type, int16 i16) noexcept : type{type}, u8{}, i16{i16} {}
		constexpr Event(Type type, uint8 u8) noexcept : type{type}, u8{u8}, u16{} {}
		constexpr Event(Type type, int8 i8) noexcept : type{type}, i8{i8}, u16{} {}
		explicit constexpr Event(Type type) noexcept : type{type}, u8{}, u16{} {}

		static constexpr Event Event24Bit(Type type, uint32 value) { value = std::min(value, uint32(0xFFFFFF)); return Event{type, static_cast<uint8>(value & 0xFF), static_cast<uint8>(value >> 8), static_cast<uint8>(value >> 16)}; }
	};

	using Events = std::vector<Event>;

	class States
	{
	public:
		struct State;
		friend struct State;

		States();
		States(const States &other);
		States(States &&other) noexcept;
		virtual ~States();
		States& operator=(const States &other);
		States& operator=(States &&other) noexcept;

		void Stop();
		void NextTick(PlayState &playState, CHANNELINDEX channel, const CSoundFile &sndFile);
		void ApplyChannelState(ModChannel &chn, int32 &period, const CSoundFile& sndFile);

	protected:
		std::vector<State> states;
	};

	std::vector<Events> m_scripts;

	bool HasScripts() const noexcept { return !m_scripts.empty(); }
	void Clear() { m_scripts.clear(); }
	void Sanitize();
};


struct GlobalScriptState final : private InstrumentSynth::States
{
	void Initialize(const CSoundFile &sndFile);
	void NextTick(PlayState &playState, const CSoundFile &sndFile);
	void ApplyChannelState(PlayState &playState, CHANNELINDEX chn, int32 &period, const CSoundFile &sndFile);
};

OPENMPT_NAMESPACE_END
