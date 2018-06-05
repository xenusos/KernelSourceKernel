/*
    Purpose: 
    Author: Reece W. 
    License: All Rights Reserved J. Reece Wilson
*/  
#include <xenus.h>
#include "sys_main.h"
#include "../core_exports.h"
//#include "../access_sys.h"

size_t wrap_msft_sysv_12(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11, size_t a_12, system_v_function_t function);

size_t wrap_msft_sysv_11(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11, system_v_function_t function);

size_t wrap_msft_sysv_10(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, size_t a_9, size_t a_10, system_v_function_t function);

size_t wrap_msft_sysv_9(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, size_t a_9, system_v_function_t function);

size_t wrap_msft_sysv_8(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, system_v_function_t function);

size_t wrap_msft_sysv_7(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, system_v_function_t function);

size_t wrap_msft_sysv_6(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,system_v_function_t function);

size_t wrap_msft_sysv_5(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, system_v_function_t function);

size_t wrap_msft_sysv_4(size_t a_1, size_t a_2, size_t a_3, size_t a_4, system_v_function_t function);

size_t wrap_msft_sysv_3(size_t a_1, size_t a_2, size_t a_3, system_v_function_t function);

size_t wrap_msft_sysv_2(size_t a_1, size_t a_2, system_v_function_t function);

size_t wrap_msft_sysv_1(size_t a_1, system_v_function_t function);

XENUS_SYM size_t linux_caller(system_v_function_t function, size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11, size_t a_12)
{
	return wrap_msft_sysv_12(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, a_12, function);
}

#define ADD_SYMBOL_B(name)																							\
	name ## _function = (void *)kallsyms_lookup_name((size_t)(#name));

