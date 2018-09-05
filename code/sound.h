
/*
    Definicao dos structs e funcoes que compoem um mixer de audio.
*/

struct SoundMixer
{
    void *api;

    u32 internal_buffer_size_bytes;
    u32 internal_sample_size_bytes;
    u32 samples_per_sec;
    u32 channel_count;
    u32 sample_count;
    u8 *samples;

    u32 system_buffer_expected_bytes_per_frame;
    u32 system_buffer_safety_bytes;
    u32 system_buffer_size_bytes;
    u32 system_sample_size_bytes;
    u32 system_buffer_last_write;
    u32 system_buffer_last_play;
    u32 system_current_sample;
    b32 system_buffer_valid;

    u32 samples_to_write;
};

internal void StartPlayingSystemBuffer (SoundMixer *mixer, b32 clear_buffer);
internal void WriteToSystemSoundBuffer (SoundMixer *mixer);
internal void GetSystemSoundBufferPosition (SoundMixer *mixer);

internal void
GenerateSineWave (SoundMixer *mixer, r32 tone_hz, s16 volume, u32 samples_to_write)
{
    if(samples_to_write > mixer->sample_count)
        samples_to_write = mixer->sample_count;

    s16 *dst = (s16*)mixer->samples;
    for (u32 i = 0; i < samples_to_write; ++i)
    {
        r32 t = (2.0f*PI*(mixer->system_current_sample+i)) / ((r32)mixer->samples_per_sec/tone_hz);
        s16 sample_value = (s16) (sinf(t)*volume);

        for (u32 j = 0; j < mixer->channel_count; ++j)
        {
            *(dst++) = sample_value;
        }
    }
}