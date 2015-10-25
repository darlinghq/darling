
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions -DUSE_COMPACT_UNWIND=1
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions -DUSE_EH_FRAME=1
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions -DUSE_COMPACT_UNWIND=1 -DUSE_EH_FRAME=1
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86.s -arch i386 -fexceptions -DUSE_EH_FRAME=1 -Wl,-no_compact_unwind


	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.long	_test_no_reg
	.long	_test_ebx
	.long	_test_esi
	.long	_test_edi
	.long	_test_ebx_esi
	.long	_test_esi_edi
	.long	_test_ebx_edi
	.long	_test_ebx_esi_edi
	.long	_test_edi_esi_ebx
	.long	_test_pad_ebx_edi
	.long	0


	.text

.globl _test_ebx
_test_ebx:
LFB13:
	pushl	%ebp
LCFI2:
	movl	%esp, %ebp
LCFI3:
	pushl	%ebx
LCFI4:
	subl	$20, %esp
LCFI5:
	movl	$0, %ebx
	call	_uwind_to_main
	addl	$20, %esp
	popl	%ebx
	leave
	ret
LFE13:

.globl _test_no_reg
_test_no_reg:
LFB12:
	pushl	%ebp
LCFI6:
	movl	%esp, %ebp
LCFI7:
	subl	$24, %esp
LCFI8:
	call	_uwind_to_main
	leave
	ret
LFE12:

.globl _test_esi
_test_esi:
LFB10:
	pushl	%ebp
LCFI13:
	movl	%esp, %ebp
LCFI14:
	pushl	%esi
LCFI15:
	subl	$20, %esp
LCFI16:
	movl	$0, %esi
	call	_uwind_to_main
	addl	$20, %esp
	popl	%esi
	leave
	ret
LFE10:

.globl _test_edi
_test_edi:
LFB11:
	pushl	%ebp
LCFI9:
	movl	%esp, %ebp
LCFI10:
	pushl	%edi
LCFI11:
	subl	$20, %esp
LCFI12:
	movl	$0, %edi
	call	_uwind_to_main
	addl	$20, %esp
	popl	%edi
	leave
	ret
LFE11:

.globl _test_ebx_esi
_test_ebx_esi:
LFB9:
	pushl	%ebp
LCFI17:
	movl	%esp, %ebp
LCFI18:
	subl	$24, %esp
LCFI19:
	movl	%ebx, -8(%ebp)
LCFI20:
	movl	%esi, -4(%ebp)
LCFI21:
	movl	$0, %ebx
	movl	$0, %esi
	call	_uwind_to_main
	movl	-8(%ebp), %ebx
	movl	-4(%ebp), %esi
	leave
	ret
LFE9:


.globl _test_esi_edi
_test_esi_edi:
LFB8:
	pushl	%ebp
LCFI22:
	movl	%esp, %ebp
LCFI23:
	subl	$24, %esp
LCFI24:
	movl	%esi, -8(%ebp)
LCFI25:
	movl	%edi, -4(%ebp)
LCFI26:
	movl	$0, %edi
	movl	$0, %esi
	call	_uwind_to_main
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %edi
	leave
	ret
LFE8:

.globl _test_ebx_edi
_test_ebx_edi:
LFB91:
	pushl	%ebp
LCFI171:
	movl	%esp, %ebp
LCFI181:
	subl	$24, %esp
LCFI191:
	movl	%ebx, -8(%ebp)
LCFI201:
	movl	%edi, -4(%ebp)
LCFI211:
	movl	$0, %ebx
	movl	$0, %edi
	call	_uwind_to_main
	movl	-8(%ebp), %ebx
	movl	-4(%ebp), %edi
	leave
	ret
LFE91:

.globl _test_ebx_esi_edi
_test_ebx_esi_edi:
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


.globl _test_edi_esi_ebx
_test_edi_esi_ebx:
LFB7a:
	pushl	%ebp
LCFI27a:
	movl	%esp, %ebp
LCFI28a:
	subl	$40, %esp
LCFI29a:
	movl	%edi, -12(%ebp)
LCFI30a:
	movl	%esi, -8(%ebp)
LCFI31a:
	movl	%ebx, -4(%ebp)
