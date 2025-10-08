/*
 * load_j2b.cpp
 * ------------
 * Purpose: RIFF AM and RIFF AMFF (Galaxy Sound System) module loader
 * Notes  : J2B is a compressed variant of RIFF AM and RIFF AMFF files used in Jazz Jackrabbit 2.
 *          It seems like no other game used the AM(FF) format.
 *          RIFF AM is the newer version of the format, generally following the RIFF "standard" closely.
 * Authors: Johannes Schultz (OpenMPT port, reverse engineering + loader implementation of the instrument format)
 *          Largely based on the J2B loader of kode54's DUMB fork
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

#include "mpt/io/base.hpp"

#if defined(MPT_WITH_ZLIB)
#include <zlib.h>
#elif defined(MPT_WITH_MINIZ)
#include <miniz/miniz.h>
#endif


#ifdef MPT_ALL_LOGGING
#define J2B_LOG
#endif


OPENMPT_NAMESPACE_BEGIN


// First off, a nice vibrato translation LUT.
static constexpr VibratoType j2bAutoVibratoTrans[] = 
{
	VIB_SINE, VIB_SQUARE, VIB_RAMP_UP, VIB_RAMP_DOWN, VIB_RANDOM,
};


// header for compressed j2b files
struct J2BFileHeader
{
	// Magic Bytes
	// 32-Bit J2B header identifiers
	enum : uint32
	{
		magicDEADBEAF = 0xAFBEADDEu,
		magicDEADBABE = 0xBEBAADDEu
	};

	char     signature[4];    // MUSE
	uint32le deadbeaf;        // 0xDEADBEAF (AM) or 0xDEADBABE (AMFF)
	uint32le fileLength;      // complete filesize
	uint32le crc32;           // checksum of the compressed data block
	uint32le packedLength;    // length of the compressed data block
	uint32le unpackedLength;  // length of the decompressed module
};

MPT_BINARY_STRUCT(J2BFileHeader, 24)


// AM(FF) stuff

struct AMFFRiffChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idRIFF = MagicLE("RIFF"),
		idAMFF = MagicLE("AMFF"),
		idAM__ = MagicLE("AM  "),
		idMAIN = MagicLE("MAIN"),
		idINIT = MagicLE("INIT"),
		idORDR = MagicLE("ORDR"),
		idPATT = MagicLE("PATT"),
		idINST = MagicLE("INST"),
		idSAMP = MagicLE("SAMP"),
		idAI__ = MagicLE("AI  "),
		idAS__ = MagicLE("AS  "),
	};

	uint32le id;      // See ChunkIdentifiers
	uint32le length;  // Chunk size without header

	size_t GetLength() const
	{
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(AMFFRiffChunk, 8)


// This header is used for both AM's "INIT" as well as AMFF's "MAIN" chunk
struct AMFFMainChunk
{
	// Main Chunk flags
	enum MainFlags
	{
		amigaSlides = 0x01,
	};

	char     songname[64];
	uint8le  flags;
	uint8le  channels;
	uint8le  speed;
	uint8le  tempo;
	uint16le minPeriod;  // 16x Amiga periods, but we should ignore them - otherwise some high notes in Medivo.j2b won't sound correct.
	uint16le maxPeriod;  // Ditto
	uint8le  globalvolume;
};

MPT_BINARY_STRUCT(AMFFMainChunk, 73)


// AMFF instrument envelope (old format)
struct AMFFEnvelope
{
	// Envelope flags (also used for RIFF AM)
	enum EnvelopeFlags
	{
		envEnabled	= 0x01,
		envSustain	= 0x02,
		envLoop		= 0x04,
	};

	struct EnvPoint
	{
		uint16le tick;
		uint8le  value;  // 0...64
	};

	uint8le envFlags;          // high nibble = pan env flags, low nibble = vol env flags (both nibbles work the same way)
	uint8le envNumPoints;      // high nibble = pan env length, low nibble = vol env length
	uint8le envSustainPoints;  // you guessed it... high nibble = pan env sustain point, low nibble = vol env sustain point
	uint8le envLoopStarts;     // I guess you know the pattern now.
	uint8le envLoopEnds;       // same here.
	EnvPoint volEnv[10];
	EnvPoint panEnv[10];

	// Convert weird envelope data to OpenMPT's internal format.
	void ConvertEnvelope(uint8 flags, uint8 numPoints, uint8 sustainPoint, uint8 loopStart, uint8 loopEnd, const EnvPoint (&points)[10], InstrumentEnvelope &mptEnv) const
	{
		// The buggy mod2j2b converter will actually NOT limit this to 10 points if the envelope is longer.
		mptEnv.resize(std::min(numPoints, static_cast<uint8>(10)));

		mptEnv.nSustainStart = mptEnv.nSustainEnd = sustainPoint;

		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;

		for(uint32 i = 0; i < mptEnv.size(); i++)
		{
			mptEnv[i].tick = points[i].tick >> 4;
			if(i == 0)
				mptEnv[0].tick = 0;
			else if(mptEnv[i].tick < mptEnv[i - 1].tick)
				mptEnv[i].tick = mptEnv[i - 1].tick + 1;

			mptEnv[i].value = Clamp<uint8, uint8>(points[i].value, 0, 64);
		}

		mptEnv.dwFlags.set(ENV_ENABLED, (flags & AMFFEnvelope::envEnabled) != 0);
		mptEnv.dwFlags.set(ENV_SUSTAIN, (flags & AMFFEnvelope::envSustain) && mptEnv.nSustainStart <= mptEnv.size());
		mptEnv.dwFlags.set(ENV_LOOP, (flags & AMFFEnvelope::envLoop) && mptEnv.nLoopStart <= mptEnv.nLoopEnd && mptEnv.nLoopStart <= mptEnv.size());
	}

	void ConvertToMPT(ModInstrument &mptIns) const
	{
		// interleaved envelope data... meh. gotta split it up here and decode it separately.
		// note: mod2j2b is BUGGY and always writes ($original_num_points & 0x0F) in the header,
		// but just has room for 10 envelope points. That means that long (>= 16 points)
		// envelopes are cut off, and envelopes have to be trimmed to 10 points, even if
		// the header claims that they are longer.
		// For XM files the number of points also appears to be off by one,
		// but luckily there are no official J2Bs using envelopes anyway.
		ConvertEnvelope(envFlags & 0x0F, envNumPoints & 0x0F, envSustainPoints & 0x0F, envLoopStarts & 0x0F, envLoopEnds & 0x0F, volEnv, mptIns.VolEnv);
		ConvertEnvelope(envFlags >> 4, envNumPoints >> 4, envSustainPoints >> 4, envLoopStarts >> 4, envLoopEnds >> 4, panEnv, mptIns.PanEnv);
	}
};

MPT_BINARY_STRUCT(AMFFEnvelope::EnvPoint, 3)
MPT_BINARY_STRUCT(AMFFEnvelope, 65)


// AMFF instrument header (old format)
struct AMFFInstrumentHeader
{
	uint8le  unknown;  // 0x00
	uint8le  index;    // actual instrument number
	char     name[28];
	uint8le  numSamples;
	uint8le  sampleMap[120];
	uint8le  vibratoType;
	uint16le vibratoSweep;
	uint16le vibratoDepth;
	uint16le vibratoRate;
	AMFFEnvelope envelopes;
	uint16le fadeout;

	// Convert instrument data to OpenMPT's internal format.
	void ConvertToMPT(ModInstrument &mptIns, SAMPLEINDEX baseSample)
	{
		mptIns.name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, name);

		static_assert(mpt::array_size<decltype(sampleMap)>::size <= mpt::array_size<decltype(mptIns.Keyboard)>::size);
		for(size_t i = 0; i < std::size(sampleMap); i++)
		{
			mptIns.Keyboard[i] = static_cast<SAMPLEINDEX>(sampleMap[i] + baseSample + 1);
		}

		mptIns.nFadeOut = fadeout << 5;
		envelopes.ConvertToMPT(mptIns);
	}

};

MPT_BINARY_STRUCT(AMFFInstrumentHeader, 225)


// AMFF sample header (old format)
struct AMFFSampleHeader
{
	// Sample flags (also used for RIFF AM)
	enum SampleFlags
	{
		smp16Bit    = 0x04,
		smpLoop     = 0x08,
		smpPingPong = 0x10,
		smpPanning  = 0x20,
		smpExists   = 0x80,
		// some flags are still missing... what is e.g. 0x8000?
	};

	uint32le id;         // "SAMP"
	uint32le chunkSize;  // header + sample size
	char     name[28];
	uint8le  pan;
	uint8le  volume;
	uint16le flags;
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
	uint32le sampleRate;
	uint32le reserved1;
	uint32le reserved2;

	// Convert sample header to OpenMPT's internal format.
	void ConvertToMPT(AMFFInstrumentHeader &instrHeader, ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nPan = pan * 4;
		mptSmp.nVolume = volume * 4;
		mptSmp.nGlobalVol = 64;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nC5Speed = sampleRate;

		if(instrHeader.vibratoType < std::size(j2bAutoVibratoTrans))
			mptSmp.nVibType = j2bAutoVibratoTrans[instrHeader.vibratoType];
		mptSmp.nVibSweep = static_cast<uint8>(instrHeader.vibratoSweep);
		mptSmp.nVibRate = static_cast<uint8>(instrHeader.vibratoRate / 16);
		mptSmp.nVibDepth = static_cast<uint8>(instrHeader.vibratoDepth / 4);
		if((mptSmp.nVibRate | mptSmp.nVibDepth) != 0)
		{
			// Convert XM-style vibrato sweep to IT
			mptSmp.nVibSweep = 255 - mptSmp.nVibSweep;
		}

		if(flags & AMFFSampleHeader::smp16Bit)
			mptSmp.uFlags.set(CHN_16BIT);
		if(flags & AMFFSampleHeader::smpLoop)
			mptSmp.uFlags.set(CHN_LOOP);
		if(flags & AMFFSampleHeader::smpPingPong)
			mptSmp.uFlags.set(CHN_PINGPONGLOOP);
		if(flags & AMFFSampleHeader::smpPanning)
			mptSmp.uFlags.set(CHN_PANNING);
	}

	// Retrieve the internal sample format flags for this sample.
	SampleIO GetSampleFormat() const
	{
		return SampleIO(
			(flags & AMFFSampleHeader::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM);
	}
};

MPT_BINARY_STRUCT(AMFFSampleHeader, 64)


// AM instrument envelope (new format)
struct AMEnvelope
{
	struct EnvPoint
	{
		uint16le tick;
		int16le value;
	};

	uint16le flags;
	uint8le  numPoints;  // actually, it's num. points - 1, and 0xFF if there is no envelope
	uint8le  sustainPoint;
	uint8le  loopStart;
	uint8le  loopEnd;
	EnvPoint values[10];
	uint16le fadeout;  // why is this here? it's only needed for the volume envelope...

	// Convert envelope data to OpenMPT's internal format.
	void ConvertToMPT(InstrumentEnvelope &mptEnv, EnvelopeType envType) const
	{
		if(numPoints == 0xFF || numPoints == 0)
			return;

		mptEnv.resize(std::min(numPoints + 1, 10));

		mptEnv.nSustainStart = mptEnv.nSustainEnd = sustainPoint;

		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;

		int32 scale = 0, offset = 0;
		switch(envType)
		{
		case ENV_VOLUME:  // 0....32767
		default:
			scale = 32767 / ENVELOPE_MAX;
			break;
		case ENV_PITCH:  // -4096....4096
			scale = 8192 / ENVELOPE_MAX;
			offset = 4096;
			break;
		case ENV_PANNING:  // -32768...32767
			scale = 65536 / ENVELOPE_MAX;
			offset = 32768;
			break;
		}

		for(uint32 i = 0; i < mptEnv.size(); i++)
		{
			mptEnv[i].tick = values[i].tick >> 4;
			if(i == 0)
				mptEnv[i].tick = 0;
			else if(mptEnv[i].tick < mptEnv[i - 1].tick)
				mptEnv[i].tick = mptEnv[i - 1].tick + 1;

			int32 val = values[i].value + offset;
			val = (val + scale / 2) / scale;
			mptEnv[i].value = static_cast<EnvelopeNode::value_t>(std::clamp(val, int32(ENVELOPE_MIN), int32(ENVELOPE_MAX)));
		}

		mptEnv.dwFlags.set(ENV_ENABLED, (flags & AMFFEnvelope::envEnabled) != 0);
		mptEnv.dwFlags.set(ENV_SUSTAIN, (flags & AMFFEnvelope::envSustain) && mptEnv.nSustainStart <= mptEnv.size());
		mptEnv.dwFlags.set(ENV_LOOP, (flags & AMFFEnvelope::envLoop) && mptEnv.nLoopStart <= mptEnv.nLoopEnd && mptEnv.nLoopStart <= mptEnv.size());
	}
};

MPT_BINARY_STRUCT(AMEnvelope, 48)


// AM instrument header (new format)
struct AMInstrumentHeader
{
	uint32le headSize;  // Header size (i.e. the size of this struct)
	uint8le  unknown1;  // 0x00
	uint8le  index;     // Actual instrument number
	char     name[32];
	uint8le  sampleMap[128];
	uint8le  vibratoType;
	uint16le vibratoSweep;
	uint16le vibratoDepth;
	uint16le vibratoRate;
	uint8le  unknown2[7];
	AMEnvelope volEnv;
	AMEnvelope pitchEnv;
	AMEnvelope panEnv;
	uint16le numSamples;

	// Convert instrument data to OpenMPT's internal format.
	void ConvertToMPT(ModInstrument &mptIns, SAMPLEINDEX baseSample)
	{
		mptIns.name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, name);

		static_assert(mpt::array_size<decltype(sampleMap)>::size <= mpt::array_size<decltype(mptIns.Keyboard)>::size);
		for(uint8 i = 0; i < std::size(sampleMap); i++)
		{
			mptIns.Keyboard[i] = static_cast<SAMPLEINDEX>(sampleMap[i] + baseSample + 1);
		}

		mptIns.nFadeOut = volEnv.fadeout << 5;

		volEnv.ConvertToMPT(mptIns.VolEnv, ENV_VOLUME);
		pitchEnv.ConvertToMPT(mptIns.PitchEnv, ENV_PITCH);
		panEnv.ConvertToMPT(mptIns.PanEnv, ENV_PANNING);

		if(numSamples == 0)
		{
			MemsetZero(mptIns.Keyboard);
		}
	}
};

MPT_BINARY_STRUCT(AMInstrumentHeader, 326)


// AM sample header (new format)
struct AMSampleHeader
{
	uint32le headSize;  // Header size (i.e. the size of this struct), apparently not including headSize.
	char     name[32];
	uint16le pan;
	uint16le volume;
	uint16le flags;
	uint16le unknown;  // 0x0000 / 0x0080?
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
	uint32le sampleRate;

	// Convert sample header to OpenMPT's internal format.
	void ConvertToMPT(AMInstrumentHeader &instrHeader, ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.nPan = static_cast<uint16>(std::min(pan.get(), uint16(32767)) * 256 / 32767);
		mptSmp.nVolume = static_cast<uint16>(std::min(volume.get(), uint16(32767)) * 256 / 32767);
		mptSmp.nGlobalVol = 64;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nC5Speed = sampleRate;

		if(instrHeader.vibratoType < std::size(j2bAutoVibratoTrans))
			mptSmp.nVibType = j2bAutoVibratoTrans[instrHeader.vibratoType];
		mptSmp.nVibSweep = static_cast<uint8>(instrHeader.vibratoSweep);
		mptSmp.nVibRate = static_cast<uint8>(instrHeader.vibratoRate / 16);
		mptSmp.nVibDepth = static_cast<uint8>(instrHeader.vibratoDepth / 4);
		if((mptSmp.nVibRate | mptSmp.nVibDepth) != 0)
		{
			// Convert XM-style vibrato sweep to IT
			mptSmp.nVibSweep = 255 - mptSmp.nVibSweep;
		}

		if(flags & AMFFSampleHeader::smp16Bit)
			mptSmp.uFlags.set(CHN_16BIT);
		if(flags & AMFFSampleHeader::smpLoop)
			mptSmp.uFlags.set(CHN_LOOP);
		if(flags & AMFFSampleHeader::smpPingPong)
			mptSmp.uFlags.set(CHN_PINGPONGLOOP);
		if(flags & AMFFSampleHeader::smpPanning)
			mptSmp.uFlags.set(CHN_PANNING);
	}

	// Retrieve the internal sample format flags for this sample.
	SampleIO GetSampleFormat() const
	{
		return SampleIO(
			(flags & AMFFSampleHeader::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM);
	}
};

MPT_BINARY_STRUCT(AMSampleHeader, 60)


// Convert RIFF AM(FF) pattern data to MPT pattern data.
static bool ConvertAMPattern(FileReader chunk, PATTERNINDEX pat, bool isAM, CSoundFile &sndFile)
{
	// Effect translation LUT
	static constexpr EffectCommand amEffTrans[] =
	{
		CMD_ARPEGGIO, CMD_PORTAMENTOUP, CMD_PORTAMENTODOWN, CMD_TONEPORTAMENTO,
		CMD_VIBRATO, CMD_TONEPORTAVOL, CMD_VIBRATOVOL, CMD_TREMOLO,
		CMD_PANNING8, CMD_OFFSET, CMD_VOLUMESLIDE, CMD_POSITIONJUMP,
		CMD_VOLUME, CMD_PATTERNBREAK, CMD_MODCMDEX, CMD_TEMPO,
		CMD_GLOBALVOLUME, CMD_GLOBALVOLSLIDE, CMD_KEYOFF, CMD_SETENVPOSITION,
		CMD_CHANNELVOLUME, CMD_CHANNELVOLSLIDE, CMD_PANNINGSLIDE, CMD_RETRIG,
		CMD_TREMOR, CMD_XFINEPORTAUPDOWN,
	};

	enum
	{
		rowDone     = 0x00,  // Advance to next row
		channelMask = 0x1F,  // Mask for retrieving channel information
		volFlag     = 0x20,  // Volume effect present
		noteFlag    = 0x40,  // Note + instr present
		effectFlag  = 0x80,  // Effect information present
		dataFlag    = 0xE0,  // Channel data present
	};

	if(chunk.NoBytesLeft())
		return false;

	const ROWINDEX numRows = std::min(static_cast<ROWINDEX>(chunk.ReadUint8() + 1), MAX_PATTERN_ROWS);

	if(!sndFile.Patterns.Insert(pat, numRows))
		return false;

	const CHANNELINDEX lastChannel = sndFile.GetNumChannels() - 1;
	ROWINDEX row = 0;
	while(row < numRows && chunk.CanRead(1))
	{
		const uint8 flags = chunk.ReadUint8();

		if(flags == rowDone)
		{
			row++;
			continue;
		}
		if(!(flags & dataFlag))
			continue;

		ModCommand &m = *sndFile.Patterns[pat].GetpModCommand(row, std::min(static_cast<CHANNELINDEX>(flags & channelMask), lastChannel));

		if(flags & effectFlag) // effect
		{
			const auto [param, command] = chunk.ReadArray<uint8, 2>();
			if(command < std::size(amEffTrans))
			{
				m.SetEffectCommand(amEffTrans[command], param);
			} else
			{
#ifdef J2B_LOG
				MPT_LOG_GLOBAL(LogDebug, "J2B", MPT_UFORMAT("J2B: Unknown command: 0x{}, param 0x{}")(mpt::ufmt::HEX0<2>(command), mpt::ufmt::HEX0<2>(m.param)));
#endif
				m.command = CMD_NONE;
			}

			// Handling special commands
			switch(m.command)
			{
			case CMD_ARPEGGIO:
				if(m.param == 0) m.command = CMD_NONE;
				break;
			case CMD_VOLUME:
				if(m.volcmd == VOLCMD_NONE && !(flags & volFlag))
				{
					m.SetVolumeCommand(VOLCMD_VOLUME, Clamp(m.param, uint8(0), uint8(64)));
					m.command = CMD_NONE;
				}
				break;
			case CMD_TONEPORTAVOL:
			case CMD_VIBRATOVOL:
			case CMD_VOLUMESLIDE:
			case CMD_GLOBALVOLSLIDE:
			case CMD_PANNINGSLIDE:
				if(m.param & 0xF0)
					m.param &= 0xF0;
				break;
			case CMD_PANNING8:
				if(m.param <= 0x80)
					m.param = mpt::saturate_cast<ModCommand::PARAM>(m.param * 2);
				else if(m.param == 0xA4)
					m.SetEffectCommand(CMD_S3MCMDEX, 0x91u);
				break;
			case CMD_PATTERNBREAK:
				m.param = static_cast<ModCommand::PARAM>(((m.param >> 4) * 10u) + (m.param & 0x0Fu));
				break;
			case CMD_MODCMDEX:
				m.ExtendedMODtoS3MEffect();
				break;
			case CMD_TEMPO:
				if(m.param <= 0x1F)
					m.command = CMD_SPEED;
				break;
			case CMD_XFINEPORTAUPDOWN:
				switch(m.param & 0xF0)
				{
				case 0x10:
					m.command = CMD_PORTAMENTOUP;
					break;
				case 0x20:
					m.command = CMD_PORTAMENTODOWN;
					break;
				}
				m.param = (m.param & 0x0F) | 0xE0;
				break;
			default:
				break;
			}
		}

		if(flags & noteFlag)  // note + ins
		{
			const auto [instr, note] = chunk.ReadArray<uint8, 2>();
			m.instr = instr;
			m.note = note;
			if(m.note == 0x80)
				m.note = NOTE_KEYOFF;
			else if(m.note > 0x80)
				m.note = NOTE_FADE;  // I guess the support for IT "note fade" notes was not intended in mod2j2b, but hey, it works! :-D
		}

		if(flags & volFlag) // volume
		{
			m.SetVolumeCommand(VOLCMD_VOLUME, chunk.ReadUint8());
			if(isAM)
				m.vol = static_cast<ModCommand::VOL>(m.vol * 64u / 127u);
		}
	}

	return true;
}


struct AMFFRiffChunkFormat
{
	uint32le format;
};

MPT_BINARY_STRUCT(AMFFRiffChunkFormat, 4)


static bool ValidateHeader(const AMFFRiffChunk &fileHeader)
{
	if(fileHeader.id != AMFFRiffChunk::idRIFF)
	{
		return false;
	}
	if(fileHeader.GetLength() < 8 + sizeof(AMFFMainChunk))
	{
		return false;
	}
	return true;
}


static bool ValidateHeader(const AMFFRiffChunkFormat &formatHeader)
{
	if(formatHeader.format != AMFFRiffChunk::idAMFF && formatHeader.format != AMFFRiffChunk::idAM__)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderAM(MemoryFileReader file, const uint64 *pfilesize)
{
	AMFFRiffChunk fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	AMFFRiffChunkFormat formatHeader;
	if(!file.ReadStruct(formatHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(formatHeader))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadAM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	AMFFRiffChunk fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	AMFFRiffChunkFormat formatHeader;
	if(!file.ReadStruct(formatHeader))
	{
		return false;
	}
	if(!ValidateHeader(formatHeader))
	{
		return false;
	}

	bool isAM; // false: AMFF, true: AM

	uint32 format = formatHeader.format;
	if(format == AMFFRiffChunk::idAMFF)
		isAM = false; // "AMFF"
	else if(format == AMFFRiffChunk::idAM__)
		isAM = true; // "AM  "
	else
		return false;

	ChunkReader chunkFile(file);

	// The main chunk is almost identical in both formats but uses different chunk IDs.
	// "MAIN" - Song info (AMFF)
	// "INIT" - Song info (AM)
	AMFFRiffChunk::ChunkIdentifiers mainChunkID = isAM ? AMFFRiffChunk::idINIT : AMFFRiffChunk::idMAIN;

	// RIFF AM has a padding byte so that all chunks have an even size.
	ChunkReader::ChunkList<AMFFRiffChunk> chunks;
	if(loadFlags == onlyVerifyHeader)
		chunks = chunkFile.ReadChunksUntil<AMFFRiffChunk>(isAM ? 2 : 1, mainChunkID);
	else
		chunks = chunkFile.ReadChunks<AMFFRiffChunk>(isAM ? 2 : 1);

	FileReader chunkMain(chunks.GetChunk(mainChunkID));
	AMFFMainChunk mainChunk;
	if(!chunkMain.IsValid()
		|| !chunkMain.ReadStruct(mainChunk)
		|| mainChunk.channels < 1
		|| !chunkMain.CanRead(mainChunk.channels))
	{
		return false;
	} else if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_J2B, std::min(static_cast<CHANNELINDEX>(mainChunk.channels), static_cast<CHANNELINDEX>(MAX_BASECHANNELS)));
	m_SongFlags = SONG_ITOLDEFFECTS | SONG_ITCOMPATGXX;
	m_SongFlags.set(SONG_LINEARSLIDES, !(mainChunk.flags & AMFFMainChunk::amigaSlides));

	Order().SetDefaultSpeed(mainChunk.speed);
	Order().SetDefaultTempoInt(mainChunk.tempo);
	m_nDefaultGlobalVolume = mainChunk.globalvolume * 2;

	m_modFormat.formatName = isAM ? UL_("Galaxy Sound System (new version)") : UL_("Galaxy Sound System (old version)");
	m_modFormat.type = UL_("j2b");
	m_modFormat.charset = mpt::Charset::CP437;

	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, mainChunk.songname);

	// It seems like there's no way to differentiate between
	// Muted and Surround channels (they're all 0xA0) - might
	// be a limitation in mod2j2b.
	for(auto &chn : ChnSettings)
	{
		uint8 pan = chunkMain.ReadUint8();
		if(isAM)
		{
			if(pan > 128)
				chn.dwFlags = CHN_MUTE;
			else
				chn.nPan = pan * 2;
		} else
		{
			if(pan >= 128)
				chn.dwFlags = CHN_MUTE;
			else
				chn.nPan = static_cast<uint16>(std::min(pan * 4, 256));
		}
	}

	if(chunks.ChunkExists(AMFFRiffChunk::idORDR))
	{
		// "ORDR" - Order list
		FileReader chunk(chunks.GetChunk(AMFFRiffChunk::idORDR));
		uint8 numOrders = chunk.ReadUint8() + 1;
		ReadOrderFromFile<uint8>(Order(), chunk, numOrders, 0xFF, 0xFE);
	}

	// "PATT" - Pattern data for one pattern
	if(loadFlags & loadPatternData)
	{
		PATTERNINDEX maxPattern = 0;
		auto pattChunks = chunks.GetAllChunks(AMFFRiffChunk::idPATT);
		Patterns.ResizeArray(static_cast<PATTERNINDEX>(pattChunks.size()));
		for(auto chunk : pattChunks)
		{
			PATTERNINDEX pat = chunk.ReadUint8();
			size_t patternSize = chunk.ReadUint32LE();
			ConvertAMPattern(chunk.ReadChunk(patternSize), pat, isAM, *this);
			maxPattern = std::max(maxPattern, pat);
		}
		for(PATTERNINDEX pat = 0; pat < maxPattern; pat++)
		{
			if(!Patterns.IsValidPat(pat))
				Patterns.Insert(pat, 64);
		}
	}

	if(!isAM)
	{
		// "INST" - Instrument (only in RIFF AMFF)
		auto instChunks = chunks.GetAllChunks(AMFFRiffChunk::idINST);
		for(auto chunk : instChunks)
		{
			AMFFInstrumentHeader instrHeader;
			if(!chunk.ReadStruct(instrHeader))
			{
				continue;
			}

			const INSTRUMENTINDEX instr = instrHeader.index + 1;
			if(instr >= MAX_INSTRUMENTS)
				continue;

			ModInstrument *pIns = AllocateInstrument(instr);
			if(pIns == nullptr)
			{
				continue;
			}

			instrHeader.ConvertToMPT(*pIns, m_nSamples);

			// read sample sub-chunks - this is a rather "flat" format compared to RIFF AM and has no nested RIFF chunks.
			for(size_t samples = 0; samples < instrHeader.numSamples; samples++)
			{
				AMFFSampleHeader sampleHeader;

				if(!CanAddMoreSamples() || !chunk.ReadStruct(sampleHeader))
				{
					continue;
				}

				const SAMPLEINDEX smp = ++m_nSamples;

				if(sampleHeader.id != AMFFRiffChunk::idSAMP)
				{
					continue;
				}

				m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);
				sampleHeader.ConvertToMPT(instrHeader, Samples[smp]);
				if(loadFlags & loadSampleData)
					sampleHeader.GetSampleFormat().ReadSample(Samples[smp], chunk);
				else
					chunk.Skip(Samples[smp].GetSampleSizeInBytes());
			}
		}
	} else
	{
		// "RIFF" - Instrument (only in RIFF AM)
		auto instChunks = chunks.GetAllChunks(AMFFRiffChunk::idRIFF);
		for(ChunkReader chunk : instChunks)
		{
			if(chunk.ReadUint32LE() != AMFFRiffChunk::idAI__)
			{
				continue;
			}

			AMFFRiffChunk instChunk;
			if(!chunk.ReadStruct(instChunk) || instChunk.id != AMFFRiffChunk::idINST)
			{
				continue;
			}

			AMInstrumentHeader instrHeader;
			if(!chunk.ReadStruct(instrHeader))
			{
				continue;
			}
			MPT_ASSERT(instrHeader.headSize + 4 == sizeof(instrHeader));

			const INSTRUMENTINDEX instr = instrHeader.index + 1;
			if(instr >= MAX_INSTRUMENTS)
				continue;

			ModInstrument *pIns = AllocateInstrument(instr);
			if(pIns == nullptr)
			{
				continue;
			}

			instrHeader.ConvertToMPT(*pIns, m_nSamples);

			// Read sample sub-chunks (RIFF nesting ftw)
			auto sampleChunks = chunk.ReadChunks<AMFFRiffChunk>(2).GetAllChunks(AMFFRiffChunk::idRIFF);
			MPT_ASSERT(sampleChunks.size() == instrHeader.numSamples);

			for(auto sampleChunk : sampleChunks)
			{
				if(sampleChunk.ReadUint32LE() != AMFFRiffChunk::idAS__ || !CanAddMoreSamples())
				{
					continue;
				}

				// Don't read more samples than the instrument header claims to have.
				if((instrHeader.numSamples--) == 0)
				{
					break;
				}

				const SAMPLEINDEX smp = ++m_nSamples;

				// Aaand even more nested chunks! Great, innit?
				AMFFRiffChunk sampleHeaderChunk;
				if(!sampleChunk.ReadStruct(sampleHeaderChunk) || sampleHeaderChunk.id != AMFFRiffChunk::idSAMP)
				{
					break;
				}

				FileReader sampleFileChunk = sampleChunk.ReadChunk(sampleHeaderChunk.length);

				AMSampleHeader sampleHeader;
				if(!sampleFileChunk.ReadStruct(sampleHeader))
				{
					break;
				}

				m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);

				sampleHeader.ConvertToMPT(instrHeader, Samples[smp]);

				if(loadFlags & loadSampleData)
				{
					sampleFileChunk.Seek(sampleHeader.headSize + 4);
					sampleHeader.GetSampleFormat().ReadSample(Samples[smp], sampleFileChunk);
				}
			}
		
		}
	}

	return true;
}


static bool ValidateHeader(const J2BFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.signature, "MUSE", 4)
		|| (fileHeader.deadbeaf != J2BFileHeader::magicDEADBEAF // 0xDEADBEAF (RIFF AM)
			&& fileHeader.deadbeaf != J2BFileHeader::magicDEADBABE) // 0xDEADBABE (RIFF AMFF)
		)
	{
		return false;
	}
	if(fileHeader.packedLength == 0)
	{
		return false;
	}
	if(fileHeader.fileLength != fileHeader.packedLength + sizeof(J2BFileHeader))
	{
		return false;
	}
	return true;
}


static bool ValidateHeaderFileSize(const J2BFileHeader &fileHeader, uint64 filesize)
{
	if(filesize != fileHeader.fileLength)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderJ2B(MemoryFileReader file, const uint64 *pfilesize)
{
	J2BFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	if(pfilesize)
	{
		if(!ValidateHeaderFileSize(fileHeader, *pfilesize))
		{
			return ProbeFailure;
		}
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadJ2B(FileReader &file, ModLoadingFlags loadFlags)
{

#if !defined(MPT_WITH_ZLIB) && !defined(MPT_WITH_MINIZ)

	MPT_UNREFERENCED_PARAMETER(file);
	MPT_UNREFERENCED_PARAMETER(loadFlags);
	return false;

#else

	file.Rewind();
	J2BFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(fileHeader.fileLength != file.GetLength()
		|| fileHeader.packedLength != file.BytesLeft()
		)
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	// Header is valid, now unpack the RIFF AM file using inflate
	z_stream strm{};
	int zlib_errc = Z_OK;
	zlib_errc = inflateInit(&strm);
	if(zlib_errc == Z_MEM_ERROR)
	{
		mpt::throw_out_of_memory();
	} else if(zlib_errc < Z_OK)
	{
		return false;
	}

	uint32 remainRead = fileHeader.packedLength, remainWrite = fileHeader.unpackedLength, totalWritten = 0;
	uint32 crc = 0;
	std::vector<Bytef> amFileData;
	int retVal = Z_OK;
	try
	{
		amFileData.resize(remainWrite);
		while(remainRead && remainWrite && retVal != Z_STREAM_END)
		{
			Bytef buffer[mpt::IO::BUFFERSIZE_TINY];
			uint32 readSize = std::min(static_cast<uint32>(sizeof(buffer)), remainRead);
			file.ReadRaw(mpt::span(buffer, readSize));
			crc = static_cast<uint32>(crc32(crc, buffer, readSize));

			strm.avail_in = readSize;
			strm.next_in = buffer;
			do
			{
				strm.avail_out = remainWrite;
				strm.next_out = amFileData.data() + totalWritten;
				zlib_errc = inflate(&strm, Z_NO_FLUSH);
				if(zlib_errc == Z_BUF_ERROR)
				{
					// expected
				} else if(zlib_errc == Z_MEM_ERROR)
				{
					mpt::throw_out_of_memory();
				} else if(zlib_errc < Z_OK)
				{
					inflateEnd(&strm);
					return false;
				}
				retVal = zlib_errc;
				uint32 written = remainWrite - strm.avail_out;
				totalWritten += written;
				remainWrite -= written;
			} while(remainWrite && strm.avail_out == 0);

			remainRead -= readSize;
		}
	} catch(mpt::out_of_memory e)
	{
		inflateEnd(&strm);
		mpt::rethrow_out_of_memory(e);	
	} catch(const std::exception &)
	{
		inflateEnd(&strm);
		return false;
	} catch(...)
	{
		inflateEnd(&strm);
		return false;
	}
	inflateEnd(&strm);

	bool result = false;
#ifndef MPT_BUILD_FUZZER
	if(fileHeader.crc32 == crc && !remainWrite && retVal == Z_STREAM_END)
#endif
	{
		// Success, now load the RIFF AM(FF) module.
		FileReader amFile(mpt::as_span(amFileData));
		result = ReadAM(amFile, loadFlags);
	}
	return result;

#endif

}


OPENMPT_NAMESPACE_END
