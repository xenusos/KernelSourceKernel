/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
    Notes: TODO: look over every strlen, memcpy, and memset.
           a lot of work needs to be done here
*/
#include <xenus.h>

#include "../Boot/access_system.h"

#include <kernel/peloader/pe_fmt.h>
#include "pe_loader.h"
#include "pe_checker.h"

#include <kernel/libx/xenus_list_linked.h>
#include <kernel/libx/xenus_memory.h>
#include <kernel/libx/xenus_strings.h>

typedef struct
{
    char symbol[PE_MAX_SYMBOL_LENGTH];
    void * data;
} sym_redir_t, *sym_redir_p;

typedef struct
{
    char from[PE_MAX_SYMBOL_LENGTH];
    char to[PE_MAX_SYMBOL_LENGTH];
} mod_redir_t, *mod_redir_p;

typedef struct
{
    bool has_loaded;
    struct
    {
        void * buffer;
        size_t length;
    } not_loaded;
    struct
    {
        union
        {
            PDOS_HEADER dos;
            void * base;
        };
        PIMAGE_NT_HEADERS nt;
        PIMAGE_OPTIONAL_HEADER64 opt;
        PIMAGE_SECTION_HEADER sections;
        int64_t difference;
        bool reloc;
        linked_list_head_p sym_redir; //sym_redir_t
    } loaded;
    char name[PE_MAX_MODULE_LENGTH];
} module_t, *module_p;

static linked_list_head_p module_redir;
static linked_list_head_p module_list = 0;
static void * module_mutex;
static module_t kern_mod;

void pe_loader_init_kernmod()
{
    PDOS_HEADER dheader;
    PIMAGE_NT_HEADERS ntheader;
    PIMAGE_SECTION_HEADER sections;

    dheader  = (PDOS_HEADER)            kernel_base;
    ntheader = (PIMAGE_NT_HEADERS)      (((size_t)kernel_base) + dheader->e_lfanew);
    sections = (PIMAGE_SECTION_HEADER)  (((size_t)(&ntheader->OptionalHeader64)) + ntheader->FileHeader.SizeOfOptionalHeader);

    kern_mod.has_loaded         = true;
    kern_mod.loaded.reloc       = true;
    kern_mod.loaded.difference  = -1;
    kern_mod.loaded.dos         = dheader;
    kern_mod.loaded.nt          = ntheader;
    kern_mod.loaded.opt         = &ntheader->OptionalHeader64;
    kern_mod.loaded.sections    = sections;
    kern_mod.loaded.sym_redir   = NULL;
}

error_t pe_loader_init()
{
    module_list = linked_list_create();

    if (!module_list)
        return XENUS_ERROR_COULDNT_ALLOCATE_HANDLE_LIST;
    
    module_redir = linked_list_create();

    if (!module_redir)
        return XENUS_ERROR_COULDNT_ALLOCATE_REDIR_LIST;

    module_mutex = mutex_init();

    if (!module_mutex)
        return XENUS_ERROR_INTERNAL_ERROR;

    pe_loader_init_kernmod();

    return XENUS_OKAY;
}

error_t pe_loader_preload_init(const void * buffer, size_t length, const char * name, pe_handle_h * handle)
{
    module_p element;
    void * saved_pe;
    size_t mod_len;
    linked_list_entry_p entry;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!buffer)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    mod_len = strnlen(name, PE_MAX_MODULE_LENGTH);

    if (mod_len == 0)
        return XENUS_ERROR_MODULE_NAME_TOO_LONG;

    if (mod_len == PE_MAX_MODULE_LENGTH)
        return XENUS_ERROR_MODULE_NAME_TOO_LONG;

    entry = linked_list_append(module_list, sizeof(module_t));

    if (!entry)
        return XENUS_ERROR_COULDNT_ALLOCATE_HANDLE;

    element = (module_p)entry->data;
    
    saved_pe = malloc(length);

    if (!saved_pe)
    {
        linked_list_remove(entry);
        return XENUS_ERROR_OUT_OF_MEMORY;
    }

    element->has_loaded        = false;
    element->not_loaded.buffer = saved_pe;
    element->not_loaded.length = length;

    memcpy(element->name, name, mod_len);

    memcpy(saved_pe, buffer, length);

    *handle = element;
    return XENUS_OKAY;
}

error_t pe_loader_preload_analyse(pe_handle_h handle)
{
    module_p element;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    element = (module_p)handle;

    if (element->has_loaded)
        return XENUS_ERROR_MODULE_ALREADY_LOADED;

    return pe_loader_check(element->not_loaded.buffer, element->not_loaded.length, 10000);
}

