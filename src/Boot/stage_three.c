/*
    Purpose: Stage three enables FPU and FPU related stuff [including stack alignment]. Do note, we pass execution onto core_start.c after this. 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "stage_three.h"

// stack fixup
#include "../StackFixup/stack_realigner.h"

// fpu enable functions
#include <kernel/libx/xenus_threads.h>

// printf
#include "../Printf/xenus_print.h"
#include "../Printf/print_floating.h"
#include "../Printf/print_unicode.h"
#include "../Printf/print_strchk.h"

// core
#include "../Core/core_start.h"

// panic
#include "access_system.h"

static void stage_three_point_5();

static printf_interface_t printf_interface_global =
{
    xenus_printf_check_string,
    xenus_printf_write_unicode,
    xenus_printf_write_floatpoint
};

START_POINT(stage_three)
{
    stack_realigner((size_t(*)(size_t))stage_three_point_5, 0);
    return 0;
}

void stage_three_point_5()
{
    chkstack_t n;

    if (((size_t)(&n) % 16) != 0)
        panic("xenus_stage_4: Xenus: not aligned");

    thread_fpu_lock();                          // allow fpu usage on the main thread
    xenus_printf_init_strs();
    printf_attach(&printf_interface_global);    // extend printf with fpu and unicode usage
    xenus_start();                              // execute main function
    thread_fpu_unlock();                        // restore fpu state and disable fpu protection on main threade
}
