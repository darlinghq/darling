/* Single-precision truncf
 *
 * Reimplemented for improved performance on in-order machines and
 * machines that support SSE 4.1
 *
 * Steve Canon, March 2009.
 */
 
#include <System/i386/cpu_capabilities.h>
.set cpubits, _COMM_PAGE_CPU_CAPABILITIES

#if defined __i386__

.text
.align 4
.globl _truncf
_truncf:
	movss	  4(%esp),			%xmm0	// load argument
	testl	  $(kHasSSE4_1),	cpubits
	jz			0f
	
	roundss		$0x3,	%xmm0,	%xmm0	// fast path using SSE 4.1
	movss		%xmm0,		  4(%esp)	//
	flds	  4(%esp)					//
	ret

.align 4
0:  mov		  4(%esp),			%eax	// load the input, x
    and			$0x7f800000,    %eax	// |x|
	mov			$23,			%ecx
	sub			$0x3f800000,	%eax	// if |x| < 1.0f, goto 2
	js			2f
	sar			%cl,			%eax	// exponent(x)
	mov			$0xffffffff,	%edx
	sub			%eax,			%ecx	// 23 - exponent(x)
	js			1f						// return x if |x| >= 0x1.0p24
	shl			%cl,			%edx
	and			%edx,		  4(%esp)	// mask off non-integral bits
	cvttps2dq	%xmm0,			%xmm0	// raise inexact
1:	flds	  4(%esp)
	ret
	
.align 4
2:	// Handle |x| < 1.0 here.
	andl		$0x80000000,  4(%esp)	// copysign(0.0, x)
	cvttps2dq	%xmm0,			%xmm0	// raise inexact
	flds	  4(%esp)
	ret

#elif defined __x86_64__

.const
.align 4
mzero:	.long	0x80000000

.text
.align 4
.globl _truncf
_truncf:
	movd		%xmm0,			%eax
    and			$0x7f800000,    %eax	// |x|
	mov			$23,			%ecx
	sub			$0x3f800000,	%eax	// if |x| < 1.0f, goto 2
	js			2f
	sar			%cl,			%eax	// exponent(x)
	mov			$0xffffffff,	%edx
	sub			%eax,			%ecx	// 23 - exponent(x)
	js			1f						// return x if |x| >= 0x1.0p23
	shl			%cl,			%edx
	movd		%edx,			%xmm2
	cvttps2dq	%xmm0,			%xmm1	// raise inexact
	andps		%xmm2,			%xmm0	// mask off non-integral bits
1:	ret
.align 4
2:	cvttps2dq	%xmm0,			%xmm1	// raise inexact
	andps		mzero(%rip),	%xmm0	// copysign(0.0, x)
	ret
	
#else
	#error unknown arch
#endif
