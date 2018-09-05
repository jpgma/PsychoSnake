
struct GameState
{
    b32 initialized;
};

internal void 
GameUpdateAndRender (GameState *game_state, Renderer *renderer, r32 dt)
{
    ClearRenderBuffer(&renderer->buffer, ' ', COLOR(165,165,249,255),COLOR(165,165,249,255));
}