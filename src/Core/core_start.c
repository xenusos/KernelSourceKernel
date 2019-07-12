/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "core_start.h"
#include "core_self_test.h"
#include "core_loader.h"

void xenus_start()
{
    // let's call this part "the last entrypoint" whereby everything is setup and we can execute whatever we want. 

    // check a pe image for the fun of it
    if (!self_test()) 
        return;
    
    // load plugins
    xenus_launch_plugins();
}
