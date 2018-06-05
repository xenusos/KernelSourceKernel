/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>

#include "pe_headers.h"
#include "pe_checker.h"

#include "../access_sys.h"

error_t pe_loader_check_dos(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader)
{
    if ((dheader->signature[0] == 'M' || dheader->signature[1] == 'Z') ||
        (dheader->signature[0] == 'x' || dheader->signature[1] == 'D'))
        return XENUS_OKAY;
    return XENUS_ERROR_PE_BAD_DOS;
}

error_t pe_loader_check_pe(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader)
{
    if (ntheader->Signature[0][0] != 'P' ||
        ntheader->Signature[0][1] != 'E')
        return XENUS_ERROR_PE_BAD_PE;
    return XENUS_OKAY;
}

error_t pe_loader_virt_to_offset(PIMAGE_NT_HEADERS ntheader, uint64_t virt, uint64_t * offset)
{
    PIMAGE_SECTION_HEADER secs;
    size_t i;

    secs = (PIMAGE_SECTION_HEADER)((uint64_t)&ntheader->OptionalHeader64 + ntheader->FileHeader.SizeOfOptionalHeader);

    for (i = 0; i < ntheader->FileHeader.NumberOfSections; i++)
    {
        size_t start;
        size_t end;

        start = secs[i].VirtualAddress;
        end = secs[i].VirtualAddress + secs[i].SizeOfRawData;

        if (virt < end && virt >= start)
        {
            *offset = (virt - start) + secs[i].PointerToRawData;
            return XENUS_OKAY;
        }
    }
    return XENUS_ERROR_PE_FAILED_TO_RESOLVE_ADDRESS;
}

error_t pe_loader_virt_to_address(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, uint64_t virt, uint64_t * address)
{
    error_t ret;
    size_t offset;
	
    if ((ret = pe_loader_virt_to_offset(ntheader, virt, &offset)) != XENUS_OKAY)
        return ret;
	
    if (!address)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
	
    *address = (size_t)dheader + offset;
	
    return XENUS_OKAY;
}

error_t pe_loader_check_datadir(PIMAGE_NT_HEADERS ntheader, uint32_t index, size_t size, size_t file_len)
{
    error_t ret;
    uint64_t offset;
    IMAGE_DATA_DIRECTORY dir;

    if (ntheader->OptionalHeader64.NumberOfRvaAndSizes < index + 1)
        return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;

    dir = ntheader->OptionalHeader64.DataDirectory[index];

    if ((!dir.Size) && (!dir.VirtualAddress))
        return XENUS_STATUS_OKAY_PE_DATA_DIR_NOT_PRESENT; 

    //Check size of struct
    if (size > dir.Size)
        return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;

    //Check that the virtual address is within the image
    if ((ret = pe_loader_virt_to_offset(ntheader, dir.VirtualAddress, &offset)) != XENUS_OKAY)
        return ret;
    
    if (offset + dir.Size >= file_len)
        return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;
    
    return XENUS_OKAY;
}

error_t pe_loader_check_import_names(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, PIMAGE_IMPORT_DESCRIPTOR imports, size_t file_len)
{    
    return XENUS_OKAY;
}

error_t pe_loader_check_imports(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, size_t file_len)
{
    error_t ret;
    PIMAGE_IMPORT_DESCRIPTOR imports;

    if ((ret = pe_loader_virt_to_address(dheader, ntheader, ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, (uint64_t *)&imports)) != XENUS_OKAY)
        return ret;

    return pe_loader_check_import_names(dheader, ntheader, imports, file_len);
}

error_t pe_loader_check_exports_names(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, PIMAGE_EXPORT_DIRECTORY exports, size_t file_len)
{
    error_t ret;
    uint32_t * address_names;

    if ((ret = pe_loader_virt_to_address(dheader, ntheader, exports->AddressOfNames, (uint64_t *)&address_names)) != XENUS_OKAY)
        return ret;

    for (int i = 0; i < exports->NumberOfNames; i++)
    {
        char * address_name;

        if ((ret = pe_loader_virt_to_address(dheader, ntheader, address_names[i], (uint64_t *)&address_name)) != XENUS_OKAY)
            return ret;

        //TODO check name length - should be within PIMAGE_EXPORT_DIRECTORY.Size
        for (int i = 0; i < 100; i++)
            if (address_name[i] == 0x00) break;
            else if (i == 100)
                return XENUS_ERROR_PE_BAD_EXPORTS;
    }

    return XENUS_OKAY;
}

