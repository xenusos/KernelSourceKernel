/*
    Purpose: Linked List
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/

#include <xenus.h>
#include <kernel/libx/xenus_list_linked.h>
#include <kernel/libx/xenus_memory.h> 
#include "../Boot/access_system.h"

XENUS_EXPORT linked_list_entry_p linked_list_append(linked_list_head_p head, size_t buf_len)  
{
    linked_list_entry_p entry;

    if (!head)
        goto error;

    if (head->dead)
        goto error;

    entry = (linked_list_entry_p) malloc(sizeof(linked_list_entry_t) + buf_len);

    if (!entry)
        goto error;

    entry->next              = 0;
    entry->before            = head->top;
    entry->head              = head;
    entry->end_of_struct     = (void*)((uint8_t*)entry + sizeof(linked_list_entry_t));
    head->length++;
    
    if (!head->bottom)
        head->bottom = entry;

    if (head->top)
        head->top->next = entry;

    head->top = entry;
    return entry;
error:
    return (linked_list_entry_p)0;
}

XENUS_EXPORT error_t linked_list_remove(linked_list_entry_p entry) // O(1)
{
    if (!entry) return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    if (entry->head->top == entry && entry->head->bottom == entry)
    {
        // Change of plan, if has callback hander, deactivate obj, otherwise reset. //
        if (entry->head->cb_died)
        {
            entry->head->dead = true;
            entry->head->cb_died((struct linked_list_head_s *)entry->head);
        }
        else
            entry->head->top = entry->head->bottom = 0;
    }
    else if (entry->head->top == entry)
    {
        /* nuke entry and remove top*/
        entry->head->top = entry->before;
    }
    else if (entry->head->bottom == entry)
    {
        /* nuke entry and remove bottom*/
        entry->head->bottom = entry->next;
    }

    if (entry->before)
        entry->before->next = entry->next;

    if (entry->next)
        entry->next->before = entry->before;

    entry->head->length--;

    free(entry);
    return XENUS_OKAY;
}

XENUS_EXPORT linked_list_head_p linked_list_create()
{
    linked_list_head_p head;
    head = (linked_list_head_p) malloc(sizeof(linked_list_head_t));
    memset(head, 0, sizeof(linked_list_head_t)); 
    return head;
}

XENUS_EXPORT error_t linked_list_destory(linked_list_head_p head)
{
    linked_list_entry_p bottom;

    if (!head) return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    while (bottom = head->bottom)
        linked_list_remove(bottom);

    free(head);
    return XENUS_OKAY;
}