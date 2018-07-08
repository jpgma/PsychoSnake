
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef float r32;
typedef double r64;
typedef u32 b32;
#define internal static;
#define global static;

#include "..\code\bitmap_font.cpp"

global u32 example_chars[] = 
{
    0x2E10,0x2E11,'1','2','3','A','B','C','a','b','c',0
};

u32 *
GenerateCharacterList ()
{
    u32 *res = 0;

    const u32 basic_latin_count         = 0x007E - 0x0021;
    const u32 basic_latin_sup_count     = 0x00FF - 0x00A1;
    const u32 box_drawing_count         = 0x257F - 0x2500;
    const u32 block_element_count       = 0x259F - 0x2580;
    const u32 geometric_shapes_count    = 0x25FF - 0x25A0;
    const u32 geometric_shapes_ex_count = 0x1F7FF - 0x1F780;
    const u32 arrows_count              = 0x21FF - 0x2190;
    const u32 domino_tiles_count        = 0x1F09F - 0x1F030;

    const u32 char_count = box_drawing_count + block_element_count + 
                           geometric_shapes_count + geometric_shapes_ex_count + 
                           arrows_count + basic_latin_count + basic_latin_sup_count + 
                           /*domino_tiles_count +*/ 1;
    // u32 char_count = sizeof(example_chars)/sizeof(u32);

    res = (u32*)calloc(char_count,sizeof(u32));

    // for (s32 i = 0; i < char_count; ++i)
    // {
    //     res[i] = example_chars[i];
    // }

    u32 c = 0;
    for (u32 i = 0x0021; i < 0x007E; ++i) res[c++] = i;
    for (u32 i = 0x00A1; i < 0x00FF; ++i) res[c++] = i;
    for (u32 i = 0x2500; i < 0x257F; ++i) res[c++] = i;
    for (u32 i = 0x2580; i < 0x259F; ++i) res[c++] = i;
    for (u32 i = 0x25A0; i < 0x25FF; ++i) res[c++] = i;
    for (u32 i = 0x1F780; i < 0x1F7FF; ++i) res[c++] = i;
    for (u32 i = 0x2190; i < 0x21FF; ++i) res[c++] = i;
    // // for (u32 i = 0x1F030; i < 0x1F09F; ++i) res[c++] = i;

    return res;
}

// char buffer[24<<20];
// unsigned char screen[20][79];

// int main(int arg, char **argv)
// {
//    stbtt_fontinfo font;
//    int i,j,ascent,baseline,ch=0;
//    float scale, xpos=2; // leave a little padding in case the character extends left
//    char *text = "Heljo World!"; // intentionally misspelled to show 'lj' brokenness

//    fread((void*)buffer, 1, 1000000, fopen("c:/windows/fonts/arialbd.ttf", "rb"));
//    stbtt_InitFont(&font, (const unsigned char*)buffer, 0);

//    scale = stbtt_ScaleForPixelHeight(&font, 15);
//    stbtt_GetFontVMetrics(&font, &ascent,0,0);
//    baseline = (int) (ascent*scale);

//    while (text[ch]) {
//       int advance,lsb,x0,y0,x1,y1;
//       float x_shift = xpos - (float) floor(xpos);
//       stbtt_GetCodepointHMetrics(&font, text[ch], &advance, &lsb);
//       stbtt_GetCodepointBitmapBoxSubpixel(&font, text[ch], scale,scale,x_shift,0, &x0,&y0,&x1,&y1);
//       stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline + y0][(int) xpos + x0], x1-x0,y1-y0, 79, scale,scale,x_shift,0, text[ch]);
//       // note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
//       // because this API is really for baking character bitmaps into textures. if you want to render
//       // a sequence of characters, you really need to render each bitmap to a temp buffer, then
//       // "alpha blend" that into the working buffer
//       xpos += (advance * scale);
//       if (text[ch+1])
//          xpos += scale*stbtt_GetCodepointKernAdvance(&font, text[ch],text[ch+1]);
//       ++ch;
//    }

//    for (j=0; j < 20; ++j) {
//       for (i=0; i < 78; ++i)
//          putchar(" .:ioVM@"[screen[j][i]>>5]);
//       putchar('\n');
//    }

//    return 0;
// }

int main(int argc, char *argv[])
{
    if(argc >= 2)
    {
        u16 glyph_height = 32;
        const char *filename = (const char *)argv[1];        
        
        UnicodeBlock unicode_blocks[] = 
        {
            UNICODE_BLOCK_BASIC_LATIN,
            UNICODE_BLOCK_LATIN_1_SUPPLEMENT,
            // UNICODE_BLOCK_GENERAL_PUNCTUATION,
            // UNICODE_BLOCK_SUPERSCRIPS_SUBSCRIPTS,
            // UNICODE_BLOCK_NUMBER_FORMS,
            // UNICODE_BLOCK_ARROWS,
            // UNICODE_BLOCK_MATHEMATICAL_OPERATORS,
            UNICODE_BLOCK_BOX_DRAWING,
            UNICODE_BLOCK_BLOCK_ELEMENTS,
            UNICODE_BLOCK_GEOMETRIC_SHAPES,
            // UNICODE_BLOCK_GEOMETRIC_SHAPES_EXTENDED,
            // UNICODE_BLOCK_BRAILLE_PATTERNS,
            // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_A,
            // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_B,
            // UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_C,
        };
        u32 block_count = sizeof(unicode_blocks)/sizeof(UnicodeBlock*);

        BitmapFontHeader *font = GenerateBitmapFont(filename, 
                                                      unicode_blocks, block_count, 
                                                      BFNT_PIXEL_FORMAT_ALPHA8, 
                                                      glyph_height);
        
        printf("font: gc:%u asc:%d desc:%d lg:%d s:%d\n", 
                font->glyph_count,
                font->ascent,font->descent,font->line_gap,
                (abs(font->ascent)+abs(font->descent)));
        printf("\nsegments:\n");
        u32 *cp_segment_ends   = CP_SEGMENT_ENDS(font);
        u32 *cp_segment_starts = CP_SEGMENT_STARTS(font);
        s32 *cp_segment_deltas = CP_SEGMENT_DELTAS(font);
        for (s32 i = 0; i < font->cp_segment_count; ++i)
        {
            printf("\t%d [%u %u] %d\n", i,cp_segment_starts[i],
                                          cp_segment_ends[i],
                                          cp_segment_deltas[i]);
        }

        GlyphHeader *glyph_headers = GLYPH_HEADERS(font);
        u8 *glyph_data = GLYPH_DATA(font);

        u32 codepoint = 'B';
        for (s32 i = 0; i < font->glyph_count; ++i)
        {
            GlyphHeader glyph_header = glyph_headers[i];
            printf("w:%u h%u lsb:%d adv:%d\n", 
                    glyph_header.width, glyph_header.height,
                    glyph_header.lsb, glyph_header.advance);
            for (s32 y=0; y < glyph_header.height; ++y) 
            {
              for (s32 x=0; x < glyph_header.width; ++x)
                 putchar(" .:ioVM@"[glyph_data[glyph_header.data_offset + x+(y*glyph_header.width)]>>5]);
              putchar('\n');
            }
            putchar('\n');
        }

        free(font);
    }
 
    return 0;
}