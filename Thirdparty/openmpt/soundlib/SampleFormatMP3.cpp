/*
 * SampleFormatMP3.cpp
 * -------------------
 * Purpose: MP3 sample import.
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
#include "../common/ComponentManager.h"
#ifdef MPT_ENABLE_MP3_SAMPLES
#include "MPEGFrame.h"
#endif // MPT_ENABLE_MP3_SAMPLES
#if defined(MPT_WITH_MINIMP3)
#include "mpt/base/alloc.hpp"
#endif // MPT_WITH_MINIMP3

#if defined(MPT_WITH_MINIMP3)
#include <minimp3.h>
#endif // MPT_WITH_MINIMP3

// mpg123 must be last because of mpg123 large file support insanity
#if defined(MPT_WITH_MPG123)

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>

#if MPT_OS_OPENBSD
// This is kind-of a hack.
// See <https://sourceforge.net/p/mpg123/bugs/330/>.
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifdef _FILE_OFFSET_BITS
#undef _FILE_OFFSET_BITS
#endif
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif
#endif
#include <mpg123.h>
#define MPT_USE_MPG123_PORTABLE_API 1

#endif


OPENMPT_NAMESPACE_BEGIN


///////////////////////////////////////////////////////////////////////////////////////////////////
// MP3 Samples

#if defined(MPT_WITH_MPG123)

#if (MPG123_API_VERSION < 48) || !MPT_USE_MPG123_PORTABLE_API

using mpg123_off_t = off_t;

using mpg123_size_t = size_t;

// Check for exactly _MSC_VER as libmpg123 does, in order to also catch clang-cl.
#ifdef _MSC_VER
// ssize_t definition in libmpg123.h.in should never have existed at all.
// It got removed from libmpg23.h.in after 1.28.0 and before 1.28.1.
using mpg123_ssize_t = ptrdiff_t;
#else
using mpg123_ssize_t = ssize_t;
#endif

#endif

class ComponentMPG123
	: public ComponentBuiltin
{
	MPT_DECLARE_COMPONENT_MEMBERS(ComponentMPG123, "")

public:

#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
	static int FileReaderRead(void *fp, void *buf, size_t count, size_t *returned)
	{
		FileReader &file = *static_cast<FileReader *>(fp);
		std::size_t readBytes = std::min(count, mpt::saturate_cast<size_t>(file.BytesLeft()));
		file.ReadRaw(mpt::span(mpt::void_cast<std::byte*>(buf), readBytes));
		if(!returned)
		{
			return -1;
		}
		*returned = readBytes;
		return 0;
	}
	static int64_t FileReaderSeek(void *fp, int64_t offset, int whence)
	{
		FileReader &file = *static_cast<FileReader *>(fp);
		if(whence == SEEK_CUR)
		{
			if(!mpt::in_range<FileReader::pos_type>(file.GetPosition() + offset))
			{
				return -1;
			}
			file.Seek(static_cast<FileReader::pos_type>(file.GetPosition() + offset));
		} else if(whence == SEEK_END)
		{
			if(!mpt::in_range<FileReader::pos_type>(file.GetLength() + offset))
			{
				return -1;
			}
			file.Seek(static_cast<FileReader::pos_type>(file.GetLength() + offset));
		} else
		{
			if(!mpt::in_range<FileReader::pos_type>(offset))
			{
				return -1;
			}
			file.Seek(static_cast<FileReader::pos_type>(offset));
		}
		return static_cast<int64_t>(file.GetPosition());
	}
#else
	static mpg123_ssize_t FileReaderRead(void *fp, void *buf, mpg123_size_t count)
	{
		FileReader &file = *static_cast<FileReader *>(fp);
		std::size_t readBytes = std::min(count, static_cast<size_t>(file.BytesLeft()));
		file.ReadRaw(mpt::span(mpt::void_cast<std::byte*>(buf), readBytes));
		return readBytes;
	}
	static mpg123_off_t FileReaderLSeek(void *fp, mpg123_off_t offset, int whence)
	{
		FileReader &file = *static_cast<FileReader *>(fp);
		FileReader::pos_type oldpos = file.GetPosition();
		if(whence == SEEK_CUR) file.Seek(file.GetPosition() + offset);
		else if(whence == SEEK_END) file.Seek(file.GetLength() + offset);
		else file.Seek(offset);
		MPT_MAYBE_CONSTANT_IF(!mpt::in_range<mpg123_off_t>(file.GetPosition()))
		{
			file.Seek(oldpos);
			return static_cast<mpg123_off_t>(-1);
		}
		return static_cast<mpg123_off_t>(file.GetPosition());
	}
#endif

public:
	ComponentMPG123()
		: ComponentBuiltin()
	{
		return;
	}
	bool DoInitialize() override
	{
		if(mpg123_init() != 0)
		{
			return false;
		}
		return true;
	}
	virtual ~ComponentMPG123()
	{
		if(IsAvailable())
		{
			mpg123_exit();
		}
	}
};


static mpt::ustring ReadMPG123String(const mpg123_string &str)
{
	mpt::ustring result;
	if(!str.p)
	{
		return result;
	}
	if(str.fill < 1)
	{
		return result;
	}
	result = mpt::ToUnicode(mpt::Charset::UTF8, std::string(str.p, str.p + str.fill - 1));
	return result;
}

static mpt::ustring ReadMPG123String(const mpg123_string *str)
{
	mpt::ustring result;
	if(!str)
	{
		return result;
	}
	result = ReadMPG123String(*str);
	return result;
}

template <std::size_t N>
static mpt::ustring ReadMPG123String(const char (&str)[N])
{
	return mpt::ToUnicode(mpt::Charset::ISO8859_1, mpt::String::ReadBuf(mpt::String::spacePadded, str));
}

#endif // MPT_WITH_MPG123


#if defined(MPT_WITH_MINIMP3)
#if MPT_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-larger-than=16000"
#endif // MPT_COMPILER_GCC
#if (MPT_CLANG_AT_LEAST(13,0,0) && !defined(MPT_COMPILER_QUIRK_APPLE_CLANG)) || MPT_CLANG_AT_LEAST(13,1,0)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wframe-larger-than"
#endif // MPT_COMPILER_CLANG
static MPT_NOINLINE int mp3dec_decode_frame_no_inline(mp3dec_t *dec, const uint8_t *mp3, int mp3_bytes, mp3d_sample_t *pcm, mp3dec_frame_info_t *info)
{
	return mp3dec_decode_frame(dec, mp3, mp3_bytes, pcm, info);
}
#if (MPT_CLANG_AT_LEAST(13,0,0) && !defined(MPT_COMPILER_QUIRK_APPLE_CLANG)) || MPT_CLANG_AT_LEAST(13,1,0)
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // MPT_COMPILER_GCC
#endif // MPT_WITH_MINIMP3


bool CSoundFile::ReadMP3Sample(SAMPLEINDEX sample, FileReader &file, bool raw, bool mo3Decode)
{
#if defined(MPT_WITH_MPG123) || defined(MPT_WITH_MINIMP3)

	// Check file for validity, or else mpg123 will happily munch many files that start looking vaguely resemble an MPEG stream mid-file.
	file.Rewind();
	while(file.CanRead(4))
	{
		uint8 magic[3];
		file.ReadArray(magic);

		if(!memcmp(magic, "ID3", 3))
		{
			// Skip ID3 tags
			uint8 header[7];
			file.ReadArray(header);

			uint32 size = 0;
			for(int i = 3; i < 7; i++)
			{
				if(header[i] & 0x80)
					return false;
				size = (size << 7) | header[i];
			}
			file.Skip(size);
		} else if(!memcmp(magic, "APE", 3) && file.ReadMagic("TAGEX"))
		{
			// Skip APE tags
			uint32 size = file.ReadUint32LE();
			file.Skip(16 + size);
		} else if(!memcmp(magic, "\x00\x00\x00", 3) || !memcmp(magic, "\xFF\x00\x00", 3))
		{
			// Some MP3 files are padded with zeroes...
		} else if(magic[0] == 0)
		{
			// This might be some padding, followed by an MPEG header, so try again.
			file.SkipBack(2);
		} else if(MPEGFrame::IsMPEGHeader(magic))
		{
			// This is what we want!
			break;
		} else
		{
			// This, on the other hand, isn't.
			return false;
		}
	}

#endif // MPT_WITH_MPG123 || MPT_WITH_MINIMP3

#if defined(MPT_WITH_MPG123)

	ComponentHandle<ComponentMPG123> mpg123;
	if(!IsComponentAvailable(mpg123))
	{
		return false;
	}

	struct MPG123Handle
	{
		mpg123_handle *mh;
		MPG123Handle() : mh(mpg123_new(0, nullptr)) { }
		~MPG123Handle() { mpg123_delete(mh); }
		operator mpg123_handle *() { return mh; }
	};

	bool hasLameXingVbriHeader = false;

	if(!raw)
	{

#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
		int64_t length_raw = 0;
		int64_t length_hdr = 0;
#else
		mpg123_off_t length_raw = 0;
		mpg123_off_t length_hdr = 0;
#endif

		// libmpg123 provides no way to determine whether it parsed ID3V2 or VBR tags.
		// Thus, we use a pre-scan with those disabled and compare the resulting length.
		// We ignore ID3V2 stream length here, althrough we parse the ID3V2 header.
		// libmpg123 only accounts for the VBR info frame if gapless &&!ignore_infoframe,
		// thus we switch both of those for comparison.
		{
			MPG123Handle mh;
			if(!mh)
			{
				return false;
			}
			file.Rewind();
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_AUTO_RESAMPLE, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_REMOVE_FLAGS, MPG123_GAPLESS, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_IGNORE_INFOFRAME, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_REMOVE_FLAGS, MPG123_SKIP_ID3V2, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_INDEX_SIZE, -1000, 0.0)) // auto-grow
			{
				return false;
			}
#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
			if(mpg123_reader64(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderSeek, 0))
			{
				return false;
			}
#else
			if(mpg123_replace_reader_handle(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderLSeek, 0))
			{
				return false;
			}
#endif
#if (MPG123_API_VERSION >= 49) && MPT_USE_MPG123_PORTABLE_API
			if(mpg123_open_handle64(mh, &file))
			{
				return false;
			}
#else
			if(mpg123_open_handle(mh, &file))
			{
				return false;
			}
#endif
			if(mpg123_scan(mh))
			{
				return false;
			}
			long rate = 0;
			int channels = 0;
			int encoding = 0;
			if(mpg123_getformat(mh, &rate, &channels, &encoding))
			{
				return false;
			}
			if((channels != 1) && (channels != 2))
			{
				return false;
			}
			mpg123_frameinfo frameinfo;
			MemsetZero(frameinfo);
			if(mpg123_info(mh, &frameinfo))
			{
				return false;
			}
			if(frameinfo.layer < 1 || frameinfo.layer > 3)
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_FORCE_RATE, rate, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, (channels > 1) ? MPG123_FORCE_STEREO : MPG123_FORCE_MONO, 0.0))
			{
				return false;
			}
#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
			length_raw = mpg123_length64(mh);
#else
			length_raw = mpg123_length(mh);
#endif
		}

		{
			MPG123Handle mh;
			if(!mh)
			{
				return false;
			}
			file.Rewind();
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_AUTO_RESAMPLE, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_GAPLESS, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_REMOVE_FLAGS, MPG123_IGNORE_INFOFRAME, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_REMOVE_FLAGS, MPG123_SKIP_ID3V2, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_INDEX_SIZE, -1000, 0.0)) // auto-grow
			{
				return false;
			}
#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
			if(mpg123_reader64(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderSeek, 0))
			{
				return false;
			}
#else
			if(mpg123_replace_reader_handle(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderLSeek, 0))
			{
				return false;
			}
#endif
#if (MPG123_API_VERSION >= 49) && MPT_USE_MPG123_PORTABLE_API
			if(mpg123_open_handle64(mh, &file))
			{
				return false;
			}
#else
			if(mpg123_open_handle(mh, &file))
			{
				return false;
			}
#endif
			if(mpg123_scan(mh))
			{
				return false;
			}
			long rate = 0;
			int channels = 0;
			int encoding = 0;
			if(mpg123_getformat(mh, &rate, &channels, &encoding))
			{
				return false;
			}
			if((channels != 1) && (channels != 2))
			{
				return false;
			}
			mpg123_frameinfo frameinfo;
			MemsetZero(frameinfo);
			if(mpg123_info(mh, &frameinfo))
			{
				return false;
			}
			if(frameinfo.layer < 1 || frameinfo.layer > 3)
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_FORCE_RATE, rate, 0.0))
			{
				return false;
			}
			if(mpg123_param(mh, MPG123_ADD_FLAGS, (channels > 1) ? MPG123_FORCE_STEREO : MPG123_FORCE_MONO, 0.0))
			{
				return false;
			}
#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
			length_hdr = mpg123_length64(mh);
#else
			length_hdr = mpg123_length(mh);
#endif
		}

		hasLameXingVbriHeader = (length_raw != length_hdr);

	}

	// Set up decoder...
	MPG123Handle mh;
	if(!mh)
	{
		return false;
	}
	file.Rewind();
	if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_QUIET, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_AUTO_RESAMPLE, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, raw ? MPG123_REMOVE_FLAGS : MPG123_ADD_FLAGS, MPG123_GAPLESS, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, raw ? MPG123_ADD_FLAGS : MPG123_REMOVE_FLAGS, MPG123_IGNORE_INFOFRAME, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, MPG123_REMOVE_FLAGS, MPG123_SKIP_ID3V2, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, raw ? MPG123_ADD_FLAGS : MPG123_REMOVE_FLAGS, MPG123_IGNORE_STREAMLENGTH, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, MPG123_INDEX_SIZE, -1000, 0.0)) // auto-grow
	{
		return false;
	}
#if (MPG123_API_VERSION >= 45)
	if(mpg123_param(mh, MPG123_REMOVE_FLAGS, MPG123_FLOAT_FALLBACK, 0.0)) // allow float
	{
		return false;
	}
#endif
#if (MPG123_API_VERSION >= 45)
	if(mpg123_format_none(mh))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_UNSIGNED_8))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_8))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_16))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_24))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_32))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32))
	{
		return false;
	}
	if(mpg123_format2(mh, 0, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_64))
	{
		return false;
	}
#else
	const long *rates = nullptr;
	size_t num_rates = 0;
	mpg123_rates(&rates, &num_rates);
	if(rates && (num_rates > 0))
	{
		if(mpg123_format_none(mh))
		{
			return false;
		}
		for(std::size_t i = 0; i < num_rates; ++i)
		{
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_UNSIGNED_8))
			{
				return false;
			}
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_8))
			{
				return false;
			}
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_16))
			{
				return false;
			}
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_24))
			{
				return false;
			}
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_32))
			{
				return false;
			}
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32))
			{
				return false;
			}
			if(mpg123_format(mh, rates[i], MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_64))
			{
				return false;
			}
		}
	}
#endif
#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
	if(mpg123_reader64(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderSeek, 0))
	{
		return false;
	}
#else
	if(mpg123_replace_reader_handle(mh, ComponentMPG123::FileReaderRead, ComponentMPG123::FileReaderLSeek, 0))
	{
		return false;
	}
#endif
#if (MPG123_API_VERSION >= 49) && MPT_USE_MPG123_PORTABLE_API
	if(mpg123_open_handle64(mh, &file))
	{
		return false;
	}
#else
	if(mpg123_open_handle(mh, &file))
	{
		return false;
	}
#endif
	if(mpg123_scan(mh))
	{
		return false;
	}
	long rate = 0;
	int channels = 0;
	int encoding = 0;
	if(mpg123_getformat(mh, &rate, &channels, &encoding))
	{
		return false;
	}
	int samplesize = mpg123_encsize(encoding);
	if((channels != 1) && (channels != 2))
	{
		return false;
	}
	if(encoding == 0)
	{
		MPT_ASSERT(false);
		return false;
	} else if(encoding == MPG123_ENC_UNSIGNED_8)
	{
		MPT_ASSERT(samplesize == sizeof(uint8));
	} else if(encoding == MPG123_ENC_SIGNED_8)
	{
		MPT_ASSERT(samplesize == sizeof(int8));
	} else if(encoding == MPG123_ENC_SIGNED_16)
	{
		MPT_ASSERT(samplesize == sizeof(int16));
	} else if(encoding == MPG123_ENC_SIGNED_24)
	{
		MPT_ASSERT(samplesize == sizeof(int24));
	} else if(encoding == MPG123_ENC_SIGNED_32)
	{
		MPT_ASSERT(samplesize == sizeof(int32));
	} else if(encoding == MPG123_ENC_FLOAT_32)
	{
		MPT_ASSERT(samplesize == sizeof(somefloat32));
	} else if(encoding == MPG123_ENC_FLOAT_64)
	{
		MPT_ASSERT(samplesize == sizeof(somefloat64));
	} else
	{
		MPT_ASSERT(false);
		return false;
	}
	mpg123_frameinfo frameinfo;
	MemsetZero(frameinfo);
	if(mpg123_info(mh, &frameinfo))
	{
		return false;
	}
	if(frameinfo.layer < 1 || frameinfo.layer > 3)
	{
		return false;
	}
	// We force samplerate, channels and sampleformat, which in
	// combination with auto-resample (set above) will cause libmpg123
	// to stay with the given format even for completely confused
	// MPG123_FRANKENSTEIN streams.
	// Note that we cannot rely on mpg123_length() for the way we
	// decode the mpeg streams because it depends on the actual frame
	// sample rate instead of the returned sample rate.
	if(mpg123_param(mh, MPG123_FORCE_RATE, rate, 0.0))
	{
		return false;
	}
	if(mpg123_param(mh, MPG123_ADD_FLAGS, (channels > 1) ? MPG123_FORCE_STEREO : MPG123_FORCE_MONO, 0.0))
	{
		return false;
	}

	std::vector<int16> data;

	// decoder delay
	std::size_t data_skip_frames = 0;
	if(!raw && !hasLameXingVbriHeader)
	{
		if(frameinfo.layer == 1)
		{
			data_skip_frames = 240 + 1;
		} else if(frameinfo.layer == 2)
		{
			data_skip_frames = 240 + 1;
		} else if(frameinfo.layer == 3)
		{
			data_skip_frames = 528 + 1;
		}
	}

	std::vector<std::byte> buf_bytes;
	std::vector<uint8> buf_samples_u8;
	std::vector<int8> buf_samples_s8;
	std::vector<int16> buf_samples_s16;
	std::vector<int24> buf_samples_s24;
	std::vector<int32> buf_samples_s32;
	std::vector<somefloat32> buf_samples_f32;
	std::vector<somefloat64> buf_samples_f64;
	std::vector<int16> buf_samples;
	bool decode_error = false;
	bool decode_done = false;
	while(!decode_error && !decode_done)
	{
		buf_bytes.resize(mpg123_outblock(mh));
#if (MPG123_API_VERSION >= 48) && MPT_USE_MPG123_PORTABLE_API
		size_t buf_bytes_decoded = 0;
#else
		mpg123_size_t buf_bytes_decoded = 0;
#endif
		int mpg123_read_result = mpg123_read(mh, mpt::byte_cast<unsigned char*>(buf_bytes.data()), buf_bytes.size(), &buf_bytes_decoded);
		if(mpg123_read_result == MPG123_OK)
		{
			// continue
		} else if(mpg123_read_result == MPG123_NEW_FORMAT)
		{
			if(mpg123_getformat(mh, &rate, &channels, &encoding))
			{
				decode_error = true;
			}
			samplesize = mpg123_encsize(encoding);
			if((channels != 1) && (channels != 2))
			{
				decode_error = true;
			}
			if(encoding == 0)
			{
				MPT_ASSERT(false);
				decode_error = true;
			} else if(encoding == MPG123_ENC_UNSIGNED_8)
			{
				MPT_ASSERT(samplesize == sizeof(uint8));
			} else if(encoding == MPG123_ENC_SIGNED_8)
			{
				MPT_ASSERT(samplesize == sizeof(int8));
			} else if(encoding == MPG123_ENC_SIGNED_16)
			{
				MPT_ASSERT(samplesize == sizeof(int16));
			} else if(encoding == MPG123_ENC_SIGNED_24)
			{
				MPT_ASSERT(samplesize == sizeof(int24));
			} else if(encoding == MPG123_ENC_SIGNED_32)
			{
				MPT_ASSERT(samplesize == sizeof(int32));
			} else if(encoding == MPG123_ENC_FLOAT_32)
			{
				MPT_ASSERT(samplesize == sizeof(somefloat32));
			} else if(encoding == MPG123_ENC_FLOAT_64)
			{
				MPT_ASSERT(samplesize == sizeof(somefloat64));
			} else
			{
				MPT_ASSERT(false);
				decode_error = true;
			}
			// continue
		} else if(mpg123_read_result == MPG123_DONE)
		{
			decode_done = true;
		} else
		{
			decode_error = true;
		}
		if(decode_error)
		{
			break;
		}
		const std::size_t num_frames = buf_bytes_decoded / (samplesize * channels);
		buf_samples.resize(num_frames * channels);
		if(encoding == 0)
		{
			MPT_ASSERT(false);
		} else if(encoding == MPG123_ENC_UNSIGNED_8)
		{
			MPT_ASSERT(samplesize == sizeof(uint8));
			buf_samples_u8.resize(num_frames * channels);
			std::memcpy(buf_samples_u8.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<uint8>(buf_samples_u8.data(), channels, num_frames));
		} else if(encoding == MPG123_ENC_SIGNED_8)
		{
			MPT_ASSERT(samplesize == sizeof(int8));
			buf_samples_s8.resize(num_frames * channels);
			std::memcpy(buf_samples_s8.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<int8>(buf_samples_s8.data(), channels, num_frames));
		} else if(encoding == MPG123_ENC_SIGNED_16)
		{
			MPT_ASSERT(samplesize == sizeof(int16));
			buf_samples_s16.resize(num_frames * channels);
			std::memcpy(buf_samples_s16.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<int16>(buf_samples_s16.data(), channels, num_frames));
		} else if(encoding == MPG123_ENC_SIGNED_24)
		{
			MPT_ASSERT(samplesize == sizeof(int24));
			buf_samples_s24.resize(num_frames * channels);
			std::memcpy(buf_samples_s24.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<int24>(buf_samples_s24.data(), channels, num_frames));
		} else if(encoding == MPG123_ENC_SIGNED_32)
		{
			MPT_ASSERT(samplesize == sizeof(int32));
			buf_samples_s32.resize(num_frames * channels);
			std::memcpy(buf_samples_s32.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<int32>(buf_samples_s32.data(), channels, num_frames));
		} else if(encoding == MPG123_ENC_FLOAT_32)
		{
			MPT_ASSERT(samplesize == sizeof(somefloat32));
			buf_samples_f32.resize(num_frames * channels);
			std::memcpy(buf_samples_f32.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<somefloat32>(buf_samples_f32.data(), channels, num_frames));
		} else if(encoding == MPG123_ENC_FLOAT_64)
		{
			MPT_ASSERT(samplesize == sizeof(somefloat64));
			buf_samples_f64.resize(num_frames * channels);
			std::memcpy(buf_samples_f64.data(), buf_bytes.data(), num_frames * channels * samplesize);
			CopyAudio(mpt::audio_span_interleaved<int16>(buf_samples.data(), channels, num_frames), mpt::audio_span_interleaved<somefloat64>(buf_samples_f64.data(), channels, num_frames));
		} else
		{
			MPT_ASSERT(false);
		}
		mpt::append(data, buf_samples.data(), buf_samples.data() + (num_frames * channels));
		if((data.size() / channels) > MAX_SAMPLE_LENGTH)
		{
			break;
		}
	}

	if((data.size() / channels) > MAX_SAMPLE_LENGTH)
	{
		return false;
	}

	FileTags tags;
	mpg123_id3v1 *id3v1 = nullptr;
	mpg123_id3v2 *id3v2 = nullptr;
	if(mpg123_id3(mh, &id3v1, &id3v2) == MPG123_OK)
	{
		if(id3v2)
		{
			if(tags.title.empty())    tags.title    = ReadMPG123String(id3v2->title);
			if(tags.artist.empty())   tags.artist   = ReadMPG123String(id3v2->artist);
			if(tags.album.empty())    tags.album    = ReadMPG123String(id3v2->album);
			if(tags.year.empty())     tags.year     = ReadMPG123String(id3v2->year);
			if(tags.genre.empty())    tags.genre    = ReadMPG123String(id3v2->genre);
			if(tags.comments.empty()) tags.comments = ReadMPG123String(id3v2->comment);
		}
		if(id3v1)
		{
			if(tags.title.empty())    tags.title    = ReadMPG123String(id3v1->title);
			if(tags.artist.empty())   tags.artist   = ReadMPG123String(id3v1->artist);
			if(tags.album.empty())    tags.album    = ReadMPG123String(id3v1->album);
			if(tags.year.empty())     tags.year     = ReadMPG123String(id3v1->year);
			if(tags.comments.empty()) tags.comments = ReadMPG123String(id3v1->comment);
		}
	}
	mpt::ustring sampleName = GetSampleNameFromTags(tags);

	DestroySampleThreadsafe(sample);
	if(!mo3Decode)
	{
		m_szNames[sample] = mpt::ToCharset(GetCharsetInternal(), sampleName);
		Samples[sample].Initialize();
		Samples[sample].nC5Speed = static_cast<uint32>(rate);
	}
	Samples[sample].nLength = mpt::saturate_cast<SmpLength>((data.size() / channels) - data_skip_frames);

	Samples[sample].uFlags.set(CHN_16BIT);
	Samples[sample].uFlags.set(CHN_STEREO, channels == 2);
	Samples[sample].AllocateSample();

	if(Samples[sample].HasSampleData())
	{
		std::memcpy(Samples[sample].sampleb(), data.data() + (data_skip_frames * channels), (data.size() - (data_skip_frames * channels)) * sizeof(int16));
	}

	if(!mo3Decode)
	{
		Samples[sample].Convert(MOD_TYPE_IT, GetType());
		Samples[sample].PrecomputeLoops(*this, false);
	}
	return Samples[sample].HasSampleData();

#elif defined(MPT_WITH_MINIMP3)

	MPT_UNREFERENCED_PARAMETER(raw);

	file.Rewind();
	FileReader::PinnedView rawDataView = file.GetPinnedView();
	int64 bytes_left = rawDataView.size();
	const uint8 *stream_pos = mpt::byte_cast<const uint8 *>(rawDataView.data());

	std::vector<int16> raw_sample_data;

	mpt::heap_value<mp3dec_t> mp3;
	std::memset(&*mp3, 0, sizeof(mp3dec_t));
	mp3dec_init(&*mp3);
	
	int rate = 0;
	int channels = 0;

	mp3dec_frame_info_t info;
	std::memset(&info, 0, sizeof(mp3dec_frame_info_t));
	std::vector<int16> sample_buf(MINIMP3_MAX_SAMPLES_PER_FRAME);
	do
	{
		int frame_samples = mp3dec_decode_frame_no_inline(&*mp3, stream_pos, mpt::saturate_cast<int>(bytes_left), sample_buf.data(), &info);
		if(frame_samples < 0 || info.frame_bytes < 0) break; // internal error in minimp3
		if(frame_samples > 0 && info.frame_bytes == 0) break; // internal error in minimp3
		if(frame_samples == 0 && info.frame_bytes == 0) break; // end of stream, no progress
		if(frame_samples == 0 && info.frame_bytes > 0) do { } while(0); // decoder skipped non-mp3 data
		if(frame_samples > 0 && info.frame_bytes > 0) do { } while(0); // normal
		if(info.frame_bytes > 0)
		{
			if(rate != 0 && rate != info.hz) break; // inconsistent stream
			if(channels != 0 && channels != info.channels) break; // inconsistent stream
			rate = info.hz;
			channels = info.channels;
			if(rate <= 0) break; // broken stream
			if(channels != 1 && channels != 2) break; // broken stream
			stream_pos += std::clamp(info.frame_bytes, 0, mpt::saturate_cast<int>(bytes_left));
			bytes_left -= std::clamp(info.frame_bytes, 0, mpt::saturate_cast<int>(bytes_left));
			if(frame_samples > 0)
			{
				try
				{
					mpt::append(raw_sample_data, sample_buf.data(), sample_buf.data() + frame_samples * channels);
				} catch(mpt::out_of_memory e)
				{
					mpt::delete_out_of_memory(e);
					break;
				}
			}
		}
		if((raw_sample_data.size() / channels) > MAX_SAMPLE_LENGTH)
		{
			break;
		}
	} while(bytes_left > 0);

	if(rate == 0 || channels == 0 || raw_sample_data.empty())
	{
		return false;
	}

	if((raw_sample_data.size() / channels) > MAX_SAMPLE_LENGTH)
	{
		return false;
	}

	DestroySampleThreadsafe(sample);
	if(!mo3Decode)
	{
		m_szNames[sample] = "";
		Samples[sample].Initialize();
		Samples[sample].nC5Speed = rate;
	}
	Samples[sample].nLength = mpt::saturate_cast<SmpLength>(raw_sample_data.size() / channels);

	Samples[sample].uFlags.set(CHN_16BIT);
	Samples[sample].uFlags.set(CHN_STEREO, channels == 2);
	Samples[sample].AllocateSample();

	if(Samples[sample].HasSampleData())
	{
		std::copy(raw_sample_data.begin(), raw_sample_data.end(), Samples[sample].sample16());
	}

	if(!mo3Decode)
	{
		Samples[sample].Convert(MOD_TYPE_IT, GetType());
		Samples[sample].PrecomputeLoops(*this, false);
	}
	return Samples[sample].HasSampleData();

#else

	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);
	MPT_UNREFERENCED_PARAMETER(raw);
	MPT_UNREFERENCED_PARAMETER(mo3Decode);

#endif // MPT_WITH_MPG123 || MPT_WITH_MINIMP3

	return false;
}


OPENMPT_NAMESPACE_END
