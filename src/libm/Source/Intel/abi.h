/*	$NetBSD: abi.h,v 1.1 2001/06/19 00:26:29 fvdl Exp $	*/

/*
 * Written by Frank van der Linden (fvdl@wasabisystems.com)
 */

/*
 * The x86-64 ABI specifies that float, double and long double
 * arguments are passed in SSE2 (xmm) registers. Unfortunately,
 * there is no way to push those on to the FP stack, which is
 * where he fancier instructions get their arguments from.
 *
 * Define some prologues and epilogues to store and retrieve
 * xmm regs to local variables.
 */

//
// Define some overrideable constants
// Default settings are for minimum stack frame, all arguments are long double
//
// LOCAL_STACK_SIZE -- number of bytes in addition to the return pointer that the stack frame holds
// FIRST_ARG_SIZE -- number of bytes used on the stack by the first argument (if passed on the stack)
// SECOND_ARG_SIZE -- number of bytes used on the stack by the second argument (if passed on the stack)
//
#ifndef LOCAL_STACK_SIZE
	#define LOCAL_STACK_SIZE	0
#endif
#ifndef FIRST_ARG_SIZE
	#define FIRST_ARG_SIZE		16
#endif
#ifndef SECOND_ARG_SIZE
	#define SECOND_ARG_SIZE		16
#endif


#if defined( __LP64__ )
	#define FRAME_SIZE			8		/* sizeof( void*) */
	#define STACKP				%rsp
	#define BASEP				%rbp
	#define AX_P				%rax
	#define BX_P				%rbx
	#define CX_P				%rcx
	#define DX_P				%rdx
	#define SIGN_EXTEND_EAX		cdqe
	#define ADDP				addq
	#define CALLP				callq
    #define FISTPP              fistpll
    #define FISTTPP             fisttpll
	#define MOVP				movq
    #define NEGP                negq
	#define SUBP				subq
	#define XORP				xorq
#else	
	#define FRAME_SIZE			4		/* sizeof( void*) */
	#define STACKP				%esp
	#define BASEP				%ebp
	#define AX_P				%eax
	#define BX_P				%ebx
	#define CX_P				%ecx
	#define DX_P				%edx
	#define SIGN_EXTEND_EAX		
	#define ADDP				addl
    #define FISTPP              fistpl
    #define FISTTPP             fisttpl
	#define MOVP				movl
    #define NEGP                negl
	#define SUBP				subl
	#define XORP				xorl
	#define CALLP				calll
#endif

#define FIRST_ARG_OFFSET		(FRAME_SIZE + LOCAL_STACK_SIZE)
#define SECOND_ARG_OFFSET		(FIRST_ARG_OFFSET + FIRST_ARG_SIZE)
#define THIRD_ARG_OFFSET		(SECOND_ARG_OFFSET + SECOND_ARG_SIZE)

/*
#ifdef __LP64__

#error these interfaces appear to violate the red zone

    #define ARG_LONG_DOUBLE_ONE		-4(%esp)  
    #define ARG_LONG_DOUBLE_TWO		-20(%esp) 
    #define ARG_DOUBLE_ONE		-8(%rsp)
    #define ARG_DOUBLE_TWO		-16(%rsp)
    #define ARG_FLOAT_ONE		-4(%rsp)
    #define ARG_FLOAT_TWO		-8(%rsp)

    #define XMM_ONE_ARG_LONG_DOUBLE_PROLOGUE
    #define XMM_TWO_ARG_LONG_DOUBLE_PROLOGUE

    #define XMM_ONE_ARG_DOUBLE_PROLOGUE \
        movsd	%xmm0, ARG_DOUBLE_ONE
        
    #define XMM_TWO_ARG_DOUBLE_PROLOGUE \
        movsd	%xmm0, ARG_DOUBLE_ONE ; \
        movsd	%xmm1, ARG_DOUBLE_TWO

    #define XMM_ONE_ARG_FLOAT_PROLOGUE \
        movss	%xmm0, ARG_FLOAT_ONE

    #define XMM_TWO_ARG_FLOAT_PROLOGUE \
        movss	%xmm0, ARG_FLOAT_ONE ; \
        movss	%xmm1, ARG_FLOAT_TWO

    #define XMM_LONG_DOUBLE_EPILOGUE

    #define XMM_DOUBLE_EPILOGUE \
        fstpl ARG_DOUBLE_ONE ; \
        movsd ARG_DOUBLE_ONE, %xmm0

    #define XMM_FLOAT_EPILOGUE \
        fstps ARG_FLOAT_ONE ; \
        movss ARG_FLOAT_ONE, %xmm0
    
#else

    #define ARG_LONG_DOUBLE_ONE		4(%esp)
    #define ARG_LONG_DOUBLE_TWO		20(%esp)
    #define ARG_DOUBLE_ONE		4(%esp)
    #define ARG_DOUBLE_TWO		12(%esp)
    #define ARG_FLOAT_ONE		4(%esp)
    #define ARG_FLOAT_TWO		8(%esp)

    #define XMM_ONE_ARG_LONG_DOUBLE_PROLOGUE
    #define XMM_TWO_ARG_LONG_DOUBLE_PROLOGUE
    #define XMM_ONE_ARG_DOUBLE_PROLOGUE
    #define XMM_TWO_ARG_DOUBLE_PROLOGUE
    #define XMM_ONE_ARG_FLOAT_PROLOGUE
    #define XMM_TWO_ARG_FLOAT_PROLOGUE

    #define XMM_LONG_DOUBLE_EPILOGUE
    #define XMM_DOUBLE_EPILOGUE
    #define XMM_FLOAT_EPILOGUE

#endif
*/
