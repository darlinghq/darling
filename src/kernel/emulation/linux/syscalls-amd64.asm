.globl __darling_bsd_syscall

__darling_bsd_syscall:
	movq    __bsd_syscall_table@GOTPCREL(%rip), %r10
	movq	(%r10,%rax,8), %r10
	test	%r10, %r10
	jz		.no_sys
	call	*%r10
	ret
.no_sys:
	movq	%rax, %rdi
	call	__unknown_syscall@PLT
	ret

