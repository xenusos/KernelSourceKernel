/*
    Purpose: Proprietary entrypoint that is called from within the linux kernel module bootstrapper.
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  

#include <xenus.h>
#include "core_init.h"
#include "g_functions.h"
bootstrap_t * g_functions = 0;
#include "SysX/sys_main.h"

#include <_/_xenus_linux_types_memory.h>

/*
    xenos_start - xenos pe header base
    bootstrap   - msft abi compliant functions
    mode        - tbd
    security    - tbd
    length      - length of security buffer
*/            
XENUS_EXPORT int StartPoint(void *xenos_start, bootstrap_t * bootstrap, void *  security, uint32_t sec_len, void * port_structs, uint32_t port_structs_length)
{
    if (bootstrap->dbg.panic && !(bootstrap->dbg.print))
        return -1;

    if (bootstrap->dbg.print && !(bootstrap->dbg.panic))
        return -2;

    if (!bootstrap->dbg.print && !bootstrap->dbg.panic)
        return -3;

#define TEST_MEMORY(function)\
    if (!bootstrap->memory.function)\
    {\
        /*bootstrap->dbg.print("Xenus Kernel: No memory " #function " function!");*/\
        return -4;\
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
#undef TEST_MEMORY

#define TEST_STRINGS(function)\
    if (!bootstrap->string.function)\
    {\
        /*bootstrap->dbg.print("Xenus Kernel: No memory " #function " function!");*/\
        return -5;\
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
        /*bootstrap->dbg.print("Xenus Kernel: No No cpu/sched/task/related " #function " function!");*/\
        return -6;\
    }

	TEST_THREADING(kernel_fpu_begin);
	TEST_THREADING(kernel_fpu_end);
	TEST_THREADING(create_thread_unsafe);
	TEST_THREADING(get_current_pid);
	TEST_THREADING(_current);
	TEST_THREADING(tls);
#undef TEST_THREADING

#define TEST_MUTEX(function)\
    if (!bootstrap->mutex.function)\
    {\
        /*bootstrap->dbg.print("Xenus Kernel: No mutex " #function " function!");*/\
        return -7;\
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
        /*bootstrap->dbg.print("Xenus Kernel: No symbol " #function " function!");*/\
        return -8;\
    }

	TEST_SYMBOLS(kallsyms_lookup_name);
#undef TEST_MUTEX

#define TEST_FIO(function)\
    if (!bootstrap->fio.function)\
    {\
        /*bootstrap->dbg.print("Xenus Kernel: No fio " #function " function!");*/\
        return -9;\
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

	gcc_no_msi_abi(bootstrap); //linux kernel 4.17 build chain broke ms_abi, now most of the XenusKernel makes zero sense. Why not dynmaically load everything?
	
    //Duplicate functions buffer (usually passed as a stack ref.)
    g_functions = bootstrap->memory.kmalloc(sizeof(bootstrap_t), GFP_KERNEL);
   
	if (!g_functions)
	{
		bootstrap->dbg.print("failed to init portable structs");
		return -10;
	}

	bootstrap->memory.memcpy(g_functions, bootstrap, sizeof(bootstrap_t));

	//Copy portable structs buffer into our local buffer
	if (ERROR(ps_buffer_configure(port_structs, port_structs_length)))
	{
		bootstrap->dbg.print("failed to init portable structs");
		return -11;
	}
	
    xenus_stage_2(xenos_start, security, sec_len);
	xenus_state_3();

	return 0;
}
