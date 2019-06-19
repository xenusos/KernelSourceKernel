/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/
#include "xlib_start.h"
#include "xenus_debug.h"
#include "xenus_threading_generic.h"

void xlib_init()
{
    xlib_dbg_init();
    xlib_generic_init();
}