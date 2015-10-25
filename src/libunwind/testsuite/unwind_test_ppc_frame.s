

# TEST-OPTIONS:  unwind_test_main.c unwind_test_ppc.s -arch ppc -fexceptions  -faltivec

	.data
.globl _unwind_tester_list
_unwind_tester_list:	
	.long	_test_v24_v31
	.long	_test_r29_r31
	.long	_test_f26_f31
	.long	_test_ccr2_ccr4_ctr
	.long	0


	.text
	.align 2
	.globl _test_r29_r31
_test_r29_r31:
LFB11:
	mflr	r0
	stmw	r29,-12(r1)
	stw		r0,8(r1)
	stwu	r1,-80(r1)
LCFI10:
	# trash non-volatile registers
	li		r31,0
	li		r30,0
	li		r29,0
	# force unwind that should restore non-volatile register
	bl		_uwind_to_main	
	addi	r1,r1,80
	lwz		r0,8(r1)
	lmw		r29,-12(r1)
	mtlr	r0
	blr
LFE11:


	.text
	.align 2
	.globl _test_ccr2_ccr4_ctr
_test_ccr2_ccr4_ctr:
LFB11a:
	mflr	r0
	stw		r0,8(r1)
	mfcr	r0
	stw		r0,-12(r1)
	stw		r0,-8(r1)
	mfctr	r0
	stw		r0,-4(r1)
	stwu	r1,-80(r1)
LCFI10a:
	# trash non-volatile registers
	li		r3,0
	mtocrf	40,r3
	mtctr	r3
	# force unwind that should restore non-volatile register
	bl		_uwind_to_main	
	addi	r1,r1,80
	lwz		r0,-4(r1)
	mtctr	r0
	lwz		r0,-8(r1)
	mtocrf	4,r0
	lwz		r0,-12(r1)
	mtocrf	128,r0
	lwz		r0,8(r1)
	mtlr	r0
	blr
LFE11a:


	.align 2
	.globl _test_f26_f31
_test_f26_f31:
LFB4:
	mflr r0
LCFI43:
	bl saveFP+48 ; save f26-f31
LCFI44:
	stwu r1,-144(r1)
LCFI45:
	# trash non-volatile registers
	fsub	f31,f31,f31
	fsub	f30,f30,f30
	fsub	f29,f29,f29
	fsub	f28,f28,f28
	fsub	f27,f27,f27
	fsub	f26,f26,f26
	# force unwind that should restore non-volatile register
	bl		_uwind_to_main	
	lwz		r0,8(r1)
	mtlr	r0
	b		restFP+48 ; restore f26-f31
LFE4:
	.align 2


	.align 2
	.globl _test_v24_v31
_test_v24_v31:
LFB3:
	mflr r0
	stw r0,8(r1)
	stwu r1,-352(r1)
LCFI2:
	li r0,208
	stvx v24,r1,r0
	li r0,224
	stvx v25,r1,r0
	li r0,240
	stvx v26,r1,r0
	li r0,256
	stvx v27,r1,r0
	li r0,272
	stvx v28,r1,r0
	li r0,288
	stvx v29,r1,r0
	li r0,304
	stvx v30,r1,r0
	li r0,320
	stvx v31,r1,r0
LCFI11:
	mfspr r0,256
	stw r0,348(r1)
	oris r0,r0,0x3ff0
	ori r0,r0,255	
	mtspr 256,r0
	
	# trash non-volatile registers
	vor		v24,v20,v20
	vor		v25,v20,v20
	vor		v26,v20,v20
	vor		v27,v20,v20
	vor		v28,v20,v20
	vor		v29,v20,v20
	vor		v30,v20,v20
	vor		v31,v20,v20
	
	# force unwind that should restore non-volatile register
	bl		_uwind_to_main	
	
	li r0,208
	lwz r12,348(r1)
	lvx v24,r1,r0
	li r0,224
	lvx v25,r1,r0
	li r0,240
	lvx v26,r1,r0
	li r0,256
	lvx v27,r1,r0
	li r0,272
	lvx v28,r1,r0
	li r0,288
	lvx v29,r1,r0
	li r0,304
	lvx v30,r1,r0
	li r0,320
	lvx v31,r1,r0
	mtspr 256,r12
	addi r1,r1,352
	lwz r0,8(r1)
	mtlr r0
	blr
LFE3:



	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$0,LECIE1-LSCIE1
	.long L$set$0	; Length of Common Information Entry
LSCIE1:
	.long	0x0	; CIE Identifier Tag
	.byte	0x1	; CIE Version
	.ascii "zR\0"	; CIE Augmentation
	.byte	0x1	; uleb128 0x1; CIE Code Alignment Factor
	.byte	0x7c	; sleb128 -4; CIE Data Alignment Factor
	.byte	0x41	; CIE RA Column
	.byte	0x1	; uleb128 0x1; Augmentation size
	.byte	0x10	; FDE Encoding (pcrel)
	.byte	0xc	; DW_CFA_def_cfa
	.byte	0x1	; uleb128 0x1
	.byte	0x0	; uleb128 0x0
	.align 2
LECIE1:


	.globl _test_r29_r31.eh
_test_r29_r31.eh:
LSFDE9:
	.set L$set$7,LEFDE9-LASFDE9
	.long L$set$7	; FDE Length
