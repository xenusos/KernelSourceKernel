// Originally: this wasn't needed
//               but then, i decided that dynamic kernel exports are annoying as fuck to deal with, so linux translation needed the following macros.
//             skip a few months, objtool now suffers from an aneurysm when dealing with too many MSFT ABI functions (let's be real here, they weren't even complient with any ABI; thank GCC for this).
//             therefore, both linux translation and pre-stage-1 global functions needs these fat-crows
#define ADD_SYMBOL_12_A(name)                                                                                        \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,                \
                 size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11, size_t a_12)                          \
    {                                                                                                                \
        return wrap_msft_sysv_12(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, a_12, name ## _function);  \
    }                                                                                                


#define ADD_SYMBOL_11_A(name)                                                                                        \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,                \
                 size_t a_7, size_t a_8, size_t a_9, size_t a_10, size_t a_11)                                       \
    {                                                                                                                \
        return wrap_msft_sysv_11(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, a_11, name ## _function);        \
    }                                                                                                                                  
                                                                                                                     
#define ADD_SYMBOL_10_A(name)                                                                                        \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,                \
                 size_t a_7, size_t a_8, size_t a_9, size_t a_10)                                                    \
    {                                                                                                                \
        return wrap_msft_sysv_10(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, a_10, name ## _function);              \
    }                                                                                                                                  
                                                                                                                     
#define ADD_SYMBOL_9_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,                \
                 size_t a_7, size_t a_8, size_t a_9)                                                                 \
    {                                                                                                                \
        return wrap_msft_sysv_9(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, a_9, name ## _function);                     \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_8_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,                \
                 size_t a_7, size_t a_8)                                                                             \
    {                                                                                                                \
        return wrap_msft_sysv_8(a_1, a_2, a_3, a_4, a_5, a_6, a_7, a_8, name ## _function);                          \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_7_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6,                \
                 size_t a_7)                                                                                         \
    {                                                                                                                \
        return wrap_msft_sysv_7(a_1, a_2, a_3, a_4, a_5, a_6, a_7, name ## _function);                               \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_6_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5, size_t a_6)                \
    {                                                                                                                \
        return wrap_msft_sysv_6(a_1, a_2, a_3, a_4, a_5, a_6, name ## _function);                                    \
    }                                                                                                                \
                                                                                                                     
#define ADD_SYMBOL_5_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4, size_t a_5)                            \
    {                                                                                                                \
        return wrap_msft_sysv_5(a_1, a_2, a_3, a_4, a_5, name ## _function);                                         \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_4_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3, size_t a_4)                                        \
    {                                                                                                                \
        return wrap_msft_sysv_4(a_1, a_2, a_3, a_4, name ## _function);                                              \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_3_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2, size_t a_3)                                                    \
    {                                                                                                                \
        return wrap_msft_sysv_3(a_1, a_2, a_3, name ## _function);                                                   \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_2_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1, size_t a_2)                                                                \
    {                                                                                                                \
        return wrap_msft_sysv_2(a_1, a_2, name ## _function);                                                        \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_1_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    (size_t a_1)                                                                            \
    {                                                                                                                \
        return wrap_msft_sysv_1(a_1, name ## _function);                                                             \
    }                                                                                                                
                                                                                                                     
#define ADD_SYMBOL_0_A(name)                                                                                         \
    static void * name ## _function;                                                                                 \
    size_t XENUS_SYM name    ()                                                                                      \
    {                                                                                                                \
        return wrap_msft_sysv_1(0, name ## _function);                                                               \
    }                                                                                                                
