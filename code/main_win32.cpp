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
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef float r32;
typedef double r64;
typedef u32 b32;
#define internal static;
#define global static;
#define assert(x) {if(!(x)) *((int*)0) = 0;}

#define PI (3.1415927)

inline u32 
U32Swap(u32 n)
{ // funcao p/ inverter um u32 (endianness)
    n = ((n << 8) & 0xFF00FF00 ) | ((n >> 8) & 0xFF00FF ); 
    return (n << 16) | (n >> 16);
}

#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 16
#define CHAR_SIZE 32
#define DEBUG_LINE_COUNT 1

///////////////////////
// GDI Renderer


#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "bitmap_font.cpp"

#include "renderer.h"

#define RENDER_MODE_SOLID       1
#define RENDER_MODE_CODEPOINTS  2
#define RENDER_MODE_FONT_GLYPHS 3
global u32 GLOBAL_RENDER_MODE = 2;

internal RenderBuffer 
AllocRenderBuffer (u16 width, u16 height, u16 debug_lines)
{
    RenderBuffer res = {};
    
    res.width = width;
    res.height = height;
    res.debug_lines = debug_lines;
    
    u32 char_count = (width*(height+debug_lines));
    res.codepoints = (u32*)calloc(char_count,sizeof(u32));
    res.foreground_colors = (Color*)calloc(char_count,sizeof(Color));
    res.background_colors = (Color*)calloc(char_count,sizeof(Color));
    
    return res;
}

internal void 
FreeRenderBuffer (RenderBuffer *buffer)
{
    buffer->width = 0;
    buffer->height = 0;
    buffer->debug_lines = 0;    
    free(buffer->codepoints);
    free(buffer->foreground_colors);
    free(buffer->background_colors);
}

// TODO: colocar no renderer->api
global HDC        GLOBAL_WINDOW_HDC;
global BITMAPINFO GLOBAL_BITMAP_INFO;
global HWND       GLOBAL_WINDOW_HANDLE;
global u8 *       GLOBAL_BITMAP_MEMORY;
global u32        GLOBAL_BITMAP_WIDTH;
global u32        GLOBAL_BITMAP_HEIGHT;

internal void 
RenderBufferToScreen (Renderer *renderer)
{
    u32 src_pixels_width = renderer->font->glyph_count_x * renderer->font->glyph_width;
    u32 src_pixels_height = renderer->font->glyph_count_y * renderer->font->glyph_height;
    u8 *src_pixels = PIXEL_DATA(renderer->font);
    u32 *dst_pixels = (u32*)GLOBAL_BITMAP_MEMORY;
    
    // preenchendo bitmap com charmap
    switch(GLOBAL_RENDER_MODE)
    {
        case RENDER_MODE_SOLID:
        {
            for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
            {
                for (u16 x = 0; x < renderer->buffer.width; ++x)
                {
                    Color color = renderer->buffer.foreground_colors[x+(y*renderer->buffer.width)];
                    for (s32 yy = 0; yy < renderer->char_size; ++yy)
                    {   
                        for (s32 xx = 0; xx < renderer->char_size; ++xx)
                        {
                            u32 index = ((x*renderer->char_size)+xx) + (((y*renderer->char_size)+yy)*(renderer->char_size*renderer->buffer.width));
                            dst_pixels[index] = (color.value);
                        }
                    }
                }
            }
        } break;


        case RENDER_MODE_CODEPOINTS:
        {
            for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
            {
                for (u16 x = 0; x < renderer->buffer.width; ++x)
                {
                    u32 codepoint = renderer->buffer.codepoints[x+(y*renderer->buffer.width)];
                    u32 glyph_offset = GetGlyphOffset(renderer->font, codepoint);
                    Color foreground = renderer->buffer.foreground_colors[x+(y*renderer->buffer.width)];
                    Color background = renderer->buffer.background_colors[x+(y*renderer->buffer.width)];
                    for (s32 yy = 0; yy < renderer->char_size; ++yy)
                    {   
                        for (s32 xx = 0; xx < renderer->char_size; ++xx)
                        {
                            u8 src_value = 0;
                            u32 src_index = glyph_offset + (xx + (yy*(renderer->font->glyph_width*renderer->font->glyph_count_x)));
                            u32 dst_index = ((x*renderer->char_size)+xx) + 
                                             (((y*renderer->char_size)+yy)*(renderer->char_size*renderer->buffer.width));
                            if(src_index < (src_pixels_width*src_pixels_height))
                            {
                                r32 src_value = src_pixels[src_index]/255.0f;
                                dst_pixels[dst_index] = COLOR((src_value*foreground.r) + ((1.0f-src_value)*background.r),
                                                              (src_value*foreground.g) + ((1.0f-src_value)*background.g),
                                                              (src_value*foreground.b) + ((1.0f-src_value)*background.b), 255).value;
                                // if(src_value > 0)
                                // {
                                //     dst_pixels[dst_index] = COLOR(src_value*foreground.r,
                                //                                   src_value*foreground.g,
                                //                                   src_value*foreground.b, 255).value;
                                // }
                                // else
                                // {
                                //     dst_pixels[dst_index] = COLOR(src_value*background.r,
                                //                                   src_value*background.g,
                                //                                   src_value*background.b, 255).value;
                                // }
                            }
                        }
                    }
                }
            }
        } break;


        case RENDER_MODE_FONT_GLYPHS:
        {
            for (u32 y = 0; y < GLOBAL_BITMAP_HEIGHT; ++y)
            {
                for (u32 x = 0; x < GLOBAL_BITMAP_WIDTH; ++x)
                {
                    u8 src_value = 0;
                    if((x < src_pixels_width) &&
                        (y < src_pixels_height))
                    {
                        src_value = src_pixels[x+(y*src_pixels_width)];
                    }
                    Color color = COLOR(src_value,src_value,src_value,255);
                    dst_pixels[x+(y*GLOBAL_BITMAP_WIDTH)] = color.value;
                }
            }
        } break;
    }

    // colocando bitmap na tela
    RECT rect = {};
    GetClientRect(GLOBAL_WINDOW_HANDLE,&rect);
    s32 window_width = rect.right - rect.left;
    s32 window_height = rect.bottom - rect.top;
    StretchDIBits(GLOBAL_WINDOW_HDC,
                    0,0,window_width,window_height,
                    0,0,GLOBAL_BITMAP_WIDTH,GLOBAL_BITMAP_HEIGHT,
                    (void*)GLOBAL_BITMAP_MEMORY,
                    &GLOBAL_BITMAP_INFO,
                    DIB_RGB_COLORS, SRCCOPY);
    // SwapBuffers(GLOBAL_WINDOW_HDC);
    // UpdateWindow(GLOBAL_WINDOW_HANDLE);
}

