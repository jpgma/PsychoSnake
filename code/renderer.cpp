#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 16
#define CHAR_SIZE 8

void LimparTela(CHAR_INFO *buffer, char clear_char, short clear_color)
{
	CHAR_INFO clear_char_info = {};
	clear_char_info.Char.UnicodeChar = clear_char;
	clear_char_info.Attributes = clear_color;
	
	for(int i = 0; i <= (SCREEN_WIDTH*SCREEN_HEIGHT); i++)
	{
		buffer[i] = clear_char_info;
	}
}


void PrintarBitMap(HANDLE screen_buffer_handle, CHAR_INFO *buffer, 
				   COORD buffer_size, COORD buffer_coord, SMALL_RECT write_rect)
{
	WriteConsoleOutput(screen_buffer_handle, buffer, buffer_size, buffer_coord, &write_rect);
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
	COORD buffer_size = {SCREEN_WIDTH, SCREEN_HEIGHT};
	SetConsoleScreenBufferSize(screen_buffer_handle, buffer_size);
	
	// dimensoes da janela em caracteres
	SMALL_RECT write_rect = { 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1 };
	SetConsoleWindowInfo(screen_buffer_handle, TRUE, &write_rect);
	
    // ocultando cursor
	CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(screen_buffer_handle, &cursor_info);
    cursor_info.bVisible = 0;
    SetConsoleCursorInfo(screen_buffer_handle, &cursor_info);
	
	// setando buffer criado como ativo no console
	SetConsoleActiveScreenBuffer(screen_buffer_handle);

	// buffer p/ uso do renderizador
	CHAR_INFO *buffer = (CHAR_INFO *)calloc((SCREEN_WIDTH*SCREEN_HEIGHT), sizeof(CHAR_INFO));

	bool esc_down = false;		
	int estadoLocal_y = 0;
	int estadoLocal_x = 0;
	int x = 0, y = 0;
	while(!esc_down)
	{
		LimparTela(buffer, -80, (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|BACKGROUND_INTENSITY));

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
	}
}