/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/
#include <xenus.h>                            // int types, common types, etc
#include <kernel/libx/xenus_threads.h>        // predecleration of public apis

#include <kernel/libx/xenus_chain.h>          // deps
#include <kernel/libx/xenus_list_dyn.h>       // deps
#include <kernel/libx/xenus_memory.h>         // deps
#include "../Boot/access_system.h"            // deps


XENUS_SYM error_t thread_indexing_create(uint_t max_threads, thread_index_counter_p * tic)
{
    dyn_list_head_p list;
    thread_index_counter_p head;
    mutex_k mutex;

    if (!tic)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    mutex = mutex_init();
    if (!mutex)
        return XENUS_ERROR_INTERNAL_ERROR;

    list = DYN_LIST_CREATE(uint32_t);

    if (!list)
    {
        mutex_destroy(mutex);
        return XENUS_ERROR_TLS_FAILED_TO_CREATE_LIST;
    }

    head = malloc(sizeof(thread_index_counter_t));

    if (!(head))
    {
        dyn_list_destory(list);
        mutex_destroy(mutex);
        return XENUS_ERROR_TLS_FAILED_TO_CREATE_LIST;
    }

    head->list            = list;
    head->max_threads     = MAX(max_threads, 2);
    head->counter         = 0;
    head->mutex           = mutex;

    *tic = head;
    return XENUS_OKAY;
}

XENUS_SYM error_t thread_indexing_destory(thread_index_counter_p tic)
{
    if (!tic)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (tic->list)
        if (ERROR(dyn_list_destory(tic->list)))
            return XENUS_ERROR_TLS_FAILED_TO_DESTORY;

    if (tic->mutex)
        mutex_destroy(tic->mutex);

    free(tic);
    return XENUS_OKAY;
}

XENUS_SYM error_t thread_indexing_register(thread_index_counter_p tic)
{
    error_t err;
    uint32_t * task;
    uint32_t pid;

    if (!tic)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    pid = thread_geti();

    if ((err = thread_indexing_get(tic, 0)) != XENUS_ERROR_TLS_NOT_REGISTERED)
        return err;

    if (tic->counter >= tic->max_threads)
        panicf("Thread counter limit exceeded. (max threads: %i)", tic->max_threads); // this is panic worthy; if you're doing thread-based indexing with a static amount of expected threads and count too many, best case: your code is wrong, worst case: you start overflowing everywhere.

    tic->counter++;

    mutex_lock(tic->mutex);
    
    err = dyn_list_append(tic->list, (void *)&task);

    if (!err)
        *task = pid;
    else
        err = XENUS_ERROR_TLS_FAILED_TO_ADD;

    mutex_unlock(tic->mutex);
    return XENUS_OKAY;
}

XENUS_SYM error_t thread_indexing_get(thread_index_counter_p tic, size_t * index)
{
    error_t ret;
    uint32_t id;

    if (!tic)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!index)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    id = thread_geti();

    mutex_lock(tic->mutex);
    ret = dyn_list_get_by_buffer((dyn_list_head_p)tic->list, &id, index);
    if (ret == XENUS_ERROR_LISTS_NOT_FOUND) //we need to treat a non-existent link as an error
        ret = XENUS_ERROR_TLS_NOT_REGISTERED;
    mutex_unlock(tic->mutex);
    return ret;
}
