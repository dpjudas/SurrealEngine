/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "openmpt/streamencoder/StreamEncoderFLAC.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/pointer.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/random/any_engine.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <algorithm>
#include <memory>
#include <string>
#ifdef MPT_WITH_FLAC
#if MPT_PLATFORM_MULTITHREADED && !defined(MPT_LIBCXX_QUIRK_NO_STD_THREAD)
#include <thread>
#endif
#endif  // MPT_WITH_FLAC
#include <vector>

#include <cassert>
#include <cstddef>

#ifdef MPT_WITH_FLAC
#include <FLAC/metadata.h>
#include <FLAC/format.h>
#include <FLAC/stream_encoder.h>
#endif  // MPT_WITH_FLAC



OPENMPT_NAMESPACE_BEGIN


#ifdef MPT_WITH_FLAC

class FLACStreamWriter : public StreamWriterBase
{
private:
	[[maybe_unused]] const FLACEncoder &enc;
	Encoder::Settings settings;
	FLAC__StreamMetadata *flac_metadata[1];
	FLAC__StreamEncoder *encoder;
	std::vector<FLAC__int32> sampleBuf;
private:
	static FLAC__StreamEncoderWriteStatus FLACWriteCallback(const FLAC__StreamEncoder *flacenc, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data)
	{
		return mpt::void_ptr<FLACStreamWriter>(client_data)->WriteCallback(flacenc, buffer, bytes, samples, current_frame);
	}
	static FLAC__StreamEncoderSeekStatus FLACSeekCallback(const FLAC__StreamEncoder *flacenc, FLAC__uint64 absolute_byte_offset, void *client_data)
	{
		return mpt::void_ptr<FLACStreamWriter>(client_data)->SeekCallback(flacenc, absolute_byte_offset);
	}
	static FLAC__StreamEncoderTellStatus FLACTellCallback(const FLAC__StreamEncoder *flacenc, FLAC__uint64 *absolute_byte_offset, void *client_data)
	{
		return mpt::void_ptr<FLACStreamWriter>(client_data)->TellCallback(flacenc, absolute_byte_offset);
	}
	FLAC__StreamEncoderWriteStatus WriteCallback(const FLAC__StreamEncoder *flacenc, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame)
	{
		MPT_UNUSED(flacenc);
		MPT_UNUSED(samples);
		MPT_UNUSED(current_frame);
		mpt::IO::WriteRaw(f, mpt::as_span(buffer, bytes));
		if(!f) return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
		return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
	}
	FLAC__StreamEncoderSeekStatus SeekCallback(const FLAC__StreamEncoder *flacenc, FLAC__uint64 absolute_byte_offset)
	{
		MPT_UNUSED(flacenc);
		mpt::IO::SeekAbsolute(f, absolute_byte_offset);
		if(!f) return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
		return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
	}
	FLAC__StreamEncoderTellStatus TellCallback(const FLAC__StreamEncoder *flacenc, FLAC__uint64 *absolute_byte_offset)
	{
		MPT_UNUSED(flacenc);
		if(absolute_byte_offset)
		{
			*absolute_byte_offset = mpt::IO::TellWrite(f);
		}
		if(!f) return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
		return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
	}
private:
	void AddCommentField(const std::string &field, const mpt::ustring &data)
	{
		if(!field.empty() && !data.empty())
		{
			FLAC__StreamMetadata_VorbisComment_Entry entry;
			FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, field.c_str(), mpt::transcode<std::string>(mpt::common_encoding::utf8, data).c_str());
			FLAC__metadata_object_vorbiscomment_append_comment(flac_metadata[0], entry, false);
		}
	}
