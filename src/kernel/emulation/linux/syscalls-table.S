.text
.globl __darling_bsd_syscall

#if defined(__x86_64__)

__darling_bsd_syscall:
	movq    __bsd_syscall_table@GOTPCREL(%rip), %r10
	movq	(%r10,%rax,8), %r10
	test	%r10, %r10
	jz		.no_sys
	# Re-push possible 7th and 8th arg
	movq	24(%rsp), %r11
	pushq	%r11
	movq	24(%rsp), %r11
	pushq	%r11
	call	*%r10
	addq	$16, %rsp
	ret
.no_sys:
	movq	%rax, %rdi
	call	__unknown_syscall@PLT
	ret

#elif defined(__i386__)

#define copy_arg(off) \
	movl	8+off(%esp), %ecx ;\
	movl	%ecx, -24+off(%esp)

__darling_bsd_syscall:
	calll	.L0$pb
.L0$pb:
	popl	%ecx
.Ltmp0:
	addl	$_GLOBAL_OFFSET_TABLE_+(.Ltmp0-.L0$pb), %ecx
	movl	__bsd_syscall_table@GOT(%ecx), %ecx
	andl	$0xffff, %eax // Because there is some extra stuff in upper bytes we don't need
	movl	(%ecx,%eax,4), %eax
	test	%eax, %eax
	jz		.no_sys
	// Copy arguments
	copy_arg(0)
	copy_arg(4)
	copy_arg(8)
	copy_arg(12)
	copy_arg(16)
	copy_arg(20)
	subl	$24, %esp
	call	*%eax
	addl	$24, %esp
	ret
.no_sys:
	pushl	%ecx
	call	__unknown_syscall@PLT
	addl	$4, %esp
	ret

#else
#	error Missing assembly
#endif

