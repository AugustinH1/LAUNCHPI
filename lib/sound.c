#include "../include/sound.h"

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

void play_tones(snd_pcm_t *handle, double *frequencies, int num_frequencies) {
    int err;
    int frames = SAMPLE_RATE / 10; // Nombre d'échantillons pour 0.1 seconde
    short buffer[frames];
    double phases[num_frequencies];
    double phase_increments[num_frequencies];
    int i,j;

    // Initialiser les phases et les incréments de phase pour chaque fréquence
    for (i = 0; i < num_frequencies; i++) {
        phases[i] = 0.0;
        phase_increments[i] = 2.0 * M_PI * frequencies[i] / SAMPLE_RATE;
    }

    // Générer les échantillons de l'onde sinusoïdale combinée
    for (i = 0; i < frames; i++) {
        double sample = 0.0;
        for (j = 0; j < num_frequencies; j++) {
            sample += sin(phases[j]);
            phases[j] += phase_increments[j];
            if (phases[j] >= 2.0 * M_PI) {
                phases[j] -= 2.0 * M_PI;
            }
        }
        buffer[i] = (short)(AMPLITUDE * sample / num_frequencies); // Moyennage pour éviter la saturation
    }

    // Jouer les échantillons
    if ((err = snd_pcm_writei(handle, buffer, frames)) < 0) {
        if (err == -EPIPE) {
            // Buffer underrun
            snd_pcm_prepare(handle);
        } else {
            fprintf(stderr, "Erreur lors de l'écriture sur le périphérique audio : %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
        }
    }
}