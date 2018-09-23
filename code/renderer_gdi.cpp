
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
    res.foreground_colors = (Color*)calloc(char_count,sizeof(Color));
    res.background_colors = (Color*)calloc(char_count,sizeof(Color));
    res.glyph_headers = (GlyphHeader**)calloc(char_count,sizeof(GlyphHeader*));
    
    return res;
}

internal void 
FreeRenderBuffer (RenderBuffer *buffer)
{
    buffer->width = 0;
    buffer->height = 0;
    buffer->debug_lines = 0;    
    free(buffer->codepoints);
    free(buffer->foreground_colors);
    free(buffer->background_colors);
    free(buffer->glyph_headers);
}

internal Renderer *
InitGDIRenderer (HWND window, u16 width, u16 height, u16 debug_lines, u16 char_size)
{
    Renderer *res = (Renderer*)calloc(1,sizeof(Renderer));
    GDIRenderer *gdi_renderer = (GDIRenderer *)calloc(1,sizeof(GDIRenderer));
    
    gdi_renderer->window_handle = window;
    gdi_renderer->window_hdc = GetDC(window);

    res->buffer = AllocRenderBuffer(width, height, debug_lines);
    res->char_size = char_size;

    ///////////
    // Obtendo fonte
    //  - no futuro será carregada diretamente de um arquivo proprio no 
    // formato gerado por GenerateBitmapFont. 
    UnicodeBlock unicode_blocks[] = 
    {
        UNICODE_BLOCK_BASIC_LATIN,
        // UNICODE_BLOCK_LATIN_1_SUPPLEMENT,
        // UNICODE_BLOCK_GENERAL_PUNCTUATION,
        // UNICODE_BLOCK_SUPERSCRIPS_SUBSCRIPTS,
        // UNICODE_BLOCK_NUMBER_FORMS,
        // UNICODE_BLOCK_MATHEMATICAL_OPERATORS,
        // UNICODE_BLOCK_BRAILLE_PATTERNS,
        UNICODE_BLOCK_BOX_DRAWING,
        UNICODE_BLOCK_BLOCK_ELEMENTS,
        UNICODE_BLOCK_GEOMETRIC_SHAPES,
        UNICODE_BLOCK_GEOMETRIC_SHAPES_EXTENDED,
        // UNICODE_BLOCK_ARROWS,
        // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_A,
        // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_B,
        // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_C,
        UNICODE_BLOCK_PRIVATE_USE_AREA_0,
    };
    u32 block_count = sizeof(unicode_blocks)/sizeof(UnicodeBlock*);
    BitmapFontHeader *font = GenerateBitmapFont("data\\psychosnake.ttf", 
                                                unicode_blocks, block_count, 
                                                BFNT_PIXEL_FORMAT_ALPHA8, 
                                                res->char_size);
    gdi_renderer->font_width = font->max_advance;
    gdi_renderer->font_height = abs(font->ascent)+abs(font->descent);
    if(!font)
    {
        MessageBoxA(NULL, "Erro no carregamento da fonte!", "Error!", MB_ICONERROR|MB_OK);
    }
    res->font = font;
    
    gdi_renderer->bitmap_width = width * gdi_renderer->font_width;
    gdi_renderer->bitmap_height = (height+debug_lines) * gdi_renderer->font_height;
    
    u32 bytes_per_pixel = 4;
    gdi_renderer->bitmap_info.bmiHeader.biSize = sizeof(gdi_renderer->bitmap_info.bmiHeader);
    gdi_renderer->bitmap_info.bmiHeader.biWidth = gdi_renderer->bitmap_width;
    gdi_renderer->bitmap_info.bmiHeader.biHeight = -gdi_renderer->bitmap_height;
    gdi_renderer->bitmap_info.bmiHeader.biPlanes = 1;
    gdi_renderer->bitmap_info.bmiHeader.biBitCount = (bytes_per_pixel*8);
    gdi_renderer->bitmap_info.bmiHeader.biCompression = BI_RGB;
    
    gdi_renderer->bitmap_memory = (u8*)calloc((gdi_renderer->bitmap_width*gdi_renderer->bitmap_height),bytes_per_pixel);
    gdi_renderer->render_mode = RENDER_MODE_CODEPOINTS;

    res->api = (void*)gdi_renderer;

    return res;
}

internal void
FreeGDIRenderer (Renderer **renderer)
{
    GDIRenderer *gdi_renderer = (GDIRenderer*)(*renderer)->api;
    free(gdi_renderer->bitmap_memory);
    free(gdi_renderer);

    FreeRenderBuffer(&(*renderer)->buffer);
    free((*renderer)->font);

    free(*renderer);
    *renderer = 0;
}

