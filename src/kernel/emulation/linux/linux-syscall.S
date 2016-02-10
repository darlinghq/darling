.text
.globl linux_syscall

linux_syscall:

#if defined(__x86_64__)

	movl	8(%rsp), %eax
	movq	%rcx, %r10
	syscall
	ret

#elif defined(__i386__)

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

#else

#	error Missing assembly!

#endif

