;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;    Purpose:                                        ;;
;;    Author: Reece W.                                ;;
;;    License: All Rights Reserved J. Reece Wilson    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;; Warning: this file is to be used with kernels that are built with no spectre mitigation
;;;; At least, remove CONFIG_RETPOLINE

PUBLIC wrap_msft_sysv_12
PUBLIC wrap_msft_sysv_11
PUBLIC wrap_msft_sysv_10
PUBLIC wrap_msft_sysv_9
PUBLIC wrap_msft_sysv_8
PUBLIC wrap_msft_sysv_7
PUBLIC wrap_msft_sysv_6
PUBLIC wrap_msft_sysv_5
PUBLIC wrap_msft_sysv_4
PUBLIC wrap_msft_sysv_3
PUBLIC wrap_msft_sysv_2
PUBLIC wrap_msft_sysv_1
PUBLIC wrap_msft_sysv_0

.code

wrap_msft_sysv_12 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    					 
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    					 
    push [rbp + 56d]     ; map 12th parameter 
    push [rbp + 48d]     ; map 11th parameter 
    push [rbp + 40d]     ; map 10th parameter 
    push [rbp + 32d]     ; map 9th parameter 
    push [rbp + 24d]     ; map 8th parameter 
    push [rbp + 16d]     ; map 7th parameter 
            
    mov rax, [rbp + 64d] ; lea rax, null_stub
    call rax             ; call rax           - if this crashes still, chk spectre mitigation!
    add rsp, 48d
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp
    ret
wrap_msft_sysv_12 ENDP

wrap_msft_sysv_11 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    
    push [rbp + 48d]     ; map 11th parameter 
    push [rbp + 40d]     ; map 10th parameter 
    push [rbp + 32d]     ; map 9th parameter 
    push [rbp + 24d]     ; map 8th parameter 
    push [rbp + 16d]     ; map 7th parameter 
                
    mov rax, [rbp + 56d]
    call rax
    add rsp, 40d
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_11 ENDP

wrap_msft_sysv_10 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    
    push [rbp + 40d]     ; map 10th parameter 
    push [rbp + 32d]     ; map 9th parameter 
    push [rbp + 24d]     ; map 8th parameter 
    push [rbp + 16d]     ; map 7th parameter 
                
    mov rax, [rbp + 48d]
    call rax
    add rsp, 32d
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_10 ENDP

wrap_msft_sysv_9 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    
    push [rbp + 32d]     ; map 9th parameter 
    push [rbp + 24d]     ; map 8th parameter 
    push [rbp + 16d]     ; map 7th parameter 
                
    mov rax, [rbp + 40d]
    call rax
    add rsp, 24d
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_9 ENDP

wrap_msft_sysv_8 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    
    push [rbp + 24d]     ; map 8th parameter 
    push [rbp + 16d]     ; map 7th parameter 
                
    mov rax, [rbp + 32d]
    call rax
    add rsp, 16d
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_8 ENDP

wrap_msft_sysv_7 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    
    push [rbp + 16d]     ; map 7th parameter 
                
    mov rax, [rbp + 24d]
    call rax
    add rsp, 8d
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_7 ENDP

wrap_msft_sysv_6 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    mov r9, [rbp + 8]    ; map 6th parameter
    
    mov rax, [rbp + 16d]
    call rax
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_6 ENDP


wrap_msft_sysv_5 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    mov r8, [rbp]        ; map 5th parameter
    
    mov rax, [rbp + 8]
    call rax
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_5 ENDP

wrap_msft_sysv_4 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    mov rcx, r9          ; map 4th parameter 
    
    
    mov rax, [rbp]
    call rax
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_4 ENDP

wrap_msft_sysv_3 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    mov rdx, r8          ; map 3rd parameter 
    
    call r9
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_3 ENDP

wrap_msft_sysv_2 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    mov rsi, rdx         ; map 2nd parameter 
    
    call r8
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_2 ENDP

wrap_msft_sysv_1 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi
    push rsi
    push rbx
    
    mov rdi, rcx         ; map 1st parameter 
    
    call rdx
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_1 ENDP

wrap_msft_sysv_0 PROC
    push rbp             ; save rbp 
    
    mov rbp, rsp         ; store stack pointer in base pointer
    add rbp, 8           ; base pointer += sizeof(rbp)
    add rbp, 8           ; base pointer += sizeof(return address)
    add rbp, 32d         ; base pointer += 32
                         ; base pointer  =  Windows x86_64 argument stack
                         ; base pointer =/= RSP 
    push rdi			 
    push rsi
    push rbx
    
    call rdx
    
    pop rbx
    pop rsi
    pop rdi 

    pop rbp                
    ret
wrap_msft_sysv_0 ENDP

END