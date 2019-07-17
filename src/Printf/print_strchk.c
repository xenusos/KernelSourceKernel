/*
    Purpose: Stage 3 protection-ish against some null pointer dereferences and/or the printing of bad strings 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "print_strchk.h"
#include "../Boot/access_system.h"

uint64_t kslr_start_addr = 0;

bool xenus_printf_check_string(const char * str)
{
    if (!kslr_start_addr) return true;
    if (kslr_start_addr > (uint64_t)str)
        return false;
    return strnlen(str, PRINTF_MAX_STRING_LENGTH) != PRINTF_MAX_STRING_LENGTH;
}

void xenus_printf_init_strs()
{
    kslr_start_addr = *(l_long*)kallsyms_lookup_name("page_offset_base");
}
