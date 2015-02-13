
/*
 *  by Ian Ollmann
 *
 *	Copyright © 2005 Apple Computer Inc. All Rights Reserved.
 *
 */


#include <machine/asm.h>
#include "abi.h"


#if defined( __LP64__ )
	#define DEST_P			%rdi
	#define LOAD_DEST_P
#else
	#define DEST_P			%ecx
	#define LOAD_DEST_P		mov			THIRD_ARG_OFFSET(STACKP), DEST_P
#endif


PRIVATE_ENTRY(__remquol)			//private interface used by single and double precision remquo
#if ! defined( BUILDING_FOR_CARBONCORE_LEGACY )
ENTRY(remquol)
#endif
	//load data
	fldt	SECOND_ARG_OFFSET(STACKP)		//	{ d }
	fldt	FIRST_ARG_OFFSET(STACKP)		//	{ n, d }

1:	fprem1						//	{ r, d }
	fstsw	%ax
	btw		$10,%ax
	jc		1b
	fstp	%st(1)

	//Calculate quo. 
	//Alas, the bits in fstat are all scrambled up.
	//It seems like there should be an easy way to do this,
	//but I don't see the magic instruction. So....
	movl	%eax,	%ecx
	movl	%eax,	%edx
	ror		$6,		%eax
	ror		$9,		%ecx
	ror		$13,	%edx
	and		$0x4,	%eax
	and		$0x1,	%ecx
	and		$0x2,	%edx
	or		%ecx,	%eax
	or		%eax,	%edx

	//set the sign appropriately according to the sign of n/d
	movw	(FIRST_ARG_OFFSET+8)(STACKP),	%cx
	movw	(SECOND_ARG_OFFSET+8)(STACKP),	%ax
	xor		%ecx,		%eax
	LOAD_DEST_P					//Get the destination pointer
	cwde	
	sar		$15,		%eax
	xor		%eax,		%edx
	sub		%eax,		%edx

	//store out quo and return
	movl	%edx,	(DEST_P)
	ret
	
