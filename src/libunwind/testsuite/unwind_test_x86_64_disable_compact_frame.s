
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86_64.s -arch x86_64 


	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.quad	_test_no_reg
	.quad	_test_rbx
	.quad	_test_rbx_r12
	.quad	_test_rbx_r14
	.quad	_test_rbx_r12_r13
	.quad	_test_rbx_r12_r13_r14
	.quad	_test_r14_r13_r12_rbx
	.quad	_test_rbx_r12_r13_r14_r15
	.quad	_test_r13_rbx_r14_r15_r12
	.quad	0


	.text

LFE14:
.globl _test_no_reg
_test_no_reg:
LFB13:
	pushq	%rbp
LCFI2:
	movq	%rsp, %rbp
LCFI3:
	call	_uwind_to_main
	leave
	ret
LFE13:


.globl _test_rbx
_test_rbx:
LFB10:
	pushq	%rbp
LCFI8:
	movq	%rsp, %rbp
LCFI9:
	pushq	%rbx
LCFI10:
	subq	$8, %rsp
LCFI11:
	movq	$0, %rbx
	call	_uwind_to_main
	addq	$8, %rsp
	popq	%rbx
	leave
	ret
LFE10:


.globl _test_rbx_r12
_test_rbx_r12:
LFB8:
	pushq	%rbp
LCFI16:
	movq	%rsp, %rbp
LCFI17:
	movq	%rbx, -16(%rbp)
LCFI18:
	movq	%r12, -8(%rbp)
LCFI19:
	subq	$16, %rsp
LCFI20:
	movq	$0, %rbx
	movq	$0, %r12
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %r12
	leave
	ret
LFE8:


.globl _test_rbx_r14
_test_rbx_r14:
LFB8a:
	pushq	%rbp
LCFI16a:
	movq	%rsp, %rbp
LCFI17a:
	movq	%rbx, -16(%rbp)
LCFI18a:
	movq	%r14, -8(%rbp)
LCFI19a:
	subq	$16, %rsp
LCFI20a:
	movq	$0, %rbx
	movq	$0, %r14
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %r14
	leave
	ret
LFE8a:


.globl _test_rbx_r12_r13
_test_rbx_r12_r13:
LFB6:
	pushq	%rbp
LCFI26:
	movq	%rsp, %rbp
LCFI27:
	movq	%rbx, -24(%rbp)
LCFI28:
	movq	%r12, -16(%rbp)
LCFI29:
	movq	%r13, -8(%rbp)
LCFI30:
	subq	$32, %rsp
LCFI31:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	call	_uwind_to_main
	movq	-24(%rbp), %rbx
	movq	-16(%rbp), %r12
	movq	-8(%rbp), %r13
	leave
	ret
LFE6:

.globl _test_rbx_r12_r13_r14
_test_rbx_r12_r13_r14:
LFB4:
	pushq	%rbp
LCFI38:
	movq	%rsp, %rbp
LCFI39:
	movq	%rbx, -32(%rbp)
LCFI40:
	movq	%r12, -24(%rbp)
LCFI41:
	movq	%r13, -16(%rbp)
LCFI42:
	movq	%r14, -8(%rbp)
LCFI43:
	subq	$32, %rsp
LCFI44:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %r12
	movq	16(%rsp), %r13
	movq	24(%rsp), %r14
	leave
	ret
LFE4:

.globl _test_r14_r13_r12_rbx
_test_r14_r13_r12_rbx:
LFB4a:
	pushq	%rbp
LCFI38a:
	movq	%rsp, %rbp
LCFI39a:
	movq	%r14, -32(%rbp)
LCFI40a:
	movq	%r13, -24(%rbp)
LCFI41a:
	movq	%r12, -16(%rbp)
LCFI42a:
	movq	%rbx, -8(%rbp)
LCFI43a:
	subq	$32, %rsp
LCFI44a:
	movq	$0, %r14
	movq	$0, %r13
	movq	$0, %r12
	movq	$0, %rbx
	call	_uwind_to_main
	movq	-32(%rbp), %r14
	movq	-24(%rbp), %r13
	movq	-16(%rbp), %r12
	movq	-8(%rbp), %rbx
	leave
	ret
LFE4a:


.globl _test_rbx_r12_r13_r14_r15
_test_rbx_r12_r13_r14_r15:
LFB3:
	pushq	%rbp
LCFI45:
	movq	%rsp, %rbp
LCFI46:
	movq	%rbx, -40(%rbp)
LCFI47:
	movq	%r12, -32(%rbp)
LCFI48:
	movq	%r13, -24(%rbp)
LCFI49:
	movq	%r14, -16(%rbp)
LCFI50:
	movq	%r15, -8(%rbp)
LCFI51:
	subq	$48, %rsp
LCFI52:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	call	_uwind_to_main
	movq	-40(%rbp), %rbx
	movq	-32(%rbp), %r12
	movq	-24(%rbp), %r13
	movq	-16(%rbp), %r14
	movq	-8(%rbp), %r15
	leave
	ret
LFE3:


.globl _test_r13_rbx_r14_r15_r12
_test_r13_rbx_r14_r15_r12:
LFB3a:
	pushq	%rbp
LCFI45a:
	movq	%rsp, %rbp
LCFI46a:
	movq	%r13, -40(%rbp)
LCFI47a:
	movq	%rbx, -32(%rbp)
LCFI48a:
	movq	%r14, -24(%rbp)
LCFI49a:
	movq	%r15, -16(%rbp)
LCFI50a:
	movq	%r12, -8(%rbp)
LCFI51a:
	subq	$48, %rsp
LCFI52a:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	movq	$0, %r15
	call	_uwind_to_main
	movq	-40(%rbp), %r13
	movq	-32(%rbp), %rbx
	movq	-24(%rbp), %r14
	movq	-16(%rbp), %r15
	movq	-8(%rbp), %r12
	leave
	ret
LFE3a:




	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$0,LECIE1-LSCIE1
	.long L$set$0
LSCIE1:
	.long	0x0
	.byte	0x1
	.ascii "zR\0"
	.byte	0x1
	.byte	0x78
	.byte	0x10
	.byte	0x1
	.byte	0x10
	.byte	0xc
	.byte	0x7
	.byte	0x8
	.byte	0x90	// DW_CFA_offset(ret_addr, -8)
	.byte	0x1
	.byte	0x90	// DW_CFA_offset(ret_addr, -8)
	.byte	0x1		// this repeat is idiom that means don't create compact unwind info
	.align 3
LECIE1:

.globl _test_no_reg.eh
_test_no_reg.eh:
LSFDE3:
	.set L$set$5,LEFDE3-LASFDE3
	.long L$set$5
LASFDE3:
	.long	LASFDE3-EH_frame1
	.quad	LFB13-.
	.set L$set$6,LFE13-LFB13
	.quad L$set$6
	.byte	0x0
	.byte	0x4
	.set L$set$7,LCFI2-LFB13
	.long L$set$7
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$8,LCFI3-LCFI2
	.long L$set$8
	.byte	0xd
	.byte	0x6
	.align 3
LEFDE3:

.globl _test_rbx.eh
_test_rbx.eh:
LSFDE9:
	.set L$set$17,LEFDE9-LASFDE9
	.long L$set$17
LASFDE9:
	.long	LASFDE9-EH_frame1
	.quad	LFB10-.
	.set L$set$18,LFE10-LFB10
	.quad L$set$18
	.byte	0x0
	.byte	0x4
	.set L$set$19,LCFI8-LFB10
	.long L$set$19
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$20,LCFI9-LCFI8
	.long L$set$20
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$21,LCFI11-LCFI9
	.long L$set$21
	.byte	0x83
	.byte	0x3
	.align 3
LEFDE9:

.globl _test_rbx_r12.eh
_test_rbx_r12.eh:
LSFDE13:
	.set L$set$27,LEFDE13-LASFDE13
	.long L$set$27
LASFDE13:
	.long	LASFDE13-EH_frame1
	.quad	LFB8-.
	.set L$set$28,LFE8-LFB8
	.quad L$set$28
	.byte	0x0
	.byte	0x4
	.set L$set$29,LCFI16-LFB8
	.long L$set$29
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$30,LCFI17-LCFI16
	.long L$set$30
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$31,LCFI20-LCFI17
	.long L$set$31
	.byte	0x8c
	.byte	0x3
	.byte	0x83
	.byte	0x4
	.align 3
LEFDE13:

.globl _test_rbx_r14.eh
_test_rbx_r14.eh:
LSFDE13a:
	.set L$set$27a,LEFDE13a-LASFDE13a
	.long L$set$27a
LASFDE13a:
	.long	LASFDE13a-EH_frame1
	.quad	LFB8a-.
	.set L$set$28a,LFE8a-LFB8a
	.quad L$set$28a
	.byte	0x0
	.byte	0x4
	.set L$set$29a,LCFI16a-LFB8a
	.long L$set$29a
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$30a,LCFI17a-LCFI16a
	.long L$set$30a
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$31a,LCFI20a-LCFI17a
	.long L$set$31a
	.byte	0x8e
	.byte	0x3
	.byte	0x83
	.byte	0x4
	.align 3
LEFDE13a:


