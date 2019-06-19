/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>                            // int types, common types, etc
#include <kernel/libx/xenus_threads.h>        // predecleration of public apis

#include "../Boot/access_system.h"            // deps

#include <kernel/libx/xenus_chain.h>        // deps
#include <kernel/libx/xenus_list_dyn.h>        // deps
#include <kernel/libx/xenus_memory.h>        // deps

#include "../StackFixup/stack_realigner.h"

typedef struct
{
    void * data;
    const char * name;
    thread_callback_t callback;
} thread_data_t,* thread_data_p;

extern void _ReadWriteBarrier(void);
extern void __faststorefence(void);

static thread_enter_cpu_p on_cpu_callback = NULL;
static chain_p hack_exit_ntfy = 0;
static mutex_k hack_mutex = 0;

static l_int thread_callback(thread_data_p data);
static void XENUS_MS_ABI trap_kt_thread_attention_callback(uint8_t id, pt_regs_p registers);
static void XENUS_MS_ABI trap_kp_thread_attention_callback(uint8_t id, pt_regs_p registers);
static void XENUS_MS_ABI thread_destory(long);
static size_t XENUS_MS_ABI syscall_kp_attention_callback(uint8_t id, size_t arg_alpha, size_t arg_bravo, size_t arg_charlie, size_t arg_delta, size_t echo);
static size_t XENUS_MS_ABI syscall_kt_attention_callback(uint8_t id, size_t arg_alpha, size_t arg_bravo, size_t arg_charlie, size_t arg_delta, size_t echo);

XENUS_EXPORT error_t thread_create(task_k * out_task, thread_callback_t callback, void * data, const char * name, bool run)
{
    thread_data_p thread_data;

    if (!out_task)    return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    if (!callback)    return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;
    //if (!data)        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name)
        name = "Unnammed Xenus Thread";

    thread_data = malloc(sizeof(thread_data_t));

    if (!thread_data) return XENUS_ERROR_OUT_OF_MEMORY;

    thread_data->data        = data;
    thread_data->callback    = callback;
    thread_data->name        = name;

    *out_task = create_thread_unsafe(thread_callback, thread_data, name, run);

    return XENUS_OKAY;
}

extern void _mm_pause(void);
XENUS_EXPORT void thread_pause()
{
    _mm_pause();
}

void sync_cache()
{
    _ReadWriteBarrier();
    __faststorefence();
}

error_t threading_ntfy_singleshot_exit(uint32_t pid, thread_exit_hack_t cb)
{
    error_t er;
    thread_exit_hack_t * ptr;

    if (!hack_mutex)
    {
        hack_mutex = mutex_init();
        ASSERT(hack_mutex, "couldn't allocate mutex");

        er = chain_allocate(&hack_exit_ntfy);
        ASSERT(NO_ERROR(er), "couldn't allocate chain type");
    }

    er = XENUS_OKAY;

    mutex_lock(hack_mutex);

    if ((er = chain_allocate_link(hack_exit_ntfy, pid, sizeof(thread_exit_hack_t *), NULL, NULL, (void**)&ptr)) != XENUS_OKAY)
        goto exit;

    *ptr = cb;

exit:
    mutex_unlock(hack_mutex);
    return er;
}

XENUS_EXPORT void thread_preempt_lock()
{
    preempt_disable();
}

XENUS_EXPORT void thread_preempt_unlock()
{
    preempt_enable(); //no resched by default in sys_access
}

// For critical FPU joint TLS stuff, we need to disable/enable preemption
#define XENUS_THREAD_TLS_START    thread_storage_data_p tls_; thread_preempt_lock(); tls_ = tls();
#define XENUS_THREAD_TLS_END      thread_preempt_unlock();

XENUS_EXPORT uint32_t thread_geti()
{
    uint64_t ret;

    ret = get_current_pid();

    ASSERT(ret < 4194304 - 1/*(2^22)-1*/, "Thread ID exceeds a 22bit integer. This isn't expected in Linux 4.14")

    return (uint32_t)ret;
}

void __kern_pre_context_switch(void)
{
    thread_storage_data_p tls_;
    
#ifdef KERNEL_THREADING_DBG_CSWITCHING
    printf("<dbg> kern_pre_context_switch current = %p\r\n", _current());
#endif 

    tls_ = tls();

    if (tls_->pub_switch_pre_callback)
        tls_->pub_switch_pre_callback();

    if (tls_->fpu_enabled_via_hook)
        __kernel_fpu_end();
}