internal void 
RenderBufferToScreen (Renderer *renderer)
{
    GDIRenderer *gdi_renderer = (GDIRenderer*)renderer->api;

    u8 *src = GLYPH_DATA(renderer->font);
    u32 *dst = (u32*)gdi_renderer->bitmap_memory;
    
    // preenchendo bitmap com charmap
    switch(gdi_renderer->render_mode)
    {
        case RENDER_MODE_SOLID:
        {
            for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
            {
                for (u16 x = 0; x < renderer->buffer.width; ++x)
                {
                    Color color = renderer->buffer.foreground_colors[x+(y*renderer->buffer.width)];
                    for (s32 yy = 0; yy < gdi_renderer->font_height; ++yy)
                    {   
                        for (s32 xx = 0; xx < gdi_renderer->font_width; ++xx)
                        {
                            u32 index = ((x*gdi_renderer->font_width)+xx) + 
                                        (((y*gdi_renderer->font_height)+yy) * (gdi_renderer->font_width*renderer->buffer.width));
                            dst[index] = (color.value);
                        }
                    }
                }
            }
        } break;

        case RENDER_MODE_CODEPOINTS:
        {
            u32 window_pixel_count = (gdi_renderer->bitmap_width*gdi_renderer->bitmap_height);
            for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
            {
                for (u16 x = 0; x < renderer->buffer.width; ++x)
                {
                    Color color = renderer->buffer.background_colors[x+(y*renderer->buffer.width)];
                    for (s32 yy = 0; yy < gdi_renderer->font_height; ++yy)
                    {   
                        for (s32 xx = 0; xx < gdi_renderer->font_width; ++xx)
                        {
                            u32 index = ((x*gdi_renderer->font_width)+xx) + 
                                         (((y*gdi_renderer->font_height)+yy)*(gdi_renderer->font_width*renderer->buffer.width));
                            dst[index] = (color.value);
                        }
                    }
                }
            }

            u32 advance = 0;
            u32 baseline = renderer->font->ascent;
            GlyphHeader *glyph_headers = GLYPH_HEADERS(renderer->font);
            for (u16 y = 0; y < (renderer->buffer.height + renderer->buffer.debug_lines); ++y)
            {
                for (u16 x = 0; x < renderer->buffer.width; ++x)
                {
                    u32 index = x+(y*renderer->buffer.width);
                    GlyphHeader *glyph_header = renderer->buffer.glyph_headers[index];
                    if(!glyph_header)
                    {
                        u32 codepoint = renderer->buffer.codepoints[index];
                        glyph_header = glyph_headers + GetGlyphIndex(renderer->font,codepoint);
                    }
                    Color foreground = renderer->buffer.foreground_colors[index];
                    Color background = renderer->buffer.background_colors[index];
                    
                    s16 min_x = advance + glyph_header->lsb;
                    s16 min_y = baseline + glyph_header->min_y;
                    
                    // s32 width = ((min_x + glyph_header->width) % GLOBAL_BITMAP_WIDTH) - min_x;
                    // s32 height = ((min_y + glyph_header->height) % GLOBAL_BITMAP_HEIGHT) - min_y;
                    for (s32 yy = 0; yy < glyph_header->height; ++yy)
                    {   
                        for (s32 xx = 0; xx < glyph_header->width; ++xx)
                        {
                            s32 dst_index = (min_x+xx) + ((min_y+yy)*gdi_renderer->bitmap_width);
                            if(dst_index >= 0 && dst_index < window_pixel_count)
                            {
                                Color last_color = COLOR(dst[dst_index]);
                                
                                r32 src_value = 0.0f;
                                {
                                    src_value = src[glyph_header->data_offset + (xx+(yy*glyph_header->width))]/255.0f;
                                }

                                u32 dst_value = COLOR((src_value*foreground.r) + ((1.0f-src_value)*last_color.r),
                                                      (src_value*foreground.g) + ((1.0f-src_value)*last_color.g),
                                                      (src_value*foreground.b) + ((1.0f-src_value)*last_color.b),255).value;
                                
                                dst[dst_index] = dst_value;
                            }
                        }
                    }

                    // forca monospaced no mundo, variacao nas debug_lines
                    if(y < renderer->buffer.height)
                        advance += renderer->font->max_advance;
                    else
                        advance += glyph_header->advance;

                }
                advance = 0;
                baseline += gdi_renderer->font_height;
            }
        } break;
    }

    // colocando bitmap na tela
    RECT rect = {};
    GetClientRect(gdi_renderer->window_handle,&rect);
    s32 window_width = rect.right - rect.left;
    s32 window_height = rect.bottom - rect.top;
    s32 window_left = (window_width - gdi_renderer->bitmap_width)/2;
    s32 window_top = (window_height - gdi_renderer->bitmap_height)/2;
    StretchDIBits(gdi_renderer->window_hdc,
                    window_left,window_top,gdi_renderer->bitmap_width,gdi_renderer->bitmap_height,
                    0,0,gdi_renderer->bitmap_width,gdi_renderer->bitmap_height,
                    (void*)gdi_renderer->bitmap_memory,
                    &gdi_renderer->bitmap_info,
                    DIB_RGB_COLORS, SRCCOPY);
    // SwapBuffers(GLOBAL_WINDOW_HDC);
    // UpdateWindow(GLOBAL_WINDOW_HANDLE);
}