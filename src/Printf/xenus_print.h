#pragma once

// 0 = big, 1 = little
#define PRINTF_UNICODE_LAZY_ENDIAN 1

#if defined(COMPILER_MSVC)                   
    #define PRINTF_COMPILER_INT_SIZE         PWS_INT_32
    #define PRINTF_COMPILER_LONG_SIZE        PWS_INT_32
    #define PRINTF_COMPILER_LONG_LONG_SIZE   PWS_INT_64
#elif defined(COMPILER_GCC8)                  
    #define PRINTF_COMPILER_INT_SIZE         PWS_INT_32
    #define PRINTF_COMPILER_LONG_SIZE        PWS_INT_64
    #define PRINTF_COMPILER_LONG_LONG_SIZE   PWS_INT_64
#endif
#define PRINTF_MACH_WORD_SIZE                PWS_INT_64;

typedef void(*putc_f)        (void * data, char c);

extern void     printf_fmt_stream(void *data, putc_f putf, const char *fmt, va_list va);
extern size_t   printf_fmt_string(char *str, size_t size, const char *fmt, va_list ap);

#pragma region Extension Apis

typedef _declspec(align(16)) struct safe_double_s
{
    union 
    {
        char _[16];

        uint64_t word;
    #ifdef STAGE_3
        double value;
    #endif
    };
} safe_double_t, * safe_double_ref;

typedef struct printf_state_s
{
    bool  leading_zeros : 1;
    bool  zero_x_hex    : 1;
    bool  ocal_prefix   : 1;
    bool  upper_case    : 1;
    bool  align_left    : 1;
    int_t  width;
    char   sign;
    uint_t base;
    char *buffer;                
} * printf_state_p,
  * printf_state_ref,
    printf_state_t;

typedef void (*printf_write_all_t)(void *data, putc_f putf, printf_state_ref current);
typedef struct printf_interface_s
{
    bool(*printf_check_string)            (char * str);
    void(*printf_write_unicode)            (void * glhf, void *data, putc_f putf, printf_state_ref current, printf_write_all_t write);
    void(*printf_write_floatpoint)        (safe_double_ref d, int_t places, void *data, putc_f putf, printf_state_ref current, printf_write_all_t write);
} * printf_interface_p,
  * printf_interface_ref,
    printf_interface_t;


extern void    printf_attach        (printf_interface_p interface);
#pragma endregion
