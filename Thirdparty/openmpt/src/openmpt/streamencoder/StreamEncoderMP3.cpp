/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "openmpt/streamencoder/StreamEncoderMP3.hpp"

#include "mpt/base/alloc.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/saturate_round.hpp"
#include "mpt/base/span.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/path/native_path.hpp"
#include "mpt/random/any_engine.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "openmpt/base/Endian.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifdef MPT_WITH_LAME
#if defined(MPT_BUILD_MSVC)
#include <lame.h>
#else
#include <lame/lame.h>
#endif
#endif  // MPT_WITH_LAME



OPENMPT_NAMESPACE_BEGIN



///////////////////////////////////////////////////////////////////////////////////////////////////
// ID3v2.4 Tags

struct ID3v2Header
{
	uint8 signature[3];
	uint8 version[2];
	uint8be flags;
	uint32be size;
};

MPT_BINARY_STRUCT(ID3v2Header, 10)

struct ID3v2Frame
{
	char frameid[4];
	uint32be size;
	uint16be flags;
};

MPT_BINARY_STRUCT(ID3v2Frame, 10)


// charset... choose text ending accordingly.
// $00 = ISO-8859-1. Terminated with $00.
// $01 = UTF-16 with BOM. Terminated with $00 00.
// $02 = UTF-16BE without BOM. Terminated with $00 00.
// $03 = UTF-8. Terminated with $00.
#define ID3v2_CHARSET    '\3'
#define ID3v2_TEXTENDING '\0'

struct ReplayGain
{
	enum GainTag
	{
		TagSkip,
		TagReserve,
		TagWrite
	};
	GainTag Tag;
	float TrackPeak;
	bool TrackPeakValid;
	float TrackGaindB;
	bool TrackGaindBValid;
	ReplayGain()
		: Tag(TagSkip)
		, TrackPeak(0.0f)
		, TrackPeakValid(false)
		, TrackGaindB(0.0f)
		, TrackGaindBValid(false)
	{
		return;
	}
};

class ID3V2Tagger
{
private:
	Encoder::StreamSettings settings;
public:
	// Write Tags
	void WriteID3v2Tags(std::ostream &s, const FileTags &tags, ReplayGain replayGain = ReplayGain());

	ID3V2Tagger(const Encoder::StreamSettings &settings_);

private:
	// Convert Integer to Synchsafe Integer (see ID3v2.4 specs)
	uint32 intToSynchsafe(uint32 in);
	// Return maximum value that fits into a syncsafe int
	uint32 GetMaxSynchsafeInt() const;
	// Write a frame
	void WriteID3v2Frame(const char cFrameID[4], std::string sFramecontent, std::ostream &s);
	// Return an upper bound for the size of all replay gain frames
	uint32 GetMaxReplayGainFramesSizes();
	uint32 GetMaxReplayGainTxxxTrackGainFrameSize();
	uint32 GetMaxReplayGainTxxxTrackPeakFrameSize();
	// Write out all ReplayGain frames
	void WriteID3v2ReplayGainFrames(ReplayGain replaygain, std::ostream &s);
	// Size of our tag
	uint32 totalID3v2Size;
};

///////////////////////////////////////////////////
// CFileTagging - helper class for writing tags

