/*
 * Load_mo3.cpp
 * ------------
 * Purpose: MO3 module loader.
 * Notes  : (currently none)
 * Authors: Johannes Schultz / OpenMPT Devs
 *          Based on documentation and the decompression routines from the
 *          open-source UNMO3 project (https://github.com/lclevy/unmo3).
 *          The modified decompression code has been relicensed to the BSD
 *          license with permission from Laurent Clévy.
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "../common/ComponentManager.h"

#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_read/filedata_base_unseekable_buffer.hpp"

#include "Tables.h"
#include "../common/version.h"
#include "mpt/audio/span.hpp"
#include "MPEGFrame.h"
#include "OggStream.h"

#if defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)
#include <sstream>
#endif

#if defined(MPT_WITH_VORBIS)
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif  // MPT_COMPILER_CLANG
#include <vorbis/codec.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif  // MPT_COMPILER_CLANG
#endif

#if defined(MPT_WITH_VORBISFILE)
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif  // MPT_COMPILER_CLANG
#include <vorbis/vorbisfile.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif  // MPT_COMPILER_CLANG
#include "openmpt/soundbase/Copy.hpp"
#endif

#ifdef MPT_WITH_STBVORBIS
#include <stb_vorbis.h>
#include "openmpt/soundbase/Copy.hpp"
#endif  // MPT_WITH_STBVORBIS


OPENMPT_NAMESPACE_BEGIN



struct MO3FileHeader
{
	enum MO3HeaderFlags
	{
		linearSlides   = 0x0001,
		isS3M          = 0x0002,
		s3mFastSlides  = 0x0004,
		isMTM          = 0x0008,  // Actually this is simply "not XM". But if none of the S3M, MOD and IT flags are set, it's an MTM.
		s3mAmigaLimits = 0x0010,
		// 0x20 and 0x40 have been used in old versions for things that can be inferred from the file format anyway.
		// The official UNMO3 ignores them.
		isMOD          = 0x0080,
		isIT           = 0x0100,
		instrumentMode = 0x0200,
		itCompatGxx    = 0x0400,
		itOldFX        = 0x0800,
		modplugMode    = 0x10000,
		unknown        = 0x20000,  // Always set (internal BASS flag to designate modules)
		modVBlank      = 0x80000,
		hasPlugins     = 0x100000,
		extFilterRange = 0x200000,
	};

	uint8le  numChannels;  // 1...64 (limited by channel panning and volume)
	uint16le numOrders;
	uint16le restartPos;
	uint16le numPatterns;
	uint16le numTracks;
	uint16le numInstruments;
	uint16le numSamples;
	uint8le  defaultSpeed;
	uint8le  defaultTempo;
	uint32le flags;          // See MO3HeaderFlags
	uint8le  globalVol;      // 0...128 in IT, 0...64 in S3M
	uint8le  panSeparation;  // 0...128 in IT
	int8le   sampleVolume;   // Only used in IT
	uint8le  chnVolume[64];  // 0...64
	uint8le  chnPan[64];     // 0...256, 127 = surround
	uint8le  sfxMacros[16];
	uint8le  fixedMacros[128][2];
};

MPT_BINARY_STRUCT(MO3FileHeader, 422)


struct MO3Envelope
{
	enum MO3EnvelopeFlags
	{
		envEnabled = 0x01,
		envSustain = 0x02,
		envLoop    = 0x04,
		envFilter  = 0x10,
		envCarry   = 0x20,
	};

	uint8le flags;  // See MO3EnvelopeFlags
	uint8le numNodes;
	uint8le sustainStart;
	uint8le sustainEnd;
	uint8le loopStart;
	uint8le loopEnd;
	int16le points[25][2];

	// Convert MO3 envelope data into OpenMPT's internal envelope format
	void ConvertToMPT(InstrumentEnvelope &mptEnv, uint8 envShift, MODTYPE type) const
	{
		if(flags & envEnabled) mptEnv.dwFlags.set(ENV_ENABLED);
		if(flags & envSustain) mptEnv.dwFlags.set(ENV_SUSTAIN);
		if(flags & envLoop) mptEnv.dwFlags.set(ENV_LOOP);
		if(flags & envFilter) mptEnv.dwFlags.set(ENV_FILTER);
		if(flags & envCarry) mptEnv.dwFlags.set(ENV_CARRY);
		mptEnv.resize(std::min(numNodes.get(), uint8(25)));
		mptEnv.nSustainStart = sustainStart;
		mptEnv.nSustainEnd = (type == MOD_TYPE_XM) ? sustainStart : sustainEnd;
		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;
		for(uint32 ev = 0; ev < mptEnv.size(); ev++)
		{
			mptEnv[ev].tick = points[ev][0];
			if(ev > 0 && mptEnv[ev].tick < mptEnv[ev - 1].tick)
				mptEnv[ev].tick = mptEnv[ev - 1].tick + 1;
			mptEnv[ev].value = static_cast<uint8>(Clamp(points[ev][1] >> envShift, 0, 64));
		}
	}
};

MPT_BINARY_STRUCT(MO3Envelope, 106)


struct MO3Instrument
{
	enum MO3InstrumentFlags
	{
		playOnMIDI = 0x01,
		mute       = 0x02,
	};

	uint32le flags;  // See MO3InstrumentFlags
	uint16le sampleMap[120][2];
	MO3Envelope volEnv;
	MO3Envelope panEnv;
	MO3Envelope pitchEnv;
	struct XMVibratoSettings
	{
		uint8le  type;
		uint8le  sweep;
		uint8le  depth;
		uint8le  rate;
	} vibrato;  // Applies to all samples of this instrument (XM)
	uint16le fadeOut;
	uint8le  midiChannel;
	uint8le  midiBank;
	uint8le  midiPatch;
	uint8le  midiBend;
	uint8le  globalVol;  // 0...128
	uint16le panning;    // 0...256 if enabled, 0xFFFF otherwise
	uint8le  nna;
	uint8le  pps;
	uint8le  ppc;
	uint8le  dct;
	uint8le  dca;
	uint16le volSwing;   // 0...100
	uint16le panSwing;   // 0...256
	uint8le  cutoff;     // 0...127, + 128 if enabled
	uint8le  resonance;  // 0...127, + 128 if enabled

	// Convert MO3 instrument data into OpenMPT's internal instrument format
	void ConvertToMPT(ModInstrument &mptIns, MODTYPE type) const
	{
		if(type == MOD_TYPE_XM)
		{
			for(size_t i = 0; i < 96; i++)
			{
				mptIns.Keyboard[i + 12] = sampleMap[i][1] + 1;
			}
		} else
		{
			for(size_t i = 0; i < 120; i++)
			{
				mptIns.NoteMap[i] = static_cast<uint8>(sampleMap[i][0] + NOTE_MIN);
				mptIns.Keyboard[i] = sampleMap[i][1] + 1;
			}
		}
		volEnv.ConvertToMPT(mptIns.VolEnv, 0, type);
		panEnv.ConvertToMPT(mptIns.PanEnv, 0, type);
		pitchEnv.ConvertToMPT(mptIns.PitchEnv, 5, type);
		mptIns.nFadeOut = fadeOut;

		if(midiChannel >= 128)
		{
			// Plugin
			mptIns.nMixPlug = midiChannel - 127;
		} else if(midiChannel < 17 && (flags & playOnMIDI))
		{
			// XM, or IT with recent encoder
			mptIns.nMidiChannel = midiChannel + MidiFirstChannel;
		} else if(midiChannel > 0 && midiChannel < 17)
		{
			// IT encoded with MO3 version prior to 2.4.1 (yes, channel 0 is represented the same way as "no channel")
			mptIns.nMidiChannel = midiChannel + MidiFirstChannel;
		}
		if(mptIns.nMidiChannel != MidiNoChannel)
		{
			if(type == MOD_TYPE_XM)
			{
				mptIns.nMidiProgram = midiPatch + 1;
			} else
			{
				if(midiBank < 128)
					mptIns.wMidiBank = midiBank + 1;
				if(midiPatch < 128)
					mptIns.nMidiProgram = midiPatch + 1;
			}
			mptIns.midiPWD = midiBend;
		}

		if(type == MOD_TYPE_IT)
			mptIns.nGlobalVol = std::min(static_cast<uint8>(globalVol), uint8(128)) / 2u;
		if(panning <= 256)
		{
			mptIns.nPan = panning;
			mptIns.dwFlags.set(INS_SETPANNING);
		}
		mptIns.nNNA = static_cast<NewNoteAction>(nna.get());
		mptIns.nPPS = pps;
		mptIns.nPPC = ppc;
		mptIns.nDCT = static_cast<DuplicateCheckType>(dct.get());
		mptIns.nDNA = static_cast<DuplicateNoteAction>(dca.get());
		mptIns.nVolSwing = static_cast<uint8>(std::min(volSwing.get(), uint16(100)));
		mptIns.nPanSwing = static_cast<uint8>(std::min(panSwing.get(), uint16(256)) / 4u);
		mptIns.SetCutoff(cutoff & 0x7F, (cutoff & 0x80) != 0);
		mptIns.SetResonance(resonance & 0x7F, (resonance & 0x80) != 0);
	}
};

MPT_BINARY_STRUCT(MO3Instrument, 826)


struct MO3Sample
{
	enum MO3SampleFlags
	{
		smp16Bit            = 0x01,
		smpLoop             = 0x10,
		smpPingPongLoop     = 0x20,
		smpSustain          = 0x100,
		smpSustainPingPong  = 0x200,
		smpStereo           = 0x400,
		smpCompressionMPEG  = 0x1000,                    // MPEG 1.0 / 2.0 / 2.5 sample
		smpCompressionOgg   = 0x1000 | 0x2000,           // Ogg sample
		smpSharedOgg        = 0x1000 | 0x2000 | 0x4000,  // Ogg sample with shared vorbis header
		smpDeltaCompression = 0x2000,                    // Deltas + compression
		smpDeltaPrediction  = 0x4000,                    // Delta prediction + compression
		smpOPLInstrument    = 0x8000,                    // OPL patch data
		smpCompressionMask  = 0x1000 | 0x2000 | 0x4000 | 0x8000
	};

	uint32le freqFinetune;  // Frequency in S3M and IT, finetune (0...255) in MOD, MTM, XM
	int8le   transpose;
	uint8le  defaultVolume;  // 0...64
	uint16le panning;        // 0...256 if enabled, 0xFFFF otherwise
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
	uint16le flags;  // See MO3SampleFlags
	uint8le  vibType;
	uint8le  vibSweep;
	uint8le  vibDepth;
	uint8le  vibRate;
	uint8le  globalVol;  // 0...64 in IT, in XM it represents the instrument number
	uint32le sustainStart;
	uint32le sustainEnd;
	int32le  compressedSize;
	uint16le encoderDelay;  // MP3: Ignore first n bytes of decoded output. Ogg: Shared Ogg header size

	// Convert MO3 sample data into OpenMPT's internal instrument format
	void ConvertToMPT(ModSample &mptSmp, MODTYPE type, bool frequencyIsHertz) const
	{
		mptSmp.Initialize();
		mptSmp.SetDefaultCuePoints();
		if(type & (MOD_TYPE_IT | MOD_TYPE_S3M))
		{
			if(frequencyIsHertz)
				mptSmp.nC5Speed = freqFinetune;
			else
				mptSmp.nC5Speed = mpt::saturate_round<uint32>(8363.0 * std::pow(2.0, static_cast<int32>(freqFinetune + 1408) / 1536.0));
		} else
		{
			mptSmp.nFineTune = static_cast<int8>(freqFinetune);
			if(type != MOD_TYPE_MTM)
				mptSmp.nFineTune -= 128;
			mptSmp.RelativeTone = transpose;
		}
		mptSmp.nVolume = std::min(defaultVolume.get(), uint8(64)) * 4u;
		if(panning <= 256)
		{
			mptSmp.nPan = panning;
			mptSmp.uFlags.set(CHN_PANNING);
		}
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		if(flags & smpLoop)
			mptSmp.uFlags.set(CHN_LOOP);
		if(flags & smpPingPongLoop)
			mptSmp.uFlags.set(CHN_PINGPONGLOOP);
		if(flags & smpSustain)
			mptSmp.uFlags.set(CHN_SUSTAINLOOP);
		if(flags & smpSustainPingPong)
			mptSmp.uFlags.set(CHN_PINGPONGSUSTAIN);

		mptSmp.nVibType = static_cast<VibratoType>(AutoVibratoIT2XM[vibType & 7]);
		mptSmp.nVibSweep = vibSweep;
		mptSmp.nVibDepth = vibDepth;
		mptSmp.nVibRate = vibRate;

		if(type == MOD_TYPE_IT)
			mptSmp.nGlobalVol = std::min(static_cast<uint8>(globalVol), uint8(64));
		mptSmp.nSustainStart = sustainStart;
		mptSmp.nSustainEnd = sustainEnd;
	}
};

MPT_BINARY_STRUCT(MO3Sample, 41)


// We need all this information for Ogg-compressed samples with shared headers:
// A shared header can be taken from a sample that has not been read yet, so
// we first need to read all headers, and then load the Ogg samples afterwards.
struct MO3SampleInfo
{
	FileReader chunk;
	const MO3Sample smpHeader;
	const int16 sharedHeader;
	MO3SampleInfo(MO3Sample smpHeader, int16 sharedHeader)
		: smpHeader{smpHeader}, sharedHeader{sharedHeader} {}
};


// Unpack macros

// shift control bits until it is empty:
// a 0 bit means literal : the next data byte is copied
// a 1 means compressed data
// then the next 2 bits determines what is the LZ ptr
// ('00' same as previous, else stored in stream)

#define READ_CTRL_BIT \
	data <<= 1; \
	carry = (data > 0xFF); \
	data &= 0xFF; \
	if(data == 0) \
	{ \
		uint8 nextByte; \
		if(!file.Read(nextByte)) \
			break; \
		data = nextByte; \
		data <<= 1; \
		data += 1; \
		carry = (data > 0xFF); \
		data &= 0xFF; \
	}

// length coded within control stream:
// most significant bit is 1
// then the first bit of each bits pair (noted n1),
// until second bit is 0 (noted n0)

#define DECODE_CTRL_BITS \
	{ \
		strLen++; \
		do \
		{ \
			READ_CTRL_BIT; \
			strLen = mpt::lshift_signed(strLen, 1) + carry; \
			READ_CTRL_BIT; \
		} while(carry); \
	}


class MO3FileReaderBuffer final : public mpt::IO::FileDataUnseekableBuffer
{
public:
	MO3FileReaderBuffer(const FileReader &file, uint32 targetSize, uint32 suggestedReserveSize)
		: file{file}
		, m_suggestedReserveSize{suggestedReserveSize}
		, m_targetSize{targetSize}
		, m_totalRemain{targetSize}
	{
	}

	bool UnpackedSuccessfully() const
	{
		return !m_totalRemain && !m_broken;
	}

	auto SourcePosition() const
	{
		return file.GetPosition();
	}

protected:
	bool InternalEof() const override
	{
		return !m_totalRemain || m_broken;
	}

	void InternalReadContinue(std::vector<std::byte> &streamCache, std::size_t suggestedCount) const override
	{
		if(!suggestedCount|| !m_targetSize || m_broken)
			return;

		uint32 remain = std::min(m_totalRemain, mpt::saturate_cast<uint32>(suggestedCount));

		if(streamCache.empty())
		{
			// Fist byte is always read verbatim
			streamCache.reserve(std::min(m_targetSize, m_suggestedReserveSize));
			streamCache.push_back(mpt::byte_cast<std::byte>(file.ReadUint8()));
			m_totalRemain--;
			remain--;
		}

		int32 strLen = m_strLen;
		if(strLen)
		{
			// Previous string copy is still in progress
			uint32 copyLen = std::min(static_cast<uint32>(strLen), remain);
			m_totalRemain -= copyLen;
			remain -= copyLen;
			strLen -= copyLen;
			streamCache.insert(streamCache.end(), copyLen, std::byte{});
			auto src = streamCache.cend() - copyLen + m_strOffset;
			auto dst = streamCache.end() - copyLen;
			do
			{
				copyLen--;
				*dst++ = *src++;
			} while(copyLen > 0);
		}
		
		uint16 data = m_data;
		int8 carry = 0;  // x86 carry (used to propagate the most significant bit from one byte to another)
		while(remain)
		{
			READ_CTRL_BIT;
			if(!carry)
			{
				// a 0 ctrl bit means 'copy', not compressed byte
				if(std::byte b; file.Read(b))
					streamCache.push_back(b);
				else
					break;
				m_totalRemain--;
				remain--;
			} else
			{
				// a 1 ctrl bit means compressed bytes are following
				uint8 lengthAdjust = 0;  // length adjustment
				DECODE_CTRL_BITS;        // read length, and if strLen > 3 (coded using more than 1 bits pair) also part of the offset value
				strLen -= 3;
				if(strLen < 0)
				{
					// reuse same previous relative LZ ptr (m_strOffset is not re-computed)
					strLen++;
				} else
				{
					// LZ ptr in ctrl stream
					if(uint8 b; file.Read(b))
						m_strOffset = mpt::lshift_signed(strLen, 8) | b;  // read less significant offset byte from stream
					else
						break;
					strLen = 0;
					m_strOffset = ~m_strOffset;
					if(m_strOffset < -1280)
						lengthAdjust++;
					lengthAdjust++;  // length is always at least 1
					if(m_strOffset < -32000)
						lengthAdjust++;
				}
				if(m_strOffset >= 0 || -static_cast<ptrdiff_t>(streamCache.size()) > m_strOffset)
					break;

				// read the next 2 bits as part of strLen
				READ_CTRL_BIT;
				strLen = mpt::lshift_signed(strLen, 1) + carry;
				READ_CTRL_BIT;
				strLen = mpt::lshift_signed(strLen, 1) + carry;
				if(strLen == 0)
				{
					// length does not fit in 2 bits
					DECODE_CTRL_BITS;  // decode length: 1 is the most significant bit,
					strLen += 2;       // then first bit of each bits pairs (noted n1), until n0.
				}
				strLen += lengthAdjust;  // length adjustment

				if(strLen <= 0 || m_totalRemain < static_cast<uint32>(strLen))
					break;

				// Copy previous string
				// Need to do this in two steps (allocate, then copy) as source and destination may overlap (e.g. strOffset = -1, strLen = 2 repeats last character twice)
				uint32 copyLen = std::min(static_cast<uint32>(strLen), remain);
				m_totalRemain -= copyLen;
				remain -= copyLen;
				strLen -= copyLen;
				streamCache.insert(streamCache.end(), copyLen, std::byte{});
				auto src = streamCache.cend() - copyLen + m_strOffset;
				auto dst = streamCache.end() - copyLen;
				do
				{
					copyLen--;
					*dst++ = *src++;
				} while(copyLen > 0);
			}
		}
		m_data = data;
		m_strLen = strLen;
		// Premature EOF or corrupted stream?
		if(remain)
			m_broken = true;
	}

	bool HasPinnedView() const override
	{
		return false;
	}

	mutable FileReader file;
	const uint32 m_suggestedReserveSize;
	const uint32 m_targetSize;
	mutable bool m_broken = false;
	mutable uint16 m_data = 0;
	mutable int32 m_strLen = 0;     // Length of repeated string
	mutable int32 m_strOffset = 0;  // Offset of repeated string
	mutable uint32 m_totalRemain = 0;
};


struct MO3Delta8BitParams
{
	using sample_t = int8;
	using unsigned_t = uint8;
	static constexpr int shift = 7;
	static constexpr uint8 dhInit = 4;

	static inline void Decode(FileReader &file, int8 &carry, uint16 &data, uint8 & /*dh*/, unsigned_t &val)
	{
		do
		{
			READ_CTRL_BIT;
			val = static_cast<unsigned_t>((val << 1) + carry);
			READ_CTRL_BIT;
		} while(carry);
	}
};

