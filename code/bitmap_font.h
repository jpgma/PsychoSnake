
/*
    Definicao do formato de fontes bitmap:

    fonte.bfnt
        
        BitmapFontHeader     
            - Sempre no inicio do arquivo, deve ser
            identificado por 4 bytes representando 'bfnt' em ASCII.

        u32 cp_segment_ends[cp_segment_count]
        u32 cp_segment_starts[cp_segment_count]
        s32 cp_segment_deltas[cp_segment_count]
            - cp_segment_ends, cp_segment_starts e cp_segment_deltas 
            representam os intervalos de caracteres colocados na fonte. 
            A posicao do bitmap do caractere e obtida pela adição do
            cp_segment_deltas correspondente ao intervalo contendo o caractere.

        HeightGroupHeader height_group_headers[height_group_count]
            - Headers com informações específicas das resoulções de
            glyphs presentes, escala pra métricas.

        GlyphHeader glyph_headers[glyph_count]
            - Headers com informacoes do offset de cada bitmap e
            posicionamento.

        u8 glyph_data[glyph_data_size]
            - Bytes dos pixels representando os caracteres da fonte.
            u32 scaled_glyph_data_offset[height_group_count]

        exemplo:
            caracteres representados: "123ABCabc" + 255
            char encoding: BFNT_ENCODING_ASCII
            cp_segment_count: 3
            cp_segment_starts: 49('1'), 65('A'), 97('a') 
            cp_segment_ends:   51('3'), 67('C'), 99('c')
            cp_segment_deltas: -49,     -62,     -91
            pixels: 123
                    ABC
                    abc

            achando indice do caractere ASCII 66('B'):
                - procurar o primeiro (deve ser o unico) 
                intervalo contendo o caracter: cp_segment_starts[1] até cp_segment_ends[1]
                - consequentemente o delta será cp_segment_deltas[1]
                - indice = 66 - cp_segment_deltas[1] = 4
*/

#define BFNT_PIXEL_FORMAT_MASK 0b11
#define BFNT_PIXEL_FORMAT_ALPHA8 0
#define BFNT_PIXEL_FORMAT_RGB8   1
#define BFNT_PIXEL_FORMAT_RGBA8  2
#define BFNT_ENCODING_ASCII        0
#define BFNT_ENCODING_UNICODE_BMP  1
#define BFNT_ENCODING_UNICODE_FULL 2

struct UnicodeBlock
{
    u32 first, last;
};

// Basic Multilingual Plane (BMP)
#define UNICODE_BLOCK_BASIC_LATIN                         {0x0020,0x007F}
#define UNICODE_BLOCK_LATIN_1_SUPPLEMENT                  {0x00A0,0x00FF}
#define UNICODE_BLOCK_GENERAL_PUNCTUATION                 {0x2000,0x206F}
#define UNICODE_BLOCK_SUPERSCRIPS_SUBSCRIPTS              {0x2070,0x209F}
#define UNICODE_BLOCK_NUMBER_FORMS                        {0x2150,0x218F}
#define UNICODE_BLOCK_ARROWS                              {0x2190,0x21FF}
#define UNICODE_BLOCK_MATHEMATICAL_OPERATORS              {0x2200,0x22FF}
#define UNICODE_BLOCK_MISC_TECHNICAL                      {0x2300,0x23FF}
#define UNICODE_BLOCK_OPTICAL_CHAR_RECOGNITION            {0x2440,0x244A}
#define UNICODE_BLOCK_ENCLOSED_ALPHANUMERICS              {0x2460,0x24FF}
#define UNICODE_BLOCK_BOX_DRAWING                         {0x2500,0x257F}
#define UNICODE_BLOCK_BLOCK_ELEMENTS                      {0x2580,0x259F}
#define UNICODE_BLOCK_GEOMETRIC_SHAPES                    {0x25A0,0x25FF}
#define UNICODE_BLOCK_MISC_SYMBOLS                        {0x2600,0x26FF}
#define UNICODE_BLOCK_DINGBATS                            {0x2700,0x27BF}
#define UNICODE_BLOCK_MISC_MATHEMATICAL_SYMBOLS_A         {0x27C0,0x27EF}
#define UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_A               {0x27F0,0x27FF}
#define UNICODE_BLOCK_BRAILLE_PATTERNS                    {0x2800,0x28FF}
#define UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_B               {0x2900,0x297F}
#define UNICODE_BLOCK_MISC_MATHEMATICAL_SYMBOLS_B         {0x2980,0x29FF}
#define UNICODE_BLOCK_SUPPLEMENTAL_MATHEMATICAL_OPERATORS {0x2A00,0x2AFF}
#define UNICODE_BLOCK_MISC_SYMBOLS_ARROWS                 {0x2B00,0x2BFF}
#define UNICODE_BLOCK_SUPPLEMENTAL_PUNCTUATION            {0x2E00,0x2E7F}
#define UNICODE_BLOCK_IDEOGRAPHIC_DESC_CHARS              {0x2FF0,0x2FFB}
#define UNICODE_BLOCK_MODIFIER_TONE_LETTERS               {0xA700,0xA71F}
#define UNICODE_BLOCK_PRIVATE_USE_AREA_0                  {0xE000,0xF8FF}
// Supplementary Multilingual Plane (SMP)
#define UNICODE_BLOCK_GEOMETRIC_SHAPES_EXTENDED           {0x1F780,0x1F7FF}
#define UNICODE_BLOCK_SUPPLEMENTAL_ARROWS_C               {0x1F800,0x1F8FF}

