#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 10

static char CLEAR_CHARMAP[] = 
{
	'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
	'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
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
			buffer[x+(y*SCREEN_HEIGHT)].Char.UnicodeChar = clear_char;
			buffer[x+(y*SCREEN_HEIGHT)].Attributes = BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY;
		}
	}
}


void PrintarBitMap(HANDLE screen_buffer_handle, CHAR_INFO *buffer, 
				   COORD buffer_size, COORD buffer_coord, SMALL_RECT rcRegion)
{
	WriteConsoleOutput(screen_buffer_handle, buffer, buffer_size, buffer_coord, &rcRegion);
}


#define WIN32_KEY_DOWN 0b1000000000000000
#define IS_KEY_DOWN(key) ((GetAsyncKeyState(key) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)

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

	bool esc_down = false;		
	int estadoLocal_y = 0;
	int estadoLocal_x = 0;
	int x =0, y = 0;
	while(!esc_down)
	{
		LimparTela(buffer);

		esc_down = IS_KEY_DOWN(VK_ESCAPE);

		bool up = IS_KEY_DOWN(VK_UP);
		bool down = IS_KEY_DOWN(VK_DOWN);
		bool left = IS_KEY_DOWN(VK_LEFT);
		bool right = IS_KEY_DOWN(VK_RIGHT);

		if(up)
		{
			
			buffer[x+(y*SCREEN_HEIGHT)].Char.UnicodeChar = -86;
			buffer[x+(y*SCREEN_HEIGHT)].Attributes = BACKGROUND_RED|BACKGROUND_INTENSITY;
		} 
		else if(down)
		{
			buffer[x+(y*SCREEN_HEIGHT)].Char.UnicodeChar = -80;
			buffer[x+(y*SCREEN_HEIGHT)].Attributes = BACKGROUND_GREEN|BACKGROUND_INTENSITY;

			estadoLocal_y = estadoLocal_y + 1;
			y = estadoLocal_y;

		}
		/*else if(left)
		{
			
			buffer[x+(y*SCREEN_HEIGHT)].Char.UnicodeChar = -80;
			buffer[x+(y*SCREEN_HEIGHT)].Attributes = BACKGROUND_GREEN|BACKGROUND_RED|BACKGROUND_INTENSITY;

		}
		else if(right)
		{

			buffer[(x+1)+(y*SCREEN_HEIGHT)].Char.UnicodeChar = -80;
			buffer[(x+1)+(y*SCREEN_HEIGHT)].Attributes = BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY;

		}*/

		PrintarBitMap(screen_buffer_handle, buffer, buffer_size, buffer_coord, rcRegion);
	}

	// if((GetAsyncKeyState(tecla) & WIN32_KEY_DOWN) == WIN32_KEY_DOWN)
	// {

	// }

	return 0;
}