void __kern_post_context_switch(void)
{
    thread_storage_data_p tls_;

#ifdef KERNEL_THREADING_DBG_CSWITCHING
    printf("<dbg> kern_post_context_switch current = %p\r\n", _current());
#endif 

    tls_ = tls();

    if (tls_->fpu_enabled_via_hook)
        __kernel_fpu_begin();

    thread_on_cpu();

    if (tls_->pub_switch_post_callback)
        tls_->pub_switch_post_callback();
}

void __try_install_switch_hooks(thread_storage_data_p tls_)
{
    if (tls_->kern_switch_post_callback)
        return;

    tls_->kern_switch_post_callback = __kern_post_context_switch;
    tls_->kern_switch_pre_callback  = __kern_pre_context_switch;
}

XENUS_EXPORT bool thread_fpu_lock()
{
    XENUS_THREAD_TLS_START

    if (tls_->fpu_enabled_via_hook)
    {
        XENUS_THREAD_TLS_END;
        return false;
    }

    tls_->fpu_enabled_via_hook = true;
    __kernel_fpu_begin();

    __try_install_switch_hooks(tls_);

    XENUS_THREAD_TLS_END
    return true;
}

XENUS_EXPORT bool thread_fpu_unlock()
{
    XENUS_THREAD_TLS_START

    if (!(tls_->fpu_enabled_via_hook))
    {
        XENUS_THREAD_TLS_END;
        return false;
    }

    tls_->fpu_enabled_via_hook = false;
    __kernel_fpu_end();

    XENUS_THREAD_TLS_END
    return true;
}

XENUS_EXPORT void thread_pre_context_switch_hook(pre_context_switch_cb_t xd)
{
    XENUS_THREAD_TLS_START

    __try_install_switch_hooks(tls_);
    tls_->pub_switch_pre_callback = xd;

    XENUS_THREAD_TLS_END
}

XENUS_EXPORT void thread_post_context_switch_hook(post_context_switch_cb_t xd)
{
    XENUS_THREAD_TLS_START

    __try_install_switch_hooks(tls_);
    tls_->pub_switch_post_callback = xd;

    XENUS_THREAD_TLS_END
}

XENUS_EXPORT void threading_set_current_trap_handler(xenus_trap_cb_t handler)
{
    thread_storage_data_p tls_;
    tls_ = tls();
    tls_->pub_task_thread_attention_callback = handler;
    tls_->trap_kt_thread_attention_callback = trap_kt_thread_attention_callback;
}

XENUS_EXPORT void threading_set_process_trap_handler(xenus_trap_cb_t handler)
{
    thread_storage_data_p tls_;
    tls_ = tls();
    tls_->pub_process_thread_attention_callback = handler;
    tls_->trap_kp_thread_attention_callback = trap_kp_thread_attention_callback;
}

XENUS_EXPORT void threading_set_current_syscall_handler(xenus_sys_cb_t handler)
{
    thread_storage_data_p tls_;
    tls_ = tls();
    tls_->pub_task_syscall_callback = handler;
    tls_->syscall_kt_attention_callback = syscall_kt_attention_callback;
}

XENUS_EXPORT void threading_set_process_syscall_handler(xenus_sys_cb_t handler)
{
    thread_storage_data_p tls_;
    tls_ = tls();
    tls_->pub_process_syscall_callback = handler;
    tls_->syscall_kp_attention_callback = syscall_kp_attention_callback;
}

XENUS_EXPORT void thread_enable_cleanup()
{
    tls()->kern_thread_exit = thread_destory;
}

XENUS_EXPORT void thread_set_on_cpu_cb(thread_enter_cpu_p callback)
{
    on_cpu_callback = callback;
}

XENUS_EXPORT void thread_on_cpu()
{
    // dont waste time w/ a context switch if we're doing on switch stuff
    thread_preempt_lock();
    if (on_cpu_callback)
        on_cpu_callback();
    thread_preempt_unlock();
}

XENUS_EXPORT error_t threading_get_exit_callbacks(thread_exit_cb_t ** list, int * cnt)
{
    thread_storage_data_p tls_; 
    tls_ = tls();

    if (!list)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!cnt)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    *list = tls_->pub_thread_exit;
    *cnt  = XENUS_PUB_THREAD_EXIT_CBS;
    return XENUS_OKAY;
}

static l_int thread_callback(thread_data_p data)
{
    l_int  ret;
    thread_fpu_lock();
    thread_enable_cleanup();
    thread_on_cpu();
    ret = (l_int)stack_realigner((size_t(*)(size_t))data->callback, (size_t)data->data);
    free(data);
    thread_fpu_unlock();
    return ret;
}

