
#include <stdio.h>
#include <malloc.h>

#include "types.h"
#include "bitmap_font.cpp"

#define SRC_FONT_PATH "..\\data\\psychosnake.ttf"
// #define SRC_FONT_PATH "..\\data\\unifont.ttf"
#define DST_FONT_PATH "..\\data\\psychosnake.bfnt"

internal BitmapFont
GenerateBitmapFont (const char *filename, 
                    UnicodeBlock *unicode_blocks, const u32 block_count, 
                    u16 *glyph_heights, const u32 glyph_height_count,
                    u16 pixel_format);

void main()
{
    printf("    -Gerando %s a partir de %s ", SRC_FONT_PATH, DST_FONT_PATH);
    
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
    const u32 block_count = sizeof(unicode_blocks)/sizeof(UnicodeBlock*);

    u16 glyph_heights[] = { 8, /*12,*/ 16, /*20, 24, 28,*/ 32, /*36, 40, 44, 48, 
                            52, 56, 60,*/ 64, /*68, 72, 76, 80, 84, 88, 92, 96*/ 120, };
    const u32 glyph_height_count = sizeof(glyph_heights)/sizeof(u16);

    BitmapFont font = GenerateBitmapFont(SRC_FONT_PATH, 
                                         unicode_blocks, block_count, 
                                         glyph_heights, glyph_height_count,
                                         BFNT_PIXEL_FORMAT_ALPHA8);

    FILE *out_file = fopen(DST_FONT_PATH,"wb");
    if(out_file)
    { 
        // escrevendo o arquivo assumindo que foi alocado em um unico bloco
        fwrite ( (const void *) font.header, font.font_size, 1, out_file);
    }

    FreeBitmapFont(font);
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

internal BitmapFont 
GenerateBitmapFont (const char *filename, 
                    UnicodeBlock *unicode_blocks, const u32 block_count, 
                    u16 *glyph_heights, const u32 glyph_height_count,
                    u16 pixel_format)
{
    BitmapFont res = {};

    u8 *font_data = 0;
    FILE *f = fopen(filename, "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        font_data = (u8*)calloc(1,fsize);
        fread(font_data, 1, fsize, f);
    }
    else
    {
        printf("n foi possivel abrir arquivo \"%s\"\n", filename);
        goto end;
    }

    // inicando fonte do stb truetype
    stbtt_fontinfo font;
    stbtt_InitFont(&font, font_data, stbtt_GetFontOffsetForIndex(font_data,0));
    
    // TODO: assegurar ordem crescente de alturas!!!
    // obtendo escalas para as varias alturas
    r32 *glyph_scales = (r32*)calloc(glyph_height_count,sizeof(r32));
    for (s32 i = 0; i < glyph_height_count; ++i)
    {
        glyph_scales[i] = stbtt_ScaleForPixelHeight(&font, glyph_heights[i]);
    }

    u32 glyph_count = 1; // p/ caractere indefinido
    for (u32 i = 0; i < block_count; ++i)
    {
        glyph_count += (unicode_blocks[i].last - unicode_blocks[i].first) + 1;
    }
    u32 undefined_codepoints = 0;
    u32 *chars = (u32*)calloc(glyph_count,sizeof(u32));
    u32 *c = chars+1;
    for (u32 i = 0; i < block_count; ++i)
    {
        u32 first = unicode_blocks[i].first;
        u32 last = unicode_blocks[i].last;
        for (u32 j = first; j <= last; ++j)
        {
            s32 index = stbtt_FindGlyphIndex(&font, j);
            if(index != 0)
            {
                *(c++) = j;
            }
            else
            {
                ++undefined_codepoints;
            }
        }
    }

    // ordenando codepoints
    for (s32 i = 0; i < glyph_count; ++i)
    {
        for (s32 j = i+1; j < glyph_count; ++j)
        {
            if(chars[j] < chars[i])
            {
                u32 a = chars[i];
                chars[i] = chars[j];
                chars[j] = a;
            }
        }
    }

    // movendo codepoints para que fique apenas um undefined, hopefully
    if(undefined_codepoints > 0)
    {
        undefined_codepoints += 1;
        // printf("u:%.4x\n", chars[undefined_codepoints]);
        for (s32 i = undefined_codepoints; i < glyph_count; ++i)
        {
            chars[i-undefined_codepoints+1] = chars[i];
        }
        glyph_count -= undefined_codepoints-1;
    }

    for (s32 i = 0; i < glyph_count; ++i)
    {
        printf("-u:%u ", chars[i]);
    }

    // contando segmentos de chars
    u16 cp_segment_count = 1;
    for (s32 i = 1; i < glyph_count; ++i)
    {
        if(chars[i]-chars[i-1] > 1)
            ++cp_segment_count;
    }
    // printf("cp_segment_count:%u\n", cp_segment_count);
  
    // calculando tamanho dos dados
    u32 glyph_data_size = 0;
    for (s32 glyph_index = 0; glyph_index < glyph_count; ++glyph_index)
    {
        for (s32 glyph_height_index = 0; 
             glyph_height_index < glyph_height_count;
             ++glyph_height_index)
        {
            s32 ix0,iy0,ix1,iy1;
            r32 glyph_scale = glyph_scales[glyph_height_index];
            stbtt_GetCodepointBitmapBox(&font, chars[glyph_index], 
                                        glyph_scale,glyph_scale, 
                                        &ix0, &iy0, &ix1, &iy1);
            u32 pixel_count = abs(ix1-ix0)*abs(iy1-iy0);
            glyph_data_size += pixel_count + sizeof(u32); // + offset dos dados
        }
    }

    u32 res_data_size = sizeof(BitmapFontHeader) + 
                        (sizeof(u32)*cp_segment_count) + 
                        (sizeof(u32)*cp_segment_count) + 
                        (sizeof(s32)*cp_segment_count) +
                        (sizeof(HeightGroupHeader)*glyph_height_count) +
                        (sizeof(GlyphHeader)*glyph_count) +
                        glyph_data_size;
    u8 *res_data = (u8*)calloc(res_data_size,1);

    // setando posicoes das secções do arquivo
    res.header = (BitmapFontHeader*)res_data;
    res.cp_segment_ends = (u32*)(res.header + 1);
    res.cp_segment_starts = res.cp_segment_ends + cp_segment_count;
    res.cp_segment_deltas = res.cp_segment_starts + cp_segment_count;
    res.height_group_headers = (HeightGroupHeader*)(res.cp_segment_deltas + cp_segment_count);
    res.glyph_headers = (GlyphHeader*)(res.height_group_headers + glyph_height_count);
    res.glyph_data = (u8*)(res.glyph_headers + glyph_count);
    res.font_size = res_data_size;

    // setando header
    res.header->tag = 'bfnt';
    res.header->flags = pixel_format;
    res.header->glyph_count = glyph_count;
    res.header->cp_encoding = BFNT_ENCODING_UNICODE_FULL;
    res.header->cp_segment_count = cp_segment_count;
    res.header->height_group_count = glyph_height_count;
    res.header->glyph_data_size = glyph_data_size;
    
    s32 ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);

    res.header->raw_ascent = ascent;
    res.header->raw_descent = descent;
    res.header->raw_line_gap = line_gap;
    res.header->raw_max_advance = 0;
    
    // computando ends, starts e deltas
    u16 cur_segment = 0;
    res.cp_segment_starts[0] = chars[0];
    s32 delta = -res.cp_segment_starts[0];
    for (s32 i = 1; i < glyph_count; ++i)
    {
        if(chars[i]-chars[i-1] > 1)
        {
            res.cp_segment_ends[cur_segment] = chars[i-1];
            res.cp_segment_deltas[cur_segment] = delta;

            if((cur_segment+1) < cp_segment_count)
            {
                res.cp_segment_starts[cur_segment+1] = chars[i];
                delta = -res.cp_segment_starts[cur_segment+1] + i;
            }

            ++cur_segment;
        }
        
        if(i == glyph_count-1)
        {
            if(chars[i]-chars[i-1] > 1)
                res.cp_segment_starts[cur_segment] = chars[i];
            res.cp_segment_ends[cur_segment] = chars[i];
            res.cp_segment_deltas[cur_segment] = delta;
        }
    }

    /*
      if (ix0) *ix0 = STBTT_ifloor( x0 * scale_x + shift_x);
      if (iy0) *iy0 = STBTT_ifloor(-y1 * scale_y + shift_y);
      if (ix1) *ix1 = STBTT_iceil ( x1 * scale_x + shift_x);
      if (iy1) *iy1 = STBTT_iceil (-y0 * scale_y + shift_y);
    */

    // computando height groups
    for (s32 height_index = 0; 
         height_index < glyph_height_count; 
         ++height_index)
    {
        HeightGroupHeader *hgh = res.height_group_headers + height_index;
        hgh->height = glyph_heights[height_index];
        hgh->scale = glyph_scales[height_index];
    }

    // preenchendo glyph headers
    u32 offset = 0;
    for (u32 i = 0; i < glyph_count; ++i)
    {
        GlyphHeader *glyph_header = res.glyph_headers + i;

        u32 gi = stbtt_FindGlyphIndex(&font, chars[i]);

        s32 ix0,iy0,ix1,iy1, lsb, advance;
        stbtt_GetGlyphBitmapBox(&font, gi, 
                                1.0f,1.0f, 
                                &ix0, &iy0, &ix1, &iy1);
        stbtt_GetGlyphHMetrics(&font, gi, &advance, &lsb);

        glyph_header->raw_min_x = ix0;
        glyph_header->raw_min_y = iy0;
        glyph_header->raw_max_x = ix1;
        glyph_header->raw_max_y = iy1;

        glyph_header->raw_width = abs(ix1-ix0);
        glyph_header->raw_height = abs(iy1-iy0);

        glyph_header->raw_lsb = lsb;
        glyph_header->raw_advance = advance;
        if(glyph_header->raw_advance > res.header->raw_max_advance)
            res.header->raw_max_advance = glyph_header->raw_advance;

        glyph_header->data_offset = offset;
        u32 *scaled_offsets = (u32*)(res.glyph_data + offset);
        offset += (glyph_height_count * sizeof(u32));
        for (s32 height_index = 0; 
             height_index < glyph_height_count; 
             ++height_index)
        {
            scaled_offsets[height_index] = offset;
            
            r32 s = glyph_scales[height_index];
            s32 scaled_width = fabs((ix1*s) - (ix0*s));
            s32 scaled_height = fabs((iy1*s) - (iy0*s));

            // preenchendo glyph data                
            u8 *dst = res.glyph_data + offset;
            stbtt_MakeGlyphBitmap(&font, dst, 
                                  scaled_width, scaled_height, 
                                  scaled_width, s,s, gi);
            
            offset += (scaled_width*scaled_height);
        }
    }

  
    free(chars);
    free(glyph_scales);

    end:
    if(font_data) free(font_data);
    return res;
}