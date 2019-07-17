/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson (See License.txt)
*/  
#include <xenus.h>
#include "../Boot/access_system.h"
#include <kernel/libx/xenus_libx.h>
#include "core_plugins.h"            
#include <kernel/peloader/pe_fmt.h>
#include <kernel/peloader/pe_loader.h>

typedef struct module_ctx_s
{
    uint32_t magic;
    char name[PE_MAX_MODULE_LENGTH];
    char module[PE_MAX_MODULE_LENGTH];
    char path[256];
    bool has_soft_deps;
    bool has_missing_dep_soft;  // use this for checking soft deps
    bool has_missing_dep_hard;  // use this for error checking
    bool has_missing_syms_soft; // use this for checking soft deps
    bool has_missing_syms_hard; // use this for error checking
    bool has_hard_error;
    error_t hard_error;
    bool has_init_failed;
    xenus_entrypoint_ctx_t ep;
    pe_handle_h handle;
    bool iat_patched;
} *module_ctx_p, module_ctx_t;

static error_t xenus_load_module(const char * friendly_name, const char * mod, const  char * path, pe_handle_h * pe_handle)
{
    error_t err;
    size_t fs;
    int_t fread;
    void * buf;
    void * file_hd;
    void * pe_hd;

    buf = 0;
    file_hd = 0;

    if (!pe_handle)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!mod)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!path)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    printf("Loading module %s (%s) from %s \n",
            friendly_name, 
            mod, 
            path);

    file_hd = file_open_readonly(path);
    if (!(file_hd))
        return XENUS_ERROR_NTOS_NOT_FOUND;

    fs = file_length(path);
    if (!(fs))                
        return XENUS_ERROR_NTOS_RD_ERR;

    buf = malloc(fs);
    if (!(buf))                    
        return XENUS_ERROR_OUT_OF_MEMORY;

    fread = file_read(file_hd, 0, buf, fs);

    if (fread != (int_t)fs && fread > 0)
        return XENUS_ERROR_NTOS_RD_ERR;

    err = pe_loader_load(buf, fs, mod, &pe_hd);
    if (ERROR(err))
    {
        printf("%s failed to load: %i \n", 
            friendly_name,
            (int) err);
        goto err_s;
    }

    err = pe_loader_postload_config_reloc(pe_hd);
    if (ERROR(err))
    {
        printf("%s failed to relocate: %i \n",
            friendly_name, 
            (int) err);
        goto err_s;
    }

    *pe_handle    = pe_hd;
    err            = XENUS_OKAY;

err_s:
    if (file_hd)    file_close(file_hd);
    if (buf)        free(buf);

    return err;
}

static error_t _plugins_load_call_ep(pe_handle_h module, xenus_entrypoint_ctx_t * data)
{
    error_t err;
    void * base;
    PIMAGE_NT_HEADERS headers;
    uint32_t ep_addr;
    entrypoint_t ep_fptr;

    if (!module)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!data)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    err = pe_loader_get_headers(module, &base, &headers, NULL, NULL);
    if (ERROR(err))
        return err;

    ep_addr = headers->OptionalHeader64.AddressOfEntryPoint;

    if (!ep_addr)
        return XENUS_STATUS_PE_OKAY_MISSING_OPT_ENTRYPOINT;

    ep_fptr = ((entrypoint_t)(((size_t)base) + ep_addr));
    ep_fptr(data);

    return XENUS_OKAY;
}

static module_ctx_p _plugins_get_handle(linked_list_head_p list, const char * module)
{
    for (linked_list_entry_t * item = list->bottom; item != NULL; item = item->next)
    {
        module_ctx_p data = (module_ctx_p)item->data;
        if (strcmp(data->module, module) == 0)
            return data;
    }
    return NULL;
}

