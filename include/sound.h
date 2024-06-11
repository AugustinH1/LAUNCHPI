#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <pthread.h>


// Constantes
#define SAMPLE_RATE 44100
#define DURATION 1
#define AMPLITUDE 20000 // Amplitude réduite pour éviter la saturation

void init_audio(snd_pcm_t **handle);
void play_tones(snd_pcm_t *handle, double *frequencies, int num_frequencies, double duration);