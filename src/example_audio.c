// WIP!

#include "creese_2D.h"

// #define DR_WAV_IMPLEMENTATION
#include "external/dr_wav.h"

#define NOB_STRIP_PREFIX
#include "../nob.h"

typedef struct {
    uint32_t frame_count;
    uint32_t sample_rate;
    uint32_t sample_size;
    uint32_t channels;
    void *data;
} Wave;

// // AudioStream, custom audio stream
// typedef struct AudioStream {
//     rAudioBuffer *buffer;       // Pointer to internal data used by the audio system
//     rAudioProcessor *processor; // Pointer to internal data processor, useful for audio effects
//
//     unsigned int sampleRate;    // Frequency (samples per second)
//     unsigned int sampleSize;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
//     unsigned int channels;      // Number of channels (1-mono, 2-stereo, ...)
// } AudioStream;
//
// // Sound
// typedef struct Sound {
//     AudioStream stream;         // Audio stream
//     unsigned int frameCount;    // Total number of frames (considering channels)
// } Sound;

int main()
{
    Wave wave = {0};

    /* load wave file */
    String_Builder sb = {0};
    if (!read_entire_file("assets/weird.wav", &sb)) return 1;
    drwav wav = {0};
    if (!drwav_init_memory(&wav, sb.items, sb.count, NULL)) return 1;
    wave.frame_count = (unsigned int)wav.totalPCMFrameCount;
    wave.sample_rate = wav.sampleRate;
    wave.sample_size = 16;
    wave.channels = wav.channels;
    wave.data = malloc(wave.frame_count*wave.channels*sizeof(short));
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, wave.data);
    drwav_uninit(&wav);
    sb_free(sb);

    /* load sound */
    if (wave.data) {

    }



    return 0;
}
