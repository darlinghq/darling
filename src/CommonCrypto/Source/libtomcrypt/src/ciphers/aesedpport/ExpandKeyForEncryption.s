/*	This file defines _aes_encrypt_key, _aes_encrypt_key128,
	_aes_encrypt_key192, and _aes_encrypt_key256.  It is designed to be
	included in another assembly file with the preprocessor #include directive,
	to benefit from some assembly-time calculations.

	Written by Eric Postpischil, January 2008.

	The comments here do not say much about the algorithm; the code just
	follows the FIPS-197 specification.  I recommend reading the specification
	before working with this code or examining the C code in the parent
	directory that illustrates key expansion.
*/


/*	Routines:

		_aes_encrypt_key.

		_aes_encrypt_key128, _aes_encrypt_key192, and _aes_encrypt_key256.

	Function:

		Expand the user's cipher key into the key schedule, as defined in
		Federal Information Processing Standards Publication 197 (FIPS-197),
		November 26, 2001.

	Input:

		Constant data:

			The following names must be locally defined so the assembler
			can calculate certain offsets.

			static const Word _AESSubBytesWordTable[4][256].

				_AESSubBytesWordTable[i][j] = SubBytes(j) << 8*i, where
				SubBytes is defined in FIPS-197.  _AESSubBytesWordTable
				differs from _AESEncryptTable in that it does not include
				the MixColumn operation.  It is used in performing the last
				round, which differs fromm the previous rounds in that it
				does not include the MixColumn operation.

			static const Byte _AESRcon[].

				Round constants, beginning with AESRcon[1] for the first round
				(AESRcon[0] is padding.)
	
		Arguments:

			const unsigned char *Key

				Address of user's cipher key.

			int Length

				Number of bytes (16, 24, or 32) or bits (128, 192, or 256) in
				user's cipher key.

				This argument is used with _aes_encrypt_key.  It is not
				present for the other routines.  In those routines, Context
				is the second argument.

			aes_encrypt_ctx *Context

				Structure to contain the expanded key beginning at offset
				ContextKey and a four-byte "key length" beginning at offset
				ContextKeyLength.  The "key length" is the number of bytes from
				the start of the first round key to the start of the last round
				key.  That is 16 less than the number of bytes in the entire
				key.

	Output:

		The expanded key and the "key length" are written to *Context.

	Return:

		aes_rval	// -1 if "key length" is invalid.  0 otherwise.
*/

/* add AES HW detection and program branch if AES HW is detected cclee 3-12-10 */

#include <System/i386/cpu_capabilities.h>

	.text
	.globl _aes_encrypt_key
//	.private_extern	_aes_encrypt_key
_aes_encrypt_key:

    // detect AES HW, cclee-3-13-10
#if defined __x86_64__
    movq    __cpu_capabilities@GOTPCREL(%rip), %rax				// %rax -> __cpu_capabilities
    mov     (%rax), %eax										// %eax  = __cpu_capabilities
#else
#ifdef	KERNEL
    leal    __cpu_capabilities, %eax							// %eax -> __cpu_capabilities
    mov     (%eax), %eax										// %eax  = __cpu_capabilities
#else
	mov    _COMM_PAGE_CPU_CAPABILITIES, %eax
#endif
#endif
    test    $(kHasAES), %eax									// __cpu_capabilities & kHasAES
    jne     _aes_encrypt_key_hw									// if AES HW detected, branch to _aes_encrypt_key_hw

#define	dr		r0d				// Dissection register.
#define	drl		r0l				// Low 8 bits of dissection register.
#define	drh		r0h				// Second-lowest 8 bits of dissection register.

#define	t0		r1
#define	t0d		r1d				// Low 32 bits of t0.

#define	offset	Arch(r5, r11)	// Address offset and loop sentinel.

#define	R		r7				// Address of round constant.
#define	K		r7				// User key pointer.
	// R and K overlap.

#define	E		r6				// Expanded key pointer.

#define	ve0		%xmm0
#define	ve1		%xmm1
#define	ve2		%xmm2
#define	ve3		%xmm3
#define	vt3		%xmm4
#define	vt2		%xmm5
#define	vt1		%xmm6
#define	vt0		%xmm7

#define	LookupS(table, index)	(table)*TableSize(STable, index, 4)

	/*	Save registers and set SaveSize to the number of bytes pushed onto the
		stack so far, including the caller's return address.
	*/
	push	r3
	#if defined __i386__
		push	r5
		push	r6
		push	r7
		#define	SaveSize	(5*4)
	#else
		#define	SaveSize	(2*8)
	#endif

	/*	Number of bytes used for local variables:

			8 16-byte spaces to save XMM registers.
	*/
	#define	LocalsSize	(8*16)

	#if 0 < LocalsSize
		// Padding to position stack pointer at a multiple of 16 bytes.
		#define	Padding	(15 & -(SaveSize + LocalsSize))
		sub		$Padding + LocalsSize, r4	// Allocate space on stack.
	#else
		#define	Padding	0
	#endif

	/*	StackFrame is the number of bytes in our stack frame, from caller's
		stack pointer to ours (so it includes the return address).
	*/
	#define	StackFrame	(SaveSize + Padding + LocalsSize)

	// Save xmm registers.
	movaps	%xmm0, 0*16(r4)
	movaps	%xmm1, 1*16(r4)
	movaps	%xmm2, 2*16(r4)
	movaps	%xmm3, 3*16(r4)
	movaps	%xmm4, 4*16(r4)
	movaps	%xmm5, 5*16(r4)
	movaps	%xmm6, 6*16(r4)
	movaps	%xmm7, 7*16(r4)

#if defined __i386__

	// Define location of argument i.
	#define	Argument(i)	StackFrame+4*(i)(r4)

	#define	Nk		t0d

	// Load arguments.
	mov		Argument(2), E
	mov		Argument(1), Nk
	mov		Argument(0), K

#elif defined __x86_64__

	#define	Nk		r9d			// Number of words in key.
	mov		r6d, Nk				// Move Nk argument out of way.
	mov		r2, E				// Move E argument to common register.

#endif

	// Dispatch on key length.
	cmp		$128, Nk
	jge		2f
	shl		$3, Nk				// Convert from bytes to bits.
	cmp		$128, Nk
2:
	je		EKeyHas4Words
	cmp		$192, Nk
	je		EKeyHas6Words
	cmp		$256, Nk
	je		EKeyHas8Words
	mov		$-1, r0				// Return error.
	jmp		9f

// Stop using Nk.
#undef	Nk

	.globl _aes_encrypt_key128
//	.private_extern	_aes_encrypt_key128
_aes_encrypt_key128:

	/*	Save registers and set SaveSize to the number of bytes pushed onto the
		stack so far, including the caller's return address.
	*/
	push	r3
	#if defined __i386__
		push	r5
		push	r6
		push	r7
		#define	SaveSize	(5*4)
	#else
		#define	SaveSize	(2*8)
	#endif

	/*	Number of bytes used for local variables:

			8 16-byte spaces to save XMM registers.
	*/
	#define	LocalsSize	(8*16)

	#if 0 < LocalsSize
		// Padding to position stack pointer at a multiple of 16 bytes.
		#define	Padding	(15 & -(SaveSize + LocalsSize))
		sub		$Padding + LocalsSize, r4	// Allocate space on stack.
	#else
		#define	Padding	0
	#endif

	/*	StackFrame is the number of bytes in our stack frame, from caller's
		stack pointer to ours (so it includes the return address).
	*/
	#define	StackFrame	(SaveSize + Padding + LocalsSize)

	// Save xmm registers.
	movaps	%xmm0, 0*16(r4)
	movaps	%xmm1, 1*16(r4)
	movaps	%xmm2, 2*16(r4)
	movaps	%xmm3, 3*16(r4)
	movaps	%xmm4, 4*16(r4)
	movaps	%xmm5, 5*16(r4)
	movaps	%xmm6, 6*16(r4)
	movaps	%xmm7, 7*16(r4)

	#if defined __i386__

		// Load arguments.
		#define	Argument(i)	StackFrame+4*(i)(r4)
		mov		Argument(1), E
		mov		Argument(0), K

	#endif

// Merge point for _aes_encrypt_key and _aes_encrypt_key128.
EKeyHas4Words:

