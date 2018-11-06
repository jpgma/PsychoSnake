
#define SCREEN_WIDTH  86 //120// 16
#define SCREEN_HEIGHT 48 // 67// 16
#define CHAR_SIZE 8
#define DEBUG_LINE_COUNT 0

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

#define PLAYER_WIDTH 3
#define PLAYER_HEIGHT 3

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

    for (s32 y = miny; y < maxy; ++y)
    {
        for (s32 x = minx; x < maxx; ++x)
        {
            SetChar(renderer, (u16)x, (u16)y, GetGlyphIndex(renderer->font,' '), color,color);
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

    Color red = COLOR(255,0,0,255);
    DrawShip(renderer, (s32)game_state->player_px, (s32)game_state->player_py, PLAYER_WIDTH, PLAYER_HEIGHT, red);
}

