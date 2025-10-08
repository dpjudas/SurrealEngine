/*
 * SampleFormatFLAC.cpp
 * --------------------
 * Purpose: FLAC sample import.
 * Notes  :
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/TrackerSettings.h"
#endif //MODPLUG_TRACKER
#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io_file/fstream.hpp"
#include "../common/mptFileIO.h"
#endif
#include "../common/misc_util.h"
#include "Tagging.h"
#include "Loaders.h"
#include "WAVTools.h"
#include "../common/FileReader.h"
#include "modsmp_ctrl.h"
#include "openmpt/soundbase/Copy.hpp"
#include "openmpt/soundbase/SampleConvert.hpp"
#include "openmpt/soundbase/SampleDecode.hpp"
#include "../soundlib/SampleCopy.h"
#include "../soundlib/ModSampleCopy.h"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/parse/parse.hpp"
//#include "mpt/crc/crc.hpp"
#include "OggStream.h"
#include <algorithm>
#ifdef MPT_WITH_FLAC
#if MPT_PLATFORM_MULTITHREADED && !defined(MPT_LIBCXX_QUIRK_NO_STD_THREAD)
#include <thread>
#endif
#endif
#ifdef MPT_WITH_OGG
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif // MPT_COMPILER_CLANG
#include <ogg/ogg.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
#endif // MPT_WITH_OGG
#ifdef MPT_WITH_FLAC
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif // MPT_COMPILER_CLANG
#include <FLAC/stream_decoder.h>
#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
#endif // MPT_WITH_FLAC


OPENMPT_NAMESPACE_BEGIN


///////////////////////////////////////////////////////////////////////////////////////////////////
// FLAC Samples

#ifdef MPT_WITH_FLAC

struct FLACDecoder
{
	FileReader &m_file;
	CSoundFile &m_sndFile;
	const SAMPLEINDEX m_sample;
	bool m_ready = false;

	FLACDecoder(FileReader &f, CSoundFile &sf, SAMPLEINDEX smp) : m_file(f), m_sndFile(sf), m_sample(smp) { }

	static FLAC__StreamDecoderReadStatus read_cb(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->m_file;
		if(*bytes > 0)
		{
			FileReader::pos_type readBytes = *bytes;
			LimitMax(readBytes, file.BytesLeft());
			file.ReadRaw(mpt::byte_cast<mpt::byte_span>(mpt::span(buffer, readBytes)));
			*bytes = readBytes;
			if(*bytes == 0)
				return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
			else
				return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		} else
		{
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		}
	}

	static FLAC__StreamDecoderSeekStatus seek_cb(const FLAC__StreamDecoder *, FLAC__uint64 absolute_byte_offset, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->m_file;
		if(!file.Seek(static_cast<FileReader::pos_type>(absolute_byte_offset)))
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
		else
			return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	}

	static FLAC__StreamDecoderTellStatus tell_cb(const FLAC__StreamDecoder *, FLAC__uint64 *absolute_byte_offset, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->m_file;
		*absolute_byte_offset = file.GetPosition();
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}

	static FLAC__StreamDecoderLengthStatus length_cb(const FLAC__StreamDecoder *, FLAC__uint64 *stream_length, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->m_file;
		*stream_length = file.GetLength();
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}

	static FLAC__bool eof_cb(const FLAC__StreamDecoder *, void *client_data)
	{
		FileReader &file = static_cast<FLACDecoder *>(client_data)->m_file;
		return file.NoBytesLeft();
	}

	static FLAC__StreamDecoderWriteStatus write_cb(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
	{
		FLACDecoder &client = *static_cast<FLACDecoder *>(client_data);
		ModSample &sample = client.m_sndFile.GetSample(client.m_sample);

		if(frame->header.number.sample_number >= sample.nLength || !client.m_ready)
		{
			// We're reading beyond the sample size already, or we aren't even ready to decode yet!
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}

		// Number of samples to be copied in this call
		const SmpLength copySamples = std::min(static_cast<SmpLength>(frame->header.blocksize), static_cast<SmpLength>(sample.nLength - frame->header.number.sample_number));
		// Number of target channels
		const uint8 modChannels = sample.GetNumChannels();
		// Offset (in samples) into target data
		const size_t offset = static_cast<size_t>(frame->header.number.sample_number) * modChannels;
		// Source size in bytes
		const size_t srcSize = frame->header.blocksize * 4;
		// Source bit depth
		const unsigned int bps = frame->header.bits_per_sample;

		MPT_ASSERT((bps <= 8 && sample.GetElementarySampleSize() == 1) || (bps > 8 && sample.GetElementarySampleSize() == 2));
		MPT_ASSERT(modChannels <= FLAC__stream_decoder_get_channels(decoder));
		MPT_ASSERT(bps == FLAC__stream_decoder_get_bits_per_sample(decoder));
		MPT_UNREFERENCED_PARAMETER(decoder); // decoder is unused if ASSERTs are compiled out

		// Do the sample conversion
		for(uint8 chn = 0; chn < modChannels; chn++)
		{
			if(bps <= 8)
			{
				int8 *sampleData8 = sample.sample8() + offset;
				CopySample<SC::ConversionChain<SC::ConvertShift< int8, int32,  0>, SC::DecodeIdentity<int32> > >(sampleData8  + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			} else if(bps <= 16)
			{
				int16 *sampleData16 = sample.sample16() + offset;
				CopySample<SC::ConversionChain<SC::ConvertShift<int16, int32,  0>, SC::DecodeIdentity<int32> > >(sampleData16 + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			} else if(bps <= 24)
			{
				int16 *sampleData16 = sample.sample16() + offset;
				CopySample<SC::ConversionChain<SC::ConvertShift<int16, int32,  8>, SC::DecodeIdentity<int32> > >(sampleData16 + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			} else if(bps <= 32)
			{
				int16 *sampleData16 = sample.sample16() + offset;
				CopySample<SC::ConversionChain<SC::ConvertShift<int16, int32, 16>, SC::DecodeIdentity<int32> > >(sampleData16 + chn, copySamples, modChannels, buffer[chn], srcSize, 1);
			}
		}

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}

	static void metadata_cb(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *client_data)
	{
		FLACDecoder &client = *static_cast<FLACDecoder *>(client_data);
		if(client.m_sample > client.m_sndFile.GetNumSamples())
		{
			client.m_sndFile.m_nSamples = client.m_sample;
		}
		ModSample &sample = client.m_sndFile.GetSample(client.m_sample);

		if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO && metadata->data.stream_info.total_samples != 0)
		{
			// Init sample information
			client.m_sndFile.DestroySampleThreadsafe(client.m_sample);
			client.m_sndFile.m_szNames[client.m_sample] = "";
			sample.Initialize();
			sample.uFlags.set(CHN_16BIT, metadata->data.stream_info.bits_per_sample > 8);
			sample.uFlags.set(CHN_STEREO, metadata->data.stream_info.channels > 1);
			sample.nLength = mpt::saturate_cast<SmpLength>(metadata->data.stream_info.total_samples);
			LimitMax(sample.nLength, MAX_SAMPLE_LENGTH);
			sample.nC5Speed = metadata->data.stream_info.sample_rate;
			client.m_ready = (sample.AllocateSample() != 0);
		} else if(metadata->type == FLAC__METADATA_TYPE_APPLICATION && !memcmp(metadata->data.application.id, "riff", 4) && client.m_ready)
		{
			// Try reading RIFF loop points and other sample information
			FileReader data(mpt::as_span(metadata->data.application.data, metadata->length));
			FileReader::ChunkList<RIFFChunk> chunks = data.ReadChunks<RIFFChunk>(2);

			// We're not really going to read a WAV file here because there will be only one RIFF chunk per metadata event, but we can still re-use the code for parsing RIFF metadata...
			WAVReader riffReader(data);
			riffReader.FindMetadataChunks(chunks);
			riffReader.ApplySampleSettings(sample, client.m_sndFile.GetCharsetInternal(), client.m_sndFile.m_szNames[client.m_sample]);
		} else if(metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT && client.m_ready)
		{
			// Try reading Vorbis Comments for sample title, sample rate and loop points
			SmpLength loopStart = 0, loopLength = 0;
			for(FLAC__uint32 i = 0; i < metadata->data.vorbis_comment.num_comments; i++)
			{
				const char *tag = mpt::byte_cast<const char *>(metadata->data.vorbis_comment.comments[i].entry);
				const FLAC__uint32 length = metadata->data.vorbis_comment.comments[i].length;
				if(length > 6 && !mpt::CompareNoCaseAscii(tag, "TITLE=", 6))
				{
					client.m_sndFile.m_szNames[client.m_sample] = mpt::ToCharset(client.m_sndFile.GetCharsetInternal(), mpt::Charset::UTF8, mpt::String::ReadBuf(mpt::String::maybeNullTerminated, tag + 6, length - 6));
				} else if(length > 11 && !mpt::CompareNoCaseAscii(tag, "SAMPLERATE=", 11))
				{
					uint32 sampleRate = mpt::parse<uint32>(tag + 11);
					if(sampleRate > 0) sample.nC5Speed = sampleRate;
				} else if(length > 10 && !mpt::CompareNoCaseAscii(tag, "LOOPSTART=", 10))
				{
					loopStart = mpt::parse<SmpLength>(tag + 10);
				} else if(length > 11 && !mpt::CompareNoCaseAscii(tag, "LOOPLENGTH=", 11))
				{
					loopLength = mpt::parse<SmpLength>(tag + 11);
				}
			}
			if(loopLength > 0)
			{
				sample.nLoopStart = loopStart;
				sample.nLoopEnd = loopStart + loopLength;
				sample.uFlags.set(CHN_LOOP);
				sample.SanitizeLoops();
			}
		}
	}

	static void error_cb(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus, void *)
	{
	}
};

#endif // MPT_WITH_FLAC


bool CSoundFile::ReadFLACSample(SAMPLEINDEX sample, FileReader &file)
{
#ifdef MPT_WITH_FLAC
	file.Rewind();
	bool isOgg = false;
#ifdef MPT_WITH_OGG
	uint32 oggFlacBitstreamSerial = 0;
#endif
	// Check whether we are dealing with native FLAC, OggFlac or no FLAC at all.
	if(file.ReadMagic("fLaC"))
	{ // ok
		isOgg = false;
#ifdef MPT_WITH_OGG
	} else if(file.ReadMagic("OggS"))
	{ // use libogg to find the first OggFlac stream header
		file.Rewind();
		bool oggOK = false;
		bool needMoreData = true;
		constexpr long bufsize = 65536;
		std::size_t readSize = 0;
		char *buf = nullptr;
		ogg_sync_state oy;
		MemsetZero(oy);
		ogg_page og;
		MemsetZero(og);
		std::map<uint32, ogg_stream_state*> oggStreams;
		ogg_packet op;
		MemsetZero(op);
		if(ogg_sync_init(&oy) != 0)
		{
			return false;
		}
		while(needMoreData)
		{
			if(file.NoBytesLeft())
			{ // stop at EOF
				oggOK = false;
				needMoreData = false;
				break;
			}
			buf = ogg_sync_buffer(&oy, bufsize);
			if(!buf)
			{
				oggOK = false;
				needMoreData = false;
				break;
			}
			readSize = file.ReadRaw(mpt::span(buf, bufsize)).size();
			if(ogg_sync_wrote(&oy, static_cast<long>(readSize)) != 0)
			{
				oggOK = false;
				needMoreData = false;
				break;
			}
			while(ogg_sync_pageout(&oy, &og) == 1)
			{
				if(!ogg_page_bos(&og))
				{ // we stop scanning when seeing the first noo-begin-of-stream page
					oggOK = false;
					needMoreData = false;
					break;
				}
				uint32 serial = ogg_page_serialno(&og);
				if(!oggStreams[serial])
				{ // previously unseen stream serial
					oggStreams[serial] = new ogg_stream_state();
					MemsetZero(*(oggStreams[serial]));
					if(ogg_stream_init(oggStreams[serial], serial) != 0)
					{
						delete oggStreams[serial];
						oggStreams.erase(serial);
						oggOK = false;
						needMoreData = false;
						break;
					}
				}
				if(ogg_stream_pagein(oggStreams[serial], &og) != 0)
				{ // invalid page
					oggOK = false;
					needMoreData = false;
					break;
				}
				if(ogg_stream_packetout(oggStreams[serial], &op) != 1)
				{ // partial or broken packet, continue with more data
					continue;
				}
				if(op.packetno != 0)
				{ // non-begin-of-stream packet.
					// This should not appear on first page for any known ogg codec,
					// but deal gracefully with badly mused streams in that regard.
					continue;
				}
				FileReader packet(mpt::as_span(op.packet, op.bytes));
				if(packet.ReadIntLE<uint8>() == 0x7f && packet.ReadMagic("FLAC"))
				{ // looks like OggFlac
					oggOK = true;
					oggFlacBitstreamSerial = serial;
					needMoreData = false;
					break;
				}
			}
		}
		while(oggStreams.size() > 0)
		{
			uint32 serial = oggStreams.begin()->first;
			ogg_stream_clear(oggStreams[serial]);
			delete oggStreams[serial];
			oggStreams.erase(serial);
		}
		ogg_sync_clear(&oy);
		if(!oggOK)
		{
			return false;
		}
		isOgg = true;
#else // !MPT_WITH_OGG
	} else if(file.CanRead(78) && file.ReadMagic("OggS"))
	{ // first OggFlac page is exactly 78 bytes long
		// only support plain OggFlac here with the FLAC logical bitstream being the first one
		uint8 oggPageVersion = file.ReadIntLE<uint8>();
		uint8 oggPageHeaderType = file.ReadIntLE<uint8>();
		uint64 oggPageGranulePosition = file.ReadIntLE<uint64>();
		uint32 oggPageBitstreamSerialNumber = file.ReadIntLE<uint32>();
		uint32 oggPageSequenceNumber = file.ReadIntLE<uint32>();
		uint32 oggPageChecksum = file.ReadIntLE<uint32>();
		uint8 oggPageSegments = file.ReadIntLE<uint8>();
		uint8 oggPageSegmentLength = file.ReadIntLE<uint8>();
		if(oggPageVersion != 0)
		{ // unknown Ogg version
			return false;
		}
		if(!(oggPageHeaderType & 0x02) || (oggPageHeaderType& 0x01))
		{ // not BOS or continuation
			return false;
		}
		if(oggPageGranulePosition != 0)
		{ // not starting position
			return false;
		}
		if(oggPageSequenceNumber != 0)
		{ // not first page
			return false;
		}
		// skip CRC check for now
		if(oggPageSegments != 1)
		{ // first OggFlac page must contain exactly 1 segment
			return false;
		}
		if(oggPageSegmentLength != 51)
		{ // segment length must be 51 bytes in OggFlac mapping
			return false;
		}
		if(file.ReadIntLE<uint8>() != 0x7f)
		{ // OggFlac mapping demands 0x7f packet type
			return false;
		}
		if(!file.ReadMagic("FLAC"))
		{ // OggFlac magic
			return false;
		}
		if(file.ReadIntLE<uint8>() != 0x01)
		{ // OggFlac major version
			return false;
		}
		// by now, we are pretty confident that we are not parsing random junk
		isOgg = true;
#endif // MPT_WITH_OGG
	} else
	{
		return false;
	}
	file.Rewind();

	FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
	if(decoder == nullptr)
	{
		return false;
	}

#ifdef MPT_WITH_OGG
	if(isOgg)
	{
		// force flac decoding of the logical bitstream that actually is OggFlac
		if(!FLAC__stream_decoder_set_ogg_serial_number(decoder, oggFlacBitstreamSerial))
		{
			FLAC__stream_decoder_delete(decoder);
			return false;
		}
	}
#endif

	// Give me all the metadata!
	FLAC__stream_decoder_set_metadata_respond_all(decoder);

	FLACDecoder client(file, *this, sample);

	// Init decoder
	FLAC__StreamDecoderInitStatus initStatus = isOgg ?
		FLAC__stream_decoder_init_ogg_stream(decoder, FLACDecoder::read_cb, FLACDecoder::seek_cb, FLACDecoder::tell_cb, FLACDecoder::length_cb, FLACDecoder::eof_cb, FLACDecoder::write_cb, FLACDecoder::metadata_cb, FLACDecoder::error_cb, &client)
		:
		FLAC__stream_decoder_init_stream(decoder, FLACDecoder::read_cb, FLACDecoder::seek_cb, FLACDecoder::tell_cb, FLACDecoder::length_cb, FLACDecoder::eof_cb, FLACDecoder::write_cb, FLACDecoder::metadata_cb, FLACDecoder::error_cb, &client)
		;
	if(initStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		FLAC__stream_decoder_delete(decoder);
		return false;
	}

	// Decode file
	FLAC__stream_decoder_process_until_end_of_stream(decoder);
	FLAC__stream_decoder_finish(decoder);
	FLAC__stream_decoder_delete(decoder);

	if(client.m_ready && Samples[sample].HasSampleData())
	{
		Samples[sample].Convert(MOD_TYPE_IT, GetType());
		Samples[sample].PrecomputeLoops(*this, false);
		return true;
	}
#else
	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);
#endif // MPT_WITH_FLAC
	return false;
}


#ifdef MPT_WITH_FLAC

// RAII-style helper struct for FLAC encoder
struct FLAC__StreamEncoder_RAII
{
	std::ostream &f;
	FLAC__StreamEncoder *encoder = nullptr;

	operator FLAC__StreamEncoder *() { return encoder; }

	FLAC__StreamEncoder_RAII(std::ostream &f_) : f(f_), encoder(FLAC__stream_encoder_new()) { }
	~FLAC__StreamEncoder_RAII()
	{
		FLAC__stream_encoder_delete(encoder);
	}

	static FLAC__StreamEncoderWriteStatus StreamEncoderWriteCallback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
	{
		mpt::IO::ofstream & file = *mpt::void_ptr<mpt::IO::ofstream>(client_data);
		MPT_UNUSED_VARIABLE(encoder);
		MPT_UNUSED_VARIABLE(samples);
		MPT_UNUSED_VARIABLE(current_frame);
		if(!mpt::IO::WriteRaw(file, mpt::as_span(buffer, bytes)))
		{
			return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
		}
		return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
	}
	static FLAC__StreamEncoderSeekStatus StreamEncoderSeekCallback(const FLAC__StreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)
	{
		mpt::IO::ofstream & file = *mpt::void_ptr<mpt::IO::ofstream>(client_data);
		MPT_UNUSED_VARIABLE(encoder);
		if(!mpt::in_range<mpt::IO::Offset>(absolute_byte_offset))
		{
			return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
		}
		if(!mpt::IO::SeekAbsolute(file, static_cast<mpt::IO::Offset>(absolute_byte_offset)))
		{
			return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
		}
		return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
	}
	static FLAC__StreamEncoderTellStatus StreamEncoderTellCallback(const FLAC__StreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
	{
		mpt::IO::ofstream & file = *mpt::void_ptr<mpt::IO::ofstream>(client_data);
		MPT_UNUSED_VARIABLE(encoder);
		mpt::IO::Offset pos = mpt::IO::TellWrite(file);
		if(pos < 0)
		{
			return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
		}
		if(!mpt::in_range<FLAC__uint64>(pos))
		{
			return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
		}
		*absolute_byte_offset = static_cast<FLAC__uint64>(pos);
		return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
	}

};

class FLAC__StreamMetadata_RAII : public std::vector<FLAC__StreamMetadata *>
{
public:
	FLAC__StreamMetadata_RAII(std::initializer_list<FLAC__StreamMetadata *> init)
		: std::vector<FLAC__StreamMetadata *>(init)
	{ }

	~FLAC__StreamMetadata_RAII()
	{
		for(auto m : *this)
		{
			FLAC__metadata_object_delete(m);
		}
	}
};

#endif


#ifndef MODPLUG_NO_FILESAVE
bool CSoundFile::SaveFLACSample(SAMPLEINDEX nSample, std::ostream &f) const
{
#ifdef MPT_WITH_FLAC
	const ModSample &sample = Samples[nSample];
	if(sample.uFlags[CHN_ADLIB] || !sample.HasSampleData())
		return false;

	FLAC__StreamEncoder_RAII encoder(f);
	if(encoder == nullptr)
		return false;

	uint32 sampleRate = sample.GetSampleRate(GetType());

	// First off, set up all the metadata...
	FLAC__StreamMetadata_RAII metadata =
	{
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT),
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION),	// MPT sample information
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION),	// Loop points
		FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION),	// Cue points
	};

	unsigned numBlocks = 2;
	if(metadata[0])
	{
		// Store sample name
		FLAC__StreamMetadata_VorbisComment_Entry entry;
		FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "TITLE", mpt::ToCharset(mpt::Charset::UTF8, GetCharsetInternal(), m_szNames[nSample]).c_str());
		FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false);
		FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ENCODER", mpt::ToCharset(mpt::Charset::UTF8, Version::Current().GetOpenMPTVersionString()).c_str());
		FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false);
		if(sampleRate > FLAC__MAX_SAMPLE_RATE)
		{
			// FLAC only supports a sample rate of up to 1048575 Hz.
			// Store the real sample rate in a custom Vorbis comment.
			FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "SAMPLERATE", mpt::afmt::val(sampleRate).c_str());
			FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false);
		}
	}
	if(metadata[1])
	{
		// Write MPT sample information
		memcpy(metadata[1]->data.application.id, "riff", 4);

		struct
		{
			RIFFChunk header;
			WAVExtraChunk mptInfo;
		} chunk;

		chunk.header.id = RIFFChunk::idxtra;
		chunk.header.length = sizeof(WAVExtraChunk);

		chunk.mptInfo.ConvertToWAV(sample, GetType());

		const uint32 length = sizeof(RIFFChunk) + sizeof(WAVExtraChunk);

		FLAC__metadata_object_application_set_data(metadata[1], reinterpret_cast<FLAC__byte *>(&chunk), length, true);
	}
	if(metadata[numBlocks] && (sample.uFlags[CHN_LOOP | CHN_SUSTAINLOOP] || ModCommand::IsNote(sample.rootNote)))
	{
		// Store loop points / root note information
		memcpy(metadata[numBlocks]->data.application.id, "riff", 4);

		struct
		{
			RIFFChunk header;
			WAVSampleInfoChunk info;
			WAVSampleLoop loops[2];
		} chunk;

		chunk.header.id = RIFFChunk::idsmpl;
		chunk.header.length = sizeof(WAVSampleInfoChunk);

		chunk.info.ConvertToWAV(sample.GetSampleRate(GetType()), sample.rootNote);

		if(sample.uFlags[CHN_SUSTAINLOOP])
		{
			chunk.loops[chunk.info.numLoops++].ConvertToWAV(sample.nSustainStart, sample.nSustainEnd, sample.uFlags[CHN_PINGPONGSUSTAIN]);
			chunk.header.length += sizeof(WAVSampleLoop);
		}
		if(sample.uFlags[CHN_LOOP])
		{
			chunk.loops[chunk.info.numLoops++].ConvertToWAV(sample.nLoopStart, sample.nLoopEnd, sample.uFlags[CHN_PINGPONGLOOP]);
			chunk.header.length += sizeof(WAVSampleLoop);
		} else if(sample.uFlags[CHN_SUSTAINLOOP])
		{
			// Invent zero-length loop to distinguish sustain loop from normal loop
			chunk.loops[chunk.info.numLoops++].ConvertToWAV(0, 0, false);
			chunk.header.length += sizeof(WAVSampleLoop);
		}

		const uint32 length = sizeof(RIFFChunk) + chunk.header.length;

		FLAC__metadata_object_application_set_data(metadata[numBlocks], reinterpret_cast<FLAC__byte *>(&chunk), length, true);
		numBlocks++;
	}
	if(metadata[numBlocks] && sample.HasCustomCuePoints())
	{
		// Store cue points
		memcpy(metadata[numBlocks]->data.application.id, "riff", 4);

		struct
		{
			RIFFChunk header;
			uint32le numPoints;
			WAVCuePoint cues[mpt::array_size<decltype(sample.cues)>::size];
		} chunk{};

		chunk.header.id = RIFFChunk::idcue_;
		chunk.header.length = 4 + sizeof(chunk.cues);
		chunk.numPoints = mpt::saturate_cast<uint32>(std::size(sample.cues));

		for(uint32 i = 0; i < std::size(sample.cues); i++)
		{
			chunk.cues[i] = ConvertToWAVCuePoint(i, sample.cues[i]);
		}

		const uint32 length = sizeof(RIFFChunk) + chunk.header.length;

		FLAC__metadata_object_application_set_data(metadata[numBlocks], reinterpret_cast<FLAC__byte *>(&chunk), length, true);
		numBlocks++;
	}

	// FLAC allows a maximum sample rate of 1048575 Hz.
	// If the real rate is higher, we store it in a Vorbis comment above.
	LimitMax(sampleRate, FLAC__MAX_SAMPLE_RATE);
	if(!FLAC__format_sample_rate_is_subset(sampleRate))
	{
		// FLAC only supports 10 Hz granularity for frequencies above 65535 Hz if the streamable subset is chosen, and only a maximum frequency of 655350 Hz.
		FLAC__stream_encoder_set_streamable_subset(encoder, false);
	}
	FLAC__stream_encoder_set_channels(encoder, sample.GetNumChannels());
	FLAC__stream_encoder_set_bits_per_sample(encoder, sample.GetElementarySampleSize() * 8);
	FLAC__stream_encoder_set_sample_rate(encoder, sampleRate);
	FLAC__stream_encoder_set_total_samples_estimate(encoder, sample.nLength);
	FLAC__stream_encoder_set_metadata(encoder, metadata.data(), numBlocks);
#ifdef MODPLUG_TRACKER
	FLAC__stream_encoder_set_compression_level(encoder, TrackerSettings::Instance().m_FLACCompressionLevel);
#if (FLAC_API_VERSION_CURRENT >= 14) && MPT_PLATFORM_MULTITHREADED && !defined(MPT_LIBCXX_QUIRK_NO_STD_THREAD)
	uint32 threads = TrackerSettings::Instance().m_FLACMultithreading ? static_cast<uint32>(std::max(std::thread::hardware_concurrency(), static_cast<unsigned int>(1))) : static_cast<uint32>(1);
	// Work-around <https://github.com/xiph/flac/issues/823>.
	//FLAC__stream_encoder_set_num_threads(encoder, threads);
	while((FLAC__stream_encoder_set_num_threads(encoder, threads) == FLAC__STREAM_ENCODER_SET_NUM_THREADS_TOO_MANY_THREADS) && (threads > 1))
	{
		threads = ((threads > 256) ? 256 : (threads - 1));
	}
#endif
#endif // MODPLUG_TRACKER

	bool success = FLAC__stream_encoder_init_stream(encoder, &FLAC__StreamEncoder_RAII::StreamEncoderWriteCallback, &FLAC__StreamEncoder_RAII::StreamEncoderSeekCallback, &FLAC__StreamEncoder_RAII::StreamEncoderTellCallback, nullptr, &encoder.f) == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
	
	// Convert and encode sample data
	SmpLength framesRemain = sample.nLength, framesRead = 0;
	const uint8 numChannels = sample.GetNumChannels();
	FLAC__int32 buffer[mpt::IO::BUFFERSIZE_TINY];
	while(framesRemain && success)
	{
		const SmpLength copyFrames = std::min(framesRemain, mpt::saturate_cast<SmpLength>(std::size(buffer) / numChannels));

		// First, convert to a 32-bit integer buffer
		switch(sample.GetElementarySampleSize())
		{
		case 1: std::copy(sample.sample8() + framesRead * numChannels, sample.sample8() + (framesRead + copyFrames) * numChannels, std::begin(buffer)); break;
		case 2: std::copy(sample.sample16() + framesRead * numChannels, sample.sample16() + (framesRead + copyFrames) * numChannels, std::begin(buffer)); break;
		default: MPT_ASSERT_NOTREACHED();
		}

		// Now do the actual encoding
		success = FLAC__stream_encoder_process_interleaved(encoder, buffer, copyFrames) != static_cast<FLAC__bool>(false);

		framesRead += copyFrames;
		framesRemain -= copyFrames;
	}

	FLAC__stream_encoder_finish(encoder);

	return success;
#else
	MPT_UNREFERENCED_PARAMETER(nSample);
	MPT_UNREFERENCED_PARAMETER(f);
	return false;
#endif // MPT_WITH_FLAC
}
#endif // MODPLUG_NO_FILESAVE


OPENMPT_NAMESPACE_END
