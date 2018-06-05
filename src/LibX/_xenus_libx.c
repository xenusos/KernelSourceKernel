/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "_xenus_libx.h"
#include "_xenus_threads.h"

#include "../access_sys.h"
error_t xlib_start()
{
	
	return xlib_thread_start();
}