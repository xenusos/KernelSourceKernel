/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/
#include <xenus.h>
#include <kernel/libx/xenus_memory.h> 
#include "../Boot/access_system.h"

#define DEFINED_IN_SOURCE_BST

typedef void(*node_deallocation_notifier_t)(uint64_t hash, void * buffer);

struct bst_s;
typedef struct link_s
{
    uint64_t hash;
    struct link_s * next;
    struct link_s * before;
    node_deallocation_notifier_t cb;
    struct bst_s * chain;
    void * buffer;
} *node_p, *link_ref, link_t;

typedef struct bst_s
{
    node_p top;
} *bst_p, *bst_ref, bst_t;

#include <kernel/libx/xenus_bst.h>

XENUS_SYM error_t bst_allocate(bst_p * chain)
{
    bst_p alloc;

    alloc = (bst_p)zalloc(sizeof(bst_t));
    if (!alloc)
        return XENUS_ERROR_OUT_OF_MEMORY;

    *chain = alloc;
    return XENUS_OKAY;
}

static node_p bst_insert_node(node_p cur, node_p link)
{
    if (cur == link)
        return NULL;

    if (!cur)
        return link;
 
    if (link->hash < cur->hash)
        cur->before = bst_insert_node(cur->before, link);
    else if (link->hash > cur->hash)
        cur->next = bst_insert_node(cur->next, link);

    return cur;
}

XENUS_EXPORT error_t bst_allocate_node(
    bst_p    bst,
    uint64_t hash,
    size_t   length,
    node_deallocation_notifier_t cb,
    node_p * out_link_handle,
    void **  out_buffer)
{
    node_p link;

    if (!bst)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!length)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    link = (node_p)zalloc(sizeof(link_t));
    if (!link)
        return XENUS_ERROR_OUT_OF_MEMORY;

    link->chain = bst;
    link->hash = hash;
    link->cb = cb;
    link->buffer = zalloc(length);

    if (!bst->top)
        bst->top = link;
 
    bst_insert_node(bst->top, link);

    if (out_link_handle)    *out_link_handle = link;
    if (out_buffer)         *out_buffer = link->buffer;

    return XENUS_OKAY;
}

static node_p bst_search_recursive(node_p node, uint64_t hash)
{
    if (!node)
        return NULL;

    if (node->hash == hash)
        return node;
    else if (hash < node->hash)
        return bst_search_recursive(node->before, hash);
    else
        return bst_search_recursive(node->next, hash);
}

XENUS_EXPORT error_t bst_get(
    bst_p     chain,
    uint64_t  hash,
    node_p *  out_link_handle,  
    void **   out_buffer)
{
    node_p node;

    if (!chain)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    node = bst_search_recursive(chain->top, hash);
    if (!node)
        return XENUS_ERROR_BST_NOT_NOT_FOUND;

    if (out_buffer)
        *out_buffer = node->buffer;

    if (out_link_handle)
        *out_link_handle = node;

    return XENUS_OKAY;
}

static void bst_transverse(node_p node, node_iterator_t iterator, void * ctx)
{
    if (!node)
        return;

    bst_transverse(node->before, iterator, ctx);
    iterator(node->hash, node->buffer, node);
    bst_transverse(node->next, iterator, ctx);
}

XENUS_EXPORT error_t bst_iterator(
    bst_p chain,
    node_iterator_t iterator,
    void * ctx
)
{
    if (!chain)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    bst_transverse(chain->top, iterator, ctx);

    return XENUS_OKAY;
}

static void bst_destroy_node(node_p node)
{
    if (node->cb)
        node->cb(node->hash, node->buffer);
    if (node->buffer)
        free(node->buffer);
    free(node);
}

static node_p bst_transverse_min(node_p node)
{
    while (node->before != NULL) 
        node = node->before;
    return node;
}

static node_p bst_deallocate_ex(node_p node, uint64_t hash)
{
    node_p successor;
    node_p minimum;

    if (hash < node->hash)
    {
        node->before = bst_deallocate_ex(node->before, hash);
        return node;
    }
    else if (hash > node->hash)
    {
        node->next = bst_deallocate_ex(node->next, hash);
        return node;
    }

    if (!node->before)
    {
        successor = node->next;
        bst_destroy_node(node);
        return successor;
    }
    else if (!node->next)
    {
        successor = node->before;
        bst_destroy_node(node);
        return successor;
    }

    minimum = bst_transverse_min(node->next);

    // node exhange
    node->hash = minimum->hash;
    node->buffer = minimum->buffer;
    minimum->buffer = NULL;
    
    // recursively delete the node we exchanged places with 
    node->next = bst_deallocate_ex(node->next, minimum->hash);
    return node;
}

XENUS_EXPORT error_t bst_deallocate_handle(node_p handle)
{
    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    bst_deallocate_ex(handle, handle->hash);
    return XENUS_OKAY;
}

static error_t bst_deleteall(node_p node)
{
    if (!node)
        return XENUS_OKAY;

    bst_deleteall(node->before);
    bst_deleteall(node->next);

    bst_destroy_node(node);

    return XENUS_OKAY;
}

XENUS_EXPORT error_t bst_deallocate_search(bst_p chain, uint64_t hash)
{
    bst_deallocate_ex(chain->top, hash);
    return XENUS_OKAY;
}

XENUS_EXPORT error_t bst_destroy(bst_p chain)
{
    error_t err;
    
    err = bst_deleteall(chain->top);
    if (ERROR(err))
        return err;
    
    free(chain);
    return XENUS_OKAY;
}