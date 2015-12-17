/* 	This files defines _aes_encrypt_hw and _aes_decrypt_hw --- Intel Westmere HW AES-based implementation
	of _aes_encrypt and _aes_decrypt. 

	These 2 functions SHOULD BE entried ONLY after the AES HW is verified to be available. 
	They SHOULD NOT be called without AES HW detection. It might cause xnu to crash.

	The AES HW is detected 1st thing in 
		_aes_encrypt (EncryptDecrypt.s) 
		_aes_decrypt (EncryptDecrypt.s)
	and, if AES HW is detected, branch without link (ie, jump) to the functions here.

	The implementation here follows the examples in an Intel White Paper
	"Intel Advanced Encryption Standard (AES) Instruction Set" Rev.2 01

	Note: Rev. 03 Final 2010 01 26 is available. Looks like some code change from Rev.2 01

	cclee 3-13-10
*/
#if defined __i386__ || defined __x86_64__

    .text
    .align 4,0x90
.globl	_aes_encrypt_hw
_aes_encrypt_hw:

#if	defined	__i386__	
	movl	4(%esp), %eax	// in
	movl	12(%esp), %edx	// ctx
	movl	8(%esp), %ecx	// out

	#define	LOCAL_SIZE	(12+16+16)		// 16-byte align (-4 for return address) + 16 (xmm0) + 16 (xmm1)
	#define	in		%eax
	#define	ctx		%edx
	#define	out		%ecx
	#define	r13		%esp

#else		// x86_64

	#define	LOCAL_SIZE	(8+16+16)		// 16-byte align (-8 for return address) + 16 (xmm0) + 16 (xmm1)
	#define	in			%rdi
	#define	ctx			%rdx
	#define	out			%rsi
	#define	r13			%rsp

#endif		// i386 or x86_64

#ifdef KERNEL
	sub		$LOCAL_SIZE, r13
	movaps	%xmm0, (r13)
#endif
	movups	(in), %xmm0

	// key length identification
	movl	240(ctx), %eax			// key length
	cmp		$160, %eax
	je		L_AES_128
	cmp		$192, %eax
	je		L_AES_192
	cmp		$224, %eax
	je		L_AES_256
	mov		$-1, %eax					// return ERROR
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret

L_AES_128:
	testb	$15, %dl					// check whether expanded key is 16-byte aligned
	jne		0f							// if not 16-byte aligned, aesenc xmm, m128 won't work	
	pxor	(ctx), %xmm0
	aesenc	16(ctx), %xmm0
	aesenc	32(ctx), %xmm0
	aesenc	48(ctx), %xmm0
	aesenc	64(ctx), %xmm0
	aesenc	80(ctx), %xmm0
	aesenc	96(ctx), %xmm0
	aesenc	112(ctx), %xmm0
	aesenc	128(ctx), %xmm0
	aesenc	144(ctx), %xmm0
	aesenclast	160(ctx), %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret
0:										// special case expanded key is not 16-byte aligned	
#ifdef	KERNEL
	movaps	%xmm1, 16(r13)				// save xmm1 into stack
#endif
	movups	(ctx), %xmm1
	pxor	%xmm1, %xmm0
	movups	16(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	32(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	48(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	64(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	80(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	96(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	112(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	128(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	144(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	160(ctx), %xmm1
	aesenclast	%xmm1, %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	movaps	16(r13), %xmm1
	add		$LOCAL_SIZE, r13
#endif
	ret

L_AES_192:
	testb	$15, %dl					// check whether expanded key is 16-byte aligned
	jne		0f							// if not 16-byte aligned, aesenc xmm, m128 won't work	
	pxor	(ctx), %xmm0
	aesenc	16(ctx), %xmm0
	aesenc	32(ctx), %xmm0
	aesenc	48(ctx), %xmm0
	aesenc	64(ctx), %xmm0
	aesenc	80(ctx), %xmm0
	aesenc	96(ctx), %xmm0
	aesenc	112(ctx), %xmm0
	aesenc	128(ctx), %xmm0
	aesenc	144(ctx), %xmm0
	aesenc	160(ctx), %xmm0
	aesenc	176(ctx), %xmm0
	aesenclast	192(ctx), %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret
0:										// special case expanded key is not 16-byte aligned	
#ifdef	KERNEL
	movaps	%xmm1, 16(r13)				// save xmm1 into stack
#endif
	movups	(ctx), %xmm1
	pxor	%xmm1, %xmm0
	movups	16(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	32(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	48(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	64(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	80(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	96(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	112(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	128(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	144(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	160(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	176(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	192(ctx), %xmm1
	aesenclast	%xmm1, %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	movaps	16(r13), %xmm1
	add		$LOCAL_SIZE, r13
#endif
	ret

L_AES_256:
	testb	$15, %dl					// check whether expanded key is 16-byte aligned
	jne		0f							// if not 16-byte aligned, aesenc xmm, m128 won't work	
	pxor	(ctx), %xmm0
	aesenc	16(ctx), %xmm0
	aesenc	32(ctx), %xmm0
	aesenc	48(ctx), %xmm0
	aesenc	64(ctx), %xmm0
	aesenc	80(ctx), %xmm0
	aesenc	96(ctx), %xmm0
	aesenc	112(ctx), %xmm0
	aesenc	128(ctx), %xmm0
	aesenc	144(ctx), %xmm0
	aesenc	160(ctx), %xmm0
	aesenc	176(ctx), %xmm0
	aesenc	192(ctx), %xmm0
	aesenc	208(ctx), %xmm0
	aesenclast	224(ctx), %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret
0:										// special case expanded key is not 16-byte aligned	
#ifdef	KERNEL
	movaps	%xmm1, 16(r13)				// save xmm1 into stack
#endif
	movups	(ctx), %xmm1
	pxor	%xmm1, %xmm0
	movups	16(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	32(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	48(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	64(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	80(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	96(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	112(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	128(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	144(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	160(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	176(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	192(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	208(ctx), %xmm1
	aesenc	%xmm1, %xmm0
	movups	224(ctx), %xmm1
	aesenclast	%xmm1, %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	movaps	16(r13), %xmm1
	add		$LOCAL_SIZE, r13
#endif
	ret


    .text
    .align 4,0x90
.globl	_aes_decrypt_hw
_aes_decrypt_hw:

#if	defined	__i386__	
	movl	4(%esp), %eax	// in
	movl	12(%esp), %edx	// ctx
	movl	8(%esp), %ecx	// out

#endif

#ifdef KERNEL
	sub		$LOCAL_SIZE, r13
	movaps	%xmm0, (r13)
#endif
	movups	(in), %xmm0

	// key length identification
	movl	240(ctx), %eax			// key length
	cmp		$160, %eax
	je		0f						// AES-128
	cmp		$192, %eax
	je		1f						// AES-192
	cmp		$224, %eax
	je		2f						// AES-256
	mov		$-1, %eax				// return ERROR
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret

0:									// AES-128
	testb	$15, %dl					// check whether expanded key is 16-byte aligned
	jne		9f							// if not 16-byte aligned, aesenc xmm, m128 won't work	
	pxor	160(ctx), %xmm0
	aesdec	144(ctx), %xmm0
	aesdec	128(ctx), %xmm0
	aesdec	112(ctx), %xmm0
	aesdec	96(ctx), %xmm0
	aesdec	80(ctx), %xmm0
	aesdec	64(ctx), %xmm0
	aesdec	48(ctx), %xmm0
	aesdec	32(ctx), %xmm0
	aesdec	16(ctx), %xmm0
	aesdeclast	(ctx), %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret
9:										// AES-128 Decrypt : special case expanded key is not 16-byte aligned 
#ifdef	KERNEL
	movaps	%xmm1, 16(r13)				// save xmm1 into stack
#endif
	movups	160(ctx), %xmm1
	pxor	%xmm1, %xmm0
	movups	144(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	128(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	112(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	96(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	80(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	64(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	48(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	32(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	16(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	(ctx), %xmm1
	aesdeclast	%xmm1, %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	movaps	16(r13), %xmm1	
	add		$LOCAL_SIZE, r13
#endif
	ret

1:								// AES-192
	testb	$15, %dl					// check whether expanded key is 16-byte aligned
	jne		9f							// if not 16-byte aligned, aesenc xmm, m128 won't work	
	pxor	192(ctx), %xmm0
	aesdec	176(ctx), %xmm0
	aesdec	160(ctx), %xmm0
	aesdec	144(ctx), %xmm0
	aesdec	128(ctx), %xmm0
	aesdec	112(ctx), %xmm0
	aesdec	96(ctx), %xmm0
	aesdec	80(ctx), %xmm0
	aesdec	64(ctx), %xmm0
	aesdec	48(ctx), %xmm0
	aesdec	32(ctx), %xmm0
	aesdec	16(ctx), %xmm0
	aesdeclast	(ctx), %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret
9:										// AES-192 Decrypt : special case expanded key is not 16-byte aligned 
#ifdef	KERNEL
	movaps	%xmm1, 16(r13)				// save xmm1 into stack
#endif
	movups	192(ctx), %xmm1
	pxor	%xmm1, %xmm0
	movups	176(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	160(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	144(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	128(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	112(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	96(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	80(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	64(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	48(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	32(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	16(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	(ctx), %xmm1
	aesdeclast	%xmm1, %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	movaps	16(r13), %xmm1	
	add		$LOCAL_SIZE, r13
#endif
	ret

2:							// AES-256
	testb	$15, %dl					// check whether expanded key is 16-byte aligned
	jne		9f							// if not 16-byte aligned, aesenc xmm, m128 won't work	
	pxor	224(ctx), %xmm0
	aesdec	208(ctx), %xmm0
	aesdec	192(ctx), %xmm0
	aesdec	176(ctx), %xmm0
	aesdec	160(ctx), %xmm0
	aesdec	144(ctx), %xmm0
	aesdec	128(ctx), %xmm0
	aesdec	112(ctx), %xmm0
	aesdec	96(ctx), %xmm0
	aesdec	80(ctx), %xmm0
	aesdec	64(ctx), %xmm0
	aesdec	48(ctx), %xmm0
	aesdec	32(ctx), %xmm0
	aesdec	16(ctx), %xmm0
	aesdeclast	(ctx), %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	add		$LOCAL_SIZE, r13
#endif
	ret
9:										// AES-256 Decrypt : special case expanded key is not 16-byte aligned 
#ifdef	KERNEL
	movaps	%xmm1, 16(r13)				// save xmm1 into stack
#endif
	movups	224(ctx), %xmm1
	pxor	%xmm1, %xmm0
	movups	208(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	192(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	176(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	160(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	144(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	128(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	112(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	96(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	80(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	64(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	48(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	32(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	16(ctx), %xmm1
	aesdec	%xmm1, %xmm0
	movups	(ctx), %xmm1
	aesdeclast	%xmm1, %xmm0
	xorl	%eax, %eax
	movups	%xmm0, (out)
#ifdef KERNEL
	movaps	(r13), %xmm0	
	movaps	16(r13), %xmm1	
	add		$LOCAL_SIZE, r13
#endif
	ret

#endif /* x86 based build */
