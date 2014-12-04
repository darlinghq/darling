/* Single-precision ceilf, reimplemented using integer operations
 * for improved performance, especially on in-order machines.
 *
 * Steve Canon, March 2009.
 */
 
#if defined __i386__

#include <System/i386/cpu_capabilities.h>
.set cpubits, _COMM_PAGE_CPU_CAPABILITIES

.text
.align 4
.globl _ceilf
_ceilf:
	movss	  4(%esp),			%xmm0	// load argument
	testl	  $(kHasSSE4_1),	cpubits
	jz			0f
	
	// fast path using SSE 4.1
	roundss		$0x2,	%xmm0,	%xmm0
	movss		%xmm0,		  4(%esp)
	flds	  4(%esp)
	ret

.align 4
0:	// no SSE 4.1	
	mov		  4(%esp),			%eax
	mov			$23,			%cl
	mov			%eax,			%edx
	shr			%cl,			%eax	// x >> 23
	sub			$0x7f,			%al		// unbiased exponent of x
	jb			2f						// if |x| < 1.0, goto 2
	
	sub			%al,			%cl		// 23 - exponent of x
	mov			$0xffffffff,	%eax
	jbe			1f						// if |x| >= 0x1.0p23, goto 1
	
	dec			%edx					// (x - 1)
	shl			%cl,			%eax	// m = mask for integral bits of x
	mov			%edx,			%ecx
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
	cmp			$1,				%edx	// if x > 0, goto 3
	jge			3f
	andl		$0x80000000,  4(%esp)	// copysign(0.0, x)
	flds	  4(%esp)
	ret
.align 4
3:	movl		$0x3f800000,  4(%esp)	// return 1.0
	flds	  4(%esp)
	ret

#elif defined __x86_64__

.const
.align 4
one:	.long	0x3f800000
absmask:.long	0x7fffffff

.text
.align 4
.globl _ceilf
_ceilf:
	movd		%xmm0,			%eax
	andl		absmask(%rip),	%eax
	movd		absmask(%rip),	%xmm1
	cmpl		$0x4b000000,	%eax
	andnps		%xmm0,			%xmm1
	jae			1f

	cvttps2dq	%xmm0,			%xmm2
	movdqa		%xmm0,			%xmm3
	psrad		$31,			%xmm0	// (x < 0) ? -1 : 0
	cvtdq2ps	%xmm2,			%xmm2
	pcmpgtd		%xmm2,			%xmm3	// (x >i trunc(x)) ? -1 : 0
	andnps		%xmm3,			%xmm0	// (x > trunc(x)) ? -1 : 0
	andps		one(%rip),		%xmm0	// (x > trunc(x)) ? 1.0 : 0.0
	addss		%xmm2,			%xmm0	// (x > trunc(x)) ? trunc(x) + 1.0 : trunc(x)
	orps		%xmm1,			%xmm0	// ceil(x)
1:	ret
	
#else
	#error unknown arch
#endif
