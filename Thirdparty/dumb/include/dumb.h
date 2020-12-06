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
 * dumb.h - The user header file for DUMB.            / / \  \
 *                                                   | <  /   \_
 * Include this file in any of your files in         |  \/ /\   /
 * which you wish to use the DUMB functions           \_  /  > /
 * and variables.                                       | \ / /
 *                                                      |  ' /
 * Allegro users, you will probably want aldumb.h.       \__/
 */

#ifndef DUMB_H
#define DUMB_H

#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * When you bump major, minor, or patch, bump both the number and the string.
 * When you bump major or minor version, bump them in CMakeLists.txt, too.
 */
#define DUMB_MAJOR_VERSION 2
#define DUMB_MINOR_VERSION 0
#define DUMB_REVISION_VERSION 2
#define DUMB_VERSION_STR "2.0.2"

#define DUMB_VERSION                                                           \
    (DUMB_MAJOR_VERSION * 10000 + DUMB_MINOR_VERSION * 100 +                   \
     DUMB_REVISION_VERSION)
#define DUMB_NAME "DUMB v" DUMB_VERSION_STR

#ifdef DEBUGMODE

#ifndef ASSERT
#include <assert.h>
#define ASSERT(n) assert(n)
#endif
#ifndef TRACE
// it would be nice if this did actually trace ...
#define TRACE 1 ? (void)0 : (void)printf
#endif

#else

#ifndef ASSERT
#define ASSERT(n)
#endif
#ifndef TRACE
#define TRACE 1 ? (void)0 : (void)printf
#endif

#endif // DEBUGMODE

#define DUMB_ID(a, b, c, d)                                                    \
    (((unsigned int)(a) << 24) | ((unsigned int)(b) << 16) |                   \
     ((unsigned int)(c) << 8) | ((unsigned int)(d)))

#if __GNUC__ * 100 + __GNUC_MINOR__ >= 301 /* GCC 3.1+ */
#ifndef DUMB_DECLARE_DEPRECATED
#define DUMB_DECLARE_DEPRECATED
#endif
#define DUMB_DEPRECATED __attribute__((__deprecated__))
#else
#define DUMB_DEPRECATED
#endif

/* Basic Sample Type. Normal range is -0x800000 to 0x7FFFFF. */

typedef int sample_t;

/* Library Clean-up Management */

int dumb_atexit(void (*proc)(void));

void dumb_exit(void);

/* File Input Functions */
#ifdef DUMB_OFF_T_CUSTOM
typedef DUMB_OFF_T_CUSTOM dumb_off_t;
#elif defined _MSC_VER || defined __WATCOMC__
typedef __int64 dumb_off_t;
#elif defined __DJGPP__ || defined __MINGW32__
/* MingW-W64 does not have off64_t and supports _FILE_OFFSET_BITS,
 * DJGPP and old MinGW do have off64_t, but don't support _FILE_OFFSET_BITS.
 */
#include <sys/types.h>
#if defined(__MINGW64_VERSION_MAJOR)
typedef off_t dumb_off_t;
#else
typedef off64_t dumb_off_t;
#endif
#elif defined __ANDROID__
/* Android NDK does not support _FILE_OFFSET_BITS before API level 24
 * and off_t is is always 32-bit regardless of _FILE_OFFSET_BITS.
 */
#include <sys/types.h>
typedef off64_t dumb_off_t;
#elif defined __EMSCRIPTEN__
#include <sys/types.h>
typedef int64_t dumb_off_t;
#elif _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 500
#include <sys/types.h>
typedef off_t dumb_off_t;
#else
typedef long long dumb_off_t;
#endif

/*
 * If the build fails here, it does so, because we need a 64-bit-type for
 * defining offsets. To fix this do either of the following:
 *
 * 1. Compile your code with -D_FILE_OFFSET_BITS=64, so that off_t is 64-bit
 *    (recommended, but make sure the rest of your code can handle it)
 * 2. Supply your own definition of a signed 64-bit integer
 *    such as off64_t or int64_t before including dumb.h as follows:
 *    #define DUMB_OFF_T_CUSTOM int64_t
 */
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) &&                \
    !defined __cplusplus
_Static_assert(sizeof(dumb_off_t) >= 8, "dumb: off_t must be 64bit");
#else
struct dumb_off_t_needs_to_be_at_least_8_bytes {
    unsigned int dumb_off_t_needs_to_be_at_least_8_bytes_
        : ((sizeof(dumb_off_t) >= 8) ? 1 : -42);
};
#endif