error_t pe_loader_preload_load(pe_handle_h handle)
{
    void * base;
    int64_t differ;
    module_p element;
    PDOS_HEADER dheader;
    void * buffer_of_peimg;
    size_t length_of_peimg;
    linked_list_head_p redir;
    PIMAGE_NT_HEADERS ntheader;
    PIMAGE_SECTION_HEADER sections;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    element = (module_p)handle;

    if (element->has_loaded)
        return XENUS_ERROR_MODULE_ALREADY_LOADED;

    redir = linked_list_create();

    if (!redir)
        return XENUS_ERROR_OUT_OF_MEMORY;

    buffer_of_peimg = element->not_loaded.buffer;
    length_of_peimg = element->not_loaded.length;

    dheader     = (PDOS_HEADER)buffer_of_peimg;
    ntheader    = (PIMAGE_NT_HEADERS)((size_t)buffer_of_peimg + dheader->e_lfanew);

    base = execalloc(ntheader->OptionalHeader64.SizeOfImage);
    memset(base, 0, ntheader->OptionalHeader64.SizeOfImage); 

    if (!base)
        return XENUS_ERROR_OUT_OF_MEMORY;

    differ   = (size_t)base - (size_t)ntheader->OptionalHeader64.ImageBase;

    memcpy(base, buffer_of_peimg, ntheader->OptionalHeader64.SizeOfHeaders);

    dheader  = (PDOS_HEADER)base;
    ntheader = (PIMAGE_NT_HEADERS)((size_t)base + dheader->e_lfanew);
    sections = (PIMAGE_SECTION_HEADER)(((size_t)(&ntheader->OptionalHeader64)) + ntheader->FileHeader.SizeOfOptionalHeader);

    element->has_loaded         = true;

    element->loaded.base        = base;
    element->loaded.sections    = sections;
    element->loaded.nt          = ntheader;
    element->loaded.opt         = &ntheader->OptionalHeader64;
    element->loaded.difference  = differ;
    element->loaded.reloc       = false;
    element->loaded.sym_redir   = redir;


    for (uint16_t x = 0; x < ntheader->FileHeader.NumberOfSections; x++)
        memcpy((void *)((size_t)base + sections[x].VirtualAddress),
                (void *)((size_t)buffer_of_peimg + sections[x].PointerToRawData),
                sections[x].SizeOfRawData); //TODO: check length in pe_checker.c

    free(buffer_of_peimg);
    return XENUS_OKAY;
}

error_t pe_loader_load(const void * buffer, size_t length, const char * name, pe_handle_h* handle)
{
    error_t ret;
    void * hd;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (ERROR(ret = pe_loader_preload_init(buffer, length, name, &hd)))
        return ret;

    if (ERROR(ret = pe_loader_preload_analyse(hd))) 
        return ret;

    if (ERROR(ret = pe_loader_preload_load(hd))) 
        return ret;

    *handle = hd;
    return XENUS_OKAY;
}

const char * pe_loader_sanitize_name(const char * mod)
{
    const char * ret = mod;

    for (linked_list_entry_p entry = module_redir->bottom; entry; entry = entry->next)
    {
        mod_redir_p redir;
        redir = (mod_redir_p)entry->data;

        if (strcmp(redir->from, mod) == 0)
        {
            ret = redir->to;
            break;
        }
    }

    return ret;
}

pe_handle_h pe_loader_find_module(const char * name)
{
    name = pe_loader_sanitize_name(name);

    if (strcmp(name, "Kernel.xdll") == 0)
        return &kern_mod;

    for (linked_list_entry_p entry = module_list->bottom; entry; entry = entry->next)
    {
        module_p mod;
        mod = (module_p)entry->data;
        
        if (strcmp(mod->name, name) == 0)
            return mod;
    }

    return NULL;
}

#define RVA(type, off) (type)(((size_t)(base)) + off) 

