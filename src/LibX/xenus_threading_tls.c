/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>					// int types, common types, etc
#include <libx\xenus_threads.h>		// predecleration of public apis
#include "_xenus_threads.h"			// init predecleration

#include <libx\xenus_chain.h>		// deps
#include <libx\xenus_list_dyn.h>	// deps
#include <libx/xenus_memory.h>		// deps
#include "../access_sys.h"			// deps

static mutex_k _threading_tls_mutex = NULL;
static mutex_k _threading_indexing_mutex = NULL;
static void * _tls_chain_base = NULL;

extern void _mm_pause(void);
XENUS_EXPORT void thread_pause()
{
    _mm_pause();
}

uint32_t thread_geti()
{
    uint64_t ret;

    ret = get_current_pid();

    if (ret > 4194304 - 1) //(2^22)-1
        panic("Xenus was made in 2018, and your Linux build was made when? Too many threads yo!");

    return (uint32_t)ret;
}

//TODO (Reece): We could significantly increase the performance of this. 
// divide and conquer or just MAKELONGLONG? eh
// modules will all share one chain so it's going to be kinda slow
// but right now, this is the least of our concerns. 
//UPDATE (Reece):
// TODO: add thread storage in task_struct

XENUS_EXPORT error_t thread_tls_allocate(uint32_t thash, uint32_t length, void ** out)
{
    error_t err;
    void * chain;

    mutex_lock(_threading_tls_mutex);

    err = chain_allocate(_tls_chain_base, 
        MAKELONGLONG(thash, thread_geti()), 
        length,
        &chain,
        out);

    if (!_tls_chain_base)
        _tls_chain_base = chain;

    mutex_unlock(_threading_tls_mutex);

    return err;
}

XENUS_EXPORT error_t thread_tls_deallocate(uint32_t thash)
{
    error_t ret;
    
    mutex_lock(_threading_tls_mutex);
    ret = chain_deallocate_ex(_tls_chain_base,
        MAKELONGLONG(thash, thread_geti()),
        NULL);
    mutex_unlock(_threading_tls_mutex);

    return ret;
}

XENUS_EXPORT error_t thread_tls_get(uint32_t thash, void ** out)
{
    error_t ret;

    mutex_lock(_threading_tls_mutex);
    ret = chain_get(_tls_chain_base,
        MAKELONGLONG(thash, thread_geti()),
        NULL,
        out);
    mutex_unlock(_threading_tls_mutex);

    return ret;
}

XENUS_SYM error_t thread_indexing_create(uint32_t max_threads, thread_index_counter_p * tic)
{
	dyn_list_head_p list;
	thread_index_counter_p head;

	if (!tic)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	list = DYN_LIST_CREATE(uint32_t);

	if (!list)
		return XENUS_ERROR_TLS_FAILED_TO_CREATE_LIST;

	head = malloc(sizeof(thread_index_counter_t));

	if (!head)
	{
		dyn_list_destory(list); //TODO: BUGCHECK IF ERROR... or?
		return XENUS_ERROR_TLS_FAILED_TO_CREATE_LIST;
	}
	
	head->list = list;
	head->max_threads = max_threads ? max_threads : 1;
	head->counter = 0;

	*tic = head;
	return XENUS_OKAY;
}

XENUS_SYM error_t thread_indexing_destory(thread_index_counter_p tic)
{
	if (!tic)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
	if (ERROR(dyn_list_destory(tic->list)))
		return XENUS_ERROR_TLS_FAILED_TO_DESTORY;
	free(tic);
	return XENUS_OKAY;
}

XENUS_SYM error_t thread_indexing_register(thread_index_counter_p tic)
{
	error_t err;
	uint32_t * task;
	uint32_t pid;

	if (!tic)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	pid = thread_geti();

	if ((err = thread_indexing_get(tic, 0)) != XENUS_ERROR_TLS_NOT_REGISTERED) 
		return err; 

	if (tic->counter >= tic->max_threads)
		panicf("Thread counter limit exceeded. (max threads: %i)", tic->max_threads);

	tic->counter++;

	mutex_lock(_threading_indexing_mutex);
	err = dyn_list_append(tic->list, (void *)&task);
	mutex_unlock(_threading_indexing_mutex);

	if (ERROR(err))
		return XENUS_ERROR_TLS_FAILED_TO_ADD;

	*task = pid;
	return XENUS_OKAY;
}

XENUS_SYM error_t thread_indexing_get(thread_index_counter_p tic, uint32_t * index)
{
	error_t ret;
	uint32_t id; 
	size_t arr_id;

	if (!tic)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (!index)
		return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	id = thread_geti();

	mutex_lock(_threading_indexing_mutex);
	ret = dyn_list_get_by_buffer((dyn_list_head_p) tic->list, &id, &arr_id);
	
	if (ERROR(ret)) goto err;
	else if (ret == XENUS_STATUS_LISTS_NOT_FOUND) //Not actually an error, this is a status code. 
	{
		ret = XENUS_ERROR_TLS_NOT_REGISTERED;
		goto err;
	}  

	ret = dyn_list_get_by_index((dyn_list_head_p)tic->list, arr_id, (void **) index);

err:
	mutex_unlock(_threading_indexing_mutex);
	return ret;
}

error_t xlib_thread_start()
{
	_threading_indexing_mutex = mutex_init();

	if (!_threading_indexing_mutex)
	{
		return XENUS_ERROR_LIBX_FAILED_TO_CREATE_MUTEX;
	}
    _threading_tls_mutex = mutex_init();

	if (!_threading_tls_mutex)
	{
		return XENUS_ERROR_LIBX_FAILED_TO_CREATE_MUTEX;
	}

	return XENUS_OKAY;
}