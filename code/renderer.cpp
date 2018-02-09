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

#define WIN32_KEY_DOWN 0x8000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)



//Limitando o FPS
#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)



global u32 bitfield_image[] = 
{
	0b11111111111111111111111111111111,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b11111111111111111111111111111111,
};

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

	CHAR_INFO player_char = {};
	player_char.Char.UnicodeChar = 3;
	player_char.Attributes = RGBColor(1,0,0,1, 0,0,0,0);//(FOREGROUND_RED|FOREGROUND_INTENSITY);

	int estadoLocal_y = 0;
	int estadoLocal_x = 0;
	u32 x = 0, y = 0;

	// iniciando o contador de tempo do windows
	timeBeginPeriod(1);
    LARGE_INTEGER perf_frequency_i;
	QueryPerformanceFrequency(&perf_frequency_i);
	i64 perf_frequency = perf_frequency_i.QuadPart;
	r64 ms_since_last_s = 0.0;
	u32 frame_count = 0;
	i64 frame_start = GetTime();

	while(!IS_KEY_DOWN(VK_ESCAPE))
	{
		LimparTela(buffer, ' ', RGBColor(1,1,1,0, 0,0,0,0)/*(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)*/);

		bool up = IS_KEY_DOWN(VK_UP);
		bool down = IS_KEY_DOWN(VK_DOWN);
		bool left = IS_KEY_DOWN(VK_LEFT);
		bool right = IS_KEY_DOWN(VK_RIGHT);

		for (u32 y = 0; y < SCREEN_HEIGHT; ++y)
		{
			u32 row = bitfield_image[y];
			for (u32 x = 0; x < SCREEN_WIDTH; ++x)
			{
				u32 mask = (0b10000000000000000000000000000000 >> x);
				if((row & mask) == mask)
				{
					buffer[x+(y*SCREEN_WIDTH)] = player_char;
				}
			}
		}


		COORD buffer_coord = {0,0};
		PrintarBitMap(screen_buffer_handle, buffer, buffer_size, buffer_coord, write_rect);

		// contando frames, ms_for_frame = final do frame - inicio do frame
		++frame_count;
		i64 frame_end = GetTime();
		r64 ms_for_frame = GetTimeElapsed(frame_start,frame_end, perf_frequency);

		// contando os milissegundos desde o ultimo segundo, 
		// se 1s ou mais se passou, mostrar qtd de frames e zerar contagem
		if(ms_for_frame < TARGET_MS_PER_FRAME)
		{
			Sleep(TARGET_MS_PER_FRAME - ms_for_frame);
			frame_end = GetTime();
			ms_for_frame = GetTimeElapsed(frame_start,frame_end, perf_frequency);
		}



		ms_since_last_s += ms_for_frame;

		if(ms_since_last_s >= 1000.0)
		{
			// escrevendo fps na linha de debug
			char str[10];
			// escrever frame_count em str
			wsprintf(str, "%d fps", frame_count);
			// copiar str p/ linha de debug no buffer
			for (u32 i = 0; i < 10; ++i)
				buffer[(SCREEN_WIDTH*SCREEN_HEIGHT)+i].Char.UnicodeChar = str[i];

			ms_since_last_s = 0.0;
			frame_count = 0;
		}

		frame_start = GetTime();
	}

	free(buffer);
}