LCFI32a:
	movl	$0, %ebx
	movl	$0, %esi
	movl	$0, %edi
	call	_uwind_to_main
	movl	-12(%ebp), %edi
	movl	-8(%ebp), %esi
	movl	-4(%ebp), %ebx
	leave
	ret
LFE7a:



.globl _test_pad_ebx_edi
_test_pad_ebx_edi:
LFB91a:
	pushl	%ebp
LCFI171a:
	movl	%esp, %ebp
LCFI181a:
	subl	$88, %esp
LCFI191a:
	movl	%ebx, -72(%ebp)
LCFI201a:
	movl	%edi, -68(%ebp)
LCFI211a:
	movl	$0, %ebx
	movl	$0, %edi
	call	_uwind_to_main
	movl	-72(%ebp), %ebx
	movl	-68(%ebp), %edi
	leave
	ret
LFE91a:


#if USE_EH_FRAME
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


.globl _test_no_reg.eh
_test_no_reg.eh:
LSFDE5:
	.set L$set$6,LEFDE5-LASFDE5
	.long L$set$6	# FDE Length
LASFDE5:
	.long	LASFDE5-EH_frame1	# FDE CIE offset
	.long	LFB12-.	# FDE initial location
	.set L$set$7,LFE12-LFB12
	.long L$set$7	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$8,LCFI6-LFB12
	.long L$set$8
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$9,LCFI7-LCFI6
	.long L$set$9
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE5:


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
	.byte	0x3	# uleb128 0x3
	.align 2
LEFDE3:


.globl _test_esi.eh
_test_esi.eh:
LSFDE9:
	.set L$set$15,LEFDE9-LASFDE9
	.long L$set$15	# FDE Length
LASFDE9:
	.long	LASFDE9-EH_frame1	# FDE CIE offset
	.long	LFB10-.	# FDE initial location
	.set L$set$16,LFE10-LFB10
	.long L$set$16	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$17,LCFI13-LFB10
	.long L$set$17
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$18,LCFI14-LCFI13
	.long L$set$18
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$19,LCFI16-LCFI14
	.long L$set$19
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x3	# uleb128 0x3
	.align 2
LEFDE9:


.globl _test_edi.eh
_test_edi.eh:
LSFDE10:
	.set L$set$15a,LEFDE10-LASFDE10
	.long L$set$15a	# FDE Length
LASFDE10:
	.long	LASFDE10-EH_frame1	# FDE CIE offset
	.long	LFB11-.	# FDE initial location
	.set L$set$16a,LFE11-LFB11
	.long L$set$16a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$17a,LCFI9-LFB11
	.long L$set$17a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$18a,LCFI10-LCFI9
	.long L$set$18a
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$19a,LCFI12-LCFI10
	.long L$set$19a
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.align 2
LEFDE10:

.globl _test_ebx_esi.eh
_test_ebx_esi.eh:
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
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x3	# uleb128 0x3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE11:

.globl _test_esi_edi.eh
_test_esi_edi.eh:
LSFDE13:
	.set L$set$25,LEFDE13-LASFDE13
	.long L$set$25	# FDE Length
LASFDE13:
	.long	LASFDE13-EH_frame1	# FDE CIE offset
	.long	LFB8-.	# FDE initial location
	.set L$set$26,LFE8-LFB8
	.long L$set$26	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$27,LCFI22-LFB8
	.long L$set$27
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$28,LCFI23-LCFI22
	.long L$set$28
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$29,LCFI26-LCFI23
	.long L$set$29
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE13:

.globl _test_ebx_edi.eh
_test_ebx_edi.eh:
LSFDE111:
	.set L$set$20a,LEFDE11-LASFDE11
	.long L$set$20a	# FDE Length
LASFDE111:
	.long	LASFDE111-EH_frame1	# FDE CIE offset
	.long	LFB91-.	# FDE initial location
	.set L$set$21a,LFE91-LFB91
	.long L$set$21a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$22a,LCFI171-LFB91
	.long L$set$22a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$23a,LCFI181-LCFI171
	.long L$set$23a
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$24a,LCFI211-LCFI181
	.long L$set$24a
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x4	# uleb128 0x4
	.align 2
LEFDE111:

.globl _test_ebx_esi_edi.eh
_test_ebx_esi_edi.eh:
LSFDE15:
	.set L$set$30,LEFDE15-LASFDE15
	.long L$set$30	# FDE Length
LASFDE15:
	.long	LASFDE15-EH_frame1	# FDE CIE offset
	.long	LFB7-.	# FDE initial location
	.set L$set$31,LFE7-LFB7
	.long L$set$31	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32,LCFI27-LFB7
	.long L$set$32
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$33,LCFI28-LCFI27
	.long L$set$33
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34,LCFI32-LCFI28
	.long L$set$34
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x5	# uleb128 0x5
	.align 2
LEFDE15:


.globl _test_edi_esi_ebx.eh
_test_edi_esi_ebx.eh:
LSFDE15a:
	.set L$set$30a,LEFDE15a-LASFDE15a
	.long L$set$30a	# FDE Length
LASFDE15a:
	.long	LASFDE15a-EH_frame1	# FDE CIE offset
	.long	LFB7a-.	# FDE initial location
	.set L$set$31a,LFE7a-LFB7a
	.long L$set$31a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$32a,LCFI27a-LFB7a
	.long L$set$32a
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$33a,LCFI28a-LCFI27a
	.long L$set$33a
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$34a,LCFI32a-LCFI28a
	.long L$set$34a
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x3	# uleb128 0x3
	.byte	0x86	# DW_CFA_offset, column 0x6
	.byte	0x4	# uleb128 0x4
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x5	# uleb128 0x5
	.align 2
LEFDE15a:


.globl _test_pad_ebx_edi.eh
_test_pad_ebx_edi.eh:
LSFDE111a:
	.set L$set$20b,LEFDE111a-LASFDE111a
	.long L$set$20b	# FDE Length
LASFDE111a:
	.long	LASFDE111a-EH_frame1	# FDE CIE offset
	.long	LFB91a-.	# FDE initial location
	.set L$set$21b,LFE91a-LFB91a
	.long L$set$21b	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$22b,LCFI171a-LFB91a
	.long L$set$22b
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x8	# uleb128 0x8
	.byte	0x84	# DW_CFA_offset, column 0x4
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$23b,LCFI181a-LCFI171a
	.long L$set$23b
	.byte	0xd	# DW_CFA_def_cfa_register
	.byte	0x4	# uleb128 0x4
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$24b,LCFI211a-LCFI181a
	.long L$set$24b
	.byte	0x87	# DW_CFA_offset, column 0x7
	.byte	0x13	# uleb128 0x13
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x14	# uleb128 0x14
	.align 2
LEFDE111a:
#endif // USE_EH_FRAME



#if USE_COMPACT_UNWIND
	
	.section __LD,__compact_unwind,regular,debug

	.long	_test_no_reg
	.set L101,LFE13-_test_no_reg
	.long	L101
	.long	0x01000000
	.long	0
	.long	0

	.long	_test_ebx
	.set L102,LFE13-_test_ebx
	.long	L102
	.long	0x01010001
	.long	0
	.long	0

	.long	_test_esi
	.set L103,LFE10-_test_esi
	.long	L103
	.long	0x01010005
	.long	0
	.long	0

	.long	_test_edi
	.set L104,LFE11-_test_edi
	.long	L104
	.long	0x01010004
	.long	0
	.long	0

	.long	_test_ebx_esi
	.set L105,LFE9-_test_ebx_esi
	.long	L105
	.long	0x01020029
	.long	0
	.long	0

	.long	_test_esi_edi
	.set L106,LFE8-_test_esi_edi
	.long	L106
	.long	0x01020025
	.long	0
	.long	0

	.long	_test_ebx_edi
	.set L107,LFE91-_test_ebx_edi
	.long	L107
	.long	0x01020021
	.long	0
	.long	0

	.long	_test_ebx_esi_edi
	.set L108,LFE7-_test_ebx_esi_edi
	.long	L108
	.long	0x01030129
	.long	0
	.long	0

	.long	_test_edi_esi_ebx
	.set L109,LFE7a-_test_edi_esi_ebx
	.long	L109
	.long	0x0103006C
	.long	0
	.long	0

	.long	_test_pad_ebx_edi
	.set L110,LFE91a-_test_pad_ebx_edi
	.long	L110
	.long	0x01120021
	.long	0
	.long	0

#endif // USE_COMPACT_UNWIND


	.subsections_via_symbols