#define WIN32_KEY_DOWN 0x8000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)

//Limitando o FPS
#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

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
                    waveformatex.wBitsPerSample = sizeof(s16) * 8;
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
DirectSoundWriteToBuffer (LPDIRECTSOUNDBUFFER ds_buffer, u32 ds_buffer_size, u32 *ds_current_sample, s16 *internal_buffer, u32 samples_to_write, u32 samples_per_sec)
{
    // TODO: passar infos do buffer
    u32 ds_sample_index = *ds_current_sample;
    u32 ds_buffer_bytes_per_sample = 2*sizeof(s16);
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

        r32 hz = 73.42f;
        s16 volume = 10000;

        s16 *dest_sample = (s16*)region_1;
        u32 region_1_count = (region_1_size/ds_buffer_bytes_per_sample);
        for (s32 sample_index = 0; 
             sample_index < region_1_count; 
             ++sample_index)
        {
            r32 t = (2.0f*PI*(r32)ds_sample_index++) / (r32)(samples_per_sec/hz);
            s16 sample_value = (s16) (sinf(t)*volume);

            *(dest_sample++) = sample_value;//*(src_sample++);
            *(dest_sample++) = sample_value;//*(src_sample++);
        }
        dest_sample = (s16*)region_2;
        u32 region_2_count = (region_2_size/ds_buffer_bytes_per_sample);
        for (s32 sample_index = 0; 
             sample_index < region_2_count; 
             ++sample_index)
        {
            r32 t = (2.0f*PI*(r32)ds_sample_index++) / (r32)(samples_per_sec/hz);
            s16 sample_value = (s16) (sinf(t)*volume);

            *(dest_sample++) = sample_value;//*(src_sample++);
            *(dest_sample++) = sample_value;//*(src_sample++);
        }
        
        *ds_current_sample += region_1_count + region_2_count;

        ds_buffer->Unlock(region_1,region_1_size, region_2,region_2_size);
    }
}

///////////////////////
// Plataforma

internal s64
GetTime ()
{
    LARGE_INTEGER res;
    QueryPerformanceCounter(&res);
    return res.QuadPart;
}

internal r64
GetTimeElapsed (s64 a, s64 b, s64 perf_frequency)
{
    r64 res = (1000.0 * (b - a)) /
              (r64) perf_frequency;
    return res;
}

global b32 QUIT_REQUESTED = false;
global b32 WINDOW_ACTIVE = true;
global b32 WINDOW_BORDERLESS = false;

LRESULT CALLBACK 
WindowProcedure(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch(msg)
    {
        case WM_KEYDOWN:
        {
            switch (wparam) 
            { 
                case VK_F2:
                    if(WINDOW_BORDERLESS)
                    {
                        ShowWindow(window, SW_HIDE);
                        SetWindowLong(window, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                        ShowWindow(window, SW_SHOW);
                        WINDOW_BORDERLESS = false;
                    }
                    else
                    {
                        ShowWindow(window, SW_HIDE);
                        SetWindowLong(window, GWL_STYLE, WS_TILED);
                        ShowWindow(window, SW_SHOW);
                        WINDOW_BORDERLESS = true;
                    }
                break;

                case '1':
                case VK_NUMPAD1: GLOBAL_RENDER_MODE = RENDER_MODE_SOLID; break;
                case '2':
                case VK_NUMPAD2: GLOBAL_RENDER_MODE = RENDER_MODE_CODEPOINTS; break;
                case '3':
                case VK_NUMPAD3: GLOBAL_RENDER_MODE = RENDER_MODE_FONT_GLYPHS; break;
            }
        } 

        case WM_ACTIVATE:
            if((wparam == WA_ACTIVE) || 
               (wparam == WA_CLICKACTIVE))
                WINDOW_ACTIVE = true;
            else if(wparam == WA_INACTIVE)
                WINDOW_ACTIVE = false;
        break;
        
        case WM_CLOSE:
        case WM_DESTROY: 
            QUIT_REQUESTED = true;
        break;

        default: return DefWindowProcA(window, msg, wparam, lparam);
    }

    return 0;
}

#include "psychosnake.h"

#define DBG_TEXT_COLOR   COLOR(59,120,255,255)

s32 WINAPI
WinMain (HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd, s32 cmd_show)
{
    srand(time(NULL));

    HWND window;
    WNDCLASSEX wc;
    const char szClassName[] = "GameWindClass";
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    wc.lpfnWndProc   = WindowProcedure;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = instance;
    wc.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursorA(NULL, IDC_CROSS);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;
    wc.hIconSm       = (HICON)LoadImageA(NULL, MAKEINTRESOURCE(IDI_APPLICATION),
                                         IMAGE_ICON, 0, 0,
                                         LR_DEFAULTCOLOR|LR_SHARED|LR_DEFAULTSIZE);
    if(RegisterClassExA(&wc))
    {
        s32 window_width = (SCREEN_WIDTH*CHAR_SIZE);
        s32 window_height = (SCREEN_HEIGHT*CHAR_SIZE);
     
        s32 monitor_width = GetSystemMetrics(SM_CXSCREEN);
        s32 monitor_height = GetSystemMetrics(SM_CYSCREEN);
        s32 x = (monitor_width*0.5f)-(window_width*0.5f);
        s32 y = (monitor_height*0.5f)-(window_height*0.5f);
     
        window = CreateWindowExA(WS_EX_CLIENTEDGE,
                                    szClassName,
                                    "PsychoSnake",
                                    WS_OVERLAPPEDWINDOW,
                                    x, y, 
                                    window_width, window_height,
                                    NULL, NULL, instance, NULL);
        if(window)
        {
            // SetWindowLong(window, GWL_STYLE, 0); //remove all window styles   
            ShowWindow(window, cmd_show);
            GLOBAL_WINDOW_HANDLE = window;
            GLOBAL_WINDOW_HDC = GetDC(window);
        }
        else
        {
            MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONERROR|MB_OK);
            goto end;
        }

    }
    else
    {
        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONERROR|MB_OK);
        goto end;
    }

    Renderer *renderer = (Renderer*)calloc(1,sizeof(Renderer));

    u16 screen_width = SCREEN_WIDTH;
    u16 screen_height = SCREEN_HEIGHT;
    u16 debug_lines = DEBUG_LINE_COUNT;
    u16 char_size = CHAR_SIZE;

    renderer->buffer = AllocRenderBuffer(screen_width, screen_height, debug_lines);
    renderer->char_size = char_size;
    
    GLOBAL_BITMAP_WIDTH = screen_width * char_size;
    GLOBAL_BITMAP_HEIGHT = (screen_height+debug_lines) * char_size;
    
    u32 bytes_per_pixel = 4;
    GLOBAL_BITMAP_INFO.bmiHeader.biSize = sizeof(GLOBAL_BITMAP_INFO.bmiHeader);
    GLOBAL_BITMAP_INFO.bmiHeader.biWidth = GLOBAL_BITMAP_WIDTH;
    GLOBAL_BITMAP_INFO.bmiHeader.biHeight = -GLOBAL_BITMAP_HEIGHT;
    GLOBAL_BITMAP_INFO.bmiHeader.biPlanes = 1;
    GLOBAL_BITMAP_INFO.bmiHeader.biBitCount = (bytes_per_pixel*8);
    GLOBAL_BITMAP_INFO.bmiHeader.biCompression = BI_RGB;
    
    GLOBAL_BITMAP_MEMORY = (u8*)calloc((GLOBAL_BITMAP_WIDTH*GLOBAL_BITMAP_HEIGHT),bytes_per_pixel);
    
    { // gerando fonte
        UnicodeBlock unicode_blocks[] = 
        {
            UNICODE_BLOCK_BASIC_LATIN,
            // UNICODE_BLOCK_LATIN_1_SUPPLEMENT,
            // UNICODE_BLOCK_GENERAL_PUNCTUATION,
            // UNICODE_BLOCK_SUPERSCRIPS_SUBSCRIPTS,
            // UNICODE_BLOCK_NUMBER_FORMS,
            // UNICODE_BLOCK_ARROWS,
            // UNICODE_BLOCK_MATHEMATICAL_OPERATORS,
            UNICODE_BLOCK_BOX_DRAWING,
            UNICODE_BLOCK_BLOCK_ELEMENTS,
            UNICODE_BLOCK_GEOMETRIC_SHAPES,
            UNICODE_BLOCK_GEOMETRIC_SHAPES_EXTENDED,
            // UNICODE_BLOCK_BRAILLE_PATTERNS,
            // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_A,
            // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_B,
            // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_C,
        };
        //seguisym.ttf
        //unifont-11.0.01.ttf
        //proggy.ttf
        //dejavu.ttf
        //blockway.ttf
        //andale_mono.ttf
        //SourceCodePro-Black.otf
        //SourceCodePro-Light.otf
        //SourceCodePro-Regular.otf
        u32 block_count = sizeof(unicode_blocks)/sizeof(UnicodeBlock*);
        renderer->font = GenerateBitmapFont("data\\seguisym.ttf", 
                                              unicode_blocks, block_count, 
                                              BFNT_PIXEL_FORMAT_ALPHA8, 
                                              renderer->char_size, false);
        if(!renderer->font)
        {
            MessageBoxA(NULL, "Erro no carregamento da fonte!", "Error!", MB_ICONERROR|MB_OK);
        }
    }

    // Iniciando DirectSound
    // u32 nchannels = 2; // dois channels de audio, esquerda e direita
    // u32 samples_per_sec = 48000; // samples de audio por segundo
    // u32 buffer_size_bytes = samples_per_sec*nchannels*sizeof(s16); // tamanho em bytes do buffer de audio 
    // u32 ds_current_sample = 0; // sample atual no buffer do DS
    // LPDIRECTSOUNDBUFFER ds_buffer; // buffer interno do DS
    // if(InitDirectSound(&ds_buffer, window, nchannels, samples_per_sec, buffer_size_bytes))
    // { // DS inicializado!
    //     DirectSoundWriteToBuffer(ds_buffer, buffer_size_bytes, &ds_current_sample, 
    //                              0, samples_per_sec, samples_per_sec);
    //     ds_buffer->Play(0, 0, DSBPLAY_LOOPING);
    // }

    GameState *game_state = (GameState*)calloc(1,sizeof(GameState));

    // iniciando o contador de tempo do windows
    timeBeginPeriod(1);
    LARGE_INTEGER perf_frequency_i;
    QueryPerformanceFrequency(&perf_frequency_i);
    s64 perf_frequency = perf_frequency_i.QuadPart;
    r64 ms_since_last_s = 0.0;
    u32 frame_count = 0;
    s64 frame_start = GetTime();
    r32 dt = 0.0f;

    b32 running = true;
    while(running)
    {
        MSG msg;
        BOOL res;
        while((res = PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if(IS_KEY_DOWN(VK_ESCAPE) || QUIT_REQUESTED) running = false;

        if(WINDOW_ACTIVE)
        {
            GameUpdateAndRender(game_state,renderer,dt);

            RenderBufferToScreen(renderer);


            // output do som
            // DirectSoundWriteToBuffer(ds_buffer, buffer_size_bytes, &ds_current_sample, 
            //                          0, (samples_per_sec*dt), samples_per_sec);

            // contando frames, ms_for_frame = final do frame - inicio do frame
            ++frame_count;
            s64 frame_end = GetTime();
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
                char str[SCREEN_WIDTH];
                // escrever frame_count em str
                wsprintf(str, "[%d FPS] [%d MORTES] [%d GOMOS]", frame_count, game_state->dead_count,game_state->gomos);
                // copiar str p/ linha de debug no buffer
                WriteDebugText(renderer, (const char *)str, DBG_TEXT_COLOR,COLOR_BLACK);
                SetWindowText(window, str);

                ms_since_last_s = 0.0;
                frame_count = 0;
            }

            frame_start = frame_end;
        }
    }

    FreeRenderBuffer(&renderer->buffer);
    free(renderer->font);
    // free(renderer->api);
    free(renderer);

    free(game_state);

    end:
    return 0;
}

#include "psychosnake.cpp"