.globl _test_rbx_r12_r13.eh
_test_rbx_r12_r13.eh:
LSFDE17:
	.set L$set$37,LEFDE17-LASFDE17
	.long L$set$37
LASFDE17:
	.long	LASFDE17-EH_frame1
	.quad	LFB6-.
	.set L$set$38,LFE6-LFB6
	.quad L$set$38
	.byte	0x0
	.byte	0x4
	.set L$set$39,LCFI26-LFB6
	.long L$set$39
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$40,LCFI27-LCFI26
	.long L$set$40
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$41,LCFI31-LCFI27
	.long L$set$41
	.byte	0x8d
	.byte	0x3
	.byte	0x8c
	.byte	0x4
	.byte	0x83
	.byte	0x5
	.align 3
LEFDE17:


.globl _test_rbx_r12_r13_r14.eh
_test_rbx_r12_r13_r14.eh:
LSFDE21:
	.set L$set$47,LEFDE21-LASFDE21
	.long L$set$47
LASFDE21:
	.long	LASFDE21-EH_frame1
	.quad	LFB4-.
	.set L$set$48,LFE4-LFB4
	.quad L$set$48
	.byte	0x0
	.byte	0x4
	.set L$set$49,LCFI38-LFB4
	.long L$set$49
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$50,LCFI39-LCFI38
	.long L$set$50
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$51,LCFI44-LCFI39
	.long L$set$51
	.byte	0x8e
	.byte	0x3
	.byte	0x8d
	.byte	0x4
	.byte	0x8c
	.byte	0x5
	.byte	0x83
	.byte	0x6
	.align 3
LEFDE21:

.globl _test_r14_r13_r12_rbx.eh
_test_r14_r13_r12_rbx.eh:
LSFDE21a:
	.set L$set$47a,LEFDE21a-LASFDE21a
	.long L$set$47a
LASFDE21a:
	.long	LASFDE21a-EH_frame1
	.quad	LFB4a-.
	.set L$set$48a,LFE4a-LFB4a
	.quad L$set$48a
	.byte	0x0
	.byte	0x4
	.set L$set$49a,LCFI38a-LFB4a
	.long L$set$49a
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$50a,LCFI39a-LCFI38a
	.long L$set$50a
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$51a,LCFI44a-LCFI39a
	.long L$set$51a
	.byte	0x8e
	.byte	0x6
	.byte	0x8d
	.byte	0x5
	.byte	0x8c
	.byte	0x4
	.byte	0x83
	.byte	0x3
	.align 3
LEFDE21a:


.globl _test_rbx_r12_r13_r14_r15.eh
_test_rbx_r12_r13_r14_r15.eh:
LSFDE23:
	.set L$set$52,LEFDE23-LASFDE23
	.long L$set$52
LASFDE23:
	.long	LASFDE23-EH_frame1
	.quad	LFB3-.
	.set L$set$53,LFE3-LFB3
	.quad L$set$53
	.byte	0x0
	.byte	0x4
	.set L$set$54,LCFI45-LFB3
	.long L$set$54
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$55,LCFI46-LCFI45
	.long L$set$55
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$56,LCFI52-LCFI46
	.long L$set$56
	.byte	0x8f
	.byte	0x3
	.byte	0x8e
	.byte	0x4
	.byte	0x8d
	.byte	0x5
	.byte	0x8c
	.byte	0x6
	.byte	0x83
	.byte	0x7
	.align 3
LEFDE23:


.globl _test_r13_rbx_r14_r15_r12.eh
_test_r13_rbx_r14_r15_r12.eh:
LSFDE23a:
	.set L$set$52a,LEFDE23a-LASFDE23a
	.long L$set$52a
LASFDE23a:
	.long	LASFDE23a-EH_frame1
	.quad	LFB3a-.
	.set L$set$53a,LFE3a-LFB3a
	.quad L$set$53a
	.byte	0x0
	.byte	0x4
	.set L$set$54a,LCFI45a-LFB3a
	.long L$set$54a
	.byte	0xe
	.byte	0x10
	.byte	0x86
	.byte	0x2
	.byte	0x4
	.set L$set$55a,LCFI46a-LCFI45a
	.long L$set$55a
	.byte	0xd
	.byte	0x6
	.byte	0x4
	.set L$set$56a,LCFI52a-LCFI46a
	.long L$set$56a
	.byte	0x8c
	.byte	0x3
	.byte	0x8f
	.byte	0x4
	.byte	0x8e
	.byte	0x5
	.byte	0x83
	.byte	0x6
	.byte	0x8d
	.byte	0x7
	.align 3
LEFDE23a:



	.subsections_via_symbols
