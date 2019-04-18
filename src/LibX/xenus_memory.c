/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <kernel/libx/xenus_memory.h> 
#include "../Boot/access_system.h"

static size_t kernel_memusage = 0;
static uint32_t mem_atomic = 0;

#define ALLOCATOR_MAGIC                       XENUS_HASHCODE_ALLOCATOR
#define ALLOCATOR_PADDING                     (sizeof(allocator_header_t) + 16) 
#define ALLOCATOR_GET_LINUX_BUFFER(header)    (void *)(((size_t)header) - header->base_buf_offset)

enum allocation_type_e
{
    XENUS_INVALID_ALLOCATOR = 0,
    XENUS_ALLOCATOR_KMALLOC,
    XENUS_ALLOCATOR_VMALLOC
};

#pragma pack(push, 1)
typedef struct allocator_header_s
{
    uint32_t allocator_magic;
    uint32_t actual_length;
    uint32_t request_length;
    //void * base_buf; - deprecated, use ALLOCATOR_GET_LINUX_BUFFER.
    uint8_t base_buf_offset;
    uint8_t type;
} *allocator_header_p, allocator_header_t;
#pragma pack(pop)


#ifdef NO_LIBX_SAFETY
#define GET_AND_CHK_HEADER_xx(p, error_one, error_two)                               \
    allocator_header_p header;                                                       \
    header = (allocator_header_p)(((size_t)p) - sizeof(allocator_header_t));         
#else
#define GET_AND_CHK_HEADER_xx(p, error_one, error_two)                               \
    allocator_header_p header;                                                       \
                                                                                     \
    if (!p)                                                                          \
        return error_one;                                                            \
                                                                                     \
    header = (allocator_header_p)(((size_t)p) - sizeof(allocator_header_t));         \
                                                                                     \
    if (header->allocator_magic != ALLOCATOR_MAGIC)                                  \
        return error_two;
#endif

static void * _realloc(void * buf, size_t length, bool zero);

static void _linux_free(void * buffer, int type)
{
    switch (type)
    {
    case XENUS_ALLOCATOR_KMALLOC:
        kfree(buffer);
        break;
    case XENUS_ALLOCATOR_VMALLOC:
        vfree(buffer);
        break;
    case 0:
    default:
        panic("_linux_free called with an illegal type!");
    }
}

static inline bool _atomic_alloc(bool atomic)
{
    return atomic || mem_atomic;
}

static bool _linux_alloc(bool atomic, size_t length, void ** out, size_t * actual_length, int * type)
{
    void * buffer;

    atomic = _atomic_alloc(atomic);

    if ((length >= 4096) && (!atomic))
        goto alt_alloc;

    buffer = kmalloc(length, (atomic ? GFP_ATOMIC : 0) | GFP_KERNEL);

    if (buffer)
    {
        *actual_length = ksize(buffer);
        *type = XENUS_ALLOCATOR_KMALLOC;
        goto out;
    }
    
alt_alloc:
 
    if (atomic)
    {
        *out = NULL;
        *type = 0;
        *actual_length = 0;
        return false;
    }

    buffer = vmalloc(length);
    *actual_length = length;
    *type = XENUS_ALLOCATOR_VMALLOC;

out:
    *out = buffer;
    return buffer != 0;
}

XENUS_EXPORT size_t alloc_get_kernel_usage()
{
    return kernel_memusage;
}

XENUS_EXPORT void * malloc_common(size_t length, bool atomic)
{
    int type;
    void * buf;
    size_t act_len;
    void * user_buf;
    allocator_header_p header;

    if (!_linux_alloc(atomic, length + ALLOCATOR_PADDING, &buf, &act_len, &type))
        return NULL;

    user_buf    = (void *)((((size_t)buf) + ALLOCATOR_PADDING - 1) & ~(16 - 1));          // find allocated buffer after padding and the header that we want to prepend
    header      = (allocator_header_p)(((size_t)user_buf) - sizeof(allocator_header_t));  // subtract header size from the allocated buffer ptr 


#ifndef NO_LIBX_SAFETY
    if ((((size_t)user_buf) + length) > (((size_t)buf) + act_len))
        panic("Malloc Math Error: integer overflow issue - out of bounds! \n");

    if (((size_t)header) < ((size_t)buf))
        panic("Malloc Math Error: integer underflow/overflow issue in header calculation! \n");
#endif

    header->base_buf_offset  = (uint8_t)(((size_t)header) - ((size_t)buf));
    header->actual_length    = act_len;
    header->request_length   = length;
    header->type             = type;
    header->allocator_magic  = ALLOCATOR_MAGIC;

    kernel_memusage += act_len;

    return user_buf;
}

XENUS_EXPORT void * malloc_atomic(size_t length)
{
    return malloc_common(length, true);
}

XENUS_EXPORT void * malloc(size_t length)
{
    return malloc_common(length, false);
}

XENUS_EXPORT void alloc_increment_atomicity()
{
    mem_atomic++;
}

XENUS_EXPORT void alloc_decrement_atomicity()
{
    ASSERT(mem_atomic != 0, "can not decrement kernel mem atomic atomicity - counter is zero");
    mem_atomic--;
}

XENUS_EXPORT uint32_t alloc_get_atomicity()
{
    return mem_atomic;
}

XENUS_EXPORT void free(void * ptr)
{
    GET_AND_CHK_HEADER_xx(ptr,,)

    kernel_memusage -= header->actual_length;
    _linux_free(ALLOCATOR_GET_LINUX_BUFFER(header), header->type);
}

XENUS_EXPORT void alloc_dbg_info(void * ptr)
{
    void * base_buffer;
    GET_AND_CHK_HEADER_xx(ptr,,)

    base_buffer = ALLOCATOR_GET_LINUX_BUFFER(header);

    printf("------------ BUFFER INFO ------------ \n"
           " Header                      : 0x%p   \n"
           " Base Buffer                 : 0x%p   \n"
           " Base Virt (usr) Buffer      : 0x%p   \n"
           " Type                        : %zu     \n"
           " Actual Length               : %zu    \n"
           " Requested Length            : %zu    \n"
           " Post Virt Buffer Padding    : %zu    \n"
           " Pre header Padding          : %zu    \n"
           " Header Length               : %zu    \n"
           "------------ BUFFER INFO ------------ \n",
                header,
                base_buffer,
                ptr,
                (uint64_t)header->type,
                header->actual_length,
                header->request_length,
                header->actual_length - ALLOCATOR_PADDING - header->request_length,
                ((size_t)ptr) - ((size_t)base_buffer) - sizeof(allocator_header_t),
                sizeof(allocator_header_t));
}

XENUS_EXPORT size_t alloc_actual_size(void * ptr)
{
    GET_AND_CHK_HEADER_xx(ptr, 0, 0)
    return header->actual_length;
}

XENUS_EXPORT size_t alloc_requested_size(void * ptr)
{
    GET_AND_CHK_HEADER_xx(ptr, 0, 0)
    return header->request_length;
}

XENUS_EXPORT void * calloc(size_t num, size_t size)
{
    return zalloc(num * size);
}

XENUS_EXPORT void * calloc_nozero(size_t num, size_t size)
{
    return malloc(num * size);
}

XENUS_EXPORT void * zalloc(size_t size)
{
    void * ret;
    
    ret = malloc(size);
    
    if (!ret)
        return NULL;
    
    return memset(ret, 0, size);
}

XENUS_EXPORT void * realloc(void * buf, size_t length)
{
    return _realloc(buf, length, false);
}

XENUS_EXPORT void * realloc_zero(void * buf, size_t length)
{
    return _realloc(buf, length, true);
}

static void * _realloc(void * buf, size_t length, bool zero)
{
    size_t oldLength;
    void * new;

    if (!buf)
        return malloc(length);

    GET_AND_CHK_HEADER_xx(buf, NULL, NULL)

    if (!(oldLength = header->request_length))
    {
        free(buf);
        return malloc(length);
    }

    new = malloc(length);

    if (!new)
        return 0;

    memcpy(new, buf, oldLength);

    if (zero)
        memset((void *)((size_t)new + oldLength), 0, length - oldLength);

    free(buf);

    return new;
}