error_t pe_loader_find_symbol_bymodhandle(pe_handle_h mod_, uint16_t ord, const char * name, bool byordinal, void ** function_ptr)
{
    module_p mod;
    PIMAGE_NT_HEADERS ntheader;
    IMAGE_DATA_DIRECTORY dir;
    PIMAGE_EXPORT_DIRECTORY pe_exports;
    uint32_t * names;
    uint32_t * functions;
    uint16_t * ords;
    void * func;
    void * base;

    mod = mod_;

    if (!mod_)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!function_ptr) 
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name && !byordinal) 
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!mod->has_loaded)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    ntheader     = mod->loaded.nt;
    dir          = ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    if (ntheader->OptionalHeader64.NumberOfRvaAndSizes < 2) 
        return XENUS_ERROR_PE_DATA_DIR_NOT_PRESENT;

    if (dir.Size == 0) 
        return XENUS_ERROR_PE_DATA_DIR_NOT_PRESENT;

    base         = mod->loaded.base;

    pe_exports   = RVA(PIMAGE_EXPORT_DIRECTORY, dir.VirtualAddress);
    names        = RVA(uint32_t *, pe_exports->AddressOfNames);
    functions    = RVA(uint32_t *, pe_exports->AddressOfFunctions);
    ords         = RVA(uint16_t *, pe_exports->AddressOfNameOrdinals);

    if (byordinal)
    {
        if (ord >= pe_exports->NumberOfFunctions)
            return XENUS_ERROR_GENERIC_FAILURE;
        func = RVA(void *, functions[ord]);
        goto exit;
    }

    if (mod->loaded.sym_redir)
    {
        for (linked_list_entry_p entry = mod->loaded.sym_redir->bottom; entry != NULL; entry = entry->next)
        {
            sym_redir_p dir;
            dir = (sym_redir_p)entry->data;

            if (strcmp(name, dir->symbol) != 0)
                continue;

            func = dir->data;
            goto exit;
        }
    }

    for (size_t i = 0; i < pe_exports->NumberOfNames; i++)
    {
        if (strcmp(name, RVA(const char *, names[i])) != 0)
            continue;
    
        func = RVA(void *, functions[ords[i]]);
        goto exit;
    }

exit:
    *function_ptr = func;
    return XENUS_OKAY;
}

error_t pe_loader_find_symbol_bymodname(const char * module, uint16_t ord, const char * name, bool byordinal, void ** function_ptr)
{
    error_t err;
    module_p mod;
    size_t len;
    void * func;

    if (!module) 
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!function_ptr) 
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name && !byordinal)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name)
        name = "NULL_SYMBOL";

    len = strnlen(module, PE_MAX_SYMBOL_LENGTH);

    if ((len == PE_MAX_SYMBOL_LENGTH) || (len == 0))
        return XENUS_ERROR_ILLEGAL_BUF_LENGTH;

    mod = pe_loader_find_module(module);

    if (!mod)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    if (ERROR(err = pe_loader_find_symbol_bymodhandle(mod, ord, name, byordinal, &func)))
        return err;

    if (!func)
        return XENUS_ERROR_SYM_NOT_FOUND;

    *function_ptr = func;
    return XENUS_OKAY;
}

error_t pe_loader_alias(const char * target, const char * dest)
{
    size_t len_target;
    size_t len_dest;
    mod_redir_p redir;
    linked_list_entry_p entry;

    if (!target)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!dest)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    len_target = strnlen(target, PE_MAX_SYMBOL_LENGTH);
    len_dest   = strnlen(dest, PE_MAX_SYMBOL_LENGTH);

    if (len_dest == PE_MAX_SYMBOL_LENGTH)
        return XENUS_ERROR_ILLEGAL_BUF_LENGTH;

    if (len_dest == 0)
        return XENUS_ERROR_ILLEGAL_BUF_LENGTH;

    if (len_target == PE_MAX_SYMBOL_LENGTH)
        return XENUS_ERROR_ILLEGAL_BUF_LENGTH;

    if (len_target == 0)
        return XENUS_ERROR_ILLEGAL_BUF_LENGTH;

    entry = linked_list_append(module_redir, sizeof(mod_redir_t));
    
    if (!entry)
        return XENUS_ERROR_OUT_OF_MEMORY;

    redir = (mod_redir_p) entry->data;

    memcpy(redir->from, target, len_target + 1);
    memcpy(redir->to, dest, len_dest + 1);
    return XENUS_OKAY;
}

error_t pe_loader_postload_iat_add_symbol_byname(pe_handle_h handle, const char * symbol, void * replacement)
{
    module_p element;
    linked_list_entry_p entry;
    sym_redir_p redir;
    size_t len;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!symbol)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!replacement)
    {
        puts("pe_loader_postload_iat_add_symbol_byname: replacement is null");
    }

    len = strlen(symbol) + 1;

    if (len > sizeof(redir->symbol))
        return XENUS_ERROR_ILLEGAL_BUF_LENGTH;

    element = (module_p)handle;

    if (!element->has_loaded)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    entry = linked_list_append(element->loaded.sym_redir, sizeof(sym_redir_t));

    if (!entry)
        return XENUS_ERROR_OUT_OF_MEMORY;

    redir = (sym_redir_p)entry->data;

    memcpy(redir->symbol, symbol, len);

    redir->data = replacement;
    return XENUS_OKAY;
}

