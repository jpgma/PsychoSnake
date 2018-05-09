// #define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

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
    const u32 buffer_size_ = (SCREEN_WIDTH*(SCREEN_HEIGHT+DEBUG_LINE_COUNT)) * sizeof(CHAR_INFO);
    const u32 mem_size = sizeof(HANDLE) + sizeof(CONSOLE_FONT_INFOEX) + 
                         sizeof(COORD) + sizeof(SMALL_RECT) + sizeof(CONSOLE_CURSOR_INFO) + buffer_size_ + 
                         sizeof(GameState);
    u8 memoria[mem_size] = {}; // memoria para tudo no programa;
    u32 offset = 0;

    HANDLE *screen_buffer_handle = (HANDLE *)(memoria + offset);
    offset += sizeof(HANDLE);
    
    // criando buffer interno do console
    *screen_buffer_handle = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE,  
                                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                       NULL,
                                                       CONSOLE_TEXTMODE_BUFFER, 
                                                       NULL);
    
    CONSOLE_FONT_INFOEX *cfi = (CONSOLE_FONT_INFOEX *)(memoria + offset);
    offset += sizeof(CONSOLE_FONT_INFOEX);
    
    // fonte monospaced e quadrada
    cfi->cbSize = sizeof(CONSOLE_FONT_INFOEX);
    cfi->nFont = 0;
    cfi->dwFontSize.X = CHAR_SIZE;
    cfi->dwFontSize.Y = CHAR_SIZE;
    cfi->FontFamily = 0x30;
    cfi->FontWeight = 0;
    wcscpy(cfi->FaceName, L"Terminal");
    SetCurrentConsoleFontEx(*screen_buffer_handle, FALSE, cfi);

    COORD *buffer_size = (COORD*)(memoria + offset);
    offset += sizeof(COORD);

    // informando tamanho do buffer interno
    buffer_size->X = SCREEN_WIDTH;
    buffer_size->Y = SCREEN_HEIGHT+DEBUG_LINE_COUNT;
    SetConsoleScreenBufferSize(*screen_buffer_handle, *buffer_size);
    
    SMALL_RECT *write_rect = (SMALL_RECT*)(memoria + offset);
    offset += sizeof(SMALL_RECT);

    // dimensoes da janela em caracteres
    write_rect->Left = 0;
    write_rect->Top = 0;
    write_rect->Right = SCREEN_WIDTH-1;
    write_rect->Bottom = SCREEN_HEIGHT+DEBUG_LINE_COUNT-1;
    SetConsoleWindowInfo(*screen_buffer_handle, TRUE, write_rect);
    
    CONSOLE_CURSOR_INFO *cursor_info = (CONSOLE_CURSOR_INFO *)(memoria + offset);
    offset += sizeof(CONSOLE_CURSOR_INFO);

    // ocultando cursor
    GetConsoleCursorInfo(*screen_buffer_handle, cursor_info);
    cursor_info->bVisible = 0;
    SetConsoleCursorInfo(*screen_buffer_handle, cursor_info);
    
    // setando buffer criado como ativo no console
    SetConsoleActiveScreenBuffer(*screen_buffer_handle);

    // buffer p/ uso do renderizador
    CHAR_INFO *buffer = (CHAR_INFO *)(memoria + offset);
    offset += buffer_size_;

    // limpando debug line
    for (u32 i = (SCREEN_WIDTH*SCREEN_HEIGHT)-1; i < (SCREEN_WIDTH*SCREEN_HEIGHT)-1+SCREEN_WIDTH; ++i)
    {
        buffer[i].Char.UnicodeChar = ' ';
        buffer[i].Attributes = RGBColor(1,1,1,1, 0,0,0,0);//(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
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

    GameState *game_state = (GameState*)(memoria + offset);
    offset += sizeof(GameState);
    
    while(!IS_KEY_DOWN(VK_ESCAPE))
    {

        GameUpdateAndRender(game_state, buffer,dt);

        COORD buffer_coord = {0,0};
        PrintarBitMap(*screen_buffer_handle, buffer, *buffer_size, buffer_coord, *write_rect);

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
            wsprintf(str, "%d FPS %d:MORTES", frame_count, game_state->dead_count);
            // copiar str p/ linha de debug no buffer
            for (u32 i = 0; i < 20; ++i)
                buffer[(SCREEN_WIDTH*SCREEN_HEIGHT)+i].Char.UnicodeChar = str[i];


            ms_since_last_s = 0.0;
            frame_count = 0;
        }

        frame_start = frame_end;
    }
}

#include "psychosnake.cpp"