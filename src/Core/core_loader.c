/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "../Boot/access_system.h"
#include "core_loader.h"

#include <kernel/libx/xenus_libx.h>
#include "core_plugins.h"            

static linked_list_head_p top_level_plugin_tracking;

static error_t loader_init()
{
    top_level_plugin_tracking = linked_list_create();
    return !top_level_plugin_tracking ? XENUS_ERROR_OUT_OF_MEMORY : XENUS_OKAY;
}

static error_t loader_append(const char * name, const char * iat, const char * path)
{
    pe_handle_h nullie;
    void * nulleo;
    return plugins_iterative_load(top_level_plugin_tracking, iat, name, path, false, &nullie, &nulleo);
}

static error_t loader_transfer()
{
    plugins_initialize(top_level_plugin_tracking);
    plugins_start(top_level_plugin_tracking);
    return XENUS_OKAY;
}

static error_t loader_load_from_csv(const char * str, uint_t length)
{
    char name[PE_MAX_MODULE_LENGTH];
    char module[PE_MAX_MODULE_LENGTH];
    char path[255];
    size_t counter;

    counter = 0;

    while (true)
    {
        error_t err;

        err = csv_process_buffer(str, length, &counter, name, sizeof(name), NULL);
        if ((err == XENUS_ERROR_CSV_EOS) || (err == XENUS_STATUS_CSV_READ_ALL))
            break;

        err = csv_process_buffer(str, length, &counter, module, sizeof(module), NULL);
        if ((err == XENUS_ERROR_CSV_EOS) || (err == XENUS_STATUS_CSV_READ_ALL))
            break;

        err = csv_process_buffer(str, length, &counter, path, sizeof(path), NULL);
        if (err == XENUS_ERROR_CSV_EOS)
            break;

        loader_append(name, module, path);

        if (err == XENUS_STATUS_CSV_READ_ALL)
            break;
    }

    return XENUS_OKAY;
}

static error_t loader_load_from_file()
{
    const char * test = "LibOS, LibOS.XSYS, /Xenus/Kernel/LibOS.XSYS\n"
                        "SelfTestModule, SelfTest.XSYS, /Xenus/Kernel/Plugins/SelfTest.XSYS";
    loader_load_from_csv(test, 111);
    return XENUS_OKAY;
}

void xenus_launch_plugins()
{
    error_t err;

    err = loader_init();
    if (ERROR(err)) 
        panicf("_loader_init failed with %lli", err);

    err = loader_load_from_file();
    if (ERROR(err))
        panicf("_loader_load_from_file failed with %lli", err);

    err = loader_transfer();
    if (ERROR(err))
        panicf("_loader_transfer failed with %lli", err);
}