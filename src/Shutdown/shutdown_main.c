/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/
#include <xenus.h>
#include "../Boot/access_system.h"
#include <kernel/libx/xenus_libx.h>
#include "shutdown_main.h"
#include "../StackFixup/stack_realigner.h"

static linked_list_head_p shutdown_handlers;

static void on_shutdown()
{
    // TODO: implement a stable shutdown mechanism
}

void shutdown_add_hook(shutdown_handler_p handler)
{
    linked_list_entry_p entry;
    
    entry = linked_list_append(shutdown_handlers, sizeof(shutdown_handlers));
    
    ASSERT(entry, "couldn't append shutdown handler");

    *(shutdown_handler_p*)entry->data = handler;
}

void shutdown_init(void)
{
    shutdown_handlers = linked_list_create();
    
    ASSERT(shutdown_handlers, "out of memory.");

    //reload(on_shutdown);
}