/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * rendduh.c - Functions for rendering a DUH into     / / \  \
 *             an end-user sample format.            | <  /   \_
 *                                                   |  \/ /\   /
 * By entheh.                                         \_  /  > /
 *                                                      | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#include <stdlib.h>
#include <limits.h>

#include "dumb.h"
#include "internal/dumb.h"

/* On the x86, we can use some tricks to speed stuff up */
#if (defined _MSC_VER) || (defined __DJGPP__) || (defined __MINGW__)
// Can't we detect Linux and other x86 platforms here? :/

#define FAST_MID(var, min, max)                                                \
    {                                                                          \
        var -= (min);                                                          \
        var &= (~var) >> (sizeof(var) * CHAR_BIT - 1);                         \
        var += (min);                                                          \
        var -= (max);                                                          \
        var &= var >> (sizeof(var) * CHAR_BIT - 1);                            \
        var += (max);                                                          \
    }

#define CONVERT8(src, pos, signconv)                                           \
    {                                                                          \
        signed int f = (src + 0x8000) >> 16;                                   \
        FAST_MID(f, -128, 127);                                                \
        ((char *)sptr)[pos] = (char)f ^ signconv;                              \
    }

#define CONVERT16(src, pos, signconv)                                          \
    {                                                                          \
        signed int f = (src + 0x80) >> 8;                                      \
        FAST_MID(f, -32768, 32767);                                            \
        ((short *)sptr)[pos] = (short)(f ^ signconv);                          \
    }

#else

#define CONVERT8(src, pos, signconv)                                           \
    {                                                                          \
        signed int f = (src + 0x8000) >> 16;                                   \
        f = MID(-128, f, 127);                                                 \
        ((char *)sptr)[pos] = (char)f ^ signconv;                              \
    }

#define CONVERT16(src, pos, signconv)                                          \
    {                                                                          \
        signed int f = (src + 0x80) >> 8;                                      \
        f = MID(-32768, f, 32767);                                             \
        ((short *)sptr)[pos] = (short)(f ^ signconv);                          \
    }

#endif

#define CONVERT24(src, pos)                                                    \
    {                                                                          \
        signed int f = src;                                                    \
        f = MID(-8388608, f, 8388607);                                         \
        ((unsigned char *)sptr)[pos] = (f)&0xFF;                               \
        ((unsigned char *)sptr)[pos + 1] = (f >> 8) & 0xFF;                    \
        ((unsigned char *)sptr)[pos + 2] = (f >> 16) & 0xFF;                   \
    }

#define CONVERT32F(src, pos)                                                   \
    {                                                                          \
        ((float *)sptr)[pos] =                                                 \
            (float)((signed int)src) * (1.0f / (float)(0xffffff / 2 + 1));     \
    }

#define CONVERT64F(src, pos)                                                   \
    {                                                                          \
        ((double *)sptr)[pos] =                                                \
            (double)((signed int)src) * (1.0 / (double)(0xffffff / 2 + 1));    \
    }

/* This is the only deprecated function in 2.0.0. */
/* DEPRECATED */
long duh_render(DUH_SIGRENDERER *sigrenderer, int bits, int unsign,
                float volume, float delta, long size, void *sptr) {
    long n;

    sample_t **sampptr;

    int n_channels;

    ASSERT(bits == 8 || bits == 16);
    ASSERT(sptr);

    if (!sigrenderer)
        return 0;

    n_channels = duh_sigrenderer_get_n_channels(sigrenderer);

    ASSERT(n_channels > 0);
    /* This restriction will be removed when need be. At the moment, tightly
     * optimised loops exist for exactly one or two channels.
     */
    ASSERT(n_channels <= 2);

    sampptr = allocate_sample_buffer(n_channels, size);

    if (!sampptr)
        return 0;

    dumb_silence(sampptr[0], n_channels * size);

    size = duh_sigrenderer_generate_samples(sigrenderer, volume, delta, size,
                                            sampptr);

    if (bits == 16) {
        int signconv = unsign ? 0x8000 : 0x0000;

        for (n = 0; n < size * n_channels; n++) {
            CONVERT16(sampptr[0][n], n, signconv);
        }
    } else {
        char signconv = unsign ? 0x80 : 0x00;

        for (n = 0; n < size * n_channels; n++) {
            CONVERT8(sampptr[0][n], n, signconv);
        }
    }

    destroy_sample_buffer(sampptr);

    return size;
}

