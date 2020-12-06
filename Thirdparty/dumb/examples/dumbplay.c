#include <argtable2.h>
#include <dumb.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// How many samples we should handle per callback call
#define SAMPLES 8192

// How many chars wide the progress bar is
#define PROGRESSBAR_LENGTH 25

typedef struct {
    // Library contexts
    DUH_SIGRENDERER *renderer;
    DUH *src;
    SDL_AudioDeviceID dev;
    sample_t **sig_samples;
    long sig_samples_size;

    // Runtime vars
    float delta;
    int spos;   // Samples read
    int ssize;  // Total samples available
    int sbytes; // bytes per sample
    bool ended;

    // Config switches
    int bits;
    int freq;
    int quality;
    int n_channels;
    bool no_progress;
    float volume;
    const char *input;
} streamer_t;

// Set to true if SIGTERM or SIGINT is caught
static bool stop_signal = false;

// Simple signal handler function
static void sig_fn(int signo) { stop_signal = true; }

// This is called from SDL2, and should read data from a source and hand it over
// to SDL2 via the stream buffer.
void stream_audio(void *userdata, Uint8 *stream, int len) {
    streamer_t *streamer = (streamer_t *)userdata;

    // Read samples from libdumb save them to the SDL buffer. Note that we are
    // reading SAMPLES, not bytes!
    int r_samples = len / streamer->sbytes;
    int got =
        duh_render_int(streamer->renderer, &streamer->sig_samples,
                       &streamer->sig_samples_size, streamer->bits, 0,
                       streamer->volume, streamer->delta, r_samples, stream);
    if (got == 0) {
        streamer->ended = true;
    }

    // Get current position from libdumb for the playback display. If we get
    // position that is 0, it probably means that the song ended and
    // duh_sigrenderer_get_position points to the start of the file.
    streamer->spos = duh_sigrenderer_get_position(streamer->renderer);
    if (streamer->spos == 0) {
        streamer->spos = streamer->ssize;
    }
}

