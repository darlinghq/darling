/*
 * Copyright (c) 1999-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * C runtime startup for interface to the dynamic linker.
 * This is the same as the entry point in crt0.o with the addition of the
 * address of the mach header passed as the an extra first argument.
 *
 * Kernel sets up stack frame to look like:
 *
 *	| STRING AREA |
 *	+-------------+
 *	|      0      |
*	+-------------+
 *	|  apple[n]   |
 *	+-------------+
 *	       :
 *	+-------------+
 *	|  apple[0]   |
 *	+-------------+
 *	|      0      |
 *	+-------------+
 *	|    env[n]   |
 *	+-------------+
 *	       :
 *	       :
 *	+-------------+
 *	|    env[0]   |
 *	+-------------+
 *	|      0      |
 *	+-------------+
 *	| arg[argc-1] |
 *	+-------------+
 *	       :
 *	       :
 *	+-------------+
 *	|    arg[0]   |
 *	+-------------+
 *	|     argc    |
 *	+-------------+
 * sp->	|      mh     | address of where the a.out's file offset 0 is in memory
 *	+-------------+
 *
 *	Where arg[i] and env[i] point into the STRING AREA
 */


#include <TargetConditionals.h>

	.globl __dyld_start

#if __i386__ && !TARGET_OS_SIMULATOR
	.text
	.align	4, 0x90
	.globl __dyld_start
__dyld_start:
	popl	%edx		# edx = mh of app
	pushl	$0		# push a zero for debugger end of frames marker
	movl	%esp,%ebp	# pointer to base of kernel frame
	andl    $-16,%esp       # force SSE alignment
	subl	$32,%esp	# room for locals and outgoing parameters

	call    L__dyld_start_picbase
L__dyld_start_picbase:
	popl	%ebx		# set %ebx to runtime value of picbase

	# call dyldbootstrap::start(app_mh, argc, argv, dyld_mh, &startGlue)
   	subl	$L__dyld_start_picbase-__dyld_start, %ebx # ebx = &__dyld_start
	subl	$0x1000, %ebx	# ebx = load address of dyld
	movl	%edx,(%esp)	# param1 = app_mh
	movl	4(%ebp),%eax
	movl	%eax,4(%esp)	# param2 = argc
	lea     8(%ebp),%eax
	movl	%eax,8(%esp)	# param3 = argv
	movl	%ebx,12(%esp)	# param4 = dyld load address
	lea	28(%esp),%eax
	movl	%eax,16(%esp)	# param5 = &startGlue
	call	__ZN13dyldbootstrap5startEPKN5dyld311MachOLoadedEiPPKcS3_Pm
	movl	28(%esp),%edx
	cmpl	$0,%edx
	jne	Lnew

    	# clean up stack and jump to "start" in main executable
	movl	%ebp,%esp	# restore the unaligned stack pointer
	addl	$4,%esp		# remove debugger end frame marker
	movl	$0,%ebp		# restore ebp back to zero
	jmp	*%eax		# jump to the entry point

	# LC_MAIN case, set up stack for call to main()
Lnew:	movl	4(%ebp),%ebx
	movl	%ebx,(%esp)	# main param1 = argc
	leal	8(%ebp),%ecx
	movl	%ecx,4(%esp)	# main param2 = argv
	leal	0x4(%ecx,%ebx,4),%ebx
	movl	%ebx,8(%esp)	# main param3 = env
Lapple:	movl	(%ebx),%ecx	# look for NULL ending env[] array
	add	$4,%ebx
	testl	%ecx,%ecx
	jne	Lapple		# once found, next pointer is "apple" parameter now in %ebx
	movl	%ebx,12(%esp)	# main param4 = apple
	pushl	%edx		# simulate return address into _start in libdyld
	jmp	*%eax		# jump to main(argc,argv,env,apple) with return address set to _start

#endif /* __i386__  && !TARGET_OS_SIMULATOR*/



#if __x86_64__ && !TARGET_OS_SIMULATOR
	.text
	.align 2,0x90
	.globl __dyld_start
__dyld_start:
	popq	%rdi		# param1 = mh of app
	pushq	$0		# push a zero for debugger end of frames marker
	movq	%rsp,%rbp	# pointer to base of kernel frame
	andq    $-16,%rsp       # force SSE alignment
	subq	$16,%rsp	# room for local variables

	# call dyldbootstrap::start(app_mh, argc, argv, dyld_mh, &startGlue)
	movl	8(%rbp),%esi	# param2 = argc into %esi
	leaq	16(%rbp),%rdx	# param3 = &argv[0] into %rdx
	leaq	___dso_handle(%rip),%rcx # param4 = dyldsMachHeader into %rcx
	leaq	-8(%rbp),%r8    # param5 = &glue into %r8
	call	__ZN13dyldbootstrap5startEPKN5dyld311MachOLoadedEiPPKcS3_Pm
	movq	-8(%rbp),%rdi
	cmpq	$0,%rdi
	jne	Lnew

    	# clean up stack and jump to "start" in main executable
	movq	%rbp,%rsp	# restore the unaligned stack pointer
	addq	$8,%rsp 	# remove the mh argument, and debugger end frame marker
	movq	$0,%rbp		# restore ebp back to zero
	jmp	*%rax		# jump to the entry point

	# LC_MAIN case, set up stack for call to main()
