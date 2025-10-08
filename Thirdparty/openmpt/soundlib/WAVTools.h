/*
 * WAVTools.h
 * ----------
 * Purpose: Definition of WAV file structures and helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io/io_virtual_wrapper.hpp"
#endif

#include "openmpt/soundfile_data/wav.hpp"
#ifndef MODPLUG_NO_FILESAVE
#include "openmpt/soundfile_write/wav_write.hpp"
#endif

#include "../common/FileReader.h"

#include "Loaders.h"


OPENMPT_NAMESPACE_BEGIN

struct FileTags;

// Sample information chunk
struct WAVSampleInfoChunk
{
	uint32le manufacturer;
	uint32le product;
	uint32le samplePeriod;	// 1000000000 / sampleRate
	uint32le baseNote;		// MIDI base note of sample
	uint32le pitchFraction;
	uint32le SMPTEFormat;
	uint32le SMPTEOffset;
	uint32le numLoops;		// number of loops
	uint32le samplerData;

	// Set up information
	void ConvertToWAV(uint32 freq, uint8 rootNote)
	{
		manufacturer = 0;
		product = 0;
		samplePeriod = 1000000000 / freq;
		if(rootNote != 0)
			baseNote = rootNote - NOTE_MIN;
		else
			baseNote = NOTE_MIDDLEC - NOTE_MIN;
		pitchFraction = 0;
		SMPTEFormat = 0;
		SMPTEOffset = 0;
		numLoops = 0;
		samplerData = 0;
	}
};

MPT_BINARY_STRUCT(WAVSampleInfoChunk, 36)


// Sample loop information chunk (found after WAVSampleInfoChunk in "smpl" chunk)
struct WAVSampleLoop
{
	// Sample Loop Types
	enum LoopType
	{
		loopForward		= 0,
		loopBidi		= 1,
		loopBackward	= 2,
	};

	uint32le identifier;
	uint32le loopType;		// See LoopType
	uint32le loopStart;		// Loop start in samples
	uint32le loopEnd;		// Loop end in samples
	uint32le fraction;
	uint32le playCount;		// Loop Count, 0 = infinite

	// Apply WAV loop information to a mod sample.
	void ApplyToSample(SmpLength &start, SmpLength &end, SmpLength sampleLength, SampleFlags &flags, ChannelFlags enableFlag, ChannelFlags bidiFlag, bool mptLoopFix) const;

	// Convert internal loop information into a WAV loop.
	void ConvertToWAV(SmpLength start, SmpLength end, bool bidi);
};

MPT_BINARY_STRUCT(WAVSampleLoop, 24)


// Instrument information chunk
struct WAVInstrumentChunk
{
	uint8 unshiftedNote;	// Root key of sample, 0...127
	int8  finetune;			// Finetune of root key in cents
	int8  gain;				// in dB
	uint8 lowNote;			// Note range, 0...127
	uint8 highNote;
	uint8 lowVelocity;		// Velocity range, 0...127
	uint8 highVelocity;
};

MPT_BINARY_STRUCT(WAVInstrumentChunk, 7)


// MPT-specific "xtra" chunk
struct WAVExtraChunk
{
	enum Flags
	{
		setPanning	= 0x20,
	};

	uint32le flags;
	uint16le defaultPan;
	uint16le defaultVolume;
	uint16le globalVolume;
	uint16le reserved;
	uint8le  vibratoType;
	uint8le  vibratoSweep;
	uint8le  vibratoDepth;
	uint8le  vibratoRate;

	// Set up sample information
	void ConvertToWAV(const ModSample &sample, MODTYPE modType)
	{
		if(sample.uFlags[CHN_PANNING])
		{
			flags = WAVExtraChunk::setPanning;
		} else
		{
			flags = 0;
		}

		defaultPan = sample.nPan;
		defaultVolume = sample.nVolume;
		globalVolume = sample.nGlobalVol;
		vibratoType = sample.nVibType;
		vibratoSweep = sample.nVibSweep;
		vibratoDepth = sample.nVibDepth;
		vibratoRate = sample.nVibRate;

		if((modType & MOD_TYPE_XM) && (vibratoDepth | vibratoRate))
		{
			// XM vibrato is upside down
			vibratoSweep = 255 - vibratoSweep;
		}
	}
};

MPT_BINARY_STRUCT(WAVExtraChunk, 16)


// Set up sample information
inline WAVCuePoint ConvertToWAVCuePoint(uint32 id_, SmpLength offset_)
{
	WAVCuePoint result{};
	result.id = id_;
	result.position = offset_;
	result.riffChunkID = static_cast<uint32>(RIFFChunk::iddata);
	result.chunkStart = 0;	// we use no Wave List Chunk (wavl) as we have only one data block, so this should be 0.
	result.blockStart = 0;	// ditto
	result.offset = offset_;
	return result;
}


class WAVReader
{
protected:
	FileReader file;
	FileReader sampleData, smplChunk, instChunk, xtraChunk, wsmpChunk, cueChunk;
	FileReader::ChunkList<RIFFChunk> infoChunk;

	FileReader::pos_type sampleLength;
	WAVFormatChunk formatInfo;
	uint16 subFormat;
	uint16 codePage;
	bool isDLS;
	bool mayBeCoolEdit16_8;

	uint16 GetFileCodePage(FileReader::ChunkList<RIFFChunk> &chunks);

public:
	WAVReader(FileReader &inputFile);

	bool IsValid() const { return sampleData.IsValid(); }

	void FindMetadataChunks(FileReader::ChunkList<RIFFChunk> &chunks);

	// Self-explanatory getters.
	WAVFormatChunk::SampleFormats GetSampleFormat() const { return IsExtensibleFormat() ? static_cast<WAVFormatChunk::SampleFormats>(subFormat) : static_cast<WAVFormatChunk::SampleFormats>(formatInfo.format.get()); }
	uint16 GetNumChannels() const { return formatInfo.numChannels; }
	uint16 GetBitsPerSample() const { return formatInfo.bitsPerSample; }
	uint32 GetSampleRate() const { return formatInfo.sampleRate; }
	uint16 GetBlockAlign() const { return formatInfo.blockAlign; }
	FileReader GetSampleData() const { return sampleData; }
	FileReader GetWsmpChunk() const { return wsmpChunk; }
	bool IsExtensibleFormat() const { return formatInfo.format == WAVFormatChunk::fmtExtensible; }
	bool MayBeCoolEdit16_8() const { return mayBeCoolEdit16_8; }

	// Get size of a single sample point, in bytes.
	uint16 GetSampleSize() const { return static_cast<uint16>(((static_cast<uint32>(GetNumChannels()) * static_cast<uint32>(GetBitsPerSample())) + 7) / 8); }

	// Get sample length (in samples)
	SmpLength GetSampleLength() const { return mpt::saturate_cast<SmpLength>(sampleLength); }

	// Apply sample settings from file (loop points, MPT extra settings, ...) to a sample.
	void ApplySampleSettings(ModSample &sample, mpt::Charset sampleCharset, mpt::charbuf<MAX_SAMPLENAME> &sampleName);
};


#ifndef MODPLUG_NO_FILESAVE


class WAVSampleWriter
	: public WAVWriter
{

public:
	WAVSampleWriter(mpt::IO::OFileBase &stream);
	~WAVSampleWriter();

public:
	// Write a sample loop information chunk to the file.
	void WriteLoopInformation(const ModSample &sample, SmpLength rangeStart = 0, SmpLength rangeEnd = 0);
	// Write a sample's cue points to the file.
	void WriteCueInformation(const ModSample &sample, SmpLength rangeStart = 0, SmpLength rangeEnd = 0);
	// Write MPT's sample information chunk to the file.
	void WriteExtraInformation(const ModSample &sample, MODTYPE modType, const char *sampleName = nullptr);
};

#endif // MODPLUG_NO_FILESAVE

OPENMPT_NAMESPACE_END
