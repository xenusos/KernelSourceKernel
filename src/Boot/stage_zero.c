/*
    Purpose: Stage zero verifies the parameters of the startpoint, and in cases where they contain SysV stuff, stage zero translates them
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "bootstrap_functions.h"

#ifdef KERNEL_BOOTSTRAP_FUNCTIONS_AMD64SYSV
    #include "../SysX/sysx_translate_hack_bootstrap.h"
#endif

void stage_zero_print(bootstrap_t * bootstrap, const char * str)
{
#ifdef KERNEL_BOOTSTRAP_FUNCTIONS_AMD64SYSV
    //We physically can not do anything here.
#else
    bootstrap->dbg.print(str);
#endif
}

START_POINT(stage_zero)
{
    if (!bootstrap->dbg.panic)
        return STAGE_ZERO_ERR;

    if (!bootstrap->dbg.print)
        return STAGE_ZERO_ERR;

#define TEST_MEMORY(function)\
    if (!bootstrap->memory.function)\
    {\
        stage_zero_print(bootstrap, "Xenus Kernel: No memory " #function " function! \n");\
        return STAGE_ZERO_ERR;\
    }
    
    TEST_MEMORY(execalloc);
    TEST_MEMORY(execfree);
    TEST_MEMORY(kmalloc);
    TEST_MEMORY(vmalloc);
    TEST_MEMORY(kfree);
    TEST_MEMORY(vfree);
    TEST_MEMORY(ksize);
    TEST_MEMORY(memcpy);
    TEST_MEMORY(memset);
    TEST_MEMORY(memmove);
    TEST_MEMORY(memcmp);
    TEST_MEMORY(memscan);
    TEST_MEMORY(linux_page_to_pfn);
    TEST_MEMORY(linux_pfn_to_page);
    
#undef TEST_MEMORY

#define TEST_STRINGS(function)\
    if (!bootstrap->string.function)\
    {\
        stage_zero_print(bootstrap, "Xenus Kernel: No memory " #function " function! \n");\
        return STAGE_ZERO_ERR;\
    }

    TEST_STRINGS(strcasecmp);
    TEST_STRINGS(strncasecmp);
    TEST_STRINGS(strncpy);
    TEST_STRINGS(strlcpy);
    TEST_STRINGS(strcat);
    TEST_STRINGS(strncat);
    TEST_STRINGS(strlcat);
    TEST_STRINGS(strcmp);
    TEST_STRINGS(strncmp);
    TEST_STRINGS(strchr);
    TEST_STRINGS(strrchr);
    TEST_STRINGS(strnchr);
    TEST_STRINGS(skip_spaces);
    TEST_STRINGS(strim);
    TEST_STRINGS(strlen);
    TEST_STRINGS(strnlen);
    TEST_STRINGS(strspn);
    TEST_STRINGS(strcspn);
    TEST_STRINGS(strpbrk);
    TEST_STRINGS(strsep);
    TEST_STRINGS(sysfs_streq);
    TEST_STRINGS(strtobool);
    TEST_STRINGS(strstr);
    TEST_STRINGS(strnstr);
    TEST_STRINGS(strcpy);
#undef TEST_STRINGS

#define TEST_THREADING(function)\
    if (!bootstrap->cpu.function)\
    {\
        stage_zero_print(bootstrap, "Xenus Kernel: No No cpu/sched/task/related " #function " function! \n");\
        return STAGE_ZERO_ERR;\
    }

    TEST_THREADING(kernel_fpu_begin);
    TEST_THREADING(kernel_fpu_end);
    TEST_THREADING(__kernel_fpu_begin);
    TEST_THREADING(__kernel_fpu_end);
    TEST_THREADING(create_thread_unsafe);
    TEST_THREADING(get_current_pid);
    TEST_THREADING(_current);
    TEST_THREADING(tls);
#undef TEST_THREADING

#define TEST_MUTEX(function)\
    if (!bootstrap->mutex.function)\
    {\
        stage_zero_print(bootstrap, "Xenus Kernel: No mutex " #function " function! \n");\
        return STAGE_ZERO_ERR;\
    }

    TEST_MUTEX(mutex_init);
    TEST_MUTEX(mutex_lock);
    TEST_MUTEX(mutex_trylock);
    TEST_MUTEX(mutex_unlock);
    TEST_MUTEX(mutex_destroy);
    TEST_MUTEX(mutex_is_locked);
#undef TEST_MUTEX

#define TEST_SYMBOLS(function)\
    if (!bootstrap->symbols.function)\
    {\
        stage_zero_print(bootstrap, "Xenus Kernel: No symbol " #function " function! \n");\
        return STAGE_ZERO_ERR;\
    }

    TEST_SYMBOLS(kallsyms_lookup_name);
    TEST_SYMBOLS(__symbol_get);
#undef TEST_MUTEX

#define TEST_FIO(function)\
    if (!bootstrap->fio.function)\
    {\
        stage_zero_print(bootstrap, "Xenus Kernel: No fio " #function " function! \n");\
        return STAGE_ZERO_ERR;\
    }
    TEST_FIO(file_open);
    TEST_FIO(file_open_readonly);
    TEST_FIO(file_open_readwrite);
    TEST_FIO(file_open_append);
    TEST_FIO(file_read);
    TEST_FIO(file_write);
    TEST_FIO(file_mode);
    TEST_FIO(file_length);
    TEST_FIO(file_mt);
    TEST_FIO(file_ct);
    TEST_FIO(file_at);
    TEST_FIO(file_sync);
    TEST_FIO(file_close);
#undef TEST_FIO

#ifdef KERNEL_BOOTSTRAP_FUNCTIONS_AMD64SYSV
    translate_stage_zero_sysv(bootstrap); //linux kernel 4.17 build chain broke ms_abi, now most of the XenusKernel makes zero sense. Why not dynmaically load everything?
#endif

    //Duplicate functions buffer (usually passed as a stack ref.)
    g_functions = bootstrap->memory.kmalloc(sizeof(bootstrap_t), GFP_KERNEL);
   
    if (!g_functions)
    {
        bootstrap->dbg.print("failed to init portable structs \n");
        return STAGE_ZERO_ERR;
    }

    bootstrap->memory.memcpy(g_functions, bootstrap, sizeof(bootstrap_t));
    return 0;
}