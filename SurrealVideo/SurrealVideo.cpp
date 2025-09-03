
#include "SurrealVideo.h"
#include <cstdint>

extern "C"
{
	#include "libavutil/log.h"
	#include "libavutil/frame.h"
	#include "libavcodec/codec_internal.h"

	int ff_set_dimensions(AVCodecContext* s, int width, int height) { return 0; }
	int ff_get_buffer(AVCodecContext* avctx, AVFrame* frame, int flags) { return 0; }

	int av_image_check_size2(unsigned int w, unsigned int h, int64_t max_pixels, enum AVPixelFormat pix_fmt, int log_offset, void* log_ctx) { return 0; }

	void av_frame_unref(AVFrame* frame) {}
	void av_frame_move_ref(AVFrame* dst, AVFrame* src) {}
	void av_frame_free(AVFrame** frame) { *frame = nullptr; }

	void av_log(void* avcl, int level, const char* fmt, ...) av_printf_format(3, 4) {}

	extern FFCodec ff_indeo5_decoder;

#ifdef _MSC_VER
	__declspec(dllexport) FFCodec* GetIndeo5Decoder()
	{
		return &ff_indeo5_decoder;
	}
#else
	FFCodec* GetIndeo5Decoder()
	{
		return &ff_indeo5_decoder;
	}
#endif

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
