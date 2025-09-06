/*
 * Copyright (c) 2001-2003 The FFmpeg project
 *
 * first version by Francois Revol (revol@free.fr)
 * fringe ADPCM codecs (e.g., DK3, DK4, Westwood)
 *   by Mike Melanson (melanson@pcisys.net)
 * CD-ROM XA ADPCM codec by BERO
 * EA ADPCM decoder by Robin Kay (komadori@myrealbox.com)
 * EA ADPCM R1/R2/R3 decoder by Peter Ross (pross@xvid.org)
 * EA IMA EACS decoder by Peter Ross (pross@xvid.org)
 * EA IMA SEAD decoder by Peter Ross (pross@xvid.org)
 * EA ADPCM XAS decoder by Peter Ross (pross@xvid.org)
 * MAXIS EA ADPCM decoder by Robert Marston (rmarston@gmail.com)
 * THP ADPCM decoder by Marco Gerards (mgerards@xs4all.nl)
 * Argonaut Games ADPCM decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Simon & Schuster Interactive ADPCM decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Ubisoft ADPCM decoder by Zane van Iperen (zane@zanevaniperen.com)
 * High Voltage Software ALP decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Cunning Developments decoder by Zane van Iperen (zane@zanevaniperen.com)
 * Sanyo LD-ADPCM decoder by Peter Ross (pross@xvid.org)
 *
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

//#include "config_components.h"

//#include "avcodec.h"
#include "get_bits.h"
#include "bytestream.h"
#include "adpcm.h"
#include "adpcm_data.h"
#include "codec_internal.h"
#include "decode.h"
#include "packet.h"

/**
 * @file
 * ADPCM decoders
 * Features and limitations:
 *
 * Reference documents:
 * http://wiki.multimedia.cx/index.php?title=Category:ADPCM_Audio_Codecs
 * http://www.pcisys.net/~melanson/codecs/simpleaudio.html [dead]
 * http://www.geocities.com/SiliconValley/8682/aud3.txt [dead]
 * http://openquicktime.sourceforge.net/
 * XAnim sources (xa_codec.c) http://xanim.polter.net/
 * http://www.cs.ucla.edu/~leec/mediabench/applications.html [dead]
 * SoX source code http://sox.sourceforge.net/
 *
 * CD-ROM XA:
 * http://ku-www.ss.titech.ac.jp/~yatsushi/xaadpcm.html [dead]
 * vagpack & depack http://homepages.compuserve.de/bITmASTER32/psx-index.html [dead]
 * readstr http://www.geocities.co.jp/Playtown/2004/
 */

typedef struct ADPCMDecodeContext {
    ADPCMChannelStatus status[14];
} ADPCMDecodeContext;

static void adpcm_flush(AVCodecContext *avctx);

static av_cold int adpcm_decode_init(AVCodecContext * avctx)
{
    ADPCMDecodeContext *c = avctx->priv_data;
    unsigned int min_channels = 1;
    unsigned int max_channels = 6;

    adpcm_flush(avctx);

    if (avctx->nb_channels < min_channels ||
        avctx->nb_channels > max_channels) {
        av_log(avctx, AV_LOG_ERROR, "Invalid number of channels\n");
        return AVERROR(EINVAL);
    }

    return 0;
}

/**
 * Clip a signed integer value into the -32768,32767 range.
 * @param a value to clip
 * @return clipped value
 */
static av_always_inline av_const int16_t av_clip_int16_c(int a)
{
    if ((a + 0x8000U) & ~0xFFFF) return (a >> 31) ^ 0x7FFF;
    else                      return a;
}

static inline int16_t adpcm_ms_expand_nibble(ADPCMChannelStatus *c, int nibble)
{
    int predictor;

    predictor = (((c->sample1) * (c->coeff1)) + ((c->sample2) * (c->coeff2))) / 64;
    predictor += ((nibble & 0x08)?(nibble - 0x10):(nibble)) * c->idelta;

    c->sample2 = c->sample1;
    c->sample1 = av_clip_int16_c(predictor);
    c->idelta = (ff_adpcm_AdaptationTable[(int)nibble] * c->idelta) >> 8;
    if (c->idelta < 16) c->idelta = 16;
    if (c->idelta > INT_MAX/768) {
        av_log(NULL, AV_LOG_WARNING, "idelta overflow\n");
        c->idelta = INT_MAX/768;
    }

    return c->sample1;
}

/**
 * Get the number of samples (per channel) that will be decoded from the packet.
 * In one case, this is actually the maximum number of samples possible to
 * decode with the given buf_size.
 *
 * @param[out] coded_samples set to the number of samples as coded in the
 *                           packet, or 0 if the codec does not encode the
 *                           number of samples in each frame.
 * @param[out] approx_nb_samples set to non-zero if the number of samples
 *                               returned is an approximation.
 */
static int get_nb_samples(AVCodecContext *avctx, GetByteContext *gb,
                          int buf_size, int *coded_samples, int *approx_nb_samples)
{
    ADPCMDecodeContext *s = avctx->priv_data;
    int nb_samples        = 0;
    int ch                = avctx->nb_channels;
    int header_size;

    *coded_samples = 0;
    *approx_nb_samples = 0;

    if(ch <= 0)
        return 0;

    if (avctx->block_align > 0)
        buf_size = FFMIN(buf_size, avctx->block_align);
    nb_samples = (buf_size - 6 * ch) * 2 / ch;

    return nb_samples;
}

