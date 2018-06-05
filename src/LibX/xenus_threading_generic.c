/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>					// int types, common types, etc
#include <libx/xenus_threads.h>		// predecleration of public apis

#include <libx/xenus_chain.h>		// deps
#include <libx/xenus_list_dyn.h>	// deps
#include <libx/xenus_memory.h>		// deps
#include "../access_sys.h"			// deps

typedef struct
{
	void * data;
	char * name;
	thread_callback_t callback;
} thread_data_t,* thread_data_p;

int thread_callback(void * data)
{
	int ret;
	thread_data_p thread_data;

	thread_data = data;

	thread_fpu_lock();
	ret = thread_data->callback(thread_data->data);
	free(data);
	thread_fpu_unlock();
	return ret;
}

XENUS_SYM error_t     thread_create(task_k * out_task, thread_callback_t callback, void * data, char * name)
{
	thread_data_p thread_data;

	if (!out_task) return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
	if (!callback) return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
	if (!data) return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

	if (!name)
		name = "Unnammed Xenus Thread";

	thread_data = malloc(sizeof(thread_data_t));

	if (!thread_data) return XENUS_ERROR_OUT_OF_MEMORY;

	thread_data->data = data;
	thread_data->callback;
	thread_data->name = name;

	*out_task = create_thread_unsafe(thread_callback, thread_data, name);
	return XENUS_OKAY;
}

void kern_pre_context_switch(void)
{
	printf("<info> kern_pre_context_switch\r\n");
	thread_storage_data_p tls_;

	tls_ = tls();

	if (tls_->usr_switch_pre_cb)
		tls_->usr_switch_pre_cb();

	if (tls_->fpu_enabled_via_hook)
		kernel_fpu_end();
}

void kern_post_context_switch(void)
{
	printf("<info> kern_post_context_switch\r\n");
	thread_storage_data_p tls_;

	tls_ = tls();

	if (tls_->fpu_enabled_via_hook)
		kernel_fpu_begin();

	if (tls_->usr_switch_post_cb)
		tls_->usr_switch_post_cb();
}

void try_install_switch_hooks()
{
	thread_storage_data_p tls_;

	tls_ = tls();

	if (tls_->kern_switch_post_cb) return;

	tls_->kern_switch_post_cb = kern_post_context_switch;
	tls_->kern_switch_pre_cb = kern_pre_context_switch;
}

XENUS_SYM error_t thread_fpu_lock()
{
	thread_storage_data_p tls_;

	tls_ = tls();

	if (tls_->fpu_enabled_via_hook)
	{
		printf("<warning> nested call to thread_fpu_lock\r\n");
		return XENUS_STATUS_NESTED_CALL;
	}

	try_install_switch_hooks();


	preempt_enable();
	tls_->fpu_enabled_via_hook = true;
	kernel_fpu_begin();
	preempt_disable();

	return XENUS_OKAY;
}

XENUS_SYM error_t thread_fpu_unlock()
{
	thread_storage_data_p tls_;

	tls_ = tls();

	if (!(tls_->fpu_enabled_via_hook))
	{
		printf("<warning> nested call to thread_fpu_lock\r\n");
		return XENUS_STATUS_NESTED_CALL;
	}

	try_install_switch_hooks();

	preempt_enable();
	tls_->fpu_enabled_via_hook = false;
	kernel_fpu_end();
	preempt_disable();

	return XENUS_OKAY;
}

XENUS_SYM error_t thread_pre_context_switch_hook(pre_context_switch_callback_t xd)
{
	try_install_switch_hooks();
	tls()->usr_switch_pre_cb = xd;
	return XENUS_OKAY;
}

XENUS_SYM error_t thread_post_context_switch_hook(post_context_switch_callback_t xd)
{
	try_install_switch_hooks();
	tls()->usr_switch_post_cb = xd;
	return XENUS_OKAY;
}