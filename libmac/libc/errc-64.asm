section .tdata

warnc_orig_retaddr: dq 0

section .text

global errc
global warnc

extern errnoDarwinToLinux
extern __errno_location
extern err
extern warn

remove_and_convert:
	; save return EIP in ecx
	pop rcx

	; ebp+8 is code (argument 2)
	mov rdx, [rsp+16]
	; move the first argument to the position of the second one
	mov rbx, [rsp+8]
	mov [rsp+16], rbx
	; also move the EIP of the original caller
	mov rbx, [rsp]
	mov [rsp+8], rbx
	mov [rsp], rcx ; restore owr return value

	; now the second argument is removed
	; we need to convert it
	push rdx
	call errnoDarwinToLinux
	add rsp, 8 ; remove argument
	push rax ; now we have the converted errno

	call __errno_location
	pop rdx ; the converted errno is in edx
	mov [rax], rdx ; we set the global errno

	ret

errc:
	call remove_and_convert
	jmp err ; it doesn't return, no need to deal with the modified stack

warnc:
	call remove_and_convert
	mov rbx, [rsp]
	mov [warnc_orig_retaddr], rbx ; store original return EIP
	pop rbx ; make room for our own
	call warn
	; add a fictional argument back to the stack
	sub rsp, 8
	jmp [warnc_orig_retaddr]

