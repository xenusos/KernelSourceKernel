/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#pragma once

void    init_exports(void * kernel_base);
void    exports_print();
void    exports_add(const char * lbl, void * function);
void *  exports_get_by_name(const char * lbl);
void *  exports_get_by_ord(int ord);