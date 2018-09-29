
/*
    Funcoes que renderizam o buffer de chars com GDI,
    uma API razoavelmente baixo nivel do Windows.
*/

#define RENDER_MODE_SOLID       1
#define RENDER_MODE_CODEPOINTS  2

struct GDIRenderer
{
    BITMAPINFO bitmap_info;
    HWND       window_handle;
    HDC        window_hdc;
    
    u8 *bitmap_memory;
    u32 bitmap_width;
    u32 bitmap_height;
    
    u32 font_width;
    u32 font_height;
    u32 char_size;
    
    u32 render_mode;
};

internal RenderBuffer 
AllocRenderBuffer (u16 width, u16 height, u16 debug_lines)
{
    RenderBuffer res = {};
    
    res.width = width;
    res.height = height;
    res.debug_lines = debug_lines;
    
    u32 char_count = (width*(height+debug_lines));
  
    res.codepoints = (u32*)calloc(char_count,sizeof(u32));
    res.glyph_indexes = (u32*)calloc(char_count,sizeof(u32));
    res.foreground_colors = (Color*)calloc(char_count,sizeof(Color));
    res.background_colors = (Color*)calloc(char_count,sizeof(Color));
    
    return res;
}

internal void 
FreeRenderBuffer (RenderBuffer *buffer)
{
    buffer->width = 0;
    buffer->height = 0;
    buffer->debug_lines = 0;    
    free(buffer->codepoints);
    free(buffer->glyph_indexes);
    free(buffer->foreground_colors);
    free(buffer->background_colors);
}

internal void
UpdateGDIRendererBitmap (Renderer *renderer)
{
    GDIRenderer *gdi_renderer = (GDIRenderer*)renderer->api;

    if(gdi_renderer->char_size != renderer->char_size)
    {
        gdi_renderer->char_size = renderer->char_size;

        s32 height_index = GetExactHeightIndex(renderer->font, renderer->char_size);
        r32 scale = renderer->font.height_group_headers[height_index].scale;

        gdi_renderer->font_width = renderer->font.header->raw_max_advance * scale;
        gdi_renderer->font_height = abs((s32)(renderer->font.header->raw_ascent * scale)) + 
                                    abs((s32)floorf(renderer->font.header->raw_descent * scale));
        
        u32 bytes_per_pixel = 4;
        u32 last_bitmap_memory_size = gdi_renderer->bitmap_width*gdi_renderer->bitmap_height*bytes_per_pixel;
        gdi_renderer->bitmap_width = renderer->buffer.width * gdi_renderer->font_width;
        gdi_renderer->bitmap_height = (renderer->buffer.height+renderer->buffer.debug_lines) * gdi_renderer->font_height;
        
        gdi_renderer->bitmap_info.bmiHeader.biSize = sizeof(gdi_renderer->bitmap_info.bmiHeader);
        gdi_renderer->bitmap_info.bmiHeader.biWidth = gdi_renderer->bitmap_width;
        gdi_renderer->bitmap_info.bmiHeader.biHeight = -gdi_renderer->bitmap_height;
        gdi_renderer->bitmap_info.bmiHeader.biPlanes = 1;
        gdi_renderer->bitmap_info.bmiHeader.biBitCount = (bytes_per_pixel*8);
        gdi_renderer->bitmap_info.bmiHeader.biCompression = BI_RGB;
        
        u32 bitmap_memory_size = gdi_renderer->bitmap_width*gdi_renderer->bitmap_height*bytes_per_pixel;
        if(last_bitmap_memory_size < bitmap_memory_size)
        {
            gdi_renderer->bitmap_memory = (u8*)realloc((void*)gdi_renderer->bitmap_memory,bitmap_memory_size);
        }
    }
}

internal Renderer *
InitGDIUnicodeRenderer (HWND window, const char *font_path, u16 width, u16 height, u16 debug_lines, u16 char_size)
{
    Renderer *res = (Renderer*)calloc(1,sizeof(Renderer));
    GDIRenderer *gdi_renderer = (GDIRenderer *)calloc(1,sizeof(GDIRenderer));
    
    gdi_renderer->window_handle = window;
    gdi_renderer->window_hdc = GetDC(window);

    res->buffer = AllocRenderBuffer(width, height, debug_lines);
    res->char_size = char_size;

    // Obtendo fonte
    BitmapFont font = LoadBitmapFont(font_path);
    res->font = font;

    gdi_renderer->render_mode = RENDER_MODE_CODEPOINTS;

    res->api = (void*)gdi_renderer;

    UpdateGDIRendererBitmap(res);

    return res;
}

internal void
FreeGDIRenderer (Renderer **renderer)
{
    GDIRenderer *gdi_renderer = (GDIRenderer*)(*renderer)->api;
    free(gdi_renderer->bitmap_memory);
    free(gdi_renderer);

    FreeRenderBuffer(&(*renderer)->buffer);
    FreeBitmapFont((*renderer)->font);

    free(*renderer);
    *renderer = 0;
}

