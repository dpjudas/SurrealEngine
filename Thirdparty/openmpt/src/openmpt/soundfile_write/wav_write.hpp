/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/io/base.hpp"
#include "mpt/io/io_virtual_wrapper.hpp"
#include "mpt/string/types.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/soundfile_data/wav.hpp"



OPENMPT_NAMESPACE_BEGIN



class WAVWriter
{
protected:
	// Output stream
	mpt::IO::OFileBase &s;

	// Currently written chunk
	mpt::IO::Offset chunkHeaderPos = 0;
	RIFFChunk chunkHeader;
	bool finalized = false;

public:
	// Output to stream: Initialize with std::ostream*.
	WAVWriter(mpt::IO::OFileBase &stream);

	// Begin writing a new chunk to the file.
	void StartChunk(RIFFChunk::ChunkIdentifiers id);

protected:
	// End current chunk by updating the chunk header and writing a padding byte if necessary.
	void FinalizeChunk();

public:
	// Write the WAV format to the file.
	void WriteFormat(uint32 sampleRate, uint16 bitDepth, uint16 numChannels, WAVFormatChunk::SampleFormats encoding);

	// Write text tags to the file.
	void WriteMetatags(const FileTags &tags);

protected:
	// Write a single tag into a open idLIST chunk
	void WriteTag(RIFFChunk::ChunkIdentifiers id, const mpt::ustring &utext);

public:
	// Finalize the file by closing the last open chunk and updating the file header. Returns total size of file.
	mpt::IO::Offset Finalize();

	~WAVWriter();
};



OPENMPT_NAMESPACE_END