static int adpcm_decode_frame(AVCodecContext *avctx, AVFrame *frame,
                              int *got_frame_ptr, AVPacket *avpkt)
{
    const uint8_t *buf = avpkt->data;
    int buf_size = avpkt->size;
    ADPCMDecodeContext *c = avctx->priv_data;
    int channels = avctx->nb_channels;
    int16_t *samples;
    int st; /* stereo */
    int nb_samples, coded_samples, approx_nb_samples, ret;
    GetByteContext gb;

    bytestream2_init(&gb, buf, buf_size);
    nb_samples = get_nb_samples(avctx, &gb, buf_size, &coded_samples, &approx_nb_samples);
    if (nb_samples <= 0) {
        av_log(avctx, AV_LOG_ERROR, "invalid number of samples in packet\n");
        return AVERROR_INVALIDDATA;
    }

    /* get output buffer */
    frame->nb_samples = nb_samples;
    if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
        return ret;

    /* use coded_samples when applicable */
    /* it is always <= nb_samples, so the output buffer will be large enough */
    if (coded_samples) {
        if (!approx_nb_samples && coded_samples != nb_samples)
            av_log(avctx, AV_LOG_WARNING, "mismatch in coded sample count\n");
        frame->nb_samples = nb_samples = coded_samples;
    }

    st = channels == 2 ? 1 : 0;

    int block_predictor;

    if (avctx->nb_channels > 2) {
        for (int channel = 0; channel < avctx->nb_channels; channel++) {
            samples = (int16_t*)frame->data[channel];
            block_predictor = bytestream2_get_byteu(&gb);
            if (block_predictor > 6) {
                av_log(avctx, AV_LOG_ERROR, "ERROR: block_predictor[%d] = %d\n",
                        channel, block_predictor);
                return AVERROR_INVALIDDATA;
            }
            c->status[channel].coeff1 = ff_adpcm_AdaptCoeff1[block_predictor];
            c->status[channel].coeff2 = ff_adpcm_AdaptCoeff2[block_predictor];
            c->status[channel].idelta = sign_extend(bytestream2_get_le16u(&gb), 16);
            c->status[channel].sample1 = sign_extend(bytestream2_get_le16u(&gb), 16);
            c->status[channel].sample2 = sign_extend(bytestream2_get_le16u(&gb), 16);
            *samples++ = c->status[channel].sample2;
            *samples++ = c->status[channel].sample1;
            for (int n = (nb_samples - 2) >> 1; n > 0; n--) {
                int byte = bytestream2_get_byteu(&gb);
                *samples++ = adpcm_ms_expand_nibble(&c->status[channel], byte >> 4  );
                *samples++ = adpcm_ms_expand_nibble(&c->status[channel], byte & 0x0F);
            }
        }
    } else {
        samples = (int16_t*)frame->data[0];
        block_predictor = bytestream2_get_byteu(&gb);
        if (block_predictor > 6) {
            av_log(avctx, AV_LOG_ERROR, "ERROR: block_predictor[0] = %d\n",
                    block_predictor);
            return AVERROR_INVALIDDATA;
        }
        c->status[0].coeff1 = ff_adpcm_AdaptCoeff1[block_predictor];
        c->status[0].coeff2 = ff_adpcm_AdaptCoeff2[block_predictor];
        if (st) {
            block_predictor = bytestream2_get_byteu(&gb);
            if (block_predictor > 6) {
                av_log(avctx, AV_LOG_ERROR, "ERROR: block_predictor[1] = %d\n",
                        block_predictor);
                return AVERROR_INVALIDDATA;
            }
            c->status[1].coeff1 = ff_adpcm_AdaptCoeff1[block_predictor];
            c->status[1].coeff2 = ff_adpcm_AdaptCoeff2[block_predictor];
        }
        c->status[0].idelta = sign_extend(bytestream2_get_le16u(&gb), 16);
        if (st){
            c->status[1].idelta = sign_extend(bytestream2_get_le16u(&gb), 16);
        }

        c->status[0].sample1 = sign_extend(bytestream2_get_le16u(&gb), 16);
        if (st) c->status[1].sample1 = sign_extend(bytestream2_get_le16u(&gb), 16);
        c->status[0].sample2 = sign_extend(bytestream2_get_le16u(&gb), 16);
        if (st) c->status[1].sample2 = sign_extend(bytestream2_get_le16u(&gb), 16);

        *samples++ = c->status[0].sample2;
        if (st) *samples++ = c->status[1].sample2;
        *samples++ = c->status[0].sample1;
        if (st) *samples++ = c->status[1].sample1;
        for (int n = (nb_samples - 2) >> (1 - st); n > 0; n--) {
            int byte = bytestream2_get_byteu(&gb);
            *samples++ = adpcm_ms_expand_nibble(&c->status[0 ], byte >> 4  );
            *samples++ = adpcm_ms_expand_nibble(&c->status[st], byte & 0x0F);
        }
    }

    if (avpkt->size && bytestream2_tell(&gb) == 0) {
        av_log(avctx, AV_LOG_ERROR, "Nothing consumed\n");
        return AVERROR_INVALIDDATA;
    }

    *got_frame_ptr = 1;

    if (avpkt->size < bytestream2_tell(&gb)) {
        av_log(avctx, AV_LOG_ERROR, "Overread of %d < %d\n", avpkt->size, bytestream2_tell(&gb));
        return avpkt->size;
    }

    return bytestream2_tell(&gb);
}

static void adpcm_flush(AVCodecContext *avctx)
{
    ADPCMDecodeContext *c = avctx->priv_data;

    /* Just nuke the entire state and re-init. */
    memset(c, 0, sizeof(ADPCMDecodeContext));
}

const FFCodec ff_adpcm_ms_decoder = {
    .priv_data_size = sizeof(ADPCMDecodeContext),
    .init           = adpcm_decode_init,
    .flush          = adpcm_flush,
    .decode         = adpcm_decode_frame,
};
