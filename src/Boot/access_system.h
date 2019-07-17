/*
    Purpose: post-stage one helper macros for generic functions
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/
#pragma once
#include "bootstrap_functions.h"

#define BOOTSTRAP_FUNCTION(section, name, ...) g_functions-> ## section ## . ## name(__VA_ARGS__)

//DBG
#define print(...)                    BOOTSTRAP_FUNCTION(dbg, print, __VA_ARGS__)
#define panic(...)                    BOOTSTRAP_FUNCTION(dbg, panic, __VA_ARGS__)
#define reload(...)                   BOOTSTRAP_FUNCTION(dbg, reload, __VA_ARGS__)

//MEM
#define execalloc(...)                BOOTSTRAP_FUNCTION(memory, execalloc, __VA_ARGS__)
#define execfree(...)                 BOOTSTRAP_FUNCTION(memory, execfree, __VA_ARGS__)
#define kmalloc(...)                  BOOTSTRAP_FUNCTION(memory, kmalloc, __VA_ARGS__)
#define vmalloc(...)                  BOOTSTRAP_FUNCTION(memory, vmalloc, __VA_ARGS__)
#define kfree(...)                    BOOTSTRAP_FUNCTION(memory, kfree, __VA_ARGS__)
#define vfree(...)                    BOOTSTRAP_FUNCTION(memory, vfree, __VA_ARGS__)
#define ksize(...)                    BOOTSTRAP_FUNCTION(memory, ksize, __VA_ARGS__)
#define memcpy(...)                   BOOTSTRAP_FUNCTION(memory, memcpy, __VA_ARGS__)
#define memset(...)                   BOOTSTRAP_FUNCTION(memory, memset, __VA_ARGS__)
#define memmove(...)                  BOOTSTRAP_FUNCTION(memory, memmove, __VA_ARGS__)
#define memcmp(...)                   BOOTSTRAP_FUNCTION(memory, memcmp, __VA_ARGS__)
#define memscan(...)                  BOOTSTRAP_FUNCTION(memory, memscan, __VA_ARGS__)

//STRING
#define strcasecmp(...)               BOOTSTRAP_FUNCTION(string, strcasecmp, __VA_ARGS__)
#define strncasecmp(...)              BOOTSTRAP_FUNCTION(string, strncasecmp, __VA_ARGS__)
#define strncpy(...)                  BOOTSTRAP_FUNCTION(string, strncpy, __VA_ARGS__)
#define strlcpy(...)                  BOOTSTRAP_FUNCTION(string, strlcpy, __VA_ARGS__)
#define strcat(...)                   BOOTSTRAP_FUNCTION(string, strcat, __VA_ARGS__)
#define strncat(...)                  BOOTSTRAP_FUNCTION(string, strncat, __VA_ARGS__)
#define strlcat(...)                  BOOTSTRAP_FUNCTION(string, strlcat, __VA_ARGS__)
#define strcmp(...)                   BOOTSTRAP_FUNCTION(string, strcmp, __VA_ARGS__)
#define strncmp(...)                  BOOTSTRAP_FUNCTION(string, strncmp, __VA_ARGS__)
#define strchr(...)                   BOOTSTRAP_FUNCTION(string, strchr, __VA_ARGS__)
#define strrchr(...)                  BOOTSTRAP_FUNCTION(string, strrchr, __VA_ARGS__)
#define strnchr(...)                  BOOTSTRAP_FUNCTION(string, strnchr, __VA_ARGS__)
#define skip_spaces(...)              BOOTSTRAP_FUNCTION(string, skip_spaces, __VA_ARGS__)
#define strim(...)                    BOOTSTRAP_FUNCTION(string, strim, __VA_ARGS__)
#define strlen(...)                   BOOTSTRAP_FUNCTION(string, strlen, __VA_ARGS__)
#define strnlen(...)                  BOOTSTRAP_FUNCTION(string, strnlen, __VA_ARGS__)
#define strspn(...)                   BOOTSTRAP_FUNCTION(string, strspn, __VA_ARGS__)
#define strcspn(...)                  BOOTSTRAP_FUNCTION(string, strcspn, __VA_ARGS__)
#define strpbrk(...)                  BOOTSTRAP_FUNCTION(string, strpbrk, __VA_ARGS__)
#define strsep(...)                   BOOTSTRAP_FUNCTION(string, strsep, __VA_ARGS__)
#define sysfs_streq(...)              BOOTSTRAP_FUNCTION(string, sysfs_streq, __VA_ARGS__)
#define strtobool(...)                BOOTSTRAP_FUNCTION(string, strtobool, __VA_ARGS__)
#define strstr(...)                   BOOTSTRAP_FUNCTION(string, strstr, __VA_ARGS__)
#define strnstr(...)                  BOOTSTRAP_FUNCTION(string, strnstr, __VA_ARGS__)
#define strcpy(...)                   BOOTSTRAP_FUNCTION(string, strcpy, __VA_ARGS__)
                                      
//THREADINAG                          
#define mutex_init(...)               BOOTSTRAP_FUNCTION(mutex, mutex_init, __VA_ARGS__)
#define mutex_lock(...)               BOOTSTRAP_FUNCTION(mutex, mutex_lock, __VA_ARGS__)
#define mutex_trylock(...)            BOOTSTRAP_FUNCTION(mutex, mutex_trylock, __VA_ARGS__)
#define mutex_unlock(...)             BOOTSTRAP_FUNCTION(mutex, mutex_unlock, __VA_ARGS__)
#define mutex_destroy(...)            BOOTSTRAP_FUNCTION(mutex, mutex_destroy, __VA_ARGS__)
#define mutex_is_locked(...)          BOOTSTRAP_FUNCTION(mutex, mutex_is_locked, __VA_ARGS__)
                                      
// CPU                                
#define kernel_fpu_begin(...)         BOOTSTRAP_FUNCTION(cpu, kernel_fpu_begin, __VA_ARGS__)
#define kernel_fpu_end(...)           BOOTSTRAP_FUNCTION(cpu, kernel_fpu_end, __VA_ARGS__)
#define __kernel_fpu_begin(...)       BOOTSTRAP_FUNCTION(cpu, __kernel_fpu_begin, __VA_ARGS__)
#define __kernel_fpu_end(...)         BOOTSTRAP_FUNCTION(cpu, __kernel_fpu_end, __VA_ARGS__)
                                      
#define create_thread_unsafe(...)     BOOTSTRAP_FUNCTION(cpu, create_thread_unsafe, __VA_ARGS__)
#define get_current_pid(...)          BOOTSTRAP_FUNCTION(cpu, get_current_pid, __VA_ARGS__)
#define _current(...)                 BOOTSTRAP_FUNCTION(cpu, _current, __VA_ARGS__)
#define tls(...)                      BOOTSTRAP_FUNCTION(cpu, tls, __VA_ARGS__)
#define preempt_disable(...)          BOOTSTRAP_FUNCTION(cpu, preempt_disable, __VA_ARGS__)
#define preempt_enable(...)           BOOTSTRAP_FUNCTION(cpu, preempt_enable, __VA_ARGS__)
                                      
// fio                                
#define file_open(...)                BOOTSTRAP_FUNCTION(fio, file_open, __VA_ARGS__)
#define file_open_readonly(...)       BOOTSTRAP_FUNCTION(fio, file_open_readonly, __VA_ARGS__)
#define file_open_readwrite(...)      BOOTSTRAP_FUNCTION(fio, file_open_readwrite, __VA_ARGS__)
#define file_open_append(...)         BOOTSTRAP_FUNCTION(fio, file_open_append, __VA_ARGS__)
#define file_read(...)                BOOTSTRAP_FUNCTION(fio, file_read, __VA_ARGS__)
#define file_write(...)               BOOTSTRAP_FUNCTION(fio, file_write, __VA_ARGS__)
#define file_mode(...)                BOOTSTRAP_FUNCTION(fio, file_mode, __VA_ARGS__)
#define file_length(...)              BOOTSTRAP_FUNCTION(fio, file_length, __VA_ARGS__)
#define file_mt(...)                  BOOTSTRAP_FUNCTION(fio, file_mt, __VA_ARGS__)
#define file_ct(...)                  BOOTSTRAP_FUNCTION(fio, file_ct, __VA_ARGS__)
#define file_at(...)                  BOOTSTRAP_FUNCTION(fio, file_at, __VA_ARGS__)
#define file_sync(...)                BOOTSTRAP_FUNCTION(fio, file_sync, __VA_ARGS__)
#define file_close(...)               BOOTSTRAP_FUNCTION(fio, file_close, __VA_ARGS__)

// The only linux specific functions:
#define kallsyms_lookup_name(...)     BOOTSTRAP_FUNCTION(symbols, kallsyms_lookup_name, __VA_ARGS__)
#define __symbol_get(...)             BOOTSTRAP_FUNCTION(symbols, __symbol_get, __VA_ARGS__)

#include <kernel/libx/xenus_strings.h>        // DOES NOT DEPEND ON A CALL TO XLIB_START! this is just formatting stuff
#include <kernel/libx/xenus_dbg.h>            // 