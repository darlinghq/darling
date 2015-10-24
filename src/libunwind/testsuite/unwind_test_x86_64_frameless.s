
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86_64.s -arch x86_64 
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86_64.s -arch x86_64 -Wl,-no_compact_unwind

	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.quad	_test_no_reg
	.quad	_test_rbx
	.quad	_test_rbx_r12
	.quad	_test_rbx_r12_r13
	.quad	_test_rbx_r12_r13_r14
	.quad	_test_rbx_r12_r13_r14_r15
	.quad	_test_rbx_rbp
	.quad	_test_rbx_rbp_r12
	.quad	_test_rbx_rbp_r12_r13
	.quad	_test_rbx_rbp_r12_r13_r14
	.quad	_test_rbx_rbp_r12_r13_r14_r15
	.quad	0


	.text

.globl _test_no_reg
_test_no_reg:
LFB13:
	subq	$8, %rsp
LCFI0:
	call	_uwind_to_main
	addq	$8, %rsp
	ret
LFE13:


.globl _test_rbx
_test_rbx:
LFB10:
	pushq	%rbx
LCFI3:
	movq	$0, %rbx
	call	_uwind_to_main
	popq	%rbx
	ret
LFE10:


.globl _test_rbx_r12
_test_rbx_r12:
LFB8:
	movq	%rbx, -16(%rsp)
LCFI5:
	movq	%r12, -8(%rsp)
LCFI6:
	subq	$24, %rsp
LCFI7:
	movq	$0, %rbx
	movq	$0, %r12
	call	_uwind_to_main
	movq	8(%rsp), %rbx
	movq	16(%rsp), %r12
	addq	$24, %rsp
	ret
LFE8:


.globl _test_rbx_rbp
_test_rbx_rbp:
LFB7:
	movq	%rbx, -16(%rsp)
LCFI8:
	movq	%rbp, -8(%rsp)
LCFI9:
	subq	$24, %rsp
LCFI10:
	movq	$0, %rbp
	movq	$0, %rbx
	call	_uwind_to_main
	movq	8(%rsp), %rbx
	movq	16(%rsp), %rbp
	addq	$24, %rsp
	ret
LFE7:


.globl _test_rbx_r12_r13
_test_rbx_r12_r13:
LFB6:
	movq	%rbx, -24(%rsp)
LCFI11:
	movq	%r12, -16(%rsp)
LCFI12:
	movq	%r13, -8(%rsp)
LCFI13:
	subq	$24, %rsp
LCFI14:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %r12
	movq	16(%rsp), %r13
	addq	$24, %rsp
	ret
LFE6:



.globl _test_rbx_r12_r13_r14
_test_rbx_r12_r13_r14:
LFB4:
	movq	%rbx, -32(%rsp)
LCFI19:
	movq	%r12, -24(%rsp)
LCFI20:
	movq	%r13, -16(%rsp)
LCFI21:
	movq	%r14, -8(%rsp)
LCFI22:
	subq	$40, %rsp
LCFI23:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	call	_uwind_to_main
	movq	8(%rsp), %rbx
	movq	16(%rsp), %r12
	movq	24(%rsp), %r13
	movq	32(%rsp), %r14
	addq	$40, %rsp
	ret
LFE4:


.globl _test_rbx_r12_r13_r14_r15
_test_rbx_r12_r13_r14_r15:
LFB3:
	movq	%rbx, -40(%rsp)
LCFI24:
	movq	%r12, -32(%rsp)
LCFI25:
	movq	%r13, -24(%rsp)
LCFI26:
	movq	%r14, -16(%rsp)
LCFI27:
	movq	%r15, -8(%rsp)
LCFI28:
	subq	$40, %rsp
LCFI29:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	movq	$0, %r15
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %r12
	movq	16(%rsp), %r13
	movq	24(%rsp), %r14
	movq	32(%rsp), %r15
	addq	$40, %rsp
	ret
LFE3:


.globl _test_rbx_rbp_r12_r13_r14_r15
_test_rbx_rbp_r12_r13_r14_r15:
LFB2:
	movq	%rbx, -48(%rsp)
LCFI30:
	movq	%rbp, -40(%rsp)
LCFI31:
	movq	%r12, -32(%rsp)
LCFI32:
	movq	%r13, -24(%rsp)
LCFI33:
	movq	%r14, -16(%rsp)
LCFI34:
	movq	%r15, -8(%rsp)
LCFI35:
	subq	$72, %rsp
LCFI36:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	movq	$0, %r15
	call	_uwind_to_main
	movq	24(%rsp), %rbx
	movq	32(%rsp), %rbp
	movq	40(%rsp), %r12
	movq	48(%rsp), %r13
	movq	56(%rsp), %r14
	movq	64(%rsp), %r15
	addq	$72, %rsp
	ret
LFE2:



.globl _test_rbx_rbp_r12
_test_rbx_rbp_r12:
LFB11:
	movq	%rbx, -24(%rsp)
	movq	%rbp, -16(%rsp)
	movq	%r12, -8(%rsp)
	subq	$24, %rsp
LCFI11a:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
	addq	$24, %rsp
	ret
LFE11:



.globl _test_rbx_rbp_r12_r13
_test_rbx_rbp_r12_r13:
LFB457:
	movq	%rbx, -32(%rsp)
	movq	%rbp, -24(%rsp)
	movq	%r12, -16(%rsp)
	movq	%r13, -8(%rsp)
	subq	$40, %rsp
LFB457a:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	movq	$0, %r13
	call	_uwind_to_main
	movq	8(%rsp), %rbx
	movq	16(%rsp), %rbp
	movq	24(%rsp), %r12
	movq	32(%rsp), %r13
	addq	$40, %rsp
	ret
LFE457:


.globl _test_rbx_rbp_r12_r13_r14
_test_rbx_rbp_r12_r13_r14:
LFB17:
	movq	%rbx, -40(%rsp)
	movq	%rbp, -32(%rsp)
	movq	%r12, -24(%rsp)
	movq	%r13, -16(%rsp)
	movq	%r14, -8(%rsp)
	subq	$40, %rsp
LFB17a:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
	movq	24(%rsp), %r13
	movq	32(%rsp), %r14
	addq	$40, %rsp
	ret
LFE17:



	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$0,LECIE1-LSCIE1
	.long L$set$0	# Length of Common Information Entry
LSCIE1:
	.long	0x0	# CIE Identifier Tag
	.byte	0x1	# CIE Version
	.ascii "zR\0"	# CIE Augmentation
	.byte	0x1	# uleb128 0x1; CIE Code Alignment Factor
	.byte	0x78	# sleb128 -8; CIE Data Alignment Factor
	.byte	0x10	# CIE RA Column
	.byte	0x1	# uleb128 0x1; Augmentation size
	.byte	0x10	# FDE Encoding (pcrel)
	.byte	0xc	# DW_CFA_def_cfa
	.byte	0x7	# uleb128 0x7
	.byte	0x8	# uleb128 0x8
	.byte	0x90	# DW_CFA_offset, column 0x10
	.byte	0x1	# uleb128 0x1
	.align 3
LECIE1:

.globl _test_no_reg.eh
_test_no_reg.eh:
LSFDE3:
	.set L$set$3,LEFDE3-LASFDE3
	.long L$set$3	# FDE Length
LASFDE3:
	.long	LASFDE3-EH_frame1	# FDE CIE offset
	.quad	LFB13-.	# FDE initial location
	.set L$set$4,LFE13-LFB13
	.quad L$set$4	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$5,LCFI0-LFB13
	.long L$set$5
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x10	# uleb128 0x10
	.align 3
LEFDE3:

.globl _test_rbx.eh
_test_rbx.eh:
LSFDE9:
	.set L$set$12,LEFDE9-LASFDE9
	.long L$set$12	# FDE Length
LASFDE9:
	.long	LASFDE9-EH_frame1	# FDE CIE offset
	.quad	LFB10-.	# FDE initial location
	.set L$set$13,LFE10-LFB10
	.quad L$set$13	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$14,LCFI3-LFB10
	.long L$set$14
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x10	# uleb128 0x10
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x2	# uleb128 0x2
	.align 3
LEFDE9:

.globl _test_rbx_r12.eh
_test_rbx_r12.eh:
LSFDE13:
	.set L$set$18,LEFDE13-LASFDE13
	.long L$set$18	# FDE Length
LASFDE13:
	.long	LASFDE13-EH_frame1	# FDE CIE offset
	.quad	LFB8-.	# FDE initial location
	.set L$set$19,LFE8-LFB8
	.quad L$set$19	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$20,LCFI7-LFB8
	.long L$set$20
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x2	# uleb128 0x2
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.align 3
LEFDE13:


.globl _test_rbx_rbp.eh
_test_rbx_rbp.eh:
LSFDE14:
	.set L$set$21,LEFDE14-LASFDE14
	.long L$set$21	# FDE Length
LASFDE14:
	.long	LASFDE14-EH_frame1	# FDE CIE offset
	.quad	LFB7-.	# FDE initial location
	.set L$set$22,LFE7-LFB7
	.quad L$set$22	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$23,LCFI10-LFB7
	.long L$set$23
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x2	# uleb128 0x2
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.align 3
LEFDE14:


.globl _test_rbx_r12_r13.eh
_test_rbx_r12_r13.eh:
LSFDE17:
	.set L$set$24,LEFDE17-LASFDE17
	.long L$set$24	# FDE Length
LASFDE17:
	.long	LASFDE17-EH_frame1	# FDE CIE offset
	.quad	LFB6-.	# FDE initial location
	.set L$set$25,LFE6-LFB6
	.quad L$set$25	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$26,LCFI14-LFB6
	.long L$set$26
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x2	# uleb128 0x2
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x3	# uleb128 0x3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 3
LEFDE17:

.globl _test_rbx_r12_r13_r14.eh
_test_rbx_r12_r13_r14.eh:
LSFDE21:
	.set L$set$30,LEFDE21-LASFDE21
	.long L$set$30	# FDE Length
LASFDE21:
	.long	LASFDE21-EH_frame1	# FDE CIE offset
	.quad	LFB4-.	# FDE initial location
	.set L$set$31,LFE4-LFB4
	.quad L$set$31	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32,LCFI23-LFB4
	.long L$set$32
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x30	# uleb128 0x30
	.byte	0x8e	# DW_CFA_offset, column 0xe
	.byte	0x2	# uleb128 0x2
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x3	# uleb128 0x3
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 3
LEFDE21:


.globl _test_rbx_r12_r13_r14_r15.eh
_test_rbx_r12_r13_r14_r15.eh:
LSFDE23:
	.set L$set$33,LEFDE23-LASFDE23
	.long L$set$33	# FDE Length
LASFDE23:
	.long	LASFDE23-EH_frame1	# FDE CIE offset
	.quad	LFB3-.	# FDE initial location
	.set L$set$34,LFE3-LFB3
	.quad L$set$34	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$35,LCFI29-LFB3
	.long L$set$35
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x30	# uleb128 0x30
	.byte	0x8f	# DW_CFA_offset, column 0xf
	.byte	0x2	# uleb128 0x2
	.byte	0x8e	# DW_CFA_offset, column 0xe
	.byte	0x3	# uleb128 0x3
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x4	# uleb128 0x4
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x5	# uleb128 0x5
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x6	# uleb128 0x6
	.align 3
LEFDE23:


.globl _test_rbx_rbp_r12_r13_r14_r15.eh
_test_rbx_rbp_r12_r13_r14_r15.eh:
LSFDE25:
	.set L$set$36,LEFDE25-LASFDE25
	.long L$set$36	# FDE Length
LASFDE25:
	.long	LASFDE25-EH_frame1	# FDE CIE offset
	.quad	LFB2-.	# FDE initial location
	.set L$set$37,LFE2-LFB2
	.quad L$set$37	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$38,LCFI36-LFB2
	.long L$set$38
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x50	# uleb128 0x50
	.byte	0x8f	# DW_CFA_offset, column 0xf
	.byte	0x2	# uleb128 0x2
	.byte	0x8e	# DW_CFA_offset, column 0xe
	.byte	0x3	# uleb128 0x3
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x4	# uleb128 0x4
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x5	# uleb128 0x5
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x6	# uleb128 0x6
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x7	# uleb128 0x7
	.align 3
LEFDE25:


.globl _test_rbx_rbp_r12.eh
_test_rbx_rbp_r12.eh:
LSFDE11:
	.set L$set$24a,LEFDE11-LASFDE11
	.long L$set$24a	# FDE Length
LASFDE11:
	.long	LASFDE11-EH_frame1	# FDE CIE offset
	.quad	LFB11-.	# FDE initial location
	.set L$set$25a,LFE11-LFB11
	.quad L$set$25a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$26a,LCFI11a-LFB11
	.long L$set$26a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x2	# uleb128 0x2
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x3	# uleb128 0x3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 3
LEFDE11:


.globl _test_rbx_rbp_r12_r13.eh
_test_rbx_rbp_r12_r13.eh:
	.set L$set$30a,LEFDE457-LASFDE457
	.long L$set$30a	# FDE Length
LASFDE457:
	.long	LASFDE457-EH_frame1	# FDE CIE offset
	.quad	LFB457-.	# FDE initial location
	.set L$set$31a,LFE457-LFB457
	.quad L$set$31a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32a,LCFI23-LFB4
	.long L$set$32a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x30	# uleb128 0x30
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x2	# uleb128 0x2
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 3
LEFDE457:


.globl _test_rbx_rbp_r12_r13_r14.eh
_test_rbx_rbp_r12_r13_r14.eh:
	.set L$set$33a,LEFDE117-LASFDE117
	.long L$set$33a	# FDE Length
LASFDE117:
	.long	LASFDE117-EH_frame1	# FDE CIE offset
	.quad	LFB17-.	# FDE initial location
	.set L$set$34a,LFE17-LFB17
	.quad L$set$34a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$35a,LCFI29-LFB3
	.long L$set$35a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x30	# uleb128 0x30
	.byte	0x8e	# DW_CFA_offset, column 0xe
	.byte	0x2	# uleb128 0x2
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x3	# uleb128 0x3
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x4	# uleb128 0x4
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x5	# uleb128 0x5
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x6	# uleb128 0x6
	.align 3
LEFDE117:





	.subsections_via_symbols
