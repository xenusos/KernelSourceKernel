/*
    Purpose: tracking of dynamic kernel exports
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/

#include <xenus.h>			// int types, common types, etc
#include "core_exports.h"	// pre-decleration
#include "PE/pe_headers.h"	// PE types
#include "access_sys.h"		// deps
#include <libx/xenus_list_dyn.h> // deps - for new 
//#include <libx/xenus_memory.h> // deps -- for deprecated
typedef struct
{
    const char * name;
    uint32_t ord;
    void * ptr;
    //void * next;
} *export_entry_p, export_entry_t;


static dyn_list_head_p exports;

void exports_add_ex(const char * lbl, int ord, void * function)
{
	export_entry_p entry;

	//TODO 2: ord = index

	if (ERROR(dyn_list_append(exports, &entry)))
		panicf("Failed to add export entry: %s\n", lbl);

	entry->name = lbl;
	entry->ptr = function;
	entry->ord = ord;
}

void exports_add(const char * lbl, void * function)
{
	exports_add_ex(lbl, -1, function);
}

void *  exports_get_by_name(const char * lbl)
{
	size_t entries;

	if (ERROR(dyn_list_entries(exports, &entries)))
		return 0;
	
	for (size_t i = 0; i < entries; i++)
	{
		export_entry_p entry;

		if (ERROR(dyn_list_get_by_index(exports, i, &entry)))
			return 0;

		if (strcmp(entry->name, lbl) == 0) return entry->ptr;
	}

	return 0;
}

void *  exports_get_by_ord(int ord)
{
	size_t entries;

	//TODO 2: ord = index

	if (ERROR(dyn_list_entries(exports, &entries)))
		return 0;
	
	for (size_t i = 0; i < entries; i++)
	{
		export_entry_p entry;

		if (ERROR(dyn_list_get_by_index(exports, i, &entry)))
			return 0;

		if (entry->ord == ord) return entry->ptr;
	}

	return 0;
}

void init_exports(void * kernel_base)
{
	size_t entries;
	PDOS_HEADER dheader;
	PIMAGE_NT_HEADERS ntheader;
	PIMAGE_EXPORT_DIRECTORY pe_exports;
	uint32_t * names;
	uint32_t * functions;
	uint16_t * ords;

	exports = DYN_LIST_CREATE(export_entry_t);

	if (!exports)
		panic("Failed to initalise xenus export list!\n");

	dheader = (PDOS_HEADER)kernel_base;
	ntheader = (PIMAGE_NT_HEADERS)((uint64_t)kernel_base + dheader->e_lfanew);

	if (ntheader->OptionalHeader64.NumberOfRvaAndSizes < 2)return;

	if (!(ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)) return;
	
	pe_exports = (PIMAGE_EXPORT_DIRECTORY)((uint64_t)kernel_base + ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	names		= (uint32_t *)(((uint64_t)(kernel_base)) + pe_exports->AddressOfNames);
	functions	= (uint32_t *)(((uint64_t)(kernel_base)) + pe_exports->AddressOfFunctions);
	ords		= (uint16_t *)(((uint64_t)(kernel_base)) + pe_exports->AddressOfNameOrdinals);


	for (size_t i = 0; i < pe_exports->NumberOfFunctions; i++)
	{
		void * function;
		uint16_t ord;

		ord = ords[i];
		function = (void *)(((uint64_t)(kernel_base)) + functions[ord]);

		exports_add_ex("NULL", ord, function);
	}

	if (ERROR(dyn_list_entries(exports, &entries)))
		panic("List error while creating exports (a: failed to get amount of entries) \n");

	if (entries != pe_exports->NumberOfFunctions)
		panic("List error while creating exports (b: amount of entries doesn't equal the amount of pe exports) \n");
	
	for (size_t i = 0; i < pe_exports->NumberOfNames; i++)
	{
		export_entry_p entry;
		const char * name;
		uint16_t ord;
		bool found;

		name = (char *)(((uint64_t)(kernel_base)) + names[i]);
		ord = ords[i];
		found = false;

		if (ERROR(dyn_list_get_by_index(exports, 0, &entry)))
			panic("List error while creating exports (c: failed to get first entry/failed to get the buffer) \n");

		for (size_t z = 0; z < entries; z++)
		{
			if (entry->ord == ord)
			{
				found = true;
				entry->name = name;
				break;
			}

			entry++;
		}

		if (!found)
			printf("warning: couldn't find ordinal entry for %s of %i    \n" 
				   "Why is NumberOfNames greater than NumberOfAddresses? \n", name, ord);
	}

	exports_print();
}

#include <libx/xenus_memory.h>
#include <_/_xenus_linux_types_fio.h>

void exports_print()
{
	export_entry_p entry;
	size_t entries;
	size_t length;
	char file_buffer[1024];
	void * handle;

#ifndef OLD_LEGACY_DUMP_SYMBOL_DEF
	return;
#endif

	if (ERROR(dyn_list_entries(exports, &entries)))
		panic("failed to get index!");

	if (ERROR(dyn_list_get_by_index(exports, 0, &entry)))
		panic("List error while creating exports (c: failed to get first entry/failed to get the buffer) \n");

	handle = file_open("/Xenus/Dynamic.def", O_RDWR | O_CREAT | O_APPEND, 0777);

	if (!handle)
	{
		print("Failed to load static definition file.\n");
		return;
	}

#define APPEND_FORMAT(fmt, ...) 								     			\
		memset(file_buffer, 0, 1024);											\
		length = string_format(file_buffer, 1024, fmt, __VA_ARGS__);  			\
        file_write(handle, 0, file_buffer, length);

	APPEND_FORMAT("LIBRARY Xenus\r\n");
	APPEND_FORMAT("EXPORTS\r\n");

	for (size_t z = 0; z < entries; z++)
	{
		APPEND_FORMAT("    %s\r\n", entry->name);

		entry++;
	}

	file_close(handle);

#undef APPEND_FORMAT
}

//static export_entry_p entry_start = 0;
//static export_entry_p cur_entry = 0;
//
//void exports_add_ex(const char * lbl, int ord, void * function)
//{
//    export_entry_p entry;
//
//    entry = (export_entry_p) malloc(sizeof(export_entry_t));
//
//    if (!entry)
//        panicf("Failed to allocate %s kernel export entry\n", lbl);
//
//    entry->name = lbl;
//    entry->ptr = function;
//    entry->next = 0;
//    entry->ord = ord;
//
//    if (cur_entry)
//        cur_entry->next = (void *)entry;
//    else
//        entry_start = (export_entry_p)entry;
//
//    cur_entry = (export_entry_p)entry;
//}
//
//void exports_add(const char * lbl, void * function)
//{
//    exports_add_ex(lbl, -1, function);
//}
//
//void * exports_get_by_name(const char * lbl)
//{
//    export_entry_p entry;
//    int(*strcmp)(const char * x, const char * y);
//
//    entry = entry_start;
//    strcmp = g_functions->string.strcmp;
//
//    if (!entry) return 0;
//
//    do
//    {
//        if (strcmp(entry->name, lbl) == 0)
//            return entry->ptr;
//
//    } while (entry = entry->next);
// 
//    return 0;
//}
//
//void * exports_get_by_ord(int ord)
//{
//    export_entry_p entry;
//
//    if (!(entry = entry_start)) return 0;
//
//    do
//    {
//        if (ord == entry->ord)
//            return entry->ptr;
//
//    } while (entry = entry->next);
//
//    return 0;
//}
//
//void init_exports(void * kernel_base)
//{
//    PDOS_HEADER dheader;
//    PIMAGE_NT_HEADERS ntheader;
//
//    dheader = (PDOS_HEADER)kernel_base;
//    ntheader = (PIMAGE_NT_HEADERS)((uint64_t)kernel_base + dheader->e_lfanew);
//
//	//TODO: actually check that imports/exports are present
//	//TODO: we used to check that the kernel is a valid pe
//	//TODO: but that was scapped because once is a pe is loaded it's memory footprint isn't equal to the original pe (ie segment offsets)
//	//TODO: perhaps implement something in pe loader that uses VirtualAddress over PointerToData?
//	//TODO: we could still run the same checks, but is that really needed?
//	//TODO: can't we just assume that the xenus image isn't tainted if it passes secchk?
//
//    if (ntheader->OptionalHeader64.NumberOfRvaAndSizes < 2) 
//        return;
//   
//    if (ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
//    {
//		size_t i;
//        PIMAGE_EXPORT_DIRECTORY exports;
//
//        exports = (PIMAGE_EXPORT_DIRECTORY)((uint64_t)kernel_base + ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
//
//        uint32_t * names;
//        uint32_t * functions;
//        uint16_t * ords;
//
//        names = (uint32_t *)((uint64_t)(kernel_base)+exports->AddressOfNames);
//        functions = (uint32_t *)((uint64_t)(kernel_base)+exports->AddressOfFunctions);
//        ords = (uint16_t *)((uint64_t)(kernel_base)+exports->AddressOfNameOrdinals);
//
//
//		for (i = 0; i < exports->NumberOfFunctions; i++)
//		{
//			void * function;
//			uint16_t ord;
//
//			ord = ords[i];
//			function = (void *)(((uint64_t)(kernel_base)) + functions[ord]);
//
//			exports_add_ex("NULL", ord, function);
//		}
//
//        for (i = 0; i < exports->NumberOfNames; i++)
//        {
//			export_entry_p entry;
//            const char * name;
//            void * function;
//            uint16_t ord;
//
//			name = (char *)((uint64_t)(kernel_base)+names[i]);
//			ord = ords[i];
//            function = (void *)(((uint64_t)(kernel_base)) + functions[ord]);
//
//			entry = entry_start;
//			do
//			{
//				if (!entry) break;
//				if (entry->ord == ord)
//					entry->name = name;
//			} while (entry = entry->next);
//        }
//    }
//}
//
//void exports_print()
//{
//    export_entry_p entry;
//	size_t count;
//	
//	count = 0;
//    printf("[DBG] Printing xenus kernel exports\n");
//    if (!(entry = entry_start)) return;
//    do
//    {
//        count++;
//        printf("[DBG] Export: %p - %i (%s)\n", entry->ptr, entry->ord, entry->name ? entry->name : "NULL");
//    } while (entry = entry->next);
//    printf("[DBG] Completed dump of %i functions.\n", count);
//}