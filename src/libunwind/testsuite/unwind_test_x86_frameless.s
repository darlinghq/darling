

# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions -Wl,-no_compact_unwind

	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.long	_test_no_reg
	.long	_test_ebx
	.long	_test_esi
	.long	_test_edi
	.long	_test_ebx_esi
	.long	_test_esi_edi
	.long	_test_esi_edi_ebp
	.long	_test_ebx_esi_edi
	.long	_test_ebx_esi_edi_ebp
	.long	_test_esi_ebp_ebx_edi
	.long	0


	.text

.globl _test_ebx
_test_ebx:
LFB13:
	pushl	%ebx
LCFI0:
	subl	$24, %esp
LCFI1:
	movl	$0, %ebx
	call	_uwind_to_main
	addl	$24, %esp
	popl	%ebx
	ret
LFE13:


.globl _test_no_reg
_test_no_reg:
LFB12:
	subl	$28, %esp
LCFI2:
	call	_uwind_to_main
	addl	$28, %esp
	ret
LFE12:


.globl _test_esi
_test_esi:
LFB10:
	pushl	%esi
LCFI5:
	subl	$24, %esp
LCFI6:
	movl	$0, %esi
	call	_uwind_to_main
	addl	$24, %esp
	popl	%esi
	ret
LFE10:


.globl _test_edi
_test_edi:
LFB11:
	pushl	%edi
LCFI3:
	subl	$24, %esp
LCFI4:
	movl	$0, %edi
	call	_uwind_to_main
	addl	$24, %esp
	popl	%edi
	ret
LFE11:


.globl _test_ebx_esi
_test_ebx_esi:
LFB9:
	subl	$28, %esp
LCFI7:
	movl	%ebx, 20(%esp)
LCFI8:
	movl	%esi, 24(%esp)
LCFI9:
	movl	$0, %ebx
	movl	$0, %esi
	call	_uwind_to_main
	movl	20(%esp), %ebx
	movl	24(%esp), %esi
	addl	$28, %esp
	ret
LFE9:


.globl _test_esi_edi
_test_esi_edi:
LFB8:
	subl	$28, %esp
LCFI10:
	movl	%esi, 20(%esp)
LCFI11:
	movl	%edi, 24(%esp)
LCFI12:
	movl	$0, %esi
	movl	$0, %edi
	call	_uwind_to_main
	movl	20(%esp), %esi
	movl	24(%esp), %edi
	addl	$28, %esp
	ret
LFE8:



.globl _test_ebx_esi_edi
_test_ebx_esi_edi:
LFB7:
	subl	$28, %esp
LCFI13:
	movl	%ebx, 16(%esp)
LCFI14:
	movl	%esi, 20(%esp)
LCFI15:
	movl	%edi, 24(%esp)
LCFI16:
	movl	$0, %ebx
	movl	$0, %esi
	movl	$0, %edi
	call	_uwind_to_main
	movl	16(%esp), %ebx
	movl	20(%esp), %esi
	movl	24(%esp), %edi
	addl	$28, %esp
	ret
LFE7:


.globl _test_esi_edi_ebp
_test_esi_edi_ebp:
LFB6:
	subl	$28, %esp
LCFI17:
	movl	%esi, 16(%esp)
LCFI18:
	movl	%edi, 20(%esp)
LCFI19:
	movl	%ebp, 24(%esp)
LCFI20:
	movl	$0, %esi
	movl	$0, %edi
	movl	$0, %ebp
	call	_uwind_to_main
	movl	16(%esp), %esi
	movl	20(%esp), %edi
	movl	24(%esp), %ebp
	addl	$28, %esp
	ret
LFE6:


.globl _test_ebx_esi_edi_ebp
_test_ebx_esi_edi_ebp:
LFB5:
	subl	$60, %esp
LCFI21:
	movl	%ebx, 44(%esp)
LCFI22:
	movl	%esi, 48(%esp)
LCFI23:
	movl	%edi, 52(%esp)
LCFI24:
	movl	%ebp, 56(%esp)
LCFI25:
	movl	$0, %ebx
	movl	$0, %esi
	movl	$0, %edi
	movl	$0, %ebp
	call	_uwind_to_main
	movl	44(%esp), %ebx
	movl	48(%esp), %esi
	movl	52(%esp), %edi
	movl	56(%esp), %ebp
	addl	$60, %esp
	ret
LFE5:

.globl _test_esi_ebp_ebx_edi
_test_esi_ebp_ebx_edi:
LFB5a:
	subl	$60, %esp
LCFI21a:
	movl	%esi, 44(%esp)
LCFI22a:
	movl	%ebp, 48(%esp)
LCFI23a:
	movl	%ebx, 52(%esp)
LCFI24a:
	movl	%edi, 56(%esp)
LCFI25a:
	movl	$0, %esi
	movl	$0, %ebp
	movl	$0, %ebx
	movl	$0, %edi
	call	_uwind_to_main
	movl	44(%esp), %esi
	movl	48(%esp), %ebp
	movl	52(%esp), %ebx
	movl	56(%esp), %edi
	addl	$60, %esp
	ret
LFE5a:


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


.globl _test_ebx.eh
_test_ebx.eh:
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
	.set L$set$3,LCFI0-LFB13
	.long L$set$3
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$4,LCFI1-LCFI0
	.long L$set$4
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x2	# uleb128 0x2
	.align 2
LEFDE3:


.globl _test_no_reg.eh
_test_no_reg.eh:
LSFDE5:
	.set L$set$5,LEFDE5-LASFDE5
	.long L$set$5	# FDE Length
LASFDE5:
	.long	LASFDE5-EH_frame1	# FDE CIE offset
	.long	LFB12-.	# FDE initial location
	.set L$set$6,LFE12-LFB12
	.long L$set$6	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$7,LCFI2-LFB12
	.long L$set$7
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.align 2
LEFDE5:


.globl _test_esi.eh
_test_esi.eh:
LSFDE9:
	.set L$set$12,LEFDE9-LASFDE9
	.long L$set$12	# FDE Length
LASFDE9:
	.long	LASFDE9-EH_frame1	# FDE CIE offset
	.long	LFB10-.	# FDE initial location
	.set L$set$13,LFE10-LFB10
	.long L$set$13	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$14,LCFI5-LFB10
	.long L$set$14
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$15,LCFI6-LCFI5
	.long L$set$15
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x2	# uleb128 0x2
	.align 2
LEFDE9:


.globl _test_edi.eh
_test_edi.eh:
LSFDE10:
	.set L$set$12a,LEFDE10-LASFDE10
	.long L$set$12a	# FDE Length
LASFDE10:
	.long	LASFDE10-EH_frame1	# FDE CIE offset
	.long	LFB11-.	# FDE initial location
	.set L$set$13a,LFE11-LFB11
	.long L$set$13a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$14a,LCFI3-LFB11
	.long L$set$14a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$15a,LCFI4-LCFI3
	.long L$set$15a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x2	# uleb128 0x2
	.align 2
LEFDE10:



.globl _test_ebx_esi.eh
_test_ebx_esi.eh:
LSFDE11:
	.set L$set$16,LEFDE11-LASFDE11
	.long L$set$16	# FDE Length
LASFDE11:
	.long	LASFDE11-EH_frame1	# FDE CIE offset
	.long	LFB9-.	# FDE initial location
	.set L$set$17,LFE9-LFB9
	.long L$set$17	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$18,LCFI7-LFB9
	.long L$set$18
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$19,LCFI9-LCFI7
	.long L$set$19
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x2	# uleb128 0x2
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.align 2
LEFDE11:


.globl _test_esi_edi.eh
_test_esi_edi.eh:
LSFDE13:
	.set L$set$20,LEFDE13-LASFDE13
	.long L$set$20	# FDE Length
LASFDE13:
	.long	LASFDE13-EH_frame1	# FDE CIE offset
	.long	LFB8-.	# FDE initial location
	.set L$set$21,LFE8-LFB8
	.long L$set$21	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$22,LCFI10-LFB8
	.long L$set$22
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$23,LCFI12-LCFI10
	.long L$set$23
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x2	# uleb128 0x2
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x3	# uleb128 0x3
	.align 2
LEFDE13:


.globl _test_ebx_esi_edi.eh
_test_ebx_esi_edi.eh:
LSFDE15:
	.set L$set$24,LEFDE15-LASFDE15
	.long L$set$24	# FDE Length
LASFDE15:
	.long	LASFDE15-EH_frame1	# FDE CIE offset
	.long	LFB7-.	# FDE initial location
	.set L$set$25,LFE7-LFB7
	.long L$set$25	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$26,LCFI13-LFB7
	.long L$set$26
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$27,LCFI16-LCFI13
	.long L$set$27
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x2	# uleb128 0x2
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x3	# uleb128 0x3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE15:


.globl _test_esi_edi_ebp.eh
_test_esi_edi_ebp.eh:
LSFDE17:
	.set L$set$28,LEFDE17-LASFDE17
	.long L$set$28	# FDE Length
LASFDE17:
	.long	LASFDE17-EH_frame1	# FDE CIE offset
	.long	LFB6-.	# FDE initial location
	.set L$set$29,LFE6-LFB6
	.long L$set$29	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$30,LCFI17-LFB6
	.long L$set$30
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x20	# uleb128 0x20
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$31,LCFI20-LCFI17
	.long L$set$31
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE17:


.globl _test_ebx_esi_edi_ebp.eh
_test_ebx_esi_edi_ebp.eh:
LSFDE19:
	.set L$set$32,LEFDE19-LASFDE19
	.long L$set$32	# FDE Length
LASFDE19:
	.long	LASFDE19-EH_frame1	# FDE CIE offset
	.long	LFB5-.	# FDE initial location
	.set L$set$33,LFE5-LFB5
	.long L$set$33	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34,LCFI21-LFB5
	.long L$set$34
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x40	# uleb128 0x40
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$35,LCFI25-LCFI21
	.long L$set$35
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 2
LEFDE19:

.globl _test_esi_ebp_ebx_edi.eh
_test_esi_ebp_ebx_edi.eh:
LSFDE19a:
	.set L$set$32a,LEFDE19a-LASFDE19a
	.long L$set$32a	# FDE Length
LASFDE19a:
	.long	LASFDE19a-EH_frame1	# FDE CIE offset
	.long	LFB5a-.	# FDE initial location
	.set L$set$33a,LFE5a-LFB5a
	.long L$set$33a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34a,LCFI21a-LFB5a
	.long L$set$34a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x40	# uleb128 0x40
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$35a,LCFI25a-LCFI21a
	.long L$set$35a
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x2	# uleb128 0x2
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x4	# uleb128 0x4
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x5	# uleb128 0x5
	.align 2
LEFDE19a:



	.subsections_via_symbols