global s32 BFNT_PIXEL_FORMAT_BPP[] = 
{ // _bytes_ por pixel de cada formato
    1, // ALPHA8
    3, // RGB8
    4, // RGBA8
};

#pragma pack(push, 1)

struct GlyphHeader
{
    s16 raw_min_x;
    s16 raw_min_y;
    s16 raw_max_x;
    s16 raw_max_y;

    u16 raw_width;
    u16 raw_height;
    
    s16 raw_lsb;
    s16 raw_advance;
    
    u32 first_data_offset;
};

struct HeightGroupHeader
{
    u32 height;
    r32 scale;
};

struct BitmapFontHeader
{
    u32 tag; // 'bfnt'
    u32 flags;
    
    u32 glyph_count;
    u32 glyph_data_size;

    s16 raw_ascent;
    s16 raw_descent;
    s16 raw_line_gap;
    u16 raw_max_advance;

    u16 cp_encoding;
    u16 cp_segment_count;

    u32 height_group_count;
};

#pragma pack(pop)

#define CP_SEGMENT_ENDS(header)   ((u32*)((header)+1))
#define CP_SEGMENT_STARTS(header) (((u32*)((header)+1))+(header)->cp_segment_count)
#define CP_SEGMENT_DELTAS(header) (((s32*)((header)+1))+((header)->cp_segment_count*2))
#define HEIGHT_GROUP_HEADERS(header)
#define GLYPH_HEADERS(header) ((GlyphHeader*)(((s32*)((header)+1))+((header)->cp_segment_count*3)))
#define GLYPH_DATA(header) (((u8*)((header)+1))+((header)->cp_segment_count*sizeof(u32)*3)+((header)->glyph_count*sizeof(GlyphHeader)))

inline r32
Round (r32 n)
{
    r32 res = n;
    // n -= ((u64)n);
    // if(n <= 0.5f)
    //     res = floorf(res);
    // else
    //     res = ceilf(res);
    return res;
}

internal u32 
GetGlyphIndex (BitmapFontHeader *font, u32 codepoint)
{
    u32 res = 0;
    if(font)
    {
        u32 *cp_segment_ends = CP_SEGMENT_ENDS(font);
        u32 *cp_segment_starts = CP_SEGMENT_STARTS(font);
        s32 *cp_segment_deltas = CP_SEGMENT_DELTAS(font);
        for (u32 i = 0; i < font->cp_segment_count; ++i)
        {
            if((cp_segment_ends[i] >= codepoint) &&
                (cp_segment_starts[i] <= codepoint))
            {
                res = codepoint + cp_segment_deltas[i];
                break;
            }
        }
        // u32 x = (index%font->glyph_count_x);
        // u32 y = (index/font->glyph_count_x);
        // res = (x*font->glyph_width) + (y*font->glyph_count_x*font->glyph_width*font->glyph_height);
    }
    return res;
}

internal s32
GetExactHeightIndex (BitmapFontHeader *font, u32 height)
{
    assert(font);

    s32 res = -1;


    HeightGroupHeader *headers = HEIGHT_GROUP_HEADERS(font);
    for (s32 i = 0; i < font->height_group_count; ++i)
    {
        if(headers[i].height == height)
        {
            res = i;
            break;
        }
    }

    return res;
}

internal u8 *
GetScaledGlyphData (BitmapFontHeader *font, u32 glyph_index, u32 height_index)
{
    assert(font);
    assert(glyph_index < font->glyph_count);
    assert(height_index < font->height_group_count);
 
    u8 *res = 0;

    u8 *glyph_data = GLYPH_DATA(font);
    GlyphHeader *header = GLYPH_HEADERS(font) + glyph_index;
    u32 scaled_data_offset = ((u32*)(glyph_data + header->first_data_offset))[height_index];
    res = glyph_data + scaled_data_offset;

    return res;
}