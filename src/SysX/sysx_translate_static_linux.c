/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "sysx_translate_static_linux.h"
#include "sysx_linux_caller.h"
#include "sysx_translate_common_macros.h"

//HACK: - we don't have access to Boot/access_system.h yet
// Technically, xlib isn't initialized, but in practice that isn't required anymore 
#include <kernel/libx/xenus_strings.h>
#include "../Boot/bootstrap_functions.h"
static int stage_1_printf(const char *fmt, ...)
{
    char msg[2049];
    va_list ap;

    msg[0] = 0;

    va_start(ap, fmt);
    vsnprintf(msg, 2048, fmt, ap);
    va_end(ap);

    return g_functions->dbg.print(msg);
}

//HACK: - defined in sys_translate_bootstrap_hack.c
extern size_t XENUS_SYM kallsyms_lookup_name(size_t name);
extern size_t XENUS_SYM __symbol_get(size_t name);

#define ADD_FUNCTION_UNOPTIMIZED      ADD_SYMBOL_12_A
#define ADD_FUNCTION_12               ADD_SYMBOL_12_A
#define ADD_FUNCTION_11               ADD_SYMBOL_11_A
#define ADD_FUNCTION_10               ADD_SYMBOL_10_A
#define ADD_FUNCTION_9                ADD_SYMBOL_9_A
#define ADD_FUNCTION_8                ADD_SYMBOL_8_A
#define ADD_FUNCTION_7                ADD_SYMBOL_7_A
#define ADD_FUNCTION_6                ADD_SYMBOL_6_A
#define ADD_FUNCTION_5                ADD_SYMBOL_5_A
#define ADD_FUNCTION_4                ADD_SYMBOL_4_A
#define ADD_FUNCTION_3                ADD_SYMBOL_3_A
#define ADD_FUNCTION_2                ADD_SYMBOL_2_A
#define ADD_FUNCTION_1                ADD_SYMBOL_1_A
#define ADD_FUNCTION_0                ADD_SYMBOL_0_A
    #include "linux/sys_functions_all.macros"
#undef ADD_FUNCTION_UNOPTIMIZED
#undef ADD_FUNCTION_12            
#undef ADD_FUNCTION_11            
#undef ADD_FUNCTION_10            
#undef ADD_FUNCTION_9            
#undef ADD_FUNCTION_8            
#undef ADD_FUNCTION_7            
#undef ADD_FUNCTION_6            
#undef ADD_FUNCTION_5            
#undef ADD_FUNCTION_4            
#undef ADD_FUNCTION_3            
#undef ADD_FUNCTION_2            
#undef ADD_FUNCTION_1    
#undef ADD_FUNCTION_0

// Swap ADD_FUNCTION to a macro containing [function_ptr = kallsyms_lookup_name/__symbol_get("function2")]

#define LOOKUP_SYM(name)                                                                                            \
    name ## _function = (void *)kallsyms_lookup_name((size_t)(#name));                                                \
    if (!(name ## _function)) name ## _function = (void *)__symbol_get((size_t)(#name));                            \
    if (!(name ## _function)) stage_1_printf("WARNING: SYMBOL %s COULDN'T BE IMPORTED. Prepare for OOPS\n", #name);

#define ADD_FUNCTION_UNOPTIMIZED      LOOKUP_SYM
#define ADD_FUNCTION_12               LOOKUP_SYM
#define ADD_FUNCTION_11               LOOKUP_SYM
#define ADD_FUNCTION_10               LOOKUP_SYM
#define ADD_FUNCTION_9                LOOKUP_SYM
#define ADD_FUNCTION_8                LOOKUP_SYM
#define ADD_FUNCTION_7                LOOKUP_SYM
#define ADD_FUNCTION_6                LOOKUP_SYM
#define ADD_FUNCTION_5                LOOKUP_SYM
#define ADD_FUNCTION_4                LOOKUP_SYM
#define ADD_FUNCTION_3                LOOKUP_SYM
#define ADD_FUNCTION_2                LOOKUP_SYM
#define ADD_FUNCTION_1                LOOKUP_SYM
#define ADD_FUNCTION_0                LOOKUP_SYM

void lookup_symbols()
{
    #include "linux/sys_functions_all.macros"
}

#undef LOOKUP_SYM
#undef ADD_FUNCTION_UNOPTIMIZED
#undef ADD_FUNCTION_12            
#undef ADD_FUNCTION_11            
#undef ADD_FUNCTION_10            
#undef ADD_FUNCTION_9            
#undef ADD_FUNCTION_8            
#undef ADD_FUNCTION_7            
#undef ADD_FUNCTION_6            
#undef ADD_FUNCTION_5            
#undef ADD_FUNCTION_4            
#undef ADD_FUNCTION_3            
#undef ADD_FUNCTION_2            
#undef ADD_FUNCTION_1            
#undef ADD_FUNCTION_0    
