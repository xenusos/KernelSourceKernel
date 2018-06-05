/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include <libx\xenus_strings.h>
#include "../core_exports.h"
#include "../access_sys.h"
#include "tinyprintf.h"

XENUS_EXPORT size_t vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
	return tfp_vsnprintf(buf, size, fmt, ap);
}

XENUS_EXPORT size_t vsprintf(char *buf, const char *fmt, va_list ap)
{
	return tfp_vsprintf(buf, fmt, ap);
}