/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <libx/xenus_dynbuffer.h>
#include <libx/xenus_memory.h> 
#include "../access_sys.h"

#define DYN_BUFFER_MAGIC 0xAD431204

XENUS_EXPORT error_t buffer_alloc(uint_t length, dynbuffer_p* outdynbuffer)
{
    dynbuffer_p buffer = 0;
    void * data = 0;

    if (!length)
        return XENUS_ERROR_ILLEGAL_SIZE;

    if (!outdynbuffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    buffer = (dynbuffer_p) malloc(sizeof(dynbuffer_t));

    if (!buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    data = malloc(length);

    if (!data)
    {
        free(buffer);
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    buffer->magic = DYN_BUFFER_MAGIC;
    buffer->length = length;
    buffer->buffer = data;

    *outdynbuffer = buffer;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_free(dynbuffer_p buffer)
{
    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->buffer)
        free(buffer->buffer);

    free(buffer);

    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_resize(dynbuffer_p buffer, uint_t length)
{
    void * data;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    
    if (!length)
        return XENUS_ERROR_ILLEGAL_SIZE;

    if (buffer->buffer)
    {
        if (!(data = realloc(buffer->buffer, length)))
        {
            buffer->buffer = 0;
            buffer->length = 0;
            return XENUS_ERROR_OUT_OF_MEMORY;
        }
    } 
    else
    {
        if (!(data = malloc(length)))
            return XENUS_ERROR_OUT_OF_MEMORY;
    }

    buffer->buffer = data;
    buffer->length = length;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_shrink(dynbuffer_p buffer, uint_t subtract)
{
    uint_t new_size;
    uint_t size;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    if (!subtract)
        return XENUS_OKAY;

    size = buffer->length;
    new_size = size - subtract;

    //TODO: REAL OVERFLOW DETECTION
    //if (new_size > size)
    //    return XENUS_ERROR_ILLEGAL_SIZE_OVERUNDERFLOW;

    if (!(buffer->buffer = realloc(buffer->buffer, new_size)))
    {
        buffer->length = 0;
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    buffer->length = new_size;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_expand(dynbuffer_p buffer, uint_t add)
{
    uint_t new_size;
    uint_t size;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    if (!add)
        return XENUS_OKAY;

    size = buffer->length;
    new_size = size + add;

    //TODO: REAL OVERFLOW DETECTION
    //if (new_size < size)
    //    return XENUS_ERROR_ILLEGAL_SIZE_OVERUNDERFLOW;

    if (!(buffer->buffer = realloc(buffer->buffer, new_size)))
    {
        buffer->length = 0;
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    buffer->length = new_size;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_memcpy(dynbuffer_p buffer, uint_t index, void * src, uint_t length)
{
    void * dest;

    if (!length)
        return XENUS_ERROR_ILLEGAL_SIZE;

    if (!src)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    if (length + index > buffer->length)
        return XENUS_ERROR_ILLEGAL_SIZE;

    dest = (void *)((uint_t)buffer->buffer + index);

    if (memcpy(dest, src, length) != dest)
        return XENUS_ERROR_GENERIC_FAILURE;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_memset(dynbuffer_p buffer, uint_t index, uint8_t c, uint_t length)
{
    void * dest;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    if (!length && !index)
        length = buffer->length;
    
    if (!length)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (index + length > buffer->length)
        return XENUS_ERROR_ILLEGAL_SIZE;
    
    dest = (void *)((uint_t)buffer->buffer + index);

    if (memset(dest, c, length) != dest)
        return XENUS_ERROR_GENERIC_FAILURE;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_fill(dynbuffer_p buffer, uint8_t c)
{
    return buffer_memset(buffer, 0, c, 0);
}

XENUS_EXPORT error_t buffer_memcmp(dynbuffer_p buffer, uint_t index, void * src, uint_t length)
{
    return XENUS_ERROR_NOT_IMEPLEMENTED;
}

XENUS_EXPORT error_t buffer_slice(dynbuffer_p buffer, uint_t start, uint_t end)
{
    return XENUS_ERROR_NOT_IMEPLEMENTED;
}

XENUS_EXPORT error_t buffer_copy(dynbuffer_p buffer, uint_t index, dynbuffer_p dynsrc, uint_t srcindx, uint_t length)
{
    return XENUS_ERROR_NOT_IMEPLEMENTED;
}

XENUS_EXPORT error_t buffer_compare(dynbuffer_p buffer, uint_t index, dynbuffer_p dynsrc, uint_t srcindx, uint_t length)
{
    return XENUS_ERROR_NOT_IMEPLEMENTED;
}

XENUS_EXPORT error_t buffer_get(dynbuffer_p buffer, uint_t index, void * outbuffer, uint_t length)
{
    void * src;

    if (!length)
        return XENUS_ERROR_ILLEGAL_SIZE;

    if (!outbuffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    if (length + index > buffer->length)
        return XENUS_ERROR_ILLEGAL_SIZE;

    src = (void *)((uint_t)buffer->buffer + index);

    if (memcpy(outbuffer, src, length) != outbuffer)
        return XENUS_ERROR_GENERIC_FAILURE;

    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_length(dynbuffer_p buffer, int_t * length)
{
    if (!length)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    *length = buffer->length;
    return XENUS_OKAY;
}

XENUS_EXPORT error_t buffer_unsafe_get(dynbuffer_p buffer, uint8_t ** out)
{
    if (!out)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (buffer->magic != DYN_BUFFER_MAGIC)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer->buffer)
        return XENUS_ERROR_OUT_OF_MEMORY;

    *out = (uint8_t *)buffer->buffer;

    return XENUS_OKAY;
}