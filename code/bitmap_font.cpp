
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
            A posicao do bitmap do caractere e obtida pela subtracao do
            cp_segment_deltas correspondente ao intervalo contendo o caractere.

        u8 pixels[n]
            - Bytes dos pixels representando os caracteres da fonte,
            n sendo glyph_count * BFNT_PIXEL_FORMAT_BPP[formato] * (glyph_width * glyph_height).

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

struct BitmapFontHeader
{
    u32 tag; // 'bfnt'
    u32 flags;
    
    u32 glyph_count;
    
    u16 glyph_width;
    u16 glyph_height;
    u16 glyph_count_x;
    u16 glyph_count_y;

    u16 cp_encoding;
    u16 cp_segment_count;
};
#define CP_SEGMENT_ENDS(header)   ((u32*)((header)+1))
#define CP_SEGMENT_STARTS(header) (((u32*)((header)+1))+(header)->cp_segment_count)
#define CP_SEGMENT_DELTAS(header) (((s32*)((header)+1))+((header)->cp_segment_count*2))
#define PIXEL_DATA(header) (((u8*)((header)+1))+((header)->cp_segment_count*sizeof(u32)*3))

#pragma pack(pop)

internal BitmapFontHeader *
GenerateBitmapFont (const char *filename, 
                    UnicodeBlock *unicode_blocks,
                    u32 block_count, 
                    u16 pixel_format, 
                    u16 glyph_height,
                    b32 pack)
{
    BitmapFontHeader *res = 0;
    u16 glyph_width = glyph_height;

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

    u32 glyph_count = 1; // p/ caractere indefinido
    for (u32 i = 0; i < block_count; ++i)
    {
        glyph_count += (unicode_blocks[i].last - unicode_blocks[i].first);
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
        printf("u:%.4x\n", chars[undefined_codepoints]);
        for (s32 i = undefined_codepoints; i < glyph_count; ++i)
        {
            chars[i-undefined_codepoints+1] = chars[i];
        }
        glyph_count -= undefined_codepoints-1;
    }

    // contando segmentos de chars
    u16 cp_segment_count = 1;
    for (s32 i = 1; i < glyph_count; ++i)
    {
        if(chars[i]-chars[i-1] > 1)
            ++cp_segment_count;
    }
    // printf("cp_segment_count:%u\n", cp_segment_count);

    // alocando memoria do resultado
    u16 dim = (u16)ceil(sqrtf(glyph_count));
    u16 dim_pot = pow(2, ceil(log(dim)/log(2))); // next pow
    u32 pixel_data_size = (pack ? 
                            (glyph_count * BFNT_PIXEL_FORMAT_BPP[pixel_format] * (glyph_width * glyph_height)) : 
                            ((dim*dim) * BFNT_PIXEL_FORMAT_BPP[pixel_format] * (glyph_width * glyph_height)));
    u32 res_data_size = sizeof(BitmapFontHeader) + 
                        (sizeof(u32)*cp_segment_count) + 
                        (sizeof(u32)*cp_segment_count) + 
                        (sizeof(s32)*cp_segment_count) +
                        pixel_data_size;
    u8 *res_data = (u8*)calloc(res_data_size,1);

    res = (BitmapFontHeader*)res_data;
    res->tag = 'bfnt';
    res->flags = pixel_format;
    res->glyph_count = glyph_count;
    res->glyph_width = glyph_width;
    res->glyph_height = glyph_height;
    res->glyph_count_x = dim;
    res->glyph_count_y = dim;
    res->cp_encoding = BFNT_ENCODING_UNICODE_FULL;
    res->cp_segment_count = cp_segment_count;
    printf("-%u %u %u\n", glyph_count, dim, dim*dim);

    u32 *cp_segment_ends = CP_SEGMENT_ENDS(res);
    u32 *cp_segment_starts = CP_SEGMENT_STARTS(res);
    s32 *cp_segment_deltas = CP_SEGMENT_DELTAS(res);

    // computando ends, starts e deltas
    u16 cur_segment = 0;
    cp_segment_starts[0] = chars[0];
    s32 delta = -cp_segment_starts[0]; // +1 pelo vazio no primeiro indice
    for (s32 i = 1; i < glyph_count; ++i)
    {
        if(chars[i]-chars[i-1] > 1)
        {
            cp_segment_ends[cur_segment] = chars[i-1];
            cp_segment_deltas[cur_segment] = delta;

            if((cur_segment+1) < cp_segment_count)
            {
                cp_segment_starts[cur_segment+1] = chars[i];
                delta = -cp_segment_starts[cur_segment+1] + (i);
            }

            ++cur_segment;
        }
        else if(i == glyph_count-1)
        {
            if(chars[i]-chars[i-1] > 1)
                cp_segment_starts[cur_segment] = chars[i];
            cp_segment_ends[cur_segment] = chars[i];
            cp_segment_deltas[cur_segment] = delta;
        }
    }

    u8 *pixels = PIXEL_DATA(res);

    // u32 x = 0;
    // u32 y = 0;
    s32 w,h;
    for (u32 y = 0; y < res->glyph_count_y; ++y)
    {
        for (u32 x = 0; x < res->glyph_count_x; ++x)
        {
            u32 g = x + (y*res->glyph_count_x);
            if(g >= glyph_count)
                break;
            
            r32 scale = stbtt_ScaleForPixelHeight(&font, glyph_height);
            // stbtt_ScaleForMappingEmToPixels(&font, glyph_height);
            s32 advance, lsb, x0,y0,x1,y1;
            s32 gi = stbtt_FindGlyphIndex(&font, chars[g]);
            // stbtt_GetGlyphHMetrics(&font, gi, &advance, &lsb);
            // stbtt_GetGlyphBitmapBox(&font, gi, scale,scale, &x0,&y0,&x1,&y1);
            

            if(chars[g]==0) gi = 0;
            
            u8 *src_pixels = stbtt_GetGlyphBitmap(&font, 0,scale, gi, &w, &h, 0,0);
            // if(src_pixels) STBTT_free(0,src_pixels);
            // if(chars[g] == 'A')
            //     printf("w:%d h:%d bb:[%d,%d][%d,%d] a:%d lsb:%d\n", w,h, x0,y0,x1,y1, advance,lsb);
            
            for (s32 yy = 0; yy < h; ++yy)
            {   
                for (s32 xx = 0; xx < w; ++xx)
                {
                    (pixels)[((x*glyph_width)+xx) + (((y*glyph_height)+yy)*(glyph_width*dim))] = (src_pixels)[xx+(yy*w)];
                }
            }

            // src_pixels = stbtt_GetGlyphBitmap(&font, s,((r32)w/(r32)h)*s, gi, &w, &h, 0,0);
            //stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint);
            // printf("c:%u(%.4x) gi:0x%.4x w:%d h:%d\n", chars[g],chars[g],gi,w,h);
            // for (s32 j=0; j < h; ++j) 
            // {
            //   for (s32 i=0; i < w; ++i)
            //   {
            //      putchar(" .:ioVM@"[src_pixels[j*w+i]>>5]);
            //      // *(dst_pixels++) = src_pixels[j*w+i];
            //   }
            //   putchar('\n');
            // }
            // printf("\n");
            if(src_pixels)
                STBTT_free(0,src_pixels);
        }
    }
    
    free(chars);

    end:
    if(font_data) free(font_data);
    return res;
}