/*
 * ssize_t is defined in POSIX to hold either a size_t or an error.
 * We will use dumb_ssize_t on all platforms for (either size_t or error) in
 * all getnc-type functions. See DUMBFILE_SYSTEM.md for that function's spec.
 */
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T dumb_ssize_t;
#else
#include <sys/types.h>
typedef ssize_t dumb_ssize_t;
#endif

/*
 * DUMB provides an abstraction over files, to work with memory-mapped files,
 * files on disk, files read into memory by other libraries in their own
 * custom formats, ...
 *
 * Register your own file-handling functions as callbacks via this struct.
 * DUMB 2.0 doesn't use long anymore. The 64-bit dumb_*_t are defined above.
 *
 * See DUMBFILE_SYSTEM.md in project's root for a complete spec.
 */
typedef struct DUMBFILE_SYSTEM {
    /* open */
    /* Open filename. Returned file may be of any custom type. */
    void *(*open)(const char *filename);

    /* skip */
    /* Ignore the next n bytes in file f. Returns 0 on succes, -1 on error. */
    int (*skip)(void *f, dumb_off_t n);

    /* getc */
    /* Read the next byte. Returns byte as unsigned, or -1 on error. */
    int (*getc)(void *f);

    /* getnc */
    /* Read n bytes into buffer ptr. Returns number of bytes read or -1. */
    dumb_ssize_t (*getnc)(char *ptr, size_t n, void *f);

    /* close */
    /* Called when DUMB is done with the file. User code may do anything. */
    void (*close)(void *f);

    /* seek */
    /* Jump to offset in bytes from beginning. Returns 0 if OK, -1 on error. */
    int (*seek)(void *f, dumb_off_t offset);

    /* get_size */
    /* Returns the size in bytes of the file. */
    dumb_off_t (*get_size)(void *f);
} DUMBFILE_SYSTEM;

typedef struct DUMBFILE DUMBFILE;

void register_dumbfile_system(const DUMBFILE_SYSTEM *dfs);

DUMBFILE *dumbfile_open(const char *filename);
DUMBFILE *dumbfile_open_ex(void *file, const DUMBFILE_SYSTEM *dfs);

dumb_off_t dumbfile_pos(DUMBFILE *f);
int dumbfile_skip(DUMBFILE *f, dumb_off_t n);

#define DFS_SEEK_SET 0
#define DFS_SEEK_CUR 1
#define DFS_SEEK_END 2

int dumbfile_seek(DUMBFILE *f, dumb_off_t n, int origin);

dumb_off_t dumbfile_get_size(DUMBFILE *f);

int dumbfile_getc(DUMBFILE *f);

int dumbfile_igetw(DUMBFILE *f);
int dumbfile_mgetw(DUMBFILE *f);

long dumbfile_igetl(DUMBFILE *f);
long dumbfile_mgetl(DUMBFILE *f);

unsigned long dumbfile_cgetul(DUMBFILE *f);
signed long dumbfile_cgetsl(DUMBFILE *f);

dumb_ssize_t dumbfile_getnc(char *ptr, size_t n, DUMBFILE *f);

int dumbfile_error(DUMBFILE *f);
int dumbfile_close(DUMBFILE *f);

/* stdio File Input Module */

void dumb_register_stdfiles(void);

DUMBFILE *dumbfile_open_stdfile(FILE *p);

/* Memory File Input Module */

DUMBFILE *dumbfile_open_memory(const char *data, size_t size);

/* DUH Management */

typedef struct DUH DUH;

#define DUH_SIGNATURE DUMB_ID('D', 'U', 'H', '!')

void unload_duh(DUH *duh);

DUH *load_duh(const char *filename);
DUH *read_duh(DUMBFILE *f);

dumb_off_t duh_get_length(DUH *duh);

const char *duh_get_tag(DUH *duh, const char *key);
int duh_get_tag_iterator_size(DUH *duh);
int duh_get_tag_iterator_get(DUH *duh, const char **key, const char **tag,
                             int i);

/* Signal Rendering Functions */

typedef struct DUH_SIGRENDERER DUH_SIGRENDERER;

DUH_SIGRENDERER *duh_start_sigrenderer(DUH *duh, int sig, int n_channels,
                                       long pos);

typedef void (*DUH_SIGRENDERER_SAMPLE_ANALYSER_CALLBACK)(
    void *data, const sample_t *const *samples, int n_channels, long length);

void duh_sigrenderer_set_sample_analyser_callback(
    DUH_SIGRENDERER *sigrenderer,
    DUH_SIGRENDERER_SAMPLE_ANALYSER_CALLBACK callback, void *data);

