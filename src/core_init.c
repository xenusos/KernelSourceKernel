/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>     // ints, common types, etc
#include "core_init.h" // pre-decleration 

// Init functions:
#include "LibX\_xenus_libx.h"
#include "LibX\xenus_threads.h"
#include "SysX\sys_main.h"
#include "core_exports.h"

// Dependencies 
#include "access_sys.h"

void xenus_start();


void xenus_stage_2(void * xenus_base, void *  security, uint32_t sec_len)
{
	// functions in _xenus_bootstapper are now callable, but nothing else is setup

	kernel_fpu_begin();

	// Startup libx 
	if (ERROR(xlib_start()))
	{
		panic("failed to init xlib");
		goto ret;
	}

	//// exports uses libx types, must be called post init xlib_start
	init_exports(xenus_base);
	
	//// look up linux symbols, create msabi compliant functions, append to export table
	add_symbols(); // pre-june, this had to be in stage 2

	ret:
	kernel_fpu_end();
}

void xenus_state_3()
{
	// xlib has been initialized and all System V functions that we require have been converted into something we can actually call

	thread_fpu_lock();
	xenus_start();
	thread_fpu_unlock();
}



void xenus_start()
{
	// let's call this part the last entrypoint whereby everything is setup and we can execute whatever we want. 
	
	size_t msleep(size_t msecs);

	print("sleeping\n");
	msleep(1100);
	print("sleeping\n");
	msleep(1100);
	print("done\n");
}