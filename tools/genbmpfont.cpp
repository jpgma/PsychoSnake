
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

        BitmapFontHeader *header = GenerateBitmapFont(filename, 
                                                      unicode_blocks, block_count, 
                                                      BFNT_PIXEL_FORMAT_ALPHA8, 
                                                      glyph_height, false);

        
        printf("codepoints:\n");
        // for (s32 i = 0; i < header->glyph_count; ++i)
        // {
        //     printf("\t%u ", chars[i]);
        // }
        printf("\nsegments:\n");
        u32 *cp_segment_ends   = CP_SEGMENT_ENDS(header);
        u32 *cp_segment_starts = CP_SEGMENT_STARTS(header);
        s32 *cp_segment_deltas = CP_SEGMENT_DELTAS(header);
        for (s32 i = 0; i < header->cp_segment_count; ++i)
        {
            printf("\t%d [%u %u] %d\n", i,cp_segment_starts[i],
                                          cp_segment_ends[i],
                                          cp_segment_deltas[i]);
        }

        u32 w = header->glyph_count_x * header->glyph_width;
        u32 h = header->glyph_count_y * header->glyph_height;
        u8 *pixel_data = PIXEL_DATA(header);

        u32 codepoint = 'J';
        u32 glyph_offset = GetGlyphOffset(header, codepoint);
        for (s32 j=0; j < header->glyph_height; ++j) 
        {
          for (s32 i=0; i < header->glyph_width; ++i)
             putchar(" .:ioVM@"[pixel_data[glyph_offset + j*w+i]>>5]);
          putchar('\n');
        }

        for (s32 j=0; j < h; ++j) 
        {
          for (s32 i=0; i < w; ++i)
             putchar(" .:ioVM@"[pixel_data[j*w+i]>>5]);
          putchar('\n');
        }
        printf("\n");

        free(header);
    }
 
    return 0;
}