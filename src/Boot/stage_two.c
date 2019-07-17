/*
    Purpose: Stage two looks up all the required linux symbols and creates translation stubs
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "bootstrap_functions.h"

#include "../SysX/sysx_translate_static_linux.h"

START_POINT(stage_two)
{
    lookup_symbols(); //TODO: in production, consider error if symbol is missing.
    return 0;
}