error_t pe_loader_postload_get_iat_length(pe_handle_h handle, size_t * length)
{
    module_p element;
    PIMAGE_NT_HEADERS ntheader;
    PIMAGE_IMPORT_DESCRIPTOR imports;
    IMAGE_DATA_DIRECTORY dir;
    size_t base;
    size_t symbols;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!length)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    element  = (module_p) handle;

    if (!element->has_loaded)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    ntheader = element->loaded.nt;
    base     = (size_t)element->loaded.base;

    symbols  = 0;

    dir      = ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    imports  = RVA(PIMAGE_IMPORT_DESCRIPTOR, dir.VirtualAddress);

    if (!dir.Size) 
        return XENUS_STATUS_PE_DATA_DIR_NOT_PRESENT; //TODO (Reece): should we hard error this?

    for (PIMAGE_IMPORT_DESCRIPTOR cur_mod = imports; cur_mod->Name != 0; cur_mod++)
    {
        size_t * img_thnk_data;

        if (cur_mod->Characteristics != 0)
            img_thnk_data = RVA(size_t *, cur_mod->OriginalFirstThunk);
        else
            img_thnk_data = RVA(size_t *, cur_mod->FirstThunk);

        for (size_t i = 0; img_thnk_data[i] != 0; i++)
            symbols++;
    }

    *length = symbols;
    return XENUS_OKAY;
}

error_t pe_loader_postload_config_iat_new(pe_handle_h handle, iat_patch_result_ref reference)
{
    size_t base;
    module_p element;
    IMAGE_DATA_DIRECTORY dir;
    PIMAGE_NT_HEADERS ntheader;
    PIMAGE_IMPORT_DESCRIPTOR imports;
    size_t entry_index;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!reference)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    element = (module_p)handle;

    if (!element->has_loaded)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    reference->entries_total    = 0;
    reference->entries_appended = 0;

    entry_index = 0;

    ntheader    = element->loaded.nt;
    base        = (size_t)element->loaded.base;
                
    dir         = ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    imports     = RVA(PIMAGE_IMPORT_DESCRIPTOR, dir.VirtualAddress);

    if (!dir.Size)
        return XENUS_STATUS_PE_DATA_DIR_NOT_PRESENT;

    for (PIMAGE_IMPORT_DESCRIPTOR cur_mod = imports; /*moved into scope as if [...] break*/; cur_mod++)
    {
        const char * mod_name;
        size_t * iat_thunks;
        size_t * symbol_table;

        if (cur_mod->Name == 0)
            break;

        if ((size_t)cur_mod >= (dir.Size + (size_t)imports))
            break;

        if (cur_mod->Characteristics != 0)
            iat_thunks = RVA(size_t *, cur_mod->OriginalFirstThunk);
        else
            iat_thunks = RVA(size_t *, cur_mod->FirstThunk);

        symbol_table   = RVA(size_t *, cur_mod->FirstThunk);
        mod_name       = RVA(const char *, cur_mod->Name);

        for (size_t i = 0; iat_thunks[i] != 0; i++)
        {
            error_t                 error;
            void **                 symbol_reference;
            bool                    is_byord;
            uint16_t                entry_byord;
            PIMAGE_IMPORT_BY_NAME   entry_byname;
            iat_patch_entry_ref     entry;

            is_byord      = (iat_thunks[i] & IMAGE_ORDINAL_FLAG64) != 0;
            entry_byord  = IMAGE_ORDINAL64(iat_thunks[i]);
            entry_byname = RVA(PIMAGE_IMPORT_BY_NAME, iat_thunks[i]);

            symbol_reference = (void **)&symbol_table[i];

            *symbol_reference = NULL;

            error = pe_loader_find_symbol_bymodname(mod_name, entry_byord, entry_byname->Name, is_byord, symbol_reference); //if (ERROR()) removed. check list for errors. we sometimes allow bad iats

            if (reference->list_size <= entry_index)
            {
                if (ERROR(error))
                    return error;

                entry_index++;
                continue;
            }
            
            entry = &reference->list[entry_index];
            
            if (is_byord)
                entry->idx = entry_byord;
            else
                entry->symbol = entry_byname->Name;
            
            entry->is_str = !is_byord;
            entry->found  = (*symbol_reference) != NULL;
            entry->module = mod_name;
            entry->error  = error;

            entry_index++;
        }
    }

    reference->entries_total    = entry_index;
    reference->entries_appended = MIN(entry_index, reference->list_size);
    return XENUS_OKAY;
}

