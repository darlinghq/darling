
#
# This is a generic function to test that restoring registers during unwinding work correctly.
# 

	.text
.globl _unwind_tester
_unwind_tester:
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
	# load magic values into non-volatile registers
	movq	$0x1234567887654321, %rbx
	movq	$0x02468ACEECA86420, %r12
	movq	$0x13579BDFFDB97531, %r13
	movq	$0x1122334455667788, %r14
	movq	$0x0022446688AACCEE, %r15
	# call test function which will invoke unwinder which "returns" here
	call	*%rdi
	# verify that non-volatile registers still contain magic values
	movq	$0x1234567887654321, %rax
	cmpq	%rax, %rbx
	jne	L2
	movq	$0x02468ACEECA86420, %rax
	cmpq	%rax, %r12
	jne	L2
	movq	$0x13579BDFFDB97531, %rax
	cmpq	%rax, %r13
	jne	L2
	movq	$0x1122334455667788, %rax
	cmpq	%rax, %r14
	jne	L2
	movq	$0x0022446688AACCEE, %rax
	cmpq	%rax, %r15
	jne	L2
	movl	$0, %eax
	jmp		L3
L2: movl	$1, %eax
L3: 		
	movq	-40(%rbp), %rbx
	movq	-32(%rbp), %r12
	movq	-24(%rbp), %r13
	movq	-16(%rbp), %r14
	movq	-8(%rbp), %r15
	leave
	ret
LFE3:


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

.globl _unwind_tester.eh
_unwind_tester.eh:
LSFDE23:
	.set L$set$52,LEFDE23-LASFDE23
	.long L$set$52	# FDE Length
LASFDE23:
	.long	LASFDE23-EH_frame1	# FDE CIE offset
	.quad	LFB3-.	# FDE initial location
	.set L$set$53,LFE3-LFB3
	.quad L$set$53	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$54,LCFI45-LFB3
	.long L$set$54
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x10	# uleb128 0x10
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$55,LCFI46-LCFI45
	.long L$set$55
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x6	# uleb128 0x6
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$56,LCFI52-LCFI46
	.long L$set$56
	.byte	0x8f	# DW_CFA_offset, column 0xf
	.byte	0x3	# uleb128 0x3
	.byte	0x8e	# DW_CFA_offset, column 0xe
	.byte	0x4	# uleb128 0x4
	.byte	0x8d	# DW_CFA_offset, column 0xd
	.byte	0x5	# uleb128 0x5
	.byte	0x8c	# DW_CFA_offset, column 0xc
	.byte	0x6	# uleb128 0x6
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x7	# uleb128 0x7
	.align 3
LEFDE23:



