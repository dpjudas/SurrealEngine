/*
 * DLSBank.cpp
 * -----------
 * Purpose: Sound bank loading.
 * Notes  : Supported sound bank types: DLS (including embedded DLS in MSS & RMI), SF2, SF3 / SF4 (modified SF2 with compressed samples)
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Dlsbank.h"
#include "Sndfile.h"
#ifdef MODPLUG_TRACKER
#include "../common/mptFileIO.h"
#include "../mptrack/Mptrack.h"
#include "mpt/io_file/inputfile.hpp"
#include "mpt/io_file_read/inputfile_filecursor.hpp"
#endif
#include "Loaders.h"
#include "SampleCopy.h"
#include "SampleIO.h"
#include "../common/FileReader.h"
#include "../common/mptStringBuffer.h"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "openmpt/base/Endian.hpp"

OPENMPT_NAMESPACE_BEGIN

#ifdef MODPLUG_TRACKER

#ifdef MPT_ALL_LOGGING
#define DLSBANK_LOG
#define DLSINSTR_LOG
#endif

#define F_RGN_OPTION_SELFNONEXCLUSIVE	0x0001

// Region Flags
enum RegionFlags
{
	DLSREGION_KEYGROUPMASK     = 0x0F,
	DLSREGION_OVERRIDEWSMP     = 0x10,
	DLSREGION_PINGPONGLOOP     = 0x20,
	DLSREGION_SAMPLELOOP       = 0x40,
	DLSREGION_SELFNONEXCLUSIVE = 0x80,
	DLSREGION_SUSTAINLOOP      = 0x100,
};

///////////////////////////////////////////////////////////////////////////
// Articulation connection graph definitions

enum ConnectionSource : uint16
{
	// Generic Sources
	CONN_SRC_NONE              = 0x0000,
	CONN_SRC_LFO               = 0x0001,
	CONN_SRC_KEYONVELOCITY     = 0x0002,
	CONN_SRC_KEYNUMBER         = 0x0003,
	CONN_SRC_EG1               = 0x0004,
	CONN_SRC_EG2               = 0x0005,
	CONN_SRC_PITCHWHEEL        = 0x0006,

	CONN_SRC_POLYPRESSURE      = 0x0007,
	CONN_SRC_CHANNELPRESSURE   = 0x0008,
	CONN_SRC_VIBRATO           = 0x0009,

	// Midi Controllers 0-127
	CONN_SRC_CC1               = 0x0081,
	CONN_SRC_CC7               = 0x0087,
	CONN_SRC_CC10              = 0x008a,
	CONN_SRC_CC11              = 0x008b,

	CONN_SRC_CC91              = 0x00db,
	CONN_SRC_CC93              = 0x00dd,

	CONN_SRC_RPN0              = 0x0100,
	CONN_SRC_RPN1              = 0x0101,
	CONN_SRC_RPN2              = 0x0102,
};

enum ConnectionDestination : uint16
{
	// Generic Destinations
	CONN_DST_NONE              = 0x0000,
	CONN_DST_ATTENUATION       = 0x0001,
	CONN_DST_RESERVED          = 0x0002,
	CONN_DST_PITCH             = 0x0003,
	CONN_DST_PAN               = 0x0004,

	// LFO Destinations
	CONN_DST_LFO_FREQUENCY     = 0x0104,
	CONN_DST_LFO_STARTDELAY    = 0x0105,

	CONN_DST_KEYNUMBER         = 0x0005,

	// EG1 Destinations
	CONN_DST_EG1_ATTACKTIME    = 0x0206,
	CONN_DST_EG1_DECAYTIME     = 0x0207,
	CONN_DST_EG1_RESERVED      = 0x0208,
	CONN_DST_EG1_RELEASETIME   = 0x0209,
	CONN_DST_EG1_SUSTAINLEVEL  = 0x020a,

	CONN_DST_EG1_DELAYTIME     = 0x020b,
	CONN_DST_EG1_HOLDTIME      = 0x020c,
	CONN_DST_EG1_SHUTDOWNTIME  = 0x020d,

	// EG2 Destinations
	CONN_DST_EG2_ATTACKTIME    = 0x030a,
	CONN_DST_EG2_DECAYTIME     = 0x030b,
	CONN_DST_EG2_RESERVED      = 0x030c,
	CONN_DST_EG2_RELEASETIME   = 0x030d,
	CONN_DST_EG2_SUSTAINLEVEL  = 0x030e,

	CONN_DST_EG2_DELAYTIME     = 0x030f,
	CONN_DST_EG2_HOLDTIME      = 0x0310,

	CONN_TRN_NONE              = 0x0000,
	CONN_TRN_CONCAVE           = 0x0001,
};


//////////////////////////////////////////////////////////
// Supported DLS1 Articulations

// [4-bit transform][12-bit dest][8-bit control][8-bit source] = 32-bit ID
constexpr uint32 DLSArt(uint8 src, uint8 ctl, uint16 dst)
{
	return (dst << 16u) | (ctl << 8u) | src;
}

enum DLSArt : uint32
{
	// Vibrato / Tremolo
	ART_LFO_FREQUENCY   = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_LFO_FREQUENCY),
	ART_LFO_STARTDELAY  = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_LFO_STARTDELAY),
	ART_LFO_ATTENUATION = DLSArt(CONN_SRC_LFO,  CONN_SRC_NONE, CONN_DST_ATTENUATION),
	ART_LFO_PITCH       = DLSArt(CONN_SRC_LFO,  CONN_SRC_NONE, CONN_DST_PITCH),
	ART_LFO_MODWTOATTN  = DLSArt(CONN_SRC_LFO,  CONN_SRC_CC1,  CONN_DST_ATTENUATION),
	ART_LFO_MODWTOPITCH = DLSArt(CONN_SRC_LFO,  CONN_SRC_CC1,  CONN_DST_PITCH),

	// Volume Envelope
	ART_VOL_EG_ATTACKTIME   = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_ATTACKTIME),
	ART_VOL_EG_DECAYTIME    = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME),
	ART_VOL_EG_SUSTAINLEVEL = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_SUSTAINLEVEL),
	ART_VOL_EG_RELEASETIME  = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_RELEASETIME),
	ART_VOL_EG_DELAYTIME    = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_DELAYTIME),
	ART_VOL_EG_HOLDTIME     = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_HOLDTIME),
	ART_VOL_EG_SHUTDOWNTIME = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG1_SHUTDOWNTIME),
	ART_VOL_EG_VELTOATTACK  = DLSArt(CONN_SRC_KEYONVELOCITY, CONN_SRC_NONE, CONN_DST_EG1_ATTACKTIME),
	ART_VOL_EG_KEYTODECAY   = DLSArt(CONN_SRC_KEYNUMBER,     CONN_SRC_NONE, CONN_DST_EG1_DECAYTIME),

	// Pitch Envelope
	ART_PITCH_EG_ATTACKTIME   = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_ATTACKTIME),
	ART_PITCH_EG_DECAYTIME    = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_DECAYTIME),
	ART_PITCH_EG_SUSTAINLEVEL = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_SUSTAINLEVEL),
	ART_PITCH_EG_RELEASETIME  = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_RELEASETIME),
	ART_PITCH_EG_VELTOATTACK  = DLSArt(CONN_SRC_KEYONVELOCITY, CONN_SRC_NONE, CONN_DST_EG2_ATTACKTIME),
	ART_PITCH_EG_KEYTODECAY   = DLSArt(CONN_SRC_KEYNUMBER,     CONN_SRC_NONE, CONN_DST_EG2_DECAYTIME),
	ART_PITCH_EG_DELAYTIME    = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_DELAYTIME),
	ART_PITCH_EG_HOLDTIME     = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_EG2_HOLDTIME),
	ART_PITCH_EG_DEPTH        = DLSArt(CONN_SRC_EG2,  CONN_SRC_NONE, CONN_DST_PITCH),

	// Default Pan
	ART_DEFAULTPAN = DLSArt(CONN_SRC_NONE, CONN_SRC_NONE, CONN_DST_PAN),
};

//////////////////////////////////////////////////////////
// DLS IFF Chunk IDs

enum IFFChunkID : uint32
{
	// Standard IFF chunks IDs
	IFFID_FORM = MagicLE("FORM"),
	IFFID_RIFF = MagicLE("RIFF"),
	IFFID_LIST = MagicLE("LIST"),
	IFFID_INFO = MagicLE("INFO"),

	// IFF Info fields
	IFFID_ICOP = MagicLE("ICOP"),
	IFFID_INAM = MagicLE("INAM"),
	IFFID_ICMT = MagicLE("ICMT"),
	IFFID_IENG = MagicLE("IENG"),
	IFFID_ISFT = MagicLE("ISFT"),
	IFFID_ISBJ = MagicLE("ISBJ"),

	// Wave IFF chunks IDs
	IFFID_wave = MagicLE("wave"),
	IFFID_wsmp = MagicLE("wsmp"),

	IFFID_XDLS = MagicLE("XDLS"),
	IFFID_DLS  = MagicLE("DLS "),
	IFFID_MLS  = MagicLE("MLS "),
	IFFID_RMID = MagicLE("RMID"),
	IFFID_colh = MagicLE("colh"),
	IFFID_ins  = MagicLE("ins "),
	IFFID_insh = MagicLE("insh"),
	IFFID_ptbl = MagicLE("ptbl"),
	IFFID_wvpl = MagicLE("wvpl"),
	IFFID_rgn  = MagicLE("rgn "),
	IFFID_rgn2 = MagicLE("rgn2"),
	IFFID_rgnh = MagicLE("rgnh"),
	IFFID_wlnk = MagicLE("wlnk"),
	IFFID_art1 = MagicLE("art1"),
	IFFID_art2 = MagicLE("art2"),
};

//////////////////////////////////////////////////////////
// DLS Structures definitions

struct IFFCHUNK
{
	uint32le id;
	uint32le len;
};

MPT_BINARY_STRUCT(IFFCHUNK, 8)

struct RIFFChunkID
{
	uint32le id_RIFF;
	uint32le riff_len;
	uint32le id_DLS;
};

MPT_BINARY_STRUCT(RIFFChunkID, 12)

struct LISTChunk
{
	uint32le id;
	uint32le len;
	uint32le listid;
};

MPT_BINARY_STRUCT(LISTChunk, 12)

struct DLSRgnRange
{
	uint16le usLow;
	uint16le usHigh;
};

MPT_BINARY_STRUCT(DLSRgnRange, 4)

struct VERSChunk
{
	uint32le id;
	uint32le len;
	uint16le version[4];
};

MPT_BINARY_STRUCT(VERSChunk, 16)

struct PTBLChunk
{
	uint32le cbSize;
	uint32le cCues;
};

MPT_BINARY_STRUCT(PTBLChunk, 8)

struct INSHChunk
{
	uint32le cRegions;
	uint32le ulBank;
	uint32le ulInstrument;
};

MPT_BINARY_STRUCT(INSHChunk, 12)

struct RGNHChunk
{
	DLSRgnRange RangeKey;
	DLSRgnRange RangeVelocity;
	uint16le fusOptions;
	uint16le usKeyGroup;
};

MPT_BINARY_STRUCT(RGNHChunk, 12)

struct WLNKChunk
{
	uint16le fusOptions;
	uint16le usPhaseGroup;
	uint32le ulChannel;
	uint32le ulTableIndex;
};

MPT_BINARY_STRUCT(WLNKChunk, 12)

struct ART1Chunk
{
	uint32le cbSize;
	uint32le cConnectionBlocks;
};

MPT_BINARY_STRUCT(ART1Chunk, 8)

struct ConnectionBlock
{
	uint16le usSource;
	uint16le usControl;
	uint16le usDestination;
	uint16le usTransform;
	int32le  lScale;
};

MPT_BINARY_STRUCT(ConnectionBlock, 12)

struct WSMPChunk
{
	uint32le cbSize;
	uint16le usUnityNote;
	int16le  sFineTune;
	int32le  lAttenuation;
	uint32le fulOptions;
	uint32le cSampleLoops;
};

MPT_BINARY_STRUCT(WSMPChunk, 20)

struct WSMPSampleLoop
{
	uint32le cbSize;
	uint32le ulLoopType;
	uint32le ulLoopStart;
	uint32le ulLoopLength;

};

MPT_BINARY_STRUCT(WSMPSampleLoop, 16)


/////////////////////////////////////////////////////////////////////
// SF2 IFF Chunk IDs

enum SF2ChunkID : uint32
{
	IFFID_ifil = MagicLE("ifil"),
	IFFID_sfbk = MagicLE("sfbk"),
	IFFID_sfpk = MagicLE("sfpk"),  // SF2Pack compressed soundfont
	IFFID_sdta = MagicLE("sdta"),
	IFFID_pdta = MagicLE("pdta"),
	IFFID_phdr = MagicLE("phdr"),
	IFFID_pbag = MagicLE("pbag"),
	IFFID_pgen = MagicLE("pgen"),
	IFFID_inst = MagicLE("inst"),
	IFFID_ibag = MagicLE("ibag"),
	IFFID_igen = MagicLE("igen"),
	IFFID_shdr = MagicLE("shdr"),
};

///////////////////////////////////////////
// SF2 Generators IDs

enum SF2Generators : uint16
{
	SF2_GEN_START_LOOP_FINE		= 2,
	SF2_GEN_END_LOOP_FINE		= 3,
	SF2_GEN_MODENVTOPITCH		= 7,
	SF2_GEN_MODENVTOFILTERFC	= 11,
	SF2_GEN_PAN					= 17,
	SF2_GEN_DELAYMODENV			= 25,
	SF2_GEN_ATTACKMODENV		= 26,
	SF2_GEN_HOLDMODENV			= 27,
	SF2_GEN_DECAYMODENV			= 28,
	SF2_GEN_SUSTAINMODENV		= 29,
	SF2_GEN_RELEASEMODENV		= 30,
	SF2_GEN_DELAYVOLENV			= 33,
	SF2_GEN_ATTACKVOLENV		= 34,
	SF2_GEN_HOLDVOLENV			= 35,
	SF2_GEN_DECAYVOLENV			= 36,
	SF2_GEN_SUSTAINVOLENV		= 37,
	SF2_GEN_RELEASEVOLENV		= 38,
	SF2_GEN_INSTRUMENT			= 41,
	SF2_GEN_KEYRANGE			= 43,
	SF2_GEN_START_LOOP_COARSE	= 45,
	SF2_GEN_ATTENUATION			= 48,
	SF2_GEN_END_LOOP_COARSE		= 50,
	SF2_GEN_COARSETUNE			= 51,
	SF2_GEN_FINETUNE			= 52,
	SF2_GEN_SAMPLEID			= 53,
	SF2_GEN_SAMPLEMODES			= 54,
	SF2_GEN_SCALE_TUNING		= 56,
	SF2_GEN_KEYGROUP			= 57,
	SF2_GEN_UNITYNOTE			= 58,
};

/////////////////////////////////////////////////////////////////////
// SF2 Structures Definitions

struct SFPresetHeader
{
	char     achPresetName[20];
	uint16le wPreset;
	uint16le wBank;
	uint16le wPresetBagNdx;
	uint32le dwLibrary;
	uint32le dwGenre;
	uint32le dwMorphology;
};

MPT_BINARY_STRUCT(SFPresetHeader, 38)

struct SFPresetBag
{
	uint16le wGenNdx;
	uint16le wModNdx;
};

MPT_BINARY_STRUCT(SFPresetBag, 4)

struct SFGenList
{
	uint16le sfGenOper;
	uint16le genAmount;

	bool ApplyToEnvelope(DLSENVELOPE &env) const;
};

MPT_BINARY_STRUCT(SFGenList, 4)

struct SFInst
{
	char     achInstName[20];
	uint16le wInstBagNdx;
};

MPT_BINARY_STRUCT(SFInst, 22)

struct SFInstBag
{
	uint16le wGenNdx;
	uint16le wModNdx;
};

MPT_BINARY_STRUCT(SFInstBag, 4)

using SFInstGenList = SFGenList;

struct SFSample
{
	char     achSampleName[20];
	uint32le dwStart;
	uint32le dwEnd;
	uint32le dwStartloop;
	uint32le dwEndloop;
	uint32le dwSampleRate;
	uint8le  byOriginalPitch;
	int8le   chPitchCorrection;
	uint16le wSampleLink;
	uint16le sfSampleType;
};

MPT_BINARY_STRUCT(SFSample, 46)

// End of structures definitions
/////////////////////////////////////////////////////////////////////


struct SF2LoaderInfo
{
	FileReader presetBags;
	FileReader presetGens;
	FileReader insts;
	FileReader instBags;
	FileReader instGens;
};


/////////////////////////////////////////////////////////////////////
// Unit conversion

static uint8 DLSSustainLevelToLinear(int32 sustain)
{
	// 0.1% units
	if(sustain >= 0)
	{
		int32 l = sustain / (1000 * 512);
		if(l >= 0 && l <= 128)
			return static_cast<uint8>(l);
	}
	return 128;
}


static int16 SF2TimeToDLS(int16 amount)
{
	int32 time = CDLSBank::DLS32BitTimeCentsToMilliseconds(static_cast<int32>(amount) << 16);
	return static_cast<int16>(Clamp(time, 20, 20000) / 20);
}


static uint8 SF2SustainLevelToLinear(int32 sustain)
{
	// 0.1% units
	int32 l = 128 * (1000 - Clamp(sustain, 0, 1000)) / 1000;
	return static_cast<uint8>(l);
}


bool SFGenList::ApplyToEnvelope(DLSENVELOPE &env) const
{
	switch(sfGenOper)
	{
	case SF2_GEN_DELAYVOLENV:
		env.volumeEnv.delay = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_ATTACKVOLENV:
		env.volumeEnv.attack = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_HOLDVOLENV:
		env.volumeEnv.hold = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_DECAYVOLENV:
		env.volumeEnv.decay = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_SUSTAINVOLENV:
		// 0.1% units
		if(genAmount >= 0)
		{
			env.volumeEnv.sustainLevel = SF2SustainLevelToLinear(genAmount);
		}
		break;
	case SF2_GEN_RELEASEVOLENV:
		env.volumeEnv.release = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_DELAYMODENV:
		env.pitchEnv.delay = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_ATTACKMODENV:
		env.pitchEnv.attack = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_HOLDMODENV:
		env.pitchEnv.hold = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_DECAYMODENV:
		env.pitchEnv.decay = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_SUSTAINMODENV:
		env.pitchEnv.sustainLevel = SF2SustainLevelToLinear(genAmount);
		break;
	case SF2_GEN_RELEASEMODENV:
		env.pitchEnv.release = SF2TimeToDLS(genAmount);
		break;
	case SF2_GEN_MODENVTOPITCH:
		env.pitchEnvDepth = static_cast<int16>(genAmount);
		break;

	default: return false;
	}
	return true;
}


int32 CDLSBank::DLS32BitTimeCentsToMilliseconds(int32 lTimeCents)
{
	// tc = log2(time[secs]) * 1200*65536
	// time[secs] = 2^(tc/(1200*65536))
	if ((uint32)lTimeCents == 0x80000000) return 0;
	double fmsecs = 1000.0 * std::pow(2.0, ((double)lTimeCents)/(1200.0*65536.0));
	if (fmsecs < -32767) return -32767;
	if (fmsecs > 32767) return 32767;
	return (int32)fmsecs;
}


uint16 CDLSBank::DLSEnvelopeTimeCentsToMilliseconds(int32 value)
{
	if(value <= -0x40000000)
		return 0;
	
	int32 decaytime = DLS32BitTimeCentsToMilliseconds(value);
	if(decaytime > 20000)
		decaytime = 20000;
	if(decaytime >= 20)
		return static_cast<uint16>(decaytime / 20);
	return 0;
}


// 0dB = 0x10000
int32 CDLSBank::DLS32BitRelativeGainToLinear(int32 lCentibels)
{
	// v = 10^(cb/(200*65536)) * V
	return (int32)(65536.0 * std::pow(10.0, ((double)lCentibels)/(200*65536.0)) );
}


int32 CDLSBank::DLS32BitRelativeLinearToGain(int32 lGain)
{
	// cb = log10(v/V) * 200 * 65536
	if (lGain <= 0) return -960 * 65536;
	return (int32)(200 * 65536.0 * std::log10(((double)lGain) / 65536.0));
}


int32 CDLSBank::DLSMidiVolumeToLinear(uint32 nMidiVolume)
{
	return (nMidiVolume * nMidiVolume << 16) / (127*127);
}


/////////////////////////////////////////////////////////////////////
// Implementation

CDLSBank::CDLSBank()
{
	m_nMaxWaveLink = 0;
	m_nType = SOUNDBANK_TYPE_INVALID;
}


bool CDLSBank::IsDLSBank(const mpt::PathString &filename)
{
	if(filename.empty())
		return false;
	mpt::IO::InputFile f(filename, false);
	if(!f.IsValid())
		return false;
	return IsDLSBank(GetFileReader(f));
}


bool CDLSBank::IsDLSBank(FileReader file)
{
	file.Rewind();
	RIFFChunkID riff;
	if(!file.ReadStruct(riff))
		return false;

	// Check for embedded DLS sections
	if(riff.id_RIFF == IFFID_FORM)
	{
		// Miles Sound System
		do
		{
			uint32 len = mpt::bit_cast<uint32be>(riff.riff_len);
			if(len <= 4) break;
			if(riff.id_DLS == IFFID_XDLS)
			{
				if(!file.ReadStruct(riff))
					return false;
				break;  // found it
			}
			if((len % 2u) != 0)
				len++;
			if(!file.Skip(len - 4))
				return false;
		} while(file.ReadStruct(riff));
	} else if(riff.id_RIFF == IFFID_RIFF && riff.id_DLS == IFFID_RMID)
	{
		for (;;)
		{
			if(!file.ReadStruct(riff))
				return false;
			if(riff.id_DLS == IFFID_DLS || riff.id_DLS == IFFID_sfbk)
				break;  // found it
			int len = riff.riff_len;
			if((len % 2u) != 0)
				len++;
			if((len <= 4) || !file.Skip(len - 4))
				return false;
		}
	}
	return ((riff.id_RIFF == IFFID_RIFF)
		&& ((riff.id_DLS == IFFID_DLS) || (riff.id_DLS == IFFID_MLS) || (riff.id_DLS == IFFID_sfbk))
		&& (riff.riff_len >= 256));
}


///////////////////////////////////////////////////////////////
// Find an instrument based on the given parameters

const DLSINSTRUMENT *CDLSBank::FindInstrument(bool isDrum, uint32 bank, uint32 program, uint32 key, uint32 *pInsNo) const
{
	uint32 minBank = ((bank << 1) & 0x7F00) | (bank & 0x7F);
	uint32 maxBank = minBank;
	if(bank >= 0x4000)
	{
		minBank = 0x0000;
		maxBank = 0x7F7F;
	}
	if(isDrum)
	{
		minBank |= F_INSTRUMENT_DRUMS;
		maxBank |= F_INSTRUMENT_DRUMS;
	}

	const bool singleInstr = (minBank == maxBank) && (program < 0x80);
	const auto CompareInstrFunc = [singleInstr](const DLSINSTRUMENT &l, const DLSINSTRUMENT &r)
	{
		if(singleInstr)
			return l < r;
		else
			return l.ulBank < r.ulBank;
	};

	DLSINSTRUMENT findInstr{};
	findInstr.ulInstrument = program;
	findInstr.ulBank = minBank;
	const auto minInstr = std::lower_bound(m_Instruments.begin(), m_Instruments.end(), findInstr, CompareInstrFunc);
	findInstr.ulBank = maxBank;
	const auto maxInstr = std::upper_bound(m_Instruments.begin(), m_Instruments.end(), findInstr, CompareInstrFunc);
	const auto instrRange = mpt::as_span(m_Instruments.data() + std::distance(m_Instruments.begin(), minInstr), std::distance(minInstr, maxInstr));

	for(const DLSINSTRUMENT &dlsIns : instrRange)
	{
		if((program < 0x80) && program != (dlsIns.ulInstrument & 0x7F))
			continue;

		if(isDrum)
		{
			const bool anyKey = !key || key >= 0x80;
			for(const auto &region : dlsIns.Regions)
			{
				if(region.IsDummy())
					continue;

				if(anyKey || (key >= region.uKeyMin && key <= region.uKeyMax))
				{
					if(pInsNo)
						*pInsNo = static_cast<uint32>(std::distance(m_Instruments.data(), &dlsIns));
					return &dlsIns;
				} else if(region.uKeyMin > key)
				{
					// Regions are sorted, if we arrived here we won't find anything in the remaining regions
					break;
				}
			}
		} else
		{
			if(pInsNo)
				*pInsNo = static_cast<uint32>(std::distance(m_Instruments.data(), &dlsIns));
			return &dlsIns;
		}
	}

	return nullptr;
}


bool CDLSBank::FindAndExtract(CSoundFile &sndFile, const INSTRUMENTINDEX ins, const bool isDrum, FileReader *file) const
{
	ModInstrument *pIns = sndFile.Instruments[ins];
	if(pIns == nullptr)
		return false;

	uint32 dlsIns = 0, drumRgn = 0;
	const uint32 program = (pIns->nMidiProgram != 0) ? pIns->nMidiProgram - 1 : 0;
	const uint32 key = isDrum ? (pIns->nMidiDrumKey & 0x7F) : 0xFF;
	if(FindInstrument(isDrum, (pIns->wMidiBank - 1) & 0x3FFF, program, key, &dlsIns)
		|| FindInstrument(isDrum, (pIns->wMidiBank - 1) & 0x3F80, program, key, &dlsIns)
		|| FindInstrument(isDrum, 0xFFFF, isDrum ? 0xFF : program, key, &dlsIns))
	{
		if(key < 0x80) drumRgn = GetRegionFromKey(dlsIns, key);
		if(ExtractInstrument(sndFile, ins, dlsIns, drumRgn, file))
		{
			pIns = sndFile.Instruments[ins]; // Reset pointer because ExtractInstrument may delete the previous value.
			if((key >= 24) && (key < 24 + std::size(szMidiPercussionNames)))
			{
#if MPT_COMPILER_MSVC
#pragma warning(push)
// false-positive
#pragma warning(disable:6385)  // Reading invalid data from 'szMidiPercussionNames'.
#endif
				pIns->name = szMidiPercussionNames[key - 24];
#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif
			}
			return true;
		}
	}
	return false;
}


///////////////////////////////////////////////////////////////
// Update DLS instrument definition from an IFF chunk

bool CDLSBank::UpdateInstrumentDefinition(DLSINSTRUMENT *pDlsIns, FileReader chunk)
{
	IFFCHUNK header;
	chunk.ReadStruct(header);
	if(!header.len || !chunk.CanRead(header.len))
		return false;
	if(header.id == IFFID_LIST)
	{
		uint32 listid = chunk.ReadUint32LE();
		while(chunk.CanRead(sizeof(IFFCHUNK)))
		{
			IFFCHUNK subHeader;
			chunk.ReadStruct(subHeader);
			chunk.SkipBack(sizeof(IFFCHUNK));
			FileReader subData = chunk.ReadChunk(subHeader.len + sizeof(IFFCHUNK));
			if(subHeader.len & 1)
			{
				chunk.Skip(1);
			}
			UpdateInstrumentDefinition(pDlsIns, subData);
		}
		switch(listid)
		{
		case IFFID_rgn:		// Level 1 region
		case IFFID_rgn2:	// Level 2 region
			pDlsIns->Regions.push_back({});
			break;
		}
	} else
	{
		switch(header.id)
		{
		case IFFID_insh:
		{
			INSHChunk insh;
			chunk.ReadStruct(insh);
			pDlsIns->ulBank = insh.ulBank;
			pDlsIns->ulInstrument = insh.ulInstrument;
			//Log("%3d regions, bank 0x%04X instrument %3d\n", insh.cRegions, pDlsIns->ulBank, pDlsIns->ulInstrument);
			break;
		}

		case IFFID_rgnh:
			if(!pDlsIns->Regions.empty())
			{
				RGNHChunk rgnh;
				chunk.ReadStruct(rgnh);
				DLSREGION &region = pDlsIns->Regions.back();
				region.uKeyMin = (uint8)rgnh.RangeKey.usLow;
				region.uKeyMax = (uint8)rgnh.RangeKey.usHigh;
				region.fuOptions = (uint8)(rgnh.usKeyGroup & DLSREGION_KEYGROUPMASK);
				if(rgnh.fusOptions & F_RGN_OPTION_SELFNONEXCLUSIVE)
					region.fuOptions |= DLSREGION_SELFNONEXCLUSIVE;
				//Log("  Region %d: fusOptions=0x%02X usKeyGroup=0x%04X ", pDlsIns->nRegions, rgnh.fusOptions, rgnh.usKeyGroup);
				//Log("KeyRange[%3d,%3d] ", rgnh.RangeKey.usLow, rgnh.RangeKey.usHigh);
			}
			break;

		case IFFID_wlnk:
			if (!pDlsIns->Regions.empty())
			{
				WLNKChunk wlnk;
				chunk.ReadStruct(wlnk);
				DLSREGION &region = pDlsIns->Regions.back();
				region.nWaveLink = (uint16)wlnk.ulTableIndex;
				if((region.nWaveLink < Util::MaxValueOfType(region.nWaveLink)) && (region.nWaveLink >= m_nMaxWaveLink))
					m_nMaxWaveLink = region.nWaveLink + 1;
				//Log("  WaveLink %d: fusOptions=0x%02X usPhaseGroup=0x%04X ", pDlsIns->nRegions, wlnk.fusOptions, wlnk.usPhaseGroup);
				//Log("ulChannel=%d ulTableIndex=%4d\n", wlnk.ulChannel, wlnk.ulTableIndex);
			}
			break;

		case IFFID_wsmp:
			if(!pDlsIns->Regions.empty())
			{
				DLSREGION &region = pDlsIns->Regions.back();
				WSMPChunk wsmp;
				chunk.ReadStruct(wsmp);
				region.fuOptions |= DLSREGION_OVERRIDEWSMP;
				region.uUnityNote = (uint8)wsmp.usUnityNote;
				region.sFineTune = wsmp.sFineTune;
				int32 lVolume = DLS32BitRelativeGainToLinear(wsmp.lAttenuation) / 256;
				if (lVolume > 256) lVolume = 256;
				if (lVolume < 4) lVolume = 4;
				region.usVolume = (uint16)lVolume;
				//Log("  WaveSample %d: usUnityNote=%2d sFineTune=%3d ", pDlsEnv->nRegions, p->usUnityNote, p->sFineTune);
				//Log("fulOptions=0x%04X loops=%d\n", p->fulOptions, p->cSampleLoops);
				if((wsmp.cSampleLoops) && (wsmp.cbSize + sizeof(WSMPSampleLoop) <= header.len))
				{
					WSMPSampleLoop loop;
					chunk.Seek(sizeof(IFFCHUNK) + wsmp.cbSize);
					chunk.ReadStruct(loop);
					//Log("looptype=%2d loopstart=%5d loopend=%5d\n", ploop->ulLoopType, ploop->ulLoopStart, ploop->ulLoopLength);
					if(loop.ulLoopLength > 3)
					{
						region.fuOptions |= DLSREGION_SAMPLELOOP;
						//if(loop.ulLoopType) region.fuOptions |= DLSREGION_PINGPONGLOOP;
						region.ulLoopStart = loop.ulLoopStart;
						region.ulLoopEnd = loop.ulLoopStart + loop.ulLoopLength;
					}
				}
			}
			break;

		case IFFID_art1:
		case IFFID_art2:
			{
				ART1Chunk art1;
				chunk.ReadStruct(art1);
				if(!(pDlsIns->ulBank & F_INSTRUMENT_DRUMS))
				{
					pDlsIns->nMelodicEnv = static_cast<uint32>(m_Envelopes.size() + 1);
				} else
				{
					if(!pDlsIns->Regions.empty())
						pDlsIns->Regions.back().uPercEnv = static_cast<uint32>(m_Envelopes.size() + 1);
				}
				if(art1.cbSize + art1.cConnectionBlocks * sizeof(ConnectionBlock) > header.len)
					break;
				DLSENVELOPE dlsEnv;
				//Log("  art1 (%3d bytes): cbSize=%d cConnectionBlocks=%d\n", p->len, p->cbSize, p->cConnectionBlocks);
				chunk.Seek(sizeof(IFFCHUNK) + art1.cbSize);
				for (uint32 iblk = 0; iblk < art1.cConnectionBlocks; iblk++)
				{
					ConnectionBlock blk;
					chunk.ReadStruct(blk);
					// [4-bit transform][12-bit dest][8-bit control][8-bit source] = 32-bit ID
					uint32 dwArticulation = blk.usTransform;
					dwArticulation = (dwArticulation << 12) | (blk.usDestination & 0x0FFF);
					dwArticulation = (dwArticulation << 8) | (blk.usControl & 0x00FF);
					dwArticulation = (dwArticulation << 8) | (blk.usSource & 0x00FF);
					switch(dwArticulation)
					{
					case ART_DEFAULTPAN:
						{
							int32 pan = 128 + blk.lScale / (65536000/128);
							dlsEnv.defaultPan = mpt::saturate_cast<uint8>(pan);
						}
						break;

					case ART_VOL_EG_DELAYTIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.volumeEnv.delay = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_VOL_EG_ATTACKTIME:
						// 32-bit time cents units. range = [0s, 20s]
						if(blk.lScale > -0x40000000)
						{
							int32 l = std::min(0, blk.lScale - 78743200); // maximum velocity
							dlsEnv.volumeEnv.attack = DLSEnvelopeTimeCentsToMilliseconds(l);
						}
						break;

					case ART_VOL_EG_HOLDTIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.volumeEnv.hold = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_VOL_EG_DECAYTIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.volumeEnv.decay = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_VOL_EG_RELEASETIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.volumeEnv.release = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_VOL_EG_SUSTAINLEVEL:
						// 0.1% units
						if(blk.lScale >= 0)
						{
							dlsEnv.volumeEnv.sustainLevel = DLSSustainLevelToLinear(blk.lScale);
						}
						break;

					case ART_PITCH_EG_DELAYTIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.pitchEnv.delay = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_PITCH_EG_ATTACKTIME:
						// 32-bit time cents units. range = [0s, 20s]
						if(blk.lScale > -0x40000000)
						{
							int32 l = std::min(0, blk.lScale - 78743200);  // maximum velocity
							dlsEnv.pitchEnv.attack = DLSEnvelopeTimeCentsToMilliseconds(l);
						}
						break;

					case ART_PITCH_EG_HOLDTIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.pitchEnv.hold = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_PITCH_EG_DECAYTIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.pitchEnv.decay = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_PITCH_EG_RELEASETIME:
						// 32-bit time cents units. range = [0s, 20s]
						dlsEnv.pitchEnv.release = DLSEnvelopeTimeCentsToMilliseconds(blk.lScale);
						break;

					case ART_PITCH_EG_SUSTAINLEVEL:
						// 0.1% units
						dlsEnv.pitchEnv.sustainLevel = DLSSustainLevelToLinear(blk.lScale);
						break;

					case ART_PITCH_EG_DEPTH:
						dlsEnv.pitchEnvDepth = mpt::saturate_cast<int16>(blk.lScale / 65536);
						break;

					//default:
					//	Log("    Articulation = 0x%08X value=%d\n", dwArticulation, blk.lScale);
					}
				}
				m_Envelopes.push_back(dlsEnv);
			}
			break;

		case IFFID_INAM:
			chunk.ReadString<mpt::String::spacePadded>(pDlsIns->szName, header.len);
			break;
		default:
#ifdef DLSINSTR_LOG
		{
				char sid[5]{};
				memcpy(sid, &header.id, 4);
				MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("Unsupported DLS chunk: {} ({} bytes)")(mpt::ToUnicode(mpt::Charset::ASCII, mpt::String::ReadAutoBuf(sid)), header.len.get()));
			}
#endif
			break;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////
// Converts SF2 chunks to DLS

bool CDLSBank::UpdateSF2PresetData(SF2LoaderInfo &sf2info, const IFFCHUNK &header, FileReader &chunk)
{
	if (!chunk.IsValid()) return false;
	switch(header.id)
	{
	case IFFID_phdr:
		if(m_Instruments.empty())
		{
			uint32 numIns = static_cast<uint32>(chunk.GetLength() / sizeof(SFPresetHeader));
			if(numIns <= 1)
				break;
			// The terminal sfPresetHeader record should never be accessed, and exists only to provide a terminal wPresetBagNdx with which to determine the number of zones in the last preset.
			numIns--;
			m_Instruments.resize(numIns);

		#ifdef DLSBANK_LOG
			MPT_LOG_GLOBAL(LogDebug, "DLSBank", MPT_UFORMAT("phdr: {} instruments")(m_Instruments.size()));
		#endif
			SFPresetHeader psfh;
			chunk.ReadStruct(psfh);
			for(auto &dlsIns : m_Instruments)
			{
				mpt::String::WriteAutoBuf(dlsIns.szName) = mpt::String::ReadAutoBuf(psfh.achPresetName);
				dlsIns.ulInstrument = psfh.wPreset & 0x7F;
				dlsIns.ulBank = (psfh.wBank >= 128) ? F_INSTRUMENT_DRUMS : (psfh.wBank << 8);
				dlsIns.wPresetBagNdx = psfh.wPresetBagNdx;
				dlsIns.wPresetBagNum = 1;
				chunk.ReadStruct(psfh);
				if (psfh.wPresetBagNdx > dlsIns.wPresetBagNdx) dlsIns.wPresetBagNum = static_cast<uint16>(psfh.wPresetBagNdx - dlsIns.wPresetBagNdx);
			}
		}
		break;

	case IFFID_pbag:
		if(!m_Instruments.empty() && chunk.CanRead(sizeof(SFPresetBag)))
		{
			sf2info.presetBags = chunk.GetChunk(chunk.BytesLeft());
		}
	#ifdef DLSINSTR_LOG
		else MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", U_("pbag: no instruments!"));
	#endif
		break;

	case IFFID_pgen:
		if(!m_Instruments.empty() && chunk.CanRead(sizeof(SFGenList)))
		{
			sf2info.presetGens = chunk.GetChunk(chunk.BytesLeft());
		}
	#ifdef DLSINSTR_LOG
		else MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", U_("pgen: no instruments!"));
	#endif
		break;

	case IFFID_inst:
		if(!m_Instruments.empty() && chunk.CanRead(sizeof(SFInst)))
		{
			sf2info.insts = chunk.GetChunk(chunk.BytesLeft());
		}
		break;

	case IFFID_ibag:
		if(!m_Instruments.empty() && chunk.CanRead(sizeof(SFInstBag)))
		{
			sf2info.instBags = chunk.GetChunk(chunk.BytesLeft());
		}
		break;

	case IFFID_igen:
		if(!m_Instruments.empty() && chunk.CanRead(sizeof(SFInstGenList)))
		{
			sf2info.instGens = chunk.GetChunk(chunk.BytesLeft());
		}
		break;

	case IFFID_shdr:
		if (m_SamplesEx.empty())
		{
			uint32 numSmp = static_cast<uint32>(chunk.GetLength() / sizeof(SFSample));
			if (numSmp < 1) break;
			m_SamplesEx.resize(numSmp);
			m_WaveForms.resize(numSmp);
			#ifdef DLSINSTR_LOG
				MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("shdr: {} samples")(m_SamplesEx.size()));
			#endif

			for (uint32 i = 0; i < numSmp; i++)
			{
				SFSample p;
				chunk.ReadStruct(p);
				DLSSAMPLEEX &dlsSmp = m_SamplesEx[i];
				mpt::String::WriteAutoBuf(dlsSmp.szName) = mpt::String::ReadAutoBuf(p.achSampleName);
				dlsSmp.dwLen = 0;
				dlsSmp.dwSampleRate = p.dwSampleRate;
				dlsSmp.byOriginalPitch = p.byOriginalPitch;
				dlsSmp.chPitchCorrection = static_cast<int8>(Util::muldivr(p.chPitchCorrection, 128, 100));
				// cognitone's sf2convert tool doesn't set the correct sample flags (0x01 / 0x02 instead of 0x10/ 0x20).
				// For SF3, we ignore this and go by https://github.com/FluidSynth/fluidsynth/wiki/SoundFont3Format instead
				// As cognitone's tool is the only tool writing SF4 files, we always assume compressed samples with SF4 files if bits 0/1 are set.
				uint16 sampleType = p.sfSampleType;
				if(m_sf2version >= 0x4'0000 && m_sf2version <= 0x4'FFFF && (sampleType & 0x03))
					sampleType = (sampleType & 0xFFFC) | 0x10;

				dlsSmp.compressed = (sampleType & 0x10);
				if(((sampleType & 0x7FCF) <= 4) && (p.dwEnd >= p.dwStart + 4))
				{
					m_WaveForms[i] = p.dwStart;
					dlsSmp.dwLen = (p.dwEnd - p.dwStart);
					if(!dlsSmp.compressed)
					{
						m_WaveForms[i] *= 2;
						dlsSmp.dwLen *= 2;
						if((p.dwEndloop > p.dwStartloop + 7) && (p.dwStartloop >= p.dwStart))
						{
							dlsSmp.dwStartloop = p.dwStartloop - p.dwStart;
							dlsSmp.dwEndloop = p.dwEndloop - p.dwStart;
						}
					} else
					{
						if(p.dwEndloop > p.dwStartloop + 7)
						{
							dlsSmp.dwStartloop = p.dwStartloop;
							dlsSmp.dwEndloop = p.dwEndloop;
						}
					}
				}
			}
		}
		break;

	default:
#ifdef DLSINSTR_LOG
		{
			char sdbg[5]{};
			memcpy(sdbg, &header.id, 4);
			MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("Unsupported SF2 chunk: {} ({} bytes)")(mpt::ToUnicode(mpt::Charset::ASCII, mpt::String::ReadAutoBuf(sdbg)), header.len.get()));
		}
#endif
		break;
	}
	return true;
}


// Convert all instruments to the DLS format
bool CDLSBank::ConvertSF2ToDLS(SF2LoaderInfo &sf2info)
{
	if (m_Instruments.empty() || m_SamplesEx.empty())
		return false;

	const uint32 numInsts = static_cast<uint32>(sf2info.insts.GetLength() / sizeof(SFInst));
	const uint32 numInstBags = static_cast<uint32>(sf2info.instBags.GetLength() / sizeof(SFInstBag));

	std::vector<std::pair<uint16, uint16>> instruments;  // instrument, key range
	std::vector<SFGenList> generators;
	std::vector<SFInstGenList> instrGenerators;
	for(auto &dlsIns : m_Instruments)
	{
		instruments.clear();
		DLSENVELOPE dlsEnv;
		int32 instrAttenuation = 0;
		int16 instrFinetune = 0;
		// Load Preset Bags
		sf2info.presetBags.Seek(dlsIns.wPresetBagNdx * sizeof(SFPresetBag));
		for(uint32 ipbagcnt = 0; ipbagcnt < dlsIns.wPresetBagNum; ipbagcnt++)
		{
			// Load generators for each preset bag
			SFPresetBag bag[2];
			if(!sf2info.presetBags.ReadArray(bag))
				break;
			sf2info.presetBags.SkipBack(sizeof(SFPresetBag));

			sf2info.presetGens.Seek(bag[0].wGenNdx * sizeof(SFGenList));
			uint16 keyRange = 0xFFFF;
			if(!sf2info.presetGens.ReadVector(generators, bag[1].wGenNdx - bag[0].wGenNdx))
				continue;
			for(const auto &gen : generators)
			{
				const int16 value = static_cast<int16>(gen.genAmount);
				switch(gen.sfGenOper)
				{
				case SF2_GEN_INSTRUMENT:
					if(const auto instr = std::make_pair(gen.genAmount.get(), keyRange); !mpt::contains(instruments, instr))
						instruments.push_back(instr);
					keyRange = 0xFFFF;
					break;
				case SF2_GEN_KEYRANGE:
					keyRange = gen.genAmount;
					break;
				case SF2_GEN_ATTENUATION:
					instrAttenuation = -value;
					break;
				case SF2_GEN_COARSETUNE:
					instrFinetune += value * 128;
					break;
				case SF2_GEN_FINETUNE:
					instrFinetune += static_cast<int16>(Util::muldiv(static_cast<int8>(value), 128, 100));
					break;
				default:
					if(!gen.ApplyToEnvelope(dlsEnv))
					{
#ifdef DLSINSTR_LOG
						MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("Preset {} bag {} gen {}: genoper={} amount={}{}")
							(static_cast<int>(&dlsIns - m_Instruments.data()), ipbagcnt, static_cast<int>(&gen - generators.data()), gen.sfGenOper, gen.genAmount, (dlsIns.ulBank & F_INSTRUMENT_DRUMS) ? U_(" (drum)") : U_("")));
#endif
					}
					break;
				}
			}
		}
		// Envelope
		if (!(dlsIns.ulBank & F_INSTRUMENT_DRUMS))
		{
			m_Envelopes.push_back(dlsEnv);
			dlsIns.nMelodicEnv = static_cast<uint32>(m_Envelopes.size());
		}
		// Load Instrument Bags
		dlsIns.Regions.clear();
		for(const auto & [nInstrNdx, keyRange] : instruments)
		{
			if(nInstrNdx >= numInsts)
				continue;
			sf2info.insts.Seek(nInstrNdx * sizeof(SFInst));
			SFInst insts[2];
			sf2info.insts.ReadArray(insts);
			const uint32 numRegions = insts[1].wInstBagNdx - insts[0].wInstBagNdx;
			dlsIns.Regions.reserve(dlsIns.Regions.size() + numRegions);
			//Log("\nIns %3d, %2d regions:\n", nIns, pSmp->nRegions);
			DLSREGION globalZone{};
			globalZone.uUnityNote = 0xFF;  // 0xFF means undefined -> use sample root note
			globalZone.tuning = 100;
			globalZone.sFineTune = instrFinetune;
			globalZone.nWaveLink = Util::MaxValueOfType(globalZone.nWaveLink);
			if(keyRange != 0xFFFF)
			{
				globalZone.uKeyMin = static_cast<uint8>(keyRange & 0xFF);
				globalZone.uKeyMax = static_cast<uint8>(keyRange >> 8);
				if(globalZone.uKeyMin > globalZone.uKeyMax)
					std::swap(globalZone.uKeyMin, globalZone.uKeyMax);
			} else
			{
				globalZone.uKeyMin = 0;
				globalZone.uKeyMax = 127;
			}
			for(uint32 nRgn = 0; nRgn < numRegions; nRgn++)
			{
				uint32 ibagcnt = insts[0].wInstBagNdx + nRgn;
				if(ibagcnt >= numInstBags)
					break;
				// Create a new envelope for drums
				DLSENVELOPE *pDlsEnv = &dlsEnv;
				if(!(dlsIns.ulBank & F_INSTRUMENT_DRUMS) && dlsIns.nMelodicEnv > 0 && dlsIns.nMelodicEnv <= m_Envelopes.size())
				{
					pDlsEnv = &m_Envelopes[dlsIns.nMelodicEnv - 1];
				}

				DLSREGION rgn = globalZone;

				if(dlsIns.ulBank & F_INSTRUMENT_DRUMS)
				{
					m_Envelopes.push_back(dlsEnv);
					rgn.uPercEnv = static_cast<uint32>(m_Envelopes.size());
					pDlsEnv = &m_Envelopes[rgn.uPercEnv - 1];
					if(globalZone.uPercEnv)
						*pDlsEnv = m_Envelopes[globalZone.uPercEnv - 1];
				}

				// Region Default Values
				int32 regionAttn = 0;
				// Load Generators
				sf2info.instBags.Seek(ibagcnt * sizeof(SFInstBag));
				SFInstBag bags[2];
				sf2info.instBags.ReadArray(bags);
				sf2info.instGens.Seek(bags[0].wGenNdx * sizeof(SFInstGenList));
				uint16 lastOp = SF2_GEN_SAMPLEID;
				int32 loopStart = 0, loopEnd = 0;
				if(!sf2info.instGens.ReadVector(instrGenerators, bags[1].wGenNdx - bags[0].wGenNdx))
					break;
				for(const auto &gen : instrGenerators)
				{
					uint16 value = gen.genAmount;
					lastOp = gen.sfGenOper;

					switch(gen.sfGenOper)
					{
					case SF2_GEN_KEYRANGE:
						{
							uint8 keyMin = static_cast<uint8>(value & 0xFF);
							uint8 keyMax = static_cast<uint8>(value >> 8);
							if(keyMin > keyMax)
								std::swap(keyMin, keyMax);
							rgn.uKeyMin = std::max(rgn.uKeyMin, keyMin);
							rgn.uKeyMax = std::min(rgn.uKeyMax, keyMax);
							// There was no overlap between instrument region and preset region - skip it
							if(rgn.uKeyMin > rgn.uKeyMax)
								rgn.uKeyMin = rgn.uKeyMax = 0xFF;
						}
						break;
					case SF2_GEN_UNITYNOTE:
						if (value < 128) rgn.uUnityNote = static_cast<uint8>(value);
						break;
					case SF2_GEN_PAN:
						{
							int32 pan = static_cast<int16>(value);
							pan = std::clamp(Util::muldivr(pan + 500, 256, 1000), 0, 256);
							rgn.panning = static_cast<int16>(pan);
							pDlsEnv->defaultPan = mpt::saturate_cast<uint8>(pan);
						}
						break;
					case SF2_GEN_ATTENUATION:
						regionAttn = -static_cast<int16>(value);
						break;
					case SF2_GEN_SAMPLEID:
						if (value < m_SamplesEx.size())
						{
							rgn.nWaveLink = value;
							rgn.ulLoopStart = mpt::saturate_cast<uint32>(m_SamplesEx[value].dwStartloop + loopStart);
							rgn.ulLoopEnd = mpt::saturate_cast<uint32>(m_SamplesEx[value].dwEndloop + loopEnd);
						}
						break;
					case SF2_GEN_SAMPLEMODES:
						value &= 3;
						rgn.fuOptions &= uint16(~(DLSREGION_SAMPLELOOP|DLSREGION_PINGPONGLOOP|DLSREGION_SUSTAINLOOP));
						if(value == 1)
							rgn.fuOptions |= DLSREGION_SAMPLELOOP;
						else if(value == 2)
							rgn.fuOptions |= DLSREGION_SAMPLELOOP | DLSREGION_PINGPONGLOOP;
						else if(value == 3)
							rgn.fuOptions |= DLSREGION_SAMPLELOOP | DLSREGION_SUSTAINLOOP;
						rgn.fuOptions |= DLSREGION_OVERRIDEWSMP;
						break;
					case SF2_GEN_KEYGROUP:
						rgn.fuOptions |= (value & DLSREGION_KEYGROUPMASK);
						break;
					case SF2_GEN_COARSETUNE:
						rgn.sFineTune += static_cast<int16>(value) * 128;
						break;
					case SF2_GEN_FINETUNE:
						rgn.sFineTune += static_cast<int16>(Util::muldiv(static_cast<int8>(value), 128, 100));
						break;
					case SF2_GEN_SCALE_TUNING:
						rgn.tuning = mpt::saturate_cast<uint8>(value);
						break;
					case SF2_GEN_START_LOOP_FINE:
						loopStart += static_cast<int16>(value);
						break;
					case SF2_GEN_END_LOOP_FINE:
						loopEnd += static_cast<int16>(value);
						break;
					case SF2_GEN_START_LOOP_COARSE:
						loopStart += static_cast<int16>(value) * 32768;
						break;
					case SF2_GEN_END_LOOP_COARSE:
						loopEnd += static_cast<int16>(value) * 32768;
						break;
					default:
						if(!gen.ApplyToEnvelope(*pDlsEnv))
						{
#ifdef DLSINSTR_LOG
							MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("Instr {} region {} gen {}: genoper={} amount={}{}")
								(nInstrNdx, nRgn, static_cast<int>(&gen - instrGenerators.data()), gen.sfGenOper, gen.genAmount, (dlsIns.ulBank & F_INSTRUMENT_DRUMS) ? U_(" (drum)") : U_("")));
#endif
						}
						break;
					}
				}
				int32 linearVol = DLS32BitRelativeGainToLinear(((instrAttenuation + regionAttn) * 65536) / 10) / 256;
				Limit(linearVol, 16, 256);
				rgn.usVolume = static_cast<uint16>(linearVol);

				if(lastOp != SF2_GEN_SAMPLEID && nRgn == 0)
					globalZone = rgn;
				else if(!rgn.IsDummy())
					dlsIns.Regions.push_back(rgn);
			}
		}
	}
	return true;
}


///////////////////////////////////////////////////////////////
// Open: opens a DLS bank

bool CDLSBank::Open(const mpt::PathString &filename)
{
	if(filename.empty()) return false;
	m_szFileName = filename;
	mpt::IO::InputFile f(filename, false);
	if(!f.IsValid()) return false;
	return Open(GetFileReader(f));
}


bool CDLSBank::Open(FileReader file)
{
	uint32 nInsDef;

	if(file.GetOptionalFileName())
	{
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
		m_szFileName = *(file.GetOptionalFileName());
#else
		m_szFileName = file.GetOptionalFileName().value();
#endif
	}

	file.Rewind();
	if(!file.CanRead(256))
		return false;

	RIFFChunkID riff;
	file.ReadStruct(riff);
	// Check DLS sections embedded in RMI midi files
	if(riff.id_RIFF == IFFID_RIFF && riff.id_DLS == IFFID_RMID)
	{
		while(file.ReadStruct(riff))
		{
			if(riff.id_RIFF == IFFID_RIFF && (riff.id_DLS == IFFID_DLS || riff.id_DLS == IFFID_sfbk))
				break;
			
			uint32 len = riff.riff_len;
			if((len % 2u) != 0)
				len++;
			file.SkipBack(4);
			file.Skip(len);
		}
	}

	// Check XDLS sections embedded in big endian IFF files (Miles Sound System)
	if (riff.id_RIFF == IFFID_FORM)
	{
		do
		{
			if(riff.id_DLS == IFFID_XDLS)
			{
				file.ReadStruct(riff);
				break;
			}
			uint32 len = mpt::bit_cast<uint32be>(riff.riff_len);
			if((len % 2u) != 0)
				len++;
			file.SkipBack(4);
			file.Skip(len);
		} while(file.ReadStruct(riff));
	}
	if (riff.id_RIFF != IFFID_RIFF
		|| (riff.id_DLS != IFFID_DLS && riff.id_DLS != IFFID_MLS && riff.id_DLS != IFFID_sfbk)
		|| !file.CanRead(riff.riff_len - 4))
	{
	#ifdef DLSBANK_LOG
		MPT_LOG_GLOBAL(LogDebug, "DLSBANK", U_("Invalid DLS bank!"));
	#endif
		return false;
	}
	SF2LoaderInfo sf2info;
	m_nType = (riff.id_DLS == IFFID_sfbk) ? SOUNDBANK_TYPE_SF2 : SOUNDBANK_TYPE_DLS;
	m_dwWavePoolOffset = 0;
	m_sf2version = 0;
	m_Instruments.clear();
	m_WaveForms.clear();
	m_Envelopes.clear();
	nInsDef = 0;
	bool applyPaddingToSampleChunk = true;
	while(file.CanRead(sizeof(IFFCHUNK)))
	{
		IFFCHUNK chunkHeader;
		file.ReadStruct(chunkHeader);
		const auto chunkStartPos = file.GetPosition();
		FileReader chunk = file.ReadChunk(chunkHeader.len);
		
		bool applyPadding = (chunkHeader.len % 2u) != 0;

		if(!chunk.LengthIsAtLeast(chunkHeader.len))
			break;

		switch(chunkHeader.id)
		{
		// DLS 1.0: Instruments Collection Header
		case IFFID_colh:
		#ifdef DLSBANK_LOG
			MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("colh ({} bytes)")(chunkHeader.len.get()));
		#endif
			if (m_Instruments.empty())
			{
				m_Instruments.resize(chunk.ReadUint32LE());
			#ifdef DLSBANK_LOG
				MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("  {} instruments")(m_Instruments.size()));
			#endif
			}
			break;

		// DLS 1.0: Instruments Pointers Table
		case IFFID_ptbl:
		#ifdef DLSBANK_LOG
			MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("ptbl ({} bytes)")(chunkHeader.len.get()));
		#endif
			if (m_WaveForms.empty())
			{
				PTBLChunk ptbl;
				chunk.ReadStruct(ptbl);
				chunk.Skip(ptbl.cbSize - 8);
				uint32 cues = std::min(ptbl.cCues.get(), mpt::saturate_cast<uint32>(chunk.BytesLeft() / sizeof(uint32)));
				m_WaveForms.reserve(cues);
				for(uint32 i = 0; i < cues; i++)
				{
					m_WaveForms.push_back(chunk.ReadUint32LE());
				}
			#ifdef DLSBANK_LOG
				MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("  {} waveforms")(m_WaveForms.size()));
			#endif
			}
			break;

		// DLS 1.0: LIST section
		case IFFID_LIST:
		#ifdef DLSBANK_LOG
			MPT_LOG_GLOBAL(LogDebug, "DLSBANK", U_("LIST"));
		#endif
			{
				uint32 listid = chunk.ReadUint32LE();
				if (((listid == IFFID_wvpl) && (m_nType & SOUNDBANK_TYPE_DLS))
				 || ((listid == IFFID_sdta) && (m_nType & SOUNDBANK_TYPE_SF2)))
				{
					m_dwWavePoolOffset = chunkStartPos + 4;
				#ifdef DLSBANK_LOG
					MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("Wave Pool offset: {}")(m_dwWavePoolOffset));
				#endif
					if(!applyPaddingToSampleChunk)
						applyPadding = false;
					break;
				}

				while (chunk.CanRead(12))
				{
					IFFCHUNK listHeader;
					chunk.ReadStruct(listHeader);

					if(!chunk.CanRead(listHeader.len))
						break;

					FileReader subData = chunk.GetChunkAt(chunk.GetPosition() - sizeof(IFFCHUNK), listHeader.len + 8);
					FileReader listChunk = chunk.ReadChunk(listHeader.len);
					if(listHeader.len % 2u)
						chunk.Skip(1);
					// DLS Instrument Headers
					if (listHeader.id == IFFID_LIST && (m_nType & SOUNDBANK_TYPE_DLS))
					{
						uint32 subID = listChunk.ReadUint32LE();
						if ((subID == IFFID_ins) && (nInsDef < m_Instruments.size()))
						{
							DLSINSTRUMENT &dlsIns = m_Instruments[nInsDef];
							//Log("Instrument %d:\n", nInsDef);
							dlsIns.Regions.push_back({});
							UpdateInstrumentDefinition(&dlsIns, subData);
							nInsDef++;
						}
					} else
					// DLS/SF2 Bank Information
					if (listid == IFFID_INFO && listHeader.len)
					{
						switch(listHeader.id)
						{
						case IFFID_ifil:
							m_sf2version = listChunk.ReadUint16LE() << 16;
							m_sf2version |= listChunk.ReadUint16LE();
							if(m_sf2version >= 0x3'0000 && m_sf2version <= 0x4'FFFF)
							{
								// "SF3" / "SF4" with compressed samples. The padding of the sample chunk is now optional (probably because it was simply forgotten to be added)
								applyPaddingToSampleChunk = false;
							}
							listChunk.Skip(2);
							break;
						case IFFID_INAM:
							listChunk.ReadString<mpt::String::maybeNullTerminated>(m_BankInfo.szBankName, listChunk.BytesLeft());
							break;
						case IFFID_IENG:
							listChunk.ReadString<mpt::String::maybeNullTerminated>(m_BankInfo.szEngineer, listChunk.BytesLeft());
							break;
						case IFFID_ICOP:
							listChunk.ReadString<mpt::String::maybeNullTerminated>(m_BankInfo.szCopyRight, listChunk.BytesLeft());
							break;
						case IFFID_ICMT:
							listChunk.ReadString<mpt::String::maybeNullTerminated>(m_BankInfo.szComments, listChunk.BytesLeft());
							break;
						case IFFID_ISFT:
							listChunk.ReadString<mpt::String::maybeNullTerminated>(m_BankInfo.szSoftware, listChunk.BytesLeft());
							break;
						case IFFID_ISBJ:
							listChunk.ReadString<mpt::String::maybeNullTerminated>(m_BankInfo.szDescription, listChunk.BytesLeft());
							break;
						}
					} else
					if ((listid == IFFID_pdta) && (m_nType & SOUNDBANK_TYPE_SF2))
					{
						UpdateSF2PresetData(sf2info, listHeader, listChunk);
					}
				}
			}
			break;

		#ifdef DLSBANK_LOG
		default:
			{
				char sdbg[5];
				memcpy(sdbg, &chunkHeader.id, 4);
				sdbg[4] = 0;
				MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("Unsupported chunk: {} ({} bytes)")(mpt::ToUnicode(mpt::Charset::ASCII, mpt::String::ReadAutoBuf(sdbg)), chunkHeader.len.get()));
			}
			break;
		#endif
		}

		if(applyPadding)
			file.Skip(1);
	}
	// Build the ptbl is not present in file
	if ((m_WaveForms.empty()) && (m_dwWavePoolOffset) && (m_nType & SOUNDBANK_TYPE_DLS) && (m_nMaxWaveLink > 0))
	{
	#ifdef DLSBANK_LOG
		MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("ptbl not present: building table ({} wavelinks)...")(m_nMaxWaveLink));
	#endif
		m_WaveForms.reserve(m_nMaxWaveLink);
		file.Seek(m_dwWavePoolOffset);
		while(m_WaveForms.size() < m_nMaxWaveLink && file.CanRead(sizeof(IFFCHUNK)))
		{
			IFFCHUNK chunk;
			file.ReadStruct(chunk);
			if (chunk.id == IFFID_LIST)
				m_WaveForms.push_back(file.GetPosition() - m_dwWavePoolOffset - sizeof(IFFCHUNK));
			file.Skip(chunk.len);
		}
#ifdef DLSBANK_LOG
		MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("Found {} waveforms")(m_WaveForms.size()));
#endif
	}
	// Convert the SF2 data to DLS
	if ((m_nType & SOUNDBANK_TYPE_SF2) && !m_SamplesEx.empty() && !m_Instruments.empty())
	{
		ConvertSF2ToDLS(sf2info);
	}

	// FindInstrument requires the instrument to be sorted for picking the best instrument from the MIDI library when there are multiple banks.
	// And of course this is also helpful for creating the treeview UI
	std::sort(m_Instruments.begin(), m_Instruments.end());
	// Sort regions (for drums)
	for(auto &instr : m_Instruments)
	{
		std::sort(instr.Regions.begin(), instr.Regions.end(), [](const DLSREGION &l, const DLSREGION &r)
				  { return std::tie(l.uKeyMin, l.uKeyMax) < std::tie(r.uKeyMin, r.uKeyMax); });
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
// Extracts the Waveforms from a DLS/SF2 bank

uint32 CDLSBank::GetRegionFromKey(uint32 nIns, uint32 nKey) const
{
	if(nIns >= m_Instruments.size())
		return 0;
	const DLSINSTRUMENT &dlsIns = m_Instruments[nIns];
	for(uint32 rgn = 0; rgn < static_cast<uint32>(dlsIns.Regions.size()); rgn++)
	{
		const auto &region = dlsIns.Regions[rgn];
		// Regions are sorted, if we arrived here we won't find anything in the remaining regions
		if(region.uKeyMin > nKey)
			break;
		if(nKey > region.uKeyMax)
			continue;
		if(region.nWaveLink == Util::MaxValueOfType(region.nWaveLink))
			continue;
		return rgn;
	}
	return 0;
}


std::vector<uint8> CDLSBank::ExtractWaveForm(uint32 nIns, uint32 nRgn, FileReader *file) const
{
	std::vector<uint8> waveData;

	if (nIns >= m_Instruments.size() || !m_dwWavePoolOffset)
	{
	#ifdef DLSBANK_LOG
		MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("ExtractWaveForm({}) failed: m_Instruments.size()={} m_dwWavePoolOffset={} m_WaveForms.size()={}")(nIns, m_Instruments.size(), m_dwWavePoolOffset, m_WaveForms.size()));
	#endif
		return waveData;
	}
	const DLSINSTRUMENT &dlsIns = m_Instruments[nIns];
	if(nRgn >= dlsIns.Regions.size())
	{
	#ifdef DLSBANK_LOG
		MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("invalid waveform region: nIns={} nRgn={} pSmp->nRegions={}")(nIns, nRgn, dlsIns.Regions.size()));
	#endif
		return waveData;
	}
	uint32 nWaveLink = dlsIns.Regions[nRgn].nWaveLink;
	if(nWaveLink >= m_WaveForms.size())
	{
	#ifdef DLSBANK_LOG
		MPT_LOG_GLOBAL(LogDebug, "DLSBANK", MPT_UFORMAT("Invalid wavelink id: nWaveLink={} nWaveForms={}")(nWaveLink, m_WaveForms.size()));
	#endif
		return waveData;
	}

	std::unique_ptr<mpt::IO::InputFile> inputFile;
	FileReader f;
	if(file)
	{
		f = *file;
	} else
	{
		inputFile = std::make_unique<mpt::IO::InputFile>(m_szFileName, false);
		if(!inputFile->IsValid())
			return waveData;
		f = GetFileReader(*inputFile);
	}

	auto sampleOffset = mpt::saturate_cast<FileReader::pos_type>(m_WaveForms[nWaveLink] + m_dwWavePoolOffset);
	if(f.Seek(sampleOffset))
	{
		if (m_nType & SOUNDBANK_TYPE_SF2)
		{
			if (m_SamplesEx[nWaveLink].dwLen)
			{
				if (f.Skip(8))
				{
					try
					{
						f.ReadVector(waveData, m_SamplesEx[nWaveLink].dwLen);
					} catch(mpt::out_of_memory e)
					{
						mpt::delete_out_of_memory(e);
					}
				}
			}
		} else
		{
			LISTChunk chunk;
			if(f.ReadStruct(chunk))
			{
				if((chunk.id == IFFID_LIST) && (chunk.listid == IFFID_wave) && (chunk.len > 4))
				{
					try
					{
						f.SkipBack(sizeof(chunk));
						f.ReadVector(waveData, chunk.len + sizeof(IFFCHUNK));
					} catch(mpt::out_of_memory e)
					{
						mpt::delete_out_of_memory(e);
					}
				}
			}
		}
	}
	return waveData;
}


bool CDLSBank::ExtractSample(CSoundFile &sndFile, SAMPLEINDEX nSample, uint32 nIns, uint32 nRgn, int transpose, FileReader *file) const
{
	bool ok, hasWaveform;

	if(nIns >= m_Instruments.size())
		return false;
	const DLSINSTRUMENT &dlsIns = m_Instruments[nIns];
	if(nRgn >= dlsIns.Regions.size())
		return false;
	const std::vector<uint8> waveData = ExtractWaveForm(nIns, nRgn, file);
	if(waveData.size() < 16)
		return false;
	ok = false;

	FileReader wsmpChunk;
	if (m_nType & SOUNDBANK_TYPE_SF2)
	{
		sndFile.DestroySample(nSample);
		uint32 nWaveLink = dlsIns.Regions[nRgn].nWaveLink;
		ModSample &sample = sndFile.GetSample(nSample);
		if (sndFile.m_nSamples < nSample) sndFile.m_nSamples = nSample;
		if (nWaveLink < m_SamplesEx.size())
		{
			const DLSSAMPLEEX &p = m_SamplesEx[nWaveLink];
		#ifdef DLSINSTR_LOG
			MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  SF2 WaveLink #{}: {}Hz")(nWaveLink, p.dwSampleRate));
		#endif
			sample.Initialize();

			FileReader chunk{mpt::as_span(waveData)};
			if(!p.compressed || !sndFile.ReadSampleFromFile(nSample, chunk, false, false))
			{
				sample.nLength = mpt::saturate_cast<SmpLength>(waveData.size() / 2);
				SampleIO(
					SampleIO::_16bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::signedPCM)
					.ReadSample(sample, chunk);
			}
			sample.nLoopStart = dlsIns.Regions[nRgn].ulLoopStart;
			sample.nLoopEnd = dlsIns.Regions[nRgn].ulLoopEnd;
			sample.nC5Speed = p.dwSampleRate;
			sample.RelativeTone = p.byOriginalPitch;
			sample.nFineTune = p.chPitchCorrection;
			if(p.szName[0])
				sndFile.m_szNames[nSample] = mpt::String::ReadAutoBuf(p.szName);
			else if(dlsIns.szName[0])
				sndFile.m_szNames[nSample] = mpt::String::ReadAutoBuf(dlsIns.szName);
		}
		hasWaveform = sample.HasSampleData();
	} else
	{
		FileReader wavChunk(mpt::as_span(waveData));
		hasWaveform = sndFile.ReadWAVSample(nSample, wavChunk, false, &wsmpChunk);
		if(dlsIns.szName[0])
			sndFile.m_szNames[nSample] = mpt::String::ReadAutoBuf(dlsIns.szName);
	}
	if (hasWaveform)
	{
		ModSample &sample = sndFile.GetSample(nSample);
		const DLSREGION &rgn = dlsIns.Regions[nRgn];
		sample.uFlags.reset(CHN_LOOP | CHN_PINGPONGLOOP | CHN_SUSTAINLOOP | CHN_PINGPONGSUSTAIN);
		if (rgn.fuOptions & DLSREGION_SAMPLELOOP) sample.uFlags.set(CHN_LOOP);
		if (rgn.fuOptions & DLSREGION_SUSTAINLOOP) sample.uFlags.set(CHN_SUSTAINLOOP);
		if (rgn.fuOptions & DLSREGION_PINGPONGLOOP) sample.uFlags.set(CHN_PINGPONGLOOP);
		if (sample.uFlags[CHN_LOOP | CHN_SUSTAINLOOP])
		{
			if (rgn.ulLoopEnd > rgn.ulLoopStart)
			{
				if (sample.uFlags[CHN_SUSTAINLOOP])
				{
					sample.nSustainStart = rgn.ulLoopStart;
					sample.nSustainEnd = rgn.ulLoopEnd;
				} else
				{
					sample.nLoopStart = rgn.ulLoopStart;
					sample.nLoopEnd = rgn.ulLoopEnd;
				}
			} else
			{
				sample.uFlags.reset(CHN_LOOP|CHN_SUSTAINLOOP);
			}
		}
		// WSMP chunk
		{
			uint32 usUnityNote = rgn.uUnityNote;
			int sFineTune = rgn.sFineTune;
			int lVolume = rgn.usVolume;

			WSMPChunk wsmp;
			if(!(rgn.fuOptions & DLSREGION_OVERRIDEWSMP) && wsmpChunk.IsValid() && wsmpChunk.Skip(sizeof(IFFCHUNK)) && wsmpChunk.ReadStructPartial(wsmp))
			{
				usUnityNote = wsmp.usUnityNote;
				sFineTune = wsmp.sFineTune;
				lVolume = DLS32BitRelativeGainToLinear(wsmp.lAttenuation) / 256;
				if(wsmp.cSampleLoops)
				{
					WSMPSampleLoop loop;
					wsmpChunk.Seek(sizeof(IFFCHUNK) + wsmp.cbSize);
					wsmpChunk.ReadStruct(loop);
					if(loop.ulLoopLength > 3)
					{
						sample.uFlags.set(CHN_LOOP);
						//if (loop.ulLoopType) sample.uFlags |= CHN_PINGPONGLOOP;
						sample.nLoopStart = loop.ulLoopStart;
						sample.nLoopEnd = loop.ulLoopStart + loop.ulLoopLength;
					}
				}
			} else if (m_nType & SOUNDBANK_TYPE_SF2)
			{
				usUnityNote = (usUnityNote < 0x80) ? usUnityNote : sample.RelativeTone;
				sFineTune += sample.nFineTune;
			}
		#ifdef DLSINSTR_LOG
			MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("WSMP: usUnityNote={}.{}, {}Hz (transp={})")(usUnityNote, sFineTune, sample.nC5Speed, transpose));
		#endif
			if (usUnityNote > 0x7F) usUnityNote = 60;
			int steps = (60 + transpose - usUnityNote) * 128 + sFineTune;
			sample.Transpose(steps * (1.0 / (12.0 * 128.0)));
			sample.RelativeTone = 0;

			Limit(lVolume, 16, 256);
			sample.nGlobalVol = (uint8)(lVolume / 4);	// 0-64
		}
		sample.nPan = GetPanning(nIns, nRgn);

		sample.Convert(MOD_TYPE_IT, sndFile.GetType());
		sample.PrecomputeLoops(sndFile, false);
		ok = true;
	}
	return ok;
}


static uint16 ScaleEnvelope(uint32 time, float tempoScale)
{
	return std::max(mpt::saturate_round<uint16>(time * tempoScale), uint16(1));
}


uint32 DLSENVELOPE::Envelope::ConvertToMPT(InstrumentEnvelope &mptEnv, const EnvelopeType envType, const float tempoScale, const int16 valueScale) const
{
	if(!attack && decay >= 20 * 50 && (!sustainLevel && envType != ENV_PITCH) && release >= 20 * 50)
		return uint32_max;

	const EnvelopeNode::value_t neutralValue = (envType == ENV_VOLUME) ? 0 : ENVELOPE_MID;
	
	mptEnv.dwFlags.set(ENV_ENABLED);
	mptEnv.clear();
	// Delay section
	uint16 attackStart = 0;
	if(delay)
	{
		mptEnv.push_back(0, neutralValue);
		attackStart = ScaleEnvelope(delay, tempoScale);
	}
	// Attack section
	const auto attackValue = mpt::saturate_cast<EnvelopeNode::value_t>(neutralValue + Util::muldivr(ENVELOPE_MAX, valueScale, 3200));
	if(attack)
	{
		mptEnv.push_back(attackStart, static_cast<EnvelopeNode::value_t>(attackValue / (attack / 2 + 2) + 8));  // /-----
		mptEnv.push_back(attackStart + ScaleEnvelope(attack, tempoScale), attackValue);                         // |
	} else
	{
		mptEnv.push_back(attackStart, attackValue);
	}
	// Hold section
	if(EnvelopeNode::tick_t holdTick = attackStart + ScaleEnvelope(attack + hold, tempoScale); hold > 0 && holdTick > mptEnv.back().tick)
		mptEnv.push_back(holdTick, attackValue);
	// Sustain Level
	if(sustainLevel > 0 || envType == ENV_PITCH)
	{
		if(sustainLevel < 128)
		{
			uint16 lStartTime = mptEnv.back().tick;
			int32 lSusLevel = -CDLSBank::DLS32BitRelativeLinearToGain(sustainLevel << 9) / 65536;
			int32 lDecayTime = 1;
			if(lSusLevel > 0)
			{
				lDecayTime = (lSusLevel * (int32)decay) / 960;
				for(uint32 i = 0; i < 7; i++)
				{
					int32 lFactor = 128 - (1 << i);
					if(lFactor <= sustainLevel) break;
					int32 lev = -CDLSBank::DLS32BitRelativeLinearToGain(lFactor << 9) / 65536;
					if(lev > 0)
					{
						int32 ltime = (lev * (int32)decay) / 960;
						if((ltime > 1) && (ltime < lDecayTime))
						{
							uint16 tick = lStartTime + ScaleEnvelope(ltime, tempoScale);
							if(tick > mptEnv.back().tick)
							{
								mptEnv.push_back(tick, mpt::saturate_cast<EnvelopeNode::value_t>(neutralValue + Util::muldivr(lFactor, valueScale, 6400)));
							}
						}
					}
				}
			}

			uint16 decayEnd = lStartTime + ScaleEnvelope(lDecayTime, tempoScale);
			if(decayEnd > mptEnv.back().tick)
			{
				mptEnv.push_back(decayEnd, mpt::saturate_cast<EnvelopeNode::value_t>(neutralValue + Util::muldivr(sustainLevel + 1, valueScale, 6400)));
			}
		}
		mptEnv.dwFlags.set(ENV_SUSTAIN);
	} else
	{
		mptEnv.dwFlags.set(ENV_SUSTAIN);
		mptEnv.push_back(mptEnv.back().tick + 1u, mptEnv.back().value);
	}
	mptEnv.nSustainStart = mptEnv.nSustainEnd = (uint8)(mptEnv.size() - 1);
	if(mptEnv.nSustainEnd > 0 && envType == ENV_VOLUME)
		mptEnv.nReleaseNode = mptEnv.nSustainEnd;
	// Release section
	const bool lastIsNeutral = mptEnv.back().value > 1 && mptEnv.back().value == neutralValue;
	if(release && mptEnv.back().value > 1 && !lastIsNeutral)
	{
		int32 lReleaseTime = release;
		uint16 lStartTime = mptEnv.back().tick;
		int32 lStartFactor = mptEnv.back().value;
		int32 lSusLevel = -CDLSBank::DLS32BitRelativeLinearToGain(lStartFactor << 10) / 65536;
		int32 lDecayEndTime = (lReleaseTime * lSusLevel) / 960;
		lReleaseTime -= lDecayEndTime;
		int32 prevFactor = lStartFactor;
		for(uint32 i = 0; i < 7; i++)
		{
			int32 lFactor = 1 + ((lStartFactor * 3) >> (i + 2));
			if((lFactor < 1) ||(lFactor == 1 && prevFactor == 1) || (lFactor >= lStartFactor))
				continue;
			prevFactor = lFactor;
			int32 lev = -CDLSBank::DLS32BitRelativeLinearToGain(lFactor << 10) / 65536;
			if(lev > 0)
			{
				int32 ltime = (((int32)release * lev) / 960) - lDecayEndTime;
				if((ltime > 1) && (ltime < lReleaseTime))
				{
					uint16 tick = lStartTime + ScaleEnvelope(ltime, tempoScale);
					if(tick > mptEnv.back().tick)
					{
						mptEnv.push_back(tick, mpt::saturate_cast<EnvelopeNode::value_t>(neutralValue + Util::muldivr(lFactor, valueScale, 3200)));
						if(mptEnv.back().value ==  neutralValue)
							break;
					}
				}
			}
		}
		if(lReleaseTime < 1) lReleaseTime = 1;
		auto releaseTicks = ScaleEnvelope(lReleaseTime, tempoScale);
		mptEnv.push_back(lStartTime + releaseTicks, neutralValue);
		if(releaseTicks > 0)
		{
			return 32768 / releaseTicks;
		}
	} else if(!lastIsNeutral)
	{
		mptEnv.push_back(mptEnv.back().tick + 1u, neutralValue);
	}
	return uint32_max;
}


bool CDLSBank::ExtractInstrument(CSoundFile &sndFile, INSTRUMENTINDEX nInstr, uint32 nIns, uint32 nDrumRgn, FileReader *file) const
{
	uint32 minRegion, maxRegion, nEnv;

	if (nIns >= m_Instruments.size())
		return false;
	const DLSINSTRUMENT &dlsIns = m_Instruments[nIns];
	const bool isDrum = (dlsIns.ulBank & F_INSTRUMENT_DRUMS) && nDrumRgn != uint32_max;
	if(isDrum)
	{
		if(nDrumRgn >= dlsIns.Regions.size())
			return false;
		minRegion = nDrumRgn;
		maxRegion = nDrumRgn + 1;
		nEnv = dlsIns.Regions[nDrumRgn].uPercEnv;
	} else
	{
		if(dlsIns.Regions.empty())
			return false;
		minRegion = 0;
		maxRegion = static_cast<uint32>(dlsIns.Regions.size());
		nEnv = dlsIns.nMelodicEnv;
	}
#ifdef DLSINSTR_LOG
	MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("DLS Instrument #{}: {}")(nIns, mpt::ToUnicode(mpt::Charset::ASCII, mpt::String::ReadAutoBuf(dlsIns.szName))));
	MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  Bank=0x{} Instrument=0x{}")(mpt::ufmt::HEX0<4>(dlsIns.ulBank), mpt::ufmt::HEX0<4>(dlsIns.ulInstrument)));
	MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  {} regions, nMelodicEnv={}")(dlsIns.Regions.size(), dlsIns.nMelodicEnv));
	for (uint32 iDbg=0; iDbg<dlsIns.Regions.size(); iDbg++)
	{
		const DLSREGION *prgn = &dlsIns.Regions[iDbg];
		MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT(" Region {}:")(iDbg));
		MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  WaveLink = {} (loop [{}, {}])")(prgn->nWaveLink, prgn->ulLoopStart, prgn->ulLoopEnd));
		MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  Key Range: [{}, {}]")(prgn->uKeyMin, prgn->uKeyMax));
		MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  fuOptions = 0x{}")(mpt::ufmt::HEX0<4>(prgn->fuOptions)));
		MPT_LOG_GLOBAL(LogDebug, "DLSINSTR", MPT_UFORMAT("  usVolume = {}, Unity Note = {}")(prgn->usVolume, prgn->uUnityNote));
	}
#endif

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	if(sndFile.Instruments[nInstr])
	{
		sndFile.DestroyInstrument(nInstr, deleteAssociatedSamples);
	}
	// Initializes Instrument
	if(isDrum)
	{
		uint32 key = dlsIns.Regions[nDrumRgn].uKeyMin;
		if((key >= 24) && (key <= 84))
		{
			std::string s = szMidiPercussionNames[key-24];
			if(!mpt::String::ReadAutoBuf(dlsIns.szName).empty())
			{
				s += MPT_AFORMAT(" ({})")(mpt::trim_right<std::string>(mpt::String::ReadAutoBuf(dlsIns.szName)));
			}
			pIns->name = s;
		} else
		{
			pIns->name = mpt::String::ReadAutoBuf(dlsIns.szName);
		}
	} else
	{
		pIns->name = mpt::String::ReadAutoBuf(dlsIns.szName);
	}
	int transpose = 0;
	if(isDrum)
	{
		for(uint32 iNoteMap = 0; iNoteMap < NOTE_MAX; iNoteMap++)
		{
			if(sndFile.GetType() & (MOD_TYPE_IT | MOD_TYPE_MID | MOD_TYPE_MPT))
			{
				// Format has instrument note mapping
				if(dlsIns.Regions[nDrumRgn].tuning == 0)
					pIns->NoteMap[iNoteMap] = NOTE_MIDDLEC;
				else if (iNoteMap < dlsIns.Regions[nDrumRgn].uKeyMin)
					pIns->NoteMap[iNoteMap] = (uint8)(dlsIns.Regions[nDrumRgn].uKeyMin + NOTE_MIN);
				else if(iNoteMap > dlsIns.Regions[nDrumRgn].uKeyMax)
					pIns->NoteMap[iNoteMap] = (uint8)(dlsIns.Regions[nDrumRgn].uKeyMax + NOTE_MIN);
			} else
			{
				if(iNoteMap == dlsIns.Regions[nDrumRgn].uKeyMin)
				{
					transpose = (dlsIns.Regions[nDrumRgn].uKeyMin + (dlsIns.Regions[nDrumRgn].uKeyMax - dlsIns.Regions[nDrumRgn].uKeyMin) / 2) - 60;
				}
			}
		}
	}
	pIns->nFadeOut = 1024;
	pIns->nMidiProgram = static_cast<uint8>(1 + (dlsIns.ulInstrument & 0x7F));
	pIns->nMidiChannel = static_cast<uint8>(isDrum ? 10 : 0);
	pIns->wMidiBank = static_cast<uint16>(1 + (((dlsIns.ulBank & 0x7F00) >> 1) | (dlsIns.ulBank & 0x7F)));
	pIns->nNNA = NewNoteAction::NoteOff;
	pIns->nDCT = DuplicateCheckType::Note;
	pIns->nDNA = DuplicateNoteAction::NoteFade;
	sndFile.Instruments[nInstr] = pIns;
	uint32 nLoadedSmp = 0;
	SAMPLEINDEX nextSample = 0;
	// Extract Samples
	std::vector<SAMPLEINDEX> RgnToSmp(dlsIns.Regions.size());
	std::set<uint16> extractedSamples;
	for(uint32 nRgn = minRegion; nRgn < maxRegion; nRgn++)
	{
		bool duplicateRegion = false;
		SAMPLEINDEX nSmp = 0;
		const DLSREGION &rgn = dlsIns.Regions[nRgn];
		if(rgn.IsDummy())
			continue;
		// Eliminate Duplicate Regions
		uint32 dupRegion;
		for(dupRegion = minRegion; dupRegion < nRgn; dupRegion++)
		{
			const DLSREGION &rgn2 = dlsIns.Regions[dupRegion];
			if(RgnToSmp[dupRegion] == 0 || rgn2.IsDummy())
				continue;
			// No need to extract the same sample data twice
			const bool sameSample = (rgn2.nWaveLink == rgn.nWaveLink) && (rgn2.ulLoopEnd == rgn.ulLoopEnd) && (rgn2.ulLoopStart == rgn.ulLoopStart) && extractedSamples.count(rgn.nWaveLink);
			// Candidate for stereo sample creation
			const bool sameKeyRange = (rgn2.uKeyMin == rgn.uKeyMin) && (rgn2.uKeyMax == rgn.uKeyMax);
			if(sameSample || sameKeyRange)
			{
				duplicateRegion = true;
				if(!sameKeyRange)
					nSmp = RgnToSmp[dupRegion];
				break;
			}
		}
		// Create a new sample
		if (!duplicateRegion)
		{
			uint32 nmaxsmp = (m_nType & MOD_TYPE_XM) ? 16 : (NOTE_MAX - NOTE_MIN + 1);
			if (nLoadedSmp >= nmaxsmp)
			{
				nSmp = RgnToSmp[nRgn - 1];
			} else
			{
				nextSample = sndFile.GetNextFreeSample(nInstr, nextSample + 1);
				if (nextSample == SAMPLEINDEX_INVALID) break;
				if (nextSample > sndFile.GetNumSamples()) sndFile.m_nSamples = nextSample;
				nSmp = nextSample;
				nLoadedSmp++;
			}
		}

		RgnToSmp[nRgn] = nSmp;
		// Map all notes to the right sample
		if(nSmp)
		{
			for(uint8 key = 0; key < NOTE_MAX; key++)
			{
				if(isDrum || (key >= rgn.uKeyMin && key <= rgn.uKeyMax))
				{
					pIns->Keyboard[key] = nSmp;
				}
			}
			// Load the sample
			if(!duplicateRegion || !sndFile.GetSample(nSmp).HasSampleData())
			{
				ExtractSample(sndFile, nSmp, nIns, nRgn, transpose, file);
				extractedSamples.insert(rgn.nWaveLink);
			}
		} else if(duplicateRegion && sndFile.GetSample(RgnToSmp[dupRegion]).GetNumChannels() == 1)
		{
			// Try to combine stereo samples
			const uint16 pan1 = GetPanning(nIns, nRgn), pan2 = GetPanning(nIns, dupRegion);
			if((pan1 < 16 && pan2 >= 240) || (pan2 < 16 && pan1 >= 240))
			{
				ModSample &sample = sndFile.GetSample(RgnToSmp[dupRegion]);
				ModSample sampleCopy = sample;
				sampleCopy.pData.pSample = nullptr;
				sampleCopy.uFlags.set(CHN_16BIT | CHN_STEREO);
				if(!sampleCopy.AllocateSample())
					continue;

				const uint8 offsetOrig = (pan1 < pan2) ? 1 : 0;
				const uint8 offsetNew = (pan1 < pan2) ? 0 : 1;

				const std::vector<uint8> waveData = ExtractWaveForm(nIns, nRgn, file);
				if(waveData.empty())
					continue;
				extractedSamples.insert(rgn.nWaveLink);

				// First copy over original channel
				auto pDest = sampleCopy.sample16() + offsetOrig;
				if(sample.uFlags[CHN_16BIT])
					CopySample<SC::ConversionChain<SC::Convert<int16, int16>, SC::DecodeIdentity<int16>>>(pDest, sample.nLength, 2, sample.sample16(), sample.GetSampleSizeInBytes(), 1);
				else
					CopySample<SC::ConversionChain<SC::Convert<int16, int8>, SC::DecodeIdentity<int8>>>(pDest, sample.nLength, 2, sample.sample8(), sample.GetSampleSizeInBytes(), 1);
				sample.FreeSample();

				// Now read the other channel
				if(m_SamplesEx[m_Instruments[nIns].Regions[nRgn].nWaveLink].compressed)
				{
					FileReader smpChunk{mpt::as_span(waveData)};
					if(sndFile.ReadSampleFromFile(nSmp, smpChunk, false, false))
					{
						pDest = sampleCopy.sample16() + offsetNew;
						const SmpLength copyLength = std::min(sample.nLength, sampleCopy.nLength);
						if(sample.uFlags[CHN_16BIT])
							CopySample<SC::ConversionChain<SC::Convert<int16, int16>, SC::DecodeIdentity<int16>>>(pDest, copyLength, 2, sample.sample16(), sample.GetSampleSizeInBytes(), sample.GetNumChannels());
						else
							CopySample<SC::ConversionChain<SC::Convert<int16, int8>, SC::DecodeIdentity<int8>>>(pDest, copyLength, 2, sample.sample8(), sample.GetSampleSizeInBytes(), sample.GetNumChannels());
					}
				} else
				{
					SmpLength len = std::min(mpt::saturate_cast<SmpLength>(waveData.size() / 2u), sampleCopy.nLength);
					const std::byte *src = mpt::byte_cast<const std::byte *>(waveData.data());
					int16 *dst = sampleCopy.sample16() + offsetNew;
					CopySample<SC::ConversionChain<SC::Convert<int16, int16>, SC::DecodeInt16<0, littleEndian16>>>(dst, len, 2, src, waveData.size(), 1);
				}
				sample.FreeSample();
				sample = sampleCopy;
			}
		}
	}

	float tempoScale = 1.0f;
	if(sndFile.m_nTempoMode == TempoMode::Modern)
	{
		uint32 ticksPerBeat = sndFile.m_nDefaultRowsPerBeat * sndFile.Order().GetDefaultSpeed();
		if(ticksPerBeat != 0)
			tempoScale = ticksPerBeat / 24.0f;
	}

	// Initializes Envelope
	if ((nEnv) && (nEnv <= m_Envelopes.size()))
	{
		const DLSENVELOPE &part = m_Envelopes[nEnv - 1];
		if(const auto fadeout = part.volumeEnv.ConvertToMPT(pIns->VolEnv, ENV_VOLUME, tempoScale, 3200); fadeout != uint32_max)
			pIns->nFadeOut = fadeout;
		if(std::abs(part.pitchEnvDepth) >= 50)
			part.pitchEnv.ConvertToMPT(pIns->PitchEnv, ENV_PITCH, tempoScale, part.pitchEnvDepth);
	}
	if(isDrum)
	{
		// Create a default envelope for drums
		if(!pIns->VolEnv.dwFlags[ENV_ENABLED])
		{
			pIns->VolEnv.dwFlags.set(ENV_ENABLED);
			pIns->VolEnv.dwFlags.reset(ENV_SUSTAIN);
			pIns->VolEnv.resize(4);
			pIns->VolEnv[0] = EnvelopeNode(0, ENVELOPE_MAX);
			pIns->VolEnv[1] = EnvelopeNode(ScaleEnvelope(5, tempoScale), ENVELOPE_MAX);
			pIns->VolEnv[2] = EnvelopeNode(pIns->VolEnv[1].tick * 2u, ENVELOPE_MID);
			pIns->VolEnv[3] = EnvelopeNode(pIns->VolEnv[2].tick * 2u, ENVELOPE_MIN);	// 1 second max. for drums
		}
	}
	pIns->Sanitize(MOD_TYPE_MPT);
	pIns->Convert(MOD_TYPE_MPT, sndFile.GetType());
	return true;
}


const char *CDLSBank::GetRegionName(uint32 nIns, uint32 nRgn) const
{
	if(nIns >= m_Instruments.size())
		return nullptr;
	const DLSINSTRUMENT &dlsIns = m_Instruments[nIns];
	if(nRgn >= dlsIns.Regions.size())
		return nullptr;

	if (m_nType & SOUNDBANK_TYPE_SF2)
	{
		uint32 nWaveLink = dlsIns.Regions[nRgn].nWaveLink;
		if (nWaveLink < m_SamplesEx.size())
		{
			return m_SamplesEx[nWaveLink].szName;
		}
	}
	return nullptr;
}


uint16 CDLSBank::GetPanning(uint32 ins, uint32 region) const
{
	const DLSINSTRUMENT &dlsIns = m_Instruments[ins];
	if(region >= std::size(dlsIns.Regions))
		return 128;
	const DLSREGION &rgn = dlsIns.Regions[region];
	if(rgn.panning >= 0)
		return static_cast<uint16>(rgn.panning);

	if(dlsIns.ulBank & F_INSTRUMENT_DRUMS)
	{
		if(rgn.uPercEnv > 0 && rgn.uPercEnv <= m_Envelopes.size())
		{
			return m_Envelopes[rgn.uPercEnv - 1].defaultPan;
		}
	} else
	{
		if(dlsIns.nMelodicEnv > 0 && dlsIns.nMelodicEnv <= m_Envelopes.size())
		{
			return m_Envelopes[dlsIns.nMelodicEnv - 1].defaultPan;
		}
	}
	return 128;
}


#else // !MODPLUG_TRACKER

MPT_MSVC_WORKAROUND_LNK4221(Dlsbank)

#endif // MODPLUG_TRACKER


OPENMPT_NAMESPACE_END
