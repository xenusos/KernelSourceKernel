;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;    Purpose:                                        ;;
;;    Author: Reece W.                                ;;
;;    License: All Rights Reserved J. Reece Wilson    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PUBLIC stack_realigner

.code

stack_realigner PROC
    mov rax, rsp
    ;;add rax, 8
    and rax, 15d       ;; rsp + 32 + 8 should be divisible by 16. 32 is divisible by 16; this is ignored. (rsp + 8) & 15 == (rsp + 8) mod 16 == 0
    cmp rax, 0         ;; if this isn't zero, appending an additional integer onto the stack is sufficient to go from 8 byte alignment to 16 byte alignment.
    
    je alreadyAligned
    notAligned:
        sub rsp, 32d
        sub rsp, 8
        mov r9, rcx
        mov rcx, rdx
        call r9
        add rsp, 8
        jmp cleanUp

    alreadyAligned:
        sub rsp, 32d
        mov r9, rcx
        mov rcx, rdx
        call r9

    cleanUp:
        add rsp, 32
        ret

stack_realigner ENDP

END