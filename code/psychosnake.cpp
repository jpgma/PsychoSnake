
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

internal void 
GameUpdateAndRender (CHAR_INFO *buffer, int *dead_count, r32 *posicao_x, r32 *posicao_y, r32 dt)
{
	r32 npx = *posicao_x;
	r32 npy = *posicao_y;

	LimparTela(buffer, ' ', RGBColor(1,1,1,0, 0,0,0,0)/*(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE)*/);

	// desenhando mapa
	for (u32 y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (u32 x = 0; x < SCREEN_WIDTH; ++x)
		{
			if(IsOccupied(wall_map,x,y))
			{
				buffer[x+(y*SCREEN_WIDTH)].Char.UnicodeChar = GetWall(wall_map, thin_walls, x,y);
				buffer[x+(y*SCREEN_WIDTH)].Attributes = RGBColor(1,1,1,1, 0,0,0,0);
			}
		}
	}

	{ // movimentando player
		bool up = IS_KEY_DOWN(VK_UP);
		bool down = IS_KEY_DOWN(VK_DOWN);
		bool left = IS_KEY_DOWN(VK_LEFT);
		bool right = IS_KEY_DOWN(VK_RIGHT);
		bool space = IS_KEY_DOWN(VK_SPACE);

    	r32 player_speed = 10.0;

		if(right)
			npx += dt*player_speed;
		if(down)
			npy += dt*player_speed;
		if(left)
			npx -= dt*player_speed;
		if(up)
			npy -= dt*player_speed;

		if(npx < 0.0f)
			npx += SCREEN_WIDTH;
		else if(npx > SCREEN_WIDTH)
			npx -= SCREEN_WIDTH;
		if(npy < 0)
			npy += SCREEN_HEIGHT;
		else if(npy > SCREEN_HEIGHT)
			npy -= SCREEN_HEIGHT;

		if(!IsOccupied(wall_map,(u32)npx,(u32)npy))
		{
			*posicao_x = npx;
			*posicao_y = npy;
		}
		else
		{
			*posicao_x = ((SCREEN_WIDTH)/2);
			*posicao_y = ((SCREEN_HEIGHT)/2);
			*dead_count += 1;
		}

		char position_player = '0';

		buffer[(u32)*posicao_x+((u32)*posicao_y*SCREEN_WIDTH)].Char.UnicodeChar = position_player;
		buffer[(u32)*posicao_x+((u32)*posicao_y*SCREEN_WIDTH)].Attributes = FOREGROUND_RED;					
	}

}