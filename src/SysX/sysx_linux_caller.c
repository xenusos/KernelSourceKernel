/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "sysx_linux_caller.h"

XENUS_SYM size_t ez_linux_caller(sysv_fptr_t function, size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11, size_t a_12)
{
    return wrap_msft_sysv_12(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, a_12, function);
}