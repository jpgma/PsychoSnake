
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
    res.value = (color); // invertendo a cor para poder usar rgba ao inves de abgr(formato do windows)
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

struct Char
{
    u32 codepoint;
    Color foreground_color;
    Color background_color;
};

struct RenderBuffer
{
    u16 width, height;
    u16 debug_lines;

    u32   *codepoints;
    Color *foreground_colors;
    Color *background_colors;
};

struct Renderer
{
    void *api;

    BitmapFontHeader *font;
    RenderBuffer buffer;
    u16 char_size;
};

internal RenderBuffer AllocRenderBuffer (u16 width, u16 height, u16 debug_lines);
internal void FreeRenderBuffer (RenderBuffer *buffer);
internal void RenderBufferToScreen (Renderer *renderer);

internal void
SetChari (RenderBuffer *buffer, u16 i, 
          u32 codepoint, Color foreground_color, Color background_color)
 {
    if(i < (buffer->width*(buffer->height+buffer->debug_lines)))
    {
        buffer->codepoints[i] = codepoint;
        buffer->foreground_colors[i] = foreground_color;
        buffer->background_colors[i] = background_color;
    }
 }

inline void
SetChar (RenderBuffer *buffer, u16 x, u16 y,
         u32 codepoint, Color foreground_color, Color background_color)
{
    u16 i = x + (y * buffer->width);
    SetChari(buffer, i, codepoint, foreground_color, background_color);
}

internal void 
ClearRenderBuffer (RenderBuffer *buffer, u32 clear_codepoint, Color foreground_color, Color background_color)
{
    u16 char_count = buffer->width * buffer->height;
    for (u16 i = 0; i < char_count; ++i)
    {
        SetChari (buffer, i,clear_codepoint,foreground_color,background_color);
        // buffer->codepoints[i] = clear_codepoint;
        // buffer->foreground_colors[i] = foreground_color;
        // buffer->background_colors[i] = background_color;
    }
}

internal void 
WriteDebugText (Renderer *renderer, const char *txt, Color foreground, Color background)
{
    char *c = (char*)txt;
    u16 start = renderer->buffer.width * renderer->buffer.height;
    for (u16 i = 0; i < (renderer->buffer.width*renderer->buffer.debug_lines); ++i)
    {
        u32 codepoint = ' ';
        if(*c != '\0')
        {
            codepoint = (u32)txt[i];
            ++c;
        }
        SetChari(&renderer->buffer, (start+i), codepoint, foreground, background);
    }

}