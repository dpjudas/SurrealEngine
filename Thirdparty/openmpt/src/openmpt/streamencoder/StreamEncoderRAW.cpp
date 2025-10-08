/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "openmpt/streamencoder/StreamEncoderRAW.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/string/types.hpp"

#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <memory>
#include <ostream>

#include <cassert>
#include <cstddef>



OPENMPT_NAMESPACE_BEGIN


class RawStreamWriter : public IAudioStreamEncoder
{
private:
	[[maybe_unused]] const RAWEncoder &enc;
	std::ostream &f;
	Encoder::Settings settings;

public:
	RawStreamWriter(const RAWEncoder &enc_, std::ostream &file, const Encoder::Settings &settings_, const FileTags &tags)
		: enc(enc_)
		, f(file)
		, settings(settings_)
	{
		assert(settings.Samplerate > 0);
		assert(settings.Channels > 0);
		MPT_UNUSED(tags);
	}
	SampleFormat GetSampleFormat() const override
	{
		return settings.Format.GetSampleFormat();
	}
	void WriteInterleaved(std::size_t frameCount, const double *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteInterleaved(std::size_t frameCount, const float *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteInterleaved(std::size_t frameCount, const int32 *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteInterleaved(std::size_t frameCount, const int24 *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteInterleaved(std::size_t frameCount, const int16 *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteInterleaved(std::size_t frameCount, const int8 *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteInterleaved(std::size_t frameCount, const uint8 *interleaved) override
	{
		if(settings.Format.endian == mpt::endian::little)
		{
			WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
		} else
		{
			WriteInterleavedBE(f, settings.Channels, settings.Format, frameCount, interleaved);
		}
	}
	void WriteCues(const std::vector<uint64> &cues) override
	{
		MPT_UNUSED(cues);
	}
	void WriteFinalize() override
	{
		// nothing
	}
	virtual ~RawStreamWriter()
	{
		// nothing
	}
};



RAWEncoder::RAWEncoder()
{
	Encoder::Traits traits;
	traits.fileExtension = MPT_NATIVE_PATH("raw");
	traits.fileShortDescription = MPT_USTRING("Raw PCM");
	traits.fileDescription = MPT_USTRING("Headerless raw PCM");
	traits.encoderSettingsName = MPT_USTRING("RAW");
	traits.canTags = false;
	traits.canCues = false;
	traits.maxChannels = 4;
	traits.samplerates = {};
	traits.modes = Encoder::ModeLossless;
	traits.formats.push_back({Encoder::Format::Encoding::Float, 64, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Float, 64, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Float, 32, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Float, 32, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 32, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 32, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 24, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 24, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 16, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 16, mpt::endian::big});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 8, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Unsigned, 8, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Alaw, 16, mpt::get_endian()});
	traits.formats.push_back({Encoder::Format::Encoding::ulaw, 16, mpt::get_endian()});
	traits.defaultSamplerate = 48000;
	traits.defaultChannels = 2;
	traits.defaultMode = Encoder::ModeLossless;
	traits.defaultFormat = {Encoder::Format::Encoding::Float, 32, mpt::endian::little};
	SetTraits(traits);
}


bool RAWEncoder::IsAvailable() const
{
	return true;
}


std::unique_ptr<IAudioStreamEncoder> RAWEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const
{
	if(!IsAvailable())
	{
		return nullptr;
	}
	MPT_UNUSED(prng);
	return std::make_unique<RawStreamWriter>(*this, file, settings, tags);
}


OPENMPT_NAMESPACE_END
