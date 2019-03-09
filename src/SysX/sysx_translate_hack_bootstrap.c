/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "sysx_translate_hack_bootstrap.h"
#include "sysx_linux_caller.h"
#include "sysx_translate_common_macros.h"

#define ADD_FUNCTION_UNOPTIMIZED(ignore, name)      ADD_SYMBOL_12_A(name)
#define ADD_FUNCTION_12(ignore, name)               ADD_SYMBOL_12_A(name)
#define ADD_FUNCTION_11(ignore, name)               ADD_SYMBOL_11_A(name)
#define ADD_FUNCTION_10(ignore, name)               ADD_SYMBOL_10_A(name)
#define ADD_FUNCTION_9(ignore, name)                ADD_SYMBOL_9_A(name)
#define ADD_FUNCTION_8(ignore, name)                ADD_SYMBOL_8_A(name)
#define ADD_FUNCTION_7(ignore, name)                ADD_SYMBOL_7_A(name)
#define ADD_FUNCTION_6(ignore, name)                ADD_SYMBOL_6_A(name)
#define ADD_FUNCTION_5(ignore, name)                ADD_SYMBOL_5_A(name)
#define ADD_FUNCTION_4(ignore, name)                ADD_SYMBOL_4_A(name)
#define ADD_FUNCTION_3(ignore, name)                ADD_SYMBOL_3_A(name)
#define ADD_FUNCTION_2(ignore, name)                ADD_SYMBOL_2_A(name)
#define ADD_FUNCTION_1(ignore, name)                ADD_SYMBOL_1_A(name)
#define ADD_FUNCTION_0(ignore, name)                ADD_SYMBOL_0_A(name)
    #include "bootstrap/bootstrap_all.macros"
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

#include "all\_generic_bootstrap.h"

#define FIXUP_BOOTSTRAP_FUNCTION(path, name) \
        (void *)(name ## _function)  = (void *)(bootstrap-> ## path ## . ## name); \
        (void *)(bootstrap-> ## path ## . ## name) = (void *)name;

#define ADD_FUNCTION_UNOPTIMIZED      FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_12               FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_11               FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_10               FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_9                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_8                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_7                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_6                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_5                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_4                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_3                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_2                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_1                FIXUP_BOOTSTRAP_FUNCTION
#define ADD_FUNCTION_0                FIXUP_BOOTSTRAP_FUNCTION

void translate_stage_zero_sysv(void * bootstrap_)
{
    bootstrap_t * bootstrap;

    bootstrap = (bootstrap_t *)bootstrap_;

    #include "bootstrap/bootstrap_all.macros"
}

#undef FIX_BOOTSTRAP_FUNCTION
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