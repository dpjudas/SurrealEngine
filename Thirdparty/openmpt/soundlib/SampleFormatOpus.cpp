/*
 * SampleFormatOpus.cpp
 * --------------------
 * Purpose: Opus sample import.
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
#if defined(MPT_WITH_OPUSFILE)
#include <opusfile.h>
#endif // MPT_WITH_OPUSFILE


OPENMPT_NAMESPACE_BEGIN


////////////////////////////////////////////////////////////////////////////////
// Opus

#if defined(MPT_WITH_OPUSFILE)

static int OpusfileFilereaderRead(void *stream, unsigned char *ptr, int nbytes)
{
	FileReader &file = *mpt::void_ptr<FileReader>(stream);
	return mpt::saturate_cast<int>(file.ReadRaw(mpt::span(mpt::byte_cast<std::byte*>(ptr), nbytes)).size());
}

static int OpusfileFilereaderSeek(void *stream, opus_int64 offset, int whence)
{
	FileReader &file = *mpt::void_ptr<FileReader>(stream);
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

static opus_int64 OpusfileFilereaderTell(void *stream)
{
	FileReader &file = *mpt::void_ptr<FileReader>(stream);
	MPT_MAYBE_CONSTANT_IF(!mpt::in_range<opus_int64>(file.GetPosition()))
	{
		return -1;
	}
	return static_cast<opus_int64>(file.GetPosition());
}

static mpt::ustring UStringFromOpus(const char *str)
{
	return str ? mpt::ToUnicode(mpt::Charset::UTF8, str) : mpt::ustring();
}

static FileTags GetOpusFileTags(OggOpusFile *of)
{
	FileTags tags;
	const OpusTags *ot = op_tags(of, -1);
	if(!ot)
	{
		return tags;
	}
	tags.encoder = UStringFromOpus(opus_tags_query(ot, "ENCODER", 0));
	tags.title = UStringFromOpus(opus_tags_query(ot, "TITLE", 0));
	tags.comments = UStringFromOpus(opus_tags_query(ot, "DESCRIPTION", 0));
	tags.bpm = UStringFromOpus(opus_tags_query(ot, "BPM", 0)); // non-standard
	tags.artist = UStringFromOpus(opus_tags_query(ot, "ARTIST", 0));
	tags.album = UStringFromOpus(opus_tags_query(ot, "ALBUM", 0));
	tags.trackno = UStringFromOpus(opus_tags_query(ot, "TRACKNUMBER", 0));
	tags.year = UStringFromOpus(opus_tags_query(ot, "DATE", 0));
	tags.url = UStringFromOpus(opus_tags_query(ot, "CONTACT", 0));
	tags.genre = UStringFromOpus(opus_tags_query(ot, "GENRE", 0));
	return tags;
}

#endif // MPT_WITH_OPUSFILE

bool CSoundFile::ReadOpusSample(SAMPLEINDEX sample, FileReader &file)
{
	file.Rewind();

#if defined(MPT_WITH_OPUSFILE)

	int rate = 0;
	int channels = 0;
	std::vector<int16> raw_sample_data;

	{
		FileReader::PinnedView initial = file.GetPinnedView(65536); // 512 is recommended by libopusfile
		if(op_test(NULL, mpt::byte_cast<const unsigned char*>(initial.data()), initial.size()) != 0)
		{
			return false;
		}
	}

	OpusFileCallbacks callbacks = {};
	callbacks.read = &OpusfileFilereaderRead;
	callbacks.seek = &OpusfileFilereaderSeek;
	callbacks.tell = &OpusfileFilereaderTell;
	callbacks.close = NULL;
	OggOpusFile *of = op_open_callbacks(mpt::void_ptr<FileReader>(&file), &callbacks, NULL, 0, NULL);
	if(!of)
	{
		return false;
	}

	rate = 48000;
	channels = op_channel_count(of, -1);
	if(rate <= 0 || channels <= 0)
	{
		op_free(of);
		of = NULL;
		return false;
	}
	if(channels > 2 || op_link_count(of) != 1)
	{
		// We downmix multichannel to stereo as recommended by Opus specification in
		// case we are not able to handle > 2 channels.
		// We also decode chained files as stereo even if they start with a mono
		// stream, which simplifies handling of link boundaries for us.
		channels = 2;
	}

	std::string sampleName = mpt::ToCharset(GetCharsetInternal(), GetSampleNameFromTags(GetOpusFileTags(of)));

	if(auto length = op_pcm_total(of, 0); length != OP_EINVAL)
		raw_sample_data.reserve(std::min(MAX_SAMPLE_LENGTH, mpt::saturate_cast<SmpLength>(length)) * channels);

	std::vector<int16> decodeBuf(120 * 48000 / 1000); // 120ms (max Opus packet), 48kHz
	bool eof = false;
	while(!eof)
	{
		int framesRead = 0;
		if(channels == 2)
		{
			framesRead = op_read_stereo(of, &(decodeBuf[0]), static_cast<int>(decodeBuf.size()));
		} else if(channels == 1)
		{
			framesRead = op_read(of, &(decodeBuf[0]), static_cast<int>(decodeBuf.size()), NULL);
		}
		if(framesRead > 0)
		{
			mpt::append(raw_sample_data, decodeBuf.begin(), decodeBuf.begin() + (framesRead * channels));
		} else if(framesRead == 0)
		{
			eof = true;
		} else if(framesRead == OP_HOLE)
		{
			// continue
		} else
		{
			// other errors are fatal, stop decoding
			eof = true;
		}
		if((raw_sample_data.size() / channels) > MAX_SAMPLE_LENGTH)
		{
			break;
		}
	}

	op_free(of);
	of = NULL;

	if(raw_sample_data.empty())
	{
		return false;
	}

	DestroySampleThreadsafe(sample);
	ModSample &mptSample = Samples[sample];
	mptSample.Initialize();
	mptSample.nC5Speed = rate;
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

#else // !MPT_WITH_OPUSFILE

	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);

	return false;

#endif // MPT_WITH_OPUSFILE

}


OPENMPT_NAMESPACE_END