int duh_sigrenderer_get_n_channels(DUH_SIGRENDERER *sigrenderer);
long duh_sigrenderer_get_position(DUH_SIGRENDERER *sigrenderer);

void duh_sigrenderer_set_sigparam(DUH_SIGRENDERER *sigrenderer,
                                  unsigned char id, long value);

long duh_sigrenderer_generate_samples(DUH_SIGRENDERER *sigrenderer,
                                      float volume, float delta, long size,
                                      sample_t **samples);

void duh_sigrenderer_get_current_sample(DUH_SIGRENDERER *sigrenderer,
                                        float volume, sample_t *samples);

void duh_end_sigrenderer(DUH_SIGRENDERER *sigrenderer);

/* DUH Rendering Functions */

/* For packed integers: 8, 16, 24-bit wide.
 * Intermediary buffer sig_samples must be freed with destroy_sample_buffer()
 * in the end of the rendering loop.
 */
long duh_render_int(DUH_SIGRENDERER *sigrenderer, sample_t ***sig_samples,
                    long *sig_samples_size, int bits, int unsign, float volume,
                    float delta, long size, void *sptr);

/* For floats: 32, 64-bit wide.
 * Intermediary buffer sig_samples must be freed with destroy_sample_buffer()
 * in the end of the rendering loop.
 */
long duh_render_float(DUH_SIGRENDERER *sigrenderer, sample_t ***sig_samples,
                      long *sig_samples_size, int bits, float volume,
                      float delta, long size, void *sptr);

#ifdef DUMB_DECLARE_DEPRECATED

/* DEPRECATED since 2.0.0. Please use duh_render_int or duh_render_float. */
long duh_render(DUH_SIGRENDERER *sigrenderer, int bits, int unsign,
                float volume, float delta, long size,
                void *sptr) DUMB_DEPRECATED;

#endif

/* Impulse Tracker Support */

extern int dumb_it_max_to_mix;

typedef struct DUMB_IT_SIGDATA DUMB_IT_SIGDATA;
typedef struct DUMB_IT_SIGRENDERER DUMB_IT_SIGRENDERER;

DUMB_IT_SIGDATA *duh_get_it_sigdata(DUH *duh);
DUH_SIGRENDERER *
duh_encapsulate_it_sigrenderer(DUMB_IT_SIGRENDERER *it_sigrenderer,
                               int n_channels, long pos);
DUMB_IT_SIGRENDERER *duh_get_it_sigrenderer(DUH_SIGRENDERER *sigrenderer);

int dumb_it_trim_silent_patterns(DUH *duh);

typedef int (*dumb_scan_callback)(void *, int, long);
int dumb_it_scan_for_playable_orders(DUMB_IT_SIGDATA *sigdata,
                                     dumb_scan_callback callback,
                                     void *callback_data);

DUH_SIGRENDERER *dumb_it_start_at_order(DUH *duh, int n_channels,
                                        int startorder);

enum {
    DUMB_IT_RAMP_NONE = 0,
    DUMB_IT_RAMP_ONOFF_ONLY = 1,
    DUMB_IT_RAMP_FULL = 2
};

void dumb_it_set_ramp_style(DUMB_IT_SIGRENDERER *sigrenderer, int ramp_style);

void dumb_it_set_loop_callback(DUMB_IT_SIGRENDERER *sigrenderer,
                               int (*callback)(void *data), void *data);
void dumb_it_set_xm_speed_zero_callback(DUMB_IT_SIGRENDERER *sigrenderer,
                                        int (*callback)(void *data),
                                        void *data);
void dumb_it_set_midi_callback(DUMB_IT_SIGRENDERER *sigrenderer,
                               int (*callback)(void *data, int channel,
                                               unsigned char midi_byte),
                               void *data);
void dumb_it_set_global_volume_zero_callback(DUMB_IT_SIGRENDERER *sigrenderer,
                                             int (*callback)(void *data),
                                             void *data);

int dumb_it_callback_terminate(void *data);
int dumb_it_callback_midi_block(void *data, int channel,
                                unsigned char midi_byte);

/* dumb_*_mod*: restrict_ - OR these together */
enum {
    DUMB_MOD_RESTRICT_NO_15_SAMPLE = (1 << 0), /* Don't load 15 sample files,
                                                  which have no identifying
                                                  signature */
    DUMB_MOD_RESTRICT_OLD_PATTERN_COUNT =
        (1 << 1) /* Use old pattern counting method */
};