ID3V2Tagger::ID3V2Tagger(const Encoder::StreamSettings &settings_)
	: settings(settings_)
	, totalID3v2Size(0)
{
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ID3v2.4 Tags

// Convert Integer to Synchsafe Integer (see ID3v2.4 specs)
// Basically, it's a BigEndian integer, but the MSB of all bytes is 0.
// Thus, a 32-bit integer turns into a 28-bit integer.
uint32 ID3V2Tagger::intToSynchsafe(uint32 in)
{
	uint32 out = 0, steps = 0;
	do
	{
		out |= (in & 0x7F) << steps;
		steps += 8;
	} while(in >>= 7);
	return out;
}

// Return maximum value that fits into a syncsafe int
uint32 ID3V2Tagger::GetMaxSynchsafeInt() const
{
	return 0x0fffffffu;
}

// Write Tags
void ID3V2Tagger::WriteID3v2Tags(std::ostream &s, const FileTags &tags, ReplayGain replayGain)
{
	if(!s) return;

	ID3v2Header tHeader;
	mpt::IO::Offset fOffset = mpt::IO::TellWrite(s);
	uint32 paddingSize = 0;

	totalID3v2Size = 0;

	// Correct header will be written later (tag size missing)
	memcpy(tHeader.signature, "ID3", 3);
	tHeader.version[0] = 0x04;  // Version 2.4.0
	tHeader.version[1] = 0x00;  // Ditto
	tHeader.flags = 0;          // No flags
	tHeader.size = 0;           // will be filled later
	mpt::IO::Write(s, tHeader);
	totalID3v2Size += sizeof(tHeader);

	WriteID3v2Frame("TIT2", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.title), s);
	WriteID3v2Frame("TPE1", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.artist), s);
	WriteID3v2Frame("TCOM", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.artist), s);
	WriteID3v2Frame("TALB", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.album), s);
	WriteID3v2Frame("TCON", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.genre), s);
	//WriteID3v2Frame("TYER", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.year), s);		// Deprecated
	WriteID3v2Frame("TDRC", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.year), s);
	WriteID3v2Frame("TBPM", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.bpm), s);
	WriteID3v2Frame("WXXX", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.url), s);
	WriteID3v2Frame("TENC", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.encoder), s);
	WriteID3v2Frame("COMM", mpt::transcode<std::string>(mpt::common_encoding::utf8, tags.comments), s);
	if(replayGain.Tag == ReplayGain::TagReserve)
	{
		paddingSize += GetMaxReplayGainFramesSizes();
	} else if(replayGain.Tag == ReplayGain::TagWrite)
	{
		mpt::IO::Offset replayGainBeg = mpt::IO::TellWrite(s);
		WriteID3v2ReplayGainFrames(replayGain, s);
		mpt::IO::Offset replayGainEnd = mpt::IO::TellWrite(s);
		paddingSize += GetMaxReplayGainFramesSizes() - static_cast<uint32>(replayGainEnd - replayGainBeg);
	}

	// Write Padding
	uint32 totalID3v2SizeWithoutPadding = totalID3v2Size;
	paddingSize += settings.MP3ID3v2MinPadding;
	totalID3v2Size += paddingSize;
	if(settings.MP3ID3v2PaddingAlignHint > 0)
	{
		totalID3v2Size = mpt::align_up<uint32>(totalID3v2Size, settings.MP3ID3v2PaddingAlignHint);
		paddingSize = totalID3v2Size - totalID3v2SizeWithoutPadding;
	}
	for(std::size_t i = 0; i < paddingSize; i++)
	{
		uint8 c = 0;
		mpt::IO::Write(s, c);
	}

	// Write correct header (update tag size)
	tHeader.size = intToSynchsafe(totalID3v2Size - sizeof(tHeader));
	mpt::IO::SeekAbsolute(s, fOffset);
	mpt::IO::Write(s, tHeader);
	mpt::IO::SeekRelative(s, totalID3v2Size - sizeof(tHeader));
}

uint32 ID3V2Tagger::GetMaxReplayGainTxxxTrackGainFrameSize()
{
	return mpt::saturate_cast<uint32>(sizeof(ID3v2Frame) + 1 + std::strlen("REPLAYGAIN_TRACK_GAIN") + 1 + std::strlen("-123.45 dB") + 1);  // should be enough
}

uint32 ID3V2Tagger::GetMaxReplayGainTxxxTrackPeakFrameSize()
{
	return mpt::saturate_cast<uint32>(sizeof(ID3v2Frame) + 1 + std::strlen("REPLAYGAIN_TRACK_PEAK") + 1 + std::strlen("2147483648.123456") + 1);  // unrealistic worst case
}

uint32 ID3V2Tagger::GetMaxReplayGainFramesSizes()
{
	uint32 size = 0;
	if(settings.MP3ID3v2WriteReplayGainTXXX)
	{
		size += GetMaxReplayGainTxxxTrackGainFrameSize();
		size += GetMaxReplayGainTxxxTrackPeakFrameSize();
	}
	return size;
}

void ID3V2Tagger::WriteID3v2ReplayGainFrames(ReplayGain replayGain, std::ostream &s)
{

	if(settings.MP3ID3v2WriteReplayGainTXXX && replayGain.TrackGaindBValid)
	{

		std::string content;

		content += std::string(1, 0x00);  // ISO-8859-1
		content += std::string("REPLAYGAIN_TRACK_GAIN");
		content += std::string(1, '\0');

		int32 gainTimes100 = mpt::saturate_round<int32>(replayGain.TrackGaindB * 100.0f);
		if(gainTimes100 < 0)
		{
			content += "-";
			gainTimes100 = std::abs(gainTimes100);
		}
		content += mpt::format<std::string>::dec(gainTimes100 / 100);
		content += ".";
		content += mpt::format<std::string>::dec0<2>(gainTimes100 % 100);
		content += " ";
		content += "dB";

		content += std::string(1, '\0');

		if(sizeof(ID3v2Frame) + content.size() <= GetMaxReplayGainTxxxTrackGainFrameSize())
		{
			ID3v2Frame frame;
			std::memset(&frame, 0, sizeof(ID3v2Frame));
			std::memcpy(&frame.frameid, "TXXX", 4);
			frame.size = intToSynchsafe(static_cast<uint32>(content.size()));
			frame.flags = 0x4000;  // discard if audio data changed
			mpt::IO::Write(s, frame);
			mpt::IO::Write(s, mpt::as_span(content));
		}
	}


	if(settings.MP3ID3v2WriteReplayGainTXXX && replayGain.TrackPeakValid)
	{

		std::string content;

		content += std::string(1, 0x00);  // ISO-8859-1
		content += std::string("REPLAYGAIN_TRACK_PEAK");
		content += std::string(1, '\0');

		int32 peakTimes1000000 = mpt::saturate_round<int32>(std::fabs(replayGain.TrackPeak) * 1000000.0f);
		std::string number;
		number += mpt::format<std::string>::dec(peakTimes1000000 / 1000000);
		number += ".";
		number += mpt::format<std::string>::dec0<6>(peakTimes1000000 % 1000000);
		content += number;

		content += std::string(1, '\0');

		if(sizeof(ID3v2Frame) + content.size() <= GetMaxReplayGainTxxxTrackPeakFrameSize())
		{
			ID3v2Frame frame;
			std::memset(&frame, 0, sizeof(ID3v2Frame));
			std::memcpy(&frame.frameid, "TXXX", 4);
			frame.size = intToSynchsafe(static_cast<uint32>(content.size()));
			frame.flags = 0x4000;  // discard if audio data changed
			mpt::IO::Write(s, frame);
			mpt::IO::Write(s, mpt::as_span(content));
		}
	}
}

// Write a ID3v2 frame
void ID3V2Tagger::WriteID3v2Frame(const char cFrameID[4], std::string sFramecontent, std::ostream &s)
{
	if(!cFrameID[0] || sFramecontent.empty() || !s) return;

	if(!std::memcmp(cFrameID, "COMM", 4))
	{
		// English language for comments - no description following (hence the text ending nullchar(s))
		// For language IDs, see https://en.wikipedia.org/wiki/ISO-639-2
		sFramecontent = "eng" + (ID3v2_TEXTENDING + sFramecontent);
	}
	if(!std::memcmp(cFrameID, "WXXX", 4))
	{
		// User-defined URL field (we have no description for the URL, so we leave it out)
		sFramecontent = ID3v2_TEXTENDING + sFramecontent;
	}
	sFramecontent = ID3v2_CHARSET + sFramecontent;
	sFramecontent += ID3v2_TEXTENDING;

	if(sFramecontent.size() <= GetMaxSynchsafeInt())
	{
		ID3v2Frame tFrame;
		std::memset(&tFrame, 0, sizeof(ID3v2Frame));
		std::memcpy(&tFrame.frameid, cFrameID, 4);                                // ID
		tFrame.size = intToSynchsafe(static_cast<uint32>(sFramecontent.size()));  // Text size
		tFrame.flags = 0x0000;                                                    // No flags
		mpt::IO::Write(s, tFrame);
		mpt::IO::Write(s, mpt::as_span(sFramecontent));

		totalID3v2Size += static_cast<uint32>((sizeof(tFrame) + sFramecontent.size()));
	}
}




#ifdef MPT_WITH_LAME


using lame_t = lame_global_flags *;


static void GenreEnumCallback(int num, const char *name, void *cookie)
{
	MPT_UNUSED(num);
	Encoder::Traits &traits = *mpt::void_ptr<Encoder::Traits>(cookie);
	if(name)
	{
		traits.genres.push_back(mpt::transcode<mpt::ustring>(mpt::common_encoding::iso8859_1, name));
	}
}


static Encoder::Traits BuildTraits(bool compatible)
{
	Encoder::Traits traits;
	traits.fileExtension = MPT_NATIVE_PATH("mp3");
	traits.fileShortDescription = (compatible ? MPT_USTRING("Compatible MP3") : MPT_USTRING("MP3"));
	traits.encoderSettingsName = (compatible ? MPT_USTRING("MP3LameCompatible") : MPT_USTRING("MP3Lame"));
	traits.fileDescription = (compatible ? MPT_USTRING("MPEG-1 Layer 3") : MPT_USTRING("MPEG-1/2 Layer 3"));
	traits.canTags = true;
	traits.genres.clear();
	id3tag_genre_list(&GenreEnumCallback, &traits);
	traits.modesWithFixedGenres = (compatible ? Encoder::ModeCBR : Encoder::ModeInvalid);
	traits.maxChannels = 2;
	traits.samplerates = (compatible ? mpt::make_vector(mpeg1layer3_samplerates) : mpt::make_vector(layer3_samplerates));
	traits.modes = (compatible ? Encoder::ModeCBR : (Encoder::ModeABR | Encoder::ModeQuality));
	traits.bitrates = (compatible ? mpt::make_vector(mpeg1layer3_bitrates) : mpt::make_vector(layer3_bitrates));
	traits.defaultSamplerate = 44100;
	traits.defaultChannels = 2;
	traits.defaultMode = (compatible ? Encoder::ModeCBR : Encoder::ModeQuality);
	traits.defaultBitrate = 256;
	traits.defaultQuality = 0.8f;
	return traits;
}


