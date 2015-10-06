
#
# This is a generic function to test that restoring registers during unwinding work correctly.
# 

	.text
.globl _unwind_tester
_unwind_tester:
LFB2:
	pushl	%ebp
LCFI0:
	movl	%esp, %ebp
LCFI1:
	subl	$56, %esp
LCFI2:
	movl	%ebx, -12(%ebp)
LCFI3:
	movl	%esi, -8(%ebp)
LCFI4:
	movl	%edi, -4(%ebp)
LCFI5:
	# load magic values into non-volatile registers
	movl	$0x12344321, %ebx
	movl	$0x56788765, %esi
	movl	$0xABCDDCBA, %edi
	# call test function which will invoke unwinder which "returns" here
	call	*8(%ebp)
	# verify that non-volatile registers still contain magic values
	cmpl	$0x12344321, %ebx
	jne	L2
	cmpl	$0x56788765, %esi
	jne	L2
	cmpl	$0xABCDDCBA, %edi
	jne	L2
	movl	$0, %eax
	jmp		L3
L2: movl	$1, %eax
L3: 		
	movl	-12(%ebp), %ebx
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	leave
	ret
LFE2:



	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$0,LECIE1-LSCIE1
	.long L$set$0	# Length of Common Information Entry
LSCIE1:
	.long	0x0	# CIE Identifier Tag
	.byte	0x1	# CIE Version
	.ascii "zR\0"	# CIE Augmentation
	.byte	0x1	# uleb128 0x1; CIE Code Alignment Factor
	.byte	0x7c	# sleb128 -4; CIE Data Alignment Factor
	.byte	0x8	# CIE RA Column
	.byte	0x1	# uleb128 0x1; Augmentation size
	.byte	0x10	# FDE Encoding (pcrel)
	.byte	0xc	# DW_CFA_def_cfa
	.byte	0x5	# uleb128 0x5
	.byte	0x4	# uleb128 0x4
	.byte	0x88	# DW_CFA_offset, column 0x8
	.byte	0x1	# uleb128 0x1
	.align 2
LECIE1:

.globl _unwind_tester.eh
_unwind_tester.eh:
LSFDE1:
	.set L$set$1,LEFDE1-LASFDE1
	.long L$set$1	# FDE Length
LASFDE1:
	.long	LASFDE1-EH_frame1	# FDE CIE offset
	.long	LFB2-.	# FDE initial location
	.set L$set$2,LFE2-LFB2
	.long L$set$2	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$3,LCFI0-LFB2
	.long L$set$3
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$4,LCFI1-LCFI0
	.long L$set$4
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$5,LCFI5-LCFI1
	.long L$set$5
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 2
LEFDE1:


