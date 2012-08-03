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
	pop edx ; remove argument
	push eax ; now we have the converted errno

	call __errno_location
	pop edx ; the converted errno is in edx
	mov [eax], edx ; we set the global errno

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