Lnew:	addq	$16,%rsp	# remove local variables
	pushq	%rdi		# simulate return address into _start in libdyld
	movq	8(%rbp),%rdi	# main param1 = argc into %rdi
	leaq	16(%rbp),%rsi	# main param2 = &argv[0] into %rsi
	leaq	0x8(%rsi,%rdi,8),%rdx # main param3 = &env[0] into %rdx
 	movq	%rdx,%rcx
Lapple: movq	(%rcx),%r8
	add	$8,%rcx
	testq	%r8,%r8		# look for NULL ending env[] array
	jne	Lapple		# main param4 = apple into %rcx
	jmp	*%rax		# jump to main(argc,argv,env,apple) with return address set to _start
#endif /* __x86_64__ && !TARGET_OS_SIMULATOR*/



#if __arm__
	.text
	.align 2
__dyld_start:
	mov	r8, sp		// save stack pointer
	sub	sp, #16		// make room for outgoing parameters
	bic     sp, sp, #15	// force 16-byte alignment

	// call dyldbootstrap::start(app_mh, argc, argv, dyld_mh, &startGlue)
	ldr	r0, [r8]	// r0 = mach_header
	ldr	r1, [r8, #4]	// r1 = argc
	add	r2, r8, #8	// r2 = argv
	adr	r3, __dyld_start
	sub	r3 ,r3, #0x1000 // r3 = dyld_mh
	add	r4, sp, #12
	str	r4, [sp, #0]	// [sp] = &startGlue

	bl	__ZN13dyldbootstrap5startEPKN5dyld311MachOLoadedEiPPKcS3_Pm
	ldr	r5, [sp, #12]
	cmp	r5, #0
	bne	Lnew

	// traditional case, clean up stack and jump to result
	add	sp, r8, #4	// remove the mach_header argument.
	bx	r0		// jump to the program's entry point

	// LC_MAIN case, set up stack for call to main()
Lnew:	mov	lr, r5		    // simulate return address into _start in libdyld
	mov	r5, r0		    // save address of main() for later use
	ldr	r0, [r8, #4]	    // main param1 = argc
	add	r1, r8, #8	    // main param2 = argv
	add	r2, r1, r0, lsl #2
	add	r2, r2, #4	    // main param3 = &env[0]
	mov	r3, r2
Lapple:	ldr	r4, [r3]
	add	r3, #4
	cmp	r4, #0
	bne	Lapple		    // main param4 = apple
	bx	r5

#endif /* __arm__ */




#if __arm64__
	.text
	.align 2
	.globl __dyld_start
__dyld_start:
	mov 	x28, sp
	and     sp, x28, #~15		// force 16-byte alignment of stack
	mov	x0, #0
	mov	x1, #0
	stp	x1, x0, [sp, #-16]!	// make aligned terminating frame
	mov	fp, sp			// set up fp to point to terminating frame
	sub	sp, sp, #16             // make room for local variables
#if __LP64__
	ldr     x0, [x28]               // get app's mh into x0
	ldr     x1, [x28, #8]           // get argc into x1 (kernel passes 32-bit int argc as 64-bits on stack to keep alignment)
	add     x2, x28, #16            // get argv into x2
#else
	ldr     w0, [x28]               // get app's mh into x0
	ldr     w1, [x28, #4]           // get argc into x1 (kernel passes 32-bit int argc as 64-bits on stack to keep alignment)
	add     w2, w28, #8             // get argv into x2
#endif
	adrp	x3,___dso_handle@page
	add 	x3,x3,___dso_handle@pageoff // get dyld's mh in to x4
	mov	x4,sp                   // x5 has &startGlue

	// call dyldbootstrap::start(app_mh, argc, argv, dyld_mh, &startGlue)
	bl	__ZN13dyldbootstrap5startEPKN5dyld311MachOLoadedEiPPKcS3_Pm
	mov	x16,x0                  // save entry point address in x16
#if __LP64__
	ldr     x1, [sp]
#else
	ldr     w1, [sp]
#endif
	cmp	x1, #0
	b.ne	Lnew

	// LC_UNIXTHREAD way, clean up stack and jump to result
#if __LP64__
	add	sp, x28, #8             // restore unaligned stack pointer without app mh
#else
	add	sp, x28, #4             // restore unaligned stack pointer without app mh
#endif
#if __arm64e__
	braaz   x16                     // jump to the program's entry point
#else
	br      x16                     // jump to the program's entry point
#endif

	// LC_MAIN case, set up stack for call to main()
Lnew:	mov	lr, x1		    // simulate return address into _start in libdyld.dylib
#if __LP64__
	ldr	x0, [x28, #8]       // main param1 = argc
	add	x1, x28, #16        // main param2 = argv
	add	x2, x1, x0, lsl #3
	add	x2, x2, #8          // main param3 = &env[0]
	mov	x3, x2
Lapple:	ldr	x4, [x3]
	add	x3, x3, #8
#else
	ldr	w0, [x28, #4]       // main param1 = argc
	add	x1, x28, #8         // main param2 = argv
	add	x2, x1, x0, lsl #2
	add	x2, x2, #4          // main param3 = &env[0]
	mov	x3, x2
Lapple:	ldr	w4, [x3]
	add	x3, x3, #4
#endif
	cmp	x4, #0
	b.ne	Lapple		    // main param4 = apple
#if __arm64e__
	braaz   x16
#else
	br      x16
#endif

#endif // __arm64__


// When iOS 10.0 simulator runs on 10.11, abort_with_payload() does not exist,
// so it falls back and uses dyld_fatal_error().
#if TARGET_OS_SIMULATOR
	.text
	.align 2
	.globl	_dyld_fatal_error
_dyld_fatal_error:
    int3
    nop
#endif





