/*
    Purpose:  
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#pragma once
#include <kernel/shutdown/shutdown.h>

extern void shutdown_add_hook(shutdown_handler_p handler);
extern void shutdown_init(void);
