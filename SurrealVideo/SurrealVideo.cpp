
#define BUILDING_SURREALVIDEO
#include "SurrealVideo.h"
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <vector>

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
		if (avctx->codec_id != AV_CODEC_ID_ADPCM_MS)
		{
			if (avctx->pix_fmt != AV_PIX_FMT_YUV410P)
				return -1;

			// planar YUV 4:1:0,  8bpp, (1 Cr & Cb sample per 4x4 Y samples)
			for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
			{
				if (i == 0)
				{
					int planewidth = avctx->width;
					int planeheight = avctx->height;
					int bytes_per_line = planewidth;
					int pitch = (bytes_per_line + 15) / 16 * 16;
					frame->data[i] = (uint8_t*)malloc(planeheight * pitch);
					frame->linesize[i] = pitch;
				}
				else if (i == 1 || i == 2)
				{
					int planewidth = (avctx->width + 3) / 4;
					int planeheight = (avctx->height + 3) / 4;
					int bytes_per_line = planewidth;
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
		}
		else
		{
			int size = frame->nb_samples * sizeof(int16_t);
			if (avctx->nb_channels > 2)
			{
				for (int i = 0; i < avctx->nb_channels; i++)
				{
					frame->data[i] = (uint8_t*)malloc(size);
					frame->linesize[i] = size;
				}
				for (int i = avctx->nb_channels; i < AV_NUM_DATA_POINTERS; i++)
				{
					frame->data[i] = 0;
					frame->linesize[i] = 0;
				}
			}
			else
			{
				size *= 2;
				frame->data[0] = (uint8_t*)malloc(size);
				frame->linesize[0] = size;
				for (int i = 1; i < AV_NUM_DATA_POINTERS; i++)
				{
					frame->data[i] = 0;
					frame->linesize[i] = 0;
				}
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
	extern FFCodec ff_adpcm_ms_decoder;

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

			if (got_frame)
			{
				pixels.resize(context.width * context.height);
				crbuffer.resize(context.width * context.height);
				cbbuffer.resize(context.width * context.height);
				Upscale(context.width, context.height, frame.data[1], frame.linesize[1], cbbuffer.data());
				Upscale(context.width, context.height, frame.data[2], frame.linesize[2], crbuffer.data());
				YUVToRGBA(context.width, context.height, frame.data[0], frame.linesize[0], cbbuffer.data(), crbuffer.data(), pixels.data());
			}

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

	const uint32_t* GetPixels() override
	{
		return pixels.data();
	}

	void Release() override
	{
		delete this;
	}

	static void Upscale(int width, int height, const uint8_t* src, int srcpitch, uint8_t* dest)
	{
		int srcxmax = width / 4 - 1;
		int srcymax = height / 4 - 1;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int sx0 = x >> 2;
				int sy0 = y >> 2;
				int sx1 = std::min(sx0 + 1, srcxmax);
				int sy1 = std::min(sy0 + 1, srcymax);
				float s00 = src[sx0 + sy0 * srcpitch];
				float s10 = src[sx1 + sy0 * srcpitch];
				float s01 = src[sx0 + sy1 * srcpitch];
				float s11 = src[sx1 + sy1 * srcpitch];
				float u = x * (1.0f / 4.0f);
				float v = y * (1.0f / 4.0f);
				float a = u - (int)u;
				float b = v - (int)v;
				float inva = 1.0f - a;
				float invb = 1.0f - b;
				float value = s00 * (inva * invb) + s10 * (a * invb) + s01 * (inva * b) + s11 * (a * b);
				dest[x + y * width] = (int)(value + 0.5f);
			}
		}
	}

	static void YUVToRGBA(int width, int height, const uint8_t* ychannel, int ypitch, const uint8_t* cbchannel, const uint8_t* crchannel, uint32_t* pixels)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				float Y = ychannel[x + y * ypitch];
				float Cb = cbchannel[x + y * width];
				float Cr = crchannel[x + y * width];
				Cr -= 128.0f;
				Cb -= 128.0f;

				float R = Y + 1.40200f * Cr;
				float G = Y - 0.34414f * Cb - 0.71414f * Cr;
				float B = Y + 1.77200f * Cb;

				R = std::max(R, 0.0f);
				R = std::min(R, 255.0f);
				G = std::max(G, 0.0f);
				G = std::min(G, 255.0f);
				B = std::max(B, 0.0f);
				B = std::min(B, 255.0f);

				R += 0.5f;
				G += 0.5f;
				B += 0.5f;

				pixels[x + y * width] = 0xff000000 + ((uint8_t)B) + (((uint8_t)G) << 8) + (((uint8_t)R) << 16);
			}
		}
	}

	std::vector<uint8_t> crbuffer, cbbuffer;
	std::vector<uint32_t> pixels;
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

class AudioDecoder : public IAudioDecoder
{
public:
	AudioDecoder(int channels, int block_align)
	{
		context.nb_channels = channels;
		context.block_align = block_align;
		context.codec_id = AV_CODEC_ID_ADPCM_MS;
		context.priv_data = malloc(ff_adpcm_ms_decoder.priv_data_size);
		if (!context.priv_data)
			throw std::runtime_error("malloc(ff_adpcm_ms_decoder.priv_data_size) failed");
		memset(context.priv_data, 0, ff_adpcm_ms_decoder.priv_data_size);
		if (ff_adpcm_ms_decoder.init(&context) < 0)
		{
			free(context.priv_data);
			throw std::runtime_error("ff_adpcm_ms_decoder.init failed");
		}
	}

	~AudioDecoder()
	{
		if (ff_adpcm_ms_decoder.close)
			ff_adpcm_ms_decoder.close(&context);
		free(context.priv_data);
		av_frame_unref(&frame);
	}

	AudioDecoderResult Decode(const void* data, size_t size) override
	{
		try
		{
			AVPacket packet =
			{
				.data = (uint8_t*)data,
				.size = (int)size
			};
			int got_frame = 0;
			int result = ff_adpcm_ms_decoder.decode(&context, &frame, &got_frame, &packet);
			if (result < 0)
				return AudioDecoderResult::Error;
			return got_frame ? AudioDecoderResult::DecodedFrame : AudioDecoderResult::Decoded;
		}
		catch (...) // Don't float C++ exceptions through this API
		{
			return AudioDecoderResult::Error;
		}
	}

	const int16_t* GetSamples() override
	{
		return (int16_t*)frame.data[0];
	}

	int GetSampleCount() override
	{
		return frame.nb_samples;
	}

	void Release() override
	{
		delete this;
	}

	std::vector<uint8_t> crbuffer, cbbuffer;
	AVCodecContext context = {};
	AVFrame frame = {};
};

SURREALVIDEO_API IAudioDecoder* CreateAudioDecoder(int channels, int block_align)
{
	try
	{
		return new AudioDecoder(channels, block_align);
	}
	catch (...)
	{
		return nullptr;
	}
}
