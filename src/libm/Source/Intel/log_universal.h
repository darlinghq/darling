// Note that while this file has a .s extension it is not intended to be assmbled directly
// but rather included with appropriate #defines in place.
	
// This begins the base-universal logarithm code. 
#ifndef BASE2	
#define BASE2			0
#endif
#ifndef BASEE	
#define BASEE			0	
#endif
#ifndef BASE10
#define BASE10			0
#endif
#ifndef LOGUP
#define LOGUP			0
#endif

// Assume we need to respect the flags unless directed otherwise	
#ifndef FENVON
#define FENVON 			1
#endif
	
#if BASE2
// double log2asm( double x )
ENTRY( log2 )
#elif BASEE
#if LOGUP	
// private double logup( double x ) // log(x+ULP(x)/2)
ENTRY( _logup )
#else	
// double log( double x )
ENTRY( log )
#endif	
#elif BASE10
// double log10( double x )
ENTRY( log10 )
#else
#error "One of BASE2, BASEE, BASE10 needs to be set to 1"
#endif
	// Early out if x is NaN, Inf, 0, or <0
	// I.e., if the encoding of x is not in (0, 7ff0000000000000)
#if defined( __x86_64__ )
	movd		%xmm0,				%rax
	movd		%xmm0,				%rdx
	subq		REL_ADDR(small_cut),		%rax
	cmpq		REL_ADDR(large_cut),		%rax
#else
	movl		4+FRAME_SIZE(STACKP),		%eax
	movl		4+FRAME_SIZE(STACKP),		%edx
	movsd		FRAME_SIZE(STACKP),		%xmm0
	call		0f				// PIC code
0:	pop		%ecx				// PIC base
	sub		$0x00100000,			%eax
	cmp		$0x7fe00000,			%eax
#endif
	jae		3f	// Jump if NaN, Inf, Denorm, 0, or negative

	SUBP		$LOCAL_STACK_SIZE, 		STACKP

#if LOGUP	
	// Need to adjust h with a small correction for log1p(x) when 1+x is not representable
        //u = (x < 1.0) ? -0x1p-53 : 0x1p-54 = ...? 0xbca0000000000000 : 0x3c90000000000000; 
            // Add in the half ulp now that we know we have room.
        //h = (f - a) + u; // h = R(f-a)+ulp(x)
	movapd		%xmm0,			%xmm6		// x
	cmpltsd		REL_ADDR(one),		%xmm6		// (x < 1.0)
	movsd		REL_ADDR(logup_ulp_mask),	%xmm5	// xor(-0x1p-53, 0x1p-54) = 0x8030000000000000
	andpd		%xmm5,			%xmm6		// m = (x < 1.0) ? 0x8030000000000000 : 0
	movsd		REL_ADDR(_1pm54),	%xmm5		// 0x1p-54
	xorpd		%xmm5,			%xmm6		// m ^ 0x1p-54 = (x < 1.0) ? 0x8030000000000000 ^ 0x1p-54 : 0x1p-54
	movsd		%xmm6,			24(STACKP)	// (x < 1.0) ? -0x1p-53 : 0x1p-54
#endif	

	// Break x down into exp, f with -0.25 <= f < 0.5 and x = 2^exp(1+f)
#if defined( __x86_64__ )
	movd		%xmm0,				%r8	// u.u = encoding of x
        shrq		$(52),				%rdx	// exponent >>= 52
	andq		REL_ADDR(frexp_mant_mask),	%r8	// mantissa = u.u & 0x800fffffffffffffULL;

	movd		%xmm0,				%rcx	// encoding of xadjust
	
	// or in -1 + bias to the new exponent for the mantissa

	addq		$-1023,				%rdx	// exponent + bias (= -1023 for normal data)
1:	//This is where denormal inputs re-enter with bias compensated for denormal exponent and appropriate mantissa
	//Note: for denormal xmm0 and rcx inputs are no longer the original x
	
	shrq		$51,				%rcx	// (encoding) >> 51
        andq		$0x1,				%rcx	// upperhalf = (encoding & 0x00080000ULL) >> 51 = (upperhalf != 0ULL);
	addq		%rcx,				%rdx	// exp = bias + (int) ( exponent >> 52 ) + (upperhalf != 0ULL);
	movl		%edx,				8(STACKP) // Save exp for later. Free dx and cx.

	//Note: for denormal inputs this is no longer the original x
	movd		%xmm0,				%rax	// u.u
	// grab before adjusting for which half we are in
        andq		REL_ADDR(frexp_head_mask),	%rax	// head = u.u & 0x000ff00000000000ULL;
        shrq		$((52-8)-5),			%rax	// k<<5 = (int)(head>>(52-8))<<5;
	movd		%xmm0,				%rcx	// u.u = encoding of xadjust
        andq		REL_ADDR(frexp_half_mask),	%rcx	// upperhalf = u.u & 0x0008000000000000ULL;
	shlq		$1,				%rcx	// (upperhalf << 1)
	orq		REL_ADDR(one),			%r8	// (mantissa | 0x3ff0000000000000ULL)
	// Divide by two if significand >= 3/2
	xorq		%rcx,				%r8	// mantissa = (mantissa | 0x3ff0000000000000ULL) ^ (upperhalf << 1);
	movd		%r8,				%xmm1	// fp1
	// Now:
	// k<<5 = %eax
	// exp = %rdx
	// fp1 = %xmm1 = (fp1 > 1.5) ? (0.5 * fp1 - 1.0) : (fp1 - 1.0);
#else
	movapd		%xmm0,				%xmm1	// u.u = encoding of x
	andpd		REL_ADDR(frexp_mant_mask),	%xmm1	// mantissa = u.u & 0x800fffffffffffffULL;

	movapd		%xmm0,				%xmm2	// u.u = encoding of adjusted x
	psrlq		$32,				%xmm0	// top half of encoding of adjusted x

	// or in -1 + bias to the new exponent for the mantissa
        andpd		REL_ADDR(frexp_half_mask),		%xmm2	// upperhalf = u.u & 0x0008000000000000ULL;
        shrl		$(52-32),				%edx	// exponent >>= 52-32
	
	addl		$-1023,				%edx	// exponent + bias; bias = -1023 for normal data
	
1:	//This is where denormal inputs re-enter with bias compensated for denormal exponent and appropriate mantissa

	movapd		%xmm2,				%xmm7	// upperhalf
	psrlq		$51,				%xmm7	// (upperhalf != 0ULL) = upperhalf >> 51 == 1 or 0
	movd		%xmm7,				8(STACKP)	// (int) ( exponent >> 52 )
	add		8(STACKP),			%edx		// exp = bias + (int) ( exponent >> 52 ) + (upperhalf != 0ULL);
	movl		%edx,				8(STACKP)	// exp
	movd		%xmm0,				%eax	// Encoding of top half of x 
	// grab before adjusting for which half we are in
        andl		$0x000ff000,			%eax	// head = u.u & 0x000ff00000000000ULL;
        shrl		$(12-5),			%eax	// k<<5 = (int)(head>>(52-8-32))<<5;
	psllq		$1,				%xmm2	// (upperhalf << 1)
	orpd		REL_ADDR(xone),			%xmm1	// (mantissa | 0x3ff0000000000000ULL)
	// Divide by two if significand >= 3/2
	xorpd		%xmm2,				%xmm1	// mantissa = (mantissa | 0x3ff0000000000000ULL) ^ (upperhalf << 1);

	// Now:
	// k<<5 = %eax
	// exp = 8(STACKP)
	// fp1 = %xmm1 = (fp1 > 1.5) ? (0.5 * fp1) : (fp1);
#endif
	
// We look up a+1 rather than a, so we can work with fp1 - ap1 rather than f - a
	
#if BASE2	
	// if fp1 == 1.0 return nd // base 2 only
	movsd		REL_ADDR(one),			%xmm2	// 1.0
	ucomisd		%xmm2,				%xmm1	// f == 1.0 set cc
	je		7f
#endif 
	
	movsd		%xmm1,	(STACKP)

// The lookup table is in a funny format since it has 2 long double and a single.
	// {10-byte va ; 2-byte pad ; 4-byte single a ; 10-byte lg1pa ; 2-byte pad, 4-byte a+1}
#if defined( __x86_64__ )
	// in 64-bit need lea to get address
	lea		REL_ADDR(LOOKUP),	%rdx
	fldt		(%rdx,AX_P,1)	// [va]
	fldl		(STACKP)	// [va ; f+1]

	fsubs		28(%rdx,AX_P,1)	// [va ; h = f+1 - a+1]
#if LOGUP	
	//halfulp stuff goes here...
        //h = (f - a) + u; // h = R(f-a)+ulp(x)
	faddl		24(STACKP)			// [va ; h = (f - a) + u]
#endif //LOGUP	
	fmulp		%st,	%st(1)	// [c = h * va]
	fstl		(STACKP) 	// [c] -- with double version stored on the stack
	
	fldt		16(%rdx,AX_P,1)	// [c ; lg1pa ]
#if BASE2
	// logbxhi = nd+lg1pa; // base 2
	fiaddl		8(STACKP) 	// [c ; lghi = nd + lg1pa]
#elif BASEE
	// base e stuff goes here...
	fiaddl		8(STACKP) 	// [c ; lghi = nd + lg1pa]
	fldt		REL_ADDR(ln2l)	// [c ; lghi = nd + lg1pa ; ln2l]
        fmulp		%st, %st(1)	// [ c ; logbxhi = ln2l*(nd+lg1pa)]
#elif BASE10
	// base 10 stuff goes here...
	// free(%rax)
	// free(%rdx)
#if FENVON
	movd		%xmm0,					%rdx	// Original x encoding
	shrq		$(53-4),				%rdx	// 16*k = k << 4. k = key >> 53 = the top 6 of bottom 7 bits of exponent of x
        andq		$0x3f,					%rdx	// key = _mm_and_pd(vx,log10_key_mask);
	movd		%xmm0,					%rax	// Original x encoding
	lea		REL_ADDR(isPowerOf10),			%rcx	// Get base of table
	cmp		(%rcx,%rdx,1),				%rax	// if(isPowerOf10[k][0] == x) {return  isPowerOf10[k][1];}
	je		8f
#endif //FENVON base 10 64-bit
	fiaddl		8(STACKP) 			// [c ; lghi = nd + lg1pa]
	fldt		REL_ADDR(log102l)		// [c ; lghi = nd + lg1pa ; log102l]
        fmulp		%st, %st(1)			// [ c ; logbxhi = log102l*(nd+lg1pa)]
#endif
	
#else	// i386
	fldt		(LOOKUP-0b)(%ecx,%eax,1)	// [va]
	fldl		(STACKP)			// [va ; f+1]
	//halfulp stuff goes here...
	fsubs		(LOOKUP-0b+28)(%ecx,%eax,1)	// [va ; h = f+1 - a+1]
#if LOGUP	
        //h = (f - a) + u; // h = R(f-a)+ulp(x)
	faddl		24(STACKP)			// [va ; h = (f - a) + u]
#endif //LOGUP	
	fmulp		%st,	%st(1)			// [c = h * va]
	fstl		(STACKP) 			// [c] -- with double version stored on the stack
	fldt		(LOOKUP-0b+16)(%ecx,%eax,1)	// [c ; lg1pa ]
	
#if BASE2
	// logbxhi = nd+lg1pa; // base 2
	fiaddl		8(STACKP) 			// [c ; lghi = nd + lg1pa]
#elif BASEE
	// base e stuff goes here...
	fiaddl		8(STACKP) 			// [c ; lghi = nd + lg1pa]
	fldt		REL_ADDR(ln2l)			// [c ; lghi = nd + lg1pa ; ln2l]
        fmulp		%st, %st(1)			// [ c ; logbxhi = ln2l*(nd+lg1pa)]
#elif BASE10
	// base 10 stuff goes here...
	// free(%edx)
#if FENVON
	movl		(4+LOCAL_STACK_SIZE)+FRAME_SIZE(STACKP),	%edx
        andl		$0x07e00000,				%edx	// key = _mm_and_pd(vx,log10_key_mask);
	shrl		$((53-32)-4),				%edx	// 16*k = k << 4. k = key >> 53 = the top 6 of bottom 7 bits of exponent of x
	movsd		(LOCAL_STACK_SIZE)+FRAME_SIZE(STACKP),	%xmm5
	comisd		(isPowerOf10-0b)(%ecx,%edx,1),		%xmm5	// if(isPowerOf10[k][0] == x) {return  isPowerOf10[k][1];}
	je		8f
#endif //FENVON base 10 i386
	fiaddl		8(STACKP) 			// [c ; lghi = nd + lg1pa]
	fldt		REL_ADDR(log102l)		// [c ; lghi = nd + lg1pa ; log102l]
        fmulp		%st, %st(1)			// [ c ; logbxhi = log102l*(nd+lg1pa)]
#endif //base 10 i386
	
#endif // i386

	
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
	fld		%st(1)	// [c ; lghi ; c]
#if BASE2
	// in base 2 logbec = lgel * c; c5b = c5_2;
	fldt		REL_ADDR(lgel)	// [c ; lghi ; c ; lgel]
	fmulp		%st, %st(1)	// [c ; lghi ; logbec = lgel * c]
#define c5b c5_2	
#elif BASEE
	// in base e logbec = c; c5b = c5_e;
					// [c ; lghi ; logbec = lnel * c = c]
#define c5b c5_e
#elif BASE10
	// in base 10 logbec = log10el * c; c5b = c5_10;
	fldt		REL_ADDR(log10el)	// [c ; lghi ; log10el ; c]
	fmulp		%st, %st(1)	// [c ; lghi ; logbec = log10el * c]
#define c5b c5_10
#endif
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
	//Do a horizontal multiply.
	movapd		%xmm0,		%xmm1	// [ *, vplo]
	shufpd		$3,	%xmm0,	%xmm0	// [ *, vphi]
	mulsd		%xmm1,		%xmm0	// [ *, cccp = c5b*cd*(b0+(a0+cd)*cd) * cd*cd*(b1+(a1+cd)*cd)]
	movsd		%xmm0,		(STACKP)	// Store so we can load into x87
	faddl		(STACKP)		// [lghi ; logbxlo = linear + cccp]
	faddp		%st,		%st(1)	// [longResult = lghi + logbxlo]
						// Round to double
	fstpl		(STACKP)		// result [<empty stack>]
#if defined( __x86_64__ )
	movq		(STACKP),	%xmm0	// result
#else
	fldl 		(STACKP)
#endif
#undef c5b

2:	// Clean up stack and return
	ADDP		$LOCAL_STACK_SIZE,	STACKP
	ret

////////////////////////////////////////////////////////////////	
	
	
3:	// Special operand handling: NaN, Inf, Denorm, 0, or negative
	// When we get here AX has as encoding(x) - 0x00100000[00000000 in 64-bit]
	// +denormal -> denormal handling
	// +zero -> -inf #ZERO_DIVIDE
	// -stuff -> sqrt(x) (pass quiet NaNs or raise invalid)
	// -zero -> -inf #ZERO_DIVIDE
	// +NaN -> x+x
	// +inf -> x+x	
	xorpd		%xmm2,			%xmm2	// 0.0
	ucomisd		%xmm2,			%xmm0	// f == 0 set cc
	jp		9f	// NaN:  compares unequal and signals invalid only for signaling NaN
	je		4f	// Zero: x == 0.0 -> -1 / zero

        // We now need to distinguish between Denormal, Negative, and NaN/Inf.
        // Adding the small_cut back in sets the EFLAGS for conditional branches
#if defined( __x86_64__ )
	addq		REL_ADDR(small_cut),	%rax
#else
	addl		$0x00100000,		%eax
#endif
	jb		5f	// Denormal (i.e., Inf > x > 0 and we have ruled out other cases)
	js		6f	// Negative sign (may be "-NaN", -Inf, -Normal, or -Denormal)
	
	// If we fall through to here, we have "+NaN" or +Inf
9:
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret

4:	// Zero: x == 0.0 -> -1 / zero
	//We need to return -Inf for zero and set div/0 flag
	xorpd 		%xmm1,			%xmm1
	pcmpeqb 	%xmm0,                  %xmm0              // -1U
	cvtdq2pd 	%xmm0, 			%xmm0              // -1.0f
	divsd   	%xmm1,			%xmm0              // -1.0f / 0 = -Inf + div/0 flag
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret
	
5:	// Denormal						     
//Denormal inputs are frexped with extra care and then we branch back into the mainline code.
	
	SUBP		$LOCAL_STACK_SIZE, 		STACKP
	// Break x down into exp, f with -0.25 <= f < 0.5 and x = 2^exp(1+f)
	// knowing that we are starting with denormal x
#if defined( __x86_64__ )
	movd		%xmm0,				%r8	// u.u = encoding of x
	andq		REL_ADDR(frexp_mant_mask),	%r8	// mantissa = u.u & 0x800fffffffffffffULL;
        // or in bias to the new exponent for the mantissa
        orq		REL_ADDR(one),			%r8	// mantissa |= 0x3ff0000000000000ULL;
	movd		%r8,				%xmm0	// mantissa
	subsd		REL_ADDR(one),			%xmm0
	movd		%xmm0,				%rdx	// encoding rescaled input
	movd		%xmm0,				%r8	// encoding rescaled input
        shrq		$(52),				%rdx	// exponent >>= 52
	andq		REL_ADDR(frexp_mant_mask),	%r8	// mantissa = u.u & 0x800fffffffffffffULL;	
	movd		%xmm0,				%rcx	// encoding of xadjust
	// or in -1 + bias to the new exponent for the mantissa
	addq		$(-1023-1022),			%rdx	// exponent + bias (= -1023-1022 for subnormal data)
#else
	movapd		%xmm0,				%xmm1	// u.u = encoding of x
        // or in bias to the new exponent for the mantissa
	movq		REL_ADDR(one),			%xmm2	// 1.0
        orpd		%xmm2,				%xmm0	// mantissa =  _mm_or_pd(x, xOne);
	subsd		%xmm2,				%xmm0	// u = mantissa - xOne;
	movapd		%xmm0,				%xmm1	// u.u = encoding of adjusted x
	movapd		%xmm0,				%xmm7	// u.u = encoding of adjusted x (but we only want the high half)
	psrlq		$32,				%xmm7	// top half of encoding of adjusted x
	movd		%xmm7,				%edx	// top half of encoding of adjusted x
	andpd		REL_ADDR(frexp_mant_mask),	%xmm1	// mantissa = u.u & 0x800fffffffffffffULL;

	movapd		%xmm0,				%xmm2	// u.u = encoding of adjusted x
	psrlq		$32,				%xmm0	// top half of encoding of adjusted x
	
	// or in -1 + bias to the new exponent for the mantissa
        andpd		REL_ADDR(frexp_half_mask),		%xmm2	// upperhalf = u.u & 0x0008000000000000ULL;
        shrl		$(52-32),				%edx	// exponent >>= 52-32
	
	addl		$(-1023-1022),				%edx	// exponent + bias; bias = -1023-1022 for subnormal data
	
#endif
	jmp		1b	// Re-enter main line with bias compensated for denormal exponent and appropriate mantissa


6:	// Negative sign (may be "-NaN", -Inf, -Normal, or -Denormal)	
	// In any case sqrt(x) will do the right thing.
	// -QNaN will pass through
	// -SNaN will raise invalid and be quieted	
	// -N will raise invalid and return a quiet NaN
	sqrtsd		%xmm0,				%xmm0
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret
    
#if BASE2	
7:	 //early out for power of 2
	ADDP 		$LOCAL_STACK_SIZE,	STACKP
#if defined( __i386__ )
	movl 		%edx, 			FRAME_SIZE( STACKP )
	fildl 		FRAME_SIZE(STACKP)
#else
	cvtsi2sd 	%rdx, 			%xmm0
#endif
	ret
#endif
	
// This base10 branch only needed if FENVON
#if FENVON	
8: 	// If(isPowerOf10[k][0] == x) {return  isPowerOf10[k][1];}
	// Come in with two things on the x87 stack			// [c ; lg1pa ]
	fstp		%st						// [c ]
	fstp		%st						// [<empty>]
#if defined( __i386__ )
	movsd		(isPowerOf10-0b+8)(%ecx,%edx,1),	%xmm0	// isPowerOf10[k][1]
#else	
	movsd		8(%rcx,%rdx,1),				%xmm0	// isPowerOf10[k][1]
#endif
	// Clean up stack and return
	ADDP		$LOCAL_STACK_SIZE,	STACKP
	
#if defined( __i386__ ) 	// Light cleanup required
	movsd 		%xmm0,                  FRAME_SIZE(STACKP)
	fldl 		FRAME_SIZE(STACKP)
#endif
	ret
#endif
	
////////////////////////////////////////////////////////////////	
