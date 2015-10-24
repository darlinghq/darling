/*
 * Written by Stephen Canon
 *
 * Copyright © 2007, Apple Inc.  All Rights Reserved.
 */

#include <machine/asm.h>

#if defined( __LP64__ )
#define PTR_SIZE	8
#define MOVP		movq
#define ADDP		addq
#define SUBP		subq
#define RESULT_DEST %rdi
#define STACKP	%rsp
#else
#define PTR_SIZE  4
#define MOVP		movl
#define ADDP		addl
#define SUBP		subl
#define RESULT_DEST %edx
#define STACKP	%esp
#endif


// void cosisinl(long double x, long double complex *z)

ENTRY(cosisinl)
  fldt	PTR_SIZE(STACKP)  // load argument

#if defined( __LP64__ )           // in lp64 case, result address is already in %rdi
#else
  MOVP	16+PTR_SIZE(STACKP), RESULT_DEST  // load result address
#endif

  fsincos                         // try sincos
  fnstsw	%ax                     // load status word
  andw	$0x400,%ax                // check status word to see if x was in range for fsincos
  jnz		1f

  fstpt	(RESULT_DEST)             // if the argument was in range, store the result
  fstpt	16(RESULT_DEST)
  ret                             // and return

  1:	fldpi                       // if the argument was out of range, put 2π on the fp stack
  fadd	%st(0)
  fxch	%st(1)

  2:	fprem1                      // while x is out of range, reduce by 2π
  fnstsw	%ax
  andw	$0x400,%ax
  jnz		2b
  fstp	%st(1)                    // clear 2π off the fp stack

  fsincos                         // compute fsincos (argument is surely in range here)

  fstpt	(RESULT_DEST)             // store the result
  fstpt	16(RESULT_DEST)
  ret                             // and return


// void cosisin(double x, double complex *z)

ENTRY(cosisin)
#if defined( __LP64__ )          // in LP64, load x from %xmm0 (result dest will stay in %rdi)
	SUBP	$8, STACKP
	movsd %xmm0, (STACKP)
	fldl	(STACKP)
#else
	fldl	PTR_SIZE(STACKP)  // otherwise, load x and the result dest off the stack
	MOVP	8+PTR_SIZE(STACKP), RESULT_DEST
#endif

	fsincos                         // try sincos
	fnstsw	%ax                     // load status word
	andw	$0x400,%ax                // check status word to see if x was in range for fsincos
	jnz		3f

	fstpl	(RESULT_DEST)             // store the result
	fstpl	8(RESULT_DEST)
#if defined( __LP64__ )
	ADDP	$8, STACKP                // restore the stack, since we touched it in 64-bit mode
#endif
	ret

	3:	fldpi                       // if the argument was out of range, put 2π on the fp stack
	fadd	%st(0)
	fxch	%st(1)

	4:	fprem1                      // while x is out of range, reduce by 2π
	fnstsw	%ax
	andw	$0x400,%ax
	jnz	4b
	fstp	%st(1)                    // clear 2π off the fp stack

	fsincos                         // compute fsincos (argument is surely in range here)

	fstpl	(RESULT_DEST)             // store the result
	fstpl	8(RESULT_DEST)
#if defined( __LP64__ )
	ADDP	$8, STACKP                // restore the stack, since we touched it in 64-bit mode
#endif
	ret


// void cosisinf(float x, float complex *z)

ENTRY(cosisinf)
#if defined( __LP64__ )           // in LP64, load x from %xmm0 (result dest will stay in %rdi)
	SUBP	$4, STACKP
	movss	%xmm0, (STACKP)
	flds	(STACKP)
#else
	flds	PTR_SIZE(STACKP)  // otherwise, load x and the result dest off the stack
	MOVP	4+PTR_SIZE(STACKP), RESULT_DEST
#endif

	fsincos                         // try sincos
	fnstsw	%ax                     // load status word
	andw	$0x400,%ax                // check status word to see if x was in range for fsincos
	jnz		5f

	fstps	(RESULT_DEST)             // store the result
	fstps	4(RESULT_DEST)
#if defined( __LP64__ )
	ADDP	$4, STACKP                // restore the stack, since we touched it in 64-bit mode
#endif
	ret

	5:	fldpi                         // if the argument was out of range, put 2π on the stack
	fadd	%st(0)
	fxch	%st(1)

	6:	fprem1                        // while x is out of range, reduce by 2π
	fnstsw	%ax
	andw	$0x400,%ax
	jnz	6b
	fstp	%st(1)                    // clear 2π off the stack

	fsincos                         // compute fsincos (argument is surely in range here)

	fstps	(RESULT_DEST)             // store the result
	fstps	4(RESULT_DEST)
#if defined( __LP64__ )
	ADDP	$4, STACKP                // restore the stack, since we touched it in 64-bit mode
#endif
	ret