error_t pe_loader_postload_config_iat_legacy(pe_handle_h handle, size_t * unresolved_symbols, size_t * resolved_symbols)
{
    error_t err;
    size_t resolved;
    size_t unresolved;
    size_t cnt;
    iat_patch_result_t patch_result;
    
    err           = XENUS_OKAY;
    unresolved    = 0;
    resolved      = 0;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (ERROR(err = pe_loader_postload_get_iat_length(handle, &cnt)))
        return err;
    
    patch_result.list_size    = cnt;
    patch_result.list        = calloc(cnt, sizeof(iat_patch_entry_t));

    if (!patch_result.list)
        return XENUS_ERROR_OUT_OF_MEMORY;

    if (ERROR(err = pe_loader_postload_config_iat_new(handle, &patch_result))) 
        goto exit;

    for (size_t i = 0; i < patch_result.entries_appended; i++)
    {
        if (patch_result.list[i].found)
            resolved++;
        else
            unresolved++;
    }

    if (unresolved_symbols)
        *unresolved_symbols = unresolved;

    if (resolved_symbols)
        *resolved_symbols    = resolved;

    exit:
    free(patch_result.list);
    return err;
}

error_t pe_loader_postload_config_reloc(pe_handle_h handle)
{
    module_p element;
    PIMAGE_NT_HEADERS ntheader;
    size_t difference;
    size_t base;
    IMAGE_DATA_DIRECTORY dir;
    IMAGE_BASE_RELOCATION *base_relocation;
    size_t bufidx;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    element = (module_p)handle;
    
    if (!element->has_loaded)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    ntheader    = element->loaded.nt;
    base        = (size_t) element->loaded.base;
    difference  = element->loaded.difference;

    dir         = ntheader->OptionalHeader64.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    
    if (!dir.Size) 
        return XENUS_OKAY;

    base_relocation = RVA(PIMAGE_BASE_RELOCATION, dir.VirtualAddress);

    bufidx = 0;
    while (base_relocation->SizeOfBlock) 
    {
        size_t items;
        uint16_t * map;
        size_t page_base;

        map          = (uint16_t *)(((size_t)(base_relocation)) + sizeof(IMAGE_BASE_RELOCATION));
        items        = (base_relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(IMAGE_RELOC);
        page_base    = RVA(size_t, base_relocation->VirtualAddress);

        for (size_t i = 0; i < items; i++) 
        {
            IMAGE_RELOC reloc = *(PIMAGE_RELOC)&map[i];
            switch (reloc.type) 
            {
            case IMAGE_REL_BASED_DIR64:
                *((size_t *)    (page_base + reloc.offset))      += difference;
                break;
            case IMAGE_REL_BASED_HIGHLOW:
                *((uint32_t *)    (page_base + reloc.offset))    += (uint32_t) difference;
                break;
            case IMAGE_REL_BASED_HIGH:
                *((uint16_t *)    (page_base + reloc.offset))    += HIWORD(difference);
                break;
            case IMAGE_REL_BASED_LOW:
                *((uint16_t *)    (page_base + reloc.offset))    += LOWORD(difference);
                break;
            case IMAGE_REL_BASED_ABSOLUTE:
                break;
            default:
                printf("Relocation type 0x%08x not yet implemented!\n", reloc.type);
                break;
            }
        }

        bufidx += base_relocation->SizeOfBlock;
        base_relocation = (PIMAGE_BASE_RELOCATION)(((size_t)base_relocation) + base_relocation->SizeOfBlock);

        if (bufidx == dir.Size)
            return XENUS_OKAY;
    }
    return XENUS_OKAY;
}

error_t pe_loader_get_headers(pe_handle_h handle, void ** dos, void ** nt, void ** opt, void ** segs)
{
    module_p element;

    if (!handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    element = (module_p)handle;

    if (!element->has_loaded)
        return XENUS_ERROR_MODULE_NOT_YET_LOADED;

    if (dos)
        *dos = element->loaded.dos;

    if (nt)
        *nt = element->loaded.nt;

    if (opt)
        *opt = element->loaded.opt;

    if (segs)
        *segs = element->loaded.sections;

    return XENUS_OKAY;
}

error_t pe_loader_iterate(void(* iterator)(pe_handle_h, void *), void * data)
{
    if (!iterator)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!data)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    for (linked_list_entry_p entry = module_list->bottom; entry != NULL; entry = entry->next)
        iterator((pe_handle_h)entry->data, data);
    
    return XENUS_OKAY;
}

void pe_loader_enter_critical()
{
    mutex_lock(module_mutex);
}

void pe_loader_leave_critical()
{
    mutex_unlock(module_mutex);
}