struct MO3Delta16BitParams
{
	using sample_t = int16;
	using unsigned_t = uint16;
	static constexpr int shift = 15;
	static constexpr uint8 dhInit = 8;

	static inline void Decode(FileReader &file, int8 &carry, uint16 &data, uint8 &dh, unsigned_t &val)
	{
		if(dh < 5)
		{
			do
			{
				READ_CTRL_BIT;
				val = static_cast<unsigned_t>((val << 1) + carry);
				READ_CTRL_BIT;
				val = static_cast<unsigned_t>((val << 1) + carry);
				READ_CTRL_BIT;
			} while(carry);
		} else
		{
			do
			{
				READ_CTRL_BIT;
				val = static_cast<unsigned_t>((val << 1) + carry);
				READ_CTRL_BIT;
			} while(carry);
		}
	}
};


template <typename Properties>
static void UnpackMO3DeltaSample(FileReader &file, typename Properties::sample_t *dst, uint32 length, uint8 numChannels)
{
	uint8 dh = Properties::dhInit, cl = 0;
	int8 carry = 0;
	uint16 data = 0;
	typename Properties::unsigned_t val;
	typename Properties::sample_t previous = 0;

	for(uint8 chn = 0; chn < numChannels; chn++)
	{
		typename Properties::sample_t *p = dst + chn;
		const typename Properties::sample_t *const pEnd = p + length * numChannels;
		while(p < pEnd)
		{
			val = 0;
			Properties::Decode(file, carry, data, dh, val);
			cl = dh;
			while(cl > 0)
			{
				READ_CTRL_BIT;
				val = static_cast<typename Properties::unsigned_t>((val << 1) + carry);
				cl--;
			}
			cl = 1;
			if(val >= 4)
			{
				cl = Properties::shift;
				while(((1 << cl) & val) == 0 && cl > 1)
					cl--;
			}
			dh = dh + cl;
			dh >>= 1;         // next length in bits of encoded delta second part
			carry = val & 1;  // sign of delta 1=+, 0=not
			val >>= 1;
			if(carry == 0)
				val = ~val;   // negative delta
			val = static_cast<typename Properties::unsigned_t>(val + previous);  // previous value + delta
			*p = val;
			p += numChannels;
			previous = val;
		}
	}
}


template <typename Properties>
static void UnpackMO3DeltaPredictionSample(FileReader &file, typename Properties::sample_t *dst, uint32 length, uint8 numChannels)
{
	uint8 dh = Properties::dhInit, cl = 0;
	int8 carry;
	uint16 data = 0;
	int32 next = 0;
	typename Properties::unsigned_t val = 0;
	typename Properties::sample_t sval = 0, delta = 0, previous = 0;

	for(uint8 chn = 0; chn < numChannels; chn++)
	{
		typename Properties::sample_t *p = dst + chn;
		const typename Properties::sample_t *const pEnd = p + length * numChannels;
		while(p < pEnd)
		{
			val = 0;
			Properties::Decode(file, carry, data, dh, val);
			cl = dh;  // length in bits of: delta second part (right most bits of delta) and sign bit
			while(cl > 0)
			{
				READ_CTRL_BIT;
				val = static_cast<typename Properties::unsigned_t>((val << 1) + carry);
				cl--;
			}
			cl = 1;
			if(val >= 4)
			{
				cl = Properties::shift;
				while(((1 << cl) & val) == 0 && cl > 1)
					cl--;
			}
			dh = dh + cl;
			dh >>= 1;         // next length in bits of encoded delta second part
			carry = val & 1;  // sign of delta 1=+, 0=not
			val >>= 1;
			if(carry == 0)
				val = ~val;  // negative delta

			delta = static_cast<typename Properties::sample_t>(val);
			val = val + static_cast<typename Properties::unsigned_t>(next);  // predicted value + delta
			*p = val;
			p += numChannels;
			sval = static_cast<typename Properties::sample_t>(val);
			next = (sval * (1 << 1)) + (delta >> 1) - previous;  // corrected next value

			Limit(next, std::numeric_limits<typename Properties::sample_t>::min(), std::numeric_limits<typename Properties::sample_t>::max());

			previous = sval;
		}
	}
}


#undef READ_CTRL_BIT
#undef DECODE_CTRL_BITS


#if defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)

static size_t VorbisfileFilereaderRead(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	FileReader &file = *mpt::void_ptr<FileReader>(datasource);
	return file.ReadRaw(mpt::span(mpt::void_cast<std::byte *>(ptr), size * nmemb)).size() / size;
}

static int VorbisfileFilereaderSeek(void *datasource, ogg_int64_t offset, int whence)
{
	FileReader &file = *mpt::void_ptr<FileReader>(datasource);
	switch(whence)
	{
	case SEEK_SET:
		if(!mpt::in_range<FileReader::pos_type>(offset))
		{
			return -1;
		}
		return file.Seek(mpt::saturate_cast<FileReader::pos_type>(offset)) ? 0 : -1;

	case SEEK_CUR:
		if(offset < 0)
		{
			if(offset == std::numeric_limits<ogg_int64_t>::min())
			{
				return -1;
			}
			if(!mpt::in_range<FileReader::pos_type>(0 - offset))
			{
				return -1;
			}
			return file.SkipBack(mpt::saturate_cast<FileReader::pos_type>(0 - offset)) ? 0 : -1;
		} else
		{
			if(!mpt::in_range<FileReader::pos_type>(offset))
			{
				return -1;
			}
			return file.Skip(mpt::saturate_cast<FileReader::pos_type>(offset)) ? 0 : -1;
		}
		break;

	case SEEK_END:
		if(!mpt::in_range<FileReader::pos_type>(offset))
		{
			return -1;
		}
		if(!mpt::in_range<FileReader::pos_type>(file.GetLength() + offset))
		{
			return -1;
		}
		return file.Seek(mpt::saturate_cast<FileReader::pos_type>(file.GetLength() + offset)) ? 0 : -1;

	default:
		return -1;
	}
}

static long VorbisfileFilereaderTell(void *datasource)
{
	FileReader &file = *mpt::void_ptr<FileReader>(datasource);
	FileReader::pos_type result = file.GetPosition();
	if(!mpt::in_range<long>(result))
	{
		return -1;
	}
	return static_cast<long>(result);
}

#endif  // MPT_WITH_VORBIS && MPT_WITH_VORBISFILE


struct MO3ContainerHeader
{
	char     magic[3];  // MO3
	uint8le  version;
	uint32le musicSize;
};

MPT_BINARY_STRUCT(MO3ContainerHeader, 8)


