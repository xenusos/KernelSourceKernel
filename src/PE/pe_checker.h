/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#pragma once


// Warning: this will not check in-memory PE images.
// Buffer must equal a buffered PE file 
error_t pe_loader_check(void * buffer, size_t length, size_t min_dir_level);