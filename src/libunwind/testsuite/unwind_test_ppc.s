
#
# This is a generic function to test that restoring registers during unwinding work correctly.
# 

	.literal4
	.align 2
LC14:	.long	1096810496
LC15:	.long	1097859072
LC16:	.long	1098907648
LC17:	.long	1099431936
LC18:	.long	1099956224
LC19:	.long	1100480512
LC20:	.long	1101004800
LC21:	.long	1101529088
LC22:	.long	1102053376
LC23:	.long	1102577664
LC24:	.long	1103101952
LC25:	.long	1103626240
LC26:	.long	1104150528
LC27:	.long	1104674816
LC28:	.long	1105199104
LC29:	.long	1105723392
LC30:	.long	1106247680
LC31:	.long	1106771968

	.literal16
LV23:	.long	0x23232323, 0x23232322, 0x23232321, 0x23232320
LV24:	.long	0x24242424, 0x24242422, 0x24242421, 0x24242420
LV25:	.long	0x25252525, 0x25252522, 0x25252521, 0x25252520
LV26:	.long	0x26262626, 0x26262622, 0x26262621, 0x26262620
LV27:	.long	0x27272727, 0x27272722, 0x27272721, 0x27272720
LV28:	.long	0x28282828, 0x28282822, 0x28282821, 0x28282820
LV29:	.long	0x29292929, 0x29292922, 0x29292921, 0x29292920
LV30:	.long	0x30303030, 0x30303022, 0x30303021, 0x30303020
LV31:	.long	0x31313131, 0x31313122, 0x31313121, 0x31313120

	.text
.globl _unwind_tester
_unwind_tester:
LFB2:
	mflr r0
LCFI43:
	bl saveFP ; save f14-f31
LCFI44:
	stmw r13,-220(r1)
LCFI45:
	stwu r1,-336(r1)