DUH *dumb_load_it(const char *filename);
DUH *dumb_load_xm(const char *filename);
DUH *dumb_load_s3m(const char *filename);
DUH *dumb_load_stm(const char *filename);
DUH *dumb_load_mod(const char *filename, int restrict_);
DUH *dumb_load_ptm(const char *filename);
DUH *dumb_load_669(const char *filename);
DUH *dumb_load_psm(const char *filename, int subsong);
DUH *dumb_load_old_psm(const char *filename);
DUH *dumb_load_mtm(const char *filename);
DUH *dumb_load_riff(const char *filename);
DUH *dumb_load_asy(const char *filename);
DUH *dumb_load_amf(const char *filename);
DUH *dumb_load_okt(const char *filename);

DUH *dumb_read_it(DUMBFILE *f);
DUH *dumb_read_xm(DUMBFILE *f);
DUH *dumb_read_s3m(DUMBFILE *f);
DUH *dumb_read_stm(DUMBFILE *f);
DUH *dumb_read_mod(DUMBFILE *f, int restrict_);
DUH *dumb_read_ptm(DUMBFILE *f);
DUH *dumb_read_669(DUMBFILE *f);
DUH *dumb_read_psm(DUMBFILE *f, int subsong);
DUH *dumb_read_old_psm(DUMBFILE *f);
DUH *dumb_read_mtm(DUMBFILE *f);
DUH *dumb_read_riff(DUMBFILE *f);
DUH *dumb_read_asy(DUMBFILE *f);
DUH *dumb_read_amf(DUMBFILE *f);
DUH *dumb_read_okt(DUMBFILE *f);

DUH *dumb_load_it_quick(const char *filename);
DUH *dumb_load_xm_quick(const char *filename);
DUH *dumb_load_s3m_quick(const char *filename);
DUH *dumb_load_stm_quick(const char *filename);
DUH *dumb_load_mod_quick(const char *filename, int restrict_);
DUH *dumb_load_ptm_quick(const char *filename);
DUH *dumb_load_669_quick(const char *filename);
DUH *dumb_load_psm_quick(const char *filename, int subsong);
DUH *dumb_load_old_psm_quick(const char *filename);
DUH *dumb_load_mtm_quick(const char *filename);
DUH *dumb_load_riff_quick(const char *filename);
DUH *dumb_load_asy_quick(const char *filename);
DUH *dumb_load_amf_quick(const char *filename);
DUH *dumb_load_okt_quick(const char *filename);

DUH *dumb_read_it_quick(DUMBFILE *f);
DUH *dumb_read_xm_quick(DUMBFILE *f);
DUH *dumb_read_s3m_quick(DUMBFILE *f);
DUH *dumb_read_stm_quick(DUMBFILE *f);
DUH *dumb_read_mod_quick(DUMBFILE *f, int restrict_);
DUH *dumb_read_ptm_quick(DUMBFILE *f);
DUH *dumb_read_669_quick(DUMBFILE *f);
DUH *dumb_read_psm_quick(DUMBFILE *f, int subsong);
DUH *dumb_read_old_psm_quick(DUMBFILE *f);
DUH *dumb_read_mtm_quick(DUMBFILE *f);
DUH *dumb_read_riff_quick(DUMBFILE *f);
DUH *dumb_read_asy_quick(DUMBFILE *f);
DUH *dumb_read_amf_quick(DUMBFILE *f);
DUH *dumb_read_okt_quick(DUMBFILE *f);

DUH *dumb_read_any_quick(DUMBFILE *f, int restrict_, int subsong);
DUH *dumb_read_any(DUMBFILE *f, int restrict_, int subsong);

DUH *dumb_load_any_quick(const char *filename, int restrict_, int subsong);
DUH *dumb_load_any(const char *filename, int restrict_, int subsong);

long dumb_it_build_checkpoints(DUMB_IT_SIGDATA *sigdata, int startorder);
void dumb_it_do_initial_runthrough(DUH *duh);

int dumb_get_psm_subsong_count(DUMBFILE *f);

const unsigned char *dumb_it_sd_get_song_message(DUMB_IT_SIGDATA *sd);

int dumb_it_sd_get_n_orders(DUMB_IT_SIGDATA *sd);
int dumb_it_sd_get_n_samples(DUMB_IT_SIGDATA *sd);
int dumb_it_sd_get_n_instruments(DUMB_IT_SIGDATA *sd);

const unsigned char *dumb_it_sd_get_sample_name(DUMB_IT_SIGDATA *sd, int i);
const unsigned char *dumb_it_sd_get_sample_filename(DUMB_IT_SIGDATA *sd, int i);
const unsigned char *dumb_it_sd_get_instrument_name(DUMB_IT_SIGDATA *sd, int i);
const unsigned char *dumb_it_sd_get_instrument_filename(DUMB_IT_SIGDATA *sd,
                                                        int i);

