/*
 * LFOPlugin.h
 * -----------
 * Purpose: Plugin for automating other plugins' parameters
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#ifndef NO_PLUGINS

#include "PlugInterface.h"
#include "../../common/mptRandom.h"

OPENMPT_NAMESPACE_BEGIN

class LFOPlugin final : public IMixPlugin
{
	friend class LFOPluginEditor;

protected:
	enum Parameters
	{
		kAmplitude = 0,
		kOffset,
		kFrequency,
		kTempoSync,
		kWaveform,
		kPolarity,
		kBypassed,
		kLoopMode,
		kCurrentPhase,
		kLFONumParameters
	};

	enum LFOWaveform
	{
		kSine = 0,
		kTriangle,
		kSaw,
		kSquare,
		kSHNoise,
		kSmoothNoise,
		kNumWaveforms
	};

	std::vector<std::byte> m_chunkData;

	static constexpr PlugParamIndex INVALID_OUTPUT_PARAM = uint32_max;

	// LFO parameters
	float m_amplitude = 0.5f, m_offset = 0.5f, m_frequency = 0.290241f;
	LFOWaveform m_waveForm = kSine;
	PlugParamIndex m_outputParam = INVALID_OUTPUT_PARAM;
	bool m_tempoSync = false, m_polarity = false, m_bypassed = false, m_outputToCC = false, m_oneshot = false;

	// LFO state
	double m_computedFrequency = 0.0;
	double m_phase = 0.0, m_increment = 0.0;
	double m_random = 0.0, m_nextRandom = 0.0;
	double m_tempo = 0.0;

	mpt::fast_prng m_PRNG;

#ifdef MODPLUG_TRACKER
	static constexpr int WM_PARAM_UDPATE = WM_USER + 500;
#endif

public:
	static IMixPlugin* Create(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct);
	LFOPlugin(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct);

	int32 GetUID() const override { int32 id; memcpy(&id, "LFO ", 4); return id; }
	int32 GetVersion() const override { return 0; }
	void Idle() override { }
	uint32 GetLatency() const override { return 0; }

	void Process(float *pOutL, float *pOutR, uint32 numFrames) override;

	float RenderSilence(uint32) override { return 0.0f; }

	// MIDI event handling (mostly passing it through to the follow-up plugin)
	bool MidiSend(mpt::const_byte_span midiData) override;
	void MidiCC(MIDIEvents::MidiCC nController, uint8 nParam, CHANNELINDEX trackChannel) override;
	void MidiPitchBend(int32 increment, int8 pwd, CHANNELINDEX trackChannel) override;
	void MidiTonePortamento(int32 increment, uint8 newNote, int8 pwd, CHANNELINDEX trackChannel) override;
	void MidiVibrato(int32 depth, int8 pwd, CHANNELINDEX trackChannel) override;
	void MidiCommand(const ModInstrument &instr, uint16 note, uint16 vol, CHANNELINDEX trackChannel) override;
	void HardAllNotesOff() override;
	bool IsNotePlaying(uint8 note, CHANNELINDEX trackerChn) override;

	int32 GetNumPrograms() const override { return 0; }
	int32 GetCurrentProgram() override { return 0; }
	void SetCurrentProgram(int32) override { }

	PlugParamIndex GetNumParameters() const override { return kLFONumParameters; }
	PlugParamValue GetParameter(PlugParamIndex index) override;
	void SetParameter(PlugParamIndex index, PlugParamValue value, PlayState * = nullptr, CHANNELINDEX = CHANNELINDEX_INVALID) override;

	void Resume() override;
	void Suspend() override { m_isResumed = false; }
	void PositionChanged() override;

	bool IsInstrument() const override { return false; }
	bool CanRecieveMidiEvents() override { return false; }
	bool ShouldProcessSilence() override { return true; }

#ifdef MODPLUG_TRACKER
	CString GetDefaultEffectName() override { return _T("LFO"); }

	std::pair<PlugParamValue, PlugParamValue> GetParamUIRange(PlugParamIndex param) override;
	CString GetParamName(PlugParamIndex param) override;
	CString GetParamLabel(PlugParamIndex) override;
	CString GetParamDisplay(PlugParamIndex param) override;

	CString GetCurrentProgramName() override { return CString(); }
	void SetCurrentProgramName(const CString &) override { }
	CString GetProgramName(int32) override { return CString(); }

	bool HasEditor() const override { return true; }
protected:
	CAbstractVstEditor *OpenEditor() override;
#endif

public:
	int GetNumInputChannels() const override { return 2; }
	int GetNumOutputChannels() const override { return 2; }

	bool ProgramsAreChunks() const override { return true; }
	// Save parameters for storing them in a module file
	void SaveAllParameters() override;
	// Restore parameters from module file
	void RestoreAllParameters(int32 program) override;
	ChunkData GetChunk(bool) override;
	void SetChunk(const ChunkData &chunk, bool) override;

protected:
	void NextRandom();
	void RecalculateFrequency();
	void RecalculateIncrement();
	IMixPlugin *GetOutputPlugin() const;

public:
	static LFOWaveform ParamToWaveform(float param) { return static_cast<LFOWaveform>(std::clamp(mpt::saturate_round<int>(param * 32.0f), 0, kNumWaveforms - 1)); }
	static float WaveformToParam(LFOWaveform waveform) { return static_cast<float>(static_cast<int>(waveform)) / 32.0f; }
};

OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS
