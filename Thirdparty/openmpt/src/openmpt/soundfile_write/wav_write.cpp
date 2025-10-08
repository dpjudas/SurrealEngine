/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "wav_write.hpp"

#include "mpt/base/memory.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/span.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_virtual_wrapper.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/soundfile_data/wav.hpp"

#include <limits>
#include <string>

#include <cassert>



OPENMPT_NAMESPACE_BEGIN



WAVWriter::WAVWriter(mpt::IO::OFileBase &stream)
	: s(stream)
{
	// Skip file header for now
	mpt::IO::SeekRelative(s, sizeof(RIFFHeader));
}


void WAVWriter::StartChunk(RIFFChunk::ChunkIdentifiers id)
{
	FinalizeChunk();

	chunkHeaderPos = mpt::IO::TellWrite(s);
	chunkHeader.id = id;
	mpt::IO::SeekRelative(s, sizeof(chunkHeader));
}


void WAVWriter::FinalizeChunk()
{
	if(chunkHeaderPos != 0)
	{
		const mpt::IO::Offset position = mpt::IO::TellWrite(s);

		const mpt::IO::Offset chunkSize = position - (chunkHeaderPos + sizeof(RIFFChunk));
		chunkHeader.length = mpt::saturate_cast<uint32>(chunkSize);

		mpt::IO::SeekAbsolute(s, chunkHeaderPos);
		mpt::IO::Write(s, chunkHeader);

		mpt::IO::SeekAbsolute(s, position);

		if((chunkSize % 2u) != 0)
		{
			// Write padding
			uint8 padding = 0;
			mpt::IO::Write(s, padding);
		}

		chunkHeaderPos = 0;
	}
}


void WAVWriter::WriteFormat(uint32 sampleRate, uint16 bitDepth, uint16 numChannels, WAVFormatChunk::SampleFormats encoding)
{
	StartChunk(RIFFChunk::idfmt_);
	WAVFormatChunk wavFormat;
	mpt::reset(wavFormat);

	bool extensible = (numChannels > 2);

	wavFormat.format = static_cast<uint16>(extensible ? WAVFormatChunk::fmtExtensible : encoding);
	wavFormat.numChannels = numChannels;
	wavFormat.sampleRate = sampleRate;
	wavFormat.blockAlign = static_cast<uint16>((bitDepth * numChannels + 7u) / 8u);
	wavFormat.byteRate = wavFormat.sampleRate * wavFormat.blockAlign;
	wavFormat.bitsPerSample = bitDepth;

	mpt::IO::Write(s, wavFormat);

	if(extensible)
	{
		WAVFormatChunkExtension extFormat;
		mpt::reset(extFormat);
		extFormat.size = sizeof(WAVFormatChunkExtension) - sizeof(uint16);
		extFormat.validBitsPerSample = bitDepth;
		switch(numChannels)
		{
			case 1:
				extFormat.channelMask = 0x0004;  // FRONT_CENTER
				break;
			case 2:
				extFormat.channelMask = 0x0003;  // FRONT_LEFT | FRONT_RIGHT
				break;
			case 3:
				extFormat.channelMask = 0x0103;  // FRONT_LEFT | FRONT_RIGHT | BACK_CENTER
				break;
			case 4:
				extFormat.channelMask = 0x0033;  // FRONT_LEFT | FRONT_RIGHT | BACK_LEFT | BACK_RIGHT
				break;
			default:
				extFormat.channelMask = 0;
				break;
		}
		extFormat.subFormat = mpt::UUID(static_cast<uint16>(encoding), 0x0000, 0x0010, 0x800000AA00389B71ull);
		mpt::IO::Write(s, extFormat);
	}
}


void WAVWriter::WriteMetatags(const FileTags &tags)
{
	StartChunk(RIFFChunk::idCSET);
	mpt::IO::Write(s, mpt::as_le(uint16(65001)));  // code page    (UTF-8)
	mpt::IO::Write(s, mpt::as_le(uint16(0)));      // country code (unset)
	mpt::IO::Write(s, mpt::as_le(uint16(0)));      // language     (unset)
	mpt::IO::Write(s, mpt::as_le(uint16(0)));      // dialect      (unset)

	StartChunk(RIFFChunk::idLIST);
	const char info[] = {'I', 'N', 'F', 'O'};
	mpt::IO::Write(s, info);

	WriteTag(RIFFChunk::idINAM, tags.title);
	WriteTag(RIFFChunk::idIART, tags.artist);
	WriteTag(RIFFChunk::idIPRD, tags.album);
	WriteTag(RIFFChunk::idICRD, tags.year);
	WriteTag(RIFFChunk::idICMT, tags.comments);
	WriteTag(RIFFChunk::idIGNR, tags.genre);
	WriteTag(RIFFChunk::idTURL, tags.url);
	WriteTag(RIFFChunk::idISFT, tags.encoder);
	WriteTag(RIFFChunk::idTRCK, tags.trackno);
}


void WAVWriter::WriteTag(RIFFChunk::ChunkIdentifiers id, const mpt::ustring &utext)
{
	std::string text = mpt::transcode<std::string>(mpt::common_encoding::utf8, utext);
	text = text.substr(0, std::numeric_limits<uint32>::max() - 1u);
	if(!text.empty())
	{
		const uint32 length = mpt::saturate_cast<uint32>(text.length() + 1);

		RIFFChunk chunk;
		mpt::reset(chunk);
		chunk.id = static_cast<uint32>(id);
		chunk.length = length;
		mpt::IO::Write(s, chunk);
		mpt::IO::Write(s, mpt::byte_cast<mpt::const_byte_span>(mpt::span(text.c_str(), length)));

		if((length % 2u) != 0)
		{
			uint8 padding = 0;
			mpt::IO::Write(s, padding);
		}
	}
}


mpt::IO::Offset WAVWriter::Finalize()
{
	FinalizeChunk();

	mpt::IO::Offset totalSize = mpt::IO::TellWrite(s);

	RIFFHeader fileHeader;
	mpt::reset(fileHeader);
	fileHeader.magic = RIFFHeader::idRIFF;
	fileHeader.length = mpt::saturate_cast<uint32>(totalSize - 8);
	fileHeader.type = RIFFHeader::idWAVE;

	mpt::IO::SeekBegin(s);
	mpt::IO::Write(s, fileHeader);
	mpt::IO::SeekAbsolute(s, totalSize);
	finalized = true;

	return totalSize;
}


WAVWriter::~WAVWriter()
{
	assert(finalized);
}



OPENMPT_NAMESPACE_END
