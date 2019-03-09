/*
    Purpose: Stage 3 floating point support 
    Author: Reece W.
    License: All rights reserved (2018) J. Reece Wislon 
*/  
#include <xenus.h>
#include "../Boot/access_system.h"
#include <kernel/libx/xenus_memory.h>
#include <kernel/libx/xenus_dbg.h>
#define STAGE_3
#include "xenus_print.h"
#include "print_floating.h"

extern int _fltused = 0;

typedef union __declspec(intrin_type) __declspec(align(16)) __m128i {
    __int8              m128i_i8[16];
    __int16             m128i_i16[8];
    __int32             m128i_i32[4];
    __int64             m128i_i64[2];
    unsigned __int8     m128i_u8[16];
    unsigned __int16    m128i_u16[8];
    unsigned __int32    m128i_u32[4];
    unsigned __int64    m128i_u64[2];
} __m128i;

typedef struct __declspec(intrin_type) __declspec(align(16)) __m128d {
    double              m128d_f64[2];
} __m128d;

extern __m128d _mm_add_pd(__m128d _A, __m128d _B);
extern __m128d _mm_load1_pd(double const*_Dp);
extern __m128d _mm_round_pd(__m128d /* val */, int /* iRoundMode */); // SSE 4.1 - WARNING
extern __m128d _mm_mul_pd(__m128d _A, __m128d _B);
extern __m128d _mm_sub_pd(__m128d _A, __m128d _B);
extern __m128i _mm_castpd_si128(__m128d);
extern __m128d _mm_set1_pd(double _A);
extern __m128i _mm_xor_si128(__m128i _A, __m128i _B);

#define D_TO_I(x) _mm_xor_si128(_mm_castpd_si128(_mm_add_pd(x, _mm_set1_pd(0x0010000000000000))),_mm_castpd_si128(_mm_set1_pd(0x0010000000000000)));
void _printf_floating_point_strip(safe_double_ref d, int_t places, uint64_t * o_integer, uint64_t * o_fraction)
{
    double pow = 1;

    for (int i = 0; i < places; i++) //there is no sse instruction and subsequent intrinsic for pow            //
        pow *= 10;                                                                                             // power := pow(10, places)
                                                                                                               // 
    __m128d number_double   = _mm_load1_pd(&d->value);                                                         // number_double := d
    __m128d power_double    = _mm_load1_pd(&pow);                                                              // power_double  := power
                                                                                                               // 
    __m128d int_double      = _mm_round_pd(number_double, 0x03 | 0x00);                                        // int_double    := round(number_double, truncate)
    __m128d frac_double     = _mm_mul_pd(_mm_sub_pd(number_double, int_double), power_double);                 // frac_double   := (number_double - int_double) * power_double
                                                                                                               //               
    __m128i integer         = D_TO_I(int_double);                                                              // integer       := int_double  
    __m128i fraction        = D_TO_I(frac_double);                                                             // fraction      := frac_double 
                                                                                                               // 
    *o_integer     =  integer.m128i_u64[0];                                                                    // *o_integer  = integer
    *o_fraction    = fraction.m128i_u64[0];                                                                    // *o_fraction = fraction
}
#undef D_TO_I

void xenus_printf_write_floatpoint(safe_double_ref d, int_t places, void *data, putc_f putf, printf_state_ref current, printf_write_all_t write)
{
    char temp[60];
    uint64_t a,  b;

    _printf_floating_point_strip(d, places, &a, &b);

    if (places)
        snprintf(temp, 60, "%i.%i", a, b);
    else
        snprintf(temp, 60, "%i", a);

    current->buffer = temp;
    write(data, putf, current);
}
