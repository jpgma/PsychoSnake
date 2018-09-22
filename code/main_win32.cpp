
///////////////////////
// Headers da CSTD e Windows

// TODO: eliminar no futuro!
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

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "bitmap_font.cpp"

#include "renderer.h"
#include "renderer_gdi.cpp"

global GDIRenderer *GDI;

///////////////////////
// Plataforma

#define WIN32_KEY_DOWN 0x8000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)

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
global b32 WINDOW_BORDERLESS = true;
global HWND GLOBAL_WINDOW_HANDLE;

internal void
SetWindowBorderless (b32 make_borderless)
{
    ShowWindow(GLOBAL_WINDOW_HANDLE, SW_HIDE);
    if(make_borderless)
    {   
        SetWindowLong(GLOBAL_WINDOW_HANDLE, GWL_STYLE, WS_TILED);
    }
    else
    {
        SetWindowLong(GLOBAL_WINDOW_HANDLE, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    }
    ShowWindow(GLOBAL_WINDOW_HANDLE, SW_SHOW);
}

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
                    WINDOW_BORDERLESS = !WINDOW_BORDERLESS;
                    SetWindowBorderless(WINDOW_BORDERLESS);
                break;

                case '1':
                case VK_NUMPAD1: GDI->render_mode = RENDER_MODE_SOLID; break;
                case '2':
                case VK_NUMPAD2: GDI->render_mode = RENDER_MODE_CODEPOINTS; break;
                case '3':
                case VK_NUMPAD3: GDI->render_mode = RENDER_MODE_FONT_GLYPHS; break;
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

#include "psychosnake.cpp"
// #include "shooterchar.cpp"

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
            ShowWindow(window, cmd_show);
            GLOBAL_WINDOW_HANDLE = window;
            SetWindowBorderless(WINDOW_BORDERLESS);
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

    Renderer *renderer = InitGDIRenderer(window, SCREEN_WIDTH, SCREEN_HEIGHT, DEBUG_LINE_COUNT, CHAR_SIZE);
    GDI = (GDIRenderer*)renderer->api;

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

    /////////
    // loop principal

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
                wsprintf(str, "[%d FPS]", frame_count);
                // copiar str p/ linha de debug no buffer
                WriteDebugText(renderer, (const char *)str, DBG_TEXT_COLOR,COLOR_BLACK);
                SetWindowText(window, str);

                ms_since_last_s = 0.0;
                frame_count = 0;
            }

            frame_start = frame_end;
        }
    }


    /////////
    // desalocando memoria do programa

    free(game_state);

    end:
    return 0;
}