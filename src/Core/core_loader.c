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
    return !top_level_plugin_tracking ? kErrorOutOfMemory : kStatusOkay;
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
    return kStatusOkay;
}

static error_t loader_read_csv_data(const char * buffer, uint_t buf_length, uint_t * offset, char * data, uint_t data_length)
{
#define MATCH_CHAR_SEPERATOR(c) (c == ' ' || c == ',' || c == '\r' || c == '\n' || c == '\x00')
    uint_t end_index;
    uint_t key_length;
    uint_t copy_length;
    uint_t start_index;

    key_length    = 0;
    start_index   = *offset;
    end_index     = *offset;

    // read until next seperator 
    for (NULL; end_index < buf_length; end_index++)
    {
        char cur;
        cur = buffer[end_index];

        if (MATCH_CHAR_SEPERATOR(cur))
            goto copy;

        key_length++;
    }
copy:

    // copy string and append null char
    copy_length = MIN(data_length - 1, key_length);
    memcpy(data, (const char *)(((size_t)buffer) + start_index), copy_length);
    data[copy_length] = '\x00';

    while (end_index < buf_length)
    {
        if (!(MATCH_CHAR_SEPERATOR(buffer[end_index])))
            break;
        end_index++;
    }

    *offset = end_index;
    return kStatusOkay; 
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
        loader_read_csv_data(str, length, &counter, name, sizeof(name));
        if (strlen(name) == 0) break;
        loader_read_csv_data(str, length, &counter, module, sizeof(module));
        if (strlen(module) == 0) break;
        loader_read_csv_data(str, length, &counter, path, sizeof(path));
        if (strlen(path) == 0) break;
        loader_append(name, module, path);
    }

    return kStatusOkay;
}

static error_t loader_load_from_file()
{
    const char * test = "LibOS, LibOS.XSYS, /Xenus/Kernel/LibOS.XSYS\n"
                        "SelfTestModule, SelfTest.XSYS, /Xenus/Kernel/Plugins/SelfTest.XSYS";
    loader_load_from_csv(test, 111);
    return kStatusOkay;
}

void xenus_launch_plugins()
{
    error_t err;

    if (ERROR(err = loader_init())) 
        panicf("_loader_init failed with %lli", err);

    if (ERROR(err = loader_load_from_file()))
        panicf("_loader_load_from_file failed with %lli", err);

    if (ERROR(err = loader_transfer()))
        panicf("_loader_transfer failed with %lli", err);
}