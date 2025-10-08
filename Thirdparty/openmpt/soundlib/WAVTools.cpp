/*
 * WAVTools.cpp
 * ------------
 * Purpose: Definition of WAV file structures and helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "WAVTools.h"
#include "Loaders.h"
#include "Tagging.h"
#include "../common/version.h"
#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io/io.hpp"
#include "mpt/io/io_virtual_wrapper.hpp"
#include "../common/mptFileIO.h"
#endif


OPENMPT_NAMESPACE_BEGIN


///////////////////////////////////////////////////////////
// WAV Reading


WAVReader::WAVReader(FileReader &inputFile) : file(inputFile)
{
	file.Rewind();

	RIFFHeader fileHeader;
	codePage = 28591; // ISO 8859-1
	isDLS = false;
	subFormat = 0;
	mayBeCoolEdit16_8 = false;
	if(!file.ReadStruct(fileHeader)
		|| (fileHeader.magic != RIFFHeader::idRIFF && fileHeader.magic != RIFFHeader::idLIST)
		|| (fileHeader.type != RIFFHeader::idWAVE && fileHeader.type != RIFFHeader::idwave))
	{
		return;
	}

	isDLS = (fileHeader.magic == RIFFHeader::idLIST);

	auto chunks = file.ReadChunks<RIFFChunk>(2);

	if(chunks.chunks.size() >= 4
		&& chunks.chunks[1].GetHeader().GetID() == RIFFChunk::iddata
		&& chunks.chunks[1].GetHeader().GetLength() % 2u != 0
		&& chunks.chunks[2].GetHeader().GetLength() == 0
		&& chunks.chunks[3].GetHeader().GetID() == RIFFChunk::id____)
	{
		// Houston, we have a problem: Old versions of (Open)MPT didn't write RIFF padding bytes. -_-
		// Luckily, the only RIFF chunk with an odd size those versions would ever write would be the "data" chunk
		// (which contains the sample data), and its size is only odd iff the sample has an odd length and is in
		// 8-Bit mono format. In all other cases, the sample size (and thus the chunk size) is even.

		// And we're even more lucky: The versions of (Open)MPT in question will always write a relatively small
		// (smaller than 256 bytes) "smpl" chunk after the "data" chunk. This means that after an unpadded sample,
		// we will always read "mpl?" (? being the length of the "smpl" chunk) as the next chunk magic. The first two
		// 32-Bit members of the "smpl" chunk are always zero in our case, so we are going to read a chunk length of 0
		// next and the next chunk magic, which will always consist of four zero bytes. Hooray! We just checked for those
		// four zero bytes and can be pretty confident that we should not have applied padding.
		file.Seek(sizeof(RIFFHeader));
		chunks = file.ReadChunks<RIFFChunk>(1);
	}

	// Read format chunk
	FileReader formatChunk = chunks.GetChunk(RIFFChunk::idfmt_);
	if(!formatChunk.ReadStruct(formatInfo))
	{
		return;
	}
	if(formatInfo.format == WAVFormatChunk::fmtPCM && formatChunk.BytesLeft() == 4)
	{
		uint16 size = formatChunk.ReadIntLE<uint16>();
		uint16 value = formatChunk.ReadIntLE<uint16>();
		if(size == 2 && value == 1)
		{
			// May be Cool Edit 16.8 format.
			// See SampleFormats.cpp for details.
			mayBeCoolEdit16_8 = true;
		}
	} else if(formatInfo.format == WAVFormatChunk::fmtExtensible)
	{
		WAVFormatChunkExtension extFormat;
		if(!formatChunk.ReadStruct(extFormat))
		{
			return;
		}
		subFormat = static_cast<uint16>(mpt::UUID(extFormat.subFormat).GetData1());
	}

	// Read sample data
	sampleData = chunks.GetChunk(RIFFChunk::iddata);

	if(!sampleData.IsValid())
	{
		// The old IMA ADPCM loader code looked for the "pcm " chunk instead of the "data" chunk...
		// Dunno why (Windows XP's audio recorder saves IMA ADPCM files with a "data" chunk), but we will just look for both.
		sampleData = chunks.GetChunk(RIFFChunk::idpcm_);
	}

	// "fact" chunk should contain sample length of compressed samples.
	sampleLength = chunks.GetChunk(RIFFChunk::idfact).ReadUint32LE();

	if((formatInfo.format != WAVFormatChunk::fmtIMA_ADPCM || sampleLength == 0) && GetSampleSize() != 0)
	{
		if((GetBlockAlign() == 0) || (GetBlockAlign() / GetNumChannels() >= 2 * GetSampleSize()))
		{
			// Some samples have an incorrect blockAlign / sample size set (e.g. it's 8 in SQUARE.WAV while it should be 1), so let's better not always trust this value.
			// The idea here is, if block align is off by twice or more, it is unlikely to be describing sample padding inside the block.
			// Ignore it in this case and calculate the length based on the single sample size and number of channels instead.
			sampleLength = sampleData.GetLength() / GetSampleSize();
		} else
		{
			// Correct case (so that 20bit WAVEFORMATEX files work).
			sampleLength = sampleData.GetLength() / GetBlockAlign();
		}
	}

	// Determine string encoding
	codePage = GetFileCodePage(chunks);

	// Check for loop points, texts, etc...
	FindMetadataChunks(chunks);

	// DLS bank chunk
	wsmpChunk = chunks.GetChunk(RIFFChunk::idwsmp);
}


void WAVReader::FindMetadataChunks(FileReader::ChunkList<RIFFChunk> &chunks)
{
	// Read sample loop points and other sampler information
	smplChunk = chunks.GetChunk(RIFFChunk::idsmpl);
	instChunk = chunks.GetChunk(RIFFChunk::idinst);

	// Read sample cues
	cueChunk = chunks.GetChunk(RIFFChunk::idcue_);

	// Read text chunks
	FileReader listChunk = chunks.GetChunk(RIFFChunk::idLIST);
	if(listChunk.ReadMagic("INFO"))
	{
		infoChunk = listChunk.ReadChunks<RIFFChunk>(2);
	}

	// Read MPT sample information
	xtraChunk = chunks.GetChunk(RIFFChunk::idxtra);
}


uint16 WAVReader::GetFileCodePage(FileReader::ChunkList<RIFFChunk> &chunks)
{
	FileReader csetChunk = chunks.GetChunk(RIFFChunk::idCSET);
	if(!csetChunk.IsValid())
	{
		FileReader iSFT = infoChunk.GetChunk(RIFFChunk::idISFT);
		if(iSFT.ReadMagic("OpenMPT"))
		{
			std::string versionString;
			iSFT.ReadString<mpt::String::maybeNullTerminated>(versionString, mpt::saturate_cast<std::size_t>(iSFT.BytesLeft()));
			versionString = mpt::trim(versionString);
			Version version = Version::Parse(mpt::ToUnicode(mpt::Charset::ISO8859_1, versionString));
			if(version && version < MPT_V("1.28.00.02"))
			{
				return 1252; // mpt::Charset::Windows1252; // OpenMPT up to and including 1.28.00.01 wrote metadata in windows-1252 encoding
			} else
			{
				return 28591; // mpt::Charset::ISO8859_1; // as per spec
			}
		} else
		{
			return 28591; // mpt::Charset::ISO8859_1; // as per spec
		}
	}
	if(!csetChunk.CanRead(2))
	{
		// chunk not parsable
		return 28591; // mpt::Charset::ISO8859_1;
	}
	uint16 codepage = csetChunk.ReadUint16LE();
	return codepage;
}


void WAVReader::ApplySampleSettings(ModSample &sample, mpt::Charset sampleCharset, mpt::charbuf<MAX_SAMPLENAME> &sampleName)
{
	// Read sample name
	FileReader textChunk = infoChunk.GetChunk(RIFFChunk::idINAM);
	if(textChunk.IsValid())
	{
		std::string sampleNameEncoded;
		textChunk.ReadString<mpt::String::nullTerminated>(sampleNameEncoded, mpt::saturate_cast<std::size_t>(textChunk.GetLength()));
		sampleName = mpt::ToCharset(sampleCharset, mpt::ToUnicode(codePage, mpt::Charset::Windows1252, sampleNameEncoded));
	}
	if(isDLS)
	{
		// DLS sample -> sample filename
		sample.filename = sampleName;
	}

	// Read software name
	const bool isOldMPT = infoChunk.GetChunk(RIFFChunk::idISFT).ReadMagic("Modplug Tracker");
	
	// Convert loops
	WAVSampleInfoChunk sampleInfo;
	smplChunk.Rewind();
	if(smplChunk.ReadStruct(sampleInfo))
	{
		WAVSampleLoop loopData;
		if(sampleInfo.numLoops > 1 && smplChunk.ReadStruct(loopData))
		{
			// First loop: Sustain loop
			loopData.ApplyToSample(sample.nSustainStart, sample.nSustainEnd, sample.nLength, sample.uFlags, CHN_SUSTAINLOOP, CHN_PINGPONGSUSTAIN, isOldMPT);
		}
		// First loop (if only one loop is present) or second loop (if more than one loop is present): Normal sample loop
		if(smplChunk.ReadStruct(loopData))
		{
			loopData.ApplyToSample(sample.nLoopStart, sample.nLoopEnd, sample.nLength, sample.uFlags, CHN_LOOP, CHN_PINGPONGLOOP, isOldMPT);
		}
		//sample.Transpose((60 - sampleInfo.baseNote) / 12.0);
		sample.rootNote = static_cast<uint8>(sampleInfo.baseNote);
		if(sample.rootNote < 128)
			sample.rootNote += NOTE_MIN;
		else
			sample.rootNote = NOTE_NONE;
		sample.SanitizeLoops();
	}

	if(sample.rootNote == NOTE_NONE && instChunk.LengthIsAtLeast(sizeof(WAVInstrumentChunk)))
	{
		WAVInstrumentChunk inst;
		instChunk.Rewind();
		if(instChunk.ReadStruct(inst))
		{
			sample.rootNote = inst.unshiftedNote;
			if(sample.rootNote < 128)
				sample.rootNote += NOTE_MIN;
			else
				sample.rootNote = NOTE_NONE;
		}
	}

	// Read cue points
	if(cueChunk.IsValid())
	{
		uint32 numPoints = cueChunk.ReadUint32LE();
		LimitMax(numPoints, mpt::saturate_cast<uint32>(std::size(sample.cues)));
		for(uint32 i = 0; i < numPoints; i++)
		{
			WAVCuePoint cuePoint;
			cueChunk.ReadStruct(cuePoint);
			sample.cues[i] = cuePoint.position;
		}
		std::fill(std::begin(sample.cues) + numPoints, std::end(sample.cues), MAX_SAMPLE_LENGTH);
	}

	// Read MPT extra info
	WAVExtraChunk mptInfo;
	xtraChunk.Rewind();
	if(xtraChunk.ReadStruct(mptInfo))
	{
		if(mptInfo.flags & WAVExtraChunk::setPanning) sample.uFlags.set(CHN_PANNING);

		sample.nPan = std::min(static_cast<uint16>(mptInfo.defaultPan), uint16(256));
		sample.nVolume = std::min(static_cast<uint16>(mptInfo.defaultVolume), uint16(256));
		sample.nGlobalVol = std::min(static_cast<uint16>(mptInfo.globalVolume), uint16(64));
		sample.nVibType = static_cast<VibratoType>(mptInfo.vibratoType.get());
		sample.nVibSweep = mptInfo.vibratoSweep;
		sample.nVibDepth = mptInfo.vibratoDepth;
		sample.nVibRate = mptInfo.vibratoRate;

		if(xtraChunk.CanRead(MAX_SAMPLENAME))
		{
			// Name present (clipboard only)
			// FIXME: When modules can have individual encoding in OpenMPT or when
			// internal metadata gets converted to Unicode, we must adjust this to
			// also specify encoding.
			xtraChunk.ReadString<mpt::String::nullTerminated>(sampleName, MAX_SAMPLENAME);
			xtraChunk.ReadString<mpt::String::nullTerminated>(sample.filename, mpt::saturate_cast<std::size_t>(xtraChunk.BytesLeft()));
		}
	}
}


// Apply WAV loop information to a mod sample.
void WAVSampleLoop::ApplyToSample(SmpLength &start, SmpLength &end, SmpLength sampleLength, SampleFlags &flags, ChannelFlags enableFlag, ChannelFlags bidiFlag, bool mptLoopFix) const
{
	if(loopEnd == 0)
	{
		// Some WAV files seem to have loops going from 0 to 0... We should ignore those.
		return;
	}
	start = std::min(static_cast<SmpLength>(loopStart), sampleLength);
	end = Clamp(static_cast<SmpLength>(loopEnd), start, sampleLength);
	if(!mptLoopFix && end < sampleLength)
	{
		// RIFF loop end points are inclusive - old versions of MPT didn't consider this.
		end++;
	}

	flags.set(enableFlag);
	if(loopType == loopBidi)
	{
		flags.set(bidiFlag);
	}
}


// Convert internal loop information into a WAV loop.
void WAVSampleLoop::ConvertToWAV(SmpLength start, SmpLength end, bool bidi)
{
	identifier = 0;
	loopType = bidi ? loopBidi : loopForward;
	loopStart = mpt::saturate_cast<uint32>(start);
	// Loop ends are *inclusive* in the RIFF standard, while they're *exclusive* in OpenMPT.
	if(end > start)
	{
		loopEnd = mpt::saturate_cast<uint32>(end - 1);
	} else
	{
		loopEnd = loopStart;
	}
	fraction = 0;
	playCount = 0;
}


#ifndef MODPLUG_NO_FILESAVE


WAVSampleWriter::WAVSampleWriter(mpt::IO::OFileBase &stream)
	: WAVWriter(stream)
{
	return;
}


WAVSampleWriter::~WAVSampleWriter()
{
	return;
}


// Write a sample loop information chunk to the file.
void WAVSampleWriter::WriteLoopInformation(const ModSample &sample, SmpLength rangeStart, SmpLength rangeEnd)
{
	if(!sample.uFlags[CHN_LOOP | CHN_SUSTAINLOOP] && !ModCommand::IsNote(sample.rootNote))
	{
		return;
	}
	if(rangeEnd <= rangeStart)
	{
		rangeStart = 0;
		rangeEnd = sample.nLength;
	}

	StartChunk(RIFFChunk::idsmpl);
	WAVSampleInfoChunk info;

	uint32 sampleRate = sample.nC5Speed;
	if(sampleRate == 0)
	{
		sampleRate = ModSample::TransposeToFrequency(sample.RelativeTone, sample.nFineTune);
	}

	info.ConvertToWAV(sampleRate, sample.rootNote);

	// Set up loops
	std::array<WAVSampleLoop, 2> loops{{}};
	const bool writeSustainLoop = sample.uFlags[CHN_SUSTAINLOOP] && sample.nSustainStart < rangeEnd && sample.nSustainEnd >= rangeStart;
	const bool writeNormalLoop = sample.uFlags[CHN_LOOP] && sample.nLoopStart < rangeEnd && sample.nLoopEnd >= rangeStart;
	if(writeSustainLoop)
	{
		loops[info.numLoops++].ConvertToWAV(std::max(sample.nSustainStart, rangeStart) - rangeStart, std::clamp(sample.nSustainEnd, rangeStart, rangeEnd) - rangeStart, sample.uFlags[CHN_PINGPONGSUSTAIN]);
	}
	if(writeNormalLoop)
	{
		loops[info.numLoops++].ConvertToWAV(std::max(sample.nLoopStart, rangeStart) - rangeStart, std::clamp(sample.nLoopEnd, rangeStart, rangeEnd) - rangeStart, sample.uFlags[CHN_PINGPONGLOOP]);
	} else if(writeSustainLoop)
	{
		// Since there are no "loop types" to distinguish between sustain and normal loops, OpenMPT assumes
		// that the first loop is a sustain loop if there are two loops. If we only want a sustain loop,
		// we will have to write a second bogus loop.
		loops[info.numLoops++].ConvertToWAV(0, 0, false);
	}

	mpt::IO::Write(s, info);
	for(uint32 i = 0; i < info.numLoops; i++)
	{
		mpt::IO::Write(s, loops[i]);
	}
}


// Write a sample's cue points to the file.
void WAVSampleWriter::WriteCueInformation(const ModSample &sample, SmpLength rangeStart, SmpLength rangeEnd)
{
	if(rangeEnd <= rangeStart)
	{
		rangeStart = 0;
		rangeEnd = sample.nLength;
	}

	uint32 numMarkers = 0;
	for(const auto cue : sample.cues)
	{
		if(mpt::is_in_range(cue, rangeStart, rangeEnd))
			numMarkers++;
	}

	StartChunk(RIFFChunk::idcue_);
	mpt::IO::Write(s, mpt::as_le(numMarkers));
	uint32 i = 0;
	for(const auto cue : sample.cues)
	{
		if(mpt::is_in_range(cue, rangeStart, rangeEnd))
		{
			WAVCuePoint cuePoint = ConvertToWAVCuePoint(i++, cue - rangeStart);
			mpt::IO::Write(s, cuePoint);
		}
	}
}


// Write MPT's sample information chunk to the file.
void WAVSampleWriter::WriteExtraInformation(const ModSample &sample, MODTYPE modType, const char *sampleName)
{
	StartChunk(RIFFChunk::idxtra);
	WAVExtraChunk mptInfo;

	mptInfo.ConvertToWAV(sample, modType);
	mpt::IO::Write(s, mptInfo);

	if(sampleName != nullptr)
	{
		// Write sample name (clipboard only)

		// FIXME: When modules can have individual encoding in OpenMPT or when
		// internal metadata gets converted to Unicode, we must adjust this to
		// also specify encoding.

		char name[MAX_SAMPLENAME];
		mpt::String::WriteBuf(mpt::String::nullTerminated, name) = sampleName;
		mpt::IO::Write(s, name);

		char filename[MAX_SAMPLEFILENAME];
		mpt::String::WriteBuf(mpt::String::nullTerminated, filename) = sample.filename;
		mpt::IO::Write(s, filename);
	}
}


#endif // MODPLUG_NO_FILESAVE


OPENMPT_NAMESPACE_END