public:
	FLACStreamWriter(const FLACEncoder &enc_, std::ostream &stream, const Encoder::Settings &settings_, const FileTags &tags)
		: StreamWriterBase(stream)
		, enc(enc_)
		, settings(settings_)
	{
		flac_metadata[0] = nullptr;
		encoder = nullptr;

		assert(settings.Samplerate > 0);
		assert(settings.Channels > 0);

		encoder = FLAC__stream_encoder_new();

		FLAC__stream_encoder_set_channels(encoder, settings.Channels);
		FLAC__stream_encoder_set_bits_per_sample(encoder, settings.Format.GetSampleFormat().GetBitsPerSample());
		FLAC__stream_encoder_set_sample_rate(encoder, settings.Samplerate);

		int compressionLevel = settings.Details.FLACCompressionLevel;
		FLAC__stream_encoder_set_compression_level(encoder, compressionLevel);

#if(FLAC_API_VERSION_CURRENT >= 14) && MPT_PLATFORM_MULTITHREADED && !defined(MPT_LIBCXX_QUIRK_NO_STD_THREAD)
		uint32 threads = settings.Details.FLACMultithreading ? static_cast<uint32>(std::max(std::thread::hardware_concurrency(), static_cast<unsigned int>(1))) : static_cast<uint32>(1);
		// Work-around <https://github.com/xiph/flac/issues/823>.
		//FLAC__stream_encoder_set_num_threads(encoder, threads);
		while((FLAC__stream_encoder_set_num_threads(encoder, threads) == FLAC__STREAM_ENCODER_SET_NUM_THREADS_TOO_MANY_THREADS) && (threads > 1))
		{
			threads = ((threads > 256) ? 256 : (threads - 1));
		}
#endif

		if(settings.Tags)
		{
			flac_metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
			AddCommentField("ENCODER", tags.encoder);
			AddCommentField("SOURCEMEDIA", MPT_USTRING("tracked music file"));
			AddCommentField("TITLE", tags.title);
			AddCommentField("ARTIST", tags.artist);
			AddCommentField("ALBUM", tags.album);
			AddCommentField("DATE", tags.year);
			AddCommentField("COMMENT", tags.comments);
			AddCommentField("GENRE", tags.genre);
			AddCommentField("CONTACT", tags.url);
			AddCommentField("BPM", tags.bpm);  // non-standard
			AddCommentField("TRACKNUMBER", tags.trackno);
			FLAC__stream_encoder_set_metadata(encoder, flac_metadata, 1);
		}

		FLAC__stream_encoder_init_stream(encoder, FLACWriteCallback, FLACSeekCallback, FLACTellCallback, nullptr, this);
	}
	SampleFormat GetSampleFormat() const override
	{
		return settings.Format.GetSampleFormat();
	}
	template <typename Tsample>
	void WriteInterleavedInt(std::size_t frameCount, const Tsample *p)
	{
		assert(settings.Format.GetSampleFormat() == SampleFormatTraits<Tsample>::sampleFormat());
		sampleBuf.resize(frameCount * settings.Channels);
		for(std::size_t frame = 0; frame < frameCount; ++frame)
		{
			for(int channel = 0; channel < settings.Channels; ++channel)
			{
				sampleBuf[frame * settings.Channels + channel] = *p;
				p++;
			}
		}
		while(frameCount > 0)
		{
			unsigned int frameCountChunk = mpt::saturate_cast<unsigned int>(frameCount);
			FLAC__stream_encoder_process_interleaved(encoder, sampleBuf.data(), frameCountChunk);
			frameCount -= frameCountChunk;
		}
	}
	void WriteInterleaved(std::size_t frameCount, const int8 *interleaved) override
	{
		WriteInterleavedInt(frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int16 *interleaved) override
	{
		WriteInterleavedInt(frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int24 *interleaved) override
	{
		WriteInterleavedInt(frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int32 *interleaved) override
	{
		WriteInterleavedInt(frameCount, interleaved);
	}
	void WriteFinalize() override
	{
		FLAC__stream_encoder_finish(encoder);
	}
	virtual ~FLACStreamWriter()
	{
		FLAC__stream_encoder_delete(encoder);
		encoder = nullptr;

		if(flac_metadata[0])
		{
			FLAC__metadata_object_delete(flac_metadata[0]);
			flac_metadata[0] = nullptr;
		}
	}
};

#endif  // MPT_WITH_FLAC



FLACEncoder::FLACEncoder()
{
	Encoder::Traits traits;
#if defined(MPT_WITH_FLAC)
	traits.fileExtension = MPT_NATIVE_PATH("flac");
	traits.fileShortDescription = MPT_USTRING("FLAC");
	traits.fileDescription = MPT_USTRING("Free Lossless Audio Codec");
	traits.encoderSettingsName = MPT_USTRING("FLAC");
	traits.canTags = true;
	traits.maxChannels = 4;
	traits.samplerates = {};
	traits.modes = Encoder::ModeLossless;
#if(FLAC_API_VERSION_CURRENT >= 12)
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 32, mpt::get_endian()});
#endif
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 24, mpt::get_endian()});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 16, mpt::get_endian()});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 8, mpt::get_endian()});
	traits.defaultSamplerate = 48000;
	traits.defaultChannels = 2;
	traits.defaultMode = Encoder::ModeLossless;
	traits.defaultFormat = {Encoder::Format::Encoding::Integer, 24, mpt::get_endian()};
#endif  // MPT_WITH_FLAC
	SetTraits(traits);
}


bool FLACEncoder::IsAvailable() const
{
#if defined(MPT_WITH_FLAC)
	return true;
#else   // !MPT_WITH_FLAC
	return false;
#endif  // MPT_WITH_FLAC
}


std::unique_ptr<IAudioStreamEncoder> FLACEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const
{
	if(!IsAvailable())
	{
		return nullptr;
	}
	std::unique_ptr<IAudioStreamEncoder> result = nullptr;
#ifdef MPT_WITH_FLAC
	result = std::make_unique<FLACStreamWriter>(*this, file, settings, tags);
	MPT_UNUSED(prng);
#else   // !MPT_WITH_FLAC
	MPT_UNUSED(file);
	MPT_UNUSED(settings);
	MPT_UNUSED(tags);
	MPT_UNUSED(prng);
#endif  // MPT_WITH_FLAC
	return result;
}


OPENMPT_NAMESPACE_END
