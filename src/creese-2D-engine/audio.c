#include "../external/miniaudio.h"
#include "../external/dr_wav.h"

#define NOB_STRIP_PREFIX
#include "../../nob.h"

#ifndef AUDIO_DEVICE_FORMAT
    #define AUDIO_DEVICE_FORMAT    ma_format_f32
#endif
#ifndef AUDIO_DEVICE_CHANNELS
    #define AUDIO_DEVICE_CHANNELS              2
#endif
#ifndef AUDIO_DEVICE_SAMPLE_RATE
    #define AUDIO_DEVICE_SAMPLE_RATE           0
#endif

typedef struct {
    uint32_t frame_count;
    uint32_t sample_rate;
    uint32_t channels;
    void *data;
} Wave;

enum {
    AUDIO_BUFFER_USAGE_STATIC,
    AUDIO_BUFFER_USAGE_STREAM,
    AUDIO_BUFFER_USAGE_COUNT,
};

struct Audio_Buffer {
    ma_data_converter converter;
    float volume;
    float pitch;
    float pan;
    bool playing;
    bool paused;
    bool looping;
    int usage;
    bool is_sub_buffer_processed[2];
    uint32_t size_in_frames;
    uint32_t frame_cursor_pos;
    uint32_t frames_processed;
    uint8_t *data;
    Audio_Buffer *next;
    Audio_Buffer *prev;
};

static struct {
    struct {
        ma_context context;
        ma_device device;
        ma_mutex lock;
        bool ready;
        size_t pcm_buffer_size;
        void *pcm_buffer;
    } system;
    struct {
        Audio_Buffer *first;
        Audio_Buffer *last;
        uint32_t default_size;
    } buffer;
} audio_ctx = {0};

static void log_from_miniaudio(void *user_data, ma_uint32 level, const char *msg);
static void on_send_audio_data_to_device(ma_device *device, void *frames_out, const void *frames_input, ma_uint32 frame_count);

void init_audio_device(void)
{
    ma_context_config ma_ctx = ma_context_config_init();
    ma_log_callback_init(log_from_miniaudio, NULL);
    ma_result result = ma_context_init(NULL, 0, &ma_ctx, &audio_ctx.system.context);
    if (result != MA_SUCCESS) {
        printf("ERROR: failed to init_audio_device(), error: %d\n", result);
        return;
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback);

    config.playback.format = AUDIO_DEVICE_FORMAT;
    config.playback.channels = AUDIO_DEVICE_CHANNELS;
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = AUDIO_DEVICE_SAMPLE_RATE;
    config.dataCallback = on_send_audio_data_to_device;

    result = ma_device_init(&audio_ctx.system.context, &config, &audio_ctx.system.device);
    if (result != MA_SUCCESS) {
        printf("ERROR: failed to initialize playback device\n");
        ma_context_uninit(&audio_ctx.system.context);
        return;
    }

    if (ma_mutex_init(&audio_ctx.system.lock) != MA_SUCCESS) {
        printf("ERROR: failed to create mutex for mixing\n");
        ma_device_uninit(&audio_ctx.system.device);
        ma_context_uninit(&audio_ctx.system.context);
        return;
    }

    result = ma_device_start(&audio_ctx.system.device);
    if (result != MA_SUCCESS) {
        printf("ERROR: failed to start playback device\n");
        return;
    }

    audio_ctx.system.ready = true;
}

void close_audio_device(void)
{
    if (audio_ctx.system.ready) {
        ma_mutex_uninit(&audio_ctx.system.lock);
        ma_device_uninit(&audio_ctx.system.device);
        ma_context_uninit(&audio_ctx.system.context);
        audio_ctx.system.ready = false;
        free(audio_ctx.system.pcm_buffer);
        audio_ctx.system.pcm_buffer = NULL;
        audio_ctx.system.pcm_buffer_size = 0;
    } else {
        printf("WARNING: audio device could not be closed, not currently initialized\n");
    }
}

static void log_from_miniaudio(void *user_data, ma_uint32 level, const char *msg)
{
    UNUSED(user_data);
    UNUSED(level);
    printf("miniaudio: %s\n", msg);
}

void stop_audio_buffer_in_locked_state(Audio_Buffer *buffer)
{
    if (buffer) {
        if (buffer->playing && !buffer->paused) {
            buffer->playing = false;
            buffer->paused = false;
            buffer->frame_cursor_pos = 0;
            buffer->frames_processed = 0;
            buffer->is_sub_buffer_processed[0] = true;
            buffer->is_sub_buffer_processed[1] = true;
        }
    }
}

void stop_audio_buffer(Audio_Buffer *buffer)
{
    ma_mutex_lock(&audio_ctx.system.lock);
    stop_audio_buffer_in_locked_state(buffer);
    ma_mutex_unlock(&audio_ctx.system.lock);
}

ma_uint32 read_audio_buffer_frames_in_internal_format(Audio_Buffer *audio_buffer, void *frames_out, ma_uint32 frame_count)
{
    ma_uint32 sub_buffer_size_in_frames = (audio_buffer->size_in_frames > 1) ? audio_buffer->size_in_frames/2 : audio_buffer->size_in_frames;
    ma_uint32 current_sub_buffer_idx = audio_buffer->frame_cursor_pos/sub_buffer_size_in_frames;

    if (current_sub_buffer_idx > 1) return 0;

    bool is_sub_buffer_processed[2] = { 0 };
    is_sub_buffer_processed[0] = audio_buffer->is_sub_buffer_processed[0];
    is_sub_buffer_processed[1] = audio_buffer->is_sub_buffer_processed[1];

    ma_uint32 frame_size_in_bytes = ma_get_bytes_per_frame(audio_buffer->converter.formatIn, audio_buffer->converter.channelsIn);
    ma_uint32 frames_read = 0;

    while (1) {
        if (audio_buffer->usage == AUDIO_BUFFER_USAGE_STATIC) {
            if (frames_read >= frame_count) break;
        } else {
            if (is_sub_buffer_processed[current_sub_buffer_idx]) break;
        }

        ma_uint32 total_frames_remaining = (frame_count - frames_read);
        if (total_frames_remaining == 0) break;

        ma_uint32 frames_remaining_in_output_buffer;
        if (audio_buffer->usage == AUDIO_BUFFER_USAGE_STATIC) {
            frames_remaining_in_output_buffer = audio_buffer->size_in_frames - audio_buffer->frame_cursor_pos;
        } else {
            ma_uint32 first_frame_idx_of_this_sub_buffer = sub_buffer_size_in_frames*current_sub_buffer_idx;
            frames_remaining_in_output_buffer = sub_buffer_size_in_frames - (audio_buffer->frame_cursor_pos - first_frame_idx_of_this_sub_buffer);
        }

        ma_uint32 frames_to_read = total_frames_remaining;
        if (frames_to_read > frames_remaining_in_output_buffer) frames_to_read = frames_remaining_in_output_buffer;

        memcpy((unsigned char *)frames_out + (frames_read*frame_size_in_bytes), audio_buffer->data + (audio_buffer->frame_cursor_pos*frame_size_in_bytes), frames_to_read*frame_size_in_bytes);
        audio_buffer->frame_cursor_pos = (audio_buffer->frame_cursor_pos + frames_to_read)%audio_buffer->size_in_frames;
        frames_read += frames_to_read;

        // If we've read to the end of the buffer, mark it as processed
        if (frames_to_read == frames_remaining_in_output_buffer) {
            audio_buffer->is_sub_buffer_processed[current_sub_buffer_idx] = true;
            is_sub_buffer_processed[current_sub_buffer_idx] = true;

            current_sub_buffer_idx = (current_sub_buffer_idx + 1)%2;

            if (!audio_buffer->looping) {
                stop_audio_buffer_in_locked_state(audio_buffer);
                break;
            }
        }
    }

    ma_uint32 total_frames_remaining = (frame_count - frames_read);
    if (total_frames_remaining > 0) {
        memset((unsigned char *)frames_out + (frames_read*frame_size_in_bytes), 0, total_frames_remaining*frame_size_in_bytes);
        if (audio_buffer->usage != AUDIO_BUFFER_USAGE_STATIC) frames_read += total_frames_remaining;
    }

    return frames_read;
}

