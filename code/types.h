
#include <stdint.h>

///////////////////////
// Definicoes basicas

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
#define assert(x) {if(!(x)) *((int*)0) = 0;}

#define PI (3.1415927f)
#define RAD_45  (PI*0.25f)
#define RAD_90  (PI*0.5f)
#define RAD_180 PI
#define RAD_360 (2.0f*PI)


inline u32 
U32Swap(u32 n)
{ // funcao p/ inverter um u32 (endianness)
    n = ((n << 8) & 0xFF00FF00 ) | ((n >> 8) & 0xFF00FF ); 
    return (n << 16) | (n >> 16);
}