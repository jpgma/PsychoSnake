
#define SCREEN_WIDTH  86 //120// 16
#define SCREEN_HEIGHT 48 // 67// 16
#define CHAR_SIZE 8
#define DEBUG_LINE_COUNT 0

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

#define PLAYER_WIDTH 3
#define PLAYER_HEIGHT 3
#define PLAYER_MAX_SHOTS 10
#define PLAYER_SHOT_COOL_SECONDS 0.1f
// #define MAX_ENEMIES 3
// #define ENEMY_WIDTH 3
// #define ENEMY_HEIGHT 5

// enum EnemyState
// {
//     ENEMY_STATE_DEAD,
//     ENEMY_STATE_ALIVE,
//     ENEMY_STATE_DYING,
// };

struct GameState
{
    b32 initialized;

    r32 player_px,player_py;
    r32 player_dx,player_dy;

    u32 player_shot_index;
    r32 player_shot_px[PLAYER_MAX_SHOTS];
    r32 player_shot_py[PLAYER_MAX_SHOTS];
    r32 player_shot_dx[PLAYER_MAX_SHOTS];
    r32 player_shot_dy[PLAYER_MAX_SHOTS];
    r32 player_shot_cooldown;

    // r32 enemy_px[MAX_ENEMIES];
    // r32 enemy_py[MAX_ENEMIES];
    // u32 enemy_state[MAX_ENEMIES];
};

internal void
PlayerShoot (GameState *game_state, r32 px, r32 py, r32 dx, r32 dy)
{
    if(game_state->player_shot_cooldown >= PLAYER_SHOT_COOL_SECONDS)
    {
        if(game_state->player_shot_index < PLAYER_MAX_SHOTS-1)
        {
            ++game_state->player_shot_index;
        }
        else
        {
            game_state->player_shot_index = 0;
        }
        u32 index = game_state->player_shot_index;

        if(dx==0.0f && dy==0.0f)
        {
            dy = -1.0f;
        }

        game_state->player_shot_px[index] = px + ((PLAYER_WIDTH+1)*dx/2.0f);
        game_state->player_shot_py[index] = py + ((PLAYER_HEIGHT+1)*dy/2.0f);

        game_state->player_shot_dx[index] = dx;
        game_state->player_shot_dy[index] = dy;

        game_state->player_shot_cooldown = 0.0f;
    }
}

internal void
PlayerRemoveShot (GameState *game_state, u32 index)
{
    game_state->player_shot_px[index] = INFINITY;
    game_state->player_shot_py[index] = INFINITY;
    game_state->player_shot_dx[index] = 0.0f;
    game_state->player_shot_dy[index] = 0.0f;
}

internal void
UpdateAndRenderShots (GameState *game_state, Renderer *renderer, r32 dt)
{
    game_state->player_shot_cooldown += dt;

    r32 player_shot_speed = 25.0f;
    Color player_shot_color = COLOR(255,100,100,255);
    for (s32 i = 0; i < PLAYER_MAX_SHOTS; ++i)
    {
        game_state->player_shot_px[i] += game_state->player_shot_dx[i] * (player_shot_speed*dt);
        game_state->player_shot_py[i] += game_state->player_shot_dy[i] * (player_shot_speed*dt);

        SetChar(renderer, (u16)game_state->player_shot_px[i], (u16)game_state->player_shot_py[i], 
                GetGlyphIndex(renderer->font,' '), player_shot_color,player_shot_color);
    }
}

internal void
DrawShip (Renderer *renderer, s32 screen_x, s32 screen_y, r32 dx, r32 dy, u32 width, u32 height, Color color)
{
    s32 half_width = width*0.5f;
    s32 half_height = height*0.5f;

    s32 minx = screen_x - half_width;
    s32 maxx = screen_x + half_width;
    s32 miny = screen_y - half_height;
    s32 maxy = screen_y + half_height;

    if((fabs((SCREEN_WIDTH/2.0f) - (r32)screen_x) <= (SCREEN_WIDTH/2.0f + half_width)) &&
       (fabs((SCREEN_HEIGHT/2.0f) - (r32)screen_y) <= (SCREEN_HEIGHT/2.0f + half_height)))
    {
        s32 dir_x = (screen_x + (dx*half_width));
        s32 dir_y = (screen_y + (dy*half_height));

        for (s32 y = miny; y <= maxy; ++y)
        {
            for (s32 x = minx; x <= maxx; ++x)
            {
                Color c = color;
                if((x == dir_x) && (y == dir_y))
                {
                    c = COLOR_WHITE;
                }
                
                SetChar(renderer, (u16)x, (u16)y, GetGlyphIndex(renderer->font,' '), c,c);
            }
        }
    }


}

internal void 
GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt)
{
    if(!game_state->initialized)
    {
        game_state->player_px = SCREEN_WIDTH/2;
        game_state->player_py = SCREEN_HEIGHT/2;
        game_state->player_dx = 0.0f;
        game_state->player_dy = -1.0f;
        for (u32 i = 0; i < PLAYER_MAX_SHOTS; ++i)
        {
            PlayerRemoveShot(game_state, i);
        }

        game_state->initialized = true;
    }

    ClearRenderBuffer(&renderer->buffer, GetGlyphIndex(renderer->font, ' '), COLOR_WHITE,COLOR_BLACK);

    bool up = IS_KEY_DOWN(VK_UP);
    bool down = IS_KEY_DOWN(VK_DOWN);
    bool left = IS_KEY_DOWN(VK_LEFT);
    bool right = IS_KEY_DOWN(VK_RIGHT);
    bool space = IS_KEY_DOWN(VK_SPACE);

    r32 player_speed = 15.0f;

    game_state->player_dx = (left ? -1.0f : (right ? 1.0f : 0.0f));
    game_state->player_dy = (up   ? -1.0f : (down  ? 1.0f : 0.0f));

    game_state->player_px += game_state->player_dx * (player_speed * dt);
    game_state->player_py += game_state->player_dy * (player_speed * dt);

    if(space)
    {
        PlayerShoot(game_state, 
                    game_state->player_px,game_state->player_py,
                    game_state->player_dx,game_state->player_dy);
    }

    Color red = COLOR(255,0,0,255);
    DrawShip(renderer, 
             (s32)game_state->player_px, (s32)game_state->player_py,
             game_state->player_dx, game_state->player_dy,
             PLAYER_WIDTH, PLAYER_HEIGHT, red);

    UpdateAndRenderShots(game_state, renderer, dt);
}

