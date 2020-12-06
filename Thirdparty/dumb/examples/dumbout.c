#include <argtable2.h>
#include <dumb.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

static const int endian_test = 1;
#define is_bigendian() ((*(char *)&endian_test) == 0)

enum ENDIANNESS { DUMB_LITTLE_ENDIAN = 0, DUMB_BIG_ENDIAN };

typedef struct {
    DUH_SIGRENDERER *renderer;
    DUH *src;
    sample_t **sig_samples;
    long sig_samples_size;
    FILE *dst;
    float delta;
    int bufsize;
    bool is_stdout;
} streamer_t;

typedef struct {
    int bits;
    int endianness;
    int is_unsigned;
    int freq;
    int quality;
    int n_channels;
    float volume;
    float delay;
    const char *input;
    char *output;
} settings_t;

int main(int argc, char *argv[]) {
    int retcode = 1;
    int nerrors = 0;
    streamer_t streamer;
    settings_t settings;
    memset(&streamer, 0, sizeof(streamer_t));
    memset(&settings, 0, sizeof(settings_t));

    // Defaults
    settings.freq = 44100;
    settings.n_channels = 2;
    settings.bits = 16;
    settings.endianness = DUMB_LITTLE_ENDIAN;
    settings.is_unsigned = false;
    settings.volume = 1.0f;
    settings.delay = 0.0f;
    settings.quality = DUMB_RQ_CUBIC;

    // commandline argument parser options
    struct arg_lit *arg_help =
        arg_lit0("h", "help", "print this help and exits");
    struct arg_dbl *arg_delay = arg_dbl0(
        "d", "delay", "<delay>",
        "sets the initial delay in seconds (0.0 to 64.0, default 0.0)");
    struct arg_dbl *arg_volume =
        arg_dbl0("v", "volume", "<volume",
                 "sets the output volume (-8.0 to +8.0, default 1.0)");
    struct arg_int *arg_samplerate = arg_int0(
        "s", "samplerate", "<freq>", "sets the sampling rate (default 44100)");
    struct arg_int *arg_quality = arg_int0(
        "r", "quality", "<quality>", "specify the resampling quality to use");
    struct arg_lit *arg_mono =
        arg_lit0("m", "mono", "generate mono output instead of stereo");
    struct arg_lit *arg_bigendian = arg_lit0(
        "b", "bigendian", "generate big-endian data instead of little-endian");
    struct arg_lit *arg_eight =
        arg_lit0("8", "eight", "generate 8-bit instead of 16-bit");
    struct arg_lit *arg_unsigned =
        arg_lit0("u", "unsigned", "generate unsigned output instead of signed");
    struct arg_file *arg_output =
        arg_file0("o", "output", "<file>", "output file");
    struct arg_file *arg_input =
        arg_file1(NULL, NULL, "<file>", "input module file");
    struct arg_end *arg_fend = arg_end(20);
    void *argtable[] = {arg_help,      arg_input,      arg_output,   arg_delay,
                        arg_volume,    arg_samplerate, arg_quality,  arg_mono,
                        arg_bigendian, arg_eight,      arg_unsigned, arg_fend};
    const char *progname = "dumbout";

    // Make sure everything got allocated
    if (arg_nullcheck(argtable) != 0) {
        fprintf(stderr, "%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse inputs
    nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if (arg_help->count > 0) {
        fprintf(stderr, "Usage: %s", progname);
        arg_print_syntax(stderr, argtable, "\n");
        fprintf(stderr, "\nArguments:\n");
        arg_print_glossary(stderr, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle errors
    if (nerrors > 0) {
        arg_print_errors(stderr, arg_fend, progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Get input and output filenames
    settings.input = arg_input->filename[0];
    if (arg_output->count > 0) {
        settings.output = malloc(strlen(arg_output->filename[0]) + 1);
        strcpy(settings.output, arg_output->filename[0]);
    } else {
        settings.output = malloc(strlen(arg_output->basename[0]) + 5);
        sprintf(settings.output, "%s%s", arg_output->basename[0], ".pcm");
    }

    // Handle the switch options
    if (arg_bigendian->count > 0) {
        settings.endianness = DUMB_BIG_ENDIAN;
    }
    if (arg_eight->count > 0) {
        settings.bits = 8;
    }
    if (arg_unsigned->count > 0) {
        settings.is_unsigned = true;
    }
    if (arg_mono->count > 0) {
        settings.n_channels = 1;
    }

    if (arg_delay->count > 0) {
        settings.delay = arg_delay->dval[0];
        if (settings.delay < 0.0f || settings.delay >= 64.0f) {
            fprintf(stderr, "Initial delay must be between 0.0f and 64.0f.\n");
            goto exit_0;
        }
    }

    if (arg_volume->count > 0) {
        settings.volume = arg_volume->dval[0];
        if (settings.volume < -8.0f || settings.volume > 8.0f) {
            fprintf(stderr, "Volume must be between -8.0f and 8.0f.\n");
            goto exit_0;
        }
    }

    if (arg_samplerate->count > 0) {
        settings.freq = arg_samplerate->ival[0];
        if (settings.freq < 1 || settings.freq > 96000) {
            fprintf(stderr, "Sampling rate must be between 1 and 96000.\n");
            goto exit_0;
        }
    }

    if (arg_quality->count > 0) {
        settings.quality = arg_quality->ival[0];
        if (settings.quality < 0 || settings.quality >= DUMB_RQ_N_LEVELS) {
            fprintf(stderr, "Quality must be between %d and %d.\n", 0,
                    DUMB_RQ_N_LEVELS - 1);
            goto exit_0;
        }
    }

    // dumb settings stuff
    dumb_register_stdfiles();

    // Load source
    streamer.src = dumb_load_any(settings.input, 0, 0);
    if (!streamer.src) {
        fprintf(stderr, "Unable to load file %s for playback!\n",
                settings.input);
        goto exit_0;
    }

    // Set up playback
    streamer.renderer =
        duh_start_sigrenderer(streamer.src, 0, settings.n_channels, 0);
    streamer.delta = 65536.0f / settings.freq;
    streamer.bufsize = 4096 * (settings.bits / 8) * settings.n_channels;

    // Stop producing samples on module end
    DUMB_IT_SIGRENDERER *itsr = duh_get_it_sigrenderer(streamer.renderer);
    dumb_it_set_loop_callback(itsr, &dumb_it_callback_terminate, NULL);
    dumb_it_set_xm_speed_zero_callback(itsr, &dumb_it_callback_terminate, NULL);
    dumb_it_set_resampling_quality(itsr, settings.quality);

    // Open output
    if (strcmp(settings.output, "-") == 0) {
        streamer.dst = stdout;
        streamer.is_stdout = true;
    } else {
        streamer.dst = fopen(settings.output, "wb");
        streamer.is_stdout = false;
        if (!streamer.dst) {
            fprintf(stderr, "Could not open output file %s!", settings.output);
            goto exit_1;
        }
    }

    bool run = true;
    char *buffer = malloc(streamer.bufsize);
    int read_samples;
    int read_bytes;

    // If output endianness is different than machine endianness, and output is
    // 16 bits, reorder bytes.
    int switch_endianness =
        ((is_bigendian() && settings.endianness == DUMB_LITTLE_ENDIAN) ||
         (!is_bigendian() && settings.endianness == DUMB_BIG_ENDIAN));

    // Write the initial delay to the file if one was requested.
    long d = ((long)floor(settings.delay * settings.freq + 0.5f)) *
             settings.n_channels * (settings.bits / 8);
    if (d) {
        // Fill the buffer with silence. Remember to take into account
        // endianness
        if (settings.is_unsigned) {
            if (settings.bits == 16) {
                if (settings.endianness == DUMB_BIG_ENDIAN) {
                    // Unsigned 16bits big endian
                    for (int i = 0; i < streamer.bufsize; i += 2) {
                        buffer[i] = (char)0x80;
                        buffer[i + 1] = (char)0x00;
                    }
                } else {
                    // Unsigned 16bits little endian
                    for (int i = 0; i < streamer.bufsize; i += 2) {
                        buffer[i] = (char)0x00;
                        buffer[i + 1] = (char)0x80;
                    }
                }
            } else {
                // Unsigned 8 bits
                memset(buffer, 0x80, streamer.bufsize);
            }
        } else {
            // Signed
            memset(buffer, 0, streamer.bufsize);
        }

        while (d >= streamer.bufsize) {
            fwrite(buffer, 1, streamer.bufsize, streamer.dst);
            d -= streamer.bufsize;
        }
        if (d) {
            fwrite(buffer, 1, d, streamer.dst);
        }
    }

    // Loop until we have nothing to loop through. Dumb will stop giving out
    // bytes when the file is done.
    while (run) {
        read_samples =
            duh_render_int(streamer.renderer, &streamer.sig_samples,
                           &streamer.sig_samples_size, settings.bits,
                           (int)settings.is_unsigned, settings.volume,
                           streamer.delta, 4096, buffer);
        read_bytes = read_samples * (settings.bits / 8) * settings.n_channels;

        // switch endianness if required
        if (switch_endianness && settings.bits == 16) {
            char tmp;
            for (int i = 0; i < read_bytes / 2; i++) {
                tmp = buffer[i * 2 + 0];
                buffer[i * 2 + 0] = buffer[i * 2 + 1];
                buffer[i * 2 + 1] = tmp;
            }
        }

        // Write to output stream and flush if it happens to be stdout
        fwrite(buffer, 1, read_bytes, streamer.dst);
        if (streamer.is_stdout) {
            fflush(streamer.dst);
        }
        run = (read_samples > 0);
    }
    free(buffer);

    // We made it this far without crashing, so let's just exit with no error :)
    retcode = 0;

    if (!streamer.is_stdout && streamer.dst) {
        fclose(streamer.dst);
    }

    if (streamer.sig_samples) {
        destroy_sample_buffer(streamer.sig_samples);
    }

exit_1:
    if (streamer.renderer) {
        duh_end_sigrenderer(streamer.renderer);
    }
    if (streamer.src) {
        unload_duh(streamer.src);
    }

exit_0:
    free(settings.output);
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return retcode;
}
