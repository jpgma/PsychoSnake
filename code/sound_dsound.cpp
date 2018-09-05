

internal SoundMixer *
InitDSSoundMixer (HWND window, 
                  u32 samples_per_sec, u32 channel_count, u32 bytes_per_sample,
                  u32 buffer_size_seconds, u32 target_fps)
{
    char error_str[128];
    SoundMixer *res = (SoundMixer*)calloc(1,sizeof(SoundMixer));
    LPDIRECTSOUNDBUFFER *dsound_buffer = (LPDIRECTSOUNDBUFFER*)calloc(1,sizeof(LPDIRECTSOUNDBUFFER));
    res->api = (void*)dsound_buffer;

    WAVEFORMATEX waveformatex = {};
    DSBUFFERDESC bufferdesc = {};

    res->internal_sample_size_bytes = channel_count*bytes_per_sample;
    res->internal_buffer_size_bytes = buffer_size_seconds*(samples_per_sec*res->internal_sample_size_bytes);
    res->samples_per_sec = samples_per_sec;
    res->channel_count = channel_count;
    res->sample_count = (buffer_size_seconds * samples_per_sec);
    res->samples = (u8*)calloc(1,res->internal_buffer_size_bytes);

    res->system_buffer_size_bytes = res->internal_buffer_size_bytes;
    res->system_sample_size_bytes = res->internal_sample_size_bytes;
    res->system_buffer_expected_bytes_per_frame = (res->samples_per_sec/target_fps)*res->system_sample_size_bytes;
    res->system_buffer_safety_bytes = res->system_buffer_expected_bytes_per_frame/2;

    // obtendo objeto do DirectSound
    LPDIRECTSOUND dsound;
    if(!SUCCEEDED(DirectSoundCreate(0, &dsound, 0)))
    {
        sprintf(error_str, "DirectSoundCreate falhou!");
        goto error;
    }

    if(!SUCCEEDED(dsound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
    {
        sprintf(error_str, "SetCooperativeLevel falhou!");
        goto error;
    }

    bufferdesc.dwSize = sizeof(DSBUFFERDESC);
    bufferdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    LPDIRECTSOUNDBUFFER primary_buffer;

    if(!SUCCEEDED(dsound->CreateSoundBuffer(&bufferdesc, &primary_buffer, 0)))
    {
        sprintf(error_str, "CreateSoundBuffer primario falhou!");
        goto error;
    }

    waveformatex.wFormatTag = WAVE_FORMAT_PCM;
    waveformatex.nChannels = channel_count;
    waveformatex.nSamplesPerSec = samples_per_sec;
    waveformatex.wBitsPerSample = bytes_per_sample * 8;
    waveformatex.nBlockAlign = (waveformatex.nChannels*waveformatex.wBitsPerSample)/8;
    waveformatex.nAvgBytesPerSec = waveformatex.nSamplesPerSec * waveformatex.nBlockAlign;

    if(!SUCCEEDED(primary_buffer->SetFormat(&waveformatex)))
    {
        sprintf(error_str, "SetFormat falhou!");
        goto error;
    }

    bufferdesc.dwFlags = 0;
    bufferdesc.lpwfxFormat = &waveformatex;
    bufferdesc.dwBufferBytes = res->system_buffer_size_bytes;
    
    if(!SUCCEEDED(dsound->CreateSoundBuffer(&bufferdesc, dsound_buffer, 0)))
    {
        sprintf(error_str, "CreateSoundBuffer secundario falhou!");
        goto error;
    }

    return res;

    error:
    free(res->samples);
    free(res);
    free(dsound_buffer);
    MessageBoxA(NULL, error_str, "Falha em InitDSSoundMixer!", MB_ICONERROR|MB_OK);
    return 0;
}

internal void
FreeDSSoundMixer (SoundMixer *mixer)
{
    free(mixer->samples);
    free(mixer->api);
    free(mixer);
}

internal void 
StartPlayingSystemBuffer (SoundMixer *mixer, b32 clear_buffer)
{
    LPDIRECTSOUNDBUFFER *dsound_buffer = (LPDIRECTSOUNDBUFFER *)mixer->api;

    if(clear_buffer)
    {
        for (u32 i = 0; i < mixer->internal_buffer_size_bytes; ++i)
        {
            mixer->samples[i] = 0;
        }
    }

    (*dsound_buffer)->Play(0, 0, DSBPLAY_LOOPING);
}

internal void 
GetSystemSoundBufferPosition (SoundMixer *mixer)
{
    LPDIRECTSOUNDBUFFER *dsound_buffer = (LPDIRECTSOUNDBUFFER *)mixer->api;

    DWORD play_cursor;
    DWORD write_cursor;
    if(SUCCEEDED((*dsound_buffer)->GetCurrentPosition(&play_cursor,&write_cursor)))
    {
        mixer->system_buffer_last_write = write_cursor;
        mixer->system_buffer_last_play = play_cursor;
        if(!mixer->system_buffer_valid)
        {
            u32 bytes_betweeen;
            if(write_cursor < play_cursor)
                bytes_betweeen = (mixer->system_buffer_size_bytes - play_cursor) + write_cursor;
            else
                bytes_betweeen = write_cursor - play_cursor;
            r32 latency = ((r32)bytes_betweeen) / (r32)mixer->system_sample_size_bytes;
            mixer->samples_to_write = latency;

            mixer->system_current_sample = write_cursor / mixer->system_sample_size_bytes;
            mixer->system_buffer_valid = true;
        }
    }
    else
    {
        mixer->system_buffer_valid = false;
    }
}

internal void 
WriteToSystemSoundBuffer (SoundMixer *mixer)
{
    if(mixer->system_buffer_valid)
    {
        LPDIRECTSOUNDBUFFER *dsound_buffer = (LPDIRECTSOUNDBUFFER *)mixer->api;

        DWORD byte_to_lock = (mixer->system_current_sample*mixer->system_sample_size_bytes) % mixer->system_buffer_size_bytes;
        
        DWORD safe_write_cursor = mixer->system_buffer_last_write;
        if(safe_write_cursor < mixer->system_buffer_last_play)
        {
            safe_write_cursor += mixer->system_buffer_size_bytes;
        }
        safe_write_cursor += mixer->system_buffer_safety_bytes;
        DWORD expected_bytes_per_frame = mixer->system_buffer_expected_bytes_per_frame;
        DWORD expected_frame_boundary_bytes = (mixer->system_buffer_last_play + expected_bytes_per_frame);
        b32 audio_is_low_latency = (safe_write_cursor < expected_frame_boundary_bytes);
        
        DWORD target_cursor = 0;
        if(audio_is_low_latency)
        {
            target_cursor = expected_frame_boundary_bytes + expected_bytes_per_frame;
        }
        else
        {
            target_cursor = (mixer->system_buffer_last_write + 
                             expected_bytes_per_frame + 
                             mixer->system_buffer_safety_bytes);
        }
        target_cursor = target_cursor % mixer->system_buffer_size_bytes;
        
        DWORD bytes_to_write;
        if(byte_to_lock > target_cursor)
        {
            bytes_to_write = (mixer->system_buffer_size_bytes - byte_to_lock);
            bytes_to_write += target_cursor;
        }
        else
        {
            bytes_to_write = target_cursor - byte_to_lock;
        }
        u32 samples_to_write = bytes_to_write / mixer->system_sample_size_bytes;

        GenerateSineWave(mixer, 260.0f, 10000, samples_to_write);

        VOID *region_1 = 0;
        DWORD region_1_size = 0;
        VOID *region_2 = 0;
        DWORD region_2_size = 0;
        if(SUCCEEDED((*dsound_buffer)->Lock(byte_to_lock, bytes_to_write,
                                             &region_1, &region_1_size,
                                             &region_2, &region_2_size,
                                             0)))
        {
            s16 *src_sample = (s16 *)mixer->samples;
            s16 *dest_sample = (s16*)region_1;
            u32 region_1_count = (region_1_size/mixer->system_sample_size_bytes);
            for (s32 sample_index = 0; 
                 sample_index < region_1_count; 
                 ++sample_index)
            {
                for (u32 i = 0; i < mixer->channel_count; ++i)
                {
                    *(dest_sample++) = *(src_sample++);
                }
            }
            dest_sample = (s16*)region_2;
            u32 region_2_count = (region_2_size/mixer->system_sample_size_bytes);
            for (s32 sample_index = 0; 
                 sample_index < region_2_count; 
                 ++sample_index)
            {
                for (u32 i = 0; i < mixer->channel_count; ++i)
                {
                    *(dest_sample++) = *(src_sample++);
                }
            }
            
            mixer->system_current_sample += region_1_count + region_2_count;

            (*dsound_buffer)->Unlock(region_1,region_1_size, region_2,region_2_size);
        }
    }
}
