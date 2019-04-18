/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/
#include <xenus.h>
#include <kernel/libx/xenus_memory.h>
#include <kernel/libx/xenus_dbg.h>
#include "../Boot/access_system.h"
#include "xenus_debug.h"

static void * mutex;
static char   line_buffer[STDOUT_BUFFER_LENGTH_W_NULL];

l_int puts(const char * str)
{
    l_int ret;
    size_t copy_len = strlen(str) + 1;

    mutex_lock(mutex);

    memcpy(line_buffer, str, copy_len);
    (*(uint16_t*)(((uint64_t)line_buffer) + copy_len)) = *(uint16_t*)("\n\00");

    ret = print(line_buffer);
    
    mutex_unlock(mutex);
    return ret;
}

l_int printf(const char *fmt, ...)
{
    va_list ap;
    size_t length;
    l_int ret;

    mutex_lock(mutex);

    va_start(ap, fmt);

    vsnprintf(line_buffer, STDOUT_BUFFER_LENGTH, fmt, ap);
    va_end(ap);

    ret = print(line_buffer);

    mutex_unlock(mutex);
    return ret;
}

__declspec(noreturn) void  panicf(const char *fmt, ...)
{
    static char msg[STDOUT_BUFFER_LENGTH_W_NULL];
    va_list ap;
    msg[0] = 0;
    va_start(ap, fmt);
    vsnprintf(msg, STDOUT_BUFFER_LENGTH, fmt, ap); //TODO check response
    va_end(ap);
    panic(msg);
}

void xlib_dbg_init()
{
    mutex = mutex_init();
}