ma_uint32 read_audio_buffer_frames_in_mixing_format(Audio_Buffer *audio_buffer, float *frames_out, ma_uint32 frame_count)
{
    ma_uint8 input_buffer[4096] = {0};
    ma_uint32 input_buffer_frame_cap = sizeof(input_buffer)/ma_get_bytes_per_frame(audio_buffer->converter.formatIn, audio_buffer->converter.channelsIn);

    ma_uint32 total_output_frames_processed = 0;
    while (total_output_frames_processed < frame_count) {
        ma_uint64 output_frames_to_process_this_iteration = frame_count - total_output_frames_processed;
        ma_uint64 input_frames_to_process_this_iteration = 0;

        ma_data_converter_get_required_input_frame_count(&audio_buffer->converter, output_frames_to_process_this_iteration, &input_frames_to_process_this_iteration);
        if (input_frames_to_process_this_iteration > input_buffer_frame_cap)
            input_frames_to_process_this_iteration = input_buffer_frame_cap;

        float *running_frames_out = frames_out + (total_output_frames_processed*audio_buffer->converter.channelsOut);

        /* At this point we can convert the data to our mixing format. */
        ma_uint64 input_frames_processed_this_iteration = read_audio_buffer_frames_in_internal_format(audio_buffer, input_buffer, (ma_uint32)input_frames_to_process_this_iteration);
        ma_uint64 output_frames_processed_this_iteration = output_frames_to_process_this_iteration;
        ma_data_converter_process_pcm_frames(&audio_buffer->converter, input_buffer, &input_frames_processed_this_iteration, running_frames_out, &output_frames_processed_this_iteration);

        total_output_frames_processed += (ma_uint32)output_frames_processed_this_iteration;
        if (input_frames_processed_this_iteration < input_frames_to_process_this_iteration) break;
        if (input_frames_processed_this_iteration == 0 && output_frames_processed_this_iteration == 0) break;
    }

    return total_output_frames_processed;
}

void mix_audio_frames(float *frames_out, const float *frames_in, ma_uint32 frame_count, Audio_Buffer *buffer)
{
    const float local_volume = buffer->volume;
    const ma_uint32 channels = audio_ctx.system.device.playback.channels;

    if (channels == 2) {  // We consider panning
        const float left = buffer->pan;
        const float right = 1.0f - left;

        // Fast sine approximation in [0..1] for pan law: y = 0.5f*x*(3 - x*x);
        const float levels[2] = { local_volume*0.5f*left*(3.0f - left*left), local_volume*0.5f*right*(3.0f - right*right) };

        float *frame_out = frames_out;
        const float *frame_in = frames_in;

        for (ma_uint32 frame = 0; frame < frame_count; frame++) {
            frame_out[0] += (frame_in[0]*levels[0]);
            frame_out[1] += (frame_in[1]*levels[1]);

            frame_out += 2;
            frame_in += 2;
        }
    } else {  // We do not consider panning
        for (ma_uint32 frame = 0; frame < frame_count; frame++) {
            for (ma_uint32 c = 0; c < channels; c++) {
                float *frame_out = frames_out + (frame*channels);
                const float *frame_in = frames_in + (frame*channels);

                // Output accumulates input multiplied by volume to provided output (usually 0)
                frame_out[c] += (frame_in[c]*local_volume);
            }
        }
    }
}