internal void 
RenderBufferToScreen (Renderer *renderer)
{
    GDIRenderer *gdi_renderer = (GDIRenderer*)renderer->api;

    BitmapFont font = renderer->font;
    GlyphHeader *glyph_headers = font.glyph_headers;
    u8 *glyph_data = font.glyph_data;

    UpdateGDIRendererBitmap(renderer);
    
    // TODO: GetClosestHeightIndex
    s32 height_index = GetExactHeightIndex(font, renderer->char_size);
    assert(height_index >= 0);
    r32 scale = font.height_group_headers[height_index].scale;

    u32 *dst = (u32*)gdi_renderer->bitmap_memory;
    u32 window_pixel_count = gdi_renderer->bitmap_width*gdi_renderer->bitmap_height;
    
    // u16 x = 0; u16 y = 0;
    // Color color = COLOR(255,0,0,255);
    // u32 glyph_index = GetGlyphIndex(font,'3');
    // GlyphHeader *glyph_header = font.glyph_headers + glyph_index;
    // u8 *src = GetScaledGlyphData(font, glyph_index, height_index);
    // s32 scaled_width = glyph_header->raw_width * scale;
    // s32 scaled_height = glyph_header->raw_height * scale;
    // for (s32 yy = 0; yy < scaled_height; ++yy)
    // {   
    //     for (s32 xx = 0; xx < scaled_width; ++xx)
    //     {
    //         r32 src_value = src[xx+(yy*scaled_width)] / 255.0f;

    //         u32 index = ((x*gdi_renderer->font_width)+xx) + 
    //                     (((y*gdi_renderer->font_height)+yy) * (gdi_renderer->font_width*renderer->buffer.width));
    //         dst[index] = COLOR((src_value*color.r), (src_value*color.g), (src_value*color.b), 255).value;
    //     }
    // }

    u32 advance = 0;
    u32 baseline = (font.header->raw_ascent * scale);
    for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
    {
        for (u16 x = 0; x < renderer->buffer.width; ++x)
        {
            u32 index = x+(y*renderer->buffer.width);
            
            u32 codepoint = renderer->buffer.codepoints[index];
            
            u32 glyph_index = GetGlyphIndex(font,codepoint); // cache!!!
            GlyphHeader *glyph_header = glyph_headers + glyph_index;
            
            Color foreground = renderer->buffer.foreground_colors[index];
            Color background = renderer->buffer.background_colors[index];
            
            s16 min_x = advance + (s32)(glyph_header->raw_lsb * scale);
            s16 min_y = baseline + (s32)(glyph_header->raw_min_y * scale);
    
            u8 *src = GetScaledGlyphData(font, glyph_index, height_index);

            s32 scaled_width = glyph_header->raw_width * scale;
            s32 scaled_height = glyph_header->raw_height * scale;
            for (s32 yy = 0; yy < scaled_height; ++yy)
            {   
                for (s32 xx = 0; xx < scaled_width; ++xx)
                {
                    s32 dst_index = (min_x+xx) + ((min_y+yy)*gdi_renderer->bitmap_width);
                    if(dst_index >= 0 && dst_index < window_pixel_count)
                    {
                        Color last_color = COLOR(0,0,0,255);//COLOR(dst[dst_index]);
                        
                        r32 src_value = src[xx+(yy*scaled_width)] / 255.0f;

                        u32 dst_value = COLOR((src_value*foreground.r) + ((1.0f-src_value)*last_color.r),
                                              (src_value*foreground.g) + ((1.0f-src_value)*last_color.g),
                                              (src_value*foreground.b) + ((1.0f-src_value)*last_color.b),255).value;
                        
                        dst[dst_index] = dst_value;
                    }
                }
            }

            // forca monospaced no mundo, variacao nas debug_lines
            if(y < renderer->buffer.height)
                advance += (font.header->raw_max_advance * scale);
            else
                advance += (glyph_header->raw_advance * scale);

        }
        advance = 0;
        baseline += gdi_renderer->font_height;
    }


    // Char sized pixels
    // for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
    // {
    //     for (u16 x = 0; x < renderer->buffer.width; ++x)
    //     {
    //         Color color = renderer->buffer.foreground_colors[x+(y*renderer->buffer.width)];
    //         for (s32 yy = 0; yy < gdi_renderer->font_height; ++yy)
    //         {   
    //             for (s32 xx = 0; xx < gdi_renderer->font_width; ++xx)
    //             {
    //                 u32 index = ((x*gdi_renderer->font_width)+xx) + 
    //                             (((y*gdi_renderer->font_height)+yy) * (gdi_renderer->font_width*renderer->buffer.width));
    //                 dst[index] = (color.value);
    //             }
    //         }
    //     }
    // }

    // colocando bitmap na tela
    RECT rect = {};
    GetClientRect(gdi_renderer->window_handle,&rect);
    s32 window_width = rect.right - rect.left;
    s32 window_height = rect.bottom - rect.top;
    s32 window_left = (window_width - gdi_renderer->bitmap_width)/2;
    s32 window_top = (window_height - gdi_renderer->bitmap_height)/2;
    
    // StretchDIBits(gdi_renderer->window_hdc,
    //                 window_left,window_top,gdi_renderer->bitmap_width,gdi_renderer->bitmap_height,
    //                 0,0,gdi_renderer->bitmap_width,gdi_renderer->bitmap_height,
    //                 (void*)gdi_renderer->bitmap_memory,
    //                 &gdi_renderer->bitmap_info,
    //                 DIB_RGB_COLORS, SRCCOPY);
    
    StretchDIBits(gdi_renderer->window_hdc,
                  0,0,window_width,window_height,
                  0,0,gdi_renderer->bitmap_width,gdi_renderer->bitmap_height,
                  (void*)gdi_renderer->bitmap_memory,
                  &gdi_renderer->bitmap_info,
                  DIB_RGB_COLORS, SRCCOPY);

    // SwapBuffers(GLOBAL_WINDOW_HDC);
    // UpdateWindow(GLOBAL_WINDOW_HANDLE);
}