// Simply formats timestamp to string
void format_ms(int ticks) {
    int total_seconds = ticks / 1000;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    // Calculate hours, minutes and seconds
    if (total_seconds > 3600) {
        hours = total_seconds / 3600;
        total_seconds = total_seconds % 3600;
    }
    if (total_seconds > 60) {
        minutes = total_seconds / 60;
        total_seconds = total_seconds % 60;
    }
    seconds = total_seconds;

    // If necessary, show hours. Otherwise only minutes and seconds.
    if (hours > 0) {
        printf("%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        printf("%02d:%02d", minutes, seconds);
    }
}

// Shows progressbar and time played
void show_progress(int width, float progress, int ticks) {
    int d = progress * width;
    printf("%3d%% [", (int)(progress * 100));
    for (int x = 0; x < d; x++) {
        printf("=");
    }
    for (int x = 0; x < (width - d); x++) {
        printf(" ");
    }
    printf("] ");
    format_ms(ticks);
    printf("\r");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    int retcode = 1;
    int nerrors = 0;
    streamer_t streamer;
    memset(&streamer, 0, sizeof(streamer_t));

    // Signal handlers
    signal(SIGINT, sig_fn);
    signal(SIGTERM, sig_fn);

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    // Defaults
    streamer.freq = 44100;
    streamer.n_channels = 2;
    streamer.bits = 16;
    streamer.volume = 1.0f;
    streamer.quality = DUMB_RQ_CUBIC;

    // commandline argument parser options
    struct arg_lit *arg_help =
        arg_lit0("h", "help", "print this help and exits");
    struct arg_dbl *arg_volume =
        arg_dbl0("v", "volume", "<volume",
                 "sets the output volume (-8.0 to +8.0, default 1.0)");
    struct arg_int *arg_samplerate = arg_int0(
        "s", "samplerate", "<freq>", "sets the sampling rate (default 44100)");
    struct arg_int *arg_quality = arg_int0(
        "r", "quality", "<quality>", "specify the resampling quality to use");
    struct arg_lit *arg_mono =
        arg_lit0("m", "mono", "generate mono output instead of stereo");
    struct arg_lit *arg_eight =
        arg_lit0("8", "eight", "generate 8-bit instead of 16-bit");
    struct arg_lit *arg_noprogress =
        arg_lit0("n", "noprogress", "hide progress bar");
    struct arg_file *arg_output =
        arg_file0("o", "output", "<file>", "output file");
    struct arg_file *arg_input =
        arg_file1(NULL, NULL, "<file>", "input module file");
    struct arg_end *arg_fend = arg_end(20);
    void *argtable[] = {arg_help,       arg_input,      arg_volume,
                        arg_samplerate, arg_quality,    arg_mono,
                        arg_eight,      arg_noprogress, arg_fend};
    const char *progname = "dumbplay";

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

    // Handle libargtable errors
    if (nerrors > 0) {
        arg_print_errors(stderr, arg_fend, progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Handle the switch options
    streamer.input = arg_input->filename[0];
    if (arg_eight->count > 0) {
        streamer.bits = 8;
    }
    if (arg_mono->count > 0) {
        streamer.n_channels = 1;
    }
    if (arg_noprogress->count > 0) {
        streamer.no_progress = true;
    }

    if (arg_volume->count > 0) {
        streamer.volume = arg_volume->dval[0];
        if (streamer.volume < -8.0f || streamer.volume > 8.0f) {
            fprintf(stderr, "Volume must be between -8.0f and 8.0f.\n");
            goto exit_0;
        }
    }

    if (arg_samplerate->count > 0) {
        streamer.freq = arg_samplerate->ival[0];
        if (streamer.freq < 1 || streamer.freq > 96000) {
            fprintf(stderr, "Sampling rate must be between 1 and 96000.\n");
            goto exit_0;
        }
    }

    if (arg_quality->count > 0) {
        streamer.quality = arg_quality->ival[0];
        if (streamer.quality < 0 || streamer.quality >= DUMB_RQ_N_LEVELS) {
            fprintf(stderr, "Quality must be between %d and %d.\n", 0,
                    DUMB_RQ_N_LEVELS - 1);
            goto exit_0;
        }
    }

    // Load source file.
    dumb_register_stdfiles();
    streamer.src = dumb_load_any(streamer.input, 0, 0);
    if (!streamer.src) {
        fprintf(stderr, "Unable to load file %s for playback!\n",
                streamer.input);
        goto exit_0;
    }

    // Set up playback
    streamer.renderer =
        duh_start_sigrenderer(streamer.src, 0, streamer.n_channels, 0);
    streamer.delta = 65536.0f / streamer.freq;
    streamer.sbytes = (streamer.bits / 8) * streamer.n_channels;
    streamer.ssize = duh_get_length(streamer.src);

    // Stop producing samples on module end
    DUMB_IT_SIGRENDERER *itsr = duh_get_it_sigrenderer(streamer.renderer);
    dumb_it_set_loop_callback(itsr, &dumb_it_callback_terminate, NULL);
    dumb_it_set_xm_speed_zero_callback(itsr, &dumb_it_callback_terminate, NULL);
    dumb_it_set_resampling_quality(itsr, streamer.quality);

    // Set up the SDL2 format we want for playback.
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = streamer.freq;
    want.format = (streamer.bits == 16) ? AUDIO_S16 : AUDIO_S8;
    want.channels = streamer.n_channels;
    want.samples = SAMPLES;
    want.callback = stream_audio;
    want.userdata = &streamer;

    // Find SDL2 audio device, and request the format we just set up.
    // SDL2 will tell us what we got in the "have" struct.
    SDL_AudioSpec have;
    streamer.dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (streamer.dev == 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        goto exit_1;
    }

    // Make sure we got the format we wanted. If not, stop here.
    if (have.format != want.format) {
        fprintf(stderr, "Could not get correct playback format.\n");
        goto exit_2;
    }

    // Play file
    SDL_PauseAudioDevice(streamer.dev, 0);

    // Show initial state of the progress bar (if it is enabled)
    int time_start = SDL_GetTicks();
    float seek = 0.0f;
    int ms_played = 0;
    if (!streamer.no_progress) {
        show_progress(PROGRESSBAR_LENGTH, seek, ms_played);
    }

    // Loop while dumb is still giving data. Update progressbar if enabled.
    while (!stop_signal && !streamer.ended) {
        if (!streamer.no_progress) {
            seek = ((float)streamer.spos) / ((float)streamer.ssize);
            ms_played = SDL_GetTicks() - time_start;
            show_progress(PROGRESSBAR_LENGTH, seek, ms_played);
        }
        SDL_Delay(100);
    }

    // We made it this far without crashing, so let's just exit with no error :)
    retcode = 0;

    // Free up resources and exit.
    if (streamer.sig_samples) {
        destroy_sample_buffer(streamer.sig_samples);
    }

exit_2:
    SDL_CloseAudioDevice(streamer.dev);

exit_1:
    if (streamer.renderer) {
        duh_end_sigrenderer(streamer.renderer);
    }
    if (streamer.src) {
        unload_duh(streamer.src);
    }

exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    SDL_Quit();
    return retcode;
}
