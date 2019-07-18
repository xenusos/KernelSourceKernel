;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;    Purpose: MS_ABI x64 STACK ALIGNMENT CORRECTOR   ;;
;;    Author: Reece W.                                ;;
;;    License: All Rights Reserved Reece Wilson       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PUBLIC stack_realigner

.code

;; note: your backtrace WILL end here. msvc and the debugging apis should identify this as external code 
;; QWORD(* stack_realigner)(QWORD(* callback)(void * context), void * context)
stack_realigner PROC    
	push rbp
	mov rbp, rsp
	
	and rsp, 0fffffffffffffff7h
	sub rbp, rsp 

	;; mov rax, rsp
	;; add rax, rbp
	;; add rax, 8
	;; rax = caller 
	
	mov rax, rcx
	mov rcx, rdx
	sub rsp, 32d
	call rax
	add rsp, 32d
	
	add rsp, rbp 
	pop rbp

	ret
stack_realigner ENDP

END