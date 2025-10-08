/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "openmpt/streamencoder/StreamEncoderVorbis.hpp"

#include "mpt/base/alloc.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/format/message.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/random/any_engine.hpp"
#include "mpt/random/random.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <algorithm>
#include <memory>
#include <ostream>

#include <cstddef>
#include <cstring>

#ifdef MPT_WITH_OGG
#include <ogg/ogg.h>
#endif
#ifdef MPT_WITH_VORBIS
#include <vorbis/codec.h>
#endif
#ifdef MPT_WITH_VORBISENC
#include <vorbis/vorbisenc.h>
#endif



OPENMPT_NAMESPACE_BEGIN



static Encoder::Traits VorbisBuildTraits()
{
	Encoder::Traits traits;
#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISENC)
	traits.fileExtension = MPT_NATIVE_PATH("ogg");
	traits.fileShortDescription = MPT_USTRING("Vorbis");
	traits.fileDescription = MPT_USTRING("Ogg Vorbis");
	traits.encoderSettingsName = MPT_USTRING("Vorbis");
	traits.canTags = true;
	traits.maxChannels = 4;
	traits.samplerates = mpt::make_vector(vorbis_samplerates);
	traits.modes = Encoder::ModeABR | Encoder::ModeQuality;
	traits.bitrates = mpt::make_vector(vorbis_bitrates);
	traits.defaultSamplerate = 48000;
	traits.defaultChannels = 2;
	traits.defaultMode = Encoder::ModeQuality;
	traits.defaultBitrate = 160;
	traits.defaultQuality = 0.5;
#endif
	return traits;
}

#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISENC)

class VorbisStreamWriter : public StreamWriterBase
{
private:
	ogg_stream_state os;
	ogg_page og;
	ogg_packet op;
	vorbis_info vi;
	vorbis_comment vc;
	vorbis_dsp_state vd;
	vorbis_block vb;
	int vorbis_channels;
private:
	void WritePage()
	{
		buf.resize(og.header_len);
		std::memcpy(buf.data(), og.header, og.header_len);
		WriteBuffer();
		buf.resize(og.body_len);
		std::memcpy(buf.data(), og.body, og.body_len);
		WriteBuffer();
	}
	void AddCommentField(const std::string &field, const mpt::ustring &data)
	{
		if(!field.empty() && !data.empty())
		{
			vorbis_comment_add_tag(&vc, field.c_str(), mpt::transcode<std::string>(mpt::common_encoding::utf8, data).c_str());
		}
	}
public:
	VorbisStreamWriter(std::ostream &stream, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng)
		: StreamWriterBase(stream)
	{
		vorbis_channels = 0;

		vorbis_channels = settings.Channels;

		vorbis_info_init(&vi);
		vorbis_comment_init(&vc);

		if(settings.Mode == Encoder::ModeQuality)
		{
			vorbis_encode_init_vbr(&vi, vorbis_channels, settings.Samplerate, settings.Quality);
		} else
		{
			vorbis_encode_init(&vi, vorbis_channels, settings.Samplerate, -1, settings.Bitrate * 1000, -1);
		}

		vorbis_analysis_init(&vd, &vi);
		vorbis_block_init(&vd, &vb);
		ogg_stream_init(&os, mpt::random<uint32>(prng));

		if(settings.Tags)
		{
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
		}

		ogg_packet header;
		ogg_packet header_comm;
		ogg_packet header_code;
		vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
		ogg_stream_packetin(&os, &header);
		while(ogg_stream_flush(&os, &og))
		{
			WritePage();
		}
		ogg_stream_packetin(&os, &header_comm);
		ogg_stream_packetin(&os, &header_code);
		while(ogg_stream_flush(&os, &og))
		{
			WritePage();
		}
	}
	void WriteInterleaved(std::size_t count, const float *interleaved) override
	{
		std::size_t countTotal = count;
		while(countTotal > 0)
		{
			int countChunk = mpt::saturate_cast<int>(countTotal);
			countTotal -= countChunk;
			float **buffer = vorbis_analysis_buffer(&vd, countChunk);
			for(int frame = 0; frame < countChunk; ++frame)
			{
				for(int channel = 0; channel < vorbis_channels; ++channel)
				{
					buffer[channel][frame] = interleaved[frame * vorbis_channels + channel];
				}
			}
			vorbis_analysis_wrote(&vd, countChunk);
			while(vorbis_analysis_blockout(&vd, &vb) == 1)
			{
				vorbis_analysis(&vb, NULL);
				vorbis_bitrate_addblock(&vb);
				while(vorbis_bitrate_flushpacket(&vd, &op))
				{
					ogg_stream_packetin(&os, &op);
					while(ogg_stream_pageout(&os, &og))
					{
						WritePage();
					}
				}
			}
		}
	}
	void WriteFinalize() override
	{
		vorbis_analysis_wrote(&vd, 0);
		while(vorbis_analysis_blockout(&vd, &vb) == 1)
		{
			vorbis_analysis(&vb, NULL);
			vorbis_bitrate_addblock(&vb);
			while(vorbis_bitrate_flushpacket(&vd, &op))
			{
				ogg_stream_packetin(&os, &op);
				while(ogg_stream_flush(&os, &og))
				{
					WritePage();
					if(ogg_page_eos(&og))
					{
						break;
					}
				}
			}
		}
	}
	virtual ~VorbisStreamWriter()
	{
		ogg_stream_clear(&os);
		vorbis_block_clear(&vb);
		vorbis_dsp_clear(&vd);
		vorbis_comment_clear(&vc);
		vorbis_info_clear(&vi);
	}
};

#endif  // MPT_WITH_OGG && MPT_WITH_VORBIS && MPT_WITH_VORBISENC



VorbisEncoder::VorbisEncoder()
{
	SetTraits(VorbisBuildTraits());
}


bool VorbisEncoder::IsAvailable() const
{
#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISENC)
	return true;
#else
	return false;
#endif
}


std::unique_ptr<IAudioStreamEncoder> VorbisEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const
{
#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISENC)
	MPT_UNUSED(prng);
	return std::make_unique<VorbisStreamWriter>(file, settings, tags, prng);
#else
	MPT_UNUSED(file);
	MPT_UNUSED(settings);
	MPT_UNUSED(tags);
	MPT_UNUSED(prng);
	return nullptr;
#endif
}


bool VorbisEncoder::IsBitrateSupported(int samplerate, int channels, int bitrate) const
{
#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISENC)
	vorbis_info vi;
	vorbis_info_init(&vi);
	bool result = (0 == vorbis_encode_init(&vi, channels, samplerate, -1, bitrate * 1000, -1));
	vorbis_info_clear(&vi);
	return result;
#else
	MPT_UNUSED(samplerate);
	MPT_UNUSED(channels);
	MPT_UNUSED(bitrate);
	return false;
#endif
}


mpt::ustring VorbisEncoder::DescribeQuality(float quality) const
{
	static constexpr int q_table[11] = {64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500};  // http://wiki.hydrogenaud.io/index.php?title=Recommended_Ogg_Vorbis
	int q = std::clamp(mpt::saturate_round<int>(quality * 10.0f), 0, 10);
	return MPT_UFORMAT_MESSAGE("Q{} (~{} kbit)")(mpt::format<mpt::ustring>::fix(quality * 10.0f, 1), q_table[q]);
}



OPENMPT_NAMESPACE_END