error_t pe_loader_check_exports(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, size_t file_len)
{
    error_t ret;
    PIMAGE_EXPORT_DIRECTORY exports;

    if ((ret = pe_loader_virt_to_address(dheader, ntheader, ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, (uint64_t *)&exports)) != XENUS_OKAY)
        return ret;

    return pe_loader_check_exports_names(dheader, ntheader, exports, file_len);
}

error_t pe_loader_check_iat(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, size_t file_len)
{
    return XENUS_OKAY;
}

error_t pe_loader_check_nt(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, size_t file_len)
{
    if ((ntheader->FileHeader.SizeOfOptionalHeader) < (sizeof(IMAGE_OPTIONAL_HEADER64) - (sizeof(IMAGE_DATA_DIRECTORY) * 20)))
        return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;

    if (file_len < (sizeof(DOS_HEADER) + 4/*pe 00*/ + sizeof(IMAGE_FILE_HEADER) + ntheader->FileHeader.SizeOfOptionalHeader + (ntheader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))))
        return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;

	if ((ntheader->FileHeader.Characteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE) == 0)
		return XENUS_ERROR_PE_NO_DYN_BASE;
	
    return XENUS_OKAY;
}

error_t pe_loader_check_segs(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, size_t file_len)
{
    size_t i;
    PIMAGE_SECTION_HEADER secs;

    secs = (PIMAGE_SECTION_HEADER)((uint64_t)&ntheader->OptionalHeader64 + ntheader->FileHeader.SizeOfOptionalHeader);

    for (i = 0; i < ntheader->FileHeader.NumberOfSections; i++)
        if ((file_len) < (secs[i].PointerToRawData + secs[i].SizeOfRawData))
            return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;


	//TODO: check alignment
    return XENUS_OKAY;
}

error_t pe_loader_check_sec(PDOS_HEADER dheader, PIMAGE_NT_HEADERS ntheader, size_t length)
{
	hackerz("You may want to change this stub to MOV RAX, XENUS_OKAY; RET; :^)");

    return XENUS_OKAY;
}

error_t pe_loader_check(void * ptr, size_t min_dir_level, size_t file_len)
{
    error_t ret;
    PDOS_HEADER dheader;
    PIMAGE_NT_HEADERS ntheader;

    dheader = (PDOS_HEADER)ptr;
    ntheader = (PIMAGE_NT_HEADERS)0;

    if ((sizeof(DOS_HEADER) + sizeof(IMAGE_FILE_HEADER) + 4) > file_len)
        return XENUS_ERROR_PE_GENERIC_UNUSUAL_SIZE;

    if ((ret = pe_loader_check_dos(dheader, ntheader)) != XENUS_OKAY)
        return ret;

    ntheader = (PIMAGE_NT_HEADERS)((uint64_t)ptr + dheader->e_lfanew); //TODO: is this pointer valid?

    if (ERROR(ret = pe_loader_check_pe(dheader, ntheader)))
        return ret;

    if (ERROR(ret = pe_loader_check_nt(dheader, ntheader, file_len)))
        return ret;

    if (ERROR(ret = pe_loader_check_segs(dheader, ntheader, file_len)))
        return ret;

    if (min_dir_level >= IMAGE_DIRECTORY_ENTRY_EXPORT)
    {
        if (ERROR(ret = pe_loader_check_datadir(ntheader, IMAGE_DIRECTORY_ENTRY_EXPORT, sizeof(IMAGE_EXPORT_DIRECTORY), file_len)))
            return ret;
        else if (ERROR(ret = pe_loader_check_exports(dheader, ntheader, file_len)))
            return ret;
    }

    if (min_dir_level >= IMAGE_DIRECTORY_ENTRY_IMPORT)
    {
        if (ERROR(ret = pe_loader_check_datadir(ntheader, IMAGE_DIRECTORY_ENTRY_IMPORT, sizeof(IMAGE_IMPORT_DESCRIPTOR), file_len)))
            return ret;
        else if (ERROR(ret = pe_loader_check_imports(dheader, ntheader, file_len)))
            return ret;
    }

    if (min_dir_level >= IMAGE_DIRECTORY_ENTRY_IAT)
    {
        if (ERROR(ret = pe_loader_check_datadir(ntheader, IMAGE_DIRECTORY_ENTRY_IAT, 0, file_len)))
            return ret;
        else if (ERROR(ret = pe_loader_check_iat(dheader, ntheader, file_len)))
            return ret;
    }

    if (ERROR(ret = pe_loader_check_sec(dheader, ntheader, file_len)))
        return ret;

    return XENUS_OKAY;
}
