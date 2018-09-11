#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 16
#define CHAR_SIZE 16
#define DEBUG_LINE_COUNT 1

#define TARGET_FPS 60
#define TARGET_MS_PER_FRAME (1000.0/(r64)TARGET_FPS)

struct GameState
{
    b32 initialized;
    r32 vx;
    r32 vy;

    r32 map_px,map_py;

    //POSIÇÕES NO ESPAÇO DO MUNDO
    r32 px;
    r32 py;

    //Qtd de chars que vai andar em 1s
    r32 vel_units_per_sec;
};


struct V2
{
    r32 x,y;
};

V2 operator + (V2 a, V2 b)
{
    return {a.x+b.x, a.y+b.y};
}

V2 operator * (V2 v, r32 s)
{
    return{v.x*s, v.y*s};
}


internal void 
GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt)
{
	if(!game_state->initialized)
	{
		game_state->initialized=true;
		game_state->vel_units_per_sec=10.0f;

	}
		
    ClearRenderBuffer(&renderer->buffer, ' ', COLOR(165,165,249,255),COLOR(165,165,249,255));

	//POSICAO ESPAÇO DA TELA
	u16 ptx;
	u16 pty;

    { // animando reta
        
        V2 a = {0.0f,0.0f};
        ptx = ((u16)a.x) + (SCREEN_WIDTH/2);
        pty = ((u16)a.y) + (SCREEN_HEIGHT/2);
        SetChar(&renderer->buffer, ptx,pty, 0x2588, COLOR(255,0,0,50),COLOR(165,165,249,255));

        V2 b = {5.0f,2.0f};
        ptx = ((u16)b.x) + (SCREEN_WIDTH/2);
        pty = ((u16)b.y) + (SCREEN_HEIGHT/2);
        SetChar(&renderer->buffer, ptx,pty, 0x2588, COLOR(255,0,0,50),COLOR(165,165,249,255));

        static r32 t = 0.0f;
        t += 2.0f*dt;
        if(t > 1.0f) t = 0.0f;

        V2 p = a + (b * t);
        ptx = ((u16)p.x) + (SCREEN_WIDTH/2);
        pty = ((u16)p.y) + (SCREEN_HEIGHT/2);
        SetChar(&renderer->buffer, ptx,pty, 0x2588, COLOR(0,0,0,50),COLOR(165,165,249,255));
    }

	bool up = IS_KEY_DOWN(VK_UP);
	bool down = IS_KEY_DOWN(VK_DOWN);
	bool left = IS_KEY_DOWN(VK_LEFT);
	bool right = IS_KEY_DOWN(VK_RIGHT);


	game_state->vx = (right ? 1.0 : 0.0)+(left ? -1.0 : 0.0);
	game_state->vy = (down ? 1.0 : 0.0)+(up ? -1.0 : 0.0);
	/*if(right)
	{
		//ORIENTAÇÃO DA DIREÇÃO QUE O MOVIMENTO VAI
		game_state->vx=1.0;
		game_state->vy=0.0;
		
	    game_state->posicao_x++;
	    if(game_state->posicao_x>(SCREEN_WIDTH-1))
	    {
	    	game_state->posicao_x=0.0;
	    }	}
	if(left)
	{
		//ORIENTAÇÃO DA DIREÇÃO QUE O MOVIMENTO VAI
		game_state->vx=-1.0;
		game_state->vy=0.0;

		
	    game_state->posicao_x--;
	    if(game_state->posicao_x<(0.0))
	    {
	    	game_state->posicao_x=(SCREEN_WIDTH-1);
	    }
	}
	if(up)
	{
		//ORIENTAÇÃO DA DIREÇÃO QUE O MOVIMENTO VAI
		game_state->vx=0.0;
		game_state->vy=-1.0;

	    /*game_state->posicao_y--;
	    if(game_state->posicao_y<0.0)
	    {
	    	game_state->posicao_y=(SCREEN_HEIGHT-1);
	    }
	}
	if(down)
	{
		//ORIENTAÇÃO DA DIREÇÃO QUE O MOVIMENTO VAI
		game_state->vx=0.0;
		game_state->vy=1.0;

	    /*game_state->posicao_y++;
	    if(game_state->posicao_y>(SCREEN_HEIGHT-1))
	    {
	    	game_state->posicao_y=0.0;
	    }
	}*/

	//DESLOCAMENTO DO FRAME EM VX E VY
	game_state->vx*= (game_state->vel_units_per_sec*dt);
	game_state->vy*= (game_state->vel_units_per_sec*dt);

	//POSIÇÃO PERSONAGEM NO MUNDO
	game_state->px+=game_state->vx;
	game_state->py+=game_state->vy;
	

	//BARRAMENTO DA JANELA
	if(game_state->px>SCREEN_WIDTH-1)
	{
		game_state->px= SCREEN_WIDTH-1;
	}
	else if(game_state->px<0.0)
	{
		game_state->px=0.0;
	}
	if(game_state->py>SCREEN_HEIGHT-1)
	{
		game_state->py = SCREEN_HEIGHT-1;
	}
	else if(game_state->py<0.0)
	{
		game_state->py = 0.0;
	}

	ptx = (u16)game_state->px;
	pty = (u16)game_state->py;

	SetChar(&renderer->buffer,ptx,pty,0xE000, COLOR(255,0,0,0), COLOR(0,0,0,0));
}

