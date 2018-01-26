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
#define true 1
#define false 0
#define internal static;
#define global static;

#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 16
#define CHAR_SIZE 16
#define DEBUG_LINE_COUNT 1

void LimparTela(CHAR_INFO *buffer, int clear_char, short clear_color)
{
	CHAR_INFO clear_char_info = {};
	clear_char_info.Char.UnicodeChar = clear_char;
	clear_char_info.Attributes = clear_color;
	
	for(int i = 0; i < (SCREEN_WIDTH*SCREEN_HEIGHT); i++)
	{
		buffer[i] = clear_char_info;
	}
}


void PrintarBitMap(HANDLE screen_buffer_handle, CHAR_INFO *buffer, 
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

#define WIN32_KEY_DOWN 0b1000000000000000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)

void main ()
{
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
	for (int i = (SCREEN_WIDTH*SCREEN_HEIGHT)-1; i < (SCREEN_WIDTH*SCREEN_HEIGHT)-1+SCREEN_WIDTH; ++i)
	{
		buffer[i].Char.UnicodeChar = ' ';
		buffer[i].Attributes = (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	}

	bool esc_down = false;		
	int estadoLocal_y = 0;
	int estadoLocal_x = 0;
	int x = 0, y = 0;

	timeBeginPeriod(1);
    LARGE_INTEGER perf_frequency_i;
	QueryPerformanceFrequency(&perf_frequency_i);
	i64 perf_frequency = perf_frequency_i.QuadPart;
	r64 ms_since_last_s = 0.0;
	u32 frame_count = 0;
	i64 frame_start = GetTime();

	while(!esc_down)
	{
		LimparTela(buffer, -80, (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE/*|FOREGROUND_INTENSITY*/));

		bool up = IS_KEY_DOWN(VK_UP);
		bool down = IS_KEY_DOWN(VK_DOWN);
		bool left = IS_KEY_DOWN(VK_LEFT);
		bool right = IS_KEY_DOWN(VK_RIGHT);

		if(up)
		{
			y++;
		} 
		else if(down)
		{
			y--;
		}

		COORD buffer_coord = {0,0};
		PrintarBitMap(screen_buffer_handle, buffer, buffer_size, buffer_coord, write_rect);
		
		esc_down = IS_KEY_DOWN(VK_ESCAPE);

		++frame_count;
		i64 frame_end = GetTime();
		r64 ms_for_frame = GetTimeElapsed(frame_start,frame_end, perf_frequency);

		ms_since_last_s += ms_for_frame;
		if(ms_since_last_s >= 1000.0)
		{
			// escrevendo fps na linha de debug
			char str[10];
			wsprintf(str, "%d fps", frame_count, 255);
			for (int i = 0; i < 10; ++i)
			{
				buffer[(SCREEN_WIDTH*SCREEN_HEIGHT)+i-1].Char.UnicodeChar = str[i];
			}

			ms_since_last_s = 0.0;
			frame_count = 0;
		}

		frame_start = frame_end;
	}

	free(buffer);
}