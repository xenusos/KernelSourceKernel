/*
    Purpose: Dynamic list
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/

#include <xenus.h>
#include <libx/xenus_list_dyn.h>
#include <libx/xenus_memory.h> 
#include "../access_sys.h"

#define DYN_LIST_HASH 0x45018413

XENUS_EXPORT dyn_list_head_p _dyn_list_create(size_t length)
{
    dyn_list_head_p head;

    head = (dyn_list_head_p) malloc(sizeof(dyn_list_head_t));
    
    if (!head)
        return (dyn_list_head_p)0;

    memset(head, 0, sizeof(dyn_list_head_t));

	head->hash = DYN_LIST_HASH;
    head->item_sizeof = length;
    return head;
}

XENUS_EXPORT error_t dyn_list_entries(dyn_list_head_p head, size_t * out_size)
{
    if (!head)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    *out_size = head->entries;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_append(dyn_list_head_p head, void ** out)
{
	if (!head)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (head->entries + 1 > head->item_slots)
    {
        size_t len;
        len = head->item_slots + LISTS_MIN_APPEND_SIZE;

        if (head->buffer)
        {
            void * temp;
            temp = realloc(head->buffer, len * head->item_sizeof);

            if (!temp)
                return XENUS_ERROR_LIST_FAILED_TO_RESIZE_BUFFER;

            head->buffer = temp;
        }
        else
            if (!(head->buffer = malloc(len * head->item_sizeof)))
                return XENUS_ERROR_LIST_FAILED_TO_ALLOC_BUFFER;

        head->item_slots = len;
    }

    head->entries++;

    *out = (void *)(((uint8_t *)head->buffer) + ((head->entries - 1) * head->item_sizeof));

    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_get_by_index(dyn_list_head_p head, size_t index, void ** entry)
{
	if (!head)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (!head->buffer)
		return XENUS_ERROR_LIST_NO_BUFFER;

	if (index > head->entries)
		return XENUS_ERROR_LIST_INDEX_OUT_BOUNDS;

	*entry = (void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof));
	return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_get_by_buffer(dyn_list_head_p head, void * entry, size_t * index)
{
	size_t item_size;
	size_t cnt;
	size_t i;

	if (!head)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (!head->buffer)
		return XENUS_ERROR_LIST_NO_BUFFER;

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
    if (!head)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    if (index + cnt > head->entries)
        //cnt = head->entries - index;
        return false;

    memmove((void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof)),
            (void *)(((uint8_t *)head->buffer) + ((index + cnt) * head->item_sizeof)),
            ((head->entries - index - cnt) * head->item_sizeof));

    head->entries -= cnt;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t dyn_list_splice(dyn_list_head_p head, size_t index, void ** new_entity)
{
    void * item;

    if (!head)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!head->buffer)
        return XENUS_ERROR_LIST_NO_BUFFER;

    if (index >= head->item_slots)
        return 0;

    if (head->entries + 1 > head->item_slots)
    {
        void * temp;

        temp = realloc(head->buffer, (head->item_slots + LISTS_MIN_APPEND_SIZE) * head->item_sizeof);

        if (!temp)
            return XENUS_ERROR_LIST_FAILED_TO_RESIZE_BUFFER;

        head->buffer = temp;
        head->item_slots += LISTS_MIN_APPEND_SIZE;
    }

    item  = (void *)(((uint8_t *)head->buffer) + (index * head->item_sizeof));

    memmove((void *)(((uint8_t *)head->buffer) + ((index + 1) * head->item_sizeof)),
            item,
            (head->entries - index) * head->item_sizeof);
    
    head->entries++;

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
    if (!head)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (head->hash != DYN_LIST_HASH)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    
    if (head->buffer)
        free(head->buffer);
    
    free(head);
    return XENUS_OKAY;
}