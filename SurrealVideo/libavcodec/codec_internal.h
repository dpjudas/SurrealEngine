/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_CODEC_INTERNAL_H
#define AVCODEC_CODEC_INTERNAL_H

#include <stdint.h>

typedef struct FFCodecDefault {
    const char *key;
    const char *value;
} FFCodecDefault;

struct AVCodecContext;

typedef struct FFCodec {

    int priv_data_size;

    int (*init)(struct AVCodecContext *);

    int (*decode)(struct AVCodecContext *avctx, struct AVFrame *frame,
                  int *got_frame_ptr, struct AVPacket *avpkt);

    int (*close)(struct AVCodecContext *);

} FFCodec;

#endif /* AVCODEC_CODEC_INTERNAL_H */
