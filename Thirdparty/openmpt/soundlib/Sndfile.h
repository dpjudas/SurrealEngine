/*
 * Sndfile.h
 * ---------
 * Purpose: Core class of the playback engine. Every song is represented by a CSoundFile object.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundFilePlayConfig.h"
#include "MixerSettings.h"
#include "../common/misc_util.h"
#include "../common/mptFileType.h"
#include "../common/mptRandom.h"
#include "../common/version.h"
#include "Snd_defs.h"
#include "tuningbase.h"
#include "MIDIMacros.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/MIDIMapping.h"
#endif // MODPLUG_TRACKER

#include "Mixer.h"
#include "Resampler.h"
#ifndef NO_REVERB
#include "../sounddsp/Reverb.h"
#endif
#ifndef NO_AGC
#include "../sounddsp/AGC.h"
#endif
#ifndef NO_DSP
#include "../sounddsp/DSP.h"
#endif
#ifndef NO_EQ
#include "../sounddsp/EQ.h"
#endif

#include "Message.h"
#include "ModChannel.h"
#include "modcommand.h"
#include "ModInstrument.h"
#include "ModSample.h"
#include "ModSequence.h"
#include "pattern.h"
#include "patternContainer.h"
#include "PlayState.h"
#include "plugins/PluginStructs.h"
#include "RowVisitor.h"

#include "mpt/audio/span.hpp"

#include "../common/FileReaderFwd.h"

#include <vector>
#include <bitset>
#include <set>


OPENMPT_NAMESPACE_BEGIN


bool SettingCacheCompleteFileBeforeLoading();


// Sample decompression routines in format-specific source files
void AMSUnpack(mpt::const_byte_span source, mpt::byte_span dest, int8 packCharacter);
uintptr_t DMFUnpack(FileReader &file, uint8 *psample, uint32 maxlen);


#ifdef LIBOPENMPT_BUILD
#ifndef NO_PLUGINS
class CVstPluginManager;
#endif
#endif


using PlayBehaviourSet = std::bitset<kMaxPlayBehaviours>;

#ifdef MODPLUG_TRACKER

// For WAV export (writing pattern positions to file)
struct PatternCuePoint
{
	uint64     offset;    // offset in the file (in samples)
	ORDERINDEX order;     // which order is this?
	bool       processed; // has this point been processed by the main WAV render function yet?
};

#endif // MODPLUG_TRACKER


// Return values for GetLength()
struct GetLengthType
{
	double     duration = 0.0;                     // Total time in seconds
	ROWINDEX   restartRow = ROWINDEX_INVALID;      // First row to play after module loops (or last parsed row if target is specified; equal to target if it was found)
	ROWINDEX   endRow = ROWINDEX_INVALID;          // Last row played before module loops (UNDEFINED if a target is specified)
	ROWINDEX   startRow = 0;                       // First row of parsed subsong
	ORDERINDEX restartOrder = ORDERINDEX_INVALID;  // First row to play after module loops (see restartRow remark)
	ORDERINDEX endOrder = ORDERINDEX_INVALID;      // Last order played before module loops (UNDEFINED if a target is specified)
	ORDERINDEX startOrder = 0;                     // First order of parsed subsong
	bool       targetReached = false;              // True if the specified order/row combination or duration has been reached while going through the module
};


struct SubSong
{
	double duration;
	ROWINDEX startRow, endRow, loopStartRow;
	ORDERINDEX startOrder, endOrder, loopStartOrder;
	SEQUENCEINDEX sequence;
};


// Target seek mode for GetLength()
struct GetLengthTarget
{
	ROWINDEX startRow;
	ORDERINDEX startOrder;
	SEQUENCEINDEX sequence;
	
	struct pos_type
	{
		ROWINDEX row;
		ORDERINDEX order;
	};

	union
	{
		double time;
		pos_type pos;
	};

	enum Mode
	{
		NoTarget,       // Don't seek, i.e. return complete length of the first subsong.
		GetAllSubsongs, // Same as NoTarget (i.e. get complete length), but returns the length of all sub songs
		SeekPosition,   // Seek to given pattern position.
		SeekSeconds,    // Seek to given time.
	} mode;

	// Don't seek, i.e. return complete module length.
	GetLengthTarget(bool allSongs = false)
	{
		mode = allSongs ? GetAllSubsongs : NoTarget;
		sequence = SEQUENCEINDEX_INVALID;
		startOrder = 0;
		startRow = 0;
	}

	// Seek to given pattern position if position is valid.
	GetLengthTarget(ORDERINDEX order, ROWINDEX row)
	{
		mode = NoTarget;
		sequence = SEQUENCEINDEX_INVALID;
		startOrder = 0;
		startRow = 0;
		if(order != ORDERINDEX_INVALID && row != ROWINDEX_INVALID)
		{
			mode = SeekPosition;
			pos.row = row;
			pos.order = order;
		}
	}

	// Seek to given time if t is valid (i.e. not negative).
	GetLengthTarget(double t)
	{
		mode = NoTarget;
		sequence = SEQUENCEINDEX_INVALID;
		startOrder = 0;
		startRow = 0;
		if(t >= 0.0)
		{
			mode = SeekSeconds;
			time = t;
		}
	}

	// Set start position from which seeking should begin.
	GetLengthTarget &StartPos(SEQUENCEINDEX seq, ORDERINDEX order, ROWINDEX row)
	{
		sequence = seq;
		startOrder = order;
		startRow = row;
		return *this;
	}
};


// Reset mode for GetLength()
enum enmGetLengthResetMode
{
	// Never adjust global variables / mod parameters
	eNoAdjust = 0x00,
	// Mod parameters (such as global volume, speed, tempo, etc...) will always be memorized if the target was reached (i.e. they won't be reset to the previous values).  If target couldn't be reached, they are reset to their default values.
	eAdjust = 0x01,
	// Same as above, but global variables will only be memorized if the target could be reached. This does *NOT* influence the visited rows vector - it will *ALWAYS* be adjusted in this mode.
	eAdjustOnSuccess = 0x02 | eAdjust,
	// Same as previous option, but will also try to emulate sample playback so that voices from previous patterns will sound when continuing playback at the target position.
	eAdjustSamplePositions = 0x04 | eAdjustOnSuccess,
	// Only adjust the visited rows state
	eAdjustOnlyVisitedRows = 0x08,
};


// Delete samples assigned to instrument
enum deleteInstrumentSamples
{
	deleteAssociatedSamples,
	doNoDeleteAssociatedSamples,
};


namespace Tuning {
class CTuningCollection;
} // namespace Tuning
using CTuningCollection = Tuning::CTuningCollection;
struct CModSpecifications;
class OPL;
class PlaybackTest;
class CModDoc;


/////////////////////////////////////////////////////////////////////////
// File edit history

#define HISTORY_TIMER_PRECISION	18.2

struct FileHistory
{
	// Date when the file was loaded in the the tracker or created.
	mpt::Date::AnyGregorian loadDate = {};
	// Time the file was open in the editor, in 1/18.2th seconds (frequency of a standard DOS timer, to keep compatibility with Impulse Tracker easy).
	uint32 openTime = 0;
	// Return the date as a (possibly truncated if not enough precision is available) ISO 8601 formatted date.
	mpt::ustring AsISO8601(mpt::Date::LogicalTimezone internalTimezone) const;
	// Returns true if the date component is valid. Some formats only store edit time, not edit date.
	bool HasValidDate() const
	{
		return loadDate != mpt::Date::AnyGregorian{};
	}

	bool operator==(const FileHistory &other) const noexcept
	{
		return std::tie(loadDate, openTime) == std::tie(other.loadDate, other.openTime);
	}
	bool operator!=(const FileHistory &other) const noexcept
	{
		return std::tie(loadDate, openTime) != std::tie(other.loadDate, other.openTime);
	}
	bool operator<(const FileHistory &other) const noexcept
	{
		return std::tie(loadDate, openTime) < std::tie(other.loadDate, other.openTime);
	}
};


struct TimingInfo
{
	double InputLatency = 0.0; // seconds
	double OutputLatency = 0.0; // seconds
	int64 StreamFrames = 0;
	uint64 SystemTimestamp = 0; // nanoseconds
	double Speed = 1.0;
};


enum class ModMessageHeuristicOrder
{
	Instruments,
	Samples,
	InstrumentsSamples,
	SamplesInstruments,
	BothInstrumentsSamples,
	BothSamplesInstruments,
	Default = InstrumentsSamples,
};

struct ModFormatDetails
{
	mpt::ustring formatName;         // "FastTracker 2"
	mpt::ustring type;               // "xm"
	mpt::ustring madeWithTracker;    // "OpenMPT 1.28.01.00"
	mpt::ustring originalFormatName; // "FastTracker 2" in the case of converted formats like MO3 or GDM
	mpt::ustring originalType;       // "xm" in the case of converted formats like MO3 or GDM
	mpt::Charset charset = mpt::Charset::UTF8;
	mpt::Date::LogicalTimezone timezone = mpt::Date::LogicalTimezone::Unspecified;
};


class IAudioTarget
{
protected:
	virtual ~IAudioTarget() = default;
public:
	virtual void Process(mpt::audio_span_interleaved<MixSampleInt> buffer) = 0;
	virtual void Process(mpt::audio_span_interleaved<MixSampleFloat> buffer) = 0;
};


class IAudioSource
{
public:
	virtual ~IAudioSource() = default;
public:
	virtual void Process(mpt::audio_span_planar<MixSampleInt> buffer) = 0;
	virtual void Process(mpt::audio_span_planar<MixSampleFloat> buffer) = 0;
};


class IMonitorInput
{
public:
	virtual ~IMonitorInput() = default;
public:
	virtual void Process(mpt::audio_span_planar<const MixSampleInt> buffer) = 0;
	virtual void Process(mpt::audio_span_planar<const MixSampleFloat> buffer) = 0;
};


class IMonitorOutput
{
public:
	virtual ~IMonitorOutput() = default;
public:
	virtual void Process(mpt::audio_span_interleaved<const MixSampleInt> buffer) = 0;
	virtual void Process(mpt::audio_span_interleaved<const MixSampleFloat> buffer) = 0;
};


class AudioSourceNone
	: public IAudioSource
{
public:
	void Process(mpt::audio_span_planar<MixSampleInt> buffer) override
	{
		for(std::size_t channel = 0; channel < buffer.size_channels(); ++channel)
		{
			for(std::size_t frame = 0; frame < buffer.size_frames(); ++frame)
			{
				buffer(channel, frame) = 0;
			}
		}
	}
	void Process(mpt::audio_span_planar<MixSampleFloat> buffer) override
	{
		for(std::size_t channel = 0; channel < buffer.size_channels(); ++channel)
		{
			for(std::size_t frame = 0; frame < buffer.size_frames(); ++frame)
			{
				buffer(channel, frame) = MixSampleFloat(0.0);
			}
		}
	}
};


using NoteName = mpt::uchar[4];


struct PlaybackTestSettings
{
	uint32 mixingFreq = 48000;
	uint32 outputChannels = 2;
	uint32 mixerChannels = MAX_CHANNELS;
	ResamplingMode srcMode = SRCMODE_CUBIC;
	void Sanitize()
	{
		if(mixingFreq < 1000)
		{
			mixingFreq = 48000;
		}
		if(outputChannels != 1 && outputChannels != 2 && outputChannels != 4)
		{
			outputChannels = 2;
		}
		if(mixerChannels < 1)
		{
			mixerChannels = MAX_CHANNELS;
		}
	}
};


class CSoundFile
{
	friend class GetLengthMemory;
	friend class MIDIMacroParser;

public:
#ifdef MODPLUG_TRACKER
	void ChangeModTypeTo(const MODTYPE newType, bool adjust = true);
#endif // MODPLUG_TRACKER

	// Returns value in seconds. If given position won't be played at all, returns -1.
	// If updateVars is true, the state of various playback variables will be updated according to the playback position.
	// If updateSamplePos is also true, the sample positions of samples still playing from previous patterns will be kept in sync.
	double GetPlaybackTimeAt(ORDERINDEX ord, ROWINDEX row, bool updateVars, bool updateSamplePos);

	std::vector<SubSong> GetAllSubSongs();

	//Tuning-->
public:
	static std::unique_ptr<CTuning> CreateTuning12TET(const mpt::ustring &name);
	CTuningCollection& GetTuneSpecificTunings() {return *m_pTuningsTuneSpecific;}

	mpt::ustring GetNoteName(const ModCommand::NOTE note, const INSTRUMENTINDEX inst, const NoteName *noteNames = nullptr) const;
	mpt::ustring GetNoteName(const ModCommand::NOTE note) const;
	static mpt::ustring GetNoteName(const ModCommand::NOTE note, const NoteName *noteNames);
#ifdef MODPLUG_TRACKER
public:
	static void SetDefaultNoteNames();
	static const NoteName *GetDefaultNoteNames();
	static mpt::ustring GetDefaultNoteName(int note)  // note = [0..11]
	{
		return m_NoteNames[note];
	}
private:
	static const NoteName *m_NoteNames;
#else
private:
	const NoteName *m_NoteNames;
#endif

private:
	CTuningCollection* m_pTuningsTuneSpecific = nullptr;

private: //Misc data
	const CModSpecifications *m_pModSpecs;

private:
	// Interleaved Front Mix Buffer (Also room for interleaved rear mix)
	mixsample_t MixSoundBuffer[MIXBUFFERSIZE * 4];
	mixsample_t MixRearBuffer[MIXBUFFERSIZE * 2];
	// Non-interleaved plugin processing buffer
	float MixFloatBuffer[2][MIXBUFFERSIZE];
	mixsample_t MixInputBuffer[NUMMIXINPUTBUFFERS][MIXBUFFERSIZE];

	// End-of-sample pop reduction tail level
	mixsample_t m_dryLOfsVol = 0, m_dryROfsVol = 0;
	mixsample_t m_surroundLOfsVol = 0, m_surroundROfsVol = 0;

public:
	MixerSettings m_MixerSettings;
	CResampler m_Resampler;
#ifndef NO_REVERB
	mixsample_t ReverbSendBuffer[MIXBUFFERSIZE * 2];
	mixsample_t m_RvbROfsVol = 0, m_RvbLOfsVol = 0;
	CReverb m_Reverb;
#endif
#ifndef NO_DSP
	CSurround m_Surround;
	CMegaBass m_MegaBass;
#endif
#ifndef NO_EQ
	CEQ m_EQ;
#endif
#ifndef NO_AGC
	CAGC m_AGC;
#endif
#ifndef NO_DSP
	BitCrush m_BitCrush;
#endif

	static constexpr uint32 TICKS_ROW_FINISHED = uint32_max - 1u;

private:
#ifdef MODPLUG_TRACKER
	CModDoc *m_pModDoc = nullptr;  // Can be a null pointer for example when previewing samples from the treeview.
#endif // MODPLUG_TRACKER
	Enum<MODTYPE> m_nType;
	ModContainerType m_ContainerType = ModContainerType::None;
public:
	SAMPLEINDEX m_nSamples = 0;
	INSTRUMENTINDEX m_nInstruments = 0;
	uint32 m_nDefaultGlobalVolume;
	FlagSet<SongFlags> m_SongFlags;
	CHANNELINDEX m_nMixChannels = 0;
private:
	CHANNELINDEX m_nMixStat;
public:
	ROWINDEX m_nDefaultRowsPerBeat, m_nDefaultRowsPerMeasure;  // default rows per beat and measure for this module
	TempoMode m_nTempoMode = TempoMode::Classic;

#ifdef MODPLUG_TRACKER
	// Lock playback between two rows. Lock is active if lock start != ROWINDEX_INVALID).
	ROWINDEX m_lockRowStart = ROWINDEX_INVALID, m_lockRowEnd = ROWINDEX_INVALID;
	// Lock playback between two orders. Lock is active if lock start != ORDERINDEX_INVALID).
	ORDERINDEX m_lockOrderStart = ORDERINDEX_INVALID, m_lockOrderEnd = ORDERINDEX_INVALID;
#endif // MODPLUG_TRACKER

	uint32 m_nSamplePreAmp, m_nVSTiVolume;
	uint32 m_OPLVolumeFactor;  // 16.16
	static constexpr uint32 m_OPLVolumeFactorScale = 1 << 16;

	constexpr bool IsGlobalVolumeUnset() const noexcept { return IsFirstTick(); }
#ifndef MODPLUG_TRACKER
	uint32 m_nFreqFactor = 65536; // Pitch shift factor (65536 = no pitch shifting). Only used in libopenmpt (openmpt::ext::interactive::set_pitch_factor)
	uint32 m_nTempoFactor = 65536; // Tempo factor (65536 = no tempo adjustment). Only used in libopenmpt (openmpt::ext::interactive::set_tempo_factor)
#endif

	// Row swing factors for modern tempo mode
	TempoSwing m_tempoSwing;

	// Min Period = highest possible frequency, Max Period = lowest possible frequency for current format
	// Note: Period is an Amiga metric that is inverse to frequency.
	// Periods in MPT are 4 times as fine as Amiga periods because of extra fine frequency slides (introduced in the S3M format).
	int32 m_nMinPeriod, m_nMaxPeriod;

	ResamplingMode m_nResampling; // Resampling mode (if overriding the globally set resampling)
	int32 m_nRepeatCount = 0;     // -1 means repeat infinitely.
	ORDERINDEX m_restartOverridePos = 0, m_maxOrderPosition = 0;
	std::vector<ModChannelSettings> ChnSettings;  // Initial channels settings
	CPatternContainer Patterns;
	ModSequenceSet Order;  // Pattern sequences (order lists)
protected:
	ModSample Samples[MAX_SAMPLES];
public:
	ModInstrument *Instruments[MAX_INSTRUMENTS];  // Instrument Headers
	InstrumentSynth::Events m_globalScript;
	MIDIMacroConfig m_MidiCfg;                    // MIDI Macro config table
#ifndef NO_PLUGINS
	std::array<SNDMIXPLUGIN, MAX_MIXPLUGINS> m_MixPlugins;  // Mix plugins
	uint32 m_loadedPlugins = 0;                             // Not a PLUGINDEX because number of loaded plugins may exceed MAX_MIXPLUGINS during MIDI conversion
#endif
	mpt::charbuf<MAX_SAMPLENAME> m_szNames[MAX_SAMPLES];  // Sample names

	Version m_dwCreatedWithVersion;
	Version m_dwLastSavedWithVersion;

	PlayBehaviourSet m_playBehaviour;

protected:

	mpt::fast_prng m_PRNG;
	inline mpt::fast_prng & AccessPRNG() const { return const_cast<CSoundFile*>(this)->m_PRNG; }
	inline mpt::fast_prng & AccessPRNG() { return m_PRNG; }

protected:
	// Mix level stuff
	CSoundFilePlayConfig m_PlayConfig;
	MixLevels m_nMixLevels;

public:
	PlayState m_PlayState;

protected:
	// For handling backwards jumps and stuff to prevent infinite loops when counting the mod length or rendering to wav.
	RowVisitor m_visitedRows;

public:
#ifdef MODPLUG_TRACKER
	std::bitset<MAX_BASECHANNELS> m_bChannelMuteTogglePending;
	std::bitset<MAX_MIXPLUGINS> m_pluginDryWetRatioChanged;  // Dry/Wet ratio was changed by playback code (e.g. through MIDI macro), need to update UI

	std::vector<PatternCuePoint> *m_PatternCuePoints = nullptr;  // For WAV export (writing pattern positions to file)
	std::vector<SmpLength> *m_SamplePlayLengths = nullptr;       // For storing the maximum play length of each sample for automatic sample trimming
#endif // MODPLUG_TRACKER

	std::unique_ptr<OPL> m_opl;

#ifdef MODPLUG_TRACKER
public:
	CMIDIMapper& GetMIDIMapper() { return m_MIDIMapper; }
	const CMIDIMapper& GetMIDIMapper() const { return m_MIDIMapper; }
private:
	CMIDIMapper m_MIDIMapper;
#endif // MODPLUG_TRACKER

public:
#if defined(LIBOPENMPT_BUILD) && !defined(NO_PLUGINS)
	std::unique_ptr<CVstPluginManager> m_PluginManager;
#endif

public:
	std::string m_songName;
	mpt::ustring m_songArtist;
	SongMessage m_songMessage;
	ModFormatDetails m_modFormat;

protected:
	std::vector<FileHistory> m_FileHistory;	// File edit history
public:
	std::vector<FileHistory> &GetFileHistory() { return m_FileHistory; }
	const std::vector<FileHistory> &GetFileHistory() const { return m_FileHistory; }

#ifdef MPT_EXTERNAL_SAMPLES
	// MPTM external on-disk sample paths
protected:
	std::vector<mpt::PathString> m_samplePaths;

public:
	void SetSamplePath(SAMPLEINDEX smp, mpt::PathString filename) { if(m_samplePaths.size() < smp) m_samplePaths.resize(smp); m_samplePaths[smp - 1] = std::move(filename); }
	void ResetSamplePath(SAMPLEINDEX smp) { if(m_samplePaths.size() >= smp) m_samplePaths[smp - 1] = mpt::PathString(); Samples[smp].uFlags.reset(SMP_KEEPONDISK | SMP_MODIFIED);}
	mpt::PathString GetSamplePath(SAMPLEINDEX smp) const { if(m_samplePaths.size() >= smp) return m_samplePaths[smp - 1]; else return mpt::PathString(); }
	bool SampleHasPath(SAMPLEINDEX smp) const { if(m_samplePaths.size() >= smp) return !m_samplePaths[smp - 1].empty(); else return false; }
	bool IsExternalSampleMissing(SAMPLEINDEX smp) const { return Samples[smp].uFlags[SMP_KEEPONDISK] && !Samples[smp].HasSampleData(); }

	bool LoadExternalSample(SAMPLEINDEX smp, const mpt::PathString &filename);
#endif // MPT_EXTERNAL_SAMPLES

	bool m_bIsRendering = false;
	TimingInfo m_TimingInfo; // only valid if !m_bIsRendering

private:
	// logging
	ILog *m_pCustomLog = nullptr;

public:
	CSoundFile();
	CSoundFile(const CSoundFile &) = delete;
	CSoundFile & operator=(const CSoundFile &) = delete;
	~CSoundFile();

public:
	// logging
	void SetCustomLog(ILog *pLog) { m_pCustomLog = pLog; }
	void AddToLog(LogLevel level, const mpt::ustring &text) const;

public:

	enum ModLoadingFlags
	{
		loadPatternData    = 0x01, // If unset, advise loaders to not process any pattern data (if possible)
		loadSampleData     = 0x02, // If unset, advise loaders to not process any sample data (if possible)
		loadPluginData     = 0x04, // If unset, plugin data is not loaded (and as a consequence, plugins are not instantiated).
		loadPluginInstance = 0x08, // If unset, plugins are not instantiated.
		skipContainer      = 0x10,
		skipModules        = 0x20,
		onlyVerifyHeader   = 0x40, // Do not combine with other flags!

		// Shortcuts
		loadCompleteModule = loadSampleData | loadPatternData | loadPluginData | loadPluginInstance,
		loadNoPatternOrPluginData	= loadSampleData,
		loadNoPluginInstance = loadSampleData | loadPatternData | loadPluginData,
	};

	#define PROBE_RECOMMENDED_SIZE 2048u

	static constexpr std::size_t ProbeRecommendedSize = PROBE_RECOMMENDED_SIZE;

	enum ProbeFlags
	{
		ProbeModules    = 0x1,
		ProbeContainers = 0x2,

		ProbeFlagsDefault = ProbeModules | ProbeContainers,
		ProbeFlagsNone = 0
	};

	enum ProbeResult
	{
		ProbeSuccess      =  1,
		ProbeFailure      =  0,
		ProbeWantMoreData = -1
	};

	static ProbeResult ProbeAdditionalSize(MemoryFileReader &file, const uint64 *pfilesize, uint64 minimumAdditionalSize);

	static ProbeResult Probe(ProbeFlags flags, mpt::span<const std::byte> data, const uint64 *pfilesize);

public:

#ifdef MODPLUG_TRACKER
	// Get parent CModDoc. Can be nullptr if previewing from tree view, and is always nullptr if we're not actually compiling OpenMPT.
	CModDoc *GetpModDoc() const noexcept { return m_pModDoc; }
#endif  // MODPLUG_TRACKER

	void Create(MODTYPE type, CHANNELINDEX numChannels, CModDoc *pModDoc = nullptr);
	bool Create(FileReader file, ModLoadingFlags loadFlags = loadCompleteModule, CModDoc *pModDoc = nullptr);
private:
	bool CreateInternal(FileReader file, ModLoadingFlags loadFlags);

public:
	bool Destroy();
	Enum<MODTYPE> GetType() const noexcept { return m_nType; }

	ModContainerType GetContainerType() const noexcept { return m_ContainerType; }

	// rough heuristic, could be improved
	mpt::Charset GetCharsetFile() const // 8bit string encoding of strings in the on-disk file
	{
		return m_modFormat.charset;
	}
	mpt::Charset GetCharsetInternal() const // 8bit string encoding of strings internal in CSoundFile
	{
		#if defined(MODPLUG_TRACKER)
			return mpt::Charset::Locale;
		#else // MODPLUG_TRACKER
			return GetCharsetFile();
		#endif // MODPLUG_TRACKER
	}
	mpt::Date::LogicalTimezone GetTimezoneInternal() const
	{
		return m_modFormat.timezone;
	}

	ModMessageHeuristicOrder GetMessageHeuristic() const;

	void SetPreAmp(uint32 vol);
	uint32 GetPreAmp() const noexcept { return m_MixerSettings.m_nPreAmp; }

	void SetMixLevels(MixLevels levels);
	MixLevels GetMixLevels() const noexcept { return m_nMixLevels; }
	const CSoundFilePlayConfig &GetPlayConfig() const noexcept { return m_PlayConfig; }

	constexpr INSTRUMENTINDEX GetNumInstruments() const noexcept { return m_nInstruments; }
	constexpr SAMPLEINDEX GetNumSamples() const noexcept { return m_nSamples; }
	constexpr PATTERNINDEX GetCurrentPattern() const noexcept { return m_PlayState.m_nPattern; }
	constexpr ORDERINDEX GetCurrentOrder() const noexcept { return m_PlayState.m_nCurrentOrder; }
	MPT_FORCEINLINE CHANNELINDEX GetNumChannels() const noexcept { return static_cast<CHANNELINDEX>(ChnSettings.size()); }

	constexpr bool CanAddMoreSamples(SAMPLEINDEX amount = 1) const noexcept { return (amount < MAX_SAMPLES) && m_nSamples < (MAX_SAMPLES - amount); }
	constexpr bool CanAddMoreInstruments(INSTRUMENTINDEX amount = 1) const noexcept { return (amount < MAX_INSTRUMENTS) && m_nInstruments < (MAX_INSTRUMENTS - amount); }

#ifndef NO_PLUGINS
	IMixPlugin* GetInstrumentPlugin(INSTRUMENTINDEX instr) const noexcept;
#endif
	const CModSpecifications& GetModSpecifications() const {return *m_pModSpecs;}
	static const CModSpecifications& GetModSpecifications(const MODTYPE type);

	static ChannelFlags GetChannelMuteFlag();

#ifdef MODPLUG_TRACKER
	void PatternTranstionChnSolo(const CHANNELINDEX first, const CHANNELINDEX last);
	void PatternTransitionChnUnmuteAll();

protected:
	void HandleRowTransitionEvents(bool nextPattern);

	const ModSample *m_metronomeMeasure = nullptr;
	const ModSample *m_metronomeBeat = nullptr;
	ModChannel m_metronomeChn{};

public:
	void SetMetronomeSamples(const ModSample *measure, const ModSample *beat)
	{
		m_metronomeMeasure = measure;
		m_metronomeBeat = beat;
		m_metronomeChn.pModSample = nullptr;
		m_metronomeChn.pCurrentSample = nullptr;
	}
	constexpr bool IsMetronomeEnabled() const noexcept { return m_metronomeMeasure || m_metronomeBeat; }
#endif  // MODPLUG_TRACKER

public:
	double GetCurrentBPM() const;
	void DontLoopPattern(PATTERNINDEX nPat, ROWINDEX nRow = 0);
	CHANNELINDEX GetMixStat() const { return m_nMixStat; }
	void ResetMixStat() { m_nMixStat = 0; }
	void ResetPlayPos();
	void SetCurrentOrder(ORDERINDEX nOrder);
	std::string GetTitle() const { return m_songName; }
	bool SetTitle(const std::string &newTitle); // Return true if title was changed.
	const char *GetSampleName(SAMPLEINDEX nSample) const;
	const char *GetInstrumentName(INSTRUMENTINDEX nInstr) const;
	uint32 GetMusicSpeed() const { return m_PlayState.m_nMusicSpeed; }
	TEMPO GetMusicTempo() const { return m_PlayState.m_nMusicTempo; }
	constexpr bool IsFirstTick() const noexcept { return (m_PlayState.m_lTotalSampleCount == 0); }

	// Get song duration in various cases: total length, length to specific order & row, etc.
	std::vector<GetLengthType> GetLength(enmGetLengthResetMode adjustMode, GetLengthTarget target = GetLengthTarget());

public:
	void RecalculateSamplesPerTick();
	double GetRowDuration(TEMPO tempo, uint32 speed) const;
	uint32 GetTickDuration(PlayState &playState) const;

	// A repeat count value of -1 means infinite loop
	void SetRepeatCount(int n) { m_nRepeatCount = n; }
	int GetRepeatCount() const { return m_nRepeatCount; }
	bool IsPaused() const { return m_PlayState.m_flags[SONG_PAUSED | SONG_STEP]; }	// Added SONG_STEP as it seems to be desirable in most cases to check for this as well.
	void LoopPattern(PATTERNINDEX nPat, ROWINDEX nRow = 0);

	void InitAmigaResampler();

	void InitOPL();
	static constexpr bool SupportsOPL(MODTYPE type) noexcept { return type & (MOD_TYPE_S3M | MOD_TYPE_MPT); }
	bool SupportsOPL() const noexcept { return SupportsOPL(m_nType); }

#if !defined(MPT_WITH_ANCIENT)
	static ProbeResult ProbeFileHeaderMMCMP(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPP20(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderXPK(MemoryFileReader file, const uint64 *pfilesize);
#endif // !MPT_WITH_ANCIENT
	static ProbeResult ProbeFileHeaderUMX(MemoryFileReader file, const uint64* pfilesize);

	static ProbeResult ProbeFileHeader667(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeader669(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderAM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderAMF_Asylum(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderAMF_DSMI(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderAMS(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderAMS2(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderC67(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderCBA(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDBM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDTM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDIGI(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDMF(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDSM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDSm(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderDSym(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderETX(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderFAR(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderFC(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderFMT(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderFTM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderGDM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderGMC(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderGT2(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderGTK(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderICE(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderIMF(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderIMS(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderIT(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderITP(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderJ2B(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderKRIS(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMUS_KM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderSTK(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMDL(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMED(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMO3(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMOD(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMT2(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderMTM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderOKT(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPLM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPSM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPSM16(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPT36(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPTM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderPuma(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderRTM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderS3M(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderSFX(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderSTM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderSTP(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderSTX(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderSymMOD(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderTCB(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderUNIC(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderULT(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderXM(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderXMF(MemoryFileReader file, const uint64 *pfilesize);

	static ProbeResult ProbeFileHeaderMID(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderUAX(MemoryFileReader file, const uint64 *pfilesize);
	static ProbeResult ProbeFileHeaderWAV(MemoryFileReader file, const uint64 *pfilesize);

	// Module Loaders
	bool Read667(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool Read669(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMF_Asylum(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMF_DSMI(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMS(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadAMS2(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadC67(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadCBA(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDBM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDIGI(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDMF(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDSM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDSm(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadDSym(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadETX(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadFAR(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadFC(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadFMT(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadFTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadGDM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadGMC(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadGT2(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadGTK(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadICE(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadIMF(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadIMS(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadIT(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadITP(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadJ2B(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadKRIS(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMUS_KM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSTK(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMDL(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMED(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMO3(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMOD(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMT2(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadMTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadOKT(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPLM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPSM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPSM16(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPT36(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadPuma(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadRTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadS3M(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSFX(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSTM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSTP(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSTX(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadSymMOD(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadTCB(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadUNIC(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadULT(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadXM(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadXMF(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);

	bool ReadMID(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadUAX(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);
	bool ReadWAV(FileReader &file, ModLoadingFlags loadFlags = loadCompleteModule);

	static std::vector<const char *> GetSupportedExtensions(bool otherFormats);
	static bool IsExtensionSupported(std::string_view ext); // UTF8, casing of ext is ignored
	static mpt::ustring ModContainerTypeToString(ModContainerType containertype);
	static mpt::ustring ModContainerTypeToTracker(ModContainerType containertype);

	// Repair non-standard stuff in modules saved with previous ModPlug versions
	void UpgradeModule();

	// Save Functions
#ifndef MODPLUG_NO_FILESAVE
	bool SaveXM(std::ostream &f, bool compatibilityExport = false);
	bool SaveS3M(std::ostream &f) const;
	bool SaveMod(std::ostream &f) const;
	bool SaveIT(std::ostream &f, const mpt::PathString &filename, bool compatibilityExport = false);
	uint32 SaveMixPlugins(std::ostream *file=nullptr, bool bUpdate=true);
	void SaveExtendedInstrumentProperties(INSTRUMENTINDEX instr, MODTYPE forceType, std::ostream &f) const;
	static void SaveExtendedInstrumentProperties(mpt::span<const ModInstrument * const> instruments, MODTYPE forceType, std::ostream &f, bool allInstruments);
	void SaveExtendedSongProperties(std::ostream &f) const;
#endif // MODPLUG_NO_FILESAVE
	static void ReadExtendedInstrumentProperty(mpt::span<ModInstrument *> instruments, const uint32 code, FileReader &file);
	bool LoadExtendedSongProperties(FileReader &file, bool ignoreChannelCount, bool* pInterpretMptMade = nullptr);
	void LoadMPTMProperties(FileReader &file, uint16 cwtv);

	static mpt::ustring GetImpulseTrackerVersion(uint16 cwtv, uint16 cmwt);
	static mpt::ustring GetSchismTrackerVersion(uint16 cwtv, uint32 reserved);

	// Reads extended instrument properties(XM/IT/MPTM/ITI/XI).
	// Returns true if extended instrument properties were found.
	bool LoadExtendedInstrumentProperties(FileReader &file) { return LoadExtendedInstrumentProperties(mpt::as_span(Instruments).subspan(1, GetNumInstruments()), file); }
	static bool LoadExtendedInstrumentProperties(mpt::span<ModInstrument *> instruments, FileReader &file);

	void SetDefaultPlaybackBehaviour(MODTYPE type);
	static PlayBehaviourSet GetSupportedPlaybackBehaviour(MODTYPE type);
	static PlayBehaviourSet GetDefaultPlaybackBehaviour(MODTYPE type);

	// MOD Convert function
	MODTYPE GetBestSaveFormat() const;
	static void ConvertModCommand(ModCommand &m, const uint8 command, const uint8 param);
	static void S3MConvert(ModCommand &m, const uint8 command, const uint8 param, const bool fromIT);
	void S3MSaveConvert(const ModCommand &source, uint8 &command, uint8 &param, const bool toIT, const bool compatibilityExport = false) const;
	void ModSaveCommand(const ModCommand &source, uint8 &command, uint8 &param, const bool toXM, const bool compatibilityExport = false) const;
	// Reads 4 bytes formatted like SoundTracker/NoiseTracker/ProTracker pattern data, converts the period to a note, fills the instrument number and returns the effect command and parameter bytes.
	static std::pair<uint8, uint8> ReadMODPatternEntry(FileReader &file, ModCommand &m);
	// Converts 4 bytes formatted like SoundTracker/NoiseTracker/ProTracker pattern data by converting the period to a note and filling the instrument number, and returns the effect command and parameter bytes.
	static std::pair<uint8, uint8> ReadMODPatternEntry(const std::array<uint8, 4> data, ModCommand &m);

	void SetupMODPanning(bool forceSetup = false);  // Setup LRRL panning

public:
	// Real-time sound functions
	void SuspendPlugins();
	void ResumePlugins();
	void UpdatePluginPositions();
	void StopAllVsti();
	void RecalculateGainForAllPlugs();
	void ResetChannels();
	samplecount_t Read(samplecount_t count, IAudioTarget &target) { AudioSourceNone source; return Read(count, target, source); }
	samplecount_t Read(
		samplecount_t count,
		IAudioTarget &target,
		IAudioSource &source,
		std::optional<std::reference_wrapper<IMonitorOutput>> outputMonitor = std::nullopt,
		std::optional<std::reference_wrapper<IMonitorInput>> inputMonitor = std::nullopt
		);
	samplecount_t ReadOneTick();
private:
	void CreateStereoMix(int count);
	bool MixChannel(int count, ModChannel &chn, CHANNELINDEX channel, bool doMix);
	std::pair<mixsample_t *, mixsample_t *> GetChannelOffsets(const ModChannel &chn, CHANNELINDEX channel);
public:
	bool FadeSong(uint32 msec);
private:
	void ProcessDSP(uint32 countChunk);
	void ProcessPlugins(uint32 nCount);
	void ProcessInputChannels(IAudioSource &source, std::size_t countChunk);
public:
	samplecount_t GetTotalSampleCount() const { return m_PlayState.m_lTotalSampleCount; }
	bool HasPositionChanged() { bool b = m_PlayState.m_flags[SONG_POSITIONCHANGED]; m_PlayState.m_flags.reset(SONG_POSITIONCHANGED); return b; }
	bool IsRenderingToDisc() const { return m_bIsRendering; }

	void PrecomputeSampleLoops(bool updateChannels = false);
	void UpdateInstrumentFilter(const ModInstrument &ins, bool updateMode, bool updateCutoff, bool updateResonance);

public:
	// Mixer Config
	void SetMixerSettings(const MixerSettings &mixersettings);
	void SetResamplerSettings(const CResamplerSettings &resamplersettings);
	void InitPlayer(bool bReset=false);
	void SetDspEffects(uint32 DSPMask);
	uint32 GetSampleRate() const { return m_MixerSettings.gdwMixingFreq; }
#ifndef NO_EQ
	void SetEQGains(const uint32 *pGains, const uint32 *pFreqs, bool bReset = false) { m_EQ.SetEQGains(pGains, pFreqs, bReset, m_MixerSettings.gdwMixingFreq); } // 0=-12dB, 32=+12dB
#endif // NO_EQ
public:
	bool ReadNote();
	bool ProcessRow();
	bool ProcessEffects();
	std::pair<bool, bool> NextRow(PlayState &playState, const bool breakRow) const;
	void SetupNextRow(PlayState &playState, const bool patternLoop) const;
	CHANNELINDEX GetNNAChannel(CHANNELINDEX nChn) const;
	CHANNELINDEX CheckNNA(CHANNELINDEX nChn, uint32 instr, int note, bool forceCut);
	void StopOldNNA(ModChannel &chn, CHANNELINDEX channel);
	void NoteChange(ModChannel &chn, int note, bool bPorta = false, bool bResetEnv = true, bool bManual = false, CHANNELINDEX channelHint = CHANNELINDEX_INVALID) const;
	void InstrumentChange(ModChannel &chn, uint32 instr, bool bPorta = false, bool bUpdVol = true, bool bResetEnv = true) const;
	void ApplyInstrumentPanning(ModChannel &chn, const ModInstrument *instr, const ModSample *smp) const;
	uint32 CalculateXParam(PATTERNINDEX pat, ROWINDEX row, CHANNELINDEX chn, uint32 *extendedRows = nullptr) const;

	// Channel Effects
	void KeyOff(ModChannel &chn) const;
	// Global Effects
	void SetTempo(TEMPO param, bool setAsNonModcommand = false) { SetTempo(m_PlayState, param, setAsNonModcommand); }
	void SetTempo(PlayState &playState, TEMPO param, bool setAsNonModcommand = false) const;
	void SetSpeed(PlayState &playState, uint32 param) const;
	static TEMPO ConvertST2Tempo(uint8 tempo);

	void ProcessRamping(ModChannel &chn) const;

	void ProcessFinetune(PATTERNINDEX pattern, ROWINDEX row, CHANNELINDEX channel, bool isSmooth);

protected:
	// Global variable initializer for loader functions
	void SetType(MODTYPE type);
	void InitializeGlobals(MODTYPE type, CHANNELINDEX numChannels);

	// Channel effect processing
	int GetVibratoDelta(int type, int position) const;

	void ProcessVolumeSwing(ModChannel &chn, int &vol) const;
	void ProcessPanningSwing(ModChannel &chn) const;
	void ProcessTremolo(ModChannel &chn, int &vol) const;
	void ProcessTremor(CHANNELINDEX nChn, int &vol);

	bool IsEnvelopeProcessed(const ModChannel &chn, EnvelopeType env) const;
	void ProcessVolumeEnvelope(ModChannel &chn, int &vol) const;
	void ProcessPanningEnvelope(ModChannel &chn) const;
	int ProcessPitchFilterEnvelope(ModChannel &chn, int32 &period) const;

	void IncrementEnvelopePosition(ModChannel &chn, EnvelopeType envType) const;
	void IncrementEnvelopePositions(ModChannel &chn) const;

	void ProcessInstrumentFade(ModChannel &chn, int &vol) const;

	static void ProcessPitchPanSeparation(int32 &pan, int note, const ModInstrument &instr);
	void ProcessPanbrello(ModChannel &chn) const;

	void ProcessArpeggio(CHANNELINDEX nChn, int32 &period, Tuning::NOTEINDEXTYPE &arpeggioSteps);
	void ProcessVibrato(CHANNELINDEX nChn, int32 &period, Tuning::RATIOTYPE &vibratoFactor);
	void ProcessSampleAutoVibrato(ModChannel &chn, int32 &period, Tuning::RATIOTYPE &vibratoFactor, int &nPeriodFrac) const;

	std::pair<SamplePosition, uint32> GetChannelIncrement(const ModChannel &chn, uint32 period, int periodFrac) const;

protected:
	// Type of panning command
	enum PanningType
	{
		Pan4bit = 4,
		Pan6bit = 6,
		Pan8bit = 8,
	};
	// Channel Effects
	void ResetAutoSlides(ModChannel &chn) const;
	void ProcessAutoSlides(PlayState &playState, CHANNELINDEX channel);
	void UpdateS3MEffectMemory(ModChannel &chn, ModCommand::PARAM param) const;
	void PortamentoFC(ModChannel &chn) const;
	void PortamentoUp(CHANNELINDEX nChn, ModCommand::PARAM param, const bool doFinePortamentoAsRegular);
	void PortamentoUp(PlayState &playState, CHANNELINDEX nChn, ModCommand::PARAM param, const bool doFinePortamentoAsRegular) const;
	void PortamentoDown(CHANNELINDEX nChn, ModCommand::PARAM param, const bool doFinePortamentoAsRegular);
	void PortamentoDown(PlayState &playState, CHANNELINDEX nChn, ModCommand::PARAM param, const bool doFinePortamentoAsRegular) const;
	void MidiPortamento(CHANNELINDEX nChn, int param, const bool doFineSlides);
	void FinePortamentoUp(ModChannel &chn, ModCommand::PARAM param) const;
	void FinePortamentoDown(ModChannel &chn, ModCommand::PARAM param) const;
	void ExtraFinePortamentoUp(ModChannel &chn, ModCommand::PARAM param) const;
	void ExtraFinePortamentoDown(ModChannel &chn, ModCommand::PARAM param) const;
	void PortamentoMPT(ModChannel &chn, int param) const;
	void PortamentoFineMPT(PlayState &playState, CHANNELINDEX nChn, int param) const;
	void PortamentoExtraFineMPT(ModChannel &chn, int param) const;
	void SetFinetune(PATTERNINDEX pattern, ROWINDEX row, CHANNELINDEX channel, PlayState &playState, bool isSmooth) const;
	int16 CalculateFinetuneTarget(PATTERNINDEX pattern, ROWINDEX row, CHANNELINDEX channel) const;
	void NoteSlide(ModChannel &chn, uint32 param, bool slideUp, bool retrig) const;
	std::pair<uint16, bool> GetVolCmdTonePorta(const ModCommand &m, uint32 startTick) const;
	bool TonePortamentoSharesEffectMemory() const;
	void InitTonePortamento(ModChannel &chn, uint16 param) const;
	void TonePortamento(CHANNELINDEX chn, uint16 param);
	int32 TonePortamento(PlayState &playState, CHANNELINDEX nChn, uint16 param) const;
	void TonePortamentoWithDuration(ModChannel &chn, uint16 param = uint16_max) const;
	void Vibrato(ModChannel &chn, uint32 param) const;
	void FineVibrato(ModChannel &chn, uint32 param) const;
	void AutoVolumeSlide(ModChannel &chn, ModCommand::PARAM param) const;
	void VolumeDownETX(const PlayState &playState, ModChannel &chn, ModCommand::PARAM param) const;
	void VolumeSlide(ModChannel &chn, ModCommand::PARAM param, bool volCol = false) const;
	void PanningSlide(ModChannel &chn, ModCommand::PARAM param, bool memory = true) const;
	void ChannelVolSlide(ModChannel &chn, ModCommand::PARAM param) const;
	void ChannelVolumeDownWithDuration(ModChannel &chn, uint16 param = uint16_max) const;
	void FineVolumeUp(ModChannel &chn, ModCommand::PARAM param, bool volCol) const;
	void FineVolumeDown(ModChannel &chn, ModCommand::PARAM param, bool volCol) const;
	void Tremolo(ModChannel &chn, uint32 param) const;
	void Panbrello(ModChannel &chn, uint32 param) const;
	void Panning(ModChannel &chn, uint32 param, PanningType panBits) const;
	void RetrigNote(CHANNELINDEX nChn, int param, int offset = 0);
	void ProcessSampleOffset(ModChannel &chn, CHANNELINDEX nChn, const PlayState &playState) const;
	void SampleOffset(ModChannel &chn, SmpLength param) const;
	void ReverseSampleOffset(ModChannel &chn, ModCommand::PARAM param) const;
	void DigiBoosterSampleReverse(ModChannel &chn, ModCommand::PARAM param) const;
	void HandleDigiSamplePlayDirection(PlayState &state, CHANNELINDEX chn) const;
	void NoteCut(CHANNELINDEX nChn, uint32 nTick, bool cutSample);
	void PatternLoop(PlayState &state, CHANNELINDEX nChn, ModCommand::PARAM param) const;
	bool HandleNextRow(PlayState &state, const ModSequence &order, bool honorPatternLoop) const;
	void ExtendedMODCommands(CHANNELINDEX nChn, ModCommand::PARAM param);
	void ExtendedS3MCommands(CHANNELINDEX nChn, ModCommand::PARAM param);
	void ExtendedChannelEffect(ModChannel &chn, uint32 param, PlayState &playState) const;
	void InvertLoop(ModChannel &chn);
	void PositionJump(PlayState &state, CHANNELINDEX chn) const;
	ROWINDEX PatternBreak(PlayState &state, CHANNELINDEX chn, uint8 param) const;
	void GlobalVolSlide(PlayState &playState, ModCommand::PARAM param, CHANNELINDEX chn) const;

	void ProcessMacroOnChannel(CHANNELINDEX nChn);
	void ProcessMIDIMacro(PlayState &playState, CHANNELINDEX nChn, bool isSmooth, const MIDIMacroConfigData::Macro &macro, uint8 param = 0, PLUGINDEX plugin = 0);
	void SendMIDIData(PlayState &playState, CHANNELINDEX nChn, bool isSmooth, const mpt::span<const uint8> macro, PLUGINDEX plugin);
	void SendMIDINote(CHANNELINDEX chn, uint16 note, uint16 volume, IMixPlugin *plugin = nullptr);

	int SetupChannelFilter(ModChannel &chn, bool bReset, int envModifier = 256) const;
	int HandleNoteChangeFilter(ModChannel &chn) const;

public:
	static float CalculateSmoothParamChange(const PlayState &playState, float currentValue, float param);

	void DoFreqSlide(ModChannel &chn, int32 &period, int32 amount, bool isTonePorta = false) const;

	// Convert frequency to IT cutoff (0...127)
	uint8 FrequencyToCutOff(double frequency) const;
	// Convert IT cutoff (0...127 + modifier) to frequency
	float CutOffToFrequency(uint32 nCutOff, int envModifier = 256) const;

	// Returns true if periods are actually plain frequency values in Hz.
	bool PeriodsAreFrequencies() const noexcept
	{
		return m_playBehaviour[kPeriodsAreHertz] && !UseFinetuneAndTranspose();
	}
	
	// Returns true if the format uses transpose+finetune rather than frequency in Hz to specify middle-C.
	static constexpr bool UseFinetuneAndTranspose(MODTYPE type) noexcept
	{
		return (type & (MOD_TYPE_AMF0 | MOD_TYPE_DIGI | MOD_TYPE_MED | MOD_TYPE_MOD | MOD_TYPE_MTM | MOD_TYPE_OKT | MOD_TYPE_SFX | MOD_TYPE_STP | MOD_TYPE_XM));
	}
	bool UseFinetuneAndTranspose() const noexcept
	{
		return UseFinetuneAndTranspose(GetType());
	}

	// Returns true if the format uses combined commands for fine and regular portamento slides
	static constexpr bool UseCombinedPortamentoCommands(MODTYPE type) noexcept
	{
		return !(type & (MOD_TYPE_MOD | MOD_TYPE_XM | MOD_TYPE_MT2 | MOD_TYPE_MED | MOD_TYPE_AMF0 | MOD_TYPE_DIGI | MOD_TYPE_STP | MOD_TYPE_DTM));
	}
	bool UseCombinedPortamentoCommands() const noexcept
	{
		return UseCombinedPortamentoCommands(GetType());
	}

	uint32 GlobalVolumeRange() const noexcept
	{
		return !(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_IMF | MOD_TYPE_J2B | MOD_TYPE_MID | MOD_TYPE_AMS | MOD_TYPE_DBM | MOD_TYPE_PTM | MOD_TYPE_MDL | MOD_TYPE_DTM)) ? 64 : 128;
	}

	bool DestroySample(SAMPLEINDEX nSample);
	bool DestroySampleThreadsafe(SAMPLEINDEX nSample);

	// Find an unused sample slot. If it is going to be assigned to an instrument, targetInstrument should be specified.
	// SAMPLEINDEX_INVLAID is returned if no free sample slot could be found.
	SAMPLEINDEX GetNextFreeSample(INSTRUMENTINDEX targetInstrument = INSTRUMENTINDEX_INVALID, SAMPLEINDEX start = 1) const;
	// Find an unused instrument slot.
	// INSTRUMENTINDEX_INVALID is returned if no free instrument slot could be found.
	INSTRUMENTINDEX GetNextFreeInstrument(INSTRUMENTINDEX start = 1) const;
	// Check whether a given sample is used by a given instrument.
	bool IsSampleReferencedByInstrument(SAMPLEINDEX sample, INSTRUMENTINDEX instr) const;

	ModInstrument *AllocateInstrument(INSTRUMENTINDEX instr, SAMPLEINDEX assignedSample = 0);
	bool DestroyInstrument(INSTRUMENTINDEX nInstr, deleteInstrumentSamples removeSamples);
	bool RemoveInstrumentSamples(INSTRUMENTINDEX nInstr, SAMPLEINDEX keepSample = SAMPLEINDEX_INVALID);
	SAMPLEINDEX DetectUnusedSamples(std::vector<bool> &sampleUsed) const;
	SAMPLEINDEX RemoveSelectedSamples(const std::vector<bool> &keepSamples);

	// Set the autovibrato settings for all samples associated to the given instrument.
	void PropagateXMAutoVibrato(INSTRUMENTINDEX ins, VibratoType type, uint8 sweep, uint8 depth, uint8 rate);

	// Samples file I/O
	bool ReadSampleFromFile(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize = false, bool includeInstrumentFormats = true);
	bool ReadWAVSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize = false, FileReader *wsmpChunk = nullptr);
protected:
	bool ReadW64Sample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize = false);
	bool ReadPATSample(SAMPLEINDEX nSample, FileReader &file);
	bool ReadS3ISample(SAMPLEINDEX nSample, FileReader &file);
	bool ReadSBISample(SAMPLEINDEX sample, FileReader &file);
	bool ReadCAFSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize = false);
	bool ReadAIFFSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize = false);
	bool ReadAUSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize = false);
	bool ReadXISample(SAMPLEINDEX nSample, FileReader &file);
	bool ReadITSSample(SAMPLEINDEX nSample, FileReader &file, bool rewind = true);
	bool ReadITISample(SAMPLEINDEX nSample, FileReader &file);
	bool ReadIFFSample(SAMPLEINDEX sample, FileReader &file, bool allowLittleEndian = true, uint8 octave = uint8_max);
	bool ReadBRRSample(SAMPLEINDEX sample, FileReader &file);
	bool ReadFLACSample(SAMPLEINDEX sample, FileReader &file);
	bool ReadOpusSample(SAMPLEINDEX sample, FileReader &file);
	bool ReadVorbisSample(SAMPLEINDEX sample, FileReader &file);
	bool ReadMP3Sample(SAMPLEINDEX sample, FileReader &file, bool raw = false, bool mo3Decode = false);  //  raw: ignore all encoder-/decodr-delays, decode just raw frames  ;  mod3Decode: skip metadata and loop-precompute
	bool ReadMediaFoundationSample(SAMPLEINDEX sample, FileReader &file, bool mo3Decode = false);  //  mod3Decode: skip metadata and loop-precompute
public:
#ifdef MODPLUG_TRACKER
	static std::vector<FileType> GetMediaFoundationFileTypes();
#endif // MODPLUG_TRACKER
#ifndef MODPLUG_NO_FILESAVE
	bool SaveWAVSample(SAMPLEINDEX nSample, std::ostream &f) const;
	bool SaveRAWSample(SAMPLEINDEX nSample, std::ostream &f) const;
	bool SaveFLACSample(SAMPLEINDEX nSample, std::ostream &f) const;
	bool SaveS3ISample(SAMPLEINDEX smp, std::ostream &f) const;
	bool SaveIFFSample(SAMPLEINDEX smp, std::ostream &f) const;
#endif

	// Instrument file I/O
	bool ReadInstrumentFromFile(INSTRUMENTINDEX nInstr, FileReader &file, bool mayNormalize = false);
	bool ReadSampleAsInstrument(INSTRUMENTINDEX nInstr, FileReader &file, bool mayNormalize = false);
protected:
	bool ReadXIInstrument(INSTRUMENTINDEX nInstr, FileReader &file);
	bool ReadITIInstrument(INSTRUMENTINDEX nInstr, FileReader &file);
	bool ReadPATInstrument(INSTRUMENTINDEX nInstr, FileReader &file);
	bool ReadSFZInstrument(INSTRUMENTINDEX nInstr, FileReader &file);
public:
#ifndef MODPLUG_NO_FILESAVE
	bool SaveXIInstrument(INSTRUMENTINDEX nInstr, std::ostream &f) const;
	bool SaveITIInstrument(INSTRUMENTINDEX nInstr, std::ostream &f, const mpt::PathString &filename, bool compress, bool allowExternal) const;
	bool SaveSFZInstrument(INSTRUMENTINDEX nInstr, std::ostream &f, const mpt::PathString &filename, bool useFLACsamples) const;
#endif

	// I/O from another sound file
	bool ReadInstrumentFromSong(INSTRUMENTINDEX targetInstr, const CSoundFile &srcSong, INSTRUMENTINDEX sourceInstr);
	bool ReadSampleFromSong(SAMPLEINDEX targetSample, const CSoundFile &srcSong, SAMPLEINDEX sourceSample);

	// Period/Note functions
	uint32 GetNoteFromPeriod(uint32 period, int32 nFineTune = 0, uint32 nC5Speed = 0) const;
	uint32 GetPeriodFromNote(uint32 note, int32 nFineTune, uint32 nC5Speed) const;
	uint32 GetFreqFromPeriod(uint32 period, uint32 c5speed, int32 nPeriodFrac = 0) const;
	// Misc functions
	ModSample &GetSample(SAMPLEINDEX sample) { MPT_ASSERT(sample <= m_nSamples && sample < std::size(Samples)); return Samples[sample]; }
	const ModSample &GetSample(SAMPLEINDEX sample) const { MPT_ASSERT(sample <= m_nSamples && sample < std::size(Samples)); return Samples[sample]; }

	// Resolve note/instrument combination to real sample index. Return value is guaranteed to be in [0, GetNumSamples()].
	SAMPLEINDEX GetSampleIndex(ModCommand::NOTE note, uint32 instr) const noexcept;

	uint32 MapMidiInstrument(uint8 program, uint16 bank, uint8 midiChannel, uint8 note, bool isXG, std::bitset<32> drumChns);
	size_t ITInstrToMPT(FileReader &file, ModInstrument &ins, uint16 trkvers);
	std::pair<bool, bool> LoadMixPlugins(FileReader &file, bool ignoreChannelCount = true);
#ifndef NO_PLUGINS
	static void ReadMixPluginChunk(FileReader &file, SNDMIXPLUGIN &plugin);
	void ProcessMidiOut(CHANNELINDEX nChn);
#endif // NO_PLUGINS

	void ProcessGlobalVolume(samplecount_t countChunk);
	void ProcessStereoSeparation(samplecount_t countChunk);

private:
	PLUGINDEX GetChannelPlugin(const ModChannel &channel, CHANNELINDEX nChn, PluginMutePriority respectMutes) const;
	static PLUGINDEX GetActiveInstrumentPlugin(const ModChannel &chn, PluginMutePriority respectMutes);
	IMixPlugin *GetChannelInstrumentPlugin(const ModChannel &chn) const;

public:
	PLUGINDEX GetBestPlugin(const ModChannel &channel, CHANNELINDEX nChn, PluginPriority priority, PluginMutePriority respectMutes) const;

#if defined(MPT_ENABLE_PLAYBACK_TRACE)
	PlaybackTest CreatePlaybackTest(PlaybackTestSettings settings);
#endif // MPT_ENABLE_PLAYBACK_TRACE

};


#ifndef NO_PLUGINS
inline IMixPlugin* CSoundFile::GetInstrumentPlugin(INSTRUMENTINDEX instr) const noexcept
{
	if(instr > 0 && instr <= GetNumInstruments() && Instruments[instr] && Instruments[instr]->nMixPlug && Instruments[instr]->nMixPlug <= MAX_MIXPLUGINS)
		return m_MixPlugins[Instruments[instr]->nMixPlug - 1].pMixPlugin;
	else
		return nullptr;
}
#endif // NO_PLUGINS


#define FADESONGDELAY		100

MPT_CONSTEXPRINLINE int8 MOD2XMFineTune(int v) { return static_cast<int8>(static_cast<uint8>(v) << 4); }
MPT_CONSTEXPRINLINE int8 XM2MODFineTune(int v) { return static_cast<int8>(static_cast<uint8>(v) >> 4); }


OPENMPT_NAMESPACE_END
