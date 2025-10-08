/*
 * PlugInterface.h
 * ---------------
 * Purpose: Interface class for plugin handling
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#ifndef NO_PLUGINS

#include "../../soundlib/Snd_defs.h"
#include "../../soundlib/MIDIEvents.h"
#include "../../soundlib/Mixer.h"
#include "PluginMixBuffer.h"
#include "PluginStructs.h"

OPENMPT_NAMESPACE_BEGIN

struct VSTPluginLib;
struct SNDMIXPLUGIN;
struct ModInstrument;
struct ModChannel;
struct PlayState;
class CSoundFile;
class CModDoc;
class CAbstractVstEditor;

struct SNDMIXPLUGINSTATE
{
	// dwFlags flags
	enum PluginStateFlags
	{
		psfMixReady      = 0x01, // Set when cleared
		psfHasInput      = 0x02, // Set when plugin has non-silent input
		psfSilenceBypass = 0x04, // Bypass because of silence detection
	};

	mixsample_t *pMixBuffer = nullptr; // Stereo effect send buffer
	uint32 dwFlags = 0;                // PluginStateFlags
	uint32 inputSilenceCount = 0;      // How much silence has been processed? (for plugin auto-turnoff)
	mixsample_t nVolDecayL = 0, nVolDecayR = 0; // End of sample click removal

	void ResetSilence()
	{
		dwFlags |= psfHasInput;
		dwFlags &= ~psfSilenceBypass;
		inputSilenceCount = 0;
	}
};


class IMixPlugin
{
	friend class CAbstractVstEditor;
	friend struct VSTPluginLib;

protected:
	IMixPlugin *m_pNext = nullptr, *m_pPrev = nullptr;
	VSTPluginLib &m_Factory;
	CSoundFile &m_SndFile;
	SNDMIXPLUGIN *m_pMixStruct;
#ifdef MODPLUG_TRACKER
	CAbstractVstEditor *m_pEditor = nullptr;
#endif // MODPLUG_TRACKER

public:
	SNDMIXPLUGINSTATE m_MixState;
	PluginMixBuffer<float, MIXBUFFERSIZE> m_mixBuffer;	// Float buffers (input and output) for plugins

protected:
	mixsample_t m_MixBuffer[MIXBUFFERSIZE * 2 + 2];		// Stereo interleaved input (sample mixer renders here)

	float m_fGain = 1.0f;
	PLUGINDEX m_nSlot = 0;

	bool m_isSongPlaying = false;
	bool m_isResumed = false;

public:
	bool m_recordAutomation = false;
	bool m_passKeypressesToPlug = false;
	bool m_recordMIDIOut = false;

	// Combine with note value sent to IMixPlugin::MidiCommand
	enum MidiNoteFlag : uint16
	{
		MIDI_NOTE_MASK     = 0x0FF,
		MIDI_NOTE_OFF      = 0x100,  // Send note-off for a specific note
		MIDI_NOTE_ARPEGGIO = 0x200,  // Note is part of an arpeggio, don't store it as the last triggered note
	};


protected:
	virtual ~IMixPlugin();

public:
	// Non-virtual part of the interface
	IMixPlugin(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct);
	inline CSoundFile &GetSoundFile() { return m_SndFile; }
	inline const CSoundFile &GetSoundFile() const { return m_SndFile; }

#ifdef MODPLUG_TRACKER
	CModDoc *GetModDoc();
	const CModDoc *GetModDoc() const;

	void SetSlot(PLUGINDEX slot);
	inline PLUGINDEX GetSlot() const { return m_nSlot; }
#endif // MODPLUG_TRACKER

	inline VSTPluginLib &GetPluginFactory() const { return m_Factory; }
	// Returns the next instance of the same plugin
	inline IMixPlugin *GetNextInstance() const { return m_pNext; }

	void SetDryRatio(float dryRatio);
	bool IsBypassed() const;
	void RecalculateGain();
	// Query output latency from host (in seconds)
	double GetOutputLatency() const;

	// Destroy the plugin
	virtual void Release() { delete this; }
	virtual int32 GetUID() const = 0;
	virtual int32 GetVersion() const = 0;
	virtual void Idle() = 0;
	// Plugin latency in samples
	virtual uint32 GetLatency() const = 0;

	virtual int32 GetNumPrograms() const = 0;
	virtual int32 GetCurrentProgram() = 0;
	virtual void SetCurrentProgram(int32 nIndex) = 0;

	virtual PlugParamIndex GetNumParameters() const = 0;
	virtual PlugParamIndex GetNumVisibleParameters() const { return GetNumParameters(); }
	virtual PlugParamValue GetParameter(PlugParamIndex nIndex) = 0;
	virtual void SetParameter(PlugParamIndex paramIndex, PlugParamValue paramValue, PlayState *playState = nullptr, CHANNELINDEX chn = CHANNELINDEX_INVALID) = 0;

	// Save parameters for storing them in a module file
	virtual void SaveAllParameters();
	// Restore parameters from module file
	virtual void RestoreAllParameters(int32 program);
	virtual void Process(float *pOutL, float *pOutR, uint32 numFrames) = 0;
	void ProcessMixOps(float *pOutL, float *pOutR, float *leftPlugOutput, float *rightPlugOutput, uint32 numFrames);
	// Render silence and return the highest resulting output level
	virtual float RenderSilence(uint32 numSamples);

	// MIDI event handling
	bool MidiSend(uint32 midiCode);
	virtual bool MidiSend(mpt::const_byte_span /*midiData*/) { return true; }
	virtual void MidiCC(MIDIEvents::MidiCC /*nController*/, uint8 /*nParam*/, CHANNELINDEX /*trackChannel*/) { }
	virtual void MidiPitchBendRaw(int32 /*pitchbend*/, CHANNELINDEX /*trackChannel*/) {}
	virtual void MidiPitchBend(int32 /*increment*/, int8 /*pwd*/, CHANNELINDEX /*trackChannel*/) { }
	virtual void MidiTonePortamento(int32 /*increment*/, uint8 /*newNote*/, int8 /*pwd*/, CHANNELINDEX /*trackChannel*/) { }
	virtual void MidiVibrato(int32 /*depth*/, int8 /*pwd*/, CHANNELINDEX /*trackerChn*/) { }
	virtual void MidiCommand(const ModInstrument &/*instr*/, uint16 /*note*/, uint16 /*vol*/, CHANNELINDEX /*trackChannel*/) { }
	virtual void HardAllNotesOff() { }
	virtual bool IsNotePlaying(uint8 /*note*/, CHANNELINDEX /*trackerChn*/) { return false; }
	virtual void MoveChannel(CHANNELINDEX /*from*/, CHANNELINDEX /*to*/) { }

	// Modify parameter by given amount. Only needs to be re-implemented if plugin architecture allows this to be performed atomically.
	virtual void ModifyParameter(PlugParamIndex nIndex, PlugParamValue diff, PlayState &playState, CHANNELINDEX chn);
	virtual void NotifySongPlaying(bool playing) { m_isSongPlaying = playing; }
	virtual bool IsSongPlaying() const { return m_isSongPlaying; }
	virtual bool IsResumed() const { return m_isResumed; }
	virtual void Resume() = 0;
	virtual void Suspend() = 0;
	// Tell the plugin that there is a discontinuity between the previous and next render call (e.g. aftert jumping around in the module)
	virtual void PositionChanged() = 0;
	virtual void Bypass(bool = true);
	bool ToggleBypass() { Bypass(!IsBypassed()); return IsBypassed(); }
	virtual bool IsInstrument() const = 0;
	virtual bool CanRecieveMidiEvents() = 0;
	// If false is returned, mixing this plugin can be skipped if its input are currently completely silent.
	virtual bool ShouldProcessSilence() = 0;
	virtual void ResetSilence() { m_MixState.ResetSilence(); }

	size_t GetOutputPlugList(std::vector<IMixPlugin *> &list);
	size_t GetInputPlugList(std::vector<IMixPlugin *> &list);
	size_t GetInputInstrumentList(std::vector<INSTRUMENTINDEX> &list);
	size_t GetInputChannelList(std::vector<CHANNELINDEX> &list);