#define	e0	r2d
#define	e1	r3d
#define	e2	Arch(r5d, r11d)
#define	e3	r7d

	// First words of expanded key are copied from user key.
	mov		0*4(K), e0
	mov		1*4(K), e1
	mov		2*4(K), e2
	mov		3*4(K), e3

	movl	$10*16, ContextKeyLength(E)	// Set "key length."

	#if 0 != ContextKey
		add		$ContextKey, E
	#endif

	// K cannot be used after we write to R, since they use the same register.

	// Cache round constants in output buffer.  The last is a sentinel.
	movb	$0x01,  1*16(E)
	movb	$0x02,  2*16(E)
	movb	$0x04,  3*16(E)
	movb	$0x08,  4*16(E)
	movb	$0x10,  5*16(E)
	movb	$0x20,  6*16(E)
	movb	$0x40,  7*16(E)
	movb	$0x80,  8*16(E)
	movb	$0x1b,  9*16(E)
	movb	$0x36, 10*16(E)

	// Store initial words of expanded key, which are copies of user's key.
	mov		e0, 0*4(E)
	mov		e1, 1*4(E)
	mov		e2, 2*4(E)
	mov		e3, 3*4(E)

	#if defined __x86_64__

		#define	STable	r8
		lea		_AESSubBytesWordTable(%rip), STable

	#else

		#define	STable	r6
		sub		$16, r4			// allocate stack memory for storing E and STable
		mov		E, (r4)			// save E

            call    0f          // Push program counter onto stack.
        0:
            pop     %eax      	// Get program counter.

        lea     _AESSubBytesWordTable-0b(%eax), STable
		mov		STable, 4(r4)	// Save STable

	#endif

1:
	mov		e3, dr				// Put previous word into dissection register.

	// Perform SubWord(RotWord(dr)).
	movzx	drl, t0
	xor		LookupS(3, t0), e0		// Look up byte 0 in table 3.
	movzx	drh, t0d
	xor		LookupS(0, t0), e0		// Look up byte 1 in table 0.
	shr		$16, dr
	movzx	drl, t0d
	xor		LookupS(1, t0), e0		// Look up byte 2 in table 1.
	movzx	drh, t0d
	xor		LookupS(2, t0), e0		// Look up byte 3 in table 2.

#if defined	__i386__
	mov		(r4), E
#endif

	add		$4*4, E

	movzx	(E), t0d				// Get cached round constant.
	xor		t0d, e0					// XOR with word from four words back.

	// Chain to successive words.
	mov		e0, 0*4(E)
	xor		e0, e1
	mov		e1, 1*4(E)
	xor		e1, e2
	mov		e2, 2*4(E)
	xor		e2, e3
	mov		e3, 3*4(E)

#if defined	__i386__
	mov		E, (r4)
	mov		4(r4), STable 
#endif

	cmp		$0x36, t0d				// Was this the last round constant?

	jne		1b

#if defined	__i386__
	add		$16, r4
#endif

	xor		r0, r0		// Return success.

9:
	// Pop stack and restore registers.
	movaps	7*16(r4), %xmm7
	movaps	6*16(r4), %xmm6
	movaps	5*16(r4), %xmm5
	movaps	4*16(r4), %xmm4
	movaps	3*16(r4), %xmm3
	movaps	2*16(r4), %xmm2
	movaps	1*16(r4), %xmm1
	movaps	0*16(r4), %xmm0
	#if 0 < LocalsSize
		add		$Padding + LocalsSize, r4
	#endif
	#if defined __i386__
		pop		r7
		pop		r6
		pop		r5
	#endif
	pop		r3

	ret


// Reset definitions for next case.
#undef	e0
#undef	e1
#undef	e2
#undef	e3
#undef	STable

#undef	vt3
#undef	vt2
#define	ve4	%xmm4
#define	ve5	%xmm5


	.globl _aes_encrypt_key192