static void on_send_audio_data_to_device(ma_device *device, void *p_frames_out, const void *p_frames_input, ma_uint32 frame_count)
{
    UNUSED(device);
    UNUSED(p_frames_input);
    memset(p_frames_out, 0, frame_count*device->playback.channels*ma_get_bytes_per_sample(device->playback.format));

    ma_mutex_lock(&audio_ctx.system.lock); {
        for (Audio_Buffer *audio_buffer = audio_ctx.buffer.first; audio_buffer; audio_buffer = audio_buffer->next) {
            if (!audio_buffer->playing || audio_buffer->paused) continue;

            ma_uint32 frames_read = 0;

            while (frames_read < frame_count) {
                ma_uint32 frames_to_read = frame_count - frames_read;

                while (frames_to_read) {
                    float tmp_buff[1024] = {0};
                    ma_uint32 frames_to_read_now = frames_to_read;
                    if (frames_to_read_now > sizeof(tmp_buff)/sizeof(tmp_buff[0])/AUDIO_DEVICE_CHANNELS)
                        frames_to_read_now = sizeof(tmp_buff)/sizeof(tmp_buff[0])/AUDIO_DEVICE_CHANNELS;

                    ma_uint32 frames_just_read = read_audio_buffer_frames_in_mixing_format(audio_buffer, tmp_buff, frames_to_read_now);
                    if (frames_just_read) {
                        float *frames_out = (float *)p_frames_out + (frames_read*audio_ctx.system.device.playback.channels);
                        float *frames_in = tmp_buff;
                        mix_audio_frames(frames_out, frames_in, frames_just_read, audio_buffer);
                        frames_to_read -= frames_just_read;
                        frames_read += frames_just_read;
                    }

                    if (!audio_buffer->playing) {
                        frames_read = frame_count;
                        break;
                    }

                    if (frames_just_read < frames_to_read_now) {
                        if (!audio_buffer->looping) {
                            stop_audio_buffer_in_locked_state(audio_buffer);
                            break;
                        } else {
                            audio_buffer->frame_cursor_pos = 0;
                            break;
                        }
                    }

                    if (frames_to_read > 0) break;
                }
            }
        }
    } ma_mutex_unlock(&audio_ctx.system.lock);
}

Wave load_wave(const char *file_path)
{
    Wave wave = {0};

    /* load wave file */
    String_Builder sb = {0};
    if (!read_entire_file(file_path, &sb)) return wave;
    drwav wav = {0};
    if (!drwav_init_memory(&wav, sb.items, sb.count, NULL)) {
        printf("ERROR: drwav_init_memory\n");
        return wave;
    }
    wave.frame_count = (unsigned int)wav.totalPCMFrameCount;
    wave.sample_rate = wav.sampleRate;
    wave.channels = wav.channels;
    wave.data = malloc(wave.frame_count*wave.channels*sizeof(short));

    /* force conversion to 16 bit sample size */
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, wave.data);

    drwav_uninit(&wav);
    sb_free(sb);

    return wave;
}

void track_audio_buffer(Audio_Buffer *buffer)
{
    ma_mutex_lock(&audio_ctx.system.lock);
        if (audio_ctx.buffer.first == NULL) audio_ctx.buffer.first = buffer;
        else {
            audio_ctx.buffer.last->next = buffer;
            buffer->prev = audio_ctx.buffer.last;
        }

        audio_ctx.buffer.last = buffer;
    ma_mutex_unlock(&audio_ctx.system.lock);
}

void untrack_audio_buffer(Audio_Buffer *buffer)
{
    ma_mutex_lock(&audio_ctx.system.lock);
        if (buffer->prev == NULL) audio_ctx.buffer.first = buffer->next;
        else buffer->prev->next = buffer->next;

        if (buffer->next == NULL) audio_ctx.buffer.last = buffer->prev;
        else buffer->next->prev = buffer->prev;

        buffer->prev = NULL;
        buffer->next = NULL;
    ma_mutex_unlock(&audio_ctx.system.lock);
}

Audio_Buffer *load_audio_buffer(ma_format format, ma_uint32 channels, ma_uint32 sample_rate, ma_uint32 size_in_frames, int usage)
{
    Audio_Buffer *audio_buffer = calloc(1, sizeof(Audio_Buffer));

    if (audio_buffer == NULL) {
        printf("ERROR: Failed to allocate memory for buffer\n");
        return NULL;
    }

    if (size_in_frames > 0) audio_buffer->data = calloc(size_in_frames*channels*ma_get_bytes_per_sample(format), 1);

    ma_data_converter_config config = ma_data_converter_config_init(format, AUDIO_DEVICE_FORMAT, channels,
                                                                    AUDIO_DEVICE_CHANNELS, sample_rate,
                                                                    audio_ctx.system.device.sampleRate);
    config.allowDynamicSampleRate = true;

    ma_result result = ma_data_converter_init(&config, NULL, &audio_buffer->converter);

    if (result != MA_SUCCESS) {
        printf("ERROR: failed to create data conversion pipeline\n");
        free(audio_buffer);
        return NULL;
    }

    audio_buffer->volume = 1.0f;
    audio_buffer->pitch = 1.0f;
    audio_buffer->pan = 0.5f;
    audio_buffer->playing = false;
    audio_buffer->paused = false;
    audio_buffer->looping = false;
    audio_buffer->usage = usage;
    audio_buffer->frame_cursor_pos = 0;
    audio_buffer->size_in_frames = size_in_frames;
    audio_buffer->is_sub_buffer_processed[0] = true;
    audio_buffer->is_sub_buffer_processed[1] = true;
    track_audio_buffer(audio_buffer);
    return audio_buffer;
}

