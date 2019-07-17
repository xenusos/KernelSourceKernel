/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
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
    error_t err;
    size_t fs;
    void * buf;
    void * hd;

    fs = file_length(BOOTSTRAP_CSV);
    ASSERT(fs, "file size of " BOOTSTRAP_CSV " was zero");

    buf = malloc(fs);
    if (!buf)
        return XENUS_ERROR_OUT_OF_MEMORY;

    hd = file_open_readonly(BOOTSTRAP_CSV);
    if (!hd)
        return XENUS_ERROR_INTERNAL_ERROR;

    file_read(hd, 0, buf, fs);
    file_close(hd);

    loader_load_from_csv((const char *)buf, fs);
    
    free(buf);
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