int dumb_it_sd_get_initial_global_volume(DUMB_IT_SIGDATA *sd);
void dumb_it_sd_set_initial_global_volume(DUMB_IT_SIGDATA *sd, int gv);

int dumb_it_sd_get_mixing_volume(DUMB_IT_SIGDATA *sd);
void dumb_it_sd_set_mixing_volume(DUMB_IT_SIGDATA *sd, int mv);

int dumb_it_sd_get_initial_speed(DUMB_IT_SIGDATA *sd);
void dumb_it_sd_set_initial_speed(DUMB_IT_SIGDATA *sd, int speed);

int dumb_it_sd_get_initial_tempo(DUMB_IT_SIGDATA *sd);
void dumb_it_sd_set_initial_tempo(DUMB_IT_SIGDATA *sd, int tempo);

int dumb_it_sd_get_initial_channel_volume(DUMB_IT_SIGDATA *sd, int channel);
void dumb_it_sd_set_initial_channel_volume(DUMB_IT_SIGDATA *sd, int channel,
                                           int volume);

int dumb_it_sr_get_current_order(DUMB_IT_SIGRENDERER *sr);
int dumb_it_sr_get_current_row(DUMB_IT_SIGRENDERER *sr);

int dumb_it_sr_get_global_volume(DUMB_IT_SIGRENDERER *sr);
void dumb_it_sr_set_global_volume(DUMB_IT_SIGRENDERER *sr, int gv);

int dumb_it_sr_get_tempo(DUMB_IT_SIGRENDERER *sr);
void dumb_it_sr_set_tempo(DUMB_IT_SIGRENDERER *sr, int tempo);

int dumb_it_sr_get_speed(DUMB_IT_SIGRENDERER *sr);
void dumb_it_sr_set_speed(DUMB_IT_SIGRENDERER *sr, int speed);

#define DUMB_IT_N_CHANNELS 64
#define DUMB_IT_N_NNA_CHANNELS 192
#define DUMB_IT_TOTAL_CHANNELS (DUMB_IT_N_CHANNELS + DUMB_IT_N_NNA_CHANNELS)

/* Channels passed to any of these functions are 0-based */
int dumb_it_sr_get_channel_volume(DUMB_IT_SIGRENDERER *sr, int channel);
void dumb_it_sr_set_channel_volume(DUMB_IT_SIGRENDERER *sr, int channel,
                                   int volume);

int dumb_it_sr_get_channel_muted(DUMB_IT_SIGRENDERER *sr, int channel);
void dumb_it_sr_set_channel_muted(DUMB_IT_SIGRENDERER *sr, int channel,
                                  int muted);

typedef struct DUMB_IT_CHANNEL_STATE DUMB_IT_CHANNEL_STATE;

struct DUMB_IT_CHANNEL_STATE {
    int channel;  /* 0-based; meaningful for NNA channels */
    int sample;   /* 1-based; 0 if nothing playing, then other fields undef */
    int freq;     /* in Hz */
    float volume; /* 1.0 maximum; affected by ALL factors, inc. mixing vol */
    unsigned char pan;  /* 0-64, 100 for surround */
    signed char subpan; /* use (pan + subpan/256.0f) or ((pan<<8)+subpan) */
    unsigned char filter_cutoff;    /* 0-127    cutoff=127 AND resonance=0 */
    unsigned char filter_subcutoff; /* 0-255      -> no filters (subcutoff */
    unsigned char filter_resonance; /* 0-127        always 0 in this case) */
    /* subcutoff only changes from zero if filter envelopes are in use. The
     * calculation (filter_cutoff + filter_subcutoff/256.0f) gives a more
     * accurate filter cutoff measurement as a float. It would often be more
     * useful to use a scaled int such as ((cutoff<<8) + subcutoff).
     */
};

/* Values of 64 or more will access NNA channels here. */
void dumb_it_sr_get_channel_state(DUMB_IT_SIGRENDERER *sr, int channel,
                                  DUMB_IT_CHANNEL_STATE *state);

/* Signal Design Helper Values */

/* Use pow(DUMB_SEMITONE_BASE, n) to get the 'delta' value to transpose up by
 * n semitones. To transpose down, use negative n.
 */
#define DUMB_SEMITONE_BASE 1.059463094359295309843105314939748495817

/* Use pow(DUMB_QUARTERTONE_BASE, n) to get the 'delta' value to transpose up
 * by n quartertones. To transpose down, use negative n.
 */
#define DUMB_QUARTERTONE_BASE 1.029302236643492074463779317738953977823

/* Use pow(DUMB_PITCH_BASE, n) to get the 'delta' value to transpose up by n
 * units. In this case, 256 units represent one semitone; 3072 units
 * represent one octave. These units are used by the sequence signal (SEQU).
 */
#define DUMB_PITCH_BASE 1.000225659305069791926712241547647863626

/* Signal Design Function Types */

typedef void sigdata_t;
typedef void sigrenderer_t;

typedef sigdata_t *(*DUH_LOAD_SIGDATA)(DUH *duh, DUMBFILE *file);

typedef sigrenderer_t *(*DUH_START_SIGRENDERER)(DUH *duh, sigdata_t *sigdata,
                                                int n_channels, long pos);

typedef void (*DUH_SIGRENDERER_SET_SIGPARAM)(sigrenderer_t *sigrenderer,
                                             unsigned char id, long value);

typedef long (*DUH_SIGRENDERER_GENERATE_SAMPLES)(sigrenderer_t *sigrenderer,
                                                 float volume, float delta,
                                                 long size, sample_t **samples);

typedef void (*DUH_SIGRENDERER_GET_CURRENT_SAMPLE)(sigrenderer_t *sigrenderer,
                                                   float volume,
                                                   sample_t *samples);

typedef long (*DUH_SIGRENDERER_GET_POSITION)(sigrenderer_t *sigrenderer);

typedef void (*DUH_END_SIGRENDERER)(sigrenderer_t *sigrenderer);

typedef void (*DUH_UNLOAD_SIGDATA)(sigdata_t *sigdata);

/* Signal Design Function Registration */

typedef struct DUH_SIGTYPE_DESC {
    long type;
    DUH_LOAD_SIGDATA load_sigdata;
    DUH_START_SIGRENDERER start_sigrenderer;
    DUH_SIGRENDERER_SET_SIGPARAM sigrenderer_set_sigparam;
    DUH_SIGRENDERER_GENERATE_SAMPLES sigrenderer_generate_samples;
    DUH_SIGRENDERER_GET_CURRENT_SAMPLE sigrenderer_get_current_sample;
    DUH_SIGRENDERER_GET_POSITION sigrenderer_get_position;
    DUH_END_SIGRENDERER end_sigrenderer;
    DUH_UNLOAD_SIGDATA unload_sigdata;
} DUH_SIGTYPE_DESC;

void dumb_register_sigtype(DUH_SIGTYPE_DESC *desc);

int duh_add_signal(DUH *duh, DUH_SIGTYPE_DESC *desc, sigdata_t *sigdata);

// Decide where to put these functions; new heading?

sigdata_t *duh_get_raw_sigdata(DUH *duh, int sig, long type);

DUH_SIGRENDERER *duh_encapsulate_raw_sigrenderer(sigrenderer_t *vsigrenderer,
                                                 DUH_SIGTYPE_DESC *desc,
                                                 int n_channels, long pos);
sigrenderer_t *duh_get_raw_sigrenderer(DUH_SIGRENDERER *sigrenderer, long type);

/* Standard Signal Types */

// void dumb_register_sigtype_sample(void);

/* Sample Buffer Allocation Helpers */

sample_t **allocate_sample_buffer(int n_channels, long length);
void destroy_sample_buffer(sample_t **samples);

/* Silencing Helper */

void dumb_silence(sample_t *samples, long length);

/* Click Removal Helpers */

typedef struct DUMB_CLICK_REMOVER DUMB_CLICK_REMOVER;

DUMB_CLICK_REMOVER *dumb_create_click_remover(void);
void dumb_record_click(DUMB_CLICK_REMOVER *cr, long pos, sample_t step);
void dumb_remove_clicks(DUMB_CLICK_REMOVER *cr, sample_t *samples, long length,
                        int step, float halflife);
sample_t dumb_click_remover_get_offset(DUMB_CLICK_REMOVER *cr);
void dumb_destroy_click_remover(DUMB_CLICK_REMOVER *cr);

DUMB_CLICK_REMOVER **dumb_create_click_remover_array(int n);
void dumb_record_click_array(int n, DUMB_CLICK_REMOVER **cr, long pos,
                             sample_t *step);
void dumb_record_click_negative_array(int n, DUMB_CLICK_REMOVER **cr, long pos,
                                      sample_t *step);
void dumb_remove_clicks_array(int n, DUMB_CLICK_REMOVER **cr,
                              sample_t **samples, long length, float halflife);
void dumb_click_remover_get_offset_array(int n, DUMB_CLICK_REMOVER **cr,
                                         sample_t *offset);
void dumb_destroy_click_remover_array(int n, DUMB_CLICK_REMOVER **cr);

/* Resampling Helpers */

#define DUMB_RQ_ALIASING 0
#define DUMB_RQ_BLEP 1
#define DUMB_RQ_LINEAR 2
#define DUMB_RQ_BLAM 3
#define DUMB_RQ_CUBIC 4
#define DUMB_RQ_FIR 5
#define DUMB_RQ_N_LEVELS 6

extern int dumb_resampling_quality; /* This specifies the default */
void dumb_it_set_resampling_quality(DUMB_IT_SIGRENDERER *sigrenderer,
                                    int quality); /* This overrides it */

typedef struct DUMB_RESAMPLER DUMB_RESAMPLER;

typedef struct DUMB_VOLUME_RAMP_INFO DUMB_VOLUME_RAMP_INFO;

typedef void (*DUMB_RESAMPLE_PICKUP)(DUMB_RESAMPLER *resampler, void *data);

struct DUMB_RESAMPLER {
    void *src;
    long pos;
    int subpos;
    long start, end;
    int dir;
    DUMB_RESAMPLE_PICKUP pickup;
    void *pickup_data;
    int quality;
    /* Everything below this point is internal: do not use. */
    union {
        sample_t x24[3 * 2];
        short x16[3 * 2];
        signed char x8[3 * 2];
    } x;
    int overshot;
    double fir_resampler_ratio;
    void *fir_resampler[2];
};

struct DUMB_VOLUME_RAMP_INFO {
    float volume;
    float delta;
    float target;
    float mix;
    unsigned char declick_stage;
};

void dumb_reset_resampler(DUMB_RESAMPLER *resampler, sample_t *src,
                          int src_channels, long pos, long start, long end,
                          int quality);
DUMB_RESAMPLER *dumb_start_resampler(sample_t *src, int src_channels, long pos,
                                     long start, long end, int quality);
long dumb_resample_1_1(DUMB_RESAMPLER *resampler, sample_t *dst, long dst_size,
                       DUMB_VOLUME_RAMP_INFO *volume, float delta);
