.text
.globl __darling_machdep_syscall
.private_extern __darling_machdep_syscall

#if defined(__x86_64__)

__darling_machdep_syscall:
	movq    ___machdep_syscall_table@GOTPCREL(%rip), %r10
	andq	$0xff, %rax
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
	call	___unknown_syscall_machdep
	ret

#elif defined(__i386__)

#define copy_arg(off) \
	movl	8+off(%esp), %ecx ;\
	movl	%ecx, -24+off(%esp)

__darling_machdep_syscall:
	calll	1f
1:
	popl	%ecx
2:
	movl	L___machdep_syscall_table$non_lazy_ptr-1b(%ecx), %ecx
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
	call	___unknown_syscall_machdep
	addl	$4, %esp
	ret

	.section        __IMPORT,__pointers,non_lazy_symbol_pointers
L___machdep_syscall_table$non_lazy_ptr:
	.indirect_symbol        ___machdep_syscall_table
	.long   0

.subsections_via_symbols

#elif __arm64__
__darling_machdep_syscall:
	and		x9,x16,#0xff					; x9 = syscall_number & 0xff
	lsl		x9,x9,3							; x9 = x9 << 3 // (x9 * 8)
	ldr		x10,=___machdep_syscall_table		 
	add		x10,x9,x10						; x10 = __machdep_syscall_table + x9
	ldr		x9,[x10]						; x9 = *(x10)
	cbz		x9, .no_sys
	blr		x9
	ret
.no_sys:
	bl		___unknown_syscall_machdep
	ret

#else
#	error Missing assembly
#endif