LCFI46:

	# load magic values into non-volatile registers
	lis		r31,0x3333
	ori		r31,r31,0x3131
	lis		r30,0x3333
	ori		r30,r30,0x3030
	lis		r29,0x2222
	ori		r29,r29,0x2929
	lis		r28,0x2222
	ori		r28,r28,0x2828
	lis		r27,0x2222
	ori		r27,r27,0x2727
	lis		r26,0x2222
	ori		r26,r26,0x2626
	lis		r25,0x2222
	ori		r25,r25,0x2525
	lis		r24,0x2222
	ori		r24,r24,0x2424
	lis		r23,0x2222
	ori		r23,r23,0x2323
	lis		r22,0x2222
	ori		r22,r22,0x2222
	lis		r21,0x2222
	ori		r21,r21,0x2121
	lis		r20,0x2222
	ori		r20,r20,0x2020
	lis		r19,0x1111
	ori		r19,r19,0x1919
	lis		r18,0x1111
	ori		r18,r18,0x1818
	lis		r17,0x1111
	ori		r17,r17,0x1717
	lis		r16,0x1111
	ori		r16,r16,0x1616
	lis		r15,0x1111
	ori		r15,r15,0x1515
	lis		r14,0x1111
	ori		r14,r14,0x1414
	lis		r13,0x1111
	ori		r13,r13,0x1313
	
	lis		r2,ha16(LC14)
	lfs		f14,lo16(LC14)(r2)
	lis		r2,ha16(LC15)
	lfs		f15,lo16(LC15)(r2)
	lis		r2,ha16(LC16)
	lfs		f16,lo16(LC16)(r2)
	lis		r2,ha16(LC17)
	lfs		f17,lo16(LC17)(r2)
	lis		r2,ha16(LC18)
	lfs		f18,lo16(LC18)(r2)
	lis		r2,ha16(LC19)
	lfs		f19,lo16(LC19)(r2)
	lis		r2,ha16(LC20)
	lfs		f20,lo16(LC20)(r2)
	lis		r2,ha16(LC21)
	lfs		f21,lo16(LC21)(r2)
	lis		r2,ha16(LC22)
	lfs		f22,lo16(LC22)(r2)
	lis		r2,ha16(LC23)
	lfs		f23,lo16(LC23)(r2)
	lis		r2,ha16(LC24)
	lfs		f24,lo16(LC24)(r2)
	lis		r2,ha16(LC25)
	lfs		f25,lo16(LC25)(r2)
	lis		r2,ha16(LC26)
	lfs		f26,lo16(LC26)(r2)
	lis		r2,ha16(LC27)
	lfs		f27,lo16(LC27)(r2)
	lis		r2,ha16(LC28)
	lfs		f28,lo16(LC28)(r2)
	lis		r2,ha16(LC29)
	lfs		f29,lo16(LC29)(r2)
	lis		r2,ha16(LC30)
	lfs		f30,lo16(LC30)(r2)
	lis		r2,ha16(LC31)
	lfs		f31,lo16(LC31)(r2)
	
	lis		r2,ha16(LV23)
	la		r2,lo16(LV23)(r2)
	lvx		v23,0,r2
	lis		r2,ha16(LV24)
	la		r2,lo16(LV24)(r2)
	lvx		v24,0,r2
	lis		r2,ha16(LV25)
	la		r2,lo16(LV25)(r2)
	lvx		v25,0,r2
	lis		r2,ha16(LV26)
	la		r2,lo16(LV26)(r2)
	lvx		v26,0,r2
	lis		r2,ha16(LV27)
	la		r2,lo16(LV27)(r2)
	lvx		v27,0,r2
	lis		r2,ha16(LV28)
	la		r2,lo16(LV28)(r2)
	lvx		v28,0,r2
	lis		r2,ha16(LV29)
	la		r2,lo16(LV29)(r2)
	lvx		v29,0,r2
	lis		r2,ha16(LV30)
	la		r2,lo16(LV30)(r2)
	lvx		v30,0,r2
	lis		r2,ha16(LV31)
	la		r2,lo16(LV31)(r2)
	lvx		v31,0,r2
	
	lis		r2,0x1234
	ori		r2,r2,0x5678
	mtocrf	255,r2
	
	# call test function which will invoke unwinder which "returns" here
	mtctr	r3
	bctrl

	# verify that non-volatile registers still contain magic values
	lis		r3,0x3333
	ori		r3,r3,0x3131
	cmpw	r3,r31
	bne		L2
	lis		r3,0x3333
	ori		r3,r3,0x3030
	cmpw	r3,r30
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2929
	cmpw	r3,r29
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2828
	cmpw	r3,r28
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2727
	cmpw	r3,r27
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2626
	cmpw	r3,r26
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2525
	cmpw	r3,r25
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2424
	cmpw	r3,r24
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2323
	cmpw	r3,r23
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2222
	cmpw	r3,r22
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2121
	cmpw	r3,r21
	bne		L2
	lis		r3,0x2222
	ori		r3,r3,0x2020
	cmpw	r3,r20
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1919
	cmpw	r3,r19
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1818
	cmpw	r3,r18
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1717
	cmpw	r3,r17
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1616
	cmpw	r3,r16
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1515
	cmpw	r3,r15
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1414
	cmpw	r3,r14
	bne		L2
	lis		r3,0x1111
	ori		r3,r3,0x1313
	cmpw	r3,r13
	bne		L2
	
	lis		r2,ha16(LC14)
	lfs		f1,lo16(LC14)(r2)
	fcmpu	cr7,f1,f14
	bne		cr7,L2
	lis		r2,ha16(LC15)
	lfs		f1,lo16(LC15)(r2)
	fcmpu	cr7,f1,f15
	bne		cr7,L2
	lis		r2,ha16(LC16)
	lfs		f1,lo16(LC16)(r2)
	fcmpu	cr7,f1,f16
	bne		cr7,L2
	lis		r2,ha16(LC17)
	lfs		f1,lo16(LC17)(r2)
	fcmpu	cr7,f1,f17
	bne		cr7,L2
	lis		r2,ha16(LC18)
	lfs		f1,lo16(LC18)(r2)
	fcmpu	cr7,f1,f18
	bne		cr7,L2
	lis		r2,ha16(LC19)
	lfs		f1,lo16(LC19)(r2)
	fcmpu	cr7,f1,f19
	bne		cr7,L2
	lis		r2,ha16(LC20)
	lfs		f1,lo16(LC20)(r2)
	fcmpu	cr7,f1,f20
	bne		cr7,L2
	lis		r2,ha16(LC21)
	lfs		f1,lo16(LC21)(r2)
	fcmpu	cr7,f1,f21
	bne		cr7,L2
	lis		r2,ha16(LC22)
	lfs		f1,lo16(LC22)(r2)
	fcmpu	cr7,f1,f22
	bne		cr7,L2
	lis		r2,ha16(LC23)
	lfs		f1,lo16(LC23)(r2)
	fcmpu	cr7,f1,f23
	bne		cr7,L2
	lis		r2,ha16(LC24)
	lfs		f1,lo16(LC24)(r2)
	fcmpu	cr7,f1,f24
	bne		cr7,L2
	lis		r2,ha16(LC25)
	lfs		f1,lo16(LC25)(r2)
	fcmpu	cr7,f1,f25
	bne		cr7,L2
	lis		r2,ha16(LC26)
	lfs		f1,lo16(LC26)(r2)
	fcmpu	cr7,f1,f26
	bne		cr7,L2
	lis		r2,ha16(LC27)
	lfs		f1,lo16(LC27)(r2)
	fcmpu	cr7,f1,f27
	bne		cr7,L2
	lis		r2,ha16(LC28)
	lfs		f1,lo16(LC28)(r2)
	fcmpu	cr7,f1,f28
	bne		cr7,L2
	lis		r2,ha16(LC29)
	lfs		f1,lo16(LC29)(r2)
	fcmpu	cr7,f1,f29
	bne		cr7,L2
	lis		r2,ha16(LC30)
	lfs		f1,lo16(LC30)(r2)
	fcmpu	cr7,f1,f30
	bne		cr7,L2
	lis		r2,ha16(LC31)
	lfs		f1,lo16(LC31)(r2)
	fcmpu	cr7,f1,f31
	bne		cr7,L2
	
	lis		r2,ha16(LV23)
	la		r2,lo16(LV23)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v23
	beq		cr6,L2
	lis		r2,ha16(LV24)
	la		r2,lo16(LV24)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v24
	beq		cr6,L2
	lis		r2,ha16(LV25)
	la		r2,lo16(LV25)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v25
	beq		cr6,L2
	lis		r2,ha16(LV26)
	la		r2,lo16(LV26)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v26
	beq		cr6,L2
	lis		r2,ha16(LV27)
	la		r2,lo16(LV27)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v27
	beq		cr6,L2
	lis		r2,ha16(LV28)
	la		r2,lo16(LV28)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v28
	beq		cr6,L2
	lis		r2,ha16(LV29)
	la		r2,lo16(LV29)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v29
	beq		cr6,L2
	lis		r2,ha16(LV30)
	la		r2,lo16(LV30)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v30
	beq		cr6,L2
	lis		r2,ha16(LV31)
	la		r2,lo16(LV31)(r2)
	lvx		v1,0,r2
	vcmpequw. v0,v1,v31
	beq		cr6,L2

	mfcr	r3
	lis		r2,0x00FF
	ori		r2,r2,0xF000
	and		r3,r3,r2
	lis		r2,0x0034
	ori		r2,r2,0x5000
	cmpw	r3,r2
	bne		L2

	li		r3,0
	b		L3