class MP3LameStreamWriter : public StreamWriterBase
{
private:
	bool compatible;
	Encoder::Settings settings;
	Encoder::Mode Mode;
	bool gfp_inited;
	lame_t gfp;
	enum ID3Type
	{
		ID3None,
		ID3v1,
		ID3v2Lame,
		ID3v2OpenMPT,
	};
	ID3Type id3type;
	mpt::IO::Offset id3v2Size;
	FileTags Tags;
public:
	MP3LameStreamWriter(std::ostream &stream, bool compatible, const Encoder::Settings &settings_, const FileTags &tags)
		: StreamWriterBase(stream)
		, compatible(compatible)
		, settings(settings_)
	{
		Mode = Encoder::ModeInvalid;
		gfp_inited = false;
		gfp = lame_t();
		id3type = ID3v2Lame;
		id3v2Size = 0;

		if(!gfp)
		{
			gfp = lame_init();
		}

		uint32 samplerate = settings.Samplerate;
		uint16 channels = settings.Channels;
		if(settings.Tags)
		{
			if(compatible)
			{
				id3type = ID3v1;
			} else if(settings.Details.MP3LameID3v2UseLame)
			{
				id3type = ID3v2Lame;
			} else
			{
				id3type = ID3v2OpenMPT;
			}
		} else
		{
			id3type = ID3None;
		}
		id3v2Size = 0;

		lame_set_in_samplerate(gfp, samplerate);
		lame_set_num_channels(gfp, channels);

		int lameQuality = settings.Details.MP3LameQuality;
		lame_set_quality(gfp, lameQuality);

		if(settings.Mode == Encoder::ModeCBR)
		{

			if(compatible)
			{
				if(settings.Bitrate >= 32)
				{
					// For maximum compatibility,
					// force samplerate to a samplerate supported by MPEG1 streams.
					if(samplerate <= 32000)
					{
						samplerate = 32000;
					} else if(samplerate >= 48000)
					{
						samplerate = 48000;
					} else
					{
						samplerate = 44100;
					}
					lame_set_out_samplerate(gfp, samplerate);
				} else
				{
					// A very low bitrate was chosen,
					// force samplerate to lowest possible for MPEG2.
					// Disable unofficial MPEG2.5 however.
					lame_set_out_samplerate(gfp, 16000);
				}
			}

			lame_set_brate(gfp, settings.Bitrate);
			lame_set_VBR(gfp, vbr_off);

			if(compatible)
			{
				lame_set_bWriteVbrTag(gfp, 0);
				lame_set_strict_ISO(gfp, 1);
				lame_set_disable_reservoir(gfp, 1);
			} else
			{
				lame_set_bWriteVbrTag(gfp, 1);
			}

		} else if(settings.Mode == Encoder::ModeABR)
		{

			lame_set_brate(gfp, settings.Bitrate);
			lame_set_VBR(gfp, vbr_abr);

			lame_set_bWriteVbrTag(gfp, 1);

		} else
		{

			float lame_quality = 10.0f - (settings.Quality * 10.0f);
			lame_quality = std::clamp(lame_quality, 0.0f, 9.999f);
			lame_set_VBR_quality(gfp, lame_quality);
			lame_set_VBR(gfp, vbr_default);

			lame_set_bWriteVbrTag(gfp, 1);
		}

		lame_set_decode_on_the_fly(gfp, settings.Details.MP3LameCalculatePeakSample ? 1 : 0);  // see LAME docs for why
		lame_set_findReplayGain(gfp, settings.Details.MP3LameCalculateReplayGain ? 1 : 0);

		switch(id3type)
		{
			case ID3None:
				lame_set_write_id3tag_automatic(gfp, 0);
				break;
			case ID3v1:
				id3tag_init(gfp);
				id3tag_v1_only(gfp);
				break;
			case ID3v2Lame:
				id3tag_init(gfp);
				id3tag_add_v2(gfp);
				id3tag_v2_only(gfp);
				id3tag_set_pad(gfp, settings.Details.MP3ID3v2MinPadding);
				break;
			case ID3v2OpenMPT:
				lame_set_write_id3tag_automatic(gfp, 0);
				break;
		}

		Mode = settings.Mode;

		if(settings.Tags)
		{
			if(id3type == ID3v2Lame || id3type == ID3v1)
			{
				// Lame API expects Latin1, which is sad, but we cannot change that.
				if(!tags.title.empty()) id3tag_set_title(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.title).c_str());
				if(!tags.artist.empty()) id3tag_set_artist(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.artist).c_str());
				if(!tags.album.empty()) id3tag_set_album(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.album).c_str());
				if(!tags.year.empty()) id3tag_set_year(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.year).c_str());
				if(!tags.comments.empty()) id3tag_set_comment(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.comments).c_str());
				if(!tags.trackno.empty()) id3tag_set_track(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.trackno).c_str());
				if(!tags.genre.empty()) id3tag_set_genre(gfp, mpt::transcode<std::string>(mpt::common_encoding::iso8859_1, tags.genre).c_str());
			} else if(id3type == ID3v2OpenMPT)
			{
				Tags = tags;
				mpt::IO::Offset id3beg = mpt::IO::TellWrite(f);
				ID3V2Tagger tagger(settings.Details);
				ReplayGain replayGain;
				if(settings.Details.MP3LameCalculatePeakSample || settings.Details.MP3LameCalculateReplayGain)
				{
					replayGain.Tag = ReplayGain::TagReserve;
				}
				tagger.WriteID3v2Tags(f, tags, replayGain);
				mpt::IO::Offset id3end = mpt::IO::TellWrite(f);
				id3v2Size = id3end - id3beg;
			}
		}
	}
	void WriteInterleaved(std::size_t count, const float *interleaved) override
	{
		if(!gfp_inited)
		{
			lame_init_params(gfp);
			gfp_inited = true;
		}
		const int count_max = 0xffff;
		while(count > 0)
		{
			int count_chunk = std::clamp(mpt::saturate_cast<int>(count), int(0), count_max);
			buf.resize(count_chunk + (count_chunk + 3) / 4 + 7200);
			int result = 0;
			if(lame_get_num_channels(gfp) == 1)
			{
				// lame always assumes stereo input with interleaved interface, so use non-interleaved for mono
				result = lame_encode_buffer_ieee_float(gfp, interleaved, nullptr, count_chunk, mpt::byte_cast<unsigned char *>(buf.data()), mpt::saturate_cast<int>(buf.size()));
			} else
			{
				result = lame_encode_buffer_interleaved_ieee_float(gfp, interleaved, count_chunk, mpt::byte_cast<unsigned char *>(buf.data()), mpt::saturate_cast<int>(buf.size()));
			}
			buf.resize((result >= 0) ? result : 0);
			if(result == -2)
			{
				mpt::throw_out_of_memory();
			}
			WriteBuffer();
			count -= static_cast<std::size_t>(count_chunk);
		}
	}
	void WriteFinalize() override
	{
		if(!gfp_inited)
		{
			lame_init_params(gfp);
			gfp_inited = true;
		}
		buf.resize(7200);
		buf.resize(lame_encode_flush(gfp, mpt::byte_cast<unsigned char *>(buf.data()), mpt::saturate_cast<int>(buf.size())));
		WriteBuffer();
		ReplayGain replayGain;
		if(settings.Details.MP3LameCalculatePeakSample)
		{
			replayGain.TrackPeak = std::fabs(lame_get_PeakSample(gfp)) / 32768.0f;
			replayGain.TrackPeakValid = true;
		}
		if(settings.Details.MP3LameCalculateReplayGain)
		{
			replayGain.TrackGaindB = lame_get_RadioGain(gfp) / 10.0f;
			replayGain.TrackGaindBValid = true;
		}
		if(id3type == ID3v2OpenMPT && (settings.Details.MP3LameCalculatePeakSample || settings.Details.MP3LameCalculateReplayGain))
		{  // update ID3v2 tag with replay gain information
			replayGain.Tag = ReplayGain::TagWrite;
			mpt::IO::Offset endPos = mpt::IO::TellWrite(f);
			mpt::IO::SeekAbsolute(f, fStart);
			std::string tagdata(static_cast<std::size_t>(id3v2Size), '\0');
			mpt::IO::Write(f, mpt::as_span(tagdata.data(), mpt::saturate_cast<std::size_t>(id3v2Size)));  // clear out the old tag
			mpt::IO::SeekAbsolute(f, fStart);
			ID3V2Tagger tagger(settings.Details);
			tagger.WriteID3v2Tags(f, Tags, replayGain);
			mpt::IO::SeekAbsolute(f, endPos);
		}
		if(id3type == ID3v2Lame)
		{
			id3v2Size = lame_get_id3v2_tag(gfp, nullptr, 0);
		} else if(id3type == ID3v2OpenMPT)
		{
			// id3v2Size already set
		}
		if(!compatible)
		{
			mpt::IO::Offset endPos = mpt::IO::TellWrite(f);
			mpt::IO::SeekAbsolute(f, fStart + id3v2Size);
			buf.resize(lame_get_lametag_frame(gfp, nullptr, 0));
			buf.resize(lame_get_lametag_frame(gfp, mpt::byte_cast<unsigned char *>(buf.data()), buf.size()));
			WriteBuffer();
			mpt::IO::SeekAbsolute(f, endPos);
		}
	}
	virtual ~MP3LameStreamWriter()
	{
		if(!gfp)
		{
			return;
		}
		lame_close(gfp);
		gfp = lame_t();
		gfp_inited = false;
	}
};

#endif  // MPT_WITH_LAME



MP3Encoder::MP3Encoder(MP3EncoderType type)
	: m_Type(type)
{
#ifdef MPT_WITH_LAME
	if(type == MP3EncoderLame)
	{
		m_Type = MP3EncoderLame;
		SetTraits(BuildTraits(false));
		return;
	}
	if(type == MP3EncoderLameCompatible)
	{
		m_Type = MP3EncoderLameCompatible;
		SetTraits(BuildTraits(true));
		return;
	}
#endif  // MPT_WITH_LAME
}


bool MP3Encoder::IsAvailable() const
{
	return false
#ifdef MPT_WITH_LAME
		|| (m_Type == MP3EncoderLame)
		|| (m_Type == MP3EncoderLameCompatible)
#endif  // MPT_WITH_LAME
		;
}


std::unique_ptr<IAudioStreamEncoder> MP3Encoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const
{
	std::unique_ptr<IAudioStreamEncoder> result = nullptr;
	if(false)
	{
		// nothing
#ifdef MPT_WITH_LAME
	} else if(m_Type == MP3EncoderLame || m_Type == MP3EncoderLameCompatible)
	{
		result = std::make_unique<MP3LameStreamWriter>(file, (m_Type == MP3EncoderLameCompatible), settings, tags);
		MPT_UNUSED(prng);
#else   // !MPT_WITH_LAME
		MPT_UNUSED(file);
		MPT_UNUSED(settings);
		MPT_UNUSED(tags);
		MPT_UNUSED(prng);
#endif  // MPT_WITH_LAME
	}
	return result;
}


mpt::ustring MP3Encoder::DescribeQuality(float quality) const
{
#ifdef MPT_WITH_LAME
	if(m_Type == MP3EncoderLame)
	{
		static constexpr int q_table[11] = {240, 220, 190, 170, 160, 130, 120, 100, 80, 70, 50};  // http://wiki.hydrogenaud.io/index.php?title=LAME
		int q = mpt::saturate_round<int>((1.0f - quality) * 10.0f);
		if(q < 0) q = 0;
		if(q >= 10)
		{
			return MPT_UFORMAT_MESSAGE("VBR -V{} (~{} kbit)")(MPT_USTRING("9.999"), q_table[q]);
		} else
		{
			return MPT_UFORMAT_MESSAGE("VBR -V{} (~{} kbit)")(q, q_table[q]);
		}
	}
#endif  // MPT_WITH_LAME
	return EncoderFactoryBase::DescribeQuality(quality);
}

mpt::ustring MP3Encoder::DescribeBitrateABR(int bitrate) const
{
	return EncoderFactoryBase::DescribeBitrateABR(bitrate);
}



OPENMPT_NAMESPACE_END