static void trap_handler(uint8_t id, pt_regs_p registers, bool is_task)
{
    xenus_trap_cb_t callback;
    xenus_attention_trap_t trap;
    bool mng_fpu;
    thread_storage_data_p tls_ = tls();

    mng_fpu = !tls_->fpu_enabled_via_hook;

    if (mng_fpu)
        thread_fpu_lock();

    sync_cache();

    callback             = is_task ? tls_->pub_task_thread_attention_callback : tls_->pub_process_thread_attention_callback;

    trap.attention_id    = id;
    trap.registers       = registers;
    
    if (callback)
        stack_realigner((size_t(*)(size_t))callback, (size_t)&trap);

    if (mng_fpu)
        thread_fpu_unlock();
}

static void XENUS_MS_ABI trap_kt_thread_attention_callback(uint8_t id, pt_regs_p registers)
{
    trap_handler(id, registers, true);
}

static void XENUS_MS_ABI trap_kp_thread_attention_callback(uint8_t id, pt_regs_p registers)
{
    trap_handler(id, registers, false);
}

static void thread_hack_ntfy(long exit)
{
    link_p link;
    thread_exit_hack_t * ptr;
    thread_exit_hack_t cb;

    if (!hack_mutex)
        return;

    mutex_lock(hack_mutex);

    if (chain_get(hack_exit_ntfy, thread_geti(), &link, (void **)&ptr) == XENUS_OKAY)
    {
        cb = *ptr;
        chain_deallocate_handle(link);
        mutex_unlock(hack_mutex);

        cb(exit);
    }
    else
    {
        mutex_unlock(hack_mutex);
    }
}

static void thread_destory_aligned(long exit)
{
    bool mng_fpu;
    thread_storage_data_p tls_ = tls();

    mng_fpu = !tls_->fpu_enabled_via_hook;

    if (mng_fpu)
        thread_fpu_lock();

    sync_cache();

    thread_hack_ntfy(exit);

    for (int i = 0; i < XENUS_PUB_THREAD_EXIT_CBS; i++)
        if (tls_->pub_thread_exit[i])
            tls_->pub_thread_exit[i](exit);

    _thread_tls_cleanup_all();

    if (mng_fpu)
        thread_fpu_unlock();
}

static void XENUS_MS_ABI thread_destory(long exit)
{
    stack_realigner((size_t(*)(size_t))thread_destory_aligned, (size_t)exit);
}

static size_t thread_on_cpu_unaligned(size_t somewhat_safer)
{
    thread_on_cpu();
}

static size_t syscall_handler(uint8_t id, size_t arg_alpha, size_t arg_bravo, size_t arg_charlie, size_t arg_delta, size_t arg_echo, bool is_task)
{
    xenus_sys_cb_t callback;
    xenus_syscall_t trap;
    bool mng_fpu;
    thread_storage_data_p tls_ = tls();

    mng_fpu = !tls_->fpu_enabled_via_hook;

    if (mng_fpu)
        thread_fpu_lock();

    thread_enable_cleanup();
    sync_cache();

    stack_realigner((size_t(*)(size_t))thread_on_cpu_unaligned, (size_t)NULL);

    callback = is_task ? tls_->pub_task_syscall_callback: tls_->pub_process_syscall_callback;

    trap.attention_id = id;
    trap.arg_alpha    = arg_alpha;
    trap.arg_bravo    = arg_bravo;
    trap.arg_charlie  = arg_charlie;
    trap.arg_delta    = arg_delta;
    trap.arg_echo     = arg_echo;

    if (callback)
        stack_realigner((size_t(*)(size_t))callback, (size_t)&trap);

    if (mng_fpu)
        thread_fpu_unlock();

    return trap.response;
}

static size_t XENUS_MS_ABI syscall_kt_attention_callback(uint8_t id, size_t arg_alpha, size_t arg_bravo, size_t arg_charlie, size_t arg_delta, size_t arg_echo)
{
    return syscall_handler(id, arg_alpha, arg_bravo, arg_charlie, arg_delta, arg_echo, true);
}

static size_t XENUS_MS_ABI syscall_kp_attention_callback(uint8_t id, size_t arg_alpha, size_t arg_bravo, size_t arg_charlie, size_t arg_delta, size_t arg_echo)
{
    return syscall_handler(id, arg_alpha, arg_bravo, arg_charlie, arg_delta, arg_echo, false);
}