# Built output for (null)
# Generated at (null)
# Using (null) configuration, (null) architecture for (null) target of (null) project




	.text
	.align 2
	.globl _mystart
_mystart:
#if __i386__ 
	call	L1
L1: popl	%eax
	movl	L_flag$non_lazy_ptr-L1(%eax), %eax
	movl	$2, (%eax)
	jmp		start
#elif __x86_64__
	movl	$2, _flag(%rip)
	jmp		start
#elif __ppc__ || __ppc64__
	li r0,2
	lis r2,ha16(_flag)
	stw r0,lo16(_flag)(r2)
	b		start
#elif __arm__
	ldr	r3, L4
	mov	r2, #2
	add	r3, pc
	ldr	r3, [r3]
	str	r2, [r3, #0]
	b	start
L4:	.long	L_flag$non_lazy_ptr-(L4-8)
#endif

#if __i386__ || __arm__
	.section	__DATA,__nl_symbol_ptr,non_lazy_symbol_pointers
	.align	2
L_flag$non_lazy_ptr:
.indirect_symbol _flag
	.long	0
#endif


