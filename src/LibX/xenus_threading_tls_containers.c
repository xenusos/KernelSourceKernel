/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/
#include <xenus.h>                             // int types, common types, etc
#include <kernel/libx/xenus_threads.h>         // predecleration of public apis

#include <kernel/libx/xenus_bst.h>           // deps
#include <kernel/libx/xenus_list_dyn.h>        // deps
#include <kernel/libx/xenus_memory.h>          // deps
#include "../Boot/access_system.h"             // deps

bst_p * tls_get_container(uint64_t type)
{
    thread_storage_data_p tls_;
    tls_ = tls();

    if (type == TLS_TYPE_GENERIC)
        return (bst_p *)&tls_->tls_container_generic_a;

    if (type == TLS_TYPE_GENERIC_2)
        return (bst_p *)&tls_->tls_container_generic_b;

    if (type == TLS_TYPE_GENERIC_3)
        return (bst_p *)&tls_->tls_container_generic_c;

    if (type == TLS_TYPE_XIP)
        return (bst_p *)&tls_->tls_container_xstatic_ip;

    if (type == TLS_TYPE_XGLOBAL)
        return (bst_p *)&tls_->tls_container_xstatic_global;

    panicf("Unknown container tls type %lli\n", type);
    return NULL;
}

XENUS_EXPORT error_t _thread_tls_allocate(uint64_t type, uint64_t hash, size_t length, void ** out_handle, void ** out_buffer)
{
    error_t err;
    bst_p * container;

    thread_enable_cleanup();
    
    container = tls_get_container(type);

    if (!(*container))
    {
        err = bst_allocate(container);
        if (ERROR(err))
            return err;
    }
    
    return bst_allocate_node(*container, hash, length, NULL, (node_p *)out_handle, out_buffer);
}

XENUS_EXPORT error_t _thread_tls_deallocate_hash(uint64_t type, uint64_t hash)
{
    bst_p container;

    if (!(container = *tls_get_container(type))) 
        return XENUS_ERROR_CONTAINER_NOT_FOUND;
    
    return bst_deallocate_search(container, hash);
}

XENUS_SYM error_t     _thread_tls_deallocate_handle(void * handle)
{
    return bst_deallocate_handle((node_p)handle);
}

XENUS_SYM error_t     _thread_tls_get(uint64_t type, uint64_t hash, void ** out_handle, void ** out_buffer)
{
    bst_p container;

    if (!(container = *tls_get_container(type)))
        return XENUS_ERROR_CONTAINER_NOT_FOUND;
    
    return bst_get(container, hash, (node_p *)out_handle, out_buffer);
}

error_t _thread_tls_cleanup(uint64_t type)
{
    bst_p container;

    if (!(container = *tls_get_container(type)))
        return XENUS_ERROR_CONTAINER_NOT_FOUND;
    
    return bst_destory(container);
}

error_t _thread_tls_cleanup_all()
{
    for (unsigned int i = 0; i < TLS_MAX_TYPE; i++)
        _thread_tls_cleanup(i);

    return XENUS_OKAY;
}