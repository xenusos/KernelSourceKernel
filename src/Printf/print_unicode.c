/*
    Purpose: Stage 3 printf unicode support 
    Author: Reece W.
    License: All rights reserved (2018) J. Reece Wislon 
*/  
#include <xenus.h>
#include "../Boot/access_system.h"
#include <kernel/libx/xenus_memory.h>
#define STAGE_3
#include "xenus_print.h"
#include "print_unicode.h"

#define READ_NEXT lower = *(strbuf++), upper = *(strbuf++)
static int_t _strwlen(uint8_t * strbuf)
{
    uint8_t lower, upper;
    uint_t ret;
    ret = 0;
    for (READ_NEXT; lower != 0 || upper != 0; READ_NEXT)
        ret++;
    return ret;
}

// We ABSOLUTELY CAN NOT do anything with unicode. we **must** use ascii only. the best we can do is strip out the ascii-chars and send them to not-so-std-out
void xenus_printf_write_unicode(void * glhf, void *data, putc_f putf, printf_state_ref current, printf_write_all_t write)
{
    uint8_t * strbuf;
    char * ascii_ish;
    uint8_t lower, upper;
    bool no_complementary_print;
    int i;

    i        = 0;
    lower    = 0;
    upper    = 0;
    strbuf   = glhf;

    ascii_ish = zalloc(_strwlen(strbuf) + 1);

    if (!ascii_ish)
        return;

    no_complementary_print = false;
    while (READ_NEXT, lower || upper)
    {
        uint8_t cur;

        if (upper && lower)
        {
            if (no_complementary_print) 
                continue;
            no_complementary_print = true;
            ascii_ish[i++] = (char)'?';
            continue;
        }

        no_complementary_print = false;
        
        #if PRINTF_UNICODE_LAZY_ENDIAN
            cur = lower;
        #else
            cur = upper;
        #endif

        ascii_ish[i++] = (char)(cur > 0x7F ? '?' : cur);
    }

    current->buffer = ascii_ish;
    write(data, putf, current);
    free(ascii_ish);
}
#undef READ_NEXT