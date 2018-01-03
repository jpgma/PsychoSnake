#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 10

static char CLEAR_CHARMAP[] = 
{
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'
};

void LimparTela(CHAR_INFO *buffer)
{
	for(int y = 0; y <= SCREEN_HEIGHT; y++)
	{
		for(int x = 0; x <= SCREEN_WIDTH; x++)
		{
			char clear_char = CLEAR_CHARMAP[x+(y*SCREEN_HEIGHT)];

			CHAR_INFO ci = {};
			ci.Char.UnicodeChar = clear_char;
			ci.Char.AsciiChar = clear_char;
			ci.Attributes = BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY;
			buffer[x+(y*SCREEN_HEIGHT)] = ci;
		}
	}
}


void PrintarBitMap(HANDLE screen_buffer_handle, CHAR_INFO *buffer, 
				   COORD buffer_size, COORD buffer_coord, SMALL_RECT rcRegion)
{
	WriteConsoleOutput(screen_buffer_handle, buffer, buffer_size, buffer_coord, &rcRegion);
}
		

int main ()
{
	SMALL_RECT rcRegion = { 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1 };
	COORD buffer_size = {SCREEN_WIDTH, SCREEN_HEIGHT};
	COORD buffer_coord = {0,0};
	HANDLE screen_buffer_handle = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE,  
					   										 FILE_SHARE_READ | FILE_SHARE_WRITE,
					   										 NULL,
					   										 CONSOLE_TEXTMODE_BUFFER, 
       														 NULL);
	SetConsoleWindowInfo(screen_buffer_handle, TRUE, &rcRegion);
	SetConsoleScreenBufferSize(screen_buffer_handle, buffer_size);
	SetConsoleActiveScreenBuffer(screen_buffer_handle);

	CHAR_INFO buffer[SCREEN_WIDTH*SCREEN_HEIGHT];

	LimparTela(buffer);
	PrintarBitMap(screen_buffer_handle, buffer, buffer_size, buffer_coord, rcRegion);

	int x;
	scanf("%d",&x);

	return 0;
}