L2: li		r3,1
L3: 		

	addi r1,r1,336
	lwz r0,8(r1)
	lmw r13,-220(r1)
	mtlr r0
	b restFP ; restore f14-f31
LFE2:


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

	.globl _unwind_tester.eh
_unwind_tester.eh:
LSFDE27:
	.set L$set$34,LEFDE27-LASFDE27
	.long L$set$34	; FDE Length
LASFDE27:
	.long	LASFDE27-EH_frame1	; FDE CIE offset
	.long	LFB2-.	; FDE initial location
	.set L$set$35,LFE2-LFB2
	.long L$set$35	; FDE address range
	.byte	0x0	; uleb128 0x0; Augmentation size
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$36,LCFI43-LFB2
	.long L$set$36
	.byte	0x9	; DW_CFA_register
	.byte	0x41	; uleb128 0x41
	.byte	0x0	; uleb128 0x0
	.byte	0x4	; DW_CFA_advance_loc4
	.set L$set$37,LCFI46-LCFI43
	.long L$set$37
	.byte	0xe	; DW_CFA_def_cfa_offset
	.byte	0xd0,0x2	; uleb128 0x150
	.byte	0x9f	; DW_CFA_offset, column 0x1f
	.byte	0x25	; uleb128 0x25
	.byte	0x9e	; DW_CFA_offset, column 0x1e
	.byte	0x26	; uleb128 0x26
	.byte	0x9d	; DW_CFA_offset, column 0x1d
	.byte	0x27	; uleb128 0x27
	.byte	0x9c	; DW_CFA_offset, column 0x1c
	.byte	0x28	; uleb128 0x28
	.byte	0x9b	; DW_CFA_offset, column 0x1b
	.byte	0x29	; uleb128 0x29
	.byte	0x9a	; DW_CFA_offset, column 0x1a
	.byte	0x2a	; uleb128 0x2a
	.byte	0x99	; DW_CFA_offset, column 0x19
	.byte	0x2b	; uleb128 0x2b
	.byte	0x98	; DW_CFA_offset, column 0x18
	.byte	0x2c	; uleb128 0x2c
	.byte	0x97	; DW_CFA_offset, column 0x17
	.byte	0x2d	; uleb128 0x2d
	.byte	0x96	; DW_CFA_offset, column 0x16
	.byte	0x2e	; uleb128 0x2e
	.byte	0x95	; DW_CFA_offset, column 0x15
	.byte	0x2f	; uleb128 0x2f
	.byte	0x94	; DW_CFA_offset, column 0x14
	.byte	0x30	; uleb128 0x30
	.byte	0x93	; DW_CFA_offset, column 0x13
	.byte	0x31	; uleb128 0x31
	.byte	0x92	; DW_CFA_offset, column 0x12
	.byte	0x32	; uleb128 0x32
	.byte	0x91	; DW_CFA_offset, column 0x11
	.byte	0x33	; uleb128 0x33
	.byte	0x90	; DW_CFA_offset, column 0x10
	.byte	0x34	; uleb128 0x34
	.byte	0x8f	; DW_CFA_offset, column 0xf
	.byte	0x35	; uleb128 0x35
	.byte	0x8e	; DW_CFA_offset, column 0xe
	.byte	0x36	; uleb128 0x36
	.byte	0x8d	; DW_CFA_offset, column 0xd
	.byte	0x37	; uleb128 0x37
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
	.byte	0xb9	; DW_CFA_offset, column 0x39
	.byte	0xe	; uleb128 0xe
	.byte	0xb8	; DW_CFA_offset, column 0x38
	.byte	0x10	; uleb128 0x10
	.byte	0xb7	; DW_CFA_offset, column 0x37
	.byte	0x12	; uleb128 0x12
	.byte	0xb6	; DW_CFA_offset, column 0x36
	.byte	0x14	; uleb128 0x14
	.byte	0xb5	; DW_CFA_offset, column 0x35
	.byte	0x16	; uleb128 0x16
	.byte	0xb4	; DW_CFA_offset, column 0x34
	.byte	0x18	; uleb128 0x18
	.byte	0xb3	; DW_CFA_offset, column 0x33
	.byte	0x1a	; uleb128 0x1a
	.byte	0xb2	; DW_CFA_offset, column 0x32
	.byte	0x1c	; uleb128 0x1c
	.byte	0xb1	; DW_CFA_offset, column 0x31
	.byte	0x1e	; uleb128 0x1e
	.byte	0xb0	; DW_CFA_offset, column 0x30
	.byte	0x20	; uleb128 0x20
	.byte	0xaf	; DW_CFA_offset, column 0x2f
	.byte	0x22	; uleb128 0x22
	.byte	0xae	; DW_CFA_offset, column 0x2e
	.byte	0x24	; uleb128 0x24
	.align 2
LEFDE27:
