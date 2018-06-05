/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <libx/xenus_memory.h>
#include "../core_exports.h"
#include <libx/xenus_memory.h> 
#include <_/_xenus_linux_types_memory.h>
#include "../access_sys.h"

#define ALLOCATOR_MAGIC 0x33221155

#pragma pack(push, 1)
typedef struct allocator_header_s
{
	uint32_t allocator_magic;
	size_t actual_length;
	size_t request_length;
	//void * base_buf;
	uint8_t base_buf_offset;
	uint8_t type;
} *allocator_header_p, allocator_header_t;
#pragma pack(pop)

#define ALLOCATOR_PADDING (16 + sizeof(allocator_header_t)) 

XENUS_EXPORT void * malloc(size_t length)
{

	size_t buf_len;             // length with header and padding space 
	size_t act_len;             // vm/kalloc padded "buf_len" length (typically is equal to buf_len + padding = aligned with page size) 
	void * buf;                 // real virtual address and allocated length
	int type;		            // allocator type (vmalloc or kalloc)
	void * user_buf;            // 16-byte aligned address that is provided to the caller (user_buf - sizeof(header) = header)
	allocator_header_p header;  // header

	buf_len = length + ALLOCATOR_PADDING;

	if (buf_len >= 4096) goto alt_alloc; // "kmalloc is the normal method of allocating memory for objects smaller than page size in the kernel. "
										 // if we're requesting a page or more, let's use virtually contiguous memory, rather than taking up all the physically contiguous memory at the cost of the iommu remapping performance hit.
	buf = kmalloc(buf_len, GFP_KERNEL);

	if (buf)
	{
		act_len = ksize(buf);
		type = 1;
		goto succ;
	}

	alt_alloc:

	buf = vmalloc(buf_len);
	act_len = buf_len; //TODO: this is slow. why would you even realloc a vmalloc'd buffer? didn't you get enough the first time? ffs
	type = 2;

	if (!buf)
		return 0;

succ:
	{
		user_buf = (void *)((((size_t)buf) + ALLOCATOR_PADDING - 1) & ~(16 - 1));		// find allocated buffer after padding and the header that we want to prepend
		header = (allocator_header_p)(((size_t)user_buf) - sizeof(allocator_header_t));	// subtract header size from the allocated buffer ptr 

		//header->base_buf = buf;
		header->base_buf_offset = (uint8_t)(((size_t)header) - ((size_t)buf));
		header->actual_length = act_len;
		header->request_length = length;
		header->type = type;
		header->allocator_magic = ALLOCATOR_MAGIC;

		return user_buf;
	}
}

XENUS_EXPORT void free(void * s)
{
	allocator_header_p header;
	void * base_buffer;

	if (!s) return;

	header = (allocator_header_p)(((size_t)s) - sizeof(allocator_header_t));

	if (header->allocator_magic != ALLOCATOR_MAGIC)
		panicf("free called with an illegal header magic!");

	base_buffer = (void *)(((size_t)header) - header->base_buf_offset);

	switch (header->type)
	{
	case 1:
		kfree(base_buffer);
		break;
	case 2:
		vfree(base_buffer);
		break;
	case 0:
	default:
		panicf("free called with an illegal header!");
	}

	return;
}

XENUS_EXPORT void alloc_dbg_info(void * s)
{
	allocator_header_p header;
	void * base_buffer;

	if (!s)
	{
		printf("Illegal buffer (null) provided to alloc_dbg_info\n");
		return;
	}

	header = ((allocator_header_p)(((size_t)s) - sizeof(allocator_header_t)));

	if (header->allocator_magic != ALLOCATOR_MAGIC)
	{
		printf("Illegal buffer (magic) provided to alloc_dbg_info\n");
		return;
	}

	base_buffer = (void *)(((size_t)header) - header->base_buf_offset);

	printf("------------ BUFFER INFO ------------ \n"
		" Header                      : 0x%p   \n"
		" Base Buffer                 : 0x%p   \n"
		" Base Virt (usr) Buffer      : 0x%p   \n"
		" Type                        : %c    \n"
		" Actual Length               : %zi    \n"
		" Requested Length            : %zi    \n"
		" Post Virt Buffer Padding    : %zi    \n"
		" Pre header Padding          : %zi    \n"
		" Header Length               : %zi    \n"
		"------------ BUFFER INFO ------------ \n",
			header,
			base_buffer,
			s,
			(uint64_t)header->type,
			header->actual_length,
			header->request_length,
			header->actual_length - ALLOCATOR_PADDING - header->request_length,
			((size_t)s) - ((size_t)base_buffer) - sizeof(allocator_header_t),
			sizeof(allocator_header_t));
}


XENUS_EXPORT size_t alloc_actual_size(void * s)
{
	allocator_header_p header;
	if (!s) return 0;
	header = (allocator_header_p)(((size_t)s) - sizeof(allocator_header_t));
	if (header->allocator_magic != ALLOCATOR_MAGIC) return 0;
	return header->actual_length;
}

XENUS_EXPORT size_t alloc_requested_size(void * s)
{
	allocator_header_p header;
	if (!s) return 0;
	header = (allocator_header_p)(((size_t)s) - sizeof(allocator_header_t));
	if (header->allocator_magic != ALLOCATOR_MAGIC) return 0;
	return header->request_length;
}

XENUS_EXPORT void * calloc(size_t num, size_t size)
{
	return malloc(num * size);
}

XENUS_EXPORT void * realloc(void * buf, size_t length)
{
	size_t size;
	void * new;

	if (!buf)
		return malloc(length);

	if (!(size = alloc_requested_size(buf)))
		return malloc(length);

	new = malloc(length);

	if (!new)
		return 0;

	memcpy(new, buf, size);
	free(buf);

	return new;
}
