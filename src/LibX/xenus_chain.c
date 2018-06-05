/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <libx/xenus_chain.h>
#include <libx/xenus_memory.h> 
#include "../access_sys.h"

//////////////////////////////////////////////////////////////////////
// Chains
//////////////////////////////////////////////////////////////////////

// Double linked list with 64 bit hash | NULL <= x <=> y <=> z <=> a => NULL
typedef struct chain_s
{
    uint64_t hash;           // = b
    struct chain_s * next;   // = c
    struct chain_s * before; // = a
    void * _buf;             // this + sizeof(chain_t)
} chain_t;

XENUS_EXPORT error_t chain_allocate(void * bk, uint64_t hash, uint32_t length, void ** bkout, void ** out)
{
    chain_t * basekey;
    chain_t * link;
    chain_t * last;

    basekey = (chain_t *)bk;

    if (basekey)
        if (chain_get(bk, hash, NULL, NULL) == XENUS_OKAY)
            return XENUS_ERROR_CHAIN_ALREADY_PRESENT;

    link = malloc(sizeof(chain_t) + length);

    if (!link)
        return XENUS_ERROR_OUT_OF_MEMORY;

    memset(link, 0, sizeof(chain_t));

    link->hash = hash;
    link->_buf = (void *)((char *)link + sizeof(chain_t));

    if (!basekey)
        goto error;

    last = basekey;
    while (last->next != NULL) last = last->next;

    last->next = (struct chain_s *)link;
    link->before = (struct chain_s *)last;

    error:

    if (bkout)
        *bkout = bk;
    if (out)
        *out = link->_buf;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t chain_deallocate_ex(void * bk, uint64_t hash, _pre_chain_dealloc cb)
{
    chain_t * basekey;
    chain_t * last;
    chain_t * found;

    basekey = (chain_t *)bk;
    last = basekey;

    if (!basekey)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;


    if (last->hash = hash) goto cleanup;

    while (last->next != NULL)
    {
        if ((last = last->next)->hash == hash)
        {
            cleanup:
            found = last;

            if (found->before)
                found->before->next = found->next;

            if (found->next)
                found->next->before = found->before;

            if (cb)
                cb(found->_buf);

            free(found);
            return XENUS_OKAY;
        }
    }

    return XENUS_ERROR_CHAIN_NOT_FOUND;
}

XENUS_EXPORT error_t chain_deallocate(void * bk, uint64_t hash)
{
    return chain_deallocate_ex(bk, hash, NULL);
}

XENUS_EXPORT error_t chain_get(void * bk, uint64_t hash, void ** bnext, void ** out)
{
    chain_t * basekey;
    chain_t * last;

    basekey = (chain_t *)bk;

    if (!basekey)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if ((last = basekey)->hash == hash)
     goto retx;
   
   while (last->next != NULL)
   {
       if ((last = last->next)->hash == hash)
       {
         retx:
         if (out)
             *out = last->_buf;
         if (bnext)
                 *bnext = last->next;
         return XENUS_OKAY;
       }
   }

    return XENUS_ERROR_LINK_NOT_FOUND;
}
