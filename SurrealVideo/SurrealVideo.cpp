
#define BUILDING_SURREALVIDEO
#include "SurrealVideo.h"
#include <cstdint>
#include <algorithm>
#include <stdexcept>

extern "C"
{
#include "libavutil/log.h"
#include "libavutil/frame.h"
#include "libavcodec/codec_internal.h"
#include "libavcodec/decode.h"
#include "libavcodec/packet.h"

	int av_image_check_size2(unsigned int w, unsigned int h, int64_t max_pixels, enum AVPixelFormat pix_fmt, int log_offset, void* log_ctx)
	{
		return 0;
	}

	int ff_set_dimensions(AVCodecContext* s, int width, int height)
	{
		int ret = av_image_check_size2(width, height, s->max_pixels, AV_PIX_FMT_NONE, 0, s);
		if (ret < 0)
			width = height = 0;
		s->width = width;
		s->height = height;
		return 0;
	}

	int ff_get_buffer(AVCodecContext* avctx, AVFrame* frame, int flags)
	{
		if (avctx->pix_fmt != AV_PIX_FMT_YUV410P)
			return -1;

		// planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
		for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
		{
			if (i == 0)
			{
				int planewidth = avctx->width;
				int planeheight = avctx->height;
				int bytes_per_line = (planewidth * 9 + 7) / 8;
				int pitch = (bytes_per_line + 15) / 16 * 16;
				frame->data[i] = (uint8_t*)malloc(planeheight * pitch);
				frame->linesize[i] = pitch;
			}
			else if (i == 1 || i == 2)
			{
				int planewidth = (avctx->width + 3) / 4;
				int planeheight = (avctx->height + 3) / 4;
				int bytes_per_line = (planewidth * 9 + 7) / 8;
				int pitch = (bytes_per_line + 15) / 16 * 16;
				frame->data[i] = (uint8_t*)malloc(planeheight * pitch);
				frame->linesize[i] = pitch;
			}
			else
			{
				frame->data[i] = 0;
				frame->linesize[i] = 0;
			}
		}
		return 0;
	}

	void av_frame_unref(AVFrame* frame)
	{
		if (!frame)
			return;

		for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
		{
			free(frame->data[i]);
			frame->data[i] = nullptr;
			frame->linesize[i] = 0;
		}
	}

	void av_frame_move_ref(AVFrame* dst, AVFrame* src)
	{
		for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
		{
			std::swap(dst->data[i], src->data[i]);
			std::swap(dst->linesize[i], src->linesize[i]);
		}
		av_frame_unref(src);
	}

	void av_frame_free(AVFrame** frame)
	{
		av_frame_unref(*frame);
		*frame = nullptr;
	}

	void av_log(void* avcl, int level, const char* fmt, ...) av_printf_format(3, 4)
	{
	}

	extern FFCodec ff_indeo5_decoder;

	/* The part of mem.c we are actually using
	void* av_mallocz(size_t size) av_malloc_attrib av_alloc_size(1) { return nullptr; }
	void* av_malloc(size_t size) av_malloc_attrib av_alloc_size(1) { return nullptr; }
	av_alloc_size(1, 2) void* av_malloc_array(size_t nmemb, size_t size) { return nullptr; }
	void* av_realloc_f(void* ptr, size_t nelem, size_t elsize) { return nullptr; }
	void* av_calloc(size_t nmemb, size_t size) av_malloc_attrib av_alloc_size(1, 2) { return nullptr; }
	void av_free(void* ptr) {}
	void av_freep(void* ptr) {}
	*/
}

class VideoDecoder : public IVideoDecoder
{
public:
	VideoDecoder()
	{
		context.codec_id = AV_CODEC_ID_INDEO5;
		context.priv_data = malloc(ff_indeo5_decoder.priv_data_size);
		if (!context.priv_data)
			throw std::runtime_error("malloc(ff_indeo5_decoder.priv_data_size) failed");
		memset(context.priv_data, 0, ff_indeo5_decoder.priv_data_size);
		if (ff_indeo5_decoder.init(&context) < 0)
		{
			free(context.priv_data);
			throw std::runtime_error("ff_indeo5_decoder.init failed");
		}
	}

	~VideoDecoder()
	{
		ff_indeo5_decoder.close(&context);
		free(context.priv_data);
		av_frame_unref(&frame);
	}

	VideoDecoderResult Decode(const void* data, size_t size) override
	{
		try
		{
			AVPacket packet =
			{
				.data = (uint8_t*)data,
				.size = (int)size
			};
			int got_frame = 0;
			int result = ff_indeo5_decoder.decode(&context, &frame, &got_frame, &packet);
			if (result < 0)
				return VideoDecoderResult::Error;
			return got_frame ? VideoDecoderResult::DecodedFrame : VideoDecoderResult::Decoded;
		}
		catch (...) // Don't float C++ exceptions through this API
		{
			return VideoDecoderResult::Error;
		}
	}

	int GetWidth() override
	{
		return context.width;
	}

	int GetHeight() override
	{
		return context.height;
	}

	int GetPitch(int channel) override
	{
		return frame.linesize[channel];
	}

	void* GetData(int channel) override
	{
		return frame.data[channel];
	}

	void Release() override
	{
		delete this;
	}

	AVCodecContext context = {};
	AVFrame frame = {};
};

SURREALVIDEO_API IVideoDecoder* CreateVideoDecoder()
{
	try
	{
		return new VideoDecoder();
	}
	catch (...)
	{
		return nullptr;
	}
}
