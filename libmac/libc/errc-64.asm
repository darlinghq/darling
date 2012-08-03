section .text

global errc
global warnc
global verrc
global vwarnc

extern errnoDarwinToLinux
extern __errno_location
extern err
extern warn
extern verr
extern vwarn

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
	pop rdx ; remove argument
	push rax ; now we have the converted errno

	call __errno_location
	pop rdx ; the converted errno is in edx
	mov [rax], rdx ; we set the global errno

	ret

errc:
	call remove_and_convert
	jmp err

warnc:
	call remove_and_convert
	jmp warn

verrc:
	call remove_and_convert
	jmp verr

vwarnc:
	call remove_and_convert
	jmp vwarn