static error_t plugins_iterative_load(linked_list_head_p list, const char * mod, const char * name, const char * path, bool soft_dep, pe_handle_h * pe_handle_out, plugin_handle_h* plugin_handle_out)
{
    error_t ret;
    module_ctx_p plugin_module;
    linked_list_entry_p list_entry;
    xenus_entrypoint_ctx_p entrypoint_data;
    pe_handle_h pe_module;

    if (!list)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!mod)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!path)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    list_entry = linked_list_append(list, sizeof(module_ctx_t));

    if (!list_entry)
        return XENUS_ERROR_OUT_OF_MEMORY;

    plugin_module = (module_ctx_p) list_entry->data;

    memset(plugin_module, 0, sizeof(module_ctx_t));

    entrypoint_data = &plugin_module->ep;

    plugin_module->magic = XENUS_HASHCODE_PLUGIN;
    
    memcpy(plugin_module->module, mod,  MIN(strlen(mod),  sizeof(plugin_module->module) - 1));
    memcpy(plugin_module->name,   name, MIN(strlen(name), sizeof(plugin_module->name) - 1));
    memcpy(plugin_module->path,   path, MIN(strlen(path), sizeof(plugin_module->path) - 1));

    if (ERROR(ret = xenus_load_module(name, mod, path, &pe_module)))
    {
        printf("Couldn't load module: %s. This could be bad if such module isn't a soft-dependency! %lli\n ", name, (int64_t)ret);

        if (!soft_dep)
            return ret;
        else
            return XENUS_OKAY;
    }

    plugin_module->handle = pe_module;

    if (ERROR(ret = _plugins_load_call_ep(pe_module, entrypoint_data)))
        return ret;

    if (entrypoint_data->size != sizeof(xenus_entrypoint_ctx_t))
        return XENUS_ERROR_PLUGIN_BAD_MOD_INFO;

    if (!entrypoint_data->static_data)
        return XENUS_ERROR_PLUGIN_BAD_MOD_INFO;

    if (!entrypoint_data->description)    
        entrypoint_data->description    = "Null Description";

    if (!entrypoint_data->copyright)    
        entrypoint_data->copyright      = "All rights reserved";


    entrypoint_data->static_data->pe_handle        = pe_module;
    entrypoint_data->static_data->plugin_handle = (plugin_handle_h)plugin_module;

    for (uint32_t i = 0; i < entrypoint_data->dependencies.count; i++)
    {
        module_ctx_p dependency;
        mod_dependency_opt_p dep_data;

        dep_data = &entrypoint_data->dependencies.dependencies[i];

        if (!dep_data->name)        
            return XENUS_ERROR_PLUGIN_BAD_MOD_INFO;

        if (!dep_data->module)        
            return XENUS_ERROR_PLUGIN_BAD_MOD_INFO;

        if (!dep_data->path)        
            return XENUS_ERROR_PLUGIN_BAD_MOD_INFO;

        dep_data->present = true;

        if (dep_data->soft_dependency)
            plugin_module->has_soft_deps = true;

        dependency = _plugins_get_handle(list, dep_data->module);
        if (dependency)
        {
            dep_data->pe_handle        = dependency->ep.static_data->pe_handle;
            dep_data->plugin_handle    = dependency->ep.static_data->plugin_handle;
            continue;
        }

        ret = plugins_iterative_load(list, dep_data->module, dep_data->name, dep_data->path, dep_data->soft_dependency, &dep_data->pe_handle, &dep_data->plugin_handle);
        if (ERROR(ret))
        {
            if (dep_data->soft_dependency)
                plugin_module->has_missing_dep_soft = true;
            else
                plugin_module->has_missing_dep_hard = true;
        }
    }

    if (pe_handle_out)
        *pe_handle_out        = pe_module;

    if (plugin_handle_out)
        *plugin_handle_out    = (plugin_handle_h)plugin_module;

    return XENUS_OKAY;
}