void unload_audio_buffer(Audio_Buffer *buffer)
{
    if (buffer) {
        ma_data_converter_uninit(&buffer->converter, NULL);
        untrack_audio_buffer(buffer);
        free(buffer->data);
        free(buffer);
    }
}

Sound load_sound_from_wave(Wave wave)
{
    Sound sound = {0};

    if (wave.data) {
        ma_uint32 frame_count = ma_convert_frames(NULL, 0, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS,
                                                  audio_ctx.system.device.sampleRate, NULL, wave.frame_count,
                                                  ma_format_s16, // force 16 bit
                                                  wave.channels, wave.sample_rate);
        if (!frame_count) printf("ERROR: failed to get frame count for format conversion\n");
        Audio_Buffer *audio_buffer = load_audio_buffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS,
                                                       audio_ctx.system.device.sampleRate, frame_count,
                                                       AUDIO_BUFFER_USAGE_STATIC);
        if (!audio_buffer) {
            printf("ERROR: failed to create buffer");
            return sound;
        }

        frame_count = ma_convert_frames(audio_buffer->data, frame_count, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS,
                                        audio_ctx.system.device.sampleRate, wave.data, wave.frame_count,
                                        ma_format_s16, // force 16 bit
                                        wave.channels, wave.sample_rate);
        if (!frame_count) printf("ERROR: failed format conversion\n");

        sound.frame_count = frame_count;
        sound.stream.sample_rate = audio_ctx.system.device.sampleRate;
        sound.stream.sample_size = 32;
        sound.stream.channels = AUDIO_DEVICE_CHANNELS;
        sound.stream.buffer = audio_buffer;
    }

    return sound;
}

