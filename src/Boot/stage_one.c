/*
    Purpose: Stage one initializes the core parts of the kernel (XLIB)
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#define FORCE_PORTABLE_STRUCTS
#include <xenus.h>
#include "bootstrap_functions.h"

#include "../Printf/xenus_print.h"
#include "../PE/pe_loader.h"

#include "../Shutdown/shutdown_main.h"

void *          kernel_base              = 0;
linux_info_t    kernel_information       = { 0 };

START_POINT(stage_one)
{
    kernel_base           = xenos_start;
    kernel_information    = *info;

    if (ERROR(ps_buffer_configure(port_structs, port_structs_length)))
        return STAGE_ONE_ERR;

    //xlib_start();        - no longer present.
    //printf_allow_xlib(); - no longer present.
    pe_loader_init();
    shutdown_init();

    return 0;
}