
#define SCREEN_WIDTH  86 //120// 16
#define SCREEN_HEIGHT 48 // 67// 16
#define CHAR_SIZE 8
#define DEBUG_LINE_COUNT 0

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

#define PLAYER_WIDTH 6
#define PLAYER_HEIGHT 6

struct GameState
{
    b32 initialized;

    r32 player_px,player_py;
};

internal void
DrawShip (Renderer *renderer, s32 screen_x, s32 screen_y, u32 width, u32 height, Color color)
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
        for (s32 y = miny; y <= maxy; ++y)
        {
            for (s32 x = minx; x <= maxx; ++x)
            {
                if(x == screen_x && y == screen_y)
                {
                    SetChar(renderer, (u16)x, (u16)y, GetGlyphIndex(renderer->font,' '), COLOR_WHITE,COLOR_WHITE);
                }
                else
                    SetChar(renderer, (u16)x, (u16)y, GetGlyphIndex(renderer->font,' '), color,color);
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
        game_state->initialized = true;
    }

    ClearRenderBuffer(&renderer->buffer, GetGlyphIndex(renderer->font, ' '), COLOR_WHITE,COLOR_BLACK);

    bool up = IS_KEY_DOWN(VK_UP);
    bool down = IS_KEY_DOWN(VK_DOWN);
    bool left = IS_KEY_DOWN(VK_LEFT);
    bool right = IS_KEY_DOWN(VK_RIGHT);

    r32 player_speed = 15.0f;
    r32 player_speed_x = (left ? -1.0f : (right ? 1.0f : 0.0f)) * (player_speed * dt);
    r32 player_speed_y = (up   ? -1.0f : (down  ? 1.0f : 0.0f)) * (player_speed * dt);

    game_state->player_px += player_speed_x;
    game_state->player_py += player_speed_y;

    Color red = COLOR(255,0,0,255);
    DrawShip(renderer, (s32)game_state->player_px, (s32)game_state->player_py, PLAYER_WIDTH, PLAYER_HEIGHT, red);
}

