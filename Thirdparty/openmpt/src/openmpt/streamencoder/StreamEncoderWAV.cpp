/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "openmpt/streamencoder/StreamEncoderWAV.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/random/any_engine.hpp"
#include "mpt/string/types.hpp"

#include "openmpt/base/Endian.hpp"
#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/SampleFormat.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/soundfile_data/wav.hpp"
#include "openmpt/soundfile_write/wav_write.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <cassert>



OPENMPT_NAMESPACE_BEGIN


class WavStreamWriter : public IAudioStreamEncoder
{
private:

	[[maybe_unused]] const WAVEncoder &enc;
	std::ostream &f;
	mpt::IO::OFile<std::ostream> ff;
	std::unique_ptr<WAVWriter> fileWAV;
	Encoder::Settings settings;

public:
	WavStreamWriter(const WAVEncoder &enc_, std::ostream &file, const Encoder::Settings &settings_, const FileTags &tags)
		: enc(enc_)
		, f(file)
		, ff(f)
		, fileWAV(nullptr)
		, settings(settings_)
	{

		assert(settings.Samplerate > 0);
		assert(settings.Channels > 0);

		fileWAV = std::make_unique<WAVWriter>(ff);
		fileWAV->WriteFormat(settings.Samplerate, settings.Format.GetSampleFormat().GetBitsPerSample(), settings.Channels, settings.Format.GetSampleFormat().IsFloat() ? WAVFormatChunk::fmtFloat : WAVFormatChunk::fmtPCM);

		if(settings.Tags)
		{
			fileWAV->WriteMetatags(tags);
		}

		fileWAV->StartChunk(RIFFChunk::iddata);
	}
	SampleFormat GetSampleFormat() const override
	{
		return settings.Format.GetSampleFormat();
	}
	void WriteInterleaved(std::size_t frameCount, const double *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const float *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int32 *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int24 *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int16 *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const int8 *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteInterleaved(std::size_t frameCount, const uint8 *interleaved) override
	{
		WriteInterleavedLE(f, settings.Channels, settings.Format, frameCount, interleaved);
	}
	void WriteCues(const std::vector<uint64> &cues) override
	{
		if(!cues.empty())
		{
			// Cue point header
			fileWAV->StartChunk(RIFFChunk::idcue_);
			uint32le numPoints;
			numPoints = mpt::saturate_cast<uint32>(cues.size());
			mpt::IO::Write(f, numPoints);

			// Write all cue points
			uint32 index = 0;
			for(auto cue : cues)
			{
				WAVCuePoint cuePoint{};
				cuePoint.id = index++;
				cuePoint.position = static_cast<uint32>(cue);
				cuePoint.riffChunkID = static_cast<uint32>(RIFFChunk::iddata);
				cuePoint.chunkStart = 0;  // we use no Wave List Chunk (wavl) as we have only one data block, so this should be 0.
				cuePoint.blockStart = 0;  // ditto
				cuePoint.offset = cuePoint.position;
				mpt::IO::Write(f, cuePoint);
			}
		}
	}
	void WriteFinalize() override
	{
		fileWAV->Finalize();
	}
	virtual ~WavStreamWriter()
	{
		fileWAV = nullptr;
	}
};



WAVEncoder::WAVEncoder()
{
	Encoder::Traits traits;
	traits.fileExtension = MPT_NATIVE_PATH("wav");
	traits.fileShortDescription = MPT_USTRING("Wave");
	traits.fileDescription = MPT_USTRING("Microsoft RIFF Wave");
	traits.encoderSettingsName = MPT_USTRING("Wave");
	traits.canTags = true;
	traits.canCues = true;
	traits.maxChannels = 4;
	traits.samplerates = {};
	traits.modes = Encoder::ModeLossless;
	traits.formats.push_back({Encoder::Format::Encoding::Float, 64, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Float, 32, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 32, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 24, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Integer, 16, mpt::endian::little});
	traits.formats.push_back({Encoder::Format::Encoding::Unsigned, 8, mpt::endian::little});
	traits.defaultSamplerate = 48000;
	traits.defaultChannels = 2;
	traits.defaultMode = Encoder::ModeLossless;
	traits.defaultFormat = {Encoder::Format::Encoding::Float, 32, mpt::endian::little};
	SetTraits(traits);
}


bool WAVEncoder::IsAvailable() const
{
	return true;
}


std::unique_ptr<IAudioStreamEncoder> WAVEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const
{
	if(!IsAvailable())
	{
		return nullptr;
	}
	MPT_UNUSED(prng);
	return std::make_unique<WavStreamWriter>(*this, file, settings, tags);
}


OPENMPT_NAMESPACE_END