void play_audio_buffer(Audio_Buffer *buffer)
{
    if (buffer) {
        ma_mutex_lock(&audio_ctx.system.lock);
            buffer->playing = true;
            buffer->paused = false;
            buffer->frame_cursor_pos = 0;
            buffer->frames_processed = 0;
            buffer->is_sub_buffer_processed[0] = true;
            buffer->is_sub_buffer_processed[1] = true;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
}

void pause_audio_buffer(Audio_Buffer *buffer)
{
    if (buffer) {
        ma_mutex_lock(&audio_ctx.system.lock);
        buffer->paused = true;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
}

void resume_audio_buffer(Audio_Buffer *buffer)
{
    if (buffer) {
        ma_mutex_lock(&audio_ctx.system.lock);
        buffer->paused = false;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
}

bool is_audio_buffer_playing(Audio_Buffer *buffer)
{
    bool result = false;
    if (buffer) {
        ma_mutex_lock(&audio_ctx.system.lock);
        result = buffer->playing && !buffer->paused;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
    return result;
}

void play_sound(Sound sound)
{
    play_audio_buffer(sound.stream.buffer);
}

void stop_sound(Sound sound)
{
    stop_audio_buffer(sound.stream.buffer);
}

void pause_sound(Sound sound)
{
    pause_audio_buffer(sound.stream.buffer);
}

void resume_sound(Sound sound)
{
    resume_audio_buffer(sound.stream.buffer);
}

bool is_sound_playing(Sound sound)
{
    return is_audio_buffer_playing(sound.stream.buffer);
}

void set_audio_buffer_volume(Audio_Buffer *buffer, float volume)
{
    if (buffer) {
        ma_mutex_lock(&audio_ctx.system.lock);
        buffer->volume = volume;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
}

void set_audio_buffer_pitch(Audio_Buffer *buffer, float pitch)
{
    if (buffer && pitch > 0.0f) {
        ma_mutex_lock(&audio_ctx.system.lock);
        ma_uint32 out_sample_rate = audio_ctx.system.device.sampleRate/pitch;
        ma_data_converter_set_rate(&buffer->converter, buffer->converter.sampleRateIn, out_sample_rate);
        buffer->pitch = pitch;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
}

void set_audio_buffer_pan(Audio_Buffer *buffer, float pan)
{
    if (pan < -1.0f) pan = -1.0f;
    else if (pan > 1.0f) pan = 1.0f;

    if (buffer) {
        ma_mutex_lock(&audio_ctx.system.lock);
        buffer->pan = pan;
        ma_mutex_unlock(&audio_ctx.system.lock);
    }
}

void set_sound_pitch(Sound sound, float pitch)
{
    set_audio_buffer_pitch(sound.stream.buffer, pitch);
}

void set_sound_pan(Sound sound, float pan)
{
    set_audio_buffer_pan(sound.stream.buffer, pan);
}

void set_sound_volume(Sound sound, float volume)
{
    set_audio_buffer_volume(sound.stream.buffer, volume);
}

Sound load_sound(const char *file_path)
{
    Wave wave = load_wave(file_path);
    Sound sound = load_sound_from_wave(wave);
    free(wave.data);
    return sound;
}

void unload_sound(Sound sound)
{
    unload_audio_buffer(sound.stream.buffer);
}

Audio_Stream load_audio_stream(uint32_t sample_rate, uint32_t sample_size, uint32_t channels)
{
    Audio_Stream stream = {
        .sample_rate = sample_rate,
        .sample_size = sample_size,
        .channels = channels,
    };

    uint32_t period_size = audio_ctx.system.device.playback.internalPeriodSizeInFrames;
    uint32_t sub_buff_size = audio_ctx.buffer.default_size == 0 ? audio_ctx.system.device.sampleRate/30 : audio_ctx.buffer.default_size;
    if (sub_buff_size < period_size) sub_buff_size = period_size;

    stream.buffer = load_audio_buffer(ma_format_s16, stream.channels, stream.sample_rate, sub_buff_size*2, AUDIO_BUFFER_USAGE_STREAM);

    if (stream.buffer) stream.buffer->looping = true;
    else printf("ERROR: failed to initialize stream\n");

    return stream;
}

void update_audio_stream_in_locked_state(Audio_Stream stream, const void *data, int frame_count)
{
    if (stream.buffer) {
        if (stream.buffer->is_sub_buffer_processed[0] || stream.buffer->is_sub_buffer_processed[1]) {
            ma_uint32 sub_buff_to_update = 0;

            if (stream.buffer->is_sub_buffer_processed[0] && stream.buffer->is_sub_buffer_processed[1]) {
                sub_buff_to_update = 0;
                stream.buffer->frame_cursor_pos = 0;
            } else {
                sub_buff_to_update = (stream.buffer->is_sub_buffer_processed[0])? 0 : 1;
            }

            ma_uint32 sub_buff_size_in_frames = stream.buffer->size_in_frames/2;
            uint8_t *sub_buff = stream.buffer->data + ((sub_buff_size_in_frames*stream.channels*(stream.sample_size/8))*sub_buff_to_update);

            // Total frames processed in buffer is always the complete size, filled with 0 if required
            stream.buffer->frames_processed += frame_count;

            if (sub_buff_size_in_frames >= (ma_uint32)frame_count) {
                ma_uint32 frames_to_write = (ma_uint32)frame_count;

                ma_uint32 bytes_to_write = frames_to_write*stream.channels*(stream.sample_size/8);
                memcpy(sub_buff, data, bytes_to_write);

                // Any leftover frames should be filled with zeros.
                ma_uint32 leftover_frame_count = sub_buff_size_in_frames - frames_to_write;

                if (leftover_frame_count > 0) memset(sub_buff + bytes_to_write, 0, leftover_frame_count*stream.channels*(stream.sample_size/8));

                stream.buffer->is_sub_buffer_processed[sub_buff_to_update] = false;
            }
            else printf("ERROR: update_audio_stream(), attempting to write too many frames to buffer\n");
        }
        else printf("ERROR: update_audio_stream(), buffer not available for updating");
    }
}

void update_audio_stream(Audio_Stream stream, const void *data, int frame_count)
{
    ma_mutex_lock(&audio_ctx.system.lock);
    update_audio_stream_in_locked_state(stream, data, frame_count);
    ma_mutex_unlock(&audio_ctx.system.lock);
}

Music load_music_stream(const char *file_path)
{
    Music music = {0};

    drwav *wav = calloc(1, sizeof(drwav));
    bool result = drwav_init_file(wav, file_path, NULL);

    if (result) {
        music.data = wav;
        music.stream = load_audio_stream(wav->sampleRate, 16, wav->channels);
        music.frame_count = wav->totalPCMFrameCount;
        music.looping = true;
    } else {
        printf("ERROR: failed to load music stream\n");
        drwav_uninit(wav);
    }

    return music;
}

void unload_music_stream(Music music)
{
    unload_audio_buffer(music.stream.buffer);
    if (music.data) drwav_uninit(music.data);
}

void play_music_stream(Music music)
{
    if (music.stream.buffer) {
        ma_uint32 frame_cursor_pos = music.stream.buffer->frame_cursor_pos;
        play_audio_buffer(music.stream.buffer);
        music.stream.buffer->frame_cursor_pos = frame_cursor_pos;
    }
}

void stop_music_stream(Music music)
{
    stop_audio_buffer(music.stream.buffer);
    drwav_seek_to_pcm_frame(music.data, 0);
}

void pause_music_stream(Music music)
{
    pause_audio_buffer(music.stream.buffer);
}

void resume_music_stream(Music music)
{
    resume_audio_buffer(music.stream.buffer);
}

void update_music_stream(Music music)
{
    if (!music.stream.buffer) return;
    if (!music.stream.buffer->playing) return;

    ma_mutex_lock(&audio_ctx.system.lock);

    uint32_t sub_buff_size_in_frames = music.stream.buffer->size_in_frames/2;
    int frame_size = music.stream.channels*music.stream.sample_size/8;
    uint32_t pcm_size = sub_buff_size_in_frames*frame_size;

    if (audio_ctx.system.pcm_buffer_size < pcm_size) {
        free(audio_ctx.system.pcm_buffer);
        audio_ctx.system.pcm_buffer = calloc(1, pcm_size);
        audio_ctx.system.pcm_buffer_size = pcm_size;
    }

    for (int i = 0; i < 2; i++) {
        uint32_t frames_left = music.frame_count - music.stream.buffer->frames_processed;
        uint32_t frames_to_stream = 0;

        if (frames_left >= sub_buff_size_in_frames || music.looping) frames_to_stream = sub_buff_size_in_frames;
        else frames_to_stream = frames_left;

        if (frames_to_stream == 0) {
            if (music.stream.buffer->is_sub_buffer_processed[0] && music.stream.buffer->is_sub_buffer_processed[1]) {
                ma_mutex_unlock(&audio_ctx.system.lock);
                stop_music_stream(music);
                return;
            }
            ma_mutex_unlock(&audio_ctx.system.lock);
            return;
        }

        if (music.stream.buffer && !music.stream.buffer->is_sub_buffer_processed[i]) continue;

        int frame_count_still_needed = frames_to_stream;
        int frame_count_read_total = 0;

        while (true) {
            if (music.stream.sample_size == 16) {
                short *out_buff = (short *)((char*)audio_ctx.system.pcm_buffer + frame_count_read_total*frame_size);
                int frame_count_read = drwav_read_pcm_frames_s16(music.data, frame_count_still_needed, out_buff);
                frame_count_read_total += frame_count_read;
                frame_count_still_needed -= frame_count_read;
                if (!frame_count_still_needed) break;
                else drwav_seek_to_pcm_frame(music.data, 0);
            } else if (music.stream.sample_size == 32) {
                float *out_buff = (float*)((char*)audio_ctx.system.pcm_buffer + frame_count_read_total*frame_size);
                int frame_count_read = drwav_read_pcm_frames_f32(music.data, frame_count_still_needed, out_buff);
                frame_count_read_total += frame_count_read;
                frame_count_still_needed -= frame_count_read;
                if (!frame_count_still_needed) break;
                else drwav_seek_to_pcm_frame(music.data, 0);
            }
        }

        update_audio_stream_in_locked_state(music.stream, audio_ctx.system.pcm_buffer, frames_to_stream);
    }

    ma_mutex_unlock(&audio_ctx.system.lock);
}
