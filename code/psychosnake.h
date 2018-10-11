/* Caracteres p/ referencia:

    0x2591 ░
    0x2592 ▒
    0x2593 ▓
    0x2588 █

    0x25AA 🞍
    0x251B ┛
    0x2503 ┃
    0x2517 ┗
    0x252B ┫
    0x253B ┻
    0x2523 ┣
    0x254B ╋
    0x2513 ┓
    0x250F ┏
    0x2533 ┳
    0x2501 ━

    0x25A1 □
    0x255D ╝
    0x2551 ║
    0x255A ╚
    0x2563 ╣
    0x2569 ╩
    0x2560 ╠
    0x256C ╬
    0x2557 ╗
    0x2554 ╔
    0x2566 ╦
    0x2550 ═

    0x25AA  ▪  quadrado pequeno
    0x25FE  ◾ quadrado medio pequeno
    0x25FC  ◼ quadrado medio
    0x25A0  ■ quadrado normal
    0x1F793 🞓
    0x1F792 🞒
    0x1F791 🞑
    0x1F790 🞐

    0x25B2 ▲
    0x25B3 △
    0x25B6 ▶
    0x25B7 ▷
    0x25BC ▼
    0x25BD ▽
    0x25C0 ◀
    0x25C1 ◁
*/

#define WALL_CENTER             0
#define WALL_UP_LEFT            1
#define WALL_UP_DOWN            2
#define WALL_UP_RIGHT           3
#define WALL_UP_DOWN_LEFT       4
#define WALL_UP_LEFT_RIGHT      5
#define WALL_UP_DOWN_RIGHT      6
#define WALL_UP_DOWN_LEFT_RIGHT 7
#define WALL_DOWN_LEFT          8
#define WALL_DOWN_RIGHT         9
#define WALL_DOWN_LEFT_RIGHT    10
#define WALL_LEFT_RIGHT         11

global u32 thin_walls[] = 
{
    0x25AA, 0x251B, 0x2503, 0x2517, 0x252B, 0x253B,
    0x2523, 0x254B, 0x2513, 0x250F, 0x2533, 0x2501
};
// global GlyphHeader *thin_wall_gh[(sizeof(thin_walls)/sizeof(u32)];

global u32 curved_walls[] = 
{
    0x25AA, 0x256F, 0x2503, 0x2570, 0x252B, 0x253B,
    0x2523, 0x254B, 0x256E, 0x256D, 0x2533, 0x2501
};

global u32 double_walls[] = 
{
    0x25A1, 0x255D, 0x2551, 0x255A, 0x2563, 0x2569,
    0x2560, 0x256C, 0x2557, 0x2554, 0x2566, 0x2550
};

// TODO: fazer essas constantes variaveis
#define SCREEN_WIDTH  86 //120// 16
#define SCREEN_HEIGHT 48 // 67// 16
#define CHAR_SIZE 8
#define DEBUG_LINE_COUNT 0

#define MAX_SNAKE_LENGTH 100
#define MAX_WALL_COUNT (SCREEN_WIDTH*SCREEN_WIDTH*2)

struct GameState
{
    b32 initialized;

    r32 posicao_x[MAX_SNAKE_LENGTH];
    r32 posicao_y[MAX_SNAKE_LENGTH];
    r32 snake_size_x[MAX_SNAKE_LENGTH];
    r32 snake_size_y[MAX_SNAKE_LENGTH];
    r32 velocidade_x;
    r32 velocidade_y;
    s32 dead_count;
    s32 gomos;

    b32 rotation_active;

    u32 food_px;
    u32 food_py;
    u32 food_type;
    r32 food_time;

    // u32 original_map[SCREEN_WIDTH * SCREEN_HEIGHT];
    u32 space_block_type[SCREEN_WIDTH * SCREEN_HEIGHT];
    r32 wall_x[MAX_WALL_COUNT];
    r32 wall_y[MAX_WALL_COUNT];
    u32 wall_count;
};

internal void GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt);