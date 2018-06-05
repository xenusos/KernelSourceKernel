#pragma once 
/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  

XENUS_SYM_VAR void * kernel_base;

void xenus_stage_2(void * xenus_base, void *  security, uint32_t sec_len);
void xenus_state_3();