static error_t _plugins_init_iat(module_ctx_p mod_data, size_t * o_resolved, size_t * o_unresolved, size_t * o_percent)
{
    error_t err;
    pe_handle_h handle;
    iat_patch_result_t iat_state = { 0 };
    size_t resolved = 0, unresolved = 0, bad_unresolved = 0;

    handle = mod_data->handle;
    err = XENUS_OKAY;

    err = pe_loader_postload_get_iat_length(handle, &iat_state.list_size);
    if (ERROR(err))
    {
        printf("Failed to enumerate iat for module %s \n", mod_data->name);
        goto err;
    }

    iat_state.list = calloc(iat_state.list_size, sizeof(iat_patch_entry_t));
    if (!(iat_state.list))
    {
        err = XENUS_ERROR_OUT_OF_MEMORY;
        goto err;
    }

    err = pe_loader_postload_config_iat_new(handle, &iat_state);
    if (ERROR(err))
    {
        printf("Failed to patch module: %s. This shouldn't be possible! \n", mod_data->name);
        goto err;
    }

    for (size_t i = 0; i < iat_state.entries_appended; i++)                //iterate over iat patches
    {
        bool mod_found;
        iat_patch_entry_p entry;
        
        entry = &iat_state.list[i];

        if (entry->found)
        {
            resolved++;
            continue;
        }

        unresolved++;

        // this shit is ugly
        mod_found = false;
        for (size_t z = 0; z < mod_data->ep.dependencies.count && !mod_found; z++)        // iterate over dependencies
        {
            mod_dependency_opt_p requested_dep;

            requested_dep = &mod_data->ep.dependencies.dependencies[i];

            if (strcmp(requested_dep->module, entry->module) != 0)         // find module by iat patch
                continue;

            mod_found = true;
            if (!requested_dep->soft_dependency)                           // if it's a hard dependency, fail. 
            {
                mod_data->has_missing_syms_hard = true;
                bad_unresolved++;
                break;
            }

            // it's a softdependency
            if (entry->error == XENUS_ERROR_SYM_NOT_FOUND) // from pe_loader_find_symbol_bymodname
            {
                // and it's missing a symbol. this is bad
                printf("Hard-faulting %s... soft-dependency has missing symbol... bad version of %s?", mod_data->name, requested_dep->name);
                mod_data->has_hard_error = true;
                mod_data->hard_error = XENUS_ERROR_SYM_NOT_FOUND;
                bad_unresolved++;
            } 
            else if (entry->error == XENUS_ERROR_MODULE_NOT_YET_LOADED) // from pe_loader_find_symbol_bymodname
            {
                // it's missing the module... we can ignore this
                printf("Ignoring IAT error for softdepedency. Module present: %b, Module name %s, Error code: %zx\n", requested_dep->present, requested_dep->name, entry->error);
                mod_data->has_missing_syms_soft = true;
            }
            else
            {
                // huh?
                printf("Not hardfauling, but, while loading %s we encountered %xu... Why? Assume okay.", requested_dep->name, entry->error);
            }
        }


        if (!mod_found)
        {
            printf("ERROR: Attempted to patch bad IAT. Reference to non-existent symbol in non-existent module. %s was not found in %s (error: 0x%zx)\n", entry->symbol, entry->module, entry->error);
            mod_data->has_hard_error = true;
            mod_data->hard_error = XENUS_ERROR_PLUGIN_BAD_IAT;
        }
    }

    *o_resolved        = resolved;
    *o_unresolved      = unresolved;
    *o_percent         = (MAX(1, iat_state.entries_appended) / MAX(1, (iat_state.entries_appended - bad_unresolved))) * 100;

    free(iat_state.list);
    return err;

err: 
    if (iat_state.list) 
        free(iat_state.list);
    
    mod_data->has_hard_error = true;
    mod_data->hard_error     = err;
    return err;
}

static error_t _plugins_tryinit_iat(module_ctx_p mod_data, size_t * o_resolved, size_t * o_unresolved, size_t * o_percent)
{
    error_t ret;

    if (mod_data->iat_patched)
        return true;

    ret = _plugins_init_iat(mod_data, o_resolved, o_unresolved, o_percent);

    mod_data->iat_patched = ((STRICTLY_OKAY(ret)) && (*o_unresolved == 0));
    return ret;
}

static error_t _plugins_exec_init(module_ctx_p handle)
{
    error_t er;
    c_bool failed;

    er = plugins_get_state(handle);
    if (ERROR(er))
        return er;

    if (handle->ep.static_data->ep_called)
        return XENUS_OKAY;

    if (handle->ep.init)
        failed = !handle->ep.init(&handle->ep.dependencies);
    else
        failed = false;

    handle->has_init_failed           = failed;
    handle->ep.static_data->ep_called = true;

    return XENUS_OKAY;
}


static error_t _plugins_exec_start(module_ctx_p mod_data)
{
    int status;
    error_t er;

    er = plugins_get_state((plugin_handle_h)mod_data);
    if (ERROR(er))
        return er;

    if (mod_data->ep.static_data->has_started)
        return XENUS_OKAY;

    if (mod_data->ep.start)
        status = mod_data->ep.start();
    else
        status = 0;

    mod_data->ep.static_data->has_started = true;
    mod_data->ep.static_data->status_code = status;

    return XENUS_OKAY;
}

static void _plugins_init_pl_iat_patchers(linked_list_head_p list)
{
    error_t err;

    printf("Loading IAT-adjusted plugins...");

    for (linked_list_entry_t * item = list->bottom; item != NULL; item = item->next)
    {
        size_t resolved = 0, unresolved = 0, percent = 0;
        module_ctx_p mod_data;

        mod_data = (module_ctx_p)item->data;

        if (!mod_data->ep.iat_hook)
            continue;

        err = _plugins_tryinit_iat(mod_data, &resolved, &unresolved, &percent);
        if (ERROR(err))
        {
            printf(" %s IAT ERROR\n", mod_data->name);
            mod_data->has_hard_error = true;
            return;
        }

        printf(" %-15s (preloaded symbols: %lli/%lli, total: %lli%%)\n", mod_data->name, resolved, unresolved + resolved, percent);
        mod_data->ep.iat_hook();
    }
}

static void _plugins_init_pl_iat(linked_list_head_p list)
{
    error_t err;

    printf("Fixing up plugins import tables...");

    for (linked_list_entry_t * item = list->bottom; item != NULL; item = item->next)
    {
        module_ctx_p mod_data;
        size_t resolved = 0, unresolved = 0, percent = 0;

        mod_data = (module_ctx_p)item->data;

        err = _plugins_tryinit_iat(mod_data, &resolved, &unresolved, &percent);
        if (ERROR(err))
        {
            printf(" %s IAT ERROR\n", mod_data->name);
            mod_data->has_hard_error = true;
            return;
        }

        printf(" %-15s (symbols: %lli/%lli, total: %lli%%)\n", mod_data->name, resolved, unresolved + resolved, percent);
    }
}

static void _plugins_init_pl_init(linked_list_head_p list)
{
    printf("Signaling plugins to initialize...");

    for (linked_list_entry_t * item = list->bottom; item != NULL; item = item->next)
    {
        error_t err;
        module_ctx_p mod_data;
        mod_data = (module_ctx_p)item->data;

        if (ERROR(err = _plugins_exec_init(mod_data)))
        {
            printf(" %-15s, failed to init %zx\n", mod_data->name, err);
        }
        else
        {
            printf(" %s \n", mod_data->name);
        }
    }
}

static void _plugins_init_pl_start(linked_list_head_p list)
{
    error_t err;

    printf("Passing execution to plugins...");

    for (linked_list_entry_t * item = list->bottom; item != NULL; item = item->next)
    {
        module_ctx_p mod_data;

        mod_data = (module_ctx_p)item->data;

        err = _plugins_exec_start(mod_data);
        if (ERROR(err))
        {
            printf(" %-15s, failed to start %zx\n", mod_data->name, err);
        }
    }
}

XENUS_SYM void plugins_initialize(linked_list_head_p list)
{
    _plugins_init_pl_iat_patchers(list);
    _plugins_init_pl_iat(list);
    _plugins_init_pl_init(list);
}

XENUS_SYM void plugins_start(linked_list_head_p list)
{
    _plugins_init_pl_start(list);
}

XENUS_SYM error_t plugins_load(const char * name, const char * mod, const char * path, linked_list_head_p plugins, pe_handle_h* root_pe_handle, plugin_handle_h* root_plugin_handle)
{
    error_t err;

    if (!plugins)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!name)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!mod)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    if (!path)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    pe_loader_enter_critical();

    if (ERROR(err = plugins_iterative_load(plugins, mod, name, path, false, root_pe_handle, root_plugin_handle)))
    {
        pe_loader_leave_critical();
        return err;
    }

    plugins_initialize(plugins);
    plugins_start(plugins);

    pe_loader_leave_critical();
    return err;
}

#define PLUGIN_HEADER_GET_AND_CHK_n(mod, magic_error)  \
    module_ctx_p plugin;                               \
    plugin = (module_ctx_p) mod;                       \
    if (plugin->magic != XENUS_HASHCODE_PLUGIN)        \
        return magic_error;

XENUS_SYM error_t plugins_get_state(plugin_handle_h handle)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)

     if (plugin->has_init_failed)
         return XENUS_ERROR_PLUGIN_INIT_FAIL;

    if (plugin->has_hard_error)
        return XENUS_ERROR_PLUGIN_HARD_ERROR;

    if (plugin->has_missing_syms_hard)
        return XENUS_ERROR_PLUGIN_MISSING_HARD_SYMS;

    if (plugin->has_missing_dep_hard)
        return XENUS_ERROR_PLUGIN_MISSING_HARD_DEPS;

    return XENUS_OKAY;
}

XENUS_SYM error_t plugins_get_hard(plugin_handle_h handle)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)
    return plugin->hard_error;
}

XENUS_SYM error_t plugins_get_status(plugin_handle_h handle, size_t * status)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)

    if (!status)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    *status = plugin->ep.static_data->status_code;
    return XENUS_OKAY;
}

XENUS_SYM error_t plugins_has_init(plugin_handle_h handle, bool * init)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)

    if (!init)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    *init = plugin->ep.static_data->ep_called;
    return XENUS_OKAY;
}

XENUS_SYM error_t plugins_get_iat_module_name(plugin_handle_h handle, const char ** name)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)
        
    if (!name)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    *name = plugin->module;
    return XENUS_OKAY;
}

XENUS_SYM error_t plugins_get_friendly_name(plugin_handle_h handle, const char ** name)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)
        
    if (!name)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    *name = plugin->name;
    return XENUS_OKAY;
}

XENUS_SYM error_t plugins_get_path(plugin_handle_h handle, const char ** path)
{
    PLUGIN_HEADER_GET_AND_CHK_n(handle, XENUS_ERROR_ILLEGAL_TYPE)
        
    if (!path)
        return XENUS_ERROR_ILLEGAL_BAD_ARGUMENT;

    *path = plugin->path;
    return XENUS_OKAY;
}