static bool ValidateHeader(const MO3ContainerHeader &containerHeader)
{
	if(std::memcmp(containerHeader.magic, "MO3", 3))
	{
		return false;
	}
	// Due to the LZ algorithm's unbounded back window, we could reach gigantic sizes with just a few dozen bytes.
	// 512 MB of music data (not samples) is chosen as a safeguard that is probably (hopefully) *way* beyond anything a real-world module will ever reach.
	if(containerHeader.musicSize <= sizeof(MO3FileHeader) || containerHeader.musicSize >= 0x2000'0000)
	{
		return false;
	}
	if(containerHeader.version > 5)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMO3(MemoryFileReader file, const uint64 *pfilesize)
{
	MO3ContainerHeader containerHeader;
	if(!file.ReadStruct(containerHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(containerHeader))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadMO3(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	MO3ContainerHeader containerHeader;
	if(!file.ReadStruct(containerHeader))
	{
		return false;
	}
	if(!ValidateHeader(containerHeader))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	const uint8 version = containerHeader.version;

	uint32 compressedSize = uint32_max, reserveSize = 1024 * 1024;  // Generous estimate based on biggest pre-v5 MO3s found in the wild (~350K music data)
	if(version >= 5)
	{
		// Size of compressed music chunk
		compressedSize = file.ReadUint32LE();
		if(!file.CanRead(compressedSize))
			return false;
		// Generous estimate based on highest real-world compression ratio I found in a module (~20:1)
		reserveSize = std::min(Util::MaxValueOfType(reserveSize) / 32u, compressedSize) * 32u;
	}

	std::shared_ptr<mpt::PathString> filenamePtr;
	if(auto filename = file.GetOptionalFileName(); filename)
		filenamePtr = std::make_shared<mpt::PathString>(std::move(*filename));
	auto musicChunkData = std::make_shared<MO3FileReaderBuffer>(file, containerHeader.musicSize, reserveSize);
	mpt::IO::FileCursor<mpt::IO::FileCursorTraitsFileData, mpt::IO::FileCursorFilenameTraits<mpt::PathString>> fileCursor{musicChunkData, std::move(filenamePtr)};
	FileReader musicChunk{fileCursor};

	std::string songName, songMessage;
	musicChunk.ReadNullString(songName);
	musicChunk.ReadNullString(songMessage);

	MO3FileHeader fileHeader;
	if(!musicChunk.ReadStruct(fileHeader)
	   || fileHeader.numChannels == 0 || fileHeader.numChannels > MAX_BASECHANNELS
	   || fileHeader.restartPos > fileHeader.numOrders
	   || fileHeader.numInstruments >= MAX_INSTRUMENTS
	   || fileHeader.numSamples >= MAX_SAMPLES)
	{
		return false;
	}

	MODTYPE modType = MOD_TYPE_XM;
	if(fileHeader.flags & MO3FileHeader::isIT)
		modType = MOD_TYPE_IT;
	else if(fileHeader.flags & MO3FileHeader::isS3M)
		modType = MOD_TYPE_S3M;
	else if(fileHeader.flags & MO3FileHeader::isMOD)
		modType = MOD_TYPE_MOD;
	else if(fileHeader.flags & MO3FileHeader::isMTM)
		modType = MOD_TYPE_MTM;

	InitializeGlobals(modType, fileHeader.numChannels);
	Order().SetRestartPos(fileHeader.restartPos);
	m_nInstruments = fileHeader.numInstruments;
	m_nSamples = fileHeader.numSamples;
	Order().SetDefaultSpeed(fileHeader.defaultSpeed ? fileHeader.defaultSpeed : 6);
	Order().SetDefaultTempoInt(fileHeader.defaultTempo ? fileHeader.defaultTempo : 125);
	m_songName = std::move(songName);
	m_songMessage.SetRaw(std::move(songMessage));

	m_SongFlags.set(SONG_IMPORTED);
	if(fileHeader.flags & MO3FileHeader::linearSlides)
		m_SongFlags.set(SONG_LINEARSLIDES);
	if((fileHeader.flags & MO3FileHeader::s3mAmigaLimits) && m_nType == MOD_TYPE_S3M)
		m_SongFlags.set(SONG_AMIGALIMITS);
	if((fileHeader.flags & MO3FileHeader::s3mFastSlides) && m_nType == MOD_TYPE_S3M)
		m_SongFlags.set(SONG_FASTVOLSLIDES);
	if(!(fileHeader.flags & MO3FileHeader::itOldFX) && m_nType == MOD_TYPE_IT)
		m_SongFlags.set(SONG_ITOLDEFFECTS);
	if(!(fileHeader.flags & MO3FileHeader::itCompatGxx) && m_nType == MOD_TYPE_IT)
		m_SongFlags.set(SONG_ITCOMPATGXX);
	if(fileHeader.flags & MO3FileHeader::extFilterRange)
		m_SongFlags.set(SONG_EXFILTERRANGE);
	if(fileHeader.flags & MO3FileHeader::modVBlank)
		m_playBehaviour.set(kMODVBlankTiming);

	if(m_nType == MOD_TYPE_IT)
		m_nDefaultGlobalVolume = std::min(fileHeader.globalVol.get(), uint8(128)) * 2;
	else if(m_nType == MOD_TYPE_S3M)
		m_nDefaultGlobalVolume = std::min(fileHeader.globalVol.get(), uint8(64)) * 4;
	else if(m_nType == MOD_TYPE_MOD)
		m_SongFlags.set(SONG_FORMAT_NO_VOLCOL);

	if(fileHeader.sampleVolume < 0)
		m_nSamplePreAmp = fileHeader.sampleVolume + 52;
	else
		m_nSamplePreAmp = static_cast<uint32>(std::exp(fileHeader.sampleVolume * 3.1 / 20.0)) + 51;

	// Header only has room for 64 channels, like in IT
	const CHANNELINDEX headerChannels = std::min(GetNumChannels(), CHANNELINDEX(64));
	for(CHANNELINDEX i = 0; i < headerChannels; i++)
	{
		if(m_nType == MOD_TYPE_IT)
			ChnSettings[i].nVolume = std::min(fileHeader.chnVolume[i].get(), uint8(64));
		if(m_nType != MOD_TYPE_XM)
		{
			if(fileHeader.chnPan[i] == 127)
				ChnSettings[i].dwFlags = CHN_SURROUND;
			else if(fileHeader.chnPan[i] == 255)
				ChnSettings[i].nPan = 256;
			else
				ChnSettings[i].nPan = fileHeader.chnPan[i];
		}
	}

	bool anyMacros = false;
	for(uint32 i = 0; i < 16; i++)
	{
		if(fileHeader.sfxMacros[i])
			anyMacros = true;
	}
	for(uint32 i = 0; i < 128; i++)
	{
		if(fileHeader.fixedMacros[i][1])
			anyMacros = true;
	}

	if(anyMacros)
	{
		for(uint32 i = 0; i < 16; i++)
		{
			if(fileHeader.sfxMacros[i])
				m_MidiCfg.SFx[i] = MPT_AFORMAT("F0F0{}z")(mpt::afmt::HEX0<2>(fileHeader.sfxMacros[i] - 1));
			else
				m_MidiCfg.SFx[i] = "";
		}
		for(uint32 i = 0; i < 128; i++)
		{
			if(fileHeader.fixedMacros[i][1])
				m_MidiCfg.Zxx[i] = MPT_AFORMAT("F0F0{}{}")(mpt::afmt::HEX0<2>(fileHeader.fixedMacros[i][1] - 1), mpt::afmt::HEX0<2>(fileHeader.fixedMacros[i][0].get()));
			else
				m_MidiCfg.Zxx[i] = "";
		}
	}

	const bool hasOrderSeparators = !(m_nType & (MOD_TYPE_MOD | MOD_TYPE_XM));
	ReadOrderFromFile<uint8>(Order(), musicChunk, fileHeader.numOrders, hasOrderSeparators ? 0xFF : uint16_max, hasOrderSeparators ? 0xFE : uint16_max);

	// Track assignments for all patterns
	FileReader trackChunk = musicChunk.ReadChunk(fileHeader.numPatterns * fileHeader.numChannels * sizeof(uint16));
	FileReader patLengthChunk = musicChunk.ReadChunk(fileHeader.numPatterns * sizeof(uint16));
	std::vector<FileReader> tracks(fileHeader.numTracks);

	for(auto &track : tracks)
	{
		uint32 len = musicChunk.ReadUint32LE();
		// A pattern can be at most 65535 rows long, one row can contain at most 15 events (with the status bytes, that 31 bytes per row).
		// Leaving some margin for error, that gives us an upper limit of 2MB per track.
		if(len >= 0x20'0000)
			return false;
		track = musicChunk.ReadChunk(len);
	}

	/*
	MO3 pattern commands:
	01 = Note
	02 = Instrument
	03 = CMD_ARPEGGIO (IT, XM, S3M, MOD, MTM)
	04 = CMD_PORTAMENTOUP (XM, MOD, MTM)   [for formats with separate fine slides]
	05 = CMD_PORTAMENTODOWN (XM, MOD, MTM) [for formats with separate fine slides]
	06 = CMD_TONEPORTAMENTO (IT, XM, S3M, MOD, MTM) / VOLCMD_TONEPORTA (IT, XM)
	07 = CMD_VIBRATO (IT, XM, S3M, MOD, MTM) / VOLCMD_VIBRATODEPTH (IT)
	08 = CMD_TONEPORTAVOL (XM, MOD, MTM)
	09 = CMD_VIBRATOVOL (XM, MOD, MTM)
	0A = CMD_TREMOLO (IT, XM, S3M, MOD, MTM)
	0B = CMD_PANNING8 (IT, XM, S3M, MOD, MTM) / VOLCMD_PANNING (IT, XM)
	0C = CMD_OFFSET (IT, XM, S3M, MOD, MTM)
	0D = CMD_VOLUMESLIDE (XM, MOD, MTM)
	0E = CMD_POSITIONJUMP (IT, XM, S3M, MOD, MTM)
	0F = CMD_VOLUME (XM, MOD, MTM) / VOLCMD_VOLUME (IT, XM, S3M)
	10 = CMD_PATTERNBREAK (IT, XM, MOD, MTM) - BCD-encoded in MOD/XM/S3M/MTM!
	11 = CMD_MODCMDEX (XM, MOD, MTM)
	12 = CMD_TEMPO (XM, MOD, MTM) / CMD_SPEED (XM, MOD, MTM)
	13 = CMD_TREMOR (XM)
	14 = VOLCMD_VOLSLIDEUP x=X0 (XM) / VOLCMD_VOLSLIDEDOWN x=0X (XM)
	15 = VOLCMD_FINEVOLUP x=X0 (XM) / VOLCMD_FINEVOLDOWN x=0X (XM)
	16 = CMD_GLOBALVOLUME (IT, XM, S3M)
	17 = CMD_GLOBALVOLSLIDE (XM)
	18 = CMD_KEYOFF (XM)
	19 = CMD_SETENVPOSITION (XM)
	1A = CMD_PANNINGSLIDE (XM)
	1B = VOLCMD_PANSLIDELEFT x=0X (XM) / VOLCMD_PANSLIDERIGHT x=X0 (XM)
	1C = CMD_RETRIG (XM)
	1D = CMD_XFINEPORTAUPDOWN X1x (XM)
	1E = CMD_XFINEPORTAUPDOWN X2x (XM)
	1F = VOLCMD_VIBRATOSPEED (XM)
	20 = VOLCMD_VIBRATODEPTH (XM)
	21 = CMD_SPEED (IT, S3M)
	22 = CMD_VOLUMESLIDE (IT, S3M)
	23 = CMD_PORTAMENTODOWN (IT, S3M) [for formats without separate fine slides]
	24 = CMD_PORTAMENTOUP (IT, S3M)   [for formats without separate fine slides]
	25 = CMD_TREMOR (IT, S3M)
	26 = CMD_RETRIG (IT, S3M)
	27 = CMD_FINEVIBRATO (IT, S3M)
	28 = CMD_CHANNELVOLUME (IT, S3M)
	29 = CMD_CHANNELVOLSLIDE (IT, S3M)
	2A = CMD_PANNINGSLIDE (IT, S3M)
	2B = CMD_S3MCMDEX (IT, S3M)
	2C = CMD_TEMPO (IT, S3M)
	2D = CMD_GLOBALVOLSLIDE (IT, S3M)
	2E = CMD_PANBRELLO (IT, XM, S3M)
	2F = CMD_MIDI (IT, XM, S3M)
	30 = VOLCMD_FINEVOLUP x=0...9 (IT) / VOLCMD_FINEVOLDOWN x=10...19 (IT) / VOLCMD_VOLSLIDEUP x=20...29 (IT) / VOLCMD_VOLSLIDEDOWN x=30...39 (IT)
	31 = VOLCMD_PORTADOWN (IT)
	32 = VOLCMD_PORTAUP (IT)
	33 = Unused XM command "W" (XM)
	34 = Any other IT volume column command to support OpenMPT extensions (IT)
	35 = CMD_XPARAM (IT)
	36 = CMD_SMOOTHMIDI (IT)
	37 = CMD_DELAYCUT (IT)
	38 = CMD_FINETUNE (MPTM)
	39 = CMD_FINETUNE_SMOOTH (MPTM)

	Note: S3M/IT CMD_TONEPORTAVOL / CMD_VIBRATOVOL are encoded as two commands:
	K= 07 00 22 x
	L= 06 00 22 x
	*/

	static constexpr EffectCommand effTrans[] =
	{
		CMD_NONE,               CMD_NONE,               CMD_NONE,               CMD_ARPEGGIO,
		CMD_PORTAMENTOUP,       CMD_PORTAMENTODOWN,     CMD_TONEPORTAMENTO,     CMD_VIBRATO,
		CMD_TONEPORTAVOL,       CMD_VIBRATOVOL,         CMD_TREMOLO,            CMD_PANNING8,
		CMD_OFFSET,             CMD_VOLUMESLIDE,        CMD_POSITIONJUMP,       CMD_VOLUME,
		CMD_PATTERNBREAK,       CMD_MODCMDEX,           CMD_TEMPO,              CMD_TREMOR,
		CMD_NONE,/*VolSlideUp*/ CMD_NONE,/*VolSlideDn*/ CMD_GLOBALVOLUME,       CMD_GLOBALVOLSLIDE,
		CMD_KEYOFF,             CMD_SETENVPOSITION,     CMD_PANNINGSLIDE,       CMD_NONE,/*PanSlide*/
		CMD_RETRIG,             CMD_XFINEPORTAUPDOWN,   CMD_XFINEPORTAUPDOWN,   CMD_NONE,/*VibSpeed*/
		CMD_NONE,/*VibDepth*/   CMD_SPEED,              CMD_VOLUMESLIDE,        CMD_PORTAMENTODOWN,
		CMD_PORTAMENTOUP,       CMD_TREMOR,             CMD_RETRIG,             CMD_FINEVIBRATO,
		CMD_CHANNELVOLUME,      CMD_CHANNELVOLSLIDE,    CMD_PANNINGSLIDE,       CMD_S3MCMDEX,
		CMD_TEMPO,              CMD_GLOBALVOLSLIDE,     CMD_PANBRELLO,          CMD_MIDI,
		CMD_NONE,/*FineVolSld*/ CMD_NONE,/*PortaDown*/  CMD_NONE, /*PortaUp*/   CMD_DUMMY,
		CMD_NONE,/*ITVolCol*/   CMD_XPARAM,             CMD_SMOOTHMIDI,         CMD_DELAYCUT,
		CMD_FINETUNE,           CMD_FINETUNE_SMOOTH,
	};

	uint8 noteOffset = NOTE_MIN;
	if(m_nType == MOD_TYPE_MTM)
		noteOffset = 13 + NOTE_MIN;
	else if(m_nType != MOD_TYPE_IT)
		noteOffset = 12 + NOTE_MIN;
	bool onlyAmigaNotes = true;

	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.numPatterns);
	for(PATTERNINDEX pat = 0; pat < fileHeader.numPatterns; pat++)
	{
		const ROWINDEX numRows = patLengthChunk.ReadUint16LE();
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, numRows))
			continue;

		for(CHANNELINDEX chn = 0; chn < fileHeader.numChannels; chn++)
		{
			uint16 trackIndex = trackChunk.ReadUint16LE();
			if(trackIndex >= tracks.size())
				continue;
			FileReader &track = tracks[trackIndex];
			track.Rewind();
			ROWINDEX row = 0;
			ModCommand *patData = Patterns[pat].GetpModCommand(0, chn);
			while(row < numRows)
			{
				const uint8 b = track.ReadUint8();
				if(!b)
					break;

				const uint8 numCommands = (b & 0x0F), rep = (b >> 4);
				ModCommand m;
				for(uint8 c = 0; c < numCommands; c++)
				{
					const auto cmd = track.ReadArray<uint8, 2>();

					// Import pattern commands
					switch(cmd[0])
					{
					case 0x01:
						// Note
						m.note = cmd[1];
						if(m.note < 120)
							m.note += noteOffset;
						else if(m.note == 0xFF)
							m.note = NOTE_KEYOFF;
						else if(m.note == 0xFE)
							m.note = NOTE_NOTECUT;
						else
							m.note = NOTE_FADE;
						if(!m.IsAmigaNote())
							onlyAmigaNotes = false;
						break;
					case 0x02:
						// Instrument
						m.instr = cmd[1] + 1;
						break;
					case 0x06:
						// Tone portamento
						if(m.volcmd == VOLCMD_NONE && m_nType == MOD_TYPE_XM && !(cmd[1] & 0x0F))
						{
							m.SetVolumeCommand(VOLCMD_TONEPORTAMENTO, cmd[1] >> 4);
							break;
						} else if(m.volcmd == VOLCMD_NONE && m_nType == MOD_TYPE_IT)
						{
							for(uint8 i = 0; i < 10; i++)
							{
								if(ImpulseTrackerPortaVolCmd[i] == cmd[1])
								{
									m.SetVolumeCommand(VOLCMD_TONEPORTAMENTO, i);
									break;
								}
							}
							if(m.volcmd != VOLCMD_NONE)
								break;
						}
						m.SetEffectCommand(CMD_TONEPORTAMENTO, cmd[1]);
						break;
					case 0x07:
						// Vibrato
						if(m.volcmd == VOLCMD_NONE && cmd[1] < 10 && m_nType == MOD_TYPE_IT)
						{
							m.SetVolumeCommand(VOLCMD_VIBRATODEPTH, cmd[1]);
						} else
						{
							m.SetEffectCommand(CMD_VIBRATO, cmd[1]);
						}
						break;
					case 0x0B:
						// Panning
						if(m.volcmd == VOLCMD_NONE)
						{
							if(m_nType == MOD_TYPE_IT && cmd[1] == 0xFF)
							{
								m.SetVolumeCommand(VOLCMD_PANNING, 64);
								break;
							}
							if((m_nType == MOD_TYPE_IT && !(cmd[1] & 0x03))
							   || (m_nType == MOD_TYPE_XM && !(cmd[1] & 0x0F)))
							{
								m.SetVolumeCommand(VOLCMD_PANNING, cmd[1] / 4);
								break;
							}
						}
						m.SetEffectCommand(CMD_PANNING8, cmd[1]);
						break;
					case 0x0F:
						// Volume
						if(m_nType != MOD_TYPE_MOD && m.volcmd == VOLCMD_NONE && cmd[1] <= 64)
							m.SetVolumeCommand(VOLCMD_VOLUME, cmd[1]);
						else
							m.SetEffectCommand(CMD_VOLUME, cmd[1]);
						break;
					case 0x10:
						// Pattern break
						m.SetEffectCommand(CMD_PATTERNBREAK, cmd[1]);
						if(m_nType != MOD_TYPE_IT)
							m.param = static_cast<ModCommand::PARAM>(((m.param >> 4) * 10) + (m.param & 0x0F));
						break;
					case 0x12:
						// Combined Tempo / Speed command
						m.SetEffectCommand((cmd[1] < 0x20) ? CMD_SPEED : CMD_TEMPO, cmd[1]);
						break;
					case 0x14:
					case 0x15:
						// XM volume column volume slides
						if(cmd[1] & 0xF0)
						{
							m.SetVolumeCommand((cmd[0] == 0x14) ? VOLCMD_VOLSLIDEUP : VOLCMD_FINEVOLUP, cmd[1] >> 4);
						} else
						{
							m.SetVolumeCommand((cmd[0] == 0x14) ? VOLCMD_VOLSLIDEDOWN : VOLCMD_FINEVOLDOWN, cmd[1] & 0x0F);
						}
						break;
					case 0x1B:
						// XM volume column panning slides
						if(cmd[1] & 0xF0)
							m.SetVolumeCommand(VOLCMD_PANSLIDERIGHT, cmd[1] >> 4);
						else
							m.SetVolumeCommand(VOLCMD_PANSLIDELEFT, cmd[1] & 0x0F);
						break;
					case 0x1D:
						// XM extra fine porta up
						m.SetEffectCommand(CMD_XFINEPORTAUPDOWN, 0x10 | cmd[1]);
						break;
					case 0x1E:
						// XM extra fine porta down
						m.SetEffectCommand(CMD_XFINEPORTAUPDOWN, 0x20 | cmd[1]);
						break;
					case 0x1F:
					case 0x20:
						// XM volume column vibrato
						m.SetVolumeCommand((cmd[0] == 0x1F) ? VOLCMD_VIBRATOSPEED: VOLCMD_VIBRATODEPTH, cmd[1]);
						break;
					case 0x22:
						// IT / S3M volume slide
						if(m.command == CMD_TONEPORTAMENTO)
							m.command = CMD_TONEPORTAVOL;
						else if(m.command == CMD_VIBRATO)
							m.command = CMD_VIBRATOVOL;
						else
							m.command = CMD_VOLUMESLIDE;
						m.param = cmd[1];
						break;
					case 0x30:
						// IT volume column volume slides
						m.vol = cmd[1] % 10;
						if(cmd[1] < 10)
							m.volcmd = VOLCMD_FINEVOLUP;
						else if(cmd[1] < 20)
							m.volcmd = VOLCMD_FINEVOLDOWN;
						else if(cmd[1] < 30)
							m.volcmd = VOLCMD_VOLSLIDEUP;
						else if(cmd[1] < 40)
							m.volcmd = VOLCMD_VOLSLIDEDOWN;
						break;
					case 0x31:
					case 0x32:
						// IT volume column portamento
						m.SetVolumeCommand((cmd[0] == 0x31) ? VOLCMD_PORTADOWN: VOLCMD_PORTAUP, cmd[1]);
						break;
					case 0x34:
						// Any unrecognized IT volume command
						if(cmd[1] >= 223 && cmd[1] <= 232)
							m.SetVolumeCommand(VOLCMD_OFFSET, cmd[1] - 223);
						break;
					default:
						if(cmd[0] < std::size(effTrans))
							m.SetEffectCommand(effTrans[cmd[0]], cmd[1]);
						break;
					}
				}
#ifdef MODPLUG_TRACKER
				if(m_nType == MOD_TYPE_MTM)
					m.Convert(MOD_TYPE_MTM, MOD_TYPE_S3M, *this);
#endif
				ROWINDEX targetRow = std::min(row + rep, numRows);
				while(row < targetRow)
				{
					*patData = m;
					patData += fileHeader.numChannels;
					row++;
				}
			}
		}
	}

	if(GetType() == MOD_TYPE_MOD && GetNumChannels() == 4 && onlyAmigaNotes)
	{
		m_SongFlags.set(SONG_AMIGALIMITS | SONG_ISAMIGA);
	}

	const bool isSampleMode = (m_nType != MOD_TYPE_XM && !(fileHeader.flags & MO3FileHeader::instrumentMode));
	std::vector<MO3Instrument::XMVibratoSettings> instrVibrato(m_nType == MOD_TYPE_XM ? m_nInstruments : 0);
	for(INSTRUMENTINDEX ins = 1; ins <= m_nInstruments; ins++)
	{
		ModInstrument *pIns = nullptr;
		if(isSampleMode || (pIns = AllocateInstrument(ins)) == nullptr)
		{
			// Even in IT sample mode, instrument headers are still stored....
			while(musicChunk.ReadUint8() != 0)
				;
			if(version >= 5)
			{
				while(musicChunk.ReadUint8() != 0)
					;
			}
			if(!musicChunk.Skip(sizeof(MO3Instrument)))
				return false;
			continue;
		}

		std::string name;
		musicChunk.ReadNullString(name);
		pIns->name = name;
		if(version >= 5)
		{
			musicChunk.ReadNullString(name);
			pIns->filename = name;
		}

		MO3Instrument insHeader;
		if(!musicChunk.ReadStruct(insHeader))
			return false;
		insHeader.ConvertToMPT(*pIns, m_nType);

		if(m_nType == MOD_TYPE_XM)
			instrVibrato[ins - 1] = insHeader.vibrato;
	}
	if(isSampleMode)
		m_nInstruments = 0;

	std::vector<MO3SampleInfo> sampleInfos;
	const bool frequencyIsHertz = (version >= 5 || !(fileHeader.flags & MO3FileHeader::linearSlides));
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		ModSample &sample = Samples[smp];
		std::string name;
		musicChunk.ReadNullString(name);
		m_szNames[smp] = name;
		if(version >= 5)
			musicChunk.ReadNullString(name);
		else
			name.clear();

		MO3Sample smpHeader;
		if(!musicChunk.ReadStruct(smpHeader))
			return false;
		smpHeader.ConvertToMPT(sample, m_nType, frequencyIsHertz);
		sample.filename = name;

		int16 sharedOggHeader = 0;
		if(version >= 5 && (smpHeader.flags & MO3Sample::smpCompressionMask) == MO3Sample::smpSharedOgg)
		{
			sharedOggHeader = musicChunk.ReadInt16LE();
		}

		if(loadFlags & loadSampleData)
		{
			sampleInfos.reserve(m_nSamples);
			sampleInfos.emplace_back(smpHeader, sharedOggHeader);
		}
	}

	if(m_nType == MOD_TYPE_XM)
	{
		// Transfer XM instrument vibrato to samples
		for(INSTRUMENTINDEX ins = 0; ins < m_nInstruments; ins++)
		{
			PropagateXMAutoVibrato(ins + 1, static_cast<VibratoType>(instrVibrato[ins].type.get()), instrVibrato[ins].sweep, instrVibrato[ins].depth, instrVibrato[ins].rate);
		}
	}

	if((fileHeader.flags & MO3FileHeader::hasPlugins) && musicChunk.CanRead(1))
	{
		// Plugin data
		uint8 pluginFlags = musicChunk.ReadUint8();
		if(pluginFlags & 1)
		{
			// Channel plugins
			for(auto &chn : ChnSettings)
			{
				chn.nMixPlugin = static_cast<PLUGINDEX>(musicChunk.ReadUint32LE());
			}
		}
		while(musicChunk.CanRead(1))
		{
			PLUGINDEX plug = musicChunk.ReadUint8();
			if(!plug)
				break;
			uint32 len = musicChunk.ReadUint32LE();
			if(len >= containerHeader.musicSize || containerHeader.musicSize - len < musicChunk.GetPosition())
				return false;
			FileReader pluginChunk = musicChunk.ReadChunk(len);
#ifndef NO_PLUGINS
			if(plug <= MAX_MIXPLUGINS)
			{
				ReadMixPluginChunk(pluginChunk, m_MixPlugins[plug - 1]);
			}
#endif  // NO_PLUGINS
		}
	}

	mpt::ustring madeWithTracker;
	uint16 cwtv = 0;
	uint16 cmwt = 0;
	while(musicChunk.CanRead(8))
	{
		uint32 id = musicChunk.ReadUint32LE();
		uint32 len = musicChunk.ReadUint32LE();
		if(len >= containerHeader.musicSize || containerHeader.musicSize - len < musicChunk.GetPosition())
			return false;
		FileReader chunk = musicChunk.ReadChunk(len);
		switch(id)
		{
		case MagicLE("VERS"):
			// Tracker magic bytes (depending on format)
			switch(m_nType)
			{
			case MOD_TYPE_IT:
				cwtv = chunk.ReadUint16LE();
				cmwt = chunk.ReadUint16LE();
				/*switch(cwtv >> 12)
				{
					
				}*/
				break;
			case MOD_TYPE_S3M:
				cwtv = chunk.ReadUint16LE();
				break;
			case MOD_TYPE_XM:
				chunk.ReadString<mpt::String::spacePadded>(madeWithTracker, mpt::Charset::CP437, std::min(FileReader::pos_type(32), chunk.GetLength()));
				break;
			case MOD_TYPE_MTM:
			{
				uint8 mtmVersion = chunk.ReadUint8();
				madeWithTracker = MPT_UFORMAT("MultiTracker {}.{}")(mtmVersion >> 4, mtmVersion & 0x0F);
			}
			break;
			default:
				break;
			}
			break;
		case MagicLE("PRHI"):
			m_nDefaultRowsPerBeat = chunk.ReadUint8();
			m_nDefaultRowsPerMeasure = chunk.ReadUint8();
			break;
		case MagicLE("MIDI"):
			// Full MIDI config
			chunk.ReadStruct<MIDIMacroConfigData>(m_MidiCfg);
			m_MidiCfg.Sanitize();
			break;
		case MagicLE("OMPT"):
			// Read pattern names: "PNAM"
			if(chunk.ReadMagic("PNAM"))
			{
				FileReader patterns = chunk.ReadChunk(chunk.ReadUint32LE());
				const PATTERNINDEX namedPats = std::min(static_cast<PATTERNINDEX>(patterns.GetLength() / MAX_PATTERNNAME), Patterns.Size());

				for(PATTERNINDEX pat = 0; pat < namedPats; pat++)
				{
					char patName[MAX_PATTERNNAME];
					patterns.ReadString<mpt::String::maybeNullTerminated>(patName, MAX_PATTERNNAME);
					Patterns[pat].SetName(patName);
				}
			}

			// Read channel names: "CNAM"
			if(chunk.ReadMagic("CNAM"))
			{
				FileReader channels = chunk.ReadChunk(chunk.ReadUint32LE());
				const CHANNELINDEX namedChans = std::min(static_cast<CHANNELINDEX>(channels.GetLength() / MAX_CHANNELNAME), GetNumChannels());
				for(CHANNELINDEX chn = 0; chn < namedChans; chn++)
				{
					channels.ReadString<mpt::String::maybeNullTerminated>(ChnSettings[chn].szName, MAX_CHANNELNAME);
				}
			}

			LoadExtendedInstrumentProperties(chunk);
			LoadExtendedSongProperties(chunk, true);
			if(cwtv > 0x0889 && cwtv <= 0x8FF)
			{
				m_nType = MOD_TYPE_MPT;
				LoadMPTMProperties(chunk, cwtv);
			}

			if(m_dwLastSavedWithVersion)
			{
				madeWithTracker = UL_("OpenMPT ") + mpt::ufmt::val(m_dwLastSavedWithVersion);
			}
			break;
		}
	}

	if((GetType() == MOD_TYPE_IT && cwtv >= 0x0100 && cwtv < 0x0214)
	   || (GetType() == MOD_TYPE_S3M && cwtv >= 0x3100 && cwtv < 0x3214)
	   || (GetType() == MOD_TYPE_S3M && cwtv >= 0x1300 && cwtv < 0x1320))
	{
		// Ignore MIDI data in files made with IT older than version 2.14 and old ST3 versions.
		m_MidiCfg.ClearZxxMacros();
	}

	if(fileHeader.flags & MO3FileHeader::modplugMode)
	{
		// Apply some old ModPlug (mis-)behaviour
		if(!m_dwLastSavedWithVersion)
		{
			// These fixes are only applied when the OpenMPT version number is not known, as otherwise the song upgrade feature will take care of it.
			for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
			{
				if(ModInstrument *ins = Instruments[i])
				{
					// Fix pitch / filter envelope being shortened by one tick (for files before v1.20)
					ins->GetEnvelope(ENV_PITCH).Convert(MOD_TYPE_XM, GetType());
					// Fix excessive pan swing range (for files before v1.26)
					ins->nPanSwing = static_cast<uint8>((ins->nPanSwing + 3) / 4u);
				}
			}
		}
		if(m_dwLastSavedWithVersion < MPT_V("1.18.00.00"))
		{
			m_playBehaviour.reset(kITOffset);
			m_playBehaviour.reset(kFT2ST3OffsetOutOfRange);
		}
		if(m_dwLastSavedWithVersion < MPT_V("1.23.00.00"))
			m_playBehaviour.reset(kFT2Periods);
		if(m_dwLastSavedWithVersion < MPT_V("1.26.00.00"))
			m_playBehaviour.reset(kITInstrWithNoteOff);
	}

	if(madeWithTracker.empty())
		madeWithTracker = MPT_UFORMAT("MO3 v{}")(version);
	else
		madeWithTracker = MPT_UFORMAT("MO3 v{} ({})")(version, madeWithTracker);

	m_modFormat.formatName = MPT_UFORMAT("Un4seen MO3 v{}")(version);
	m_modFormat.type = UL_("mo3");

	switch(GetType())
	{
	case MOD_TYPE_MTM:
		m_modFormat.originalType = UL_("mtm");
		m_modFormat.originalFormatName = UL_("MultiTracker");
		break;
	case MOD_TYPE_MOD:
		m_modFormat.originalType = UL_("mod");
		m_modFormat.originalFormatName = UL_("Generic MOD");
		break;
	case MOD_TYPE_XM:
		m_modFormat.originalType = UL_("xm");
		m_modFormat.originalFormatName = UL_("FastTracker 2");
		break;
	case MOD_TYPE_S3M:
		m_modFormat.originalType = UL_("s3m");
		m_modFormat.originalFormatName = UL_("Scream Tracker 3");
		break;
	case MOD_TYPE_IT:
		m_modFormat.originalType = UL_("it");
		if(cmwt)
			m_modFormat.originalFormatName = MPT_UFORMAT("Impulse Tracker {}.{}")(cmwt >> 8, mpt::ufmt::hex0<2>(cmwt & 0xFF));
		else
			m_modFormat.originalFormatName = UL_("Impulse Tracker");
		break;
	case MOD_TYPE_MPT:
		m_modFormat.originalType = UL_("mptm");
		m_modFormat.originalFormatName = UL_("OpenMPT MPTM");
		break;
	default:
		MPT_ASSERT_NOTREACHED();
	}
	m_modFormat.madeWithTracker = std::move(madeWithTracker);
	if(m_dwLastSavedWithVersion)
		m_modFormat.charset = mpt::Charset::Windows1252;
	else if(GetType() == MOD_TYPE_MOD)
		m_modFormat.charset = mpt::Charset::Amiga_no_C1;
	else
		m_modFormat.charset = mpt::Charset::CP437;

	if(!(loadFlags & loadSampleData))
		return true;

	if(containerHeader.version < 5)
	{
		// As we don't know where the compressed data ends, we don't know where the sample data starts, either.
		if(!musicChunkData->UnpackedSuccessfully())
			return false;
		file.Seek(musicChunkData->SourcePosition());
	} else
	{
		file.Seek(12 + compressedSize);
	}

	bool unsupportedSamples = false;
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		MO3SampleInfo &smpInfo = sampleInfos[smp - 1];
		const MO3Sample &smpHeader = smpInfo.smpHeader;
		const uint32 compression = (smpHeader.flags & MO3Sample::smpCompressionMask);

		if(!compression && smpHeader.compressedSize == 0)
		{
			// Uncompressed sample
			SampleIO(
				(smpHeader.flags & MO3Sample::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
				(smpHeader.flags & MO3Sample::smpStereo) ? SampleIO::stereoSplit : SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(Samples[smp], file);
		} else if(smpHeader.compressedSize > 0)
		{
			// Compressed sample; we read those in a second pass because Ogg samples with shared headers may reference a later sample's header
			smpInfo.chunk = file.ReadChunk(smpHeader.compressedSize);
		}
	}

	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		ModSample &sample = Samples[smp];
		MO3SampleInfo &smpInfo = sampleInfos[smp - 1];
		const MO3Sample &smpHeader = smpInfo.smpHeader;

		if(smpHeader.compressedSize < 0 && (smp + smpHeader.compressedSize) > 0)
		{
			// Duplicate sample
			sample.CopyWaveform(Samples[smp + smpHeader.compressedSize]);
			continue;
		}

		// Not a compressed sample?
		if(!smpHeader.length || !smpInfo.chunk.IsValid())
			continue;

		if(smpHeader.flags & MO3Sample::smp16Bit)
			sample.uFlags.set(CHN_16BIT);
		if(smpHeader.flags & MO3Sample::smpStereo)
			sample.uFlags.set(CHN_STEREO);

		FileReader &sampleData = smpInfo.chunk;
		const uint8 numChannels = sample.GetNumChannels();
		const uint32 compression = (smpHeader.flags & MO3Sample::smpCompressionMask);

		if(compression == MO3Sample::smpDeltaCompression || compression == MO3Sample::smpDeltaPrediction)
		{
			// In the best case, MO3 compression represents each sample point as two bits.
			// As a result, if we have a file length of n, we know that the sample can be at most n*4 sample points long.
			auto maxLength = sampleData.GetLength();
			uint8 maxSamplesPerByte = 4 / numChannels;
			if(Util::MaxValueOfType(maxLength) / maxSamplesPerByte >= maxLength)
				maxLength *= maxSamplesPerByte;
			else
				maxLength = Util::MaxValueOfType(maxLength);
			LimitMax(sample.nLength, mpt::saturate_cast<SmpLength>(maxLength));
		}

		if(compression == MO3Sample::smpDeltaCompression)
		{
			if(sample.AllocateSample())
			{
				if(smpHeader.flags & MO3Sample::smp16Bit)
					UnpackMO3DeltaSample<MO3Delta16BitParams>(sampleData, sample.sample16(), sample.nLength, numChannels);
				else
					UnpackMO3DeltaSample<MO3Delta8BitParams>(sampleData, sample.sample8(), sample.nLength, numChannels);
			}
		} else if(compression == MO3Sample::smpDeltaPrediction)
		{
			if(sample.AllocateSample())
			{
				if(smpHeader.flags & MO3Sample::smp16Bit)
					UnpackMO3DeltaPredictionSample<MO3Delta16BitParams>(sampleData, sample.sample16(), sample.nLength, numChannels);
				else
					UnpackMO3DeltaPredictionSample<MO3Delta8BitParams>(sampleData, sample.sample8(), sample.nLength, numChannels);
			}
		} else if(compression == MO3Sample::smpCompressionOgg || compression == MO3Sample::smpSharedOgg)
		{
			const uint16 sharedHeaderSize = smpHeader.encoderDelay;
			SAMPLEINDEX sharedOggHeader = (smp + smpInfo.sharedHeader > 0) ? static_cast<SAMPLEINDEX>(smp + smpInfo.sharedHeader) : smp;
			// Which chunk are we going to read the header from?
			// Note: Every Ogg stream has a unique serial number.
			// stb_vorbis (currently) ignores this serial number so we can just stitch
			// together our sample without adjusting the shared header's serial number.
			const bool sharedHeader = sharedOggHeader != smp && sharedOggHeader > 0 && sharedOggHeader <= m_nSamples && sharedHeaderSize > 0;

#if defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)

			std::vector<char> mergedData;
			if(sharedHeader)
			{
				// Prepend the shared header to the actual sample data and adjust bitstream serial numbers.
				// We do not handle multiple muxed logical streams as they do not exist in practice in mo3.
				// We assume sequence numbers are consecutive at the end of the headers.
				// Corrupted pages get dropped as required by Ogg spec. We cannot do any further sane parsing on them anyway.
				// We do not match up multiple muxed stream properly as this would need parsing of actual packet data to determine or guess the codec.
				// Ogg Vorbis files may contain at least an additional Ogg Skeleton stream. It is not clear whether these actually exist in MO3.
				// We do not validate packet structure or logical bitstream structure (i.e. sequence numbers and granule positions).

				// TODO: At least handle Skeleton streams here, as they violate our stream ordering assumptions here.

#if 0
				// This block may still turn out to be useful as it does a more thourough validation of the stream than the optimized version below.

				// We copy the whole data into a single consecutive buffer in order to keep things simple when interfacing libvorbisfile.
				// We could in theory only adjust the header and pass 2 chunks to libvorbisfile.
				// Another option would be to demux both chunks on our own (or using libogg) and pass the raw packet data to libvorbis directly.

				std::ostringstream mergedStream(std::ios::binary);
				mergedStream.imbue(std::locale::classic());

				sampleInfos[sharedOggHeader - 1].chunk.Rewind();
				FileReader sharedChunk = sampleInfos[sharedOggHeader - 1].chunk.ReadChunk(sharedHeaderSize);
				sharedChunk.Rewind();

				std::vector<uint32> streamSerials;
				Ogg::PageInfo oggPageInfo;
				std::vector<uint8> oggPageData;

				streamSerials.clear();
				while(Ogg::ReadPageAndSkipJunk(sharedChunk, oggPageInfo, oggPageData))
				{
					auto it = std::find(streamSerials.begin(), streamSerials.end(), oggPageInfo.header.bitstream_serial_number);
					if(it == streamSerials.end())
					{
						streamSerials.push_back(oggPageInfo.header.bitstream_serial_number);
						it = streamSerials.begin() + (streamSerials.size() - 1);
					}
					uint32 newSerial = it - streamSerials.begin() + 1;
					oggPageInfo.header.bitstream_serial_number = newSerial;
					Ogg::UpdatePageCRC(oggPageInfo, oggPageData);
					Ogg::WritePage(mergedStream, oggPageInfo, oggPageData);
				}

				streamSerials.clear();
				while(Ogg::ReadPageAndSkipJunk(smpInfo.chunk, oggPageInfo, oggPageData))
				{
					auto it = std::find(streamSerials.begin(), streamSerials.end(), oggPageInfo.header.bitstream_serial_number);
					if(it == streamSerials.end())
					{
						streamSerials.push_back(oggPageInfo.header.bitstream_serial_number);
						it = streamSerials.begin() + (streamSerials.size() - 1);
					}
					uint32 newSerial = it - streamSerials.begin() + 1;
					oggPageInfo.header.bitstream_serial_number = newSerial;
					Ogg::UpdatePageCRC(oggPageInfo, oggPageData);
					Ogg::WritePage(mergedStream, oggPageInfo, oggPageData);
				}

				std::string mergedStreamData = mergedStream.str();
				mergedData.insert(mergedData.end(), mergedStreamData.begin(), mergedStreamData.end());

#else

				// We assume same ordering of streams in both header and data if
				// multiple streams are present.

				std::ostringstream mergedStream(std::ios::binary);
				mergedStream.imbue(std::locale::classic());

				sampleInfos[sharedOggHeader - 1].chunk.Rewind();
				FileReader sharedChunk = sampleInfos[sharedOggHeader - 1].chunk.ReadChunk(sharedHeaderSize);
				sharedChunk.Rewind();

				std::vector<uint32> dataStreamSerials;
				std::vector<uint32> headStreamSerials;
				Ogg::PageInfo oggPageInfo;
				std::vector<uint8> oggPageData;

				// Gather bitstream serial numbers form sample data chunk
				dataStreamSerials.clear();
				while(Ogg::ReadPageAndSkipJunk(smpInfo.chunk, oggPageInfo, oggPageData))
				{
					if(!mpt::contains(dataStreamSerials, oggPageInfo.header.bitstream_serial_number))
					{
						dataStreamSerials.push_back(oggPageInfo.header.bitstream_serial_number);
					}
				}

				// Apply the data bitstream serial numbers to the header
				headStreamSerials.clear();
				while(Ogg::ReadPageAndSkipJunk(sharedChunk, oggPageInfo, oggPageData))
				{
					auto it = std::find(headStreamSerials.begin(), headStreamSerials.end(), oggPageInfo.header.bitstream_serial_number);
					if(it == headStreamSerials.end())
					{
						headStreamSerials.push_back(oggPageInfo.header.bitstream_serial_number);
						it = headStreamSerials.begin() + (headStreamSerials.size() - 1);
					}
					uint32 newSerial = 0;
					if(dataStreamSerials.size() >= static_cast<std::size_t>(it - headStreamSerials.begin()))
					{
						// Found corresponding stream in data chunk.
						newSerial = dataStreamSerials[it - headStreamSerials.begin()];
					} else
					{
						// No corresponding stream in data chunk. Find a free serialno.
						std::size_t extraIndex = (it - headStreamSerials.begin()) - dataStreamSerials.size();
						for(newSerial = 1; newSerial < 0xffffffffu; ++newSerial)
						{
							if(!mpt::contains(dataStreamSerials, newSerial))
							{
								extraIndex -= 1;
							}
							if(extraIndex == 0)
							{
								break;
							}
						}
					}
					oggPageInfo.header.bitstream_serial_number = newSerial;
					Ogg::UpdatePageCRC(oggPageInfo, oggPageData);
					Ogg::WritePage(mergedStream, oggPageInfo, oggPageData);
				}

				if(headStreamSerials.size() > 1)
				{
					AddToLog(LogWarning, MPT_UFORMAT("Sample {}: Ogg Vorbis data with shared header and multiple logical bitstreams in header chunk found. This may be handled incorrectly.")(smp));
				} else if(dataStreamSerials.size() > 1)
				{
					AddToLog(LogWarning, MPT_UFORMAT("Sample {}: Ogg Vorbis sample with shared header and multiple logical bitstreams found. This may be handled incorrectly.")(smp));
				} else if((dataStreamSerials.size() == 1) && (headStreamSerials.size() == 1) && (dataStreamSerials[0] != headStreamSerials[0]))
				{
					AddToLog(LogInformation, MPT_UFORMAT("Sample {}: Ogg Vorbis data with shared header and different logical bitstream serials found.")(smp));
				}

				std::string mergedStreamData = mergedStream.str();
				mergedData.insert(mergedData.end(), mergedStreamData.begin(), mergedStreamData.end());

				smpInfo.chunk.Rewind();
				FileReader::PinnedView sampleChunkView = smpInfo.chunk.GetPinnedView();
				mpt::span<const char> sampleChunkViewSpan = mpt::byte_cast<mpt::span<const char>>(sampleChunkView.span());
				mergedData.insert(mergedData.end(), sampleChunkViewSpan.begin(), sampleChunkViewSpan.end());

#endif
			}
			FileReader mergedDataChunk(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(mergedData)));

			FileReader &sampleChunk = sharedHeader ? mergedDataChunk : smpInfo.chunk;
			FileReader &headerChunk = sampleChunk;

#else  // !(MPT_WITH_VORBIS && MPT_WITH_VORBISFILE)

			FileReader &headerChunk = sharedHeader ? sampleInfos[sharedOggHeader - 1].chunk : sampleData;
#if defined(MPT_WITH_STBVORBIS)
			std::size_t initialRead = sharedHeader ? sharedHeaderSize : headerChunk.GetLength();
#endif  // MPT_WITH_STBVORBIS

#endif  // MPT_WITH_VORBIS && MPT_WITH_VORBISFILE

			headerChunk.Rewind();
			if(sharedHeader && !headerChunk.CanRead(sharedHeaderSize))
				continue;

#if defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)

			ov_callbacks callbacks = {
			    &VorbisfileFilereaderRead,
			    &VorbisfileFilereaderSeek,
			    nullptr,
			    &VorbisfileFilereaderTell};
			OggVorbis_File vf;
			MemsetZero(vf);
			if(ov_open_callbacks(mpt::void_ptr<FileReader>(&sampleChunk), &vf, nullptr, 0, callbacks) == 0)
			{
				if(ov_streams(&vf) == 1)
				{  // we do not support chained vorbis samples
					vorbis_info *vi = ov_info(&vf, -1);
					if(vi && vi->rate > 0 && vi->channels > 0)
					{
						sample.AllocateSample();
						SmpLength offset = 0;
						int channels = vi->channels;
						int current_section = 0;
						long decodedSamples = 0;
						bool eof = false;
						while(!eof && offset < sample.nLength && sample.HasSampleData())
						{
							float **output = nullptr;
							long ret = ov_read_float(&vf, &output, 1024, &current_section);
							if(ret == 0)
							{
								eof = true;
							} else if(ret < 0)
							{
								// stream error, just try to continue
							} else
							{
								decodedSamples = ret;
								LimitMax(decodedSamples, mpt::saturate_cast<long>(sample.nLength - offset));
								if(decodedSamples > 0 && channels == sample.GetNumChannels())
								{
									if(sample.uFlags[CHN_16BIT])
									{
										CopyAudio(mpt::audio_span_interleaved(sample.sample16() + (offset * sample.GetNumChannels()), sample.GetNumChannels(), decodedSamples), mpt::audio_span_planar(output, channels, decodedSamples));
									} else
									{
										CopyAudio(mpt::audio_span_interleaved(sample.sample8() + (offset * sample.GetNumChannels()), sample.GetNumChannels(), decodedSamples), mpt::audio_span_planar(output, channels, decodedSamples));
									}
								}
								offset += static_cast<SmpLength>(decodedSamples);
							}
						}
					} else
					{
						unsupportedSamples = true;
					}
				} else
				{
					AddToLog(LogWarning, MPT_UFORMAT("Sample {}: Unsupported Ogg Vorbis chained stream found.")(smp));
					unsupportedSamples = true;
				}
				ov_clear(&vf);
			} else
			{
				unsupportedSamples = true;
			}

#elif defined(MPT_WITH_STBVORBIS)

			// NOTE/TODO: stb_vorbis does not handle inferred negative PCM sample
			// position at stream start. (See
			// <https://www.xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-132000A.2>).
			// This means that, for remuxed and re-aligned/cutted (at stream start)
			// Vorbis files, stb_vorbis will include superfluous samples at the
			// beginning. MO3 files with this property are yet to be spotted in the
			// wild, thus, this behaviour is currently not problematic.

			int consumed = 0, error = 0;
			stb_vorbis *vorb = nullptr;
			if(sharedHeader)
			{
				FileReader::PinnedView headChunkView = headerChunk.GetPinnedView(initialRead);
				vorb = stb_vorbis_open_pushdata(mpt::byte_cast<const unsigned char *>(headChunkView.data()), mpt::saturate_cast<int>(headChunkView.size()), &consumed, &error, nullptr);
				headerChunk.Skip(consumed);
			}
			FileReader::PinnedView sampleDataView = sampleData.GetPinnedView();
			const std::byte *data = sampleDataView.data();
			std::size_t dataLeft = sampleDataView.size();
			if(!sharedHeader)
			{
				vorb = stb_vorbis_open_pushdata(mpt::byte_cast<const unsigned char *>(data), mpt::saturate_cast<int>(dataLeft), &consumed, &error, nullptr);
				sampleData.Skip(consumed);
				data += consumed;
				dataLeft -= consumed;
			}
			if(vorb)
			{
				// Header has been read, proceed to reading the sample data
				sample.AllocateSample();
				SmpLength offset = 0;
				while((error == VORBIS__no_error || (error == VORBIS_need_more_data && dataLeft > 0))
				      && offset < sample.nLength && sample.HasSampleData())
				{
					int channels = 0, decodedSamples = 0;
					float **output;
					consumed = stb_vorbis_decode_frame_pushdata(vorb, mpt::byte_cast<const unsigned char *>(data), mpt::saturate_cast<int>(dataLeft), &channels, &output, &decodedSamples);
					sampleData.Skip(consumed);
					data += consumed;
					dataLeft -= consumed;
					LimitMax(decodedSamples, mpt::saturate_cast<int>(sample.nLength - offset));
					if(decodedSamples > 0 && channels == sample.GetNumChannels())
					{
						if(sample.uFlags[CHN_16BIT])
						{
							CopyAudio(mpt::audio_span_interleaved(sample.sample16() + (offset * sample.GetNumChannels()), sample.GetNumChannels(), decodedSamples), mpt::audio_span_planar(output, channels, decodedSamples));
						} else
						{
							CopyAudio(mpt::audio_span_interleaved(sample.sample8() + (offset * sample.GetNumChannels()), sample.GetNumChannels(), decodedSamples), mpt::audio_span_planar(output, channels, decodedSamples));
						}
					}
					offset += decodedSamples;
					error = stb_vorbis_get_error(vorb);
				}
				stb_vorbis_close(vorb);
			} else
			{
				unsupportedSamples = true;
			}

#else  // !VORBIS

			unsupportedSamples = true;

#endif  // VORBIS
		} else if(compression == MO3Sample::smpCompressionMPEG)
		{
			// Old MO3 encoders didn't remove LAME info frames. This is unfortunate since the encoder delay
			// specified in the sample header does not take the gapless information from the LAME info frame
			// into account. We should not depend on the MP3 decoder's capabilities to read or ignore such frames:
			// - libmpg123 has MPG123_IGNORE_INFOFRAME but that requires API version 31 (mpg123 v1.14) or higher
			// - Media Foundation does (currently) not read LAME gapless information at all
			// So we just play safe and remove such frames.
			FileReader mpegData(sampleData);
			MPEGFrame frame(sampleData);
			uint16 encoderDelay = smpHeader.encoderDelay;
			uint16 frameDelay = frame.numSamples * 2;
			if(frame.isLAME && encoderDelay >= frameDelay)
			{
				// The info frame does not produce any output, but still counts towards the encoder delay.
				encoderDelay -= frameDelay;
				sampleData.Seek(frame.frameSize);
				mpegData = sampleData.ReadChunk(sampleData.BytesLeft());
			}

			if(ReadMP3Sample(smp, mpegData, true, true) || ReadMediaFoundationSample(smp, mpegData, true))
			{
				if(encoderDelay > 0 && encoderDelay < sample.GetSampleSizeInBytes())
				{
					SmpLength delay = encoderDelay / sample.GetBytesPerSample();
					memmove(sample.sampleb(), sample.sampleb() + encoderDelay, sample.GetSampleSizeInBytes() - encoderDelay);
					sample.nLength -= delay;
				}
				LimitMax(sample.nLength, smpHeader.length);
			} else
			{
				unsupportedSamples = true;
			}
		} else if(compression == MO3Sample::smpOPLInstrument)
		{
			OPLPatch patch;
			if(sampleData.ReadArray(patch))
			{
				sample.SetAdlib(true, patch);
			}
		} else
		{
			unsupportedSamples = true;
		}
	}

	if(unsupportedSamples)
	{
		AddToLog(LogWarning, U_("Some compressed samples could not be loaded because they use an unsupported codec."));
	}

	return true;
}


OPENMPT_NAMESPACE_END
