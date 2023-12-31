
#define SCREEN_WIDTH  86 //120// 16
#define SCREEN_HEIGHT 48 // 67// 16
#define CHAR_SIZE 8
#define DEBUG_LINE_COUNT 0

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

struct V2
{
    r32 x,y;
};
#define V2(x,y) v2((x),(y))
V2 v2(r32 x, r32 y) { return{x,y}; }
V2 operator * (V2 v, r32 s) { return{v.x*s, v.y*s}; }
V2 operator * (r32 s, V2 v) { return{v.x*s, v.y*s}; }
V2 operator / (V2 v, r32 s) { return{v.x/s, v.y/s}; }
V2 operator / (r32 s, V2 v) { return{v.x/s, v.y/s}; }
V2 operator + (V2 a, V2 b)  { return{a.x+b.x, a.y+b.y};}
V2 operator - (V2 a, V2 b)  { return{a.x-b.x, a.y-b.y};}

inline r32
Magnitude (V2 v)
{
    r32 res = sqrtf(v.x*v.x + v.y*v.y);
    return res;
}

inline void
Normalize (V2 *v)
{
    r32 mag2 = v->x*v->x + v->y*v->y;
    if(mag2 > 0.0f)
    {
        r32 one_over_mag = 1.0f/sqrtf(mag2);
        v->x *= one_over_mag;
        v->y *= one_over_mag;
    }
}

inline V2 
Normalized (V2 v)
{
    V2 res = v;
    Normalize(&res);
    return res;
}

#define PLAYER_WIDTH 3
#define PLAYER_HEIGHT 3
#define PLAYER_MAX_SHOTS 100
#define PLAYER_SHOT_COOL_SECONDS 0.01f

#define MAX_ENEMIES 10
#define ENEMY_WIDTH 3
#define ENEMY_HEIGHT 3

struct Shot
{
    V2 p,d;
};

struct GameState
{
    b32 initialized;

    V2 player_p;
    V2 player_d;

    u32 player_shot_index;
    r32 player_shot_cooldown;
    Shot player_shots[PLAYER_MAX_SHOTS];

    V2 enemy_p[MAX_ENEMIES];
    V2 enemy_d[MAX_ENEMIES];
    r32 enemy_health[MAX_ENEMIES];
};

#define MIN_SPAWN_DISTANCE 10.0f//((SCREEN_HEIGHT+10.0f)/2.0f)
#define SPAWN_SPAN 10.0f

inline r32 RandomUnilateral()
{
    r32 res = (((r32)rand())/RAND_MAX);
    return res;
}

inline r32 RandomBilateral()
{
    r32 res = ((((r32)rand())/RAND_MAX)*2.0f) - 1.0f;
    return res;
}

internal void
SpawnEnemy(GameState *game_state, u32 index)
{
    const V2 spawn_center = V2((SCREEN_WIDTH/2.0f),(SCREEN_HEIGHT/2.0f));
    const V2 spawn_min = V2(MIN_SPAWN_DISTANCE * (RandomBilateral() > 0.0f ? 1.0f : -1.0f),
                            MIN_SPAWN_DISTANCE * (RandomBilateral() > 0.0f ? 1.0f : -1.0f));

    game_state->enemy_p[index] = spawn_center + spawn_min + (V2(RandomBilateral(),RandomBilateral()) * SPAWN_SPAN);
    game_state->enemy_d[index] = Normalized(game_state->player_p - game_state->enemy_p[index]);
    game_state->enemy_health[index] = 1.0f;
}

internal void
Shoot (GameState *game_state, Shot *shots, u32 *shot_index, u32 max_shots, V2 p, V2 d)
{
    if(*shot_index < max_shots-1)
    {
        *shot_index = *shot_index+1;
    }
    else
    {
        *shot_index = 0;
    }
    u32 index = *shot_index;

    if(d.x==0.0f && d.y==0.0f)
    {
        d.y = -1.0f;
    }

    game_state->player_shots[index].p = p + V2(((PLAYER_WIDTH+1)*d.x/2.0f),((PLAYER_HEIGHT+1)*d.y/2.0f));
    game_state->player_shots[index].d = d;    
}

internal void
PlayerRemoveShot (GameState *game_state, u32 index)
{
    game_state->player_shots[index].p = V2(INFINITY,INFINITY);
    game_state->player_shots[index].d = V2(0.0f,0.0f);
}

internal void
UpdateAndRenderShots (GameState *game_state, Renderer *renderer, r32 dt)
{
    game_state->player_shot_cooldown += dt;

    r32 player_shot_speed = 15.0f;
    Color player_shot_color = COLOR(255,100,100,255);
    for (s32 i = 0; i < PLAYER_MAX_SHOTS; ++i)
    {

        game_state->player_shots[i].p = game_state->player_shots[i].p + (game_state->player_shots[i].d * (player_shot_speed*dt));

        SetChar(renderer, (u16)game_state->player_shots[i].p.x, (u16)game_state->player_shots[i].p.y, 
                GetGlyphIndex(renderer->font,' '), player_shot_color,player_shot_color);
    }
}

internal void
DrawShip (Renderer *renderer, s32 screen_x, s32 screen_y, V2 d, u32 width, u32 height, Color color)
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
        d.x = (d.x > 0.1f ? 1.0f : (d.x < -0.1f ? -1.0f : 0.0f));
        d.y = (d.y > 0.1f ? 1.0f : (d.y < -0.1f ? -1.0f : 0.0f));

        s32 dir_x = (screen_x + (d.x*half_width));
        s32 dir_y = (screen_y + (d.y*half_height));

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
UpdateAndRenderEnemies (GameState *game_state, Renderer *renderer, r32 dt)
{
        for (s32 i = 0; i < MAX_ENEMIES; ++i)
        {
            if(game_state->enemy_health[i] > 0.0f)
            {
                game_state->enemy_p[i] = game_state->enemy_p[i] + (game_state->enemy_d[i]*1.0f*dt);

                game_state->enemy_d[i] = Normalized(game_state->player_p - game_state->enemy_p[i]);

                s32 half_width = ENEMY_WIDTH/2;
                s32 half_height = ENEMY_HEIGHT/2;

                s32 screen_x = (s32)game_state->enemy_p[i].x;
                s32 screen_y = (s32)game_state->enemy_p[i].y;

                for (s32 j = 0; j < PLAYER_MAX_SHOTS; ++j)
                {
                    s32 x = (s32)game_state->player_shots[j].p.x;
                    s32 y = (s32)game_state->player_shots[j].p.y;

                    if((fabs(screen_x - x) <= half_width) && 
                       (fabs(screen_y - y) <= half_height))
                    {
                        game_state->enemy_health[i] -= (1.0f/5.0f);
                        PlayerRemoveShot(game_state, j);
                    }
                }

                Color dead_color = COLOR(10,10,25,255);
                Color live_color = COLOR(100,100,255,255);
                Color color = LerpColor(dead_color, live_color, game_state->enemy_health[i]);
             
                DrawShip(renderer, 
                         screen_x, screen_y, game_state->enemy_d[i], 
                         ENEMY_WIDTH, ENEMY_HEIGHT, color);
            }
            else
            {
                SpawnEnemy(game_state, i);
            }
        }

}

internal void 
GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt)
{
    if(!game_state->initialized)
    {
        srand(time(0));

        game_state->player_p = V2((SCREEN_WIDTH/2),(SCREEN_HEIGHT/2));
        game_state->player_d = V2(0.0f,-1.0f);
        for (u32 i = 0; i < PLAYER_MAX_SHOTS; ++i)
        {
            PlayerRemoveShot(game_state, i);
        }

        for (u32 i = 0; i < MAX_ENEMIES; ++i)
        {
            SpawnEnemy(game_state, i);
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

    game_state->player_d = V2(((right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f)),
                              ((down  ? 1.0f : 0.0f) - (up   ? 1.0f : 0.0f)));
    Normalize(&game_state->player_d);
    game_state->player_p = game_state->player_p + (game_state->player_d * (player_speed * dt));

    if(space && 
       (game_state->player_shot_cooldown >= PLAYER_SHOT_COOL_SECONDS))
    {
        Shoot(game_state, 
              game_state->player_shots,
              &game_state->player_shot_index,
              PLAYER_MAX_SHOTS,
              game_state->player_p,
              game_state->player_d);

        game_state->player_shot_cooldown = 0.0f;
    }

    Color red  = COLOR(255,0,0,255);
    
    DrawShip(renderer, 
             (s32)game_state->player_p.x, (s32)game_state->player_p.y,
             game_state->player_d, PLAYER_WIDTH, PLAYER_HEIGHT, red);

    UpdateAndRenderShots(game_state, renderer, dt);
    
    UpdateAndRenderEnemies(game_state, renderer, dt);

}

