/* Caracteres p/ construçao de obstaculos:

    176 ░
    177 ▒
    178 ▓
    219 █
    220 ▄
    223 ▀
    254 ■

    250 ·
    217 ┘
    179 │
    192 └
    180 ┤
    193 ┴
    195 ├
    197 ┼
    191 ┐
    218 ┌
    194 ┬
    196 ─

    250 ·
    188 ╝
    186 ║
    200 ╚
    185 ╣
    202 ╩
    204 ╠
    206 ╬
    187 ╗
    201 ╔
    203 ╦
    205 ═

    exemplos:
    ╔═════╦
    ║
    ╠═══╗  
    ╚═══╝ 
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

global u8 thin_walls[] = 
{
    250, 217, 179, 192, 180, 193, 
    195, 197, 191, 218, 194, 196
};

global u8 thick_walls[] = 
{
    254, 188, 186, 200, 185, 202, 
    204, 206, 187, 201, 203, 205,
};

global u32 wall_map[] = 
{
    0b11111111111111101111111111111111,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b00000000000000000000000000000000,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b10000000000000000000000000000001,
    0b11111111111111101111111111111111,
};

struct GameState
{
    b32 initialized;

    r32 posicao_x[3];
    r32 posicao_y[3];
    i32 dead_count;
    r32 velocidade_x;
    r32 velocidade_y;
};

internal void GameUpdateAndRender (GameState *game_state, CHAR_INFO *buffer, r32 dt);