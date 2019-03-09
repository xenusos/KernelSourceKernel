/*
    Purpose: 
    Author:  Reece W.
    License: All rights reserved (2018) - J. Reece Wilson. 
             The original works are no longer present to such an extent that the GPL license is void. Later developers tried claiming BSD - this is also legally void.
*/
#include <xenus.h>
#include "xenus_print.h"

typedef struct printf_string_stream_s
{
    size_t    num_chars;
    size_t    dest_capacity;
    char *    dest;
} * printf_string_stream_p,
  * printf_string_stream_ref,
    printf_string_stream_t;

typedef enum print_word_size_e
{
    PWS_INT_8,
    PWS_INT_16,
    PWS_INT_32,
    PWS_INT_64,
} print_word_size_t;

static printf_interface_p    printf_interface = NULL;

void printf_write_machine_unsigned_int(uint_t num, printf_state_ref p)
{
    uint_t n;
    uint_t d;
    char *buffer;
    uint_t base;

    n        = 0;
    d        = 1;
    buffer   = p->buffer;
    base     = p->base;

    while (num / d >= p->base)
        d *= p->base;

    while (d != 0)
    {
        uint_t dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0)
        {
            *buffer++ = ((char) dgt) + (dgt < 10 ? '0' : (p->upper_case ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *buffer = 0;
}

void printf_write_machine_int(int_t num, printf_state_ref p)
{
    if (num < 0)
    {
        num = -num;
        p->sign = '-';
    }
    printf_write_machine_unsigned_int((uint_t)num, p);
}

void printf_write_string(const char * buffer, void *data, putc_f putf)
{
    char ch;
    while ((ch = *(buffer++)) != 0)
        putf(data, ch);
}

uint_t printf_calc_length(printf_state_ref current)
{
    uint_t ret;
    char * buffer;

    ret       = 0;
    buffer    = current->buffer;

    while ((*(buffer++)))
        ret++;

    if (current->sign)
        ret++;

    if (current->ocal_prefix)
        ret++;

    if (current->zero_x_hex)
        ret += 2;
    return ret;
}

void printf_write_all(void * data, putc_f putf, printf_state_ref current)
{
    uint_t remaining_spaces;
    char *buffer;

    remaining_spaces    = MAX(0, (int_t)current->width - (int_t)printf_calc_length(current));
    buffer              = current->buffer;

    if ((!(current->leading_zeros)) && (!(current->align_left))) 
        while (remaining_spaces-- > 0)
            putf(data, ' ');

    if (current->sign)
        putf(data, current->sign);

    if (current->zero_x_hex)
        printf_write_string("0x", data, putf);

    if (current->ocal_prefix)
        putf(data, '0');

    if (current->leading_zeros) 
        while (remaining_spaces-- > 0)
            putf(data, '0');

    printf_write_string(current->buffer, data, putf);

    if ((!(current->leading_zeros)) && (current->align_left))
        while ((remaining_spaces--) > 0)
            putf(data, ' ');
}

bool printf_check_string(char * str)
{
    if (printf_interface && printf_interface->printf_check_string)
        return printf_interface->printf_check_string(str);

    return true;
}

void printf_write_unicode(void * unicode, void *data, putc_f putf, printf_state_ref current)
{
    if (printf_interface && printf_interface->printf_write_unicode)
        printf_interface->printf_write_unicode(unicode, data, putf, current, printf_write_all);
}

void printf_write_floatpoint(safe_double_ref unicode, int_t places, void *data, putc_f putf, printf_state_ref current)
{
    if (printf_interface && printf_interface->printf_write_floatpoint)
        printf_interface->printf_write_floatpoint(unicode, places, data, putf, current, printf_write_all);
}

void printf_attach(printf_interface_p interface)
{
    printf_interface = interface;
}

#define STREAM_CUR        (*((*fmtp) - 1))
#define STREAM_PEAK       (*(*fmtp))
#define STREAM_NEXT       (*fmtp = ((*fmtp + 1)), *c = STREAM_CUR)
print_word_size_t printf_translate_word(const char **fmtp, char * c)
{
    if ((STREAM_CUR == 'z') || (STREAM_CUR == 'j'))
    {
        STREAM_NEXT;
        return PRINTF_MACH_WORD_SIZE;
    }
    
    if (STREAM_CUR == 'l')
    {
        STREAM_NEXT;
        if (STREAM_CUR == 'l')
        {
            STREAM_NEXT;
            return PRINTF_COMPILER_LONG_LONG_SIZE;
        }
        return PRINTF_COMPILER_LONG_SIZE;
    }
    return PRINTF_COMPILER_INT_SIZE;
}

static int printf_character_to_int(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - '0';

    if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;

    if ((ch >= 'A') && (ch <= 'F'))
        return ch - 'A' + 10;

    return -1;
}

static int_t printf_translate_int(const char **fmtp, char * c, int base)
{
    int_t digit;
    int_t num;
    num        = 0;

    while (digit = printf_character_to_int(STREAM_CUR), digit < base && digit != -1)
    {
        STREAM_NEXT;
        num = num * base + digit;
    }

    return num;
}
#undef STREAM_CUR    
#undef STREAM_PEAK    
#undef STREAM_NEXT
#define STREAM_CUR    ((*(fmt - 1)))
#define STREAM_PEAK ((*(fmt)))
#define STREAM_NEXT (cur = *(fmt++))     

#define PRINTF_APPEND_NUMBER(state, word, _base, is_sign, va)                              \
{                                                                                          \
    state->base = _base;                                                                   \
                                                                                           \
    if (is_sign)                                                                           \
    {                                                                                      \
        switch (word)                                                                      \
        {                                                                                  \
        case PWS_INT_8:                                                                    \
            printf_write_machine_int((int_t) va_arg(va, uint8_t), state);                  \
            break;                                                                         \
        case PWS_INT_16:                                                                   \
            printf_write_machine_int((int_t) va_arg(va, uint16_t), state);                 \
            break;                                                                         \
        case PWS_INT_32:                                                                   \
            printf_write_machine_int((int_t) va_arg(va, uint32_t), state);                 \
            break;                                                                         \
        case PWS_INT_64:                                                                   \
            printf_write_machine_int((int_t) va_arg(va, uint64_t), state);                 \
            break;                                                                         \
        }                                                                                  \
    }                                                                                      \
    else                                                                                   \
    {                                                                                      \
        switch (word)                                                                      \
        {                                                                                  \
        case PWS_INT_8:                                                                    \
            printf_write_machine_unsigned_int((uint_t) va_arg(va, uint8_t), state);        \
            break;                                                                         \
        case PWS_INT_16:                                                                   \
            printf_write_machine_unsigned_int((uint_t) va_arg(va, uint16_t), state);       \
            break;                                                                         \
        case PWS_INT_32:                                                                   \
            printf_write_machine_unsigned_int((uint_t) va_arg(va, uint32_t), state);       \
            break;                                                                         \
        case PWS_INT_64:                                                                   \
            printf_write_machine_unsigned_int((uint_t) va_arg(va, uint64_t), state);       \
            break;                                                                         \
        }                                                                                  \
    }                                                                                      \
}

void printf_fmt_stream(void *data, putc_f putf, const char *fmt, va_list va)
{
    char temp_buf[50];
    char cur;

    STREAM_NEXT;
    while (cur)
    {
        char idn;
        bool alt_form;
        printf_state_t current;
        print_word_size_t word_size;

        if (cur != '%')
        {
            putf(data, cur);
            STREAM_NEXT;
            continue;
        }

        alt_form  = false;
        current.buffer        = temp_buf;
        current.leading_zeros = false;
        current.zero_x_hex    = false;
        current.ocal_prefix   = false;
        current.upper_case    = false;
        current.align_left    = false;
        current.width         = false;
        current.sign          = '\x00';

        while (STREAM_NEXT)
        {
            switch (cur)
            {
            case '-':
                current.align_left = 1;
                continue;
            case '0':
                current.leading_zeros = 1;
                continue;
            case '#':
                alt_form = true;
                continue;
            default:
                break;
            }
            break;
        }
        
        current.width    = printf_translate_int(&fmt, &cur, 10);
        word_size        = printf_translate_word(&fmt, &cur);
        
        idn = cur;
        STREAM_NEXT;
        switch (idn)
        {
        case 0:
            goto abort;
        case 'u':
        {
            PRINTF_APPEND_NUMBER((&current), word_size, 10, false, va);
            printf_write_all(data, putf, &current);
            break;
        }
        case 'd':
        case 'i':
        {
            PRINTF_APPEND_NUMBER((&current), word_size, 10, true, va);
            printf_write_all(data, putf, &current);
            break;
        }
        case 'p':
        {
            word_size = PRINTF_MACH_WORD_SIZE;
            current.zero_x_hex = 1;
            current.upper_case = 1;
            PRINTF_APPEND_NUMBER((&current), word_size, 16, false, va);
            printf_write_all(data, putf, &current);
            break;
        }
        case 'X':
            current.upper_case = 1;
        case 'x':
        {
            PRINTF_APPEND_NUMBER((&current), word_size, 16, false, va);
            printf_write_all(data, putf, &current);
            break;
        }
        case 'o':
        {
            if (alt_form)
                current.ocal_prefix = 1;
            PRINTF_APPEND_NUMBER((&current), PRINTF_COMPILER_INT_SIZE, 8, false, va);
            printf_write_all(data, putf, &current);
            break;
        }
        case 'c':
        {
            putf(data, (char)(va_arg(va, uint8_t)));
            break;
        }
        case 'C':
        {
            uint16_t wide;
            wide = va_arg(va, uint16_t);
            if (LOBYTE(wide) && HIBYTE(wide))
            {
                putf(data, '?');
                break;
            }
#if PRINTF_UNICODE_LAZY_ENDIAN
            putf(data, LOBYTE(wide));
#else
            putf(data, HIBYTE(wide));
#endif
            break;
        }
        case 's':
        {
            current.buffer = va_arg(va, char *);
            if (!printf_check_string(current.buffer))
                break;
            printf_write_all(data, putf, &current);
            break;
        }
        case 'S':
        {
            printf_write_unicode(va_arg(va, void *), data, putf, &current);
            break;
        }
        case 'H':
        case 'D':
        {
            int_t places;
            safe_double_t x;

            places    = 0;
            x.word        = va_arg(va, uint64_t);

            if (STREAM_CUR == ':')
            {
                STREAM_NEXT;
                places = printf_translate_int((char **)&fmt, &cur, 10);
            }
            
            printf_write_floatpoint(&x, places, data, putf, &current);
            break;
        }
        case 'b':
        {
            current.buffer = va_arg(va, bool) ? "true" : "false";
            printf_write_all(data, putf, &current);
            break;
        }
        case '%':
        {
            putf(data, '%');
        }
        default:
            break;
        }
    }
abort:
    ;
}

static void printf_strstream_putcf(void *data, char c)
{
    printf_string_stream_p stream;

    stream = (printf_string_stream_p)data;

    if (stream->num_chars < stream->dest_capacity)
        stream->dest[stream->num_chars] = c;

    stream->num_chars++;
}

size_t printf_fmt_string(char *str, size_t size, const char *format, va_list ap)
{
    printf_string_stream_t data;

    if (size < 1)
        return 0;

    data.dest = str;
    data.dest_capacity = size - 1;
    data.num_chars = 0;

    printf_fmt_stream(&data, printf_strstream_putcf, format, ap);

    if (data.num_chars < data.dest_capacity)
        data.dest[data.num_chars] = '\0';
    else
        data.dest[data.dest_capacity] = '\0';

    return data.num_chars;
}
