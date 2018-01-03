#include <stdio.h>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 10

static char LOADED_BITMAP[] = 
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

void LimparTela()
{
	char bitmap [SCREEN_HEIGHT*SCREEN_WIDTH];
	for(int y = 1; y <= SCREEN_HEIGHT; y++)
	{
		for(int x = 1; x <= SCREEN_WIDTH; x++)
		{	

			bitmap[x+(y*SCREEN_HEIGHT)] = -80;
		}
	}
}


void PrintarBitMap()
{
	char bitmap [SCREEN_HEIGHT*SCREEN_WIDTH];
	for(int y = 1; y <= SCREEN_HEIGHT; y++)
	{
		for(int x = 1; x <= SCREEN_WIDTH; x++)
		{
			HANDLE hOutput = (HANDLE)GetStdHandle( STD_OUTPUT_HANDLE );

			COORD dwBufferSize = { SCREEN_WIDTH,SCREEN_HEIGHT };
			COORD dwBufferCoord = { 0, 0 };
			SMALL_RECT rcRegion = { 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1 };

			CHAR_INFO buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

			ReadConsoleOutput( hOutput, (CHAR_INFO *)buffer, dwBufferSize,
			dwBufferCoord, &rcRegion );

			buffer[y][x].Attributes = 0x0E;

			WriteConsoleOutput( hOutput, (CHAR_INFO *)buffer, dwBufferSize,
			dwBufferCoord, &rcRegion );
			printf("%c", bitmap[x+(y*SCREEN_HEIGHT)]);
		}
		printf("\n");
	}
}
		

int main ()
{

	
	LimparTela();
	PrintarBitMap();


	return 0;
}