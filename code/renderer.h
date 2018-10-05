
/*
    Definicao dos structs e funcoes que compoem um renderer.

    Funcoes independentes de plataforma podem ser implementadas aqui.
    
    Funcoes como AllocRenderBuffer, FreeRenderBuffer e RenderBufferToScreen devem
    ser implementadas em um arquivo de uma plataforma em especifico (ex: renderer_gdi.cpp).

    O struct renderer guarda variaveis especificas de plataforma em void *api, alocado em 
    algum ponto na inicializacao do renderer.
*/

union Color
{
    u32 value;
    struct
    {
        u8 b,g,r,a;
    };
};

inline Color
COLOR(u32 color)
{
    Color res = {};
    res.value = color; // inverter a cor para poder usar rgba ao inves de bgra(formato do windows) ?
    return res;
}

inline Color
COLOR(u8 r, u8 g, u8 b, u8 a)
{
    Color res = {};
    res.r = r;
    res.g = g;
    res.b = b;
    res.a = a;
    return res;
}

#define COLOR_WHITE COLOR(0xffffffff)
#define COLOR_BLACK COLOR(0xff000000)

struct RenderBuffer
{
    u16 width, height;
    u16 debug_lines;

    u32   *glyph_indexes;
    Color *foreground_colors;
    Color *background_colors;
};

struct Renderer
{
    void *api;

    BitmapFont font;

    RenderBuffer buffer;
    u16 char_size;
};

internal RenderBuffer AllocRenderBuffer (u16 width, u16 height, u16 debug_lines);
internal void FreeRenderBuffer (RenderBuffer *buffer);
internal void RenderBufferToScreen (Renderer *renderer);

inline void
SetChar (RenderBuffer *buffer, u16 x, u16 y,
         u32 glyph_index, Color foreground_color, Color background_color)
{
    if((x < buffer->width) && (y < (buffer->height+buffer->debug_lines)))
    {
        u16 index = x + (y * buffer->width);

        buffer->glyph_indexes[index] = glyph_index;
        buffer->foreground_colors[index] = foreground_color;
        buffer->background_colors[index] = background_color;
    }
}

inline void
SetChar (Renderer *renderer, u16 x, u16 y,
         u32 codepoint, Color foreground_color, Color background_color)
{
    u32 glyph_index = GetGlyphIndex (renderer->font, codepoint);
    SetChar (&renderer->buffer, x,y, glyph_index, foreground_color,background_color);
}


internal void 
ClearRenderBuffer (RenderBuffer *buffer, u32 clear_glyph_index, Color foreground_color, Color background_color)
{
    for (u16 y = 0; y < buffer->height; ++y)
    {
        for (u16 x = 0; x < buffer->width; ++x)
        {
            SetChar(buffer, x,y, clear_glyph_index,foreground_color,background_color);
        }
    }
}

internal void 
WriteDebugText (Renderer *renderer, const char *txt, Color foreground, Color background)
{
    u32 txt_index = 0;

    for (u16 y = renderer->buffer.height; y < (renderer->buffer.height+renderer->buffer.debug_lines); ++y)
    {
        for (u16 x = 0; x < renderer->buffer.width; ++x)
        {
            u32 codepoint = ' ';
            if(txt[txt_index] != '\0')
            {
                codepoint = (u32)txt[txt_index];
                ++txt_index;
            }
            u32 glyph_index = GetGlyphIndex (renderer->font, codepoint);
            SetChar(&renderer->buffer, x,y , glyph_index, foreground, background);
        }
    }

}