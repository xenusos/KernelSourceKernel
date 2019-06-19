/*
    Purpose: Dynamic list
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/

#include <xenus.h>
#include <kernel/libx/xenus_list_dyn.h>
#include <kernel/libx/xenus_memory.h> 
#include "../Boot/access_system.h"

#ifndef NO_LIBX_SAFETY
    #define CHK_LIST                                   \
        if (!head)                                     \
            return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;   \
                                                       \
        if (head->hash != XENUS_HASHCODE_LIST_DYN)     \
            return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    
#else
    #define CHK_LIST
#endif 

XENUS_EXPORT dyn_list_head_p _dyn_list_create(size_t length)
{
    dyn_list_head_p head;

#ifndef NO_LIBX_SAFETY
    if (!length)
        return 0;
#endif 

    head = (dyn_list_head_p) calloc(1, sizeof(dyn_list_head_t));
    
    if (!head)
        return (dyn_list_head_p)0;

    head->hash = XENUS_HASHCODE_LIST_DYN;
    head->item_sizeof = length;

    return head;
}

XENUS_EXPORT error_t dyn_list_entries(dyn_list_head_p head, size_t * out_size)
{
    CHK_LIST;
    *out_size = head->entries;
    return XENUS_OKAY;
}


XENUS_EXPORT error_t dyn_list_append(dyn_list_head_p head, void ** out)
{
    return dyn_list_append_ex(head, out, NULL);
}

XENUS_EXPORT error_t dyn_list_append_ex(dyn_list_head_p head, void ** out, size_t * idx)           // O(1)
{
    CHK_LIST;

    size_t index;

    if (head->entries + 1 > head->item_slots)
    {
        size_t len;
        len = head->item_slots + LISTS_MIN_APPEND_SIZE;

        if (head->buffer)
        {
            void * temp;
            temp = realloc_zero(head->buffer, len * head->item_sizeof);

            if (!temp)
                return XENUS_ERROR_LIST_FAILED_TO_RESIZE_BUFFER;

            head->buffer = temp;
        }
        else
        {
            if (!(head->buffer = calloc(len, head->item_sizeof)))
            {
                return XENUS_ERROR_LIST_FAILED_TO_ALLOC_BUFFER;
            }
        }

        head->item_slots = len;
    }

    index = head->entries++;

    if (out)
        *out = (void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof));
    
    if (idx)
        *idx = index;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_get_by_index(dyn_list_head_p head, size_t index, void ** entry)
{
    CHK_LIST;

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    if (index >= head->entries)
        return XENUS_ERROR_LIST_INDEX_OUT_BOUNDS;

    if (entry)
        *entry = (void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof));
    
    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_get_by_buffer(dyn_list_head_p head, void * entry, size_t * index)
{
    CHK_LIST;

    size_t item_size;
    size_t cnt;
    size_t i;

#ifndef NO_LIBX_SAFETY
    if (!entry)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
#endif 

    cnt = head->entries;
    item_size = head->item_sizeof;

    for (i = 0; i < cnt; i++)
    {
        void * lst = (void *)(((uint8_t *)head->buffer) + (i * item_size));

        if (memcmp(lst, entry, item_size) == 0)
        {
            if (index)
                *index = i;

            return XENUS_STATUS_LISTS_FOUND;
        }
    }

    return XENUS_STATUS_LISTS_NOT_FOUND;
}

XENUS_EXPORT error_t dyn_list_slice(dyn_list_head_p head, size_t index, size_t cnt)
{
    CHK_LIST;

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    if (index + cnt > head->entries)
        //cnt = head->entries - index;
        return XENUS_ERROR_LIST_INDEX_OUT_BOUNDS;

    memmove((void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof)),
            (void *)(((uint8_t *)head->buffer) + ((index + cnt) * head->item_sizeof)),
            ((head->entries - index - cnt) * head->item_sizeof));

    head->entries -= cnt;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_splice(dyn_list_head_p head, size_t index, void ** new_entity)
{
    CHK_LIST;

    void * item;

#ifndef NO_LIBX_SAFETY
    if (!new_entity)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
#endif 

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    if (index >= head->item_slots)
    {
        void * temp;
        size_t add;

        add = index / LISTS_MIN_APPEND_SIZE; 
        add *= LISTS_MIN_APPEND_SIZE;
        add += LISTS_MIN_APPEND_SIZE; // expect floor

        ASSERT(add > index, "array size (entries) should be greater than index");

        temp = realloc(head->buffer, add * head->item_sizeof);

        if (!temp)
            return XENUS_ERROR_LIST_FAILED_TO_RESIZE_BUFFER;

        head->buffer = temp;
        head->item_slots = add;
    }

    item  = (void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof));

    if (head->entries > index)
    {
        memmove((void *)(((uint8_t *)head->buffer) + ((index + 1) * head->item_sizeof)),
            item,
            (head->entries - index) * head->item_sizeof);
    }
    
    head->entries = MAX(head->entries, index + 1);

    *new_entity = item;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_preappend(dyn_list_head_p head, void ** entry)
{
    return dyn_list_splice(head, 0, entry);
}

XENUS_EXPORT error_t dyn_list_remove(dyn_list_head_p head, size_t index)
{
    return dyn_list_slice(head, index, 1);
}

XENUS_EXPORT error_t dyn_list_destory(dyn_list_head_p head)
{
    CHK_LIST;
    
    if (head->buffer)
        free(head->buffer);
    
    free(head);
    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_iterate(dyn_list_head_p head, void(*iterator)(void * buffer, void * ctx), void * ctx)
{
    CHK_LIST;

#ifndef NO_LIBX_SAFETY
    if (!iterator)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
#endif

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    for (size_t i = 0; i < head->entries; i++)
        iterator((void *)(((uint8_t *)head->buffer) + (i * head->item_sizeof)), ctx);

    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_reset(dyn_list_head_p head)
{
    CHK_LIST;

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    head->entries = 0;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_rebuffer(dyn_list_head_p head)
{
    CHK_LIST;

    void * nbuf;
    size_t len;

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    len = head->item_slots + LISTS_MIN_APPEND_SIZE;

    if (!(nbuf = realloc_zero(head->buffer, len)))
        return XENUS_ERROR_OUT_OF_MEMORY;

    head->buffer = nbuf;

    return XENUS_OKAY;
}