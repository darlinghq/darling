.text
.globl __darling_bsd_syscall
.private_extern __darling_bsd_syscall

#if defined(__x86_64__)

__darling_bsd_syscall:
Lentry_hook:
	.space 13, 0x90
	movq    ___bsd_syscall_table@GOTPCREL(%rip), %r10
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
.std_ret:
Lexit_hook:
	.space 13, 0x90
	ret
.no_sys:
	movq	%rax, %rdi
	call	___unknown_syscall
	jmp		.std_ret

.section        __DATA,__data
.globl __darling_bsd_syscall_entry
.globl __darling_bsd_syscall_exit
__darling_bsd_syscall_entry:
	.quad Lentry_hook
__darling_bsd_syscall_exit:
	.quad Lexit_hook

#elif defined(__i386__)

#define copy_arg(off) \
	movl	8+off(%esp), %ecx ;\
	movl	%ecx, -56+off(%esp)

__darling_bsd_syscall:
Lentry_hook:
	.space 6, 0x90
	calll	1f
1:
	popl	%ecx
2:
	movl	L___bsd_syscall_table$non_lazy_ptr-1b(%ecx), %ecx
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
	copy_arg(24)
	copy_arg(28)
	copy_arg(32)
	copy_arg(36)
	copy_arg(40)
	copy_arg(44)
	copy_arg(48)
	copy_arg(52)
	subl	$56, %esp
	call	*%eax
	addl	$56, %esp
.std_ret:
Lexit_hook:
	.space 6, 0x90
	ret
.no_sys:
	pushl	%ecx
	call	___unknown_syscall
	addl	$4, %esp
	jmp .std_ret

	.section        __IMPORT,__pointers,non_lazy_symbol_pointers
L___bsd_syscall_table$non_lazy_ptr:
	.indirect_symbol        ___bsd_syscall_table
	.long   0

.section        __DATA,__data
.globl __darling_bsd_syscall_entry
.globl __darling_bsd_syscall_exit
__darling_bsd_syscall_entry:
	.long Lentry_hook
__darling_bsd_syscall_exit:
	.long Lexit_hook

.subsections_via_symbols


#elif defined(__arm64__)
// For i386/x64, the `syscall_sw.h` file has the BSD and Mach syscall defined
// between two different macros (`UNIX_SYSCALL_TRAP` and `kernel_trap`). This 
// is not the case for ARM.

// For ARM64, there is only one syscall that is used for either BSD and Mach.
// If the syscall number is less then zero, it will do a Mach syscall; otherwise,
// it will do a BSD syscall.

// Because of the above, I decided to have the both the BSD and Mach syscall be
// handled by the __darling_bsd_syscall.

// Notes:
// https://stackoverflow.com/a/56993314/5988706
// xnu/osfmk/arm64/sleh.c - handle_svc function

__darling_bsd_syscall:
	; Push temporary registers (syscalls don't modify temp register)
	sub sp,sp,#16
	stp x9,x10,[sp]
	
	; if (x16 / *syscall number */ < 0)
	cmp x16,#0
	blt .get_mach_table

	; Get &___bsd_syscall_table (function pointer)
	adrp x9, ___bsd_syscall_table@PAGE
	add x9, x9, ___bsd_syscall_table@PAGEOFF
	mov x10,x16
	b .calc_offset

.get_mach_table:
	; Get &___mach_syscall_table (function pointer)
	adrp x9, ___mach_syscall_table@PAGE
	add x9, x9, ___mach_syscall_table@PAGEOFF
	neg x10,x16

.calc_offset:
	; &syscall_table + x10 * sizeof(void*) 
	lsl x10,x10,3
	add x9,x9,x10

	; Call ___bsd_syscall_table method
	blr x9

	; Restore original temp register values
	ldp x9,x10,[sp]
	add sp,sp,#16
	ret

#warning "Heed to improve __darling_bsd_syscall ARM64 implementation"
#else
#	error Missing assembly
#endif