long duh_render_int(DUH_SIGRENDERER *sigrenderer, sample_t ***sig_samples,
                    long *sig_samples_size, int bits, int unsign, float volume,
                    float delta, long size, void *sptr) {
    long n;

    sample_t **sampptr;

    int n_channels;

    ASSERT(bits == 8 || bits == 16 || bits == 24);
    ASSERT(sptr);
    ASSERT(sig_samples);
    ASSERT(sig_samples_size);

    if (!sigrenderer)
        return 0;

    n_channels = duh_sigrenderer_get_n_channels(sigrenderer);

    ASSERT(n_channels > 0);
    /* This restriction will be removed when need be. At the moment, tightly
     * optimised loops exist for exactly one or two channels.
     */
    ASSERT(n_channels <= 2);

    if ((*sig_samples == NULL) || (*sig_samples_size != size)) {
        destroy_sample_buffer(*sig_samples);
        *sig_samples = allocate_sample_buffer(n_channels, size);
        *sig_samples_size = size;
    }
    sampptr = *sig_samples;

    if (!sampptr)
        return 0;

    dumb_silence(sampptr[0], n_channels * size);

    size = duh_sigrenderer_generate_samples(sigrenderer, volume, delta, size,
                                            sampptr);

    if (bits == 24) {
        long i = 0;
        ASSERT(unsign == 0);

        for (n = 0; n < size * n_channels; n++, i += 3) {
            CONVERT24(sampptr[0][n], i);
        }
    } else if (bits == 16) {
        int signconv = unsign ? 0x8000 : 0x0000;

        for (n = 0; n < size * n_channels; n++) {
            CONVERT16(sampptr[0][n], n, signconv);
        }
    } else {
        char signconv = unsign ? 0x80 : 0x00;

        for (n = 0; n < size * n_channels; n++) {
            CONVERT8(sampptr[0][n], n, signconv);
        }
    }

    return size;
}

long duh_render_float(DUH_SIGRENDERER *sigrenderer, sample_t ***sig_samples,
                      long *sig_samples_size, int bits, float volume,
                      float delta, long size, void *sptr) {
    long n;

    sample_t **sampptr;

    int n_channels;

    ASSERT(bits == 32 || bits == 64);
    ASSERT(sptr);
    ASSERT(sig_samples);
    ASSERT(sig_samples_size);

    if (!sigrenderer)
        return 0;

    n_channels = duh_sigrenderer_get_n_channels(sigrenderer);

    ASSERT(n_channels > 0);
    /* This restriction will be removed when need be. At the moment, tightly
     * optimised loops exist for exactly one or two channels.
     */
    ASSERT(n_channels <= 2);

    if ((*sig_samples == NULL) || (*sig_samples_size != size)) {
        destroy_sample_buffer(*sig_samples);
        *sig_samples = allocate_sample_buffer(n_channels, size);
        *sig_samples_size = size;
    }
    sampptr = *sig_samples;

    if (!sampptr)
        return 0;

    dumb_silence(sampptr[0], n_channels * size);

    size = duh_sigrenderer_generate_samples(sigrenderer, volume, delta, size,
                                            sampptr);

    if (bits == 64) {
        for (n = 0; n < size * n_channels; n++) {
            CONVERT64F(sampptr[0][n], n);
        }
    } else if (bits == 32) {
        for (n = 0; n < size * n_channels; n++) {
            CONVERT32F(sampptr[0][n], n);
        }
    }

    return size;
}
