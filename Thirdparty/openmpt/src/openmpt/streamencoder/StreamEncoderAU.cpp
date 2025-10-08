/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "openmpt/streamencoder/StreamEncoderAU.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/random/any_engine.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/string_transcode/macros.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <memory>
#include <ostream>
#include <string>

#include <cassert>
#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


class AUStreamWriter : public IAudioStreamEncoder
{
private:

	[[maybe_unused]] const AUEncoder &enc;
	std::ostream &f;
	Encoder::Settings settings;

private:
	static std::string TagToAnnotation(const std::string &field, const mpt::ustring &tag)
	{
		if(tag.empty())
		{
			return std::string();
		}
		return MPT_AFORMAT_MESSAGE("{}={}\n")(field, mpt::transcode<std::string>(mpt::common_encoding::utf8, mpt::replace(tag, MPT_USTRING("="), MPT_UTF8_STRING("\xEF\xBF\xBD"))));  // U+FFFD
	}

public:
	AUStreamWriter(const AUEncoder &enc_, std::ostream &file, const Encoder::Settings &settings_, const FileTags &tags)
		: enc(enc_)
		, f(file)
		, settings(settings_)
	{

		assert(settings.Samplerate > 0);
		assert(settings.Channels > 0);

		std::string annotation;
		std::size_t annotationSize = 0;
		std::size_t annotationTotalSize = 8;
		if(settings.Tags)
		{
			// same format as invented by sox and implemented by ffmpeg
			annotation += TagToAnnotation("title", tags.title);
			annotation += TagToAnnotation("artist", tags.artist);
			annotation += TagToAnnotation("album", tags.album);
			annotation += TagToAnnotation("track", tags.trackno);
			annotation += TagToAnnotation("genre", tags.genre);
			annotation += TagToAnnotation("comment", tags.comments);
			annotationSize = annotation.length() + 1;
			annotationTotalSize = annotationSize;
			if(settings.Details.AUPaddingAlignHint > 0)
			{
				annotationTotalSize = mpt::align_up<std::size_t>(24u + annotationTotalSize, settings.Details.AUPaddingAlignHint) - 24u;
			}
			annotationTotalSize = mpt::align_up<std::size_t>(annotationTotalSize, 8u);
		}
		assert(annotationTotalSize >= annotationSize);
		assert(annotationTotalSize % 8 == 0);

		mpt::IO::WriteText(f, ".snd");
		mpt::IO::WriteIntBE<uint32>(f, mpt::saturate_cast<uint32>(24u + annotationTotalSize));
		mpt::IO::WriteIntBE<uint32>(f, ~uint32(0));
		uint32 encoding = 0;
		if(settings.Format.encoding == Encoder::Format::Encoding::Float)
		{
			switch(settings.Format.bits)
			{
				case 32: encoding = 6; break;
				case 64: encoding = 7; break;
			}
		} else if(settings.Format.encoding == Encoder::Format::Encoding::Integer)
		{
			switch(settings.Format.bits)
			{
				case 8: encoding = 2; break;
				case 16: encoding = 3; break;
				case 24: encoding = 4; break;
				case 32: encoding = 5; break;
			}
		} else if(settings.Format.encoding == Encoder::Format::Encoding::Alaw)
		{
			encoding = 27;
		} else if(settings.Format.encoding == Encoder::Format::Encoding::ulaw)
		{
			encoding = 1;
		}
		mpt::IO::WriteIntBE<uint32>(f, encoding);
		mpt::IO::WriteIntBE<uint32>(f, settings.Samplerate);
		mpt::IO::WriteIntBE<uint32>(f, settings.Channels);
		if(annotationSize > 0)
		{
			mpt::IO::WriteText(f, annotation);
			mpt::IO::WriteIntBE<uint8>(f, '\0');
		}
		for(std::size_t i = 0; i < annotationTotalSize - annotationSize; ++i)
		{
			mpt::IO::WriteIntBE<uint8>(f, '\0');
		}
	}

	SampleFormat GetSampleFormat() const override
	{
		return settings.Format.GetSampleFormat();
	}

	void WriteInterleaved(std::size_t frameCount, const double *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const float *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int32 *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int24 *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int16 *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int8 *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const uint8 *interleaved) override
	{
		WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}

	void WriteCues(const std::vector<uint64> &cues) override
	{
		MPT_UNUSED(cues);
	}
	void WriteFinalize() override
	{
		return;
	}
	virtual ~AUStreamWriter()
	{
		return;
	}
};



AUEncoder::AUEncoder()
{
	Encoder::Traits traits;
	traits.fileExtension = MPT_NATIVE_PATH("au");
	traits.fileShortDescription = MPT_USTRING("AU");
	traits.fileDescription = MPT_USTRING("NeXT/Sun Audio");
	traits.encoderSettingsName = MPT_USTRING("AU");
	traits.canTags = true;
	traits.canCues = false;
	traits.maxChannels = 4;
	traits.samplerates = {};
	traits.modes = Encoder::ModeLossless;
	traits.formats.push_back({Encoder::Format::Encoding::Float, 64, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Float, 32, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 32, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 24, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 16, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 8, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Alaw, 16, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::ulaw, 16, mpt::endian::big});
	traits.defaultSamplerate = 48000;
	traits.defaultChannels = 2;
	traits.defaultMode = Encoder::ModeLossless;
	traits.defaultFormat = {Encoder::Format::Encoding::Float, 32, mpt::endian::big};
	SetTraits(traits);
}


bool AUEncoder::IsAvailable() const
{
	return true;
}


std::unique_ptr<IAudioStreamEncoder> AUEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const
{
	if(!IsAvailable())
	{
		return nullptr;
	}
	MPT_UNUSED(prng);
	return std::make_unique<AUStreamWriter>(*this, file, settings, tags);
}


OPENMPT_NAMESPACE_END
