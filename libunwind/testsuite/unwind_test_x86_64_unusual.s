
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86_64.s -arch x86_64 
# TEST-OPTIONS:  unwind_test_main.c unwind_test_x86_64.s -arch x86_64 -Wl,-no_compact_unwind


	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.quad	_test_no_reg
	.quad	Ltest_rbx
	.quad	_test_rbx_pad_r12
	.quad	_test_rbx_same_value
	.quad	_test_rbx_same_register
	.quad	0


	.text


.globl _test_big_frameless_stack_odd_dwarf
_test_big_frameless_stack_odd_dwarf:
L100:
	subq	$160016, %rsp
L102:
	movq	$0, %rbx
	call	_uwind_to_main
	addq	$160016, %rsp
	ret
L103:


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


Ltest_rbx:
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


.globl _test_rbx_pad_r12
_test_rbx_pad_r12:
LFB8:
	pushq	%rbp
LCFI16:
	movq	%rsp, %rbp
LCFI17:
	movq	%rbx, -56(%rbp)
LCFI18:
	movq	%r12, -8(%rbp)
LCFI19:
	subq	$64, %rsp
LCFI20:
	movq	$0, %rbx
	movq	$0, %r12
	call	_uwind_to_main
	movq	(%rsp), %rbx
	movq	8(%rsp), %r12
	leave
	ret
LFE8:


.globl _test_rbx_same_value
_test_rbx_same_value:
LFB10b:
	jmp		L33
	pushq	%rbx
LCFI3b:
	movq	$0, %rbx
	call	_uwind_to_main
	popq	%rbx
	ret
L33:
	pushq	$0
	call	_uwind_to_main
	popq	%rax
	ret
LFE10b:


.globl _test_rbx_same_register
_test_rbx_same_register:
LFB10c:
	jmp		L34
	pushq	%rbx
LCFI3c:
	movq	$0, %rbx
	call	_uwind_to_main
	popq	%rbx
	ret
L34:
	pushq	$0
	call	_uwind_to_main
	popq	%rax
	ret
LFE10c:




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
	.byte	0x90
	.byte	0x1
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

Ltest_rbx.eh:
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

.globl _test_rbx_pad_r12.eh
_test_rbx_pad_r12.eh:
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
	.byte	0x9
	.align 3
LEFDE13:



.globl _test_big_frameless_stack_odd_dwarf.eh
_test_big_frameless_stack_odd_dwarf.eh:
	.set L$set$12a,LEFDE9a-LASFDE9a
	.long L$set$12a	# FDE Length
LASFDE9a:
	.long	LASFDE9a-EH_frame1	# FDE CIE offset
	.quad	L100-.	# FDE initial location
	.set L$set$13a,L103-L100
	.quad L$set$13a	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.long	7
	.byte	0xc	# DW_CFA_def_cfa
	.byte	0x07	# uleb128 7
	.byte	0x90,0xe2,0x9	# uleb128 0x27110
	.align 3
LEFDE9a:


.globl _test_rbx_same_value.eh
_test_rbx_same_value.eh:
LSFDE9b:
	.set L$set$12,LEFDE9b-LASFDE9b
	.long L$set$12	# FDE Length
LASFDE9b:
	.long	LASFDE9b-EH_frame1	# FDE CIE offset
	.quad	LFB10b-.	# FDE initial location
	.set L$set$13,LFE10b-LFB10b
	.quad L$set$13	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$14,LCFI3b-LFB10b
	.long L$set$14
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x10	# uleb128 0x10
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$15,L33-LCFI3b
	.long L$set$15
	.byte	0x08	# DW_CFA_same_value
	.byte	0x3	# uleb128 0x3
	.align 3
LEFDE9b:


.globl _test_rbx_same_register.eh
_test_rbx_same_register.eh:
LSFDE9c:
	.set L$set$12c,LEFDE9c-LASFDE9c
	.long L$set$12c	# FDE Length
LASFDE9c:
	.long	LASFDE9c-EH_frame1	# FDE CIE offset
	.quad	LFB10c-.	# FDE initial location
	.set L$set$16c,LFE10c-LFB10c
	.quad L$set$16c	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$17c,LCFI3c-LFB10c
	.long L$set$17c
	.byte	0xe	# DW_CFA_def_cfa_offset
	.byte	0x10	# uleb128 0x10
	.byte	0x83	# DW_CFA_offset, column 0x3
	.byte	0x2	# uleb128 0x2
	.byte	0x4	# DW_CFA_advance_loc4
	.set L$set$18c,L34-LCFI3c
	.long L$set$18c
	.byte	0x09	# DW_CFA_register
	.byte	0x3	# uleb128 0x3
	.byte	0x3	# uleb128 0x3
	.align 3
LEFDE9c:


