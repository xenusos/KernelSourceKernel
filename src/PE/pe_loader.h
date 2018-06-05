#pragma once
/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  

#pragma once

error_t pe_load_preload_init(void ** handle, void * buffer, size_t length); // alloc buffers
error_t pe_load_preload_analyse(void * handle);                             // chk pe
error_t pe_load_preload_load(void * handle);                                // copy header and segments
error_t pe_load_preload_config_iat(void * handle, size ** unresolved_symbols, size_t ** resolved_symbols); // update import entries

// error_t pe_get_module_address(void * handle, void * virt_address, void ** out_address);
// error_t pe_check_charactistic(void * handle, size_t flag, bool ** state)
// error_t  pe_get_export_by_ordinal(void * handle, uint16_t ord, char ** name, void ** data_ptr)
// error_t  pe_get_export_by_name(void * handle, char * name, uint16_t * ord,void ** data_ptr)
