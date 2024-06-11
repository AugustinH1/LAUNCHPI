#include "../include/sound.h"

static double phases[100];  // Store phases for up to 100 frequencies

// Mutex to synchronize phase updates
pthread_mutex_t phase_mutex = PTHREAD_MUTEX_INITIALIZER;


void init_audio(snd_pcm_t **handle) {
    int err;
    snd_pcm_hw_params_t *params;

    // Ouvrir le périphérique de lecture/écriture de PCM (Playback)
    if ((err = snd_pcm_open(handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Impossible d'ouvrir le périphérique audio : %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    // Allouer un ensemble de paramètres matériels
    snd_pcm_hw_params_malloc(&params);

    // Remplir les paramètres matériels par défaut
    snd_pcm_hw_params_any(*handle, params);

    // Définir le mode d'accès (interleaved)
    snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    // Définir le format des échantillons (Signed 16-bit Little Endian)
    snd_pcm_hw_params_set_format(*handle, params, SND_PCM_FORMAT_S16_LE);

    // Définir le taux d'échantillonnage
    snd_pcm_hw_params_set_rate(*handle, params, SAMPLE_RATE, 0);

    // Définir le nombre de canaux (mono)
    snd_pcm_hw_params_set_channels(*handle, params, 1);

    // Définir la taille du tampon
    snd_pcm_uframes_t buffer_size = 4096;
    snd_pcm_hw_params_set_buffer_size_near(*handle, params, &buffer_size);

    // Appliquer les paramètres matériels au périphérique PCM
    if ((err = snd_pcm_hw_params(*handle, params)) < 0) {
        fprintf(stderr, "Impossible de définir les paramètres matériels : %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    // Libérer les paramètres matériels
    snd_pcm_hw_params_free(params);
}

void play_tones(snd_pcm_t *handle, double *frequencies, int num_frequencies, double duration) {
    int err;
    int frames = (SAMPLE_RATE * duration); // Number of samples for the given duration
    short buffer[frames];
    double phase_increments[num_frequencies];
    int i, j;

    pthread_mutex_lock(&phase_mutex);

    // Initialize phase increments for each frequency
    for (i = 0; i < num_frequencies; i++) {
        phase_increments[i] = 2.0 * M_PI * frequencies[i] / SAMPLE_RATE;
    }

    // Generate the combined sine wave samples
    for (i = 0; i < frames; i++) {
        double sample = 0.0;
        for (j = 0; j < num_frequencies; j++) {
            sample += sin(phases[j]);
            phases[j] += phase_increments[j];
            if (phases[j] >= 2.0 * M_PI) {
                phases[j] -= 2.0 * M_PI;
            }
        }
        buffer[i] = (short)(AMPLITUDE * sample / num_frequencies); // Averaging to avoid saturation
    }

    pthread_mutex_unlock(&phase_mutex);

    // Write the samples to the audio device
    int total_frames_written = 0;
    while (total_frames_written < frames) {
        err = snd_pcm_writei(handle, buffer + total_frames_written, frames - total_frames_written);
        if (err == -EPIPE) {
            // Buffer underrun
            snd_pcm_prepare(handle);
        } else if (err < 0) {
            fprintf(stderr, "Error writing to audio device: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
        } else {
            total_frames_written += err;
        }
    }
}