	.text
	.file	"test.c"
	.globl	func
	.align	16, 0x90
	.type	func,@function
func:                                   # @func
	.cfi_startproc
# BB#0:
	pushq	%rbp
.Ltmp0:
	.cfi_def_cfa_offset 16
.Ltmp1:
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.Ltmp2:
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	cmpq	$0, -8(%rbp)
	jge	.LBB0_3
# BB#1:
	cmpq	$-4096, -8(%rbp)        # imm = 0xFFFFFFFFFFFFF000
	jle	.LBB0_3
# BB#2:
	movb	$0, %al
	callq	cerror@PLT
.LBB0_3:
	addq	$16, %rsp
	popq	%rbp
	retq
.Ltmp3:
	.size	func, .Ltmp3-func
	.cfi_endproc


	.ident	"clang version 3.6.2 (tags/RELEASE_362/final)"
	.section	".note.GNU-stack","",@progbits