LASFDE9:
	.long	LASFDE9-EH_frame1	; FDE CIE offset
	.long	LFB11-.	; FDE initial location
	.set L$set$8,LFE11-LFB11
	.long L$set$8	; FDE address range
	.byte	0x0	; uleb128 0x0; Augmentation size
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$9,LCFI10-LFB11
	.long L$set$9
	.byte	0xe	; DW_CFA_def_cfa_offset
	.byte	0x50	; uleb128 0x50
	.byte	0x9f	; DW_CFA_offset, column 0x1f
	.byte	0x1	; uleb128 0x1
	.byte	0x9e	; DW_CFA_offset, column 0x1e
	.byte	0x2	; uleb128 0x2
	.byte	0x9d	; DW_CFA_offset, column 0x1d
	.byte	0x3	; uleb128 0x3
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x41	; uleb128 0x41
	.byte	0x7e	; sleb128 -2
	.align 2
LEFDE9:

	.globl _test_ccr2_ccr4_ctr.eh
_test_ccr2_ccr4_ctr.eh:
LSFDE9a:
	.set L$set$7,LEFDE9a-LASFDE9a
	.long L$set$7	; FDE Length
LASFDE9a:
	.long	LASFDE9a-EH_frame1	; FDE CIE offset
	.long	LFB11a-.	; FDE initial location
	.set L$set$8,LFE11a-LFB11a
	.long L$set$8	; FDE address range
	.byte	0x0	; uleb128 0x0; Augmentation size
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$9,LCFI10a-LFB11a
	.long L$set$9
	.byte	0xe	; DW_CFA_def_cfa_offset
	.byte	0x50	; uleb128 0x50
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x42	; uleb128 0x42
	.byte	0x01	; sleb128 1
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x48	; uleb128 0x48
	.byte	0x02	; sleb128 2
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x46	; uleb128 0x46
	.byte	0x03	; sleb128 3
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x41	; uleb128 0x41
	.byte	0x7e	; sleb128 -2
	.align 2
LEFDE9a:



	.globl _test_f26_f31
_test_f26_f31.eh:
LSFDE29:
	.set L$set$34,LEFDE29-LASFDE29
	.long L$set$34	; FDE Length
LASFDE29:
	.long	LASFDE29-EH_frame1	; FDE CIE offset
	.long	LFB4-.	; FDE initial location
	.set L$set$35,LFE4-LFB4
	.long L$set$35	; FDE address range
	.byte	0x0	; uleb128 0x0; Augmentation size
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$36,LCFI43-LFB4
	.long L$set$36
	.byte	0x9	; DW_CFA_register
	.byte	0x41	; uleb128 0x41
	.byte	0x0	; uleb128 0x0
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$37,LCFI45-LCFI43
	.long L$set$37
	.byte	0xe	; DW_CFA_def_cfa_offset
	.byte	0x90,0x1	; uleb128 0x90
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x41	; uleb128 0x41
	.byte	0x7e	; sleb128 -2
	.byte	0xbf	; DW_CFA_offset, column 0x3f
	.byte	0x2	; uleb128 0x2
	.byte	0xbe	; DW_CFA_offset, column 0x3e
	.byte	0x4	; uleb128 0x4
	.byte	0xbd	; DW_CFA_offset, column 0x3d
	.byte	0x6	; uleb128 0x6
	.byte	0xbc	; DW_CFA_offset, column 0x3c
	.byte	0x8	; uleb128 0x8
	.byte	0xbb	; DW_CFA_offset, column 0x3b
	.byte	0xa	; uleb128 0xa
	.byte	0xba	; DW_CFA_offset, column 0x3a
	.byte	0xc	; uleb128 0xc
	.align 2
LEFDE29:


	.globl _test_v24_v31.eh
_test_v24_v31.eh:
LSFDE3:
	.set L$set$1,LEFDE3-LASFDE3
	.long L$set$1	; FDE Length
LASFDE3:
	.long	LASFDE3-EH_frame1	; FDE CIE offset
	.long	LFB3-.	; FDE initial location
	.set L$set$2,LFE3-LFB3
	.long L$set$2	; FDE address range
	.byte	0x0	; uleb128 0x0; Augmentation size
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$3,LCFI2-LFB3
	.long L$set$3
	.byte	0xe	; DW_CFA_def_cfa_offset
	.byte	0xe0,0x2	; uleb128 0x160
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$4,LCFI11-LCFI2
	.long L$set$4
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x6c	; uleb128 0x6c
	.byte	0x8	; uleb128 0x8
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x6b	; uleb128 0x6b
	.byte	0xc	; uleb128 0xc
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x6a	; uleb128 0x6a
	.byte	0x10	; uleb128 0x10
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x69	; uleb128 0x69
	.byte	0x14	; uleb128 0x14
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x68	; uleb128 0x68
	.byte	0x18	; uleb128 0x18
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x67	; uleb128 0x67
	.byte	0x1c	; uleb128 0x1c
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x66	; uleb128 0x66
	.byte	0x20	; uleb128 0x20
	.byte	0x5	; DW_CFA_offset_extended
	.byte	0x65	; uleb128 0x65
	.byte	0x24	; uleb128 0x24
	.byte	0x11	; DW_CFA_offset_extended_sf
	.byte	0x41	; uleb128 0x41
	.byte	0x7e	; sleb128 -2
	.align 2
LEFDE3:


