
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions -Wl,-no_compact_unwind


	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.long	_test_ebx_wrong_slot
	.long	_test_esi_in_ebx
	.long	_test_edi_pad_ebx
	.long	_test_coal_ebx_esi_edi
	.long	Ltest_edi_pad_ebx
	.long	0


	.text

.globl _test_ebx_wrong_slot
_test_ebx_wrong_slot:
LFB13:
	pushl	%ebp
LCFI2:
	movl	%esp, %ebp
LCFI3:
	subl	$4, %esp
	pushl	%ebx
LCFI4:
	subl	$16, %esp
LCFI5:
	movl	$0, %ebx
	call	_uwind_to_main
	addl	$16, %esp
	popl	%ebx
	addl	$4, %esp
	leave
	ret
LFE13:


.globl _test_esi_in_ebx
_test_esi_in_ebx:
LFB9:
	pushl	%ebp
LCFI17:
	movl	%esp, %ebp
LCFI18:
	subl	$24, %esp
LCFI19:
	movl	%ebx, -8(%ebp)
LCFI20:
	movl	%esi, %ebx
LCFI21:
	movl	$0, %esi
	call	_uwind_to_main
	movl	%ebx, %esi
	movl	-8(%ebp), %ebx
	leave
	ret
LFE9:



.globl _test_edi_pad_ebx
_test_edi_pad_ebx:
LFB7a:
	pushl	%ebp
LCFI27a:
	movl	%esp, %ebp
LCFI28a:
	subl	$140, %esp
LCFI29a:
	movl	%edi, -44(%ebp)
LCFI30a:
LCFI31a:
	movl	%ebx, -4(%ebp)
LCFI32a:
	movl	$0, %ebx
	movl	$0, %edi
	call	_uwind_to_main
	movl	-44(%ebp), %edi
	movl	-4(%ebp), %ebx
	leave
	ret
LFE7a:

Ltest_edi_pad_ebx:
LFB7b:
	pushl	%ebp
LCFI27b:
	movl	%esp, %ebp
LCFI28b:
	subl	$140, %esp
LCFI29b:
	movl	%edi, -44(%ebp)
LCFI30b:
LCFI31b:
	movl	%ebx, -4(%ebp)
LCFI32b:
	movl	$0, %ebx
	movl	$0, %edi
	call	_uwind_to_main
	movl	-44(%ebp), %edi
	movl	-4(%ebp), %ebx
	leave
	ret
LFE7b:



	.section __TEXT,__textcoal_nt,coalesced,pure_instructions
	.globl _test_coal_ebx_esi_edi
	.weak_definition _test_coal_ebx_esi_edi
_test_coal_ebx_esi_edi:
LFB7:
	pushl	%ebp
LCFI27:
	movl	%esp, %ebp
LCFI28:
	subl	$40, %esp
LCFI29:
	movl	%ebx, -12(%ebp)
LCFI30:
	movl	%esi, -8(%ebp)
LCFI31:
	movl	%edi, -4(%ebp)
LCFI32:
	movl	$0, %ebx
	movl	$0, %edi
	movl	$0, %esi
	call	_uwind_to_main
	movl	-12(%ebp), %ebx
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	leave
	ret
LFE7:


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



.globl _test_ebx_wrong_slot.eh
_test_ebx_wrong_slot.eh:
LSFDE3:
	.set L$set$1,LEFDE3-LASFDE3
	.long L$set$1	# FDE Length
LASFDE3:
	.long	LASFDE3-EH_frame1	# FDE CIE offset
	.long	LFB13-.	# FDE initial location
	.set L$set$2,LFE13-LFB13
	.long L$set$2	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$3,LCFI2-LFB13
	.long L$set$3
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$4,LCFI3-LCFI2
	.long L$set$4
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$5,LCFI5-LCFI3
	.long L$set$5
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE3:


.globl _test_esi_in_ebx.eh
_test_esi_in_ebx.eh:
LSFDE11:
	.set L$set$20,LEFDE11-LASFDE11
	.long L$set$20	# FDE Length
LASFDE11:
	.long	LASFDE11-EH_frame1	# FDE CIE offset
	.long	LFB9-.	# FDE initial location
	.set L$set$21,LFE9-LFB9
	.long L$set$21	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$22,LCFI17-LFB9
	.long L$set$22
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$23,LCFI18-LCFI17
	.long L$set$23
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$24,LCFI21-LCFI18
	.long L$set$24
	.byte	0x09, 0x06, 0x03 # DW_CFA_register 6, 3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE11:


.globl _test_edi_pad_ebx.eh
_test_edi_pad_ebx.eh:
LSFDE15a:
	.set L$set$30,LEFDE15a-LASFDE15a
	.long L$set$30	# FDE Length
LASFDE15a:
	.long	LASFDE15a-EH_frame1	# FDE CIE offset
	.long	LFB7a-.	# FDE initial location
	.set L$set$31,LFE7a-LFB7a
	.long L$set$31	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32,LCFI27a-LFB7a
	.long L$set$32
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$33,LCFI28a-LCFI27a
	.long L$set$33
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34,LCFI32a-LCFI28a
	.long L$set$34
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0xD	# uleb128 0xD
	.align 2
LEFDE15a:

ltest_edi_pad_ebx.eh:
LSFDE15b:
	.set L$set$30a,LEFDE15b-LASFDE15b
	.long L$set$30a	# FDE Length
LASFDE15b:
	.long	LASFDE15b-EH_frame1	# FDE CIE offset
	.long	LFB7b-.	# FDE initial location
	.set L$set$31a,LFE7b-LFB7b
	.long L$set$31a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32a,LCFI27b-LFB7b
	.long L$set$32a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$33a,LCFI28b-LCFI27b
	.long L$set$33a
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34a,LCFI32b-LCFI28b
	.long L$set$34a
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0xD	# uleb128 0xD
	.align 2
LEFDE15b:

.globl _test_coal_ebx_esi_edi.eh
_test_coal_ebx_esi_edi.eh:
LSFDE15:
	.set L$set$30b,LEFDE15-LASFDE15
	.long L$set$30b	# FDE Length
LASFDE15:
	.long	LASFDE15-EH_frame1	# FDE CIE offset
	.long	LFB7-.	# FDE initial location
	.set L$set$31b,LFE7-LFB7
	.long L$set$31b	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32b,LCFI27-LFB7
	.long L$set$32b
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$33b,LCFI28-LCFI27
	.long L$set$33b
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34b,LCFI32-LCFI28
	.long L$set$34b
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 2
LEFDE15:



	.subsections_via_symbols