//	.private_extern	_aes_encrypt_key192
_aes_encrypt_key192:

	/*	Save registers and set SaveSize to the number of bytes pushed onto the
		stack so far, including the caller's return address.
	*/
	push	r3
	#if defined __i386__
		push	r5
		push	r6
		push	r7
		#define	SaveSize	(5*4)
	#else
		#define	SaveSize	(2*8)
	#endif

	/*	Number of bytes used for local variables:

			8 16-byte spaces to save XMM registers.
	*/
	#define	LocalsSize	(8*16)

	#if 0 < LocalsSize
		// Padding to position stack pointer at a multiple of 16 bytes.
		#define	Padding	(15 & -(SaveSize + LocalsSize))
		sub		$Padding + LocalsSize, r4	// Allocate space on stack.
	#else
		#define	Padding	0
	#endif

	/*	StackFrame is the number of bytes in our stack frame, from caller's
		stack pointer to ours (so it includes the return address).
	*/
	#define	StackFrame	(SaveSize + Padding + LocalsSize)

	// Save xmm registers.
	movaps	%xmm0, 0*16(r4)
	movaps	%xmm1, 1*16(r4)
	movaps	%xmm2, 2*16(r4)
	movaps	%xmm3, 3*16(r4)
	movaps	%xmm4, 4*16(r4)
	movaps	%xmm5, 5*16(r4)
	movaps	%xmm6, 6*16(r4)
	movaps	%xmm7, 7*16(r4)

	#if defined __i386__

		// Load arguments.
		#define	Argument(i)	StackFrame+4*(i)(r4)
		mov		Argument(1), E
		mov		Argument(0), K

	#endif

// Merge point for _aes_encrypt_key and _aes_encrypt_key192.
EKeyHas6Words:

	// First words of expanded key are copied from user key.
	movd	0*4(K), ve0
	movd	1*4(K), ve1
	movd	2*4(K), ve2
	movd	3*4(K), ve3

	movl	$12*16, ContextKeyLength(E)	// Set "key length."

	#if 0 != ContextKey
		add		$ContextKey, E
	#endif

	movd	4*4(K), ve4
	movd	5*4(K), ve5

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__

	// Get address of 0 in R.
            call    0f          // Push program counter onto stack.
        0:
            pop     %eax      // Get program counter.
 
		#define	STable	r3
        lea     _AESRcon-0b(%eax), R
        lea     _AESSubBytesWordTable-0b(%eax), STable

	#elif defined __x86_64__

		#define	STable	r8
		lea		_AESRcon(%rip), R
		lea		_AESSubBytesWordTable(%rip), STable

	#endif

	/*	With a six-word key, there are twelve rounds (thirteen 16-byte key
		blocks).
	*/
	mov		$-12*4*4, offset
	sub		offset, E

	// Store initial words of expanded key, which are copies of user's key.
	movd	ve0, 0*4(E, offset)
	movd	ve1, 1*4(E, offset)
	movd	ve2, 2*4(E, offset)
	movd	ve3, 3*4(E, offset)
	movd	ve4, 4*4(E, offset)
	movd	ve5, 5*4(E, offset)

/*	Jump into loop body.  The key expansion processes six four-byte words per
	iteration.  52 are needed in the key.  So only four are needed in the last
	iteration.
*/
	jmp		2f		
1:
	// Continue chaining to successive words.
	pxor	ve3, ve4
	movd	ve4, 4*4(E, offset)
	pxor	ve4, ve5
	movd	ve5, 5*4(E, offset)
2:
	add		$1, R				// Advance pointer.
	movd	ve5, dr				// Put previous word into dissection register.
	movzx	(R), t0				// Get round constant.
	movd	t0d, vt1
	pxor	vt1, ve0			// XOR with word from six words back.

	// Perform SubWord(RotWord(dr)).
	movzx	drl, t0d
	movd	LookupS(3, t0), vt0		// Look up byte 0 in table 3.
	movzx	drh, t0d
	movd	LookupS(0, t0), vt1		// Look up byte 1 in table 0.
	shr		$16, dr
	movzx	drl, t0d
	pxor	vt1, vt0
	pxor	vt0, ve0
	movd	LookupS(1, t0), vt0		// Look up byte 2 in table 1.
	movzx	drh, t0d
	movd	LookupS(2, t0), vt1		// Look up byte 3 in table 2.
	pxor	vt1, vt0
	pxor	vt0, ve0

	add		$6*4, offset

	// Chain to successive words.
	movd	ve0, 0*4(E, offset)
	pxor	ve0, ve1
	movd	ve1, 1*4(E, offset)
	pxor	ve1, ve2
	movd	ve2, 2*4(E, offset)
	pxor	ve2, ve3
	movd	ve3, 3*4(E, offset)

	jne		1b

	xor		r0, r0		// Return success.

	// Pop stack and restore registers.
	movaps	7*16(r4), %xmm7
	movaps	6*16(r4), %xmm6
	movaps	5*16(r4), %xmm5
	movaps	4*16(r4), %xmm4
	movaps	3*16(r4), %xmm3
	movaps	2*16(r4), %xmm2
	movaps	1*16(r4), %xmm1
	movaps	0*16(r4), %xmm0
	#if 0 < LocalsSize
		add		$Padding + LocalsSize, r4
	#endif
	#if defined __i386__
		pop		r7
		pop		r6
		pop		r5
	#endif
	pop		r3

	ret


// Reset definitions for next case.
#undef	ve4
#undef	ve5
#define	vt3	%xmm4
#define	vt2	%xmm5

#undef	STable

	.globl _aes_encrypt_key256
//	.private_extern	_aes_encrypt_key256
_aes_encrypt_key256:

	/*	Save registers and set SaveSize to the number of bytes pushed onto the
		stack so far, including the caller's return address.
	*/
	push	r3
	#if defined __i386__
		push	r5
		push	r6
		push	r7
		#define	SaveSize	(5*4)
	#else
		#define	SaveSize	(2*8)
	#endif

	/*	Number of bytes used for local variables:

			8 16-byte spaces to save XMM registers.
	*/
	#define	LocalsSize	(8*16)

	#if 0 < LocalsSize
		// Padding to position stack pointer at a multiple of 16 bytes.
		#define	Padding	(15 & -(SaveSize + LocalsSize))
		sub		$Padding + LocalsSize, r4	// Allocate space on stack.
	#else
		#define	Padding	0
	#endif

	/*	StackFrame is the number of bytes in our stack frame, from caller's
		stack pointer to ours (so it includes the return address).
	*/
	#define	StackFrame	(SaveSize + Padding + LocalsSize)

	// Save xmm registers.
	movaps	%xmm0, 0*16(r4)
	movaps	%xmm1, 1*16(r4)
	movaps	%xmm2, 2*16(r4)
	movaps	%xmm3, 3*16(r4)
	movaps	%xmm4, 4*16(r4)
	movaps	%xmm5, 5*16(r4)
	movaps	%xmm6, 6*16(r4)
	movaps	%xmm7, 7*16(r4)

	#if defined __i386__

		// Load arguments.
		#define	Argument(i)	StackFrame+4*(i)(r4)
		mov		Argument(1), E
		mov		Argument(0), K

	#endif

// Merge point for _aes_encrypt_key and _aes_encrypt_key256.
EKeyHas8Words:

	// First words of expanded key are copied from user key.
	movd	0*4(K), ve0
	movd	1*4(K), ve1
	movd	2*4(K), ve2
	movd	3*4(K), ve3

	movl	$14*16, ContextKeyLength(E)	// Set "key length."

	#if 0 != ContextKey
		add		$ContextKey, E
	#endif

	// Store initial words of expanded key, which are copies of user's key.
	movd	ve0, 0*4(E)
	movd	ve1, 1*4(E)
	movd	ve2, 2*4(E)
	movd	ve3, 3*4(E)
	movd	4*4(K), ve0
	movd	5*4(K), ve1
	movd	6*4(K), ve2
	movd	7*4(K), ve3

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__

		// Get address of 0 in R.
            call    0f          // Push program counter onto stack.
        0:
            pop     %eax      // Get program counter.
 
		#define	STable	r3
        lea     _AESRcon-0b(%eax), R
        lea     _AESSubBytesWordTable-0b(%eax), STable

	#elif defined __x86_64__

		#define	STable	r8
		lea		_AESRcon(%rip), R
		lea		_AESSubBytesWordTable(%rip), STable

	#endif

	/*	With an eight-word key, there are fourteen rounds (fifteen 16-byte key
	 	blocks).
	*/
	mov		$-14*4*4, offset
	sub		offset, E

	// Store initial words of expanded key, which are copies of user's key.
	movd	ve0, 4*4(E, offset)
	movd	ve1, 5*4(E, offset)
	movd	ve2, 6*4(E, offset)
	movd	ve3, 7*4(E, offset)

