.globl __darling_bsd_syscall

#if defined(__x86_64__)

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

#elif defined(__i386__)

__darling_bsd_syscall:
        calll   .L0$pb
.L0$pb:
        popl    %ecx
.Ltmp0:
        addl    $_GLOBAL_OFFSET_TABLE_+(.Ltmp0-.L0$pb), %ecx
        movl    __bsd_syscall_table@GOT(%eax), %eax
		movl	(%eax), %eax
        movl    8(%esp), %ecx
		movl    (%eax,%ecx,4), %eax
		test	%eax, %eax
		jz		.no_sys
		call	*%eax
		ret
.no_sys:
		pushl	%ecx
		call	__unknown_syscall@PLT
		addl	$4, %esp
		ret

#else
#	error Missing assembly
#endif

