/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/uuid/uuid.hpp"

#include "openmpt/base/Endian.hpp"
#include "openmpt/fileformat_base/magic.hpp"

#include <cstddef>



OPENMPT_NAMESPACE_BEGIN



// RIFF header
struct RIFFHeader
{
	// 32-Bit chunk identifiers
	enum RIFFMagic
	{
		idRIFF = MagicLE("RIFF"),  // magic for WAV files
		idLIST = MagicLE("LIST"),  // magic for samples in DLS banks
		idWAVE = MagicLE("WAVE"),  // type for WAV files
		idwave = MagicLE("wave"),  // type for samples in DLS banks
	};

	uint32le magic;   // RIFF (in WAV files) or LIST (in DLS banks)
	uint32le length;  // Size of the file, not including magic and length
	uint32le type;    // WAVE (in WAV files) or wave (in DLS banks)
};

MPT_BINARY_STRUCT(RIFFHeader, 12)


// General RIFF Chunk header
struct RIFFChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idfmt_ = MagicLE("fmt "),  // Sample format information
		iddata = MagicLE("data"),  // Sample data
		idpcm_ = MagicLE("pcm "),  // IMA ADPCM samples
		idfact = MagicLE("fact"),  // Compressed samples
		idsmpl = MagicLE("smpl"),  // Sampler and loop information
		idinst = MagicLE("inst"),  // Instrument information
		idLIST = MagicLE("LIST"),  // List of chunks
		idxtra = MagicLE("xtra"),  // OpenMPT extra infomration
		idcue_ = MagicLE("cue "),  // Cue points
		idwsmp = MagicLE("wsmp"),  // DLS bank samples
		idCSET = MagicLE("CSET"),  // Character Set
		id____ = 0x00000000,       // Found when loading buggy MPT samples

		// Identifiers in "LIST" chunk
		idINAM = MagicLE("INAM"),  // title
		idISFT = MagicLE("ISFT"),  // software
		idICOP = MagicLE("ICOP"),  // copyright
		idIART = MagicLE("IART"),  // artist
		idIPRD = MagicLE("IPRD"),  // product (album)
		idICMT = MagicLE("ICMT"),  // comment
		idIENG = MagicLE("IENG"),  // engineer
		idISBJ = MagicLE("ISBJ"),  // subject
		idIGNR = MagicLE("IGNR"),  // genre
		idICRD = MagicLE("ICRD"),  // date created

		idYEAR = MagicLE("YEAR"),  // year
		idTRCK = MagicLE("TRCK"),  // track number
		idTURL = MagicLE("TURL"),  // url
	};

	uint32le id;      // See ChunkIdentifiers
	uint32le length;  // Chunk size without header

	std::size_t GetLength() const
	{
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(RIFFChunk, 8)


// Format Chunk
struct WAVFormatChunk
{
	// Sample formats
	enum SampleFormats
	{
		fmtPCM = 1,
		fmtFloat = 3,
		fmtALaw = 6,
		fmtULaw = 7,
		fmtIMA_ADPCM = 17,
		fmtMP3 = 85,
		fmtExtensible = 0xFFFE,
	};

	uint16le format;         // Sample format, see SampleFormats
	uint16le numChannels;    // Number of audio channels
	uint32le sampleRate;     // Sample rate in Hz
	uint32le byteRate;       // Bytes per second (should be freqHz * blockAlign)
	uint16le blockAlign;     // Size of a sample, in bytes (do not trust this value, it's incorrect in some files)
	uint16le bitsPerSample;  // Bits per sample
};

MPT_BINARY_STRUCT(WAVFormatChunk, 16)


// Extension of the WAVFormatChunk structure, used if format == formatExtensible
struct WAVFormatChunkExtension
{
	uint16le size;
	uint16le validBitsPerSample;
	uint32le channelMask;
	mpt::GUIDms subFormat;
};

MPT_BINARY_STRUCT(WAVFormatChunkExtension, 24)


// Sample cue point structure for the "cue " chunk
struct WAVCuePoint
{
	uint32le id;           // Unique identification value
	uint32le position;     // Play order position
	uint32le riffChunkID;  // RIFF ID of corresponding data chunk
	uint32le chunkStart;   // Byte Offset of Data Chunk
	uint32le blockStart;   // Byte Offset to sample of First Channel
	uint32le offset;       // Byte Offset to sample byte of First Channel
};

MPT_BINARY_STRUCT(WAVCuePoint, 24)



OPENMPT_NAMESPACE_END