#ifdef MODPLUG_TRACKER
	bool SaveProgram();
	bool LoadProgram(mpt::PathString fileName = mpt::PathString());

	virtual CString GetDefaultEffectName() = 0;

	// Cache a range of names, in case one-by-one retrieval would be slow (e.g. when using plugin bridge)
	virtual void CacheProgramNames(int32 /*firstProg*/, int32 /*lastProg*/) { }
	virtual void CacheParameterNames(int32 /*firstParam*/, int32 /*lastParam*/) { }

	// Allowed value range for a parameter
	virtual std::pair<PlugParamValue, PlugParamValue> GetParamUIRange(PlugParamIndex /*param*/) { return {0.0f, 1.0f}; }
	// Scale allowed value range of a parameter to/from [0,1]
	PlugParamValue GetScaledUIParam(PlugParamIndex param);
	void SetScaledUIParam(PlugParamIndex param, PlugParamValue value);

	virtual CString GetParamName(PlugParamIndex param) = 0;
	virtual CString GetParamLabel(PlugParamIndex param) = 0;
	virtual CString GetParamDisplay(PlugParamIndex param) = 0;
	CString GetFormattedParamName(PlugParamIndex param);
	CString GetFormattedParamValue(PlugParamIndex param);
	virtual CString GetCurrentProgramName() = 0;
	virtual void SetCurrentProgramName(const CString &name) = 0;
	virtual CString GetProgramName(int32 program) = 0;
	CString GetFormattedProgramName(int32 index);

	virtual bool HasEditor() const = 0;
