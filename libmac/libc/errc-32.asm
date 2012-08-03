section .tdata

warnc_orig_retaddr: dw 0

section .text

global errc
global warnc

extern errnoDarwinToLinux
extern __errno_location
extern err
extern warn

remove_and_convert:
	; save return EIP in ecx
	pop ecx

	; ebp+8 is code (argument 2)
	mov edx, [esp+8]
	; move the first argument to the position of the second one
	mov ebx, [esp+4]
	mov [esp+8], ebx
	; also move the EIP of the original caller
	mov ebx, [esp]
	mov [esp+4], ebx
	mov [esp], ecx ; restore owr return value

	; now the second argument is removed
	; we need to convert it
	push edx
	call errnoDarwinToLinux
	add esp, 4 ; remove argument
	push eax ; now we have the converted errno

	call __errno_location
	pop edx ; the converted errno is in edx
	mov [eax], edx ; we set the global errno

	ret

errc:
	call remove_and_convert
	jmp err ; it doesn't return, no need to deal with the modified stack

warnc:
	call remove_and_convert
	mov ebx, [esp]
	mov [warnc_orig_retaddr], ebx ; store original return EIP
	pop ebx ; make room for our own
	call warn
	; add a fictional argument back to the stack
	sub esp, 4
	jmp [warnc_orig_retaddr]

