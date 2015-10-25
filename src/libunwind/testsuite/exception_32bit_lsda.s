
// TEST-OPTIONS: -arch x86_64 -lstdc++ -Wl,-no_compact_unwind


	.section	__TEXT,__text,regular,pure_instructions
	.globl	__Z3foov
	.align	4, 0x90
__Z3foov:
Leh_func_begin1:
	pushq	%rbp
Ltmp0:
	movq	%rsp, %rbp
Ltmp1:
	subq	$16, %rsp
Ltmp2:
	movabsq	$4, %rax
	movq	%rax, %rdi
	callq	___cxa_allocate_exception
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movl	$10, (%rax)
	movq	-16(%rbp), %rax
	movq	__ZTIi@GOTPCREL(%rip), %rcx
	leaq	(%rcx), %rcx
	movabsq	$0, %rdx
	movq	%rax, %rdi
	movq	%rcx, %rsi
	callq	___cxa_throw
Leh_func_end1:

	.globl	_main
	.align	4, 0x90
_main:
Leh_func_begin2:
	pushq	%rbp
Ltmp6:
	movq	%rsp, %rbp
Ltmp7:
	subq	$48, %rsp
Ltmp8:
	movl	$1, -28(%rbp)
	movl	$2, -28(%rbp)
Ltmp3:
	callq	__Z3foov
Ltmp4:
	jmp	LBB2_1
LBB2_1:
	movl	$3, -28(%rbp)
	jmp	LBB2_10
LBB2_2:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	callq	___cxa_begin_catch
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, -32(%rbp)
	movl	-28(%rbp), %eax
	cmpl	$2, %eax
	je	LBB2_4
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	$0, -8(%rbp)
	jmp	LBB2_7
LBB2_4:
	movl	-32(%rbp), %eax
	cmpl	$10, %eax
	je	LBB2_6
	movl	$1, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	$0, -8(%rbp)
	jmp	LBB2_7
LBB2_6:
	movl	$4, -28(%rbp)
	movl	$1, -8(%rbp)
LBB2_7:
	callq	___cxa_end_catch
	movl	-8(%rbp), %eax
	cmpl	$1, %eax
	jne	LBB2_9
	jmp	LBB2_10
LBB2_9:
	jmp	LBB2_13
LBB2_10:
	movl	-28(%rbp), %eax
	cmpl	$4, %eax
	jne	LBB2_12
	movl	$0, -12(%rbp)
	jmp	LBB2_13
LBB2_12:
	movl	$1, -12(%rbp)
LBB2_13:
	movl	-12(%rbp), %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	addq	$48, %rsp
	popq	%rbp
	ret
LBB2_15:
Ltmp5:
	movq	%rax, -40(%rbp)
	movq	-40(%rbp), %rax
	movl	%edx, %eax
	movl	%eax, -44(%rbp)
	movl	$2, %eax
	movl	-44(%rbp), %ecx
	cmpl	%eax, %ecx
	je	LBB2_2
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	callq	__Unwind_Resume_or_Rethrow
Leh_func_end2:


	.section	__TEXT,__gcc_except_tab
	.align	2
GCC_except_table2:
Lexception2:
	.byte	255
	.byte	155
	.byte	168
	.space	1
	.byte	3
	.byte	26
Lset0 = Ltmp3-Leh_func_begin2
	.long	Lset0
Lset1 = Ltmp4-Ltmp3
	.long	Lset1
Lset2 = Ltmp5-Leh_func_begin2
	.long	Lset2
	.byte	3
Lset3 = Ltmp4-Leh_func_begin2
	.long	Lset3
Lset4 = Leh_func_end2-Ltmp4
	.long	Lset4
	.long	0
	.byte	0


	.byte	1
	.byte	0

	.byte	2
	.byte	125

	.long	__ZTIi@GOTPCREL+4
	.long	0
	.align	2

	.section	__TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame0:
Lsection_eh_frame:
Leh_frame_common:
Lset5 = Leh_frame_common_end-Leh_frame_common_begin
	.long	Lset5
Leh_frame_common_begin:
	.long	0
	.byte	1
	.asciz	 "zPLR"
	.byte	1
	.byte	120
	.byte	16
	.byte	7
	.byte	155
	.long	___gxx_personality_v0@GOTPCREL+4
	.byte	0x1B // lsda encoding DW_EH_PE_pcrel + DW_EH_PE_sdata4
	.byte	16
	.byte	12
	.byte	7
	.byte	8
	.byte	144
	.byte	1
	.align	3
Leh_frame_common_end:

	.globl	__Z3foov.eh
__Z3foov.eh:
Lset6 = Leh_frame_end1-Leh_frame_begin1
	.long	Lset6
Leh_frame_begin1:
Lset7 = Leh_frame_begin1-Leh_frame_common
	.long	Lset7
Ltmp9:
	.quad	Leh_func_begin1-Ltmp9
Lset8 = Leh_func_end1-Leh_func_begin1
	.quad	Lset8
	.byte	4
	.long	0
	.byte	4
Lset9 = Ltmp0-Leh_func_begin1
	.long	Lset9
	.byte	14
	.byte	16
	.byte	134
	.byte	2
	.byte	4
Lset10 = Ltmp1-Ltmp0
	.long	Lset10
	.byte	13
	.byte	6
	.align	3
Leh_frame_end1:

	.globl	_main.eh
_main.eh:
Lset11 = Leh_frame_end2-Leh_frame_begin2
	.long	Lset11
Leh_frame_begin2:
Lset12 = Leh_frame_begin2-Leh_frame_common
	.long	Lset12
Ltmp10:
	.quad	Leh_func_begin2-Ltmp10
Lset13 = Leh_func_end2-Leh_func_begin2
	.quad	Lset13
	.byte	4
Ltmp11:
	.long	Lexception2-Ltmp11
	.byte	4
Lset14 = Ltmp6-Leh_func_begin2
	.long	Lset14
	.byte	14
	.byte	16
	.byte	134
	.byte	2
	.byte	4
Lset15 = Ltmp7-Ltmp6
	.long	Lset15
	.byte	13
	.byte	6
	.align	3
Leh_frame_end2:


.subsections_via_symbols
