/*
 * SampleFormatVorbis.cpp
 * ----------------------
 * Purpose: Vorbis sample import
 * Notes  :
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#ifndef MODPLUG_NO_FILESAVE
#include "../common/mptFileIO.h"
#endif
#include "../common/misc_util.h"
#include "Tagging.h"
#include "Loaders.h"
#include "../common/FileReader.h"
#include "modsmp_ctrl.h"
#include "openmpt/soundbase/Copy.hpp"
#include "mpt/audio/span.hpp"
#include "../soundlib/ModSampleCopy.h"
//#include "mpt/crc/crc.hpp"
#include "OggStream.h"
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
#if defined(MPT_WITH_VORBIS)
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif // MPT_COMPILER_CLANG
#include <vorbis/codec.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
#endif // MPT_WITH_VORBIS
#if defined(MPT_WITH_VORBISFILE)
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif // MPT_COMPILER_CLANG
#include <vorbis/vorbisfile.h>
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
#endif // MPT_WITH_VORBISFILE
#ifdef MPT_WITH_STBVORBIS
#include <stb_vorbis.h>
#endif // MPT_WITH_STBVORBIS


OPENMPT_NAMESPACE_BEGIN


////////////////////////////////////////////////////////////////////////////////
// Vorbis

#if defined(MPT_WITH_VORBISFILE)

static size_t VorbisfileFilereaderRead(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	FileReader &file = *mpt::void_ptr<FileReader>(datasource);
	return file.ReadRaw(mpt::span(mpt::void_cast<std::byte*>(ptr), size * nmemb)).size() / size;
}

static int VorbisfileFilereaderSeek(void *datasource, ogg_int64_t offset, int whence)
{
	FileReader &file = *mpt::void_ptr<FileReader>(datasource);
	switch(whence)
	{
	case SEEK_SET:
		{
			if(!mpt::in_range<FileReader::pos_type>(offset))
			{
				return -1;
			}
			return file.Seek(mpt::saturate_cast<FileReader::pos_type>(offset)) ? 0 : -1;
		}
		break;
	case SEEK_CUR:
		{
			if(offset < 0)
			{
				if(offset == std::numeric_limits<ogg_int64_t>::min())
				{
					return -1;
				}
				if(!mpt::in_range<FileReader::pos_type>(0-offset))
				{
					return -1;
				}
				return file.SkipBack(mpt::saturate_cast<FileReader::pos_type>(0 - offset)) ? 0 : -1;
			} else
			{
				if(!mpt::in_range<FileReader::pos_type>(offset))
				{
					return -1;
				}
				return file.Skip(mpt::saturate_cast<FileReader::pos_type>(offset)) ? 0 : -1;
			}
		}
		break;
	case SEEK_END:
		{
			if(!mpt::in_range<FileReader::pos_type>(offset))
			{
				return -1;
			}
			if(!mpt::in_range<FileReader::pos_type>(file.GetLength() + offset))
			{
				return -1;
			}
			return file.Seek(mpt::saturate_cast<FileReader::pos_type>(file.GetLength() + offset)) ? 0 : -1;
		}
		break;
	default:
		return -1;
	}
}

static long VorbisfileFilereaderTell(void *datasource)
{
	FileReader &file = *mpt::void_ptr<FileReader>(datasource);
	MPT_MAYBE_CONSTANT_IF(!mpt::in_range<long>(file.GetPosition()))
	{
		return -1;
	}
	return static_cast<long>(file.GetPosition());
}

#if defined(MPT_WITH_VORBIS)
static mpt::ustring UStringFromVorbis(const char *str)
{
	return str ? mpt::ToUnicode(mpt::Charset::UTF8, str) : mpt::ustring();
}
#endif // MPT_WITH_VORBIS

static FileTags GetVorbisFileTags(OggVorbis_File &vf)
{
	FileTags tags;
	#if defined(MPT_WITH_VORBIS)
		vorbis_comment *vc = ov_comment(&vf, -1);
		if(!vc)
		{
			return tags;
		}
		tags.encoder = UStringFromVorbis(vorbis_comment_query(vc, "ENCODER", 0));
		tags.title = UStringFromVorbis(vorbis_comment_query(vc, "TITLE", 0));
		tags.comments = UStringFromVorbis(vorbis_comment_query(vc, "DESCRIPTION", 0));
		tags.bpm = UStringFromVorbis(vorbis_comment_query(vc, "BPM", 0)); // non-standard
		tags.artist = UStringFromVorbis(vorbis_comment_query(vc, "ARTIST", 0));
		tags.album = UStringFromVorbis(vorbis_comment_query(vc, "ALBUM", 0));
		tags.trackno = UStringFromVorbis(vorbis_comment_query(vc, "TRACKNUMBER", 0));
		tags.year = UStringFromVorbis(vorbis_comment_query(vc, "DATE", 0));
		tags.url = UStringFromVorbis(vorbis_comment_query(vc, "CONTACT", 0));
		tags.genre = UStringFromVorbis(vorbis_comment_query(vc, "GENRE", 0));
	#else // !MPT_WITH_VORBIS
		MPT_UNREFERENCED_PARAMETER(vf);
	#endif // MPT_WITH_VORBIS
	return tags;
}

#endif // MPT_WITH_VORBISFILE

bool CSoundFile::ReadVorbisSample(SAMPLEINDEX sample, FileReader &file)
{

#if defined(MPT_WITH_VORBISFILE) || defined(MPT_WITH_STBVORBIS)

	file.Rewind();

	long rate = 0;
	int channels = 0;
	std::vector<int16> raw_sample_data;

	std::string sampleName;

#endif // VORBIS

#if defined(MPT_WITH_VORBISFILE)

	bool unsupportedSample = false;

	ov_callbacks callbacks = {
		&VorbisfileFilereaderRead,
		&VorbisfileFilereaderSeek,
		NULL,
		&VorbisfileFilereaderTell
	};
	OggVorbis_File vf;
	MemsetZero(vf);
	if(ov_open_callbacks(mpt::void_ptr<FileReader>(&file), &vf, NULL, 0, callbacks) == 0)
	{
		if(ov_streams(&vf) == 1)
		{ // we do not support chained vorbis samples
			vorbis_info *vi = ov_info(&vf, -1);
			if(vi && vi->rate > 0 && vi->channels > 0)
			{
				sampleName = mpt::ToCharset(GetCharsetInternal(), GetSampleNameFromTags(GetVorbisFileTags(vf)));
				rate = vi->rate;
				channels = vi->channels;
				std::size_t offset = 0;
				int current_section = 0;
				long decodedSamples = 0;
				bool eof = false;

				if(auto length = ov_pcm_total(&vf, 0); length != OV_EINVAL)
					raw_sample_data.reserve(std::min(MAX_SAMPLE_LENGTH, mpt::saturate_cast<SmpLength>(length)) * std::clamp(channels, 1, 2));

				while(!eof)
				{
					float **output = nullptr;
					long ret = ov_read_float(&vf, &output, 1024, &current_section);
					if(ret == 0)
					{
						eof = true;
					} else if(ret < 0)
					{
						// stream error, just try to continue
					} else
					{
						decodedSamples = ret;
						if(decodedSamples > 0 && (channels == 1 || channels == 2))
						{
							raw_sample_data.resize(raw_sample_data.size() + (channels * decodedSamples));
							CopyAudio(mpt::audio_span_interleaved(raw_sample_data.data() + (offset * channels), channels, decodedSamples), mpt::audio_span_planar(output, channels, decodedSamples));
							offset += decodedSamples;
							if((raw_sample_data.size() / channels) > MAX_SAMPLE_LENGTH)
							{
								break;
							}
						}
					}
				}
			} else
			{
				unsupportedSample = true;
			}
		} else
		{
			unsupportedSample = true;
		}
		ov_clear(&vf);
	} else
	{
		unsupportedSample = true;
	}

	if(unsupportedSample)
	{
		return false;
	}

#elif defined(MPT_WITH_STBVORBIS)

	// NOTE/TODO: stb_vorbis does not handle inferred negative PCM sample position
	// at stream start. (See
	// <https://www.xiph.org/vorbis/doc/Vorbis_I_spec.html#x1-132000A.2>). This
	// means that, for remuxed and re-aligned/cutted (at stream start) Vorbis
	// files, stb_vorbis will include superfluous samples at the beginning.

	FileReader::PinnedView fileView = file.GetPinnedView();
	const std::byte* data = fileView.data();
	std::size_t dataLeft = fileView.size();

	std::size_t offset = 0;
	int consumed = 0;
	int error = 0;
	stb_vorbis *vorb = stb_vorbis_open_pushdata(mpt::byte_cast<const unsigned char*>(data), mpt::saturate_cast<int>(dataLeft), &consumed, &error, nullptr);
	file.Skip(consumed);
	data += consumed;
	dataLeft -= consumed;
	if(!vorb)
	{
		return false;
	}
	rate = stb_vorbis_get_info(vorb).sample_rate;
	channels = stb_vorbis_get_info(vorb).channels;
	if(rate <= 0 || channels <= 0)
	{
		return false;
	}
	while((error == VORBIS__no_error || (error == VORBIS_need_more_data && dataLeft > 0)))
	{
		int frame_channels = 0;
		int decodedSamples = 0;
		float **output = nullptr;
		consumed = stb_vorbis_decode_frame_pushdata(vorb, mpt::byte_cast<const unsigned char*>(data), mpt::saturate_cast<int>(dataLeft), &frame_channels, &output, &decodedSamples);
		file.Skip(consumed);
		data += consumed;
		dataLeft -= consumed;
		LimitMax(frame_channels, channels);
		if(decodedSamples > 0 && (frame_channels == 1 || frame_channels == 2))
		{
			raw_sample_data.resize(raw_sample_data.size() + (channels * decodedSamples));
			CopyAudio(mpt::audio_span_interleaved(raw_sample_data.data() + (offset * channels), channels, decodedSamples), mpt::audio_span_planar(output, channels, decodedSamples));
			offset += decodedSamples;
			if((raw_sample_data.size() / channels) > MAX_SAMPLE_LENGTH)
			{
				break;
			}
		}
		error = stb_vorbis_get_error(vorb);
	}
	stb_vorbis_close(vorb);

#endif // VORBIS

#if defined(MPT_WITH_VORBISFILE) || defined(MPT_WITH_STBVORBIS)

	if(rate <= 0 || channels <= 0 || raw_sample_data.empty())
	{
		return false;
	}

	DestroySampleThreadsafe(sample);
	ModSample &mptSample = Samples[sample];
	mptSample.Initialize();
	mptSample.nC5Speed = static_cast<uint32>(rate);
	mptSample.nLength = std::min(MAX_SAMPLE_LENGTH, mpt::saturate_cast<SmpLength>(raw_sample_data.size() / channels));

	mptSample.uFlags.set(CHN_16BIT);
	mptSample.uFlags.set(CHN_STEREO, channels == 2);

	if(!mptSample.AllocateSample())
	{
		return false;
	}

	if(raw_sample_data.size() / channels > MAX_SAMPLE_LENGTH)
	{
		AddToLog(LogWarning, U_("Sample has been truncated!"));
	}

	std::copy(raw_sample_data.begin(), raw_sample_data.begin() + mptSample.nLength * channels, mptSample.sample16());

	mptSample.Convert(MOD_TYPE_IT, GetType());
	mptSample.PrecomputeLoops(*this, false);
	m_szNames[sample] = sampleName;

	return true;

#else // !VORBIS

	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);

	return false;

#endif // VORBIS

}


OPENMPT_NAMESPACE_END
