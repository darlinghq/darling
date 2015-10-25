

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
	subq	$160008, %rsp
LCFI0:
	call	_uwind_to_main
	addq	$160008, %rsp
	ret
LFE13:



.globl _test_rbx
_test_rbx:
LFB10:
	pushq	%rbx
LCFI3:
	subq	$160000, %rsp
LCFI4:
	movq	$0, %rbx
	call	_uwind_to_main
	addq	$160000, %rsp
	popq	%rbx
	ret
LFE10:


.globl _test_rbx_r12
_test_rbx_r12:
LFB8:
	movq	%rbx, -16(%rsp)
LCFI7:
	movq	%r12, -8(%rsp)
LCFI8:
	subq	$160024, %rsp
LCFI9:
	movq	$0, %rbx
	movq	$0, %r12
	call	_uwind_to_main
	movq	160008(%rsp), %rbx
	movq	160016(%rsp), %r12
	addq	$160024, %rsp
	ret
LFE8:


.globl _test_rbx_rbp
_test_rbx_rbp:
LFB7:
	movq	%rbx, -16(%rsp)
LCFI10:
	movq	%rbp, -8(%rsp)
LCFI11:
	subq	$160024, %rsp
LCFI12:
	movq	$0, %rbp
	movq	$0, %rbx
	call	_uwind_to_main
	movq	160008(%rsp), %rbx
	movq	160016(%rsp), %rbp
	addq	$160024, %rsp
	ret
LFE7:


.globl _test_rbx_r12_r13
_test_rbx_r12_r13:
LFB6:
	movq	%rbx, -24(%rsp)
LCFI13:
	movq	%r12, -16(%rsp)
LCFI14:
	movq	%r13, -8(%rsp)
LCFI15:
	subq	$160024, %rsp
LCFI16:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	call	_uwind_to_main
	movq	160000(%rsp), %rbx
	movq	160008(%rsp), %r12
	movq	160016(%rsp), %r13
	addq	$160024, %rsp
	ret
LFE6:


.globl _test_rbx_rbp_r12
_test_rbx_rbp_r12:
LFB9:
	movq	%rbx, -24(%rsp)
	movq	%rbp, -16(%rsp)
	movq	%r12, -8(%rsp)
	subq	$160024, %rsp
LCFI9a:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	call	_uwind_to_main
	movq	160000(%rsp), %rbx
	movq	160008(%rsp), %rbp
	movq	160016(%rsp), %r12
	addq	$160024, %rsp
	ret
LFE9:




.globl _test_rbx_r12_r13_r14
_test_rbx_r12_r13_r14:
LFB4:
	movq	%rbx, -32(%rsp)
LCFI21:
	movq	%r12, -24(%rsp)
LCFI22:
	movq	%r13, -16(%rsp)
LCFI23:
	movq	%r14, -8(%rsp)
LCFI24:
	subq	$160040, %rsp
LCFI25:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	call	_uwind_to_main
	movq	160008(%rsp), %rbx
	movq	160016(%rsp), %r12
	movq	160024(%rsp), %r13
	movq	160032(%rsp), %r14
	addq	$160040, %rsp
	ret
LFE4:


.globl _test_rbx_r12_r13_r14_r15
_test_rbx_r12_r13_r14_r15:
LFB3:
	movq	%rbx, -40(%rsp)
LCFI26:
	movq	%r12, -32(%rsp)
LCFI27:
	movq	%r13, -24(%rsp)
LCFI28:
	movq	%r14, -16(%rsp)
LCFI29:
	movq	%r15, -8(%rsp)
LCFI30:
	subq	$160040, %rsp
LCFI31:
	movq	$0, %rbx
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	movq	$0, %r15
	call	_uwind_to_main
	movq	160000(%rsp), %rbx
	movq	160008(%rsp), %r12
	movq	160016(%rsp), %r13
	movq	160024(%rsp), %r14
	movq	160032(%rsp), %r15
	addq	$160040, %rsp
	ret
LFE3:


.globl _test_rbx_rbp_r12_r13_r14_r15
_test_rbx_rbp_r12_r13_r14_r15:
LFB2:
	movq	%rbx, -48(%rsp)
LCFI32:
	movq	%rbp, -40(%rsp)
LCFI33:
	movq	%r12, -32(%rsp)
LCFI34:
	movq	%r13, -24(%rsp)
LCFI35:
	movq	%r14, -16(%rsp)
LCFI36:
	movq	%r15, -8(%rsp)
LCFI37:
	subq	$160072, %rsp
LCFI38:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	movq	$0, %r15
	call	_uwind_to_main
	movq	160024(%rsp), %rbx
	movq	160032(%rsp), %rbp
	movq	160040(%rsp), %r12
	movq	160048(%rsp), %r13
	movq	160056(%rsp), %r14
	movq	160064(%rsp), %r15
	addq	$160072, %rsp
	ret
LFE2:


.globl _test_rbx_rbp_r12_r13
_test_rbx_rbp_r12_r13:
LFB457:
	movq	%rbx, -32(%rsp)
LCFI18:
	movq	%rbp, -24(%rsp)
LCFI19:
	movq	%r12, -16(%rsp)
	movq	%r13, -8(%rsp)
	subq	$160040, %rsp
LCFI20:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	movq	$0, %r13
	call	_uwind_to_main
	movq	160008(%rsp), %rbx
	movq	160016(%rsp), %rbp
	movq	160024(%rsp), %r12
	movq	160032(%rsp), %r13
	addq	$160040, %rsp
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
	subq	$160040, %rsp