protected:
	virtual CAbstractVstEditor *OpenEditor();
public:
	// Get the plugin's editor window
	CAbstractVstEditor *GetEditor() { return m_pEditor; }
	const CAbstractVstEditor *GetEditor() const { return m_pEditor; }
	void ToggleEditor();
	void CloseEditor();
	void SetEditorPos(int32 x, int32 y);
	void GetEditorPos(int32 &x, int32 &y) const;

	// Notify OpenMPT that a plugin parameter has changed and set document as modified
	void AutomateParameter(PlugParamIndex param);
	// Plugin state changed, set document as modified.
	void SetModified();
#endif

	virtual int GetNumInputChannels() const = 0;
	virtual int GetNumOutputChannels() const = 0;

	using ChunkData = mpt::const_byte_span;
	virtual bool ProgramsAreChunks() const { return false; }
	virtual ChunkData GetChunk(bool /*isBank*/) { return ChunkData(); }
	virtual void SetChunk(const ChunkData &/*chunk*/, bool /*isBank*/) { }

	virtual void BeginSetProgram(int32 /*program*/ = -1) {}
	virtual void EndSetProgram() {}
	virtual void BeginGetProgram(int32 /*program*/ = -1) {}
	virtual void EndGetProgram() {}
};


inline void IMixPlugin::ModifyParameter(PlugParamIndex nIndex, PlugParamValue diff, PlayState &playState, CHANNELINDEX chn)
{
	PlugParamValue val = GetParameter(nIndex) + diff;
	Limit(val, PlugParamValue(0), PlugParamValue(1));
	SetParameter(nIndex, val, &playState, chn);
}


// IMidiPlugin: Default implementation of plugins with MIDI input

class IMidiPlugin : public IMixPlugin
{
protected:
	enum
	{
		// Pitch wheel constants
		kPitchBendShift = 12,  // Use lowest 12 bits for fractional part and vibrato flag => 16.11 fixed point precision
		kPitchBendMask  = (~1),
		kVibratoFlag    = 1,
	};

	struct PlugInstrChannel
	{
		int32 midiPitchBendPos = 0;  // Current Pitch Wheel position, in 16.11 fixed point format. Lowest bit is used for indicating that vibrato was applied. Vibrato offset itself is not stored in this value.
		uint16 currentProgram = uint16_max;
		uint16 currentBank = uint16_max;
		uint8 lastNote = 0 /* NOTE_NONE */;
		uint8  noteOnMap[128][MAX_CHANNELS];

		void ResetProgram(bool oldBehaviour) { currentProgram = oldBehaviour ? 0 : uint16_max; currentBank = oldBehaviour ? 0 : uint16_max; }
	};

	std::array<PlugInstrChannel, 16> m_MidiCh;  // MIDI channel state

public:
	IMidiPlugin(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct);

	void MidiCC(MIDIEvents::MidiCC nController, uint8 nParam, CHANNELINDEX trackChannel) override;
	void MidiPitchBendRaw(int32 pitchbend, CHANNELINDEX trackerChn) override;
	void MidiPitchBend(int32 increment, int8 pwd, CHANNELINDEX trackerChn) override;
	void MidiTonePortamento(int32 increment, uint8 newNote, int8 pwd, CHANNELINDEX trackerChn) override;
	void MidiVibrato(int32 depth, int8 pwd, CHANNELINDEX trackerChn) override;
	void MidiCommand(const ModInstrument &instr, uint16 note, uint16 vol, CHANNELINDEX trackChannel) override;
	bool IsNotePlaying(uint8 note, CHANNELINDEX trackerChn) override;
	void MoveChannel(CHANNELINDEX from, CHANNELINDEX to) override;

	// Get the MIDI channel currently associated with a given tracker channel
	virtual uint8 GetMidiChannel(const ModChannel &chn, CHANNELINDEX trackChannel) const;

protected:
	uint8 GetMidiChannel(CHANNELINDEX trackChannel) const;

	// Plugin wants to send MIDI to OpenMPT
	virtual void ReceiveMidi(mpt::const_byte_span midiData);

	// Converts a 14-bit MIDI pitch bend position to our internal pitch bend position representation
	static constexpr int32 EncodePitchBendParam(int32 position) { return (position << kPitchBendShift); }
	// Converts the internal pitch bend position to a 14-bit MIDI pitch bend position
	static constexpr int16 DecodePitchBendParam(int32 position) { return static_cast<int16>(position >> kPitchBendShift); }
	// Apply Pitch Wheel Depth (PWD) to some MIDI pitch bend value.
	static inline void ApplyPitchWheelDepth(int32 &value, int8 pwd);

	void SendMidiPitchBend(uint8 midiCh, int32 newPitchBendPos);
};

OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS

