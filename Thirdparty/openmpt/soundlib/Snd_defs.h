/*
 * Snd_defs.h
 * ----------
 * Purpose: Basic definitions of data types, enums, etc. for the playback engine core.
 * Notes  : (currently none)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "openmpt/base/FlagSet.hpp"


OPENMPT_NAMESPACE_BEGIN


using ROWINDEX = uint32;
inline constexpr ROWINDEX ROWINDEX_INVALID = uint32_max;
using CHANNELINDEX = uint16;
inline constexpr CHANNELINDEX CHANNELINDEX_INVALID = uint16_max;
using ORDERINDEX = uint16;
inline constexpr ORDERINDEX ORDERINDEX_INVALID = uint16_max;
inline constexpr ORDERINDEX ORDERINDEX_MAX = uint16_max - 1;
using PATTERNINDEX = uint16;
inline constexpr PATTERNINDEX PATTERNINDEX_INVALID = uint16_max;   // "---" in order list
inline constexpr PATTERNINDEX PATTERNINDEX_SKIP = uint16_max - 1;  // "+++" in order list
using PLUGINDEX = uint8;
inline constexpr PLUGINDEX PLUGINDEX_INVALID = uint8_max;
using SAMPLEINDEX = uint16;
inline constexpr SAMPLEINDEX SAMPLEINDEX_INVALID = uint16_max;
using INSTRUMENTINDEX = uint16;
inline constexpr INSTRUMENTINDEX INSTRUMENTINDEX_INVALID = uint16_max;
using SEQUENCEINDEX = uint8;
inline constexpr SEQUENCEINDEX SEQUENCEINDEX_INVALID = uint8_max;

using SmpLength = uint32;


inline constexpr SmpLength MAX_SAMPLE_LENGTH = 0x10000000; // Sample length in frames. Sample size in bytes can be more than this (= 256 MB).

inline constexpr ROWINDEX MAX_ROWS_PER_MEASURE     = 65536;
inline constexpr ROWINDEX MAX_ROWS_PER_BEAT        = 65536;
inline constexpr ROWINDEX DEFAULT_ROWS_PER_BEAT    = 4;
inline constexpr ROWINDEX DEFAULT_ROWS_PER_MEASURE = 16;

inline constexpr ROWINDEX MAX_PATTERN_ROWS       = 4096;
inline constexpr ORDERINDEX MAX_ORDERS           = ORDERINDEX_MAX + 1;
inline constexpr PATTERNINDEX MAX_PATTERNS       = 4000;
inline constexpr SAMPLEINDEX MAX_SAMPLES         = 4000;
inline constexpr INSTRUMENTINDEX MAX_INSTRUMENTS = 256;
inline constexpr PLUGINDEX MAX_MIXPLUGINS        = 250;

inline constexpr SEQUENCEINDEX MAX_SEQUENCES     = 50;

inline constexpr CHANNELINDEX MAX_BASECHANNELS   = 192; // Maximum pattern channels.
inline constexpr CHANNELINDEX MAX_CHANNELS       = 256; // Maximum number of mixing channels.

enum { FREQ_FRACBITS = 4 }; // Number of fractional bits in return value of CSoundFile::GetFreqFromPeriod()

using samplecount_t = uint32;  // Number of rendered samples

using PlugParamIndex = uint32;
using PlugParamValue = float;

// String lengths (including trailing null char)
enum
{
	MAX_SAMPLENAME         = 32,
	MAX_SAMPLEFILENAME     = 22,
	MAX_INSTRUMENTNAME     = 32,
	MAX_INSTRUMENTFILENAME = 32,
	MAX_PATTERNNAME        = 32,
	MAX_CHANNELNAME        = 20,
};

enum MODTYPE
{
	MOD_TYPE_NONE = 0x00,
	MOD_TYPE_MOD  = 0x01,
	MOD_TYPE_S3M  = 0x02,
	MOD_TYPE_XM   = 0x04,
	MOD_TYPE_MED  = 0x08,
	MOD_TYPE_MTM  = 0x10,
	MOD_TYPE_IT   = 0x20,
	MOD_TYPE_669  = 0x40,
	MOD_TYPE_ULT  = 0x80,
	MOD_TYPE_STM  = 0x100,
	MOD_TYPE_FAR  = 0x200,
	MOD_TYPE_DTM  = 0x400,
	MOD_TYPE_AMF  = 0x800,
	MOD_TYPE_AMS  = 0x1000,
	MOD_TYPE_DSM  = 0x2000,
	MOD_TYPE_MDL  = 0x4000,
	MOD_TYPE_OKT  = 0x8000,
	MOD_TYPE_MID  = 0x10000,
	MOD_TYPE_DMF  = 0x20000,
	MOD_TYPE_PTM  = 0x40000,
	MOD_TYPE_DBM  = 0x80000,
	MOD_TYPE_MT2  = 0x100000,
	MOD_TYPE_AMF0 = 0x200000,
	MOD_TYPE_PSM  = 0x400000,
	MOD_TYPE_J2B  = 0x800000,
	MOD_TYPE_MPT  = 0x1000000,
	MOD_TYPE_IMF  = 0x2000000,
	MOD_TYPE_DIGI = 0x4000000,
	MOD_TYPE_STP  = 0x8000000,
	MOD_TYPE_PLM  = 0x10000000,
	MOD_TYPE_SFX  = 0x20000000,

	MOD_TYPE_MOD_PC = MOD_TYPE_MOD | MOD_TYPE_XM,
};
DECLARE_FLAGSET(MODTYPE)


enum class ModContainerType
{
	None,
	UMX,
	XPK,
	PP20,
	MMCMP,
	WAV,      // WAV as module
	UAX,      // Unreal sample set as module
	Generic,  // Generic CUnarchiver container
};


enum class AutoSlideCommand
{
	TonePortamento,
	TonePortamentoWithDuration,
	PortamentoUp,
	PortamentoDown,
	FinePortamentoUp,
	FinePortamentoDown,
	PortamentoFC,
	FineVolumeSlideUp,
	FineVolumeSlideDown,
	VolumeDownETX,
	VolumeSlideSTK,
	VolumeDownWithDuration,
	GlobalVolumeSlide,
	Vibrato,
	Tremolo,
	NumCommands
};


// Module channel / sample flags
enum ChannelFlags : uint32
{
	// Sample Flags
	CHN_16BIT           = 0x01,        // 16-bit sample
	CHN_LOOP            = 0x02,        // Looped sample
	CHN_PINGPONGLOOP    = 0x04,        // Bidi-looped sample
	CHN_SUSTAINLOOP     = 0x08,        // Sample with sustain loop
	CHN_PINGPONGSUSTAIN = 0x10,        // Sample with bidi sustain loop
	CHN_PANNING         = 0x20,        // Sample with forced panning
	CHN_STEREO          = 0x40,        // Stereo sample
	CHN_REVERSE         = 0x80,        // Start sample playback from sample / loop end (Velvet Studio feature)
	CHN_SURROUND        = 0x100,       // Use surround channel
	CHN_ADLIB           = 0x200,       // Adlib / OPL instrument is active on this channel

	// Channel Flags
	CHN_PINGPONGFLAG    = 0x80,        // When flag is on, sample is processed backwards - this is intentionally the same flag as CHN_REVERSE.
	CHN_MUTE            = 0x400,       // Muted channel
	CHN_KEYOFF          = 0x800,       // Exit sustain
	CHN_NOTEFADE        = 0x1000,      // Fade note (instrument mode)
	CHN_WRAPPED_LOOP    = 0x2000,      // Loop just wrapped around to loop start (required for correct interpolation around loop points)
	CHN_AMIGAFILTER     = 0x4000,      // Apply Amiga low-pass filter
	CHN_FILTER          = 0x8000,      // Apply resonant filter on sample
	CHN_VOLUMERAMP      = 0x10000,     // Apply volume ramping
	CHN_VIBRATO         = 0x20000,     // Apply vibrato
	CHN_TREMOLO         = 0x40000,     // Apply tremolo
	CHN_PORTAMENTO      = 0x80000,     // Apply portamento
	CHN_GLISSANDO       = 0x100000,    // Glissando (force portamento to semitones) mode
	CHN_FASTVOLRAMP     = 0x200000,    // Force usage of global ramping settings instead of ramping over the complete render buffer length
	CHN_EXTRALOUD       = 0x400000,    // Force sample to play at 0dB
	CHN_REVERB          = 0x800000,    // Apply reverb on this channel
	CHN_NOREVERB        = 0x1000000,   // Disable reverb on this channel
	CHN_NOFX            = 0x2000000,   // Dry channel (no plugins)
	CHN_SYNCMUTE        = 0x4000000,   // Keep sample sync on mute

	// Sample flags (only present in ModSample::uFlags, may overlap with CHN_CHANNELFLAGS)
	SMP_MODIFIED        = 0x2000,      // Sample data has been edited in the tracker
	SMP_KEEPONDISK      = 0x4000,      // Sample is not saved to file, data is restored from original sample file
	SMP_NODEFAULTVOLUME = 0x8000,      // Ignore default volume setting
};
DECLARE_FLAGSET(ChannelFlags)

inline constexpr ChannelFlags CHN_SAMPLEFLAGS = (CHN_16BIT | CHN_LOOP | CHN_PINGPONGLOOP | CHN_SUSTAINLOOP | CHN_PINGPONGSUSTAIN | CHN_PANNING | CHN_STEREO | CHN_PINGPONGFLAG | CHN_REVERSE | CHN_SURROUND | CHN_ADLIB).as_enum();
inline constexpr ChannelFlags CHN_CHANNELFLAGS = (~CHN_SAMPLEFLAGS | CHN_SURROUND).as_enum();

// Sample flags fit into the first 16 bits, and with the current memory layout, storing them as a 16-bit integer packs struct ModSample nicely.
using SampleFlags = FlagSet<ChannelFlags, uint16>;


// Instrument envelope-specific flags
enum EnvelopeFlags : uint8
{
	ENV_ENABLED = 0x01, // env is enabled
	ENV_LOOP    = 0x02, // env loop
	ENV_SUSTAIN = 0x04, // env sustain
	ENV_CARRY   = 0x08, // env carry
	ENV_FILTER  = 0x10, // filter env enabled (this has to be combined with ENV_ENABLED in the pitch envelope's flags)
};
DECLARE_FLAGSET(EnvelopeFlags)


// Envelope value boundaries
inline constexpr uint8 ENVELOPE_MIN = 0;     // Vertical min value of a point
inline constexpr uint8 ENVELOPE_MID = 32;    // Vertical middle line
inline constexpr uint8 ENVELOPE_MAX = 64;    // Vertical max value of a point
inline constexpr uint8 MAX_ENVPOINTS = 240;  // Maximum length of each instrument envelope


// Instrument-specific flags
enum InstrumentFlags : uint8
{
	INS_SETPANNING = 0x01, // Panning enabled
	INS_MUTE       = 0x02, // Instrument is muted
};
DECLARE_FLAGSET(InstrumentFlags)


// envelope types in instrument editor
enum EnvelopeType : uint8
{
	ENV_VOLUME = 0,
	ENV_PANNING,
	ENV_PITCH,

	ENV_MAXTYPES
};

// Filter Modes
enum class FilterMode : uint8
{
	Unchanged = 0xFF,
	LowPass   = 0,
	HighPass  = 1,
};


// NNA types (New Note Action)
enum class NewNoteAction : uint8
{
	NoteCut  = 0,
	Continue = 1,
	NoteOff  = 2,
	NoteFade = 3,
};

// DCT types (Duplicate Check Types)
enum class DuplicateCheckType : uint8
{
	None       = 0,
	Note       = 1,
	Sample     = 2,
	Instrument = 3,
	Plugin     = 4,
};

// DNA types (Duplicate Note Action)
enum class DuplicateNoteAction : uint8
{
	NoteCut  = 0,
	NoteOff  = 1,
	NoteFade = 2,
};


enum PlayFlags : uint16
{
	SONG_PATTERNLOOP     =  0x01,  // Loop current pattern (pattern editor)
	SONG_STEP            =  0x02,  // Song is in "step" mode (pattern editor)
	SONG_PAUSED          =  0x04,  // Song is paused (no tick processing, just rendering audio)
	SONG_FADINGSONG      =  0x08,  // Song is fading out
	SONG_ENDREACHED      =  0x10,  // Song is finished
	SONG_FIRSTTICK       =  0x20,  // Is set when the current tick is the first tick of the row
	SONG_MPTFILTERMODE   =  0x40,  // Local filter mode (reset filter on each note)
	SONG_SURROUNDPAN     =  0x80,  // Pan in the rear channels
	SONG_POSJUMP         = 0x100,  // Position jump encountered
	SONG_BREAKTOROW      = 0x200,  // Break to row command encountered
	SONG_POSITIONCHANGED = 0x400,  // Report to plugins that we jumped around in the module
};
DECLARE_FLAGSET(PlayFlags)


enum SongFlags
{
	SONG_FASTPORTAS          =     0x01,  // Portamentos are executed on every tick
	SONG_FASTVOLSLIDES       =     0x02,  // Old Scream Tracker 3.0 volume slides (executed on every tick)
	SONG_ITOLDEFFECTS        =     0x04,  // Old Impulse Tracker effect implementations
	SONG_ITCOMPATGXX         =     0x08,  // IT "Compatible Gxx" (IT's flag to behave more like other trackers w/r/t portamento effects)
	SONG_LINEARSLIDES        =     0x10,  // Linear slides vs. Amiga slides
	SONG_EXFILTERRANGE       =     0x20,  // Cutoff Filter has double frequency range (up to ~10Khz)
	SONG_AMIGALIMITS         =     0x40,  // Enforce amiga frequency limits
	SONG_S3MOLDVIBRATO       =     0x80,  // ScreamTracker 2 vibrato in S3M files
	SONG_PT_MODE             =    0x100,  // ProTracker 1/2 playback mode
	SONG_ISAMIGA             =    0x200,  // Is an Amiga module and thus qualifies to be played using the Paula BLEP resampler
	SONG_IMPORTED            =    0x400,  // Song type does not represent actual module format / was imported from a different format (OpenMPT)
	SONG_PLAYALLSONGS        =    0x800,  // Play all subsongs consecutively (libopenmpt)
	SONG_AUTO_TONEPORTA      =   0x1000,  // Tone portamento command is continued automatically
	SONG_AUTO_TONEPORTA_CONT =   0x2000,  // Auto tone portamento is not interruped by a tone portamento with parameter 0
	SONG_AUTO_GLOBALVOL      =   0x4000,  // Global volume slide command is continued automatically
	SONG_AUTO_VIBRATO        =   0x8000,  // Vibrato command is continued automatically
	SONG_AUTO_TREMOLO        = 0x1'8000,  // Tremolo command is continued automatically
	SONG_AUTO_VOLSLIDE_STK   = 0x2'0000,  // Automatic volume slide command is interpreted like in STK files (rather than like in STP files)
	SONG_FORMAT_NO_VOLCOL    = 0x4'0000,  // The original (imported) format has no volume column, so it can be hidden in the pattern editor.
};
DECLARE_FLAGSET(SongFlags)


// Global Options (Renderer)
#ifndef NO_AGC
#define SNDDSP_AGC            0x40     // Automatic gain control
#endif // ~NO_AGC
#ifndef NO_DSP
#define SNDDSP_MEGABASS       0x02     // Bass expansion
#define SNDDSP_SURROUND       0x08     // Surround mix
#define SNDDSP_BITCRUSH       0x01
#endif // NO_DSP
#ifndef NO_REVERB
#define SNDDSP_REVERB         0x20     // Apply reverb
#endif // NO_REVERB
#ifndef NO_EQ
#define SNDDSP_EQ             0x80     // Apply EQ
#endif // NO_EQ

#define SNDMIX_SOFTPANNING    0x10     // Soft panning mode (this is forced with mixmode RC3 and later)

// Misc Flags (can safely be turned on or off)
#define SNDMIX_MAXDEFAULTPAN  0x80000  // Currently unused (should be used by Amiga MOD loaders)
#define SNDMIX_MUTECHNMODE    0x100000 // Notes are not played on muted channels


inline constexpr uint32 MAX_GLOBAL_VOLUME = 256;
inline constexpr uint32 MAX_PREAMP = 2000;

// When to execute a position override event
enum class OrderTransitionMode : uint8
{
	AtPatternEnd,
	AtMeasureEnd,
	AtBeatEnd,
	AtRowEnd,
};


// Resampling modes
enum ResamplingMode : uint8
{
	// ATTENTION: Do not change ANY of these values, as they get written out to files in per instrument interpolation settings
	// and old files have these exact values in them which should not change meaning.
	SRCMODE_NEAREST   = 0,  // 1 tap, no AA
	SRCMODE_LINEAR    = 1,  // 2 tap, no AA
	SRCMODE_CUBIC     = 2,  // 4 tap, no AA
	SRCMODE_SINC8     = 4,  // 8 tap, no AA   (yes, index 4) (XMMS-ModPlug)
	SRCMODE_SINC8LP   = 3,  // 8 tap, with AA (yes, index 3) (Polyphase)

	SRCMODE_DEFAULT   = 5,  // Only used for instrument settings, not used inside the mixer

	SRCMODE_AMIGA  = 0xFF,  // Not explicitely user-selectable
};

namespace Resampling
{

enum class AmigaFilter
{
	Off        = 0,
	A500       = 1,
	A1200      = 2,
	Unfiltered = 3,
};

inline std::array<ResamplingMode, 5> AllModes() noexcept { return { { SRCMODE_NEAREST, SRCMODE_LINEAR, SRCMODE_CUBIC, SRCMODE_SINC8, SRCMODE_SINC8LP } }; }

inline std::array<ResamplingMode, 6> AllModesWithDefault() noexcept { return { { SRCMODE_NEAREST, SRCMODE_LINEAR, SRCMODE_CUBIC, SRCMODE_SINC8, SRCMODE_SINC8LP, SRCMODE_DEFAULT } }; }

constexpr ResamplingMode Default() noexcept { return SRCMODE_SINC8LP; }

constexpr bool IsKnownMode(int mode) noexcept { return (mode >= 0) && (mode < SRCMODE_DEFAULT); }

constexpr ResamplingMode ToKnownMode(int mode) noexcept
{
	return Resampling::IsKnownMode(mode) ? static_cast<ResamplingMode>(mode)
		: (mode == SRCMODE_AMIGA) ? SRCMODE_LINEAR
		: Resampling::Default();
}

constexpr int Length(ResamplingMode mode) noexcept
{
	return mode == SRCMODE_NEAREST ? 1
		: mode == SRCMODE_LINEAR ? 2
		: mode == SRCMODE_CUBIC ? 4
		: mode == SRCMODE_SINC8 ? 8
		: mode == SRCMODE_SINC8LP ? 8
		: 0;
}

constexpr bool HasAA(ResamplingMode mode) noexcept { return (mode == SRCMODE_SINC8LP); }

constexpr ResamplingMode AddAA(ResamplingMode mode) noexcept { return (mode == SRCMODE_SINC8) ? SRCMODE_SINC8LP : mode; }

constexpr ResamplingMode RemoveAA(ResamplingMode mode) noexcept { return (mode == SRCMODE_SINC8LP) ? SRCMODE_SINC8 : mode; }

}



// Release node defines
#define ENV_RELEASE_NODE_UNSET	0xFF
#define NOT_YET_RELEASED		(-1)
static_assert(ENV_RELEASE_NODE_UNSET > MAX_ENVPOINTS);


enum PluginPriority
{
	ChannelOnly,
	InstrumentOnly,
	PrioritiseInstrument,
	PrioritiseChannel,
};

enum PluginMutePriority
{
	EvenIfMuted,
	RespectMutes,
};

// Plugin velocity handling options
enum PlugVelocityHandling : uint8
{
	PLUGIN_VELOCITYHANDLING_CHANNEL = 0,
	PLUGIN_VELOCITYHANDLING_VOLUME
};

// Plugin volumecommand handling options
enum PlugVolumeHandling : uint8
{
	PLUGIN_VOLUMEHANDLING_MIDI = 0,
	PLUGIN_VOLUMEHANDLING_DRYWET,
	PLUGIN_VOLUMEHANDLING_IGNORE,
	PLUGIN_VOLUMEHANDLING_CUSTOM,
	PLUGIN_VOLUMEHANDLING_MAX,
};

enum MidiChannel : uint8
{
	MidiNoChannel		= 0,
	MidiFirstChannel	= 1,
	MidiLastChannel		= 16,
	MidiMappedChannel	= 17,
};


// Vibrato Types
enum VibratoType : uint8
{
	VIB_SINE = 0,
	VIB_SQUARE,
	VIB_RAMP_UP,
	VIB_RAMP_DOWN,
	VIB_RANDOM
};


// Tracker-specific playback behaviour
// Note: The index of every flag has to be fixed, so do not remove flags. Always add new flags at the end!
enum PlayBehaviour
{
	MSF_COMPATIBLE_PLAY,            // No-op - only used during loading (Old general compatibility flag for IT/MPT/XM)
	kMPTOldSwingBehaviour,          // MPT 1.16 swing behaviour (IT/MPT, deprecated)
	kMIDICCBugEmulation,            // Emulate broken volume MIDI CC behaviour (IT/MPT/XM, deprecated)
	kOldMIDIPitchBends,             // Old VST MIDI pitch bend behaviour (IT/MPT/XM, deprecated)
	kFT2VolumeRamping,              // Smooth volume ramping like in FT2 (XM)
	kMODVBlankTiming,               // F21 and above set speed instead of tempo
	kSlidesAtSpeed1,                // Execute normal slides at speed 1 as if they were fine slides
	kPeriodsAreHertz,               // Compute note frequency in Hertz rather than periods
	kTempoClamp,                    // Clamp tempo to 32-255 range.
	kPerChannelGlobalVolSlide,      // Global volume slide memory is per-channel
	kPanOverride,                   // Panning commands override surround and random pan variation

	kITInstrWithoutNote,            // Avoid instrument handling if there is no note
	kITVolColFinePortamento,        // Volume column portamento never does fine portamento
	kITArpeggio,                    // IT arpeggio algorithm
	kITOutOfRangeDelay,             // Out-of-range delay command behaviour in IT
	kITPortaMemoryShare,            // Gxx shares memory with Exx and Fxx
	kITPatternLoopTargetReset,      // After finishing a pattern loop, set the pattern loop target to the next row
	kITFT2PatternLoop,              // Nested pattern loop behaviour
	kITPingPongNoReset,             // Don't reset ping pong direction with instrument numbers
	kITEnvelopeReset,               // IT envelope reset behaviour
	kITClearOldNoteAfterCut,        // Forget the previous note after cutting it
	kITVibratoTremoloPanbrello,     // More IT-like Hxx / hx, Rxx, Yxx and autovibrato handling, including more precise LUTs
	kITTremor,                      // Ixx behaves like in IT
	kITRetrigger,                   // Qxx behaves like in IT
	kITMultiSampleBehaviour,        // Properly update C-5 frequency when changing in multisampled instrument
	kITPortaTargetReached,          // Clear portamento target after it has been reached
	kITPatternLoopBreak,            // Don't reset loop count on pattern break.
	kITOffset,                      // IT-style Oxx edge case handling
	kITSwingBehaviour,              // IT's swing behaviour
	kITNNAReset,                    // NNA is reset on every note change, not every instrument change
	kITSCxStopsSample,              // SCx really stops the sample and does not just mute it
	kITEnvelopePositionHandling,    // IT-style envelope position advance + enable/disable behaviour
	kITPortamentoInstrument,        // No sample changes during portamento with Compatible Gxx enabled, instrument envelope reset with portamento
	kITPingPongMode,                // Don't repeat last sample point in ping pong loop, like IT's software mixer
	kITRealNoteMapping,             // Use triggered note rather than translated note for PPS and other effects
	kITHighOffsetNoRetrig,          // SAx should not apply an offset effect to a note next to it
	kITFilterBehaviour,             // User IT's filter coefficients (unless extended filter range is used)
	kITNoSurroundPan,               // Panning and surround are mutually exclusive
	kITShortSampleRetrig,           // Don't retrigger already stopped channels
	kITPortaNoNote,                 // Don't apply any portamento if no previous note is playing
	kITFT2DontResetNoteOffOnPorta,  // Only reset note-off status on portamento in IT Compatible Gxx mode
	kITVolColMemory,                // IT volume column effects share their memory with the effect column
	kITPortamentoSwapResetsPos,     // Portamento with sample swap plays the new sample from the beginning
	kITEmptyNoteMapSlot,            // IT ignores instrument note map entries with no note completely
	kITFirstTickHandling,           // IT-style first tick handling
	kITSampleAndHoldPanbrello,      // IT-style sample&hold panbrello waveform
	kITClearPortaTarget,            // New notes reset portamento target in IT
	kITPanbrelloHold,               // Don't reset panbrello effect until next note or panning effect
	kITPanningReset,                // Sample and instrument panning is only applied on note change, not instrument change
	kITPatternLoopWithJumpsOld,     // Bxx on the same row as SBx terminates the loop in IT (old implementation of kITPatternLoopWithJumps)
	kITInstrWithNoteOff,            // Instrument number with note-off recalls default volume

	kFT2Arpeggio,                   // FT2 arpeggio algorithm
	kFT2Retrigger,                  // Rxx behaves like in FT2
	kFT2VolColVibrato,              // Vibrato depth in volume column does not actually execute the vibrato effect
	kFT2PortaNoNote,                // Don't play portamento-ed note if no previous note is playing
	kFT2KeyOff,                     // FT2-style Kxx handling
	kFT2PanSlide,                   // Volume-column pan slides should be handled like fine slides
	kFT2ST3OffsetOutOfRange,        // Offset past sample end stops the note
	kFT2RestrictXCommand,           // Don't allow MPT extensions to Xxx command in XM
	kFT2RetrigWithNoteDelay,        // Retrigger envelopes if there is a note delay with no note
	kFT2SetPanEnvPos,               // Lxx only sets the pan env position if the volume envelope's sustain flag is set
	kFT2PortaIgnoreInstr,           // Portamento plus instrument number applies the volume settings of the new sample, but not the new sample itself.
	kFT2VolColMemory,               // No volume column memory in FT2
	kFT2LoopE60Restart,             // Next pattern starts on the same row as the last E60 command
	kFT2ProcessSilentChannels,      // Keep processing silent channels for later 3xx pickup
	kFT2ReloadSampleSettings,       // Reload sample settings even if a note-off is placed next to an instrument number
	kFT2PortaDelay,                 // Portamento with note delay next to it is ignored in FT2
	kFT2Transpose,                  // Out-of-range transposed notes in FT2
	kFT2PatternLoopWithJumps,       // Bxx or Dxx on the same row as E6x terminates the loop in FT2
	kFT2PortaTargetNoReset,         // Portamento target is not reset with new notes in FT2
	kFT2EnvelopeEscape,             // FT2 sustain point at end of envelope
	kFT2Tremor,                     // Txx behaves like in FT2
	kFT2OutOfRangeDelay,            // Out-of-range delay command behaviour in FT2
	kFT2Periods,                    // Use FT2's broken period handling
	kFT2PanWithDelayedNoteOff,      // Pan command with delayed note-off
	kFT2VolColDelay,                // FT2-style volume column handling if there is a note delay
	kFT2FinetunePrecision,          // Only take the upper 4 bits of sample finetune.

	kST3NoMutedChannels,            // Don't process any effects on muted S3M channels
	kST3EffectMemory,               // Most effects share the same memory in ST3
	kST3PortaSampleChange,          // Portamento plus instrument number applies the volume settings of the new sample, but not the new sample itself (GUS behaviour).
	kST3VibratoMemory,              // Do not remember vibrato type in effect memory
	kST3LimitPeriod,                // Cut note instead of limiting  final period (ModPlug Tracker style)
	KST3PortaAfterArpeggio,         // Portamento after arpeggio continues at the note where the arpeggio left off

	kMODOneShotLoops,               // Allow ProTracker-like oneshot loops
	kMODIgnorePanning,              // Do not process any panning commands
	kMODSampleSwap,                 // On-the-fly sample swapping

	kFT2NoteOffFlags,               // Set and reset the correct fade/key-off flags with note-off and instrument number after note-off
	kITMultiSampleInstrumentNumber, // After portamento to different sample within multi-sampled instrument, lone instrument numbers in patterns always recall the new sample's default settings
	kRowDelayWithNoteDelay,         // Retrigger note delays on every reptition of a row
	kFT2MODTremoloRampWaveform,     // FT2-/ProTracker-compatible tremolo ramp down / triangle waveform
	kFT2PortaUpDownMemory,          // Portamento up and down have separate memory

	kMODOutOfRangeNoteDelay,        // ProTracker behaviour for out-of-range note delays
	kMODTempoOnSecondTick,          // ProTracker sets tempo after the first tick

	kFT2PanSustainRelease,          // If the sustain point of a panning envelope is reached before key-off, FT2 does not escape it anymore
	kLegacyReleaseNode,             // Legacy release node volume processing
	kOPLBeatingOscillators,         // Emulate beating FM oscillators from CDFM / Composer 670
	kST3OffsetWithoutInstrument,    // Note without instrument uses same offset as previous note
	kReleaseNodePastSustainBug,     // OpenMPT 1.23.01.02 / r4009 broke release nodes past the sustain point, fixed in OpenMPT 1.28
	kFT2NoteDelayWithoutInstr,      // Sometime between OpenMPT 1.18.03.00 and 1.19.01.00, delayed instrument-less notes in XM started recalling the default sample volume and panning
	kOPLFlexibleNoteOff,            // Full control after note-off over OPL voices, ^^^ sends note cut instead of just note-off
	kITInstrWithNoteOffOldEffects,  // Instrument number with note-off recalls default volume - special cases with Old Effects enabled
	kMIDIVolumeOnNoteOffBug,        // Update MIDI channel volume on note-off (legacy bug emulation)
	kITDoNotOverrideChannelPan,     // Sample / instrument pan does not override channel pan for following samples / instruments that are not panned
	kITPatternLoopWithJumps,        // Bxx right of SBx terminates the loop in IT
	kITDCTBehaviour,                // DCT="Sample" requires sample instrument, DCT="Note" checks old pattern note against new pattern note (previously was checking old pattern note against new translated note)
	kOPLwithNNA,                    // NNA note-off / fade are applied to OPL channels
	kST3RetrigAfterNoteCut,         // Qxy does not retrigger note after it has been cut with ^^^ or SCx
	kST3SampleSwap,                 // On-the-fly sample swapping (SoundBlaster behaviour)
	kOPLRealRetrig,                 // Retrigger effect (Qxy) restarts OPL notes
	kOPLNoResetAtEnvelopeEnd,       // Do not reset OPL channel status at end of envelope (OpenMPT 1.28 inconsistency with samples)
	kOPLNoteStopWith0Hz,            // Set note frequency to 0 Hz to "stop" OPL notes
	kOPLNoteOffOnNoteChange,        // Send note-off events for old note on every note change
	kFT2PortaResetDirection,        // Reset portamento direction when reaching portamento target from below
	kApplyUpperPeriodLimit,         // Enforce m_nMaxPeriod
	kApplyOffsetWithoutNote,        // Offset commands even work when there's no note next to them (e.g. DMF, MDL, PLM formats)
	kITPitchPanSeparation,          // Pitch/Pan Separation can be overridden by panning commands (this also fixes a bug where any "special" notes affect PPS)
	kImprecisePingPongLoops,        // Use old (less precise) ping-pong overshoot calculation
	kPluginIgnoreTonePortamento,    // Use old tone portamento behaviour for plugins (XM: no plugin pitch slides with commands E1x/E2x/X1x/X2x)
	kST3TonePortaWithAdlibNote,     // Adlib note next to tone portamento is delayed until next row
	kITResetFilterOnPortaSmpChange, // Filter is reset on portamento if sample is swapped
	kITInitialNoteMemory,           // Initial "last note memory" for each channel is C-0 and not "no note"
	kPluginDefaultProgramAndBank1,  // Default program and bank is set to 1 for plugins, so if an instrument is set to either of those, the program / bank change event is not sent to the plugin
	kITNoSustainOnPortamento,       // Do not re-enable sustain loop on portamento, even when switching between samples
	kITEmptyNoteMapSlotIgnoreCell,  // IT ignores the entire pattern cell when trying to play an unmapped note of an instrument
	kITOffsetWithInstrNumber,       // IT applies offset commands even if just an instrument number without note is present
	kContinueSampleWithoutInstr,    // FTM: A note without instrument number continues looped samples with the new pitch instead of retriggering them
	kMIDINotesFromChannelPlugin,    // Behaviour before OpenMPT 1.26: Channel plugin can be used to send MIDI notes
	kITDoublePortamentoSlides,      // IT only reads parameters once per row, so if two commands sharing effect parameters are found in the two effect columns, they influence each other
	kS3MIgnoreCombinedFineSlides,   // S3M commands Kxy and Lxy ignore fine slides
	kFT2AutoVibratoAbortSweep,      // Key-off before auto-vibrato sweep-in is complete resets auto-vibrato depth
	kLegacyPPQpos,                  // Report fake PPQ position to VST plugins
	kLegacyPluginNNABehaviour,      // Plugin notes with NNA=continue are affected by note-offs etc.
	kITCarryAfterNoteOff,           // Envelope Carry continues to function as normal even after note-off
	kFT2OffsetMemoryRequiresNote,   // Offset memory is only updated when offset command is next to a note
	kITNoteCutWithPorta,            // Note Cut (SCx) resets note frequency and interacts with tone portamento with row delay
	kITVolColNoSlidePropagation,    // Don't propagate volume command c/d parameter to regular command D memory
	kITStoppedFilterEnvAtStart,     // Stopped filter envelope is still applied even when its first tick has not been processed yet

	// Add new play behaviours here.

	kMaxPlayBehaviours,
};


// Tempo swing determines how much every row in modern tempo mode contributes to a beat.
class TempoSwing : public std::vector<uint32>
{
public:
	static constexpr uint32 Unity = 1u << 24;
	// Normalize the tempo swing coefficients so that they add up to exactly the specified tempo again
	void Normalize();
	void resize(size_type newSize, value_type val = Unity) { std::vector<uint32>::resize(newSize, val); Normalize(); }

	static void Serialize(std::ostream &oStrm, const TempoSwing &swing);
	static void Deserialize(std::istream &iStrm, TempoSwing &swing, const size_t);
};


// Sample position and sample position increment value
struct SamplePosition
{
	using value_t = int64;
	using unsigned_value_t = uint64;

protected:
	value_t v = 0;

public:
	static constexpr uint32 fractMax = 0xFFFFFFFFu;

	MPT_CONSTEXPRINLINE SamplePosition() { }
	MPT_CONSTEXPRINLINE explicit SamplePosition(value_t pos) : v(pos) { }
	MPT_CONSTEXPRINLINE SamplePosition(int32 intPart, uint32 fractPart) : v((static_cast<value_t>(intPart) * (1ll << 32)) | fractPart) { }
	static SamplePosition Ratio(uint32 dividend, uint32 divisor) { return SamplePosition((static_cast<int64>(dividend) << 32) / divisor); }
	static SamplePosition FromDouble(double pos) { return SamplePosition(static_cast<value_t>(pos * 4294967296.0)); }
	double ToDouble() const { return static_cast<double>(v) / 4294967296.0; }

	// Set integer and fractional part
	MPT_CONSTEXPRINLINE SamplePosition &Set(int32 intPart, uint32 fractPart = 0) { v = (static_cast<int64>(intPart) << 32) | fractPart; return *this; }
	// Set integer part, keep fractional part
	MPT_CONSTEXPRINLINE SamplePosition &SetInt(int32 intPart) { v = (static_cast<value_t>(intPart) << 32) | GetFract(); return *this; }
	// Get integer part (as sample length / position)
	MPT_CONSTEXPRINLINE SmpLength GetUInt() const { return static_cast<SmpLength>(static_cast<unsigned_value_t>(v) >> 32); }
	// Get integer part
	MPT_CONSTEXPRINLINE int32 GetInt() const { return static_cast<int32>(static_cast<unsigned_value_t>(v) >> 32); }
	// Get fractional part
	MPT_CONSTEXPRINLINE uint32 GetFract() const { return static_cast<uint32>(v); }
	// Get the inverted fractional part
	MPT_CONSTEXPRINLINE SamplePosition GetInvertedFract() const { return SamplePosition(0x100000000ll - GetFract()); }
	// Get the raw fixed-point value
	MPT_CONSTEXPRINLINE int64 GetRaw() const { return v; }
	// Negate the current value
	MPT_CONSTEXPRINLINE SamplePosition &Negate() { v = -v; return *this; }
	// Multiply and divide by given integer scalars
	MPT_CONSTEXPRINLINE SamplePosition &MulDiv(uint32 mul, uint32 div) { v = (v * mul) / div; return *this; }
	// Removes the integer part, only keeping fractions
	MPT_CONSTEXPRINLINE SamplePosition &RemoveInt() { v &= fractMax; return *this; }
	// Check if value is 1.0
	MPT_CONSTEXPRINLINE bool IsUnity() const { return v == 0x100000000ll; }
	// Check if value is 0
	MPT_CONSTEXPRINLINE bool IsZero() const { return v == 0; }
	// Check if value is > 0
	MPT_CONSTEXPRINLINE bool IsPositive() const { return v > 0; }
	// Check if value is < 0
	MPT_CONSTEXPRINLINE bool IsNegative() const { return v < 0; }

	// Addition / subtraction of another fixed-point number
	SamplePosition operator+ (const SamplePosition &other) const { return SamplePosition(v + other.v); }
	SamplePosition operator- (const SamplePosition &other) const { return SamplePosition(v - other.v); }
	void operator+= (const SamplePosition &other) { v += other.v; }
	void operator-= (const SamplePosition &other) { v -= other.v; }

	// Multiplication with integer scalar
	template<typename T>
	SamplePosition operator* (T other) const { return SamplePosition(static_cast<value_t>(v * other)); }
	template<typename T>
	void operator*= (T other) { v = static_cast<value_t>(v *other); }

	// Division by other fractional point number; returns scalar
	value_t operator/ (SamplePosition other) const { return v / other.v; }
	// Division by scalar; returns fractional point number
	SamplePosition operator/ (int div) const { return SamplePosition(v / div); }

	MPT_CONSTEXPRINLINE bool operator==(const SamplePosition &other) const { return v == other.v; }
	MPT_CONSTEXPRINLINE bool operator!=(const SamplePosition &other) const { return v != other.v; }
	MPT_CONSTEXPRINLINE bool operator<=(const SamplePosition &other) const { return v <= other.v; }
	MPT_CONSTEXPRINLINE bool operator>=(const SamplePosition &other) const { return v >= other.v; }
	MPT_CONSTEXPRINLINE bool operator<(const SamplePosition &other) const { return v < other.v; }
	MPT_CONSTEXPRINLINE bool operator>(const SamplePosition &other) const { return v > other.v; }
};


// Aaaand another fixed-point type, e.g. used for fractional tempos
// Note that this doesn't use classical bit shifting for the fixed point part.
// This is mostly for the clarity of stored values and to be able to represent any value .0000 to .9999 properly.
// For easier debugging, use the Debugger Visualizers available in build/vs/debug/
// to easily display the stored values.
template <std::size_t FFact, typename T>
struct FPInt
{
protected:
	T v;
	MPT_CONSTEXPRINLINE FPInt(T rawValue) : v(rawValue) { }

public:
	enum : T { fractFact = static_cast<T>(FFact) };
	using store_t = T;

	MPT_CONSTEXPRINLINE FPInt() : v(0) { }
	MPT_CONSTEXPRINLINE FPInt(T intPart, T fractPart) : v((intPart * fractFact) + (fractPart % fractFact)) { }
	explicit MPT_CONSTEXPRINLINE FPInt(float f) : v(mpt::saturate_round<T>(f * float(fractFact))) { }
	explicit MPT_CONSTEXPRINLINE FPInt(double f) : v(mpt::saturate_round<T>(f * double(fractFact))) { }

	// Set integer and fractional part
	MPT_CONSTEXPRINLINE FPInt<fractFact, T> &Set(T intPart, T fractPart = 0) { v = (intPart * fractFact) + (fractPart % fractFact); return *this; }
	// Set raw internal representation directly
	MPT_CONSTEXPRINLINE FPInt<fractFact, T> &SetRaw(T value) { v = value; return *this; }
	// Retrieve the integer part of the stored value
	MPT_CONSTEXPRINLINE T GetInt() const { return v / fractFact; }
	// Retrieve the fractional part of the stored value
	MPT_CONSTEXPRINLINE T GetFract() const { return v % fractFact; }
	// Retrieve the raw internal representation of the stored value
	MPT_CONSTEXPRINLINE T GetRaw() const { return v; }
	// Formats the stored value as a floating-point value
	MPT_CONSTEXPRINLINE double ToDouble() const { return v / double(fractFact); }

	MPT_CONSTEXPRINLINE friend FPInt<fractFact, T> operator+ (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return FPInt<fractFact, T>(a.v + b.v); }
	MPT_CONSTEXPRINLINE friend FPInt<fractFact, T> operator- (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return FPInt<fractFact, T>(a.v - b.v); }
	MPT_CONSTEXPRINLINE FPInt<fractFact, T> operator+= (const FPInt<fractFact, T> &other) noexcept { v += other.v; return *this; }
	MPT_CONSTEXPRINLINE FPInt<fractFact, T> operator-= (const FPInt<fractFact, T> &other) noexcept { v -= other.v; return *this; }

	MPT_CONSTEXPRINLINE friend bool operator== (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return a.v == b.v; }
	MPT_CONSTEXPRINLINE friend bool operator!= (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return a.v != b.v; }
	MPT_CONSTEXPRINLINE friend bool operator<= (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return a.v <= b.v; }
	MPT_CONSTEXPRINLINE friend bool operator>= (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return a.v >= b.v; }
	MPT_CONSTEXPRINLINE friend bool operator< (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return a.v < b.v; }
	MPT_CONSTEXPRINLINE friend bool operator> (const FPInt<fractFact, T> &a, const FPInt<fractFact, T> &b) noexcept { return a.v > b.v; }
};

using TEMPO = FPInt<10000, uint32>;

using OPLPatch = std::array<uint8, 12>;

OPENMPT_NAMESPACE_END
