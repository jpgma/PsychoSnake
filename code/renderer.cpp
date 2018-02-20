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

/* Caracteres p/ construçao de obstaculos:

	176 ░
	177 ▒
	178 ▓
	219 █
	220 ▄
	223 ▀
	254 ■

	250 ·
	217 ┘
	179 │
	192 └
	180 ┤
	193 ┴
	195 ├
	197 ┼
	191 ┐
	218 ┌
	194 ┬
	196 ─

	250 ·
	188 ╝
	186 ║
	200 ╚
	185 ╣
	202 ╩
	204 ╠
	206 ╬
	187 ╗
	201 ╔
	203 ╦
	205 ═

	exemplos:
	╔═════╦
	║
	╠═══╗  
	╚═══╝ 
*/

#define WALL_CENTER             0
#define WALL_UP_LEFT            1
#define WALL_UP_DOWN            2
#define WALL_UP_RIGHT           3
#define WALL_UP_DOWN_LEFT       4
#define WALL_UP_LEFT_RIGHT      5
#define WALL_UP_DOWN_RIGHT      6
#define WALL_UP_DOWN_LEFT_RIGHT 7
#define WALL_DOWN_LEFT          8
#define WALL_DOWN_RIGHT         9
#define WALL_DOWN_LEFT_RIGHT    10
#define WALL_LEFT_RIGHT         11

global u8 thin_walls[] = 
{
	250, 217, 179, 192, 180, 193, 
	195, 197, 191, 218, 194, 196
};

global u8 thick_walls[] = 
{
	254, 188, 186, 200, 185, 202, 
	204, 206, 187, 201, 203, 205,
};

global u32 bitfield_image[] = 
{
	0b11111111111111111111111111111111,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000000000000000000000000001,
	0b10000000111111111000000000000001,
	0b10000000000001000000000000000001,
	0b10000000000001001111110000000001,
	0b10000000111111001000010000000001,
	0b10000000000000001111110000000001,
	0b10000000000000001000000000000001,
	0b10000000000000001000000000000001,
	0b10000000000000001000000000000001,
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

internal b32
IsOccupied (u32 *map, u32 x, u32 y)
{
	b32 res = false;

	if((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
	{
		u32 row = map[y];
		u32 mask = (0b10000000000000000000000000000000 >> x);
		if((row & mask) == mask) res = true;
	}

	return res;
}

internal void
SetMapBlock (u32 *map, u32 x, u32 y, b32 value)
{
	if((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
	{
		u32 row = map[y];
		u32 mask = (0b10000000000000000000000000000000 >> x);
		if((row & mask) == mask)
		{
			if(!value) map[y] = map[y] & ~mask;
		}
		else if(value)
		{
			map[y] = map[y] | mask;
		}
	}
}

internal u8
GetWall (u32 *map, u8 *wall_set, u32 x, u32 y)
{
	u8 res = wall_set[WALL_CENTER];

	if((x < SCREEN_WIDTH) && (y < SCREEN_HEIGHT))
	{
		b32 up    = IsOccupied(map, x,y-1);
		b32 down  = IsOccupied(map, x,y+1);
		b32 left  = IsOccupied(map, x-1,y);
		b32 right = IsOccupied(map, x+1,y);

		if(up)
		{
			if(down)
			{
				if(left)
				{
					if(right) res = wall_set[WALL_UP_DOWN_LEFT_RIGHT];
					else res = wall_set[WALL_UP_DOWN_LEFT];
				}
				else if(right) res = wall_set[WALL_UP_DOWN_RIGHT];
				else res = wall_set[WALL_UP_DOWN];
			}
			else if(left)
			{
				if(right) res = wall_set[WALL_UP_LEFT_RIGHT];
				else res = wall_set[WALL_UP_LEFT];
			}
			else if(right) res = wall_set[WALL_UP_RIGHT];
			else res = wall_set[WALL_UP_DOWN];
		}
		else if(down)
		{
			if(left)
			{
				if(right) res = wall_set[WALL_DOWN_LEFT_RIGHT];
				else res = wall_set[WALL_DOWN_LEFT];
			}
			else if(right) res = wall_set[WALL_DOWN_RIGHT];
			else res = wall_set[WALL_UP_DOWN];
		}
		else if(left || right) res = wall_set[WALL_LEFT_RIGHT];
	}

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
	player_char.Char.UnicodeChar = 207;
	player_char.Attributes = RGBColor(1,0,0,1, 0,0,0,0);//(FOREGROUND_RED|FOREGROUND_INTENSITY);

	int estadoLocal_y = 0;
	int estadoLocal_x = 0;
	u32 t = 0;
	u32 x = 0, y = 0;
	b32 wasnt_down = true;

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

		// t++;
		// r32 period = 0.5f;
		// if((t%(u32)(TARGET_FPS * period)) == 0)
		// {
		// 	t = 0;
		// 	if(player_char.Attributes == RGBColor(1,0,0,1, 0,0,0,0))
		// 		player_char.Attributes = RGBColor(0,0,0,0, 1,0,0,1);//player_char.Char.UnicodeChar = 176;
		// 	else
		// 		player_char.Attributes = RGBColor(1,0,0,1, 0,0,0,0);
		// }

		if(IS_KEY_DOWN(VK_LBUTTON) && wasnt_down)
		{
			CONSOLE_SCREEN_BUFFER_INFO sbinfo;
			GetConsoleScreenBufferInfo(screen_buffer_handle, &sbinfo);

			u32 x = sbinfo.dwCursorPosition.X;
			u32 y = sbinfo.dwCursorPosition.Y;
			SetMapBlock(bitfield_image,x,y, !IsOccupied(bitfield_image,x,y));
			wasnt_down = false;
		}
		if(!IS_KEY_DOWN(VK_LBUTTON)) wasnt_down = true;

		for (u32 y = 0; y < SCREEN_HEIGHT; ++y)
		{
			for (u32 x = 0; x < SCREEN_WIDTH; ++x)
			{
				if(IsOccupied(bitfield_image, x,y))
				{
					player_char.Char.UnicodeChar = GetWall(bitfield_image, thick_walls, x,y);
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


		if(ms_for_frame < TARGET_MS_PER_FRAME)
		{
			Sleep(TARGET_MS_PER_FRAME - ms_for_frame);
			frame_end = GetTime();
			ms_for_frame = GetTimeElapsed(frame_start,frame_end, perf_frequency);
		}

		// contando os milissegundos desde o ultimo segundo, 
		// se 1s ou mais se passou, mostrar qtd de frames e zerar contagem
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