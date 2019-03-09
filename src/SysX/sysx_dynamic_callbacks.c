/*
    Purpose: Dynamic system v stub generation for object oriented components (calling convention translation with an additional parameter for arbitrary user data)
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "sysx_dynamic_callbacks.h"
#include "../Boot/access_system.h"
#include <kernel/libx/xenus_memory.h>

const uint8_t dseg_init_stub[]               = { 0x55 , 0x48, 0x89, 0xE5, 0x48, 0x83, 0xC5, 0x10 };                           // push rbx; mov rbx, rsp; add rbx, 8; add rbx, 8;
const uint8_t dseg_end_stub[]                = { 0x5D, 0xC3 };                                                                // pop rbx, ret
                                                                                                                              //
const uint8_t dseg_push_vol_regs[]           = { 0x41, 0x53, 0x41, 0x57 };                                                    // push all registers that are non-volatile in SysV and volatile in MSFT. + push registers that we change. push   r11; push   r15
const uint8_t dseg_pop_vol_regs[]            = { 0x41, 0x5F, 0x41, 0x5B };                                                    // inverse of the former ^ 
                                                                                                                              //
const uint8_t dseg_push_parameter_n[]        = { 0xFF, 0x75, 0x69 };                                                          // 7 or data[2] = (parameter - 7) * 8
const uint8_t dseg_push_parameter_6[]        = { 0x41, 0x51 };                                                                // 6
const uint8_t dseg_push_parameter_5[]        = { 0x41, 0x50 };                                                                // 5
                                                                                                                              //
const uint8_t dseg_remap_first_four[]        = { 0x49, 0x89, 0xD0, 0x49, 0x89, 0xC9, 0x48, 0x89, 0xF9, 0x48, 0x89, 0xF2 };    // order specific, remap first four paremters via movs
                                                                                                                              //
const uint8_t dseg_allocate_shadow_space[]   = { 0x48, 0x83, 0xEC, 0x20 };                                                    // sub rsp, 32
const uint8_t dseg_nuke_shadow_space[]       = { 0x48, 0x83, 0xC4, 0x20 };                                                    // add rsp, 32
                                                                                                                              //
const uint8_t dseg_nuke_stack_n[]            = { 0x48, 0x83, 0xC4, 0x69 };                                                    // sub rsp, 0x69
                                                                                                                              //
const uint8_t dseg_call_addr[]               = { 0x48, 0xB8, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0xFF, 0xD0 };    //
                                                                                                                              //
const uint8_t dseg_push_n[]                  = { 0x48, 0xB8, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x50 };          // movabs rax, 0x69; push rax;

const uint8_t selector_init_rax[]            = { 0x48, 0x89, 0xe0 };                                                          // mov    rax,rsp; 
const uint8_t selector_and_rax[]             = { 0x48, 0x83, 0xe0, 0x0f };                                                    // and    rax,0xf; 
const uint8_t selector_cmp[]                 = { 0x48, 0x83, 0xf8, 0x00 };                                                    // cmp    rax,0x0 
const uint8_t selector_jmp_to_true_n[]       = { 0x74, 0x0c };                                                                // je     n <equal>
const uint8_t selector_call_n[]              = { 0x48, 0xB8, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0xff, 0xe0 };    // movabs rax, 0x69;

typedef struct
{
    void * selector;
    void * stub_aligned;
    void * stub_unaligned;
} dyncb_struct_t, *dyncb_struct_p;

static size_t _dyncb_calc_translator_size(uint8_t parameters)
{
    size_t additional = 0;

    if (parameters >= 5)
        additional += sizeof(dseg_push_parameter_5);

    if (parameters >= 6)
        additional += sizeof(dseg_push_parameter_6);

    if (parameters > 6)
        additional += (parameters - 6) * sizeof(dseg_push_parameter_n);

    return sizeof(dseg_init_stub) + sizeof(dseg_push_vol_regs) + sizeof(dseg_push_n) + sizeof(dseg_push_n) + sizeof(dseg_push_n) + additional + sizeof(dseg_remap_first_four) +
           sizeof(dseg_allocate_shadow_space) + sizeof(dseg_call_addr) + sizeof(dseg_nuke_shadow_space) + sizeof(dseg_nuke_stack_n) + sizeof(dseg_pop_vol_regs) + sizeof(dseg_end_stub);
}

static size_t _dyncb_calc_selector_size()
{
    return sizeof(selector_init_rax) + sizeof(selector_and_rax) + sizeof(selector_cmp) + sizeof(selector_jmp_to_true_n) + sizeof(selector_call_n) + sizeof(selector_call_n);
}

static void _dyncb_alloc_translator_stub(bool aligned, int parameters, void * msft_data, void * msft_func, void * sys_v, uint64_t max_len)
{
    bool needs_algn_pop;
    size_t index;

    needs_algn_pop = false;
    index = 0;

    #define CALC_OFFSET(off, length)       ((void *)(((size_t)(sys_v)) + index - length + off))
    #define EMIT(buffer, length)    {memcpy((void *)(((size_t)(sys_v)) + index), buffer, length); index += length;}
    #define EMIT_CONST_ARRAY(arr)     EMIT(arr, sizeof(arr))
    
    #define EMIT_INIT                 EMIT_CONST_ARRAY(dseg_init_stub)
    #define EMIT_END                  EMIT_CONST_ARRAY(dseg_end_stub)
    
    #define EMIT_PUSH_REGS            EMIT_CONST_ARRAY(dseg_push_vol_regs)
    #define EMIT_POP_REGS             EMIT_CONST_ARRAY(dseg_pop_vol_regs) 
    
    #define EMIT_PUSH_PAREMTER(n)     {EMIT_CONST_ARRAY(dseg_push_parameter_n) *(uint8_t *)CALC_OFFSET(2, sizeof(dseg_push_parameter_n)) = n; } 
    
    #define EMIT_PUSH_PAREMTER_6      EMIT_CONST_ARRAY(dseg_push_parameter_6)
    #define EMIT_PUSH_PAREMTER_5      EMIT_CONST_ARRAY(dseg_push_parameter_5)
    
    #define EMIT_REMAP                EMIT_CONST_ARRAY(dseg_remap_first_four)
    
    #define EMIT_ALLOC_SHADOWSPACE    EMIT_CONST_ARRAY(dseg_allocate_shadow_space) 
    #define EMIT_NUKE_SHADOWSPACE     EMIT_CONST_ARRAY(dseg_nuke_shadow_space) 
    
    #define EMIT_PUSH(n)              {EMIT_CONST_ARRAY(dseg_push_n)            *(uint64_t *)CALC_OFFSET(2, sizeof(dseg_push_n))             = n; } 
    
    #define EMIT_NUKE_STACK(n)        {EMIT_CONST_ARRAY(dseg_nuke_stack_n)      *(uint8_t  *)CALC_OFFSET(3, sizeof(dseg_nuke_stack_n))       = n; } 
    
    #define EMIT_CALL(addr)           {EMIT_CONST_ARRAY(dseg_call_addr)         *(void      **)CALC_OFFSET(2, sizeof(dseg_call_addr))        = addr; } 

    EMIT_INIT
    EMIT_PUSH_REGS
    
    if ((!aligned && (parameters % 2 == 0)) || (aligned && (parameters % 2 != 0)))
    {
        // EMIT_INIT and EMIT_PUSH_REGS pushes an odd amount of registers on to the stack - do nothing
        // If that changes to an even amount, copy the latter else scope into here.
    }
    else
    {
        needs_algn_pop = true;
        EMIT_PUSH(0);
    }

    EMIT_PUSH((size_t)msft_data)
    EMIT_PUSH((size_t)SYSV_MAGIC)

    if (parameters > 6)
        for (uint8_t i = parameters; i > 6; i--)
            EMIT_PUSH_PAREMTER((i - 7) * 8);

    if (parameters >= 6)
        EMIT_PUSH_PAREMTER_6

    if (parameters >= 5)
        EMIT_PUSH_PAREMTER_5
    
    EMIT_REMAP
    EMIT_ALLOC_SHADOWSPACE
    EMIT_CALL(msft_func)
    EMIT_NUKE_SHADOWSPACE
    EMIT_NUKE_STACK(((2 + needs_algn_pop) * 8) + (parameters > 4 ? ((parameters - 4) * 8) : 0))

    EMIT_POP_REGS
    EMIT_END

    if (index > max_len)
        panicf("Buffer overflow detected in dynamic callback stub! (%lli/%lli)", index, max_len);

    #undef CALC_OFFSET
    #undef EMIT_CONST_ARRAY
    #undef EMIT
    #undef EMIT_INIT
    #undef EMIT_END
    #undef EMIT_PUSH_REGS
    #undef EMIT_POP_REGS
    #undef EMIT_PUSH_PAREMTER
    #undef EMIT_PUSH_PAREMTER_6    
    #undef EMIT_PUSH_PAREMTER_5
    #undef EMIT_REMAP
    #undef EMIT_ALLOC_SHADOWSPACE
    #undef EMIT_NUKE_SHADOWSPACE
    #undef EMIT_PUSH    
    #undef EMIT_NUKE_STACK
    #undef EMIT_CALL
}

static void _dyncb_alloc_selector_stub(void * aligned, void * unaligned, void * sys_v, uint64_t max_len)
{
    size_t index;
    index = 0;

    #define CALC_OFFSET(off, length)       ((void *)(((size_t)(sys_v)) + index - length + off))
    #define EMIT(buffer, length)    {memcpy((void *)(((size_t)(sys_v)) + index), buffer, length); index += length;}
    #define EMIT_CONST_ARRAY(arr)    EMIT(arr, sizeof(arr))
    
    
    #define EMIT_INIT                 EMIT_CONST_ARRAY(selector_init_rax)
    #define EMIT_AND                  EMIT_CONST_ARRAY(selector_and_rax)
    #define EMIT_CMP                  EMIT_CONST_ARRAY(selector_cmp)
    #define EMIT_JE(n)               {EMIT_CONST_ARRAY(selector_jmp_to_true_n) *(uint8_t *)CALC_OFFSET(1, sizeof(selector_jmp_to_true_n)) = n; } 
    #define EMIT_JMP(n)              {EMIT_CONST_ARRAY(selector_call_n)            *(void **)CALC_OFFSET(2, sizeof(selector_call_n)) = n; } 

    EMIT_INIT
    EMIT_AND
    EMIT_CMP
    EMIT_JE(sizeof(selector_call_n)) // goto aligned
    EMIT_JMP(unaligned)
    // aligned:
    EMIT_JMP(aligned)
        
    if (index > max_len)
        panicf("Buffer overflow detected in selector callback stub! (%lli/%lli)", index, max_len);

    #undef CALC_OFFSET
    #undef EMIT_CONST_ARRAY
    #undef EMIT
    #undef EMIT_INIT
    #undef EMIT_AND
    #undef EMIT_CMP
    #undef EMIT_JE
    #undef EMIT_JMP
}

error_t dyncb_allocate_stub(void * msft, uint8_t parameters, void * data, sysv_fptr_t * out, void ** handle)
{
    size_t translator_len, selector_length;
    dyncb_struct_p dyncb;

    translator_len  = _dyncb_calc_translator_size(parameters);
    selector_length = _dyncb_calc_selector_size();

    if (!out)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!msft)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    dyncb = (dyncb_struct_p)malloc(sizeof(dyncb_struct_t));

    if (!(dyncb->stub_aligned = execalloc(translator_len)))
    {
        free(dyncb);
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    if (!(dyncb->stub_unaligned = execalloc(translator_len)))
    {
        execfree(dyncb->stub_aligned);
        free(dyncb);
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    if (!(dyncb->selector = execalloc(selector_length)))
    {
        execfree(dyncb->stub_aligned);
        execfree(dyncb->stub_unaligned);
        free(dyncb);
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    if (parameters < 4)
        return XENUS_ERROR_NOT_IMPLEMENTED; //TOOD: i haven't implemented mov magic + data into registers yet. we just push them onto the stack for now.
                                             // SysV and MSFT x64 allow for varags (not _va_struct_) by default. if you need to do something with less than 4 parameters, just lie.
                                             // This is just an artificial error so nobody makes any dumb mistakes (wanting parameter 1, 2, 3, and/or 4 to host [magic, data]) 

    _dyncb_alloc_translator_stub(true,  parameters, data, msft, dyncb->stub_aligned,   translator_len);
    _dyncb_alloc_translator_stub(false, parameters, data, msft, dyncb->stub_unaligned, translator_len);

    _dyncb_alloc_selector_stub(dyncb->stub_aligned, dyncb->stub_unaligned, dyncb->selector, selector_length);

    *out    = dyncb->selector;
    *handle = dyncb;
    return XENUS_OKAY;
}


error_t dyncb_free_stub(void * handle)
{
    dyncb_struct_p dyncb;
    dyncb = handle;
    execfree(dyncb->selector);
    execfree(dyncb->stub_unaligned);
    execfree(dyncb->stub_aligned);
    free(dyncb);
    return XENUS_OKAY;
}