long dumb_resample_1_2(DUMB_RESAMPLER *resampler, sample_t *dst, long dst_size,
                       DUMB_VOLUME_RAMP_INFO *volume_left,
                       DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_2_1(DUMB_RESAMPLER *resampler, sample_t *dst, long dst_size,
                       DUMB_VOLUME_RAMP_INFO *volume_left,
                       DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_2_2(DUMB_RESAMPLER *resampler, sample_t *dst, long dst_size,
                       DUMB_VOLUME_RAMP_INFO *volume_left,
                       DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
void dumb_resample_get_current_sample_1_1(DUMB_RESAMPLER *resampler,
                                          DUMB_VOLUME_RAMP_INFO *volume,
                                          sample_t *dst);
void dumb_resample_get_current_sample_1_2(DUMB_RESAMPLER *resampler,
                                          DUMB_VOLUME_RAMP_INFO *volume_left,
                                          DUMB_VOLUME_RAMP_INFO *volume_right,
                                          sample_t *dst);
void dumb_resample_get_current_sample_2_1(DUMB_RESAMPLER *resampler,
                                          DUMB_VOLUME_RAMP_INFO *volume_left,
                                          DUMB_VOLUME_RAMP_INFO *volume_right,
                                          sample_t *dst);
void dumb_resample_get_current_sample_2_2(DUMB_RESAMPLER *resampler,
                                          DUMB_VOLUME_RAMP_INFO *volume_left,
                                          DUMB_VOLUME_RAMP_INFO *volume_right,
                                          sample_t *dst);
void dumb_end_resampler(DUMB_RESAMPLER *resampler);

void dumb_reset_resampler_16(DUMB_RESAMPLER *resampler, short *src,
                             int src_channels, long pos, long start, long end,
                             int quality);
DUMB_RESAMPLER *dumb_start_resampler_16(short *src, int src_channels, long pos,
                                        long start, long end, int quality);
long dumb_resample_16_1_1(DUMB_RESAMPLER *resampler, sample_t *dst,
                          long dst_size, DUMB_VOLUME_RAMP_INFO *volume,
                          float delta);
long dumb_resample_16_1_2(DUMB_RESAMPLER *resampler, sample_t *dst,
                          long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                          DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_16_2_1(DUMB_RESAMPLER *resampler, sample_t *dst,
                          long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                          DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_16_2_2(DUMB_RESAMPLER *resampler, sample_t *dst,
                          long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                          DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
void dumb_resample_get_current_sample_16_1_1(DUMB_RESAMPLER *resampler,
                                             DUMB_VOLUME_RAMP_INFO *volume,
                                             sample_t *dst);
void dumb_resample_get_current_sample_16_1_2(
    DUMB_RESAMPLER *resampler, DUMB_VOLUME_RAMP_INFO *volume_left,
    DUMB_VOLUME_RAMP_INFO *volume_right, sample_t *dst);
void dumb_resample_get_current_sample_16_2_1(
    DUMB_RESAMPLER *resampler, DUMB_VOLUME_RAMP_INFO *volume_left,
    DUMB_VOLUME_RAMP_INFO *volume_right, sample_t *dst);
void dumb_resample_get_current_sample_16_2_2(
    DUMB_RESAMPLER *resampler, DUMB_VOLUME_RAMP_INFO *volume_left,
    DUMB_VOLUME_RAMP_INFO *volume_right, sample_t *dst);
void dumb_end_resampler_16(DUMB_RESAMPLER *resampler);

void dumb_reset_resampler_8(DUMB_RESAMPLER *resampler, signed char *src,
                            int src_channels, long pos, long start, long end,
                            int quality);
DUMB_RESAMPLER *dumb_start_resampler_8(signed char *src, int src_channels,
                                       long pos, long start, long end,
                                       int quality);
long dumb_resample_8_1_1(DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume,
                         float delta);
long dumb_resample_8_1_2(DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                         DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_8_2_1(DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                         DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_8_2_2(DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                         DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
void dumb_resample_get_current_sample_8_1_1(DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume,
                                            sample_t *dst);
void dumb_resample_get_current_sample_8_1_2(DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume_left,
                                            DUMB_VOLUME_RAMP_INFO *volume_right,
                                            sample_t *dst);
void dumb_resample_get_current_sample_8_2_1(DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume_left,
                                            DUMB_VOLUME_RAMP_INFO *volume_right,
                                            sample_t *dst);
void dumb_resample_get_current_sample_8_2_2(DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume_left,
                                            DUMB_VOLUME_RAMP_INFO *volume_right,
                                            sample_t *dst);
void dumb_end_resampler_8(DUMB_RESAMPLER *resampler);

void dumb_reset_resampler_n(int n, DUMB_RESAMPLER *resampler, void *src,
                            int src_channels, long pos, long start, long end,
                            int quality);
DUMB_RESAMPLER *dumb_start_resampler_n(int n, void *src, int src_channels,
                                       long pos, long start, long end,
                                       int quality);
long dumb_resample_n_1_1(int n, DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume,
                         float delta);
long dumb_resample_n_1_2(int n, DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                         DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_n_2_1(int n, DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                         DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
long dumb_resample_n_2_2(int n, DUMB_RESAMPLER *resampler, sample_t *dst,
                         long dst_size, DUMB_VOLUME_RAMP_INFO *volume_left,
                         DUMB_VOLUME_RAMP_INFO *volume_right, float delta);
void dumb_resample_get_current_sample_n_1_1(int n, DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume,
                                            sample_t *dst);
void dumb_resample_get_current_sample_n_1_2(int n, DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume_left,
                                            DUMB_VOLUME_RAMP_INFO *volume_right,
                                            sample_t *dst);
void dumb_resample_get_current_sample_n_2_1(int n, DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume_left,
                                            DUMB_VOLUME_RAMP_INFO *volume_right,
                                            sample_t *dst);
void dumb_resample_get_current_sample_n_2_2(int n, DUMB_RESAMPLER *resampler,
                                            DUMB_VOLUME_RAMP_INFO *volume_left,
                                            DUMB_VOLUME_RAMP_INFO *volume_right,
                                            sample_t *dst);
void dumb_end_resampler_n(int n, DUMB_RESAMPLER *resampler);

/* This sets the default panning separation for hard panned formats,
   or for formats with default panning information. This must be set
   before using any readers or loaders, and is not really thread safe. */

extern int dumb_it_default_panning_separation; /* in percent, default 25 */

/* DUH Construction */

DUH *make_duh(dumb_off_t length, int n_tags, const char *const tag[][2],
              int n_signals, DUH_SIGTYPE_DESC *desc[], sigdata_t *sigdata[]);

void duh_set_length(DUH *duh, dumb_off_t length);

#ifdef __cplusplus
}
#endif

#endif /* DUMB_H */