LFB17a:
	movq	$0, %rbx
	movq	$0, %rbp
	movq	$0, %r12
	movq	$0, %r13
	movq	$0, %r14
	call	_uwind_to_main
	movq	160000(%rsp), %rbx
	movq	160008(%rsp), %rbp
	movq	160016(%rsp), %r12
	movq	160024(%rsp), %r13
	movq	160032(%rsp), %r14
	addq	$160040, %rsp
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
	.byte	0x90,0xe2,0x9	# uleb128 0x27110
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
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$15,LCFI4-LCFI3
	.long L$set$15
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x90,0xe2,0x9	# uleb128 0x27110
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x2	# uleb128 0x2
	.align 3
LEFDE9:


.globl _test_rbx_r12.eh
_test_rbx_r12.eh:
LSFDE13:
	.set L$set$20,LEFDE13-LASFDE13
	.long L$set$20	# FDE Length
LASFDE13:
	.long	LASFDE13-EH_frame1	# FDE CIE offset
	.quad	LFB8-.	# FDE initial location
	.set L$set$21,LFE8-LFB8
	.quad L$set$21	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$22,LCFI9-LFB8
	.long L$set$22
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xa0,0xe2,0x9	# uleb128 0x27120
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x2	# uleb128 0x2
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.align 3
LEFDE13:


.globl _test_rbx_rbp.eh
_test_rbx_rbp.eh:
LSFDE14:
	.set L$set$23,LEFDE14-LASFDE14
	.long L$set$23	# FDE Length
LASFDE14:
	.long	LASFDE14-EH_frame1	# FDE CIE offset
	.quad	LFB7-.	# FDE initial location
	.set L$set$24,LFE7-LFB7
	.quad L$set$24	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$25,LCFI12-LFB7
	.long L$set$25
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xa0,0xe2,0x9	# uleb128 0x27120
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x2	# uleb128 0x2
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.align 3
LEFDE14:


.globl _test_rbx_r12_r13.eh
_test_rbx_r12_r13.eh:
LSFDE17:
	.set L$set$26,LEFDE17-LASFDE17
	.long L$set$26	# FDE Length
LASFDE17:
	.long	LASFDE17-EH_frame1	# FDE CIE offset
	.quad	LFB6-.	# FDE initial location
	.set L$set$27,LFE6-LFB6
	.quad L$set$27	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$28,LCFI16-LFB6
	.long L$set$28
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xa0,0xe2,0x9	# uleb128 0x27120
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
	.set L$set$32,LEFDE21-LASFDE21
	.long L$set$32	# FDE Length
LASFDE21:
	.long	LASFDE21-EH_frame1	# FDE CIE offset
	.quad	LFB4-.	# FDE initial location
	.set L$set$33,LFE4-LFB4
	.quad L$set$33	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34,LCFI25-LFB4
	.long L$set$34
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xb0,0xe2,0x9	# uleb128 0x27130
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
	.set L$set$35,LEFDE23-LASFDE23
	.long L$set$35	# FDE Length
LASFDE23:
	.long	LASFDE23-EH_frame1	# FDE CIE offset
	.quad	LFB3-.	# FDE initial location
	.set L$set$36,LFE3-LFB3
	.quad L$set$36	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$37,LCFI31-LFB3
	.long L$set$37
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xb0,0xe2,0x9	# uleb128 0x27130
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
	.set L$set$38,LEFDE25-LASFDE25
	.long L$set$38	# FDE Length
LASFDE25:
	.long	LASFDE25-EH_frame1	# FDE CIE offset
	.quad	LFB2-.	# FDE initial location
	.set L$set$39,LFE2-LFB2
	.quad L$set$39	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$40,LCFI38-LFB2
	.long L$set$40
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xd0,0xe2,0x9	# uleb128 0x27150
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


.globl _test_rbx_rbp_r12_r13.eh
_test_rbx_rbp_r12_r13.eh:
LSFDE11:
	.set L$set$17a,LEFDE11-LASFDE11
	.long L$set$17a	# FDE Length
LASFDE11:
	.long	LASFDE11-EH_frame1	# FDE CIE offset
	.quad	LFB457-.	# FDE initial location
	.set L$set$18a,LFE457-LFB457
	.quad L$set$18a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$19a,LCFI20-LFB457
	.long L$set$19a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xb0,0xe2,0x9	# uleb128 0x27130
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x2	# uleb128 0x2
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 3
LEFDE11:


.globl _test_rbx_rbp_r12_r13_r14.eh
_test_rbx_rbp_r12_r13_r14.eh:
LSFDE7:
	.set L$set$35a,LEFDE7-LASFDE7
	.long L$set$35a	# FDE Length
LASFDE7:
	.long	LASFDE7-EH_frame1	# FDE CIE offset
	.quad	LFB17-.	# FDE initial location
	.set L$set$36a,LFE17-LFB17
	.quad L$set$36a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$37a,LFB17a-LFB17
	.long L$set$37a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xb0,0xe2,0x9	# uleb128 0x27130
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
LEFDE7:


.globl _test_rbx_rbp_r12.eh
_test_rbx_rbp_r12.eh:
LSFDE19:
	.set L$set$26a,LEFDE19-LASFDE19
	.long L$set$26a	# FDE Length
LASFDE19:
	.long	LASFDE19-EH_frame1	# FDE CIE offset
	.quad	LFB9-.	# FDE initial location
	.set L$set$27a,LFE9-LFB9
	.quad L$set$27a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$28a,LCFI9a-LFB9
	.long L$set$28a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0xa0,0xe2,0x9	# uleb128 0x27120
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x2	# uleb128 0x2
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x3	# uleb128 0x3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 3
LEFDE19:



	.subsections_via_symbols
