
#include "psychosnake.h"

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

#define DBG_TEXT_COLOR   COLOR(59,120,255,255)


enum FoodType 
{ 
    FOODTYPE0, 
    FOODTYPE1, 
    FOODTYPE2, 
    FOODTYPE3, 
    FOODTYPE4, 
    FOODTYPE5
};

enum SpaceBlockType
{
    SPACEBLOCKTYPE_EMPTY,
    SPACEBLOCKTYPE_FULL_WALL,
};

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

internal u32
GetWall (u32 *map, u32 *wall_set, u32 x, u32 y)
{
    u32 res = wall_set[WALL_CENTER];

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
ResetSnake (GameState *game_state)
{
    game_state->gomos = 1;
    for (int i = 0; i < game_state->gomos+1; ++i)
    {
        game_state->posicao_x[i] = (SCREEN_WIDTH/2.0f) - i;
        game_state->posicao_y[i] = SCREEN_HEIGHT/2.0f;
    }
    game_state->dead_count = 0;
    game_state->velocidade_x = 0.0f;
    game_state->velocidade_y = 0.0f;
}


#define CLASSIC_COLOR_BACKGROUND COLOR( 30, 30, 30,255)
#define CLASSIC_COLOR_FOREGROUND COLOR(249,241,241,255)

#define SNAKE_COLOR      COLOR( 19,161, 14,255)
#define FOOD_COLOR       COLOR(249,241,165,255)
#define WALL_COLOR       COLOR(242,242,242,255)
#define BACKGROUND_COLOR COLOR( 12, 12, 12,255)

internal void 
GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt)
{
    if(!game_state->initialized)
    {
        ResetSnake(game_state);
        
        game_state->food_px = game_state->posicao_x[0] + 1;
        game_state->food_py = game_state->posicao_y[0] + 1;

        // for (u32 y = 0; y < SCREEN_HEIGHT; ++y)
        // {
        //     for (u32 x = 0; x < SCREEN_WIDTH; ++x)
        //     {
        //         if(((x > 39)&&(x < 56)) && ((y >= 16)&&(y < 32)))
        //         {
        //             game_state->space_block_type[x+(y*SCREEN_WIDTH)] = SPACEBLOCKTYPE_EMPTY;
        //         }
        //         else
        //         {
        //             game_state->space_block_type[x+(y*SCREEN_WIDTH)] = SPACEBLOCKTYPE_FULL_WALL;
        //         }
        //     }
        // }

        game_state->initialized = true;
    }

    ClearRenderBuffer(&renderer->buffer, ' ', CLASSIC_COLOR_BACKGROUND,CLASSIC_COLOR_BACKGROUND);

    { // movimentando player
        bool up = IS_KEY_DOWN(VK_UP);
        bool down = IS_KEY_DOWN(VK_DOWN);
        bool left = IS_KEY_DOWN(VK_LEFT);
        bool right = IS_KEY_DOWN(VK_RIGHT);
        bool space = IS_KEY_DOWN(VK_SPACE);
        bool pause = IS_KEY_DOWN(0x50);

        r32 player_speed = 5.0f;
        // if(game_state->gomos >= 10)
        // {
        //     player_speed = 15.0f;
        // }
        r32 npx = game_state->posicao_x[0];
        r32 npy = game_state->posicao_y[0];
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


        //MOVIMENTO DA SNAKE(GOMO BY GOMO)
        if(game_state->space_block_type[((u32)npx)+(((u32)npy)*SCREEN_WIDTH)] == SPACEBLOCKTYPE_EMPTY)
        {

            if(((u32)game_state->posicao_x[0]!=(u32)npx)||((u32)game_state->posicao_y[0]!=(u32)npy))
            {
                for(s32 i = game_state->gomos; i > 0; i--)
                {
                    game_state->posicao_x[i] = game_state->posicao_x[i-1];
                    game_state->posicao_y[i] = game_state->posicao_y[i-1];
                }
            }

            game_state->posicao_x[0] = npx;
            game_state->posicao_y[0] = npy;

        }
        // else
        // {

        //     FILE *scores;
        //     scores = fopen("scores.txt", "a");
        //     fprintf(scores, "Score:%d\n", game_state->gomos);
        //     fclose(scores);

        //     ResetSnake(game_state);
        
        //     game_state->dead_count += 1;
        // }

        //COLISÃO ENTRE O CORPO DA SNAKE
        // for(u32 i =1; i< game_state->gomos; i++)
        // {
        //     if((((u32)game_state->posicao_x[0])==((u32)game_state->posicao_x[i]))&&
        //         (((u32)game_state->posicao_y[0])==((u32)game_state->posicao_y[i])))
        //     {
        //         ResetSnake(game_state);
        //     }
        // }
    }

    if((((u32)game_state->posicao_x[0]==game_state->food_px)&&((u32)game_state->posicao_y[0])==game_state->food_py))
    {
        game_state->gomos++;
        game_state->food_type = rand()%5+1;
        game_state->posicao_x[game_state->gomos] = game_state->posicao_x[game_state->gomos-1];
        game_state->posicao_y[game_state->gomos] = game_state->posicao_y[game_state->gomos-1];

        

        b32 food_in_wall, food_in_snake;

        do
        {
            game_state->food_px = rand()%SCREEN_WIDTH;
            game_state->food_py = rand()%SCREEN_HEIGHT;

            food_in_wall = ((game_state->space_block_type[((u32)game_state->food_px)+(((u32)game_state->food_py)*SCREEN_WIDTH)] == SPACEBLOCKTYPE_EMPTY) ? false : true);

            food_in_snake = false;
            for(u32 j=1; j<game_state->gomos; j++)
            {
                if((game_state->food_px == ((u32)game_state->posicao_x[j])) && 
                    (game_state->food_py == ((u32)game_state->posicao_y[j])))
                {
                    food_in_snake = true;
                    break;
                }    
            }

        } while(food_in_wall || food_in_snake);
    }



    // desenhando mapa
    for (u32 y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (u32 x = 0; x < SCREEN_WIDTH; ++x)
        {
            u32 space_block_type = game_state->space_block_type[x+(y*SCREEN_WIDTH)];
            u32 codepoint = ' ';

            switch(space_block_type)
            {
                case SPACEBLOCKTYPE_FULL_WALL:
                {
                    codepoint = 0x2588;
                }break;
            }

            SetChar(&renderer->buffer, 
                    x, y, codepoint, 
                    CLASSIC_COLOR_FOREGROUND, CLASSIC_COLOR_BACKGROUND);
        }
    }

    //desenhando foods
    {
        switch(game_state->food_type)
        {

            case FOODTYPE0:
            {
                Color color = CLASSIC_COLOR_FOREGROUND;
                SetChar(&renderer->buffer, 
                    (u16)game_state->food_px, (u16)game_state->food_py, 0x25AA, color,CLASSIC_COLOR_BACKGROUND);
            }
            break;

            case FOODTYPE1:
                Color color1 = COLOR(66, 194, 244, 255);
                SetChar(&renderer->buffer, 
                    (u16)game_state->food_px, (u16)game_state->food_py, 0x25AA, color1,CLASSIC_COLOR_BACKGROUND);
            break;

            case FOODTYPE2:
                Color color2 = COLOR(78,41,173,255);
                SetChar(&renderer->buffer, 
                    (u16)game_state->food_px, (u16)game_state->food_py, 0x25AA, color2,CLASSIC_COLOR_BACKGROUND);
            break;

            case FOODTYPE3:
                Color color3 = COLOR(3,157,196,255);
                SetChar(&renderer->buffer, 
                    (u16)game_state->food_px, (u16)game_state->food_py, 0x25AA, color3,CLASSIC_COLOR_BACKGROUND);
            break;

            case FOODTYPE4:
                Color color4 = COLOR(60, 69, 242,255);
                SetChar(&renderer->buffer, 
                    (u16)game_state->food_px, (u16)game_state->food_py, 0x25AA, color4,CLASSIC_COLOR_BACKGROUND);
            break;

            case FOODTYPE5:
                Color color5 = COLOR(11,58,119,255);
                SetChar(&renderer->buffer, 
                    (u16)game_state->food_px, (u16)game_state->food_py, 0x25AA, color5,CLASSIC_COLOR_BACKGROUND);
            break;
        }
    }

    // desenhando snake
    const u32 player_char[] = {0x2588,0x2593,0x2592,0x2591};
    const r32 player_char_count = (r32)(sizeof(player_char)/sizeof(u32));
    for(s32 i=game_state->gomos; i>=0; --i)
    {
        u32 dist_index = (u32)((r32)i * (player_char_count/(game_state->gomos+1)));
        u32 gomo_index = (u32)game_state->posicao_x[i]+((u32)game_state->posicao_y[i]*SCREEN_WIDTH);
        u32 codepoint = 0x2588;//player_char[dist_index];

        Color color = SNAKE_COLOR;
        r32 damp = (game_state->gomos+2)*2;
        color.r *= (damp-i)*(1.0f/damp);
        color.g *= (damp-i)*(1.0f/damp);
        color.b *= (damp-i)*(1.0f/damp);

        SetChar(&renderer->buffer, 
                (u16)game_state->posicao_x[i], (u16)game_state->posicao_y[i], 
                codepoint, color, CLASSIC_COLOR_BACKGROUND);
    }
}