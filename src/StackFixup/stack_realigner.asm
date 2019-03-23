;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;    Purpose: MS_ABI x64 STACK ALIGNMENT CORRECTOR   ;;
;;    Author: Reece W.                                ;;
;;    License: All Rights Reserved J. Reece Wilson    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PUBLIC stack_realigner

.code

;; note: your backtrace WILL end here. msvc and the debugging apis should determine this to be external code 
;; QWORD(* stack_realigner)(QWORD(* callback)(void * context), void * context)
stack_realigner PROC    
    mov rax, rsp
    ;;add rax, 8	   ;;   [example prolog:  push  R15		    - WE ARE HERE - NOT ALIGNED
                       ;;                     push  R14
                       ;;                     push  R13		    - POSTEXEC: 16 byte aligned
                       ;;                     sub rsp, reserve; - POSTEXEC: 16 byte aligned	] 
                       ;;
    and rax, 15d       ;; 
    cmp rax, 0         ;;  ZF = bit((rsp & 15) - 0) == 0) == bit(rsp mod 16 == 0)
                      
    je alreadyAlignedAsLeaf  ;; if (ZF) { alreadyAligned } else { notAligned }
    notAligned:
        sub rsp, 8  
        sub rsp, 32d
        mov rax, rcx
        mov rcx, rdx
        call rax
        add rsp, 8
        jmp cleanUp

    alreadyAlignedAsLeaf:
        sub rsp, 32d
        mov rax, rcx
        mov rcx, rdx
        call rax
    cleanUp:
        add rsp, 32
        ret

stack_realigner ENDP

END