/*	Jump into loop body.  The key expansion processes eight four-byte words per
	iteration.  60 are needed in the key.  So only four are needed in the last
	iteration.
*/
	jmp		2f		
1:
	movd	ve3, dr				// Put previous word into dissection register.

	/*	Get word from eight words back (it is four words back from where E
	 	currently points, and we use it to prepare the value to be stored
		four words beyond where E currently points).
	*/
	movd	-4*4(E, offset), ve0

	// Perform SubWord(dr).
	movzx	drl, t0
	movd	LookupS(0, t0), vt0		// Look up byte 0 in table 0.
	movzx	drh, t0d
	movd	LookupS(1, t0), vt1		// Look up byte 1 in table 1.
	shr		$16, dr
	movzx	drl, t0d
	movd	LookupS(2, t0), vt2		// Look up byte 2 in table 2.
	movzx	drh, t0d
	movd	LookupS(3, t0), vt3		// Look up byte 3 in table 3.
	pxor	vt1, vt0
	pxor	vt3, vt2
	pxor	vt0, ve0
	pxor	vt2, ve0

	movd	-3*4(E, offset), ve1	// Get words from eight words back.
	movd	-2*4(E, offset), ve2
	movd	-1*4(E, offset), ve3

	// Chain to successive words.
	movd	ve0, 4*4(E, offset)
	pxor	ve0, ve1
	movd	ve1, 5*4(E, offset)
	pxor	ve1, ve2
	movd	ve2, 6*4(E, offset)
	pxor	ve2, ve3
	movd	ve3, 7*4(E, offset)

2:
	add		$1, R				// Advance pointer.
	movd	ve3, dr				// Put previous word into dissection register.
	movzx	(R), t0d			// Get round constant.
	movd	t0d, vt1
	movd	0*4(E, offset), ve0	// Get word from eight words back.
	pxor	vt1, ve0

	// Perform SubWord(RotWord(dr)).
	movzx	drl, t0
	movd	LookupS(3, t0), vt0		// Look up byte 0 in table 3.
	movzx	drh, t0d
	movd	LookupS(0, t0), vt1		// Look up byte 1 in table 0.
	shr		$16, dr
	movzx	drl, t0d
	movd	LookupS(1, t0), vt2		// Look up byte 2 in table 1.
	movzx	drh, t0d
	movd	LookupS(2, t0), vt3		// Look up byte 3 in table 2.
	pxor	vt1, vt0
	pxor	vt3, vt2
	pxor	vt0, ve0
	pxor	vt2, ve0

	movd	1*4(E, offset), ve1
	movd	2*4(E, offset), ve2
	movd	3*4(E, offset), ve3

	add		$8*4, offset

	// Chain to successive words.
	movd	ve0, 0*4(E, offset)
	pxor	ve0, ve1
	movd	ve1, 1*4(E, offset)
	pxor	ve1, ve2
	movd	ve2, 2*4(E, offset)
	pxor	ve2, ve3
	movd	ve3, 3*4(E, offset)

	jne		1b

	xor		r0, r0		// Return success.

	// Pop stack and restore registers.
	movaps	7*16(r4), %xmm7
	movaps	6*16(r4), %xmm6
	movaps	5*16(r4), %xmm5
	movaps	4*16(r4), %xmm4
	movaps	3*16(r4), %xmm3
	movaps	2*16(r4), %xmm2
	movaps	1*16(r4), %xmm1
	movaps	0*16(r4), %xmm0
	#if 0 < LocalsSize
		add		$Padding + LocalsSize, r4
	#endif
	#if defined __i386__
		pop		r7
		pop		r6
		pop		r5
	#endif
	pop		r3

	ret


#undef	Address
#undef	Argument
#undef	E
#undef	K
#undef	LocalsSize
#undef	LookupS
#undef	Padding
#undef	R
#undef	SaveSize
#undef	STable
#undef	StackFrame
#undef	dr
#undef	drh
#undef	drl
#undef	offset
#undef	t0
#undef	t0d
#undef	ve0
#undef	ve1
#undef	ve2
#undef	ve3
#undef	vt0
#undef	vt1
#undef	vt2
#undef	vt3
