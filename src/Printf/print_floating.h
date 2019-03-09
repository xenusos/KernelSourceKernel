/*
    Purpose: Stage 3 floating point and unicode support 
    Author: Reece W.
    License: All rights reserved (2018) J. Reece Wislon 
*/  
#pragma once

extern void xenus_printf_write_floatpoint(safe_double_ref d, int_t places, void *data, putc_f putf, printf_state_ref current, printf_write_all_t write);