
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
GameUpdateAndRender (GameState *game_state, CHAR_INFO *buffer, r32 dt)
{


	if(!game_state->initialized)
	{
		game_state->posicao_x[game_state->gomos] = SCREEN_WIDTH/2.0f;
		game_state->posicao_y[game_state->gomos] = SCREEN_HEIGHT/2.0f;

		game_state->dead_count = 0;

		game_state->initialized = true;
	}



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

    	r32 player_speed = 10.0f;
    	r32 npx = game_state->posicao_x[0];
		r32 npy = game_state->posicao_y[0];
		i32 var_Local_X = 0;
		i32 var_Local_Y = 0;
		r32 nvx = game_state->velocidade_x;
		r32 nvy = game_state->velocidade_y;
	

		if(right)
		{
			nvx = 1.0;
			nvy = 0.0;
		}
		if(left)
		{
			nvx = -1.0;
			nvy = 0.0;
		}
		if(up)
		{
			nvx = 0.0;
			nvy = -1.0;
		}
		if(down)
		{
			nvx = 0.0;
			nvy = 1.0;
		}

		if(((nvx!=0.0f)||(nvy!=0.0f))&&(game_state->velocidade_x!=-(nvx))||(game_state->velocidade_y!=-(nvy)))
		{

			game_state->velocidade_x = nvx;
			game_state->velocidade_y = nvy;

		}


		npx += dt*game_state->velocidade_x*player_speed;
		npy += dt*game_state->velocidade_y*player_speed; 

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

			if(((u32)game_state->posicao_x[0]!=(u32)npx)||((u32)game_state->posicao_y[0]!=(u32)npy))
			{
				for(i32 i=game_state->gomos; i>0; i--)
				{
					game_state->posicao_x[i] = game_state->posicao_x[i-1];
					game_state->posicao_y[i] = game_state->posicao_y[i-1];
				}
			}

			game_state->posicao_x[0] = npx;
			game_state->posicao_y[0] = npy;

		}
		else
		{
			game_state->velocidade_x = 0.0;
			game_state->velocidade_y = 0.0;
			game_state->gomos=0;
			game_state->posicao_x[game_state->gomos] = ((SCREEN_WIDTH)/2);
			game_state->posicao_y[game_state->gomos] = ((SCREEN_HEIGHT)/2);

			game_state->dead_count += 1;
		}


		char player_char = '0';
		for(i32 i=game_state->gomos; i>=0; --i)
		{
			buffer[(u32)game_state->posicao_x[i]+((u32)game_state->posicao_y[i]*SCREEN_WIDTH)].Char.UnicodeChar = player_char;
			buffer[(u32)game_state->posicao_x[i]+((u32)game_state->posicao_y[i]*SCREEN_WIDTH)].Attributes = FOREGROUND_RED;	
		}	

		i32 SHUFFLE_X, SHUFFLE_Y;

		if(IsOccupied(food_map, (u32)game_state->posicao_x[0], (u32)game_state->posicao_y[0]))
		{
			SetMapBlock(food_map, (u32)game_state->posicao_x[0], (u32)game_state->posicao_y[0],0);

			game_state->gomos++;

		do
		{
			SHUFFLE_X = rand()%SCREEN_WIDTH;
			SHUFFLE_Y = rand()%(SCREEN_HEIGHT-1);


		}while(IsOccupied(wall_map, SHUFFLE_X, SHUFFLE_Y));

		SetMapBlock(food_map, SHUFFLE_X,SHUFFLE_Y,1);

		}


	}


	//desenhando food_map
	for (u32 y = 0; y < SCREEN_HEIGHT; ++y)
	{
		for (u32 x = 0; x < SCREEN_WIDTH; ++x)
		{
			if(IsOccupied(food_map,x,y))
			{
				buffer[x+(y*SCREEN_WIDTH)].Char.UnicodeChar = 254;
				buffer[x+(y*SCREEN_WIDTH)].Attributes = RGBColor(1,1,0,1, 0,0,0,0);
			}
		}
	}



}