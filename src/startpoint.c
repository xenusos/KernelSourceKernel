/*
    Purpose: Entrypoint of the Xenus Kernel
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>

#include "Boot/stage_zero.h"
#include "Boot/stage_one.h"
#include "Boot/stage_two.h"
#include "Boot/stage_three.h"

START_POINT(StartPoint)
{
    ssize_t err;

    err = 0;

    if (err = stage_zero(xenos_start, bootstrap, security, sec_len, port_structs, port_structs_length, info))
        return -err;

    if (err = stage_one(xenos_start, bootstrap, security, sec_len, port_structs, port_structs_length, info))
        return -err;

    if (err = stage_two(xenos_start, bootstrap, security, sec_len, port_structs, port_structs_length, info))
        return -err;

    if (err = stage_three(xenos_start, bootstrap, security, sec_len, port_structs, port_structs_length, info))
        return -err;

    return err;
}