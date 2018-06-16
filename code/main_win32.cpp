// #define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <dsound.h>
#include <math.h>

///////////////////////
// Definicoes basicas

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef float r32;
typedef double r64;
typedef u32 b32;
#define internal static;
#define global static;
#define assert(x) {if(!(x)) *((int*)0) = 0;}

#define PI (3.1415927)

///////////////////////
// CMD Renderer

#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 16
#define CHAR_SIZE 16
#define DEBUG_LINE_COUNT 1

#include "psychosnake.h"

#define WIN32_KEY_DOWN 0x8000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)

//Limitando o FPS
#define TARGET_FPS 30
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

inline u16
RGBColor(b32 r, b32 g, b32 b, b32 i, b32 rb, b32 gb, b32 bb, b32 ib)
{
    return(u16)((r?FOREGROUND_RED:0)|(g?FOREGROUND_GREEN:0)|(b?FOREGROUND_BLUE:0)|(i?FOREGROUND_INTENSITY:0))|
               ((rb?BACKGROUND_RED:0)|(gb?BACKGROUND_GREEN:0)|(bb?BACKGROUND_BLUE:0)|(ib?BACKGROUND_INTENSITY:0));
}

internal void 
LimparTela(CHAR_INFO *buffer, u32 clear_char, u16 clear_color)
{
    CHAR_INFO clear_char_info = {};
    clear_char_info.Char.UnicodeChar = clear_char;
    clear_char_info.Attributes = clear_color;
    
    for(u32 i = 0; i < (SCREEN_WIDTH*SCREEN_HEIGHT); i++)
    {
        buffer[i] = clear_char_info;
    }
}

internal void 
PrintarBitMap(HANDLE screen_buffer_handle, CHAR_INFO *buffer, 
              COORD buffer_size, COORD buffer_coord, SMALL_RECT write_rect)
{
    WriteConsoleOutput(screen_buffer_handle, buffer, buffer_size, buffer_coord, &write_rect);
}

///////////////////////
// DirectSound

// Criando assinatura da funcao q carregarei do .dll
typedef HRESULT WINAPI DIRECTSOUNDCREATE(LPGUID,LPDIRECTSOUND*,LPUNKNOWN);
DIRECTSOUNDCREATE *DirectSoundCreate_;

internal b32
InitDirectSound (LPDIRECTSOUNDBUFFER *ds_buffer, HWND window, u32 nchannels, u32 samples_per_sec, u32 buffer_size_bytes)
{
    b32 res = false;

    // carregando dll 
    HMODULE dsound_library = LoadLibraryA("dsound.dll");
    if(dsound_library)
    {
        // obtendo a funcao do dll
        DirectSoundCreate_ = (DIRECTSOUNDCREATE*) GetProcAddress(dsound_library, "DirectSoundCreate");

        // obtendo objeto do DirectSound
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate_ && SUCCEEDED(DirectSoundCreate_(0, &DirectSound, 0)))
        {
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC bufferdesc = {};
                bufferdesc.dwSize = sizeof(DSBUFFERDESC);
                bufferdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                LPDIRECTSOUNDBUFFER primary_buffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&bufferdesc, &primary_buffer, 0)))
                {
                    WAVEFORMATEX waveformatex = {};
                    waveformatex.wFormatTag = WAVE_FORMAT_PCM;
                    waveformatex.nChannels = nchannels;
                    waveformatex.nSamplesPerSec = samples_per_sec;
                    waveformatex.wBitsPerSample = sizeof(i16) * 8;
                    waveformatex.nBlockAlign = (waveformatex.nChannels*waveformatex.wBitsPerSample)/8;
                    waveformatex.nAvgBytesPerSec = waveformatex.nSamplesPerSec * waveformatex.nBlockAlign;

                    if(SUCCEEDED(primary_buffer->SetFormat(&waveformatex)))
                    {
                        bufferdesc.dwFlags = 0;
                        bufferdesc.lpwfxFormat = &waveformatex;
                        bufferdesc.dwBufferBytes = buffer_size_bytes;
                        if(SUCCEEDED(DirectSound->CreateSoundBuffer(&bufferdesc, ds_buffer, 0)))
                        {
                            res = true;
                        }
                        else
                        {
                            // Nao criou buffer primario
                            assert(0);
                        }
                    }
                    else
                    {
                        // Nao setou o formato do buffer primario
                        assert(0);
                    }
                }
                else
                {
                    // Nao criou buffer primario
                    assert(0);
                }
            }
            else
            {
                // Nao setou coop level
                assert(0);
            }
        }
        else
        {
            // Nao obteve dsound
            assert(0);
        }
    }

    return res;
}

internal void
DirectSoundWriteToBuffer (LPDIRECTSOUNDBUFFER ds_buffer, u32 ds_buffer_size, u32 *ds_current_sample, i16 *internal_buffer, u32 samples_to_write, u32 samples_per_sec)
{
    // TODO: passar infos do buffer
    u32 ds_sample_index = *ds_current_sample;
    u32 ds_buffer_bytes_per_sample = 2*sizeof(i16);
    DWORD byte_to_lock = (ds_sample_index*ds_buffer_bytes_per_sample)%ds_buffer_size;
    DWORD bytes_to_write = samples_to_write*ds_buffer_bytes_per_sample;

    VOID *region_1 = 0;
    DWORD region_1_size = 0;
    VOID *region_2 = 0;
    DWORD region_2_size = 0;
    if(SUCCEEDED(ds_buffer->Lock(byte_to_lock, bytes_to_write,
                                 &region_1, &region_1_size,
                                 &region_2, &region_2_size,
                                 0)))
    {
        // i16 *src_sample = sound_state->samples;

        r32 hz = 261.83f;
        i16 volume = 10000;

        i16 *dest_sample = (i16*)region_1;
        u32 region_1_count = (region_1_size/ds_buffer_bytes_per_sample);
        for (i32 sample_index = 0; 
             sample_index < region_1_count; 
             ++sample_index)
        {
            r32 t = (2.0f*PI*(r32)ds_sample_index++) / (r32)(samples_per_sec/hz);
            i16 sample_value = (i16) (sinf(t)*volume);

            *(dest_sample++) = sample_value;//*(src_sample++);
            *(dest_sample++) = sample_value;//*(src_sample++);
        }
        dest_sample = (i16*)region_2;
        u32 region_2_count = (region_2_size/ds_buffer_bytes_per_sample);
        for (i32 sample_index = 0; 
             sample_index < region_2_count; 
             ++sample_index)
        {
            r32 t = (2.0f*PI*(r32)ds_sample_index++) / (r32)(samples_per_sec/hz);
            i16 sample_value = (i16) (sinf(t)*volume);

            *(dest_sample++) = sample_value;//*(src_sample++);
            *(dest_sample++) = sample_value;//*(src_sample++);
        }
        
        *ds_current_sample += region_1_count + region_2_count;

        ds_buffer->Unlock(region_1,region_1_size, region_2,region_2_size);
    }
}

///////////////////////
// Plataforma

internal i64
GetTime ()
{
    LARGE_INTEGER res;
    QueryPerformanceCounter(&res);
    return res.QuadPart;
}

internal r64
GetTimeElapsed (i64 a, i64 b, i64 perf_frequency)
{
    r64 res = (1000.0 * (b - a)) /
              (r64) perf_frequency;
    return res;
}

