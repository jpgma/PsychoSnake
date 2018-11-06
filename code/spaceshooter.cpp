
#define SCREEN_WIDTH  86 //120// 16
#define SCREEN_HEIGHT 48 // 67// 16
#define CHAR_SIZE 8
#define DEBUG_LINE_COUNT 0

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

struct GameState
{
    b32 initialized;
};

internal void 
GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt)
{

}