#define ADD_SYMBOL_12_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, 				\
				 size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11, size_t a_12)							\
	{																												\
		return wrap_msft_sysv_12(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, a_12, name ## _function);	\
	}																								


#define ADD_SYMBOL_11_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, 				\
				 size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11)										\
	{																												\
		return wrap_msft_sysv_11(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, name ## _function);		\
	}																																

#define ADD_SYMBOL_10_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, 				\
				 size_t a_7, size_t a_8, size_t a_9, size_t a_10)													\
	{																												\
		return wrap_msft_sysv_10(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, name ## _function);				\
	}																																

#define ADD_SYMBOL_9_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, 				\
				 size_t a_7, size_t a_8, size_t a_9)																\
	{																												\
		return wrap_msft_sysv_9(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, name ## _function);					\
	}	

#define ADD_SYMBOL_8_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, 				\
				 size_t a_7, size_t a_8)																			\
	{																												\
		return wrap_msft_sysv_8(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, name ## _function);							\
	}			

#define ADD_SYMBOL_7_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6, 				\
				 size_t a_7)																						\
	{																												\
		return wrap_msft_sysv_7(a_1, a_2, a_3, a_4, a_5, a_6, a_7, name ## _function);								\
	}					

#define ADD_SYMBOL_6_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6)				\
	{																												\
		return wrap_msft_sysv_6(a_1, a_2, a_3, a_4, a_5, a_6, name ## _function);									\
	}			

#define ADD_SYMBOL_5_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5)							\
	{																												\
		return wrap_msft_sysv_5(a_1, a_2, a_3, a_4, a_5, name ## _function);										\
	}			

#define ADD_SYMBOL_4_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3, size_t a_4)										\
	{																												\
		return wrap_msft_sysv_4(a_1, a_2, a_3, a_4, name ## _function);												\
	}			


#define ADD_SYMBOL_3_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2, size_t a_3)													\
	{																												\
		return wrap_msft_sysv_3(a_1, a_2, a_3, name ## _function);													\
	}			

#define ADD_SYMBOL_2_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1, size_t a_2)																\
	{																												\
		return wrap_msft_sysv_2(a_1, a_2, name ## _function);														\
	}			

#define ADD_SYMBOL_1_A(name)																						\
	static void * name ## _function;																				\
	size_t XENUS_SYM name	(size_t a_1)																			\
	{																												\
		return wrap_msft_sysv_1(a_1, name ## _function);															\
	}																								
	

ADD_SYMBOL_1_A(kallsyms_lookup_name);

#define ADD_FUNCTION_UNOPTIMIZED	ADD_SYMBOL_12_A
#define ADD_FUNCTION_12				ADD_SYMBOL_12_A
#define ADD_FUNCTION_11				ADD_SYMBOL_11_A
#define ADD_FUNCTION_10				ADD_SYMBOL_10_A
#define ADD_FUNCTION_9				ADD_SYMBOL_9_A
#define ADD_FUNCTION_8				ADD_SYMBOL_8_A
#define ADD_FUNCTION_7				ADD_SYMBOL_7_A
#define ADD_FUNCTION_6				ADD_SYMBOL_6_A
#define ADD_FUNCTION_5				ADD_SYMBOL_6_A
#define ADD_FUNCTION_4				ADD_SYMBOL_6_A
#define ADD_FUNCTION_3				ADD_SYMBOL_6_A
#define ADD_FUNCTION_2				ADD_SYMBOL_6_A
#define ADD_FUNCTION_1				ADD_SYMBOL_6_A
#include "sys_functions_all.macros"
#undef ADD_FUNCTION_UNOPTIMIZED
#undef ADD_FUNCTION_12			
#undef ADD_FUNCTION_11			
#undef ADD_FUNCTION_10			
#undef ADD_FUNCTION_9			
#undef ADD_FUNCTION_8			
#undef ADD_FUNCTION_7			
#undef ADD_FUNCTION_6			
#undef ADD_FUNCTION_5			
#undef ADD_FUNCTION_4			
#undef ADD_FUNCTION_3			
#undef ADD_FUNCTION_2			
#undef ADD_FUNCTION_1			

void add_symbols()
{
	#define ADD_FUNCTION_UNOPTIMIZED	ADD_SYMBOL_B
	#define ADD_FUNCTION_12				ADD_SYMBOL_B
	#define ADD_FUNCTION_11				ADD_SYMBOL_B
	#define ADD_FUNCTION_10				ADD_SYMBOL_B
	#define ADD_FUNCTION_9				ADD_SYMBOL_B
	#define ADD_FUNCTION_8				ADD_SYMBOL_B
	#define ADD_FUNCTION_7				ADD_SYMBOL_B
	#define ADD_FUNCTION_6				ADD_SYMBOL_B
	#define ADD_FUNCTION_5				ADD_SYMBOL_B
	#define ADD_FUNCTION_4				ADD_SYMBOL_B
	#define ADD_FUNCTION_3				ADD_SYMBOL_B
	#define ADD_FUNCTION_2				ADD_SYMBOL_B
	#define ADD_FUNCTION_1				ADD_SYMBOL_B
	#include "sys_functions_all.macros"
	#undef ADD_FUNCTION_UNOPTIMIZED
	#undef ADD_FUNCTION_UNOPTIMIZED
	#undef ADD_FUNCTION_12			
	#undef ADD_FUNCTION_11			
	#undef ADD_FUNCTION_10			
	#undef ADD_FUNCTION_9			
	#undef ADD_FUNCTION_8			
	#undef ADD_FUNCTION_7			
	#undef ADD_FUNCTION_6			
	#undef ADD_FUNCTION_5			
	#undef ADD_FUNCTION_4			
	#undef ADD_FUNCTION_3			
	#undef ADD_FUNCTION_2			
	#undef ADD_FUNCTION_1		
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// In june, I decided that we should pass SysV apis to the entrypoint to prevent additional gcc and objtool fuck assery 					  //
// do note, it's still convenient to have some functions that we don't lookup																  //
// in therory, we could just have the dynamic ksym lookup function, but i can't be fucking arsed to rewrite this bullshit for a third time.   //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define ADD_FUNCTION_UNOPTIMIZED(ignore, name)		ADD_SYMBOL_12_A(name)
#define ADD_FUNCTION_12(ignore, name)				ADD_SYMBOL_12_A(name)
#define ADD_FUNCTION_11(ignore, name)				ADD_SYMBOL_11_A(name)
#define ADD_FUNCTION_10(ignore, name)				ADD_SYMBOL_10_A(name)
#define ADD_FUNCTION_9(ignore, name)				ADD_SYMBOL_9_A(name)
#define ADD_FUNCTION_8(ignore, name)				ADD_SYMBOL_8_A(name)
#define ADD_FUNCTION_7(ignore, name)				ADD_SYMBOL_7_A(name)
#define ADD_FUNCTION_6(ignore, name)				ADD_SYMBOL_6_A(name)
#define ADD_FUNCTION_5(ignore, name)				ADD_SYMBOL_6_A(name)
#define ADD_FUNCTION_4(ignore, name)				ADD_SYMBOL_6_A(name)
#define ADD_FUNCTION_3(ignore, name)				ADD_SYMBOL_6_A(name)
#define ADD_FUNCTION_2(ignore, name)				ADD_SYMBOL_6_A(name)
#define ADD_FUNCTION_1(ignore, name)				ADD_SYMBOL_6_A(name)
#include "bootstrap_no_ms_abi.macros"
#undef ADD_FUNCTION_UNOPTIMIZED
#undef ADD_FUNCTION_12			
#undef ADD_FUNCTION_11			
#undef ADD_FUNCTION_10			
#undef ADD_FUNCTION_9			
#undef ADD_FUNCTION_8			
#undef ADD_FUNCTION_7			
#undef ADD_FUNCTION_6			
#undef ADD_FUNCTION_5			
#undef ADD_FUNCTION_4			
#undef ADD_FUNCTION_3			
#undef ADD_FUNCTION_2			
#undef ADD_FUNCTION_1		


#include "_\_xenus_bootstrap.h"
void gcc_no_msi_abi(void * bootstrap_)
{
	bootstrap_t * bootstrap;

	bootstrap = (bootstrap_t *)bootstrap_;

	#define FIX_BOOTSTRAP_FUNCTION(path, name) \
		*(void **)(name ## _function)  = (void *)(bootstrap-> ## path ## . ## name ##); \
		*(void **)(bootstrap-> ## path ## . ## name) = (void *)name;
	
	#define ADD_FUNCTION_UNOPTIMIZED	FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_12				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_11				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_10				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_9				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_8				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_7				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_6				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_5				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_4				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_3				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_2				FIX_BOOTSTRAP_FUNCTION
	#define ADD_FUNCTION_1				FIX_BOOTSTRAP_FUNCTION
	
	#include "bootstrap_no_ms_abi.macros"
	
	#undef ADD_FUNCTION_UNOPTIMIZED
	#undef ADD_FUNCTION_12			
	#undef ADD_FUNCTION_11			
	#undef ADD_FUNCTION_10			
	#undef ADD_FUNCTION_9			
	#undef ADD_FUNCTION_8			
	#undef ADD_FUNCTION_7			
	#undef ADD_FUNCTION_6			
	#undef ADD_FUNCTION_5			
	#undef ADD_FUNCTION_4			
	#undef ADD_FUNCTION_3			
	#undef ADD_FUNCTION_2			
	#undef ADD_FUNCTION_1	
	
	FIX_BOOTSTRAP_FUNCTION(symbols, kallsyms_lookup_name);
}