void main ()
{
    srand(time(NULL));

    // obtendo handle da janela do cmd
    HWND window = GetConsoleWindow();

    // criando buffer interno do console
    HANDLE screen_buffer_handle = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE,  
                                                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                             NULL,
                                                             CONSOLE_TEXTMODE_BUFFER, 
                                                             NULL);
    // fonte monospaced e quadrada
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = CHAR_SIZE;
    cfi.dwFontSize.Y = CHAR_SIZE;
    cfi.FontFamily = 0x30;
    cfi.FontWeight = 0;
    wcscpy(cfi.FaceName, L"Terminal");
    SetCurrentConsoleFontEx(screen_buffer_handle, FALSE, &cfi);

    // informando tamanho do buffer interno
    COORD buffer_size = {SCREEN_WIDTH, SCREEN_HEIGHT+DEBUG_LINE_COUNT};
    SetConsoleScreenBufferSize(screen_buffer_handle, buffer_size);
    
    // dimensoes da janela em caracteres
    SMALL_RECT write_rect = { 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT+DEBUG_LINE_COUNT-1 };
    SetConsoleWindowInfo(screen_buffer_handle, TRUE, &write_rect);
    
    // ocultando cursor
    CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(screen_buffer_handle, &cursor_info);
    cursor_info.bVisible = 0;
    SetConsoleCursorInfo(screen_buffer_handle, &cursor_info);
    
    // setando buffer criado como ativo no console
    SetConsoleActiveScreenBuffer(screen_buffer_handle);

    // buffer p/ uso do renderizador
    CHAR_INFO *buffer = (CHAR_INFO *)calloc((SCREEN_WIDTH*(SCREEN_HEIGHT+DEBUG_LINE_COUNT)), sizeof(CHAR_INFO));
    // limpando debug line
    for (u32 i = (SCREEN_WIDTH*SCREEN_HEIGHT)-1; i < (SCREEN_WIDTH*SCREEN_HEIGHT)-1+SCREEN_WIDTH; ++i)
    {
        buffer[i].Char.UnicodeChar = ' ';
        buffer[i].Attributes = RGBColor(1,1,1,1, 0,0,0,0);//(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
    }

    // Iniciando DirectSound
    u32 nchannels = 2; // dois channels de audio, esquerda e direita
    u32 samples_per_sec = 48000; // samples de audio por segundo
    u32 buffer_size_bytes = samples_per_sec*nchannels*sizeof(i16); // tamanho em bytes do buffer de audio 
    u32 ds_current_sample = 0; // sample atual no buffer do DS
    LPDIRECTSOUNDBUFFER ds_buffer; // buffer interno do DS
    if(InitDirectSound(&ds_buffer, window, nchannels, samples_per_sec, buffer_size_bytes))
    { // DS inicializado!
        DirectSoundWriteToBuffer(ds_buffer, buffer_size_bytes, &ds_current_sample, 
                                 0, samples_per_sec, samples_per_sec);
        ds_buffer->Play(0, 0, DSBPLAY_LOOPING);
    }

    CHAR_INFO player_char = {};
    player_char.Char.UnicodeChar = 207;
    player_char.Attributes = RGBColor(1,0,0,1, 0,0,0,0);//(FOREGROUND_RED|FOREGROUND_INTENSITY);

    // iniciando o contador de tempo do windows
    timeBeginPeriod(1);
    LARGE_INTEGER perf_frequency_i;
    QueryPerformanceFrequency(&perf_frequency_i);
    i64 perf_frequency = perf_frequency_i.QuadPart;
    r64 ms_since_last_s = 0.0;
    u32 frame_count = 0;
    i64 frame_start = GetTime();
    r32 dt = 0.0f;

    GameState game_state = {};

    while(!IS_KEY_DOWN(VK_ESCAPE))
    {

        GameUpdateAndRender(&game_state, buffer,dt);

        COORD buffer_coord = {0,0};
        PrintarBitMap(screen_buffer_handle, buffer, buffer_size, buffer_coord, write_rect);

        // output do som
        DirectSoundWriteToBuffer(ds_buffer, buffer_size_bytes, &ds_current_sample, 
                                 0, (samples_per_sec*dt), samples_per_sec);

        // contando frames, ms_for_frame = final do frame - inicio do frame
        ++frame_count;
        i64 frame_end = GetTime();
        r64 ms_for_frame = GetTimeElapsed(frame_start,frame_end, perf_frequency);


        if(ms_for_frame < TARGET_MS_PER_FRAME)
        {
            Sleep(TARGET_MS_PER_FRAME - ms_for_frame);
            frame_end = GetTime();
            ms_for_frame = GetTimeElapsed(frame_start,frame_end, perf_frequency);
        }


        dt = ms_for_frame/1000.0f;




        // contando os milissegundos desde o ultimo segundo, 
        // se 1s ou mais se passou, mostrar qtd de frames e zerar contagem
        ms_since_last_s += ms_for_frame;
        if(ms_since_last_s >= 1000.0)
        {
            // escrevendo fps na linha de debug
            char str[20];
            // escrever frame_count em str
            wsprintf(str, "%d FPS %d:MORTES %d:", frame_count, game_state.dead_count,game_state.gomos);
            // copiar str p/ linha de debug no buffer
            for (u32 i = 0; i < 20; ++i)
            {
                buffer[(SCREEN_WIDTH*SCREEN_HEIGHT)+i].Char.UnicodeChar = str[i];
                buffer[(SCREEN_WIDTH*SCREEN_HEIGHT)+i].Attributes = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
            }



            ms_since_last_s = 0.0;
            frame_count = 0;
        }

        frame_start = frame_end;
    }

    free(buffer);

}

#include "psychosnake.cpp"