/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <kernel/libx/xenus_strings.h>
#include "../Boot/access_system.h"
#include "../Printf/xenus_print.h"

XENUS_EXPORT unsigned char _ctype[] = {
    __C,__C,__C,__C,__C,__C,__C,__C,                                        /* 0-7     */
    __C,__C | __S,__C | __S,__C | __S,__C | __S,__C | __S,__C,__C,          /* 8-15    */
    __C,__C,__C,__C,__C,__C,__C,__C,                                        /* 16-23   */
    __C,__C,__C,__C,__C,__C,__C,__C,                                        /* 24-31   */
    __S | __SP,__P,__P,__P,__P,__P,__P,__P,                                 /* 32-39   */
    __P,__P,__P,__P,__P,__P,__P,__P,                                        /* 40-47   */
    __D,__D,__D,__D,__D,__D,__D,__D,                                        /* 48-55   */
    __D,__D,__P,__P,__P,__P,__P,__P,                                        /* 56-63   */
    __P,__U | __X,__U | __X,__U | __X,__U | __X,__U | __X,__U | __X,__U,    /* 64-71   */
    __U,__U,__U,__U,__U,__U,__U,__U,                                        /* 72-79   */
    __U,__U,__U,__U,__U,__U,__U,__U,                                        /* 80-87   */
    __U,__U,__U,__P,__P,__P,__P,__P,                                        /* 88-95   */
    __P,__L | __X,__L | __X,__L | __X,__L | __X,__L | __X,__L | __X,__L,    /* 96-103  */
    __L,__L,__L,__L,__L,__L,__L,__L,                                        /* 104-111 */
    __L,__L,__L,__L,__L,__L,__L,__L,                                        /* 112-119 */
    __L,__L,__L,__P,__P,__P,__P,__C,                                        /* 120-127 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                        /* 128-143 */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                        /* 144-159 */
    __S | __SP,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P, /* 160-175 */
    __P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,__P,        /* 176-191 */
    __U,__U,__U,__U,__U,__U,__U,__U,__U,__U,__U,__U,__U,__U,__U,__U,        /* 192-207 */
    __U,__U,__U,__U,__U,__U,__U,__P,__U,__U,__U,__U,__U,__U,__U,__L,        /* 208-223 */
    __L,__L,__L,__L,__L,__L,__L,__L,__L,__L,__L,__L,__L,__L,__L,__L,        /* 224-239 */
    __L,__L,__L,__L,__L,__L,__L,__P,__L,__L,__L,__L,__L,__L,__L,__L };


XENUS_EXPORT size_t vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
    return printf_fmt_string(buf, size, fmt, ap);
}

XENUS_EXPORT size_t vsprintf(char *buf, const char *fmt, va_list ap)
{
    // Having this function available is such a bad idea
    return strlen(strcat(buf, "NoSBO")); // No StackBufferOverfollow
}