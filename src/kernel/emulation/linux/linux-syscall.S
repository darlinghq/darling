.text
.globl _linux_syscall
.private_extern _linux_syscall

_linux_syscall:

#if defined(__x86_64__)

	movl	8(%rsp), %eax
	movq	%rcx, %r10
	syscall
	ret

#elif defined(__i386__)

	// With 32in64, we cannot use sysenter, because our process doesn't have a 32-bit VDSO mapped
	// and sysenter needs that for returning to user space.

	// eax, ecx and edx are caller saved, the rest is calee saved
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp

	mov		20(%esp), %ebx
	mov		24(%esp), %ecx
	mov		28(%esp), %edx
	mov		32(%esp), %esi
	mov		36(%esp), %edi
	mov		40(%esp), %ebp
	mov		44(%esp), %eax

	int		$0x80

	pop		%ebp
	pop		%edi
	pop		%esi
	pop		%ebx
	ret

#elif 0 //defined(__i386__)

	push	%ebp
	push	%ebx
	push	%esi
	push	%edi
	call	1f
1:
	pop		%ebp
	addl	$(2f-1b), %ebp
	push	%ebp
	push	%ecx
	push	%edx
	movl	28+28(%esp), %eax
	movl	4+28(%esp), %ebx
	movl	8+28(%esp), %ecx
	movl	12+28(%esp), %edx
	movl	16+28(%esp), %esi
	movl	20+28(%esp), %edi
	push	%ebp
	movl	24+28(%esp), %ebp
	movl	%ebp, -4(%esp)
	movl	%esp, %ebp
	sysenter
2:
	pop %edi
	pop %esi
	pop %ebx
	pop %ebp
	ret

#elif __arm64__
; linux_syscall(long x0, x1, x2, x3, x4, x5, x6 /* syscall number */)
	mov x8,x6	; copy syscall number to x8
	svc #0		; activate syscall
	ret

#else

#	error Missing assembly!

#endif

