/*
 * SampleIO.h
 * ----------
 * Purpose: Central code for reading and writing samples. Create your SampleIO object and have a go at the ReadSample and WriteSample functions!
 * Notes  : Not all combinations of possible sample format combinations are implemented, especially for WriteSample.
 *          Using the existing generic sample conversion functors in SampleFormatConverters.h, it should be quite easy to extend the code, though.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"
#include "../common/FileReaderFwd.h"


OPENMPT_NAMESPACE_BEGIN


struct ModSample;

// Sample import / export formats
class SampleIO
{
public:
	// Bits per sample
	enum Bitdepth : uint8
	{
		_8bit	= 8,
		_16bit	= 16,
		_24bit	= 24,
		_32bit	= 32,
		_64bit	= 64,
	};

	// Number of channels + channel format
	enum Channels : uint8
	{
		mono = 1,
		stereoInterleaved,	// LRLRLR...
		stereoSplit,		// LLL...RRR...
	};

	// Sample byte order
	enum Endianness : uint8
	{
		littleEndian = 0,
		bigEndian = 1,
	};

	// Sample encoding
	enum Encoding : uint8
	{
		signedPCM = 0,      // Integer PCM, signed
		unsignedPCM,        // Integer PCM, unsigned
		deltaPCM,           // Integer PCM, delta-encoded
		floatPCM,           // Floating point PCM
		IT214,              // Impulse Tracker 2.14 compressed
		IT215,              // Impulse Tracker 2.15 compressed
		AMS,                // AMS / Velvet Studio packed
		DMF,                // DMF Huffman compression
		MDL,                // MDL Huffman compression
		PTM8Dto16,          // PTM 8-Bit delta value -> 16-Bit sample
		ADPCM,              // 4-Bit ADPCM-packed
		MT2,                // MadTracker 2 stereo delta encoding
		floatPCM15,         // Floating point PCM with 2^15 full scale
		floatPCM23,         // Floating point PCM with 2^23 full scale
		floatPCMnormalize,  // Floating point PCM and data will be normalized while reading
		signedPCMnormalize, // Integer PCM and data will be normalized while reading
		uLaw,               // 8-to-16 bit G.711 u-law compression
		aLaw,               // 8-to-16 bit G.711 a-law compression
	};

protected:
	Bitdepth m_bitdepth;
	Channels m_channels;
	Endianness m_endianness;
	Encoding m_encoding;

public:
	constexpr SampleIO(Bitdepth bits = _8bit, Channels channels = mono, Endianness endianness = littleEndian, Encoding encoding = signedPCM)
		: m_bitdepth(bits), m_channels(channels), m_endianness(endianness), m_encoding(encoding)
	{ }

	bool operator== (const SampleIO &other) const
	{
		return memcmp(this, &other, sizeof(*this)) == 0;
	}

	bool operator!= (const SampleIO &other) const
	{
		return memcmp(this, &other, sizeof(*this)) != 0;
	}

	void operator|= (Bitdepth bits)
	{
		m_bitdepth = bits;
	}

	void operator|= (Channels channels)
	{
		m_channels = channels;
	}

	void operator|= (Endianness endianness)
	{
		m_endianness = endianness;
	}

	void operator|= (Encoding encoding)
	{
		m_encoding = encoding;
	}

	void MayNormalize()
	{
		if(GetBitDepth() >= 24)
		{
			if(GetEncoding() == SampleIO::signedPCM)
			{
				m_encoding = SampleIO::signedPCMnormalize;
			} else if(GetEncoding() == SampleIO::floatPCM)
			{
				m_encoding = SampleIO::floatPCMnormalize;
			}
		}
	}

	// Return 0 in case of variable-length encoded samples.
	MPT_CONSTEXPRINLINE uint8 GetEncodedBitsPerSample() const
	{
		switch(GetEncoding())
		{
			case signedPCM:          // Integer PCM, signed
			case unsignedPCM:        //Integer PCM, unsigned
			case deltaPCM:           // Integer PCM, delta-encoded
			case floatPCM:           // Floating point PCM
			case MT2:                // MadTracker 2 stereo delta encoding
			case floatPCM15:         // Floating point PCM with 2^15 full scale
			case floatPCM23:         // Floating point PCM with 2^23 full scale
			case floatPCMnormalize:  // Floating point PCM and data will be normalized while reading
			case signedPCMnormalize: // Integer PCM and data will be normalized while reading
				return GetBitDepth();

			case IT214:   // Impulse Tracker 2.14 compressed
			case IT215:   // Impulse Tracker 2.15 compressed
			case AMS:     // AMS / Velvet Studio packed
			case DMF:     // DMF Huffman compression
			case MDL:     // MDL Huffman compression
				return 0; // variable-length compressed

			case PTM8Dto16: // PTM 8-Bit delta value -> 16-Bit sample
				return 16;
			case ADPCM:     // 4-Bit ADPCM-packed
				return 4;
			case uLaw:      // G.711 u-law
				return 8;
			case aLaw:      // G.711 a-law
				return 8;

			default:
				return 0;
		}
	}

	// Return the static header size additional to the raw encoded sample data.
	MPT_CONSTEXPRINLINE std::size_t GetEncodedHeaderSize() const
	{
		switch(GetEncoding())
		{
		case ADPCM:
			return 16;
		default:
			return 0;
		}
	}

	// Returns true if the encoded size cannot be calculated apriori from the encoding format and the sample length.
	MPT_CONSTEXPRINLINE bool IsVariableLengthEncoded() const
	{
		return GetEncodedBitsPerSample() == 0;
	}

	// Returns true if the decoder for a given format uses FileReader interface and thus do not need to call GetPinnedView()
	MPT_CONSTEXPRINLINE bool UsesFileReaderForDecoding() const
	{
		switch(GetEncoding())
		{
		case IT214:
		case IT215:
		case AMS:
		case DMF:
		case MDL:
			return true;
		default:
			return false;
		}
	}

	// Get bits per sample
	constexpr uint8 GetBitDepth() const
	{
		return static_cast<uint8>(m_bitdepth);
	}
	// Get channel layout
	constexpr Channels GetChannelFormat() const
	{
		return m_channels;
	}
	// Get number of channels
	constexpr uint8 GetNumChannels() const
	{
		return GetChannelFormat() == mono ? 1u : 2u;
	}
	// Get sample byte order
	constexpr Endianness GetEndianness() const
	{
		return m_endianness;
	}
	// Get sample format / encoding
	constexpr Encoding GetEncoding() const
	{
		return m_encoding;
	}

	// Returns the encoded size of the sample. In case of variable-length encoding returns 0.
	std::size_t CalculateEncodedSize(SmpLength length) const
	{
		if(IsVariableLengthEncoded())
		{
			return 0;
		}
		uint8 bps = GetEncodedBitsPerSample();
		if(bps % 8u != 0)
		{
			MPT_ASSERT(GetEncoding() == ADPCM && bps == 4);
			return GetEncodedHeaderSize() + (((length + 1) / 2) * GetNumChannels()); // round up
		}
		return GetEncodedHeaderSize() + (length * (bps / 8) * GetNumChannels());
	}

	// Read a sample from memory
	size_t ReadSample(ModSample &sample, FileReader &file) const;

#ifndef MODPLUG_NO_FILESAVE
	// Write a sample to file
	size_t WriteSample(std::ostream &f, const ModSample &sample, SmpLength maxSamples = 0) const;
#endif // MODPLUG_NO_FILESAVE
};


OPENMPT_NAMESPACE_END
