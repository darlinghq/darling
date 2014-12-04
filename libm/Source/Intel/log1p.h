// Note that while this file has a .s extension it is not intended to be assmbled directly
// but rather included with appropriate #defines in place.
	
// double log1p( double x )
ENTRY( log1p )
// Cases	
// Main case: -0.25 <= x && x < 0.5
// 	[0000000000000000, 3fe0000000000000)
//	[8000000000000000, bfd0000000000000]
// Log1p_core: compute k and jump into log with f = x, n = 0.
// Early out:	
// x is small: |x| in [0, 3f10000000000000)

// x < -0.25 || x >= 0.5:	
// 	Call log(x+1) if x == (x+1)-1, otherwise
// 	Call logup(x+1) = log(x+1+ulp(x)/2)
// Note, the Special operands could be handled by the log(x+1) call:
// 	x is NaN or inf: fff*, 7ff*
// 	x+1 <= 0: [bff0000000000000, fff0000000000000)
	
#if defined( __x86_64__ )
	movd		%xmm0,				%rax
	movd		%xmm0,				%rdx
	andq		REL_ADDR(mzero),		%rdx	// Sign bit
	andq		REL_ADDR(notmzero),		%rax	// absx
	shrq		$11,				%rdx	// sign >> 11
	addq		%rdx,				%rax	// t = x < 0?-2*x:x
	movsd		REL_ADDR(mone),			%xmm2	// -1.0
	movapd		%xmm0,				%xmm1	// x
	cmpq		REL_ADDR(half),			%rax	// t < 0.5
#else
	movl		4+FRAME_SIZE(STACKP),		%eax
	movl		4+FRAME_SIZE(STACKP),		%edx
	andl		$0x80000000,			%edx	// Sign bit
	andl		$0x7fffffff,			%eax	// absx
	call		0f				// PIC code
0:	pop		%ecx				// PIC base
	movsd		REL_ADDR(mone),			%xmm2	// -1.0
	shrl		$11,				%edx	// sign >> 11
	addl		%edx,				%eax	// t = x < 0?-2*x:x
	movsd		FRAME_SIZE(STACKP),		%xmm0
	movsd		FRAME_SIZE(STACKP),		%xmm1
	cmpl		$0x3fe00000,			%eax	// t < 0.5
#endif
	jb		7f	// Jump for main case: -0.25 < x && x < 0.5
	// x < -0.25, or x >= 0.5, or x is NoN or Inf
            // This does not work for x in [0.5-, 1.0) since we lose a bit when adding one. 
	//TBD: It might be faster to do this in parallel with the frexp in log
	//     and compute a correction that is always added. 
	
        ucomisd		%xmm2,				%xmm0	// x < -1.0 ? Since log(1+x) would be invalid
	jb		5f
	movapd		%xmm2,				%xmm3	// -1.0
	subsd		%xmm2,				%xmm0	// xp = x + 1.0 = x - -1.0
	addsd		%xmm0,				%xmm2	// xpm = xp - 1.0 = xp + -1.0
	ucomisd		%xmm1,				%xmm2	// x == xpm?
#if defined(__i386__)	
	movsd 		%xmm0,				FRAME_SIZE(STACKP) //overwrite x with xp
#endif	
	je		_log					// log(x+1.0)
	andpd		REL_ADDR(log1p_not_ulp_mask),	%xmm1   // xm
	movsd		REL_ADDR(one),			%xmm0	// 1.0
	addsd		%xmm1,				%xmm0	// xp = 1.0+xm
#if defined(__i386__)	
	movsd 		%xmm0,				FRAME_SIZE(STACKP)
#endif	
	jmp		__logup				// log((x-ulp)+1+ulp)

7:
	// %xmm0 = x
	// %xmm1 = x

////////////////////////////////////////////////////////////////	
// log1p core early out for small values
	movsd		REL_ADDR(mzero),		%xmm2	// 0x8000000000000000
	andnpd		%xmm0,				%xmm2	// absx = fabs(x) 	// TBD: already have computed absx in 64-bit mode???
	comisd		REL_ADDR(_1pm14),		%xmm2	// (absx < 0x1p-14)
	jbe		3f
	
//get_k: compute the table lookup index, k.
	SUBP		$LOCAL_STACK_SIZE, 		STACKP
	
////////////////////////////////////////////////////////////////
									// f = x
	addsd		REL_ADDR(one),				%xmm0	// fp1 = f+1
	movsd		REL_ADDR(threehalves),			%xmm2	// 1.5		
	cmpeqsd		%xmm0,				%xmm2	// (fp1 == 1.5) ? -1 : 0
	psrlq		$(52-8),			%xmm0	// top 8 bits of significand encoding of adjusted x
	paddq		%xmm2,				%xmm0		// (int)(fp1>>(52-8))  - (fp1 == 1.5); 
	movd		%xmm0,				AX_P	
	// grab before adjusting for which half we are in
        and		$0x000000ff,			AX_P	//k = (int)(head>>(52-8))  - (fp1 == 1.5); 

	// Now:
	// k = %eax
	// exp = 0
	// fp1 = (fp1 > 1.5) ? (0.5 * fp1) : (fp1);
	// %xmm1 = f = fp1 - 1 = x
	
	movsd		%xmm1,	(STACKP)
	shl		$5,	AX_P

// The lookup table is in a funny format since it has 2 long double and a single.
	// {10-byte va ; 2-byte pad ; 4-byte single a ; 10-byte lg1pa ; 6-byte pad}
#if defined( __x86_64__ )
	// in 64-bit need lea to get address
	lea		REL_ADDR(LOOKUP),	%rdx
	fldt		(%rdx,AX_P,1)	// [va]
	fldl		(STACKP)	// [va ; f]
	//halfulp stuff goes here...
	fsubs		12(%rdx,AX_P,1)	// [va ; h = f - a] // should we do f-a in x87 or SSE??? In x87 it is easy to write the load and convert from single
	fmulp		%st,	%st(1)	// [c = h * va]
	fstl		(STACKP) 	// [c] -- with double version stored on the stack
	
	fldt		16(%rdx,AX_P,1)	// [c ; lg1pa ]
#else	// i386
	fldt		(LOOKUP-0b)(%ecx,%eax,1)	// [va]
	fldl		(STACKP)			// [va ; f]

	fsubs		(LOOKUP-0b+12)(%ecx,%eax,1)	// [va ; h = f - a]
	fmulp		%st,	%st(1)			// [c = h * va]
	fstl		(STACKP) 			// [c] -- with double version stored on the stack
	fldt		(LOOKUP-0b+16)(%ecx,%eax,1)	// [c ; lg1pa ]
#endif // i386
	
	// base e specific stuff  here...
 							// [c ; lghi = 0 + lg1pa] since nd = 0;
	fldt		REL_ADDR(ln2l)			// [c ; lghi = nd + lg1pa ; ln2l]
        fmulp		%st, %st(1)			// [ c ; logbxhi = ln2l*(nd+lg1pa)]

#ifdef __SSE3__	
	movddup		(STACKP),	%xmm0	// get cd, double version of c off stack in both registers
#else
	movsd		(STACKP),	%xmm0
	movhpd		(STACKP),	%xmm0	
#endif
	movapd		%xmm0,		%xmm1	// [cd,cd]
	addpd		REL_ADDR(a01),	%xmm0	// [a0,a1] + [cd,cd]
	mulpd		%xmm1,		%xmm0	// [a0+cd,a1+cd] * [cd,cd]
	addpd		REL_ADDR(b01),	%xmm0	// vpoly = [b0,b1] + [(a0+cd)*cd,(a1+cd)*cd]
	
//long double logbec is log_b(e) * c
//double c5b is log_b(e) * c5
	fld		%st(1)		// [c ; lghi ; c]
	// in base e logbec = c; c5b = c5_e;
					// [c ; lghi ; logbec = lnel * c = c]
#define c5b c5_e
	fxch		%st(2)		// [logbec ; lghi ; c]
	fldt		REL_ADDR(c0)	// [logbec ; lghi ; c ; c0]
	fmulp		%st, %st(1)	// [logbec ; lghi ; c*c0]
	fld1				// [logbec ; lghi ; c*c0 ; 1.0]
	faddp		%st, %st(1)	// [logbec ; lghi ; 1.0 + c*c0]
	fmulp		%st, %st(2)	// [linear = logbec*(1.0 + c*c0) ; lghi]
	fxch		%st(1)		// [lghi ; linear]
	
	movapd		%xmm1,		%xmm2	// [cd, cd]
	movhpd		REL_ADDR(c5b),	%xmm1	// [c5b, cd]
	mulpd		%xmm2,		%xmm1	// vccc5 = [c5b*cd, cd*cd]
	mulpd		%xmm1,		%xmm0	// vp = vccc5 * vpoly = [c5b*cd*(b0+(a0+cd)*cd), cd*cd*(b1+(a1+cd)*cd)]
	//Need to do a horizontal multiply
	movapd		%xmm0,		%xmm1	// [ *, vplo]
	shufpd		$3,	%xmm0,	%xmm0	// [ *, vphi]
	mulsd		%xmm1,		%xmm0	// [ *, cccp = c5b*cd*(b0+(a0+cd)*cd) * cd*cd*(b1+(a1+cd)*cd)]
	movsd		%xmm0,		(STACKP)	// Store so we can load into x87
	faddl		(STACKP)		// [lghi ; logbxlo = linear + cccp]
	faddp		%st,		%st(1)	// [longResult = lghi + logbxlo]
						// Convert to double
	fstpl		(STACKP)		// result [<empty stack>]
#if defined( __x86_64__ )
	movq		(STACKP),	%xmm0	// result
#else
	fldl 		(STACKP)		// result
#endif
#undef c5b

	// Clean up stack and return
	ADDP		$LOCAL_STACK_SIZE,	STACKP
	ret

////////////////////////////////////////////////////////////////	
// Here we know that |x| <= 0x1p-14
// %xmm0	x
// %xmm1	x	
// %xmm2	absx

// TODO could have a better poly for less error, or wider interval	
3:
	comisd		REL_ADDR(_1pm54),		%xmm2	// (absx <= 0x1p-54)
	jbe		4f
	// So 0x1p-54 < |x| < 0x1p-14
	// We  compute a small poly:
	// p = ( (-0.25 * x + 0x0.55555555555555p0) * x + -0.5) * (x*x) + x
	mulsd		REL_ADDR(mquarter),	%xmm1	// -0.25 * x
	mulsd		%xmm2,			%xmm2	// x*x = absx * absx
	addsd		REL_ADDR(third),	%xmm1	// (-0.25 * x + 0x0.55555555555555p0)
	mulsd		%xmm0,			%xmm1	// (-0.25 * x + 0x0.55555555555555p0) * x
	subsd		REL_ADDR(half),		%xmm1	// ( (-0.25 * x + 0x0.55555555555555p0) * x + -0.5)
	mulsd		%xmm2,			%xmm1	// p = ( (-0.25 * x + 0x0.55555555555555p0) * x + -0.5) * (x*x)
	addsd		%xmm1,			%xmm0	// return p 
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret


4:	// if(absx < 0x1p-54) ...
	// Note: this handles denormals, 0, and -0 also
#if FENVON
	addsd		REL_ADDR(one),	%xmm1	// x + 1.0 to raise inexact since x is small (except when x == 0)
#endif
	// retrun x
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret

5:	// x < -1.0
	sqrtsd		%xmm0,				%xmm0
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret
