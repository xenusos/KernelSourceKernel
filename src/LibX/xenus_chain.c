/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <kernel/libx/xenus_chain.h>
#include <kernel/libx/xenus_memory.h> 
#include "../Boot/access_system.h"

XENUS_SYM error_t chain_allocate(chain_p * chain)
{
    chain_p alloc;

    if (!(alloc = (chain_p)zalloc(sizeof(chain_t))))
        return XENUS_ERROR_OUT_OF_MEMORY;

    *chain = alloc;
    return XENUS_OKAY;
}

XENUS_SYM error_t chain_allocate_link(
    chain_p  chain,
    uint64_t hash,                        // REQUIRED: arbitrary hash
    size_t length,                        // REQUIRED: length of the required buffer to allocate 
    chains_deallocation_notifier_t cb,    // OPTIONAL: deallocation notifier 
    link_p * out_link_handle,             // OPTIONAL / OUT: link handle
    void **  out_buffer)                  // OPTIONAL / OUT: requested buffer (REQUIRED / out)
{
    link_p link;

    if (!chain)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!length)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (chain->bottom && STRICTLY_OKAY(chain_get(chain, hash, out_link_handle, out_buffer)))
        return XENUS_STATUS_LINK_ALREADY_PRESENT;

    if (!(link = (link_p)zalloc(sizeof(link_t) + length)))
        return XENUS_ERROR_OUT_OF_MEMORY;

    link->chain   = chain;
    link->hash    = hash;
    link->cb      = cb;
    link->_buf    = (void *)(((size_t)link) + sizeof(link_t));

    if (chain->tail)
    {
        link_p last;

        last = chain->tail;

        last->next    = (struct link_s *)link;
        link->before  = (struct link_s *)last;
    }
    else
    {
        chain->bottom = link;
    }

    chain->tail = link;

    if (out_link_handle)    *out_link_handle  = link;
    if (out_buffer)         *out_buffer       = link->_buf;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t chain_get(
    chain_p   chain,                    // REQUIRED: link handle to begin searching from
    uint64_t  hash,                     // REQUIRED: hash to search for
    link_p *  out_link_handle,          // OPTIONAL: 
    void **   out_buffer)               // OPTIONAL: 
{
    if (!chain)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    for (link_t * current = chain->bottom; current != NULL; current = current->next)
    {
        if (current->hash == hash)
        {
            if (out_buffer)
                *out_buffer = current->_buf;

            if (out_link_handle)
                *out_link_handle = current;

            return XENUS_OKAY;
        }
    }
    return XENUS_ERROR_LINK_NOT_FOUND;
}

XENUS_EXPORT error_t chain_iterator(
    chain_p chain,
    chains_iterator_t iterator,
    void * ctx
)
{
    if (!chain)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    for (link_t * current = chain->bottom; current != NULL; current = current->next)
        iterator(current->hash, current->_buf, ctx);

    return XENUS_OKAY;
}

XENUS_SYM error_t chain_deallocate_handle(link_p handle)
{
    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (handle->before)
        handle->before->next = handle->next;

    if (handle->next)
        handle->next->before = handle->before;

    if (handle->cb)
        handle->cb(handle->hash, handle->_buf);

    if (handle->chain->bottom == handle)
        handle->chain->bottom = handle->next;

    if (handle->chain->tail == handle)
        handle->chain->tail = handle->before;

    free(handle);
    return XENUS_OKAY;
}

XENUS_SYM error_t _chain_deallocate_ex(chain_p chain, bool deleteAll, uint64_t hash)
{
    if (!chain)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    for (link_t * current = chain->bottom; current != NULL; current = current->next)
    {
        if (deleteAll || (current->hash == hash))
        {
            chain_deallocate_handle(current);

            if (!deleteAll)
                return XENUS_OKAY;
        }
    }

    if (deleteAll) return XENUS_OKAY;
    else return XENUS_ERROR_CHAIN_NOT_FOUND;
}

XENUS_SYM error_t chain_deallocate_search(chain_p chain, uint64_t hash)
{
    return _chain_deallocate_ex(chain, false, hash);
}

XENUS_SYM error_t chain_destory(chain_p chain)
{
    error_t err;
    if (ERROR(err = _chain_deallocate_ex(chain, true, 0)))
        return err;
    free(chain);
    return XENUS_OKAY;
}