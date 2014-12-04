/* Single-precision floorf, reimplemented using integer operations
 * for improved performance, especially on in-order machines.
 *
 * Steve Canon, March 2009.
 */

#if defined __i386__
 
#include <System/i386/cpu_capabilities.h>
.set cpubits, _COMM_PAGE_CPU_CAPABILITIES

.text
.align 4
.globl _floorf
_floorf:
	movss	  4(%esp),			%xmm0	// load argument
	testl	  $(kHasSSE4_1),	cpubits
	jz			0f
	
	// fast path using SSE 4.1
	roundss		$0x1,	%xmm0,	%xmm0
	movss		%xmm0,		  4(%esp)
	flds	  4(%esp)
	ret

.align 4
0:	// no SSE 4.1
	mov		  4(%esp),			%eax
	mov			$23,			%cl
	mov			%eax,			%edx
	shr			%cl,			%eax
	dec			%edx
	sub			$0x7f,			%al
	jb			2f
	
	sub			%al,			%cl
	mov			$0xffffffff,	%eax
	jbe			1f

	shl			%cl,			%eax	// m = mask for integral bits of x
	mov			%edx,			%ecx
	not			%edx
	sar			$31,			%edx	// (x < 0) ? -1 : 0
	or			%eax,			%edx	// (x < 0) ? -1 : m
	sub			%edx,			%ecx	// (x < 0) ? x : (x + (1.0 - ulp(x)))
	and			%ecx,			%eax	// ceil(x)
	mov			%eax,		  4(%esp)
	cvttps2dq	%xmm0,			%xmm0	// set inexact
1:	flds	  4(%esp)
	ret
.align 4
2:	cvttps2dq	%xmm0,			%xmm0	// set inexact
	cmp			$0xffffffff,	%edx	// if x < 0.0, goto 3
	jl			3f
	andl		$0x80000000,  4(%esp)	// copysign(0.0, x)
	flds	  4(%esp)
	ret
.align 4
3:	movl		$0xbf800000,  4(%esp)	// return 1.0
	flds	  4(%esp)
	ret

#elif defined __x86_64__

.const
.align 4
mone:	.long	0xbf800000
absmask:.long	0x7fffffff

.text
.align 4
.globl _floorf
_floorf:
    movd		%xmm0,			%ecx
    andl		$0x7fffffff,	%ecx	// |x|
    subl		$1,				%ecx	// subtract 1. This forces |+-0| to -0
    cmpl		$0x4afffffe,	%ecx	// values >= 0x4b000000 - 1 are either integers, NaN or Inf	
    ja			1f						// unsigned compare adds 0 to the list

    cvttps2dq   %xmm0,			%xmm2
    cvtdq2ps    %xmm2,			%xmm2
    cmpltss		%xmm2,			%xmm0
    andps		mone(%rip),		%xmm0
    addss		%xmm2,			%xmm0
1:	ret

#else
	#error unknown arch
#endif
