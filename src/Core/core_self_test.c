/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "core_self_test.h"
#include "../Boot/access_system.h"

#include <kernel/libx/xenus_memory.h>
#include "../PE/pe_checker.h"

static bool test_pe_checker();
static bool test_compiler_ints();
static bool test_gcc_types();

bool self_test()
{
    if (!test_pe_checker())
    {
        printf("According to the pe checker, the kernel asset is invalid. Unlikely modificaitons may have been made, but it's probable that reece fucked it up again \n");
        return false;
    }

    if (!test_compiler_ints())
    {
        printf("Illegal msvc type lengths\n");
        return false;
    }

    if (!test_gcc_types())
    {
        printf("Illegal gcc type lengths\n");
        return false;
    }

    return true;
}

static bool test_pe_checker()
{
    error_t err;
    size_t fs;
    void * buf;
    void * hd;

    fs = file_length(BOOTSTRAP_DLL);

    ASSERT(fs, "file size of " BOOTSTRAP_DLL " was zero");

    buf = malloc(fs);

    ASSERT(buf, "out of memory");

    hd = file_open_readonly(BOOTSTRAP_DLL);
    file_read(hd, 0, buf, fs);

    err = pe_loader_check(buf, fs, 9999);

    ASSERT(NO_ERROR(err), "PE Loader check error: %i\n", (int)err);

    free(buf);
    return true;
}

static bool test_compiler_ints()
{
#define TEST_TYPE(type, bytes) if (sizeof(type) != bytes) return false;
    TEST_TYPE(uint64_t, 8)
    TEST_TYPE(int64_t, 8)
    TEST_TYPE(uint32_t, 4)
    TEST_TYPE(int32_t, 4)
    TEST_TYPE(uint16_t, 2)
    TEST_TYPE(int16_t, 2)
    TEST_TYPE(uint8_t, 1)
    TEST_TYPE(int8_t, 1)
#undef TEST_TYPE
    return true;
}

static bool test_gcc_types()
{
#define TEST_TYPE(type, bytes) if (sizeof(type) != bytes) return false;
    TEST_TYPE(l_short, 2)
    TEST_TYPE(l_int, 4)
    TEST_TYPE(l_long, 8)
    TEST_TYPE(l_longlong, 8)
    TEST_TYPE(l_signed, 4)
    TEST_TYPE(l_unsigned, 4)
#undef TEST_TYPE
    return true;
}