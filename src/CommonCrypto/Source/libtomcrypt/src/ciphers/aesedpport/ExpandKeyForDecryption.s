/*	This file defines _aes_decrypt_key, _aes_decrypt_key128,
	_aes_decrypt_key192, and _aes_decrypt_key256.  It is designed to be
	included in another assembly file with the preprocessor #include directive,
	to benefit from some assembly-time calculations.

	Written by Eric Postpischil, January 2008.

	The comments here do not say much about the algorithm; the code just
	follows the FIPS-197 specification.  I recommend reading the specification
	before working with this code or examining the C code in the parent
	directory that illustrates key expansion.

	One complication is that this routine both expands the key and applies
	InvMixColumn to most of the words in the expanded key.  This modifies the
	key for use with the Equivalent Inverse Cipher.

	During key expansion, there are sequences of four or six words that are
	produced like this:

		E[i+0] = E[i+0-Nk] ^ f(E[i-1]), where f is some function.
		E[i+1] = E[i+1-Nk] ^ E[i+0].
		E[i+2] = E[i+2-Nk] ^ E[i+1].
		E[i+3] = E[i+3-Nk] ^ E[i+2].

	When Nk is four or eight, the sequence stops there.  When it is six, it
	goes on for two more words.  Let I be the InvMixColumn function.  for the
	Equivalent Inverse Cipher, we want to store I(E[i+0]), I(E[i+1]),
	I(E[i+2]), I(E[i+3]) (and two more when Nk is six).  However, we do not
	need to calculate I four times.  In AES' finite field, I is a linear
	combination of the four bytes of its input.  The ^ operation on the bits
	that represent field elements is an addition in the Galois field.  So
	I(a ^ b) = I(a) ^ I(b).  Then we have:

		I(E[i+0]) = I(E[i+0-Nk] ^ f(E[i-1])) = I(E[i+0-Nk]) ^ I(f(E[i-1])).
		I(E[i+1]) = I(E[i+1-Nk]) ^ I(E[i+0]).
		I(E[i+2]) = I(E[i+2-Nk]) ^ I(E[i+1]).
		I(E[i+3]) = I(E[i+3-Nk]) ^ I(E[i+2]).

	To compute this, we compute I(f(E[i-1])) and XOR it with the previously
	stored E[i+0-Nk])) to get I(E[i+0])).  Then we XOR that with the previously
	stored E[i+1-Nk])) to get I(E[i+1])), and so on.

	Note that to compute I(f(E[i-1])), we need to have E[i-1].  So we have to
	compute the pre-InvMixColumn words of the expanded key; it is not
	sufficient to have the post-InvMixColumn words.
*/


/*	Routine:

		_aes_decrypt_key.

		_aes_decrypt_key128, _aes_decrypt_key192, and _aes_decrypt_key256.

	Function:

		Expand the user's cipher key into the key schedule, as defined in
		Federal Information Processing Standards Publication 197 (FIPS-197),
		November 26, 2001.

		For decryption, the key is modified as shown in Figure 15 in FIPS-197,
		to support the Equivalent Inverse Cipher.

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

			static const Word _AESSInvMixColumnTable[4][256].

				_AESInvMixColumnTable[i][j] contains the contribution of byte
				j to element i of the InvMixColumn operation.

				The four bytes of the word _AESInvMixColumnTable[0][j] are:

					{0xe}*{j}, {0x9}*{j}, {0xd}*{j}, {0xb}*{j},

				listed in increasing address order, where multiplication is
				performed in the Galois field.  {j} designates the element of
				the Galois field represented by j.  _AESInvMixColumn[i][j] has
				the same bytes, rotated right in the order shown above.

			static const Byte _AESRcon[].

				Round constants, beginning with AESRcon[1] for the first round
				(AESRcon[0] is padding.)
	
		Arguments:

			const unsigned char *Key

				Address of user's cipher key.

			int Length

				Number of bytes (16, 24, or 32) or bits (128, 192, or 256) in
				user's cipher key.

				This argument is used with _aes_decrypt_key.  It is not
				present for the other routines.  In those routines, Context
				is the second argument.

			aes_decrypt_ctx *Context

				Structure to contain the expanded key beginning at offset
				ContextKey and a four-byte "key length" beginning at offset
				ContextKeyLength.  The "key length" is the number of bytes from
				the start of the first round key to the startof the last rond
				key.  That is 16 less than the number of bytes in the entire
				key.

	Output:

		The expanded key and the "key length" are written to *Context.

	Return:

		aes_rval	// -1 if "key length" is invalid.  0 otherwise.
*/
/* add AES HW detection and program branch if AES HW is detected cclee 3-12-10 */

#include <System/i386/cpu_capabilities.h>

#define	dr		r0d				// Dissection register.
#define	drl		r0l				// Low 8 bits of dissection register.
#define	drh		r0h				// Second-lowest 8 bits of dissection register.

#define	t0		r1
#define	t0d		r1d				// Low 32 bits of t0.

#define	STable	r2				// Address of SubBytes table.  Overlaps Nk.
#define	ITable	r3				// Address of InvMixColumn table.
#define	offset	Arch(r5, r11)	// Address offset and loop sentinel.

#define	R		r7				// Address of round constant.
#define	K		r7				// User key pointer.
	// R and K overlap.

#define	E		r6				// Expanded key pointer.

#define	ve0		%xmm0
#define	ve1		%xmm1
#define	ve2		%xmm2
#define	ve3		%xmm3
#define	ve4		%xmm4
#define	ve5		%xmm5
#define	vt1		%xmm6
#define	vt0		%xmm7

#define	LookupS(table, index)	(table)*TableSize(STable, index, 4)
#define	LookupI(table, index)	(table)*TableSize(ITable, index, 4)


/*	InvMixColumn puts InvMixColumn(dr) into vt0.  This is a non-standard
	subroutine.  It does not conform to the ABI.  It is an integral part of
	_ExpandKeyForDecryption and shares register use with it.
*/
InvMixColumn:
	movzx	drl, t0
	movd	LookupI(0, t0), vt0		// Look up byte 0 in table 0.
	movzx	drh, t0d
	movd	LookupI(1, t0), vt1		// Look up byte 1 in table 1.
	pxor	vt1, vt0
	shr		$16, dr
	movzx	drl, t0d
	movd	LookupI(2, t0), vt1		// Look up byte 2 in table 2.
	pxor	vt1, vt0
	movzx	drh, t0d
	movd	LookupI(3, t0), vt1		// Look up byte 3 in table 3.
	pxor	vt1, vt0
	ret


	// SubWordRotWord adds (XORs) SubWord(RotWord(dr)) to vt0.
	.macro	SubWordRotWord
		movzx	drl, t0
		movd	LookupS(3, t0), vt1		// Look up byte 0 in table 3.
		pxor	vt1, vt0
		movzx	drh, t0d
		movd	LookupS(0, t0), vt1		// Look up byte 1 in table 0.
		pxor	vt1, vt0
		shr		$$16, dr
		movzx	drl, t0d
		movd	LookupS(1, t0), vt1		// Look up byte 2 in table 1.
		pxor	vt1, vt0
		movzx	drh, t0d
		movd	LookupS(2, t0), vt1		// Look up byte 3 in table 2.
		pxor	vt1, vt0
	.endmacro


	// SubWord puts SubWord(dr) into vt0.
	.macro	SubWord
		movzx	drl, t0
		movd	LookupS(0, t0), vt0		// Look up byte 0 in table 0.
		movzx	drh, t0d
		movd	LookupS(1, t0), vt1		// Look up byte 1 in table 1.
		pxor	vt1,vt0
		shr		$$16, dr
		movzx	drl, t0d
		movd	LookupS(2, t0), vt1		// Look up byte 2 in table 2.
		pxor	vt1,vt0
		movzx	drh, t0d
		movd	LookupS(3, t0), vt1		// Look up byte 3 in table 3.
		pxor	vt1,vt0
	.endmacro

	.text
	.globl _aes_decrypt_key
//	.private_extern	_aes_decrypt_key
_aes_decrypt_key:

	// detect AES HW, cclee 3-13-10
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
    jne     _aes_decrypt_key_hw									// if AES HW detected, branch to _aes_decrypt_key_hw

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

			8 four-byte spaces for work.
	*/
	#define	LocalsSize	(8*16 + 8*4)

	// Define stack offset to storage space for local data.
	#define	Local	(8*16)

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
	je		DKeyHas4Words
	cmp		$192, Nk
	je		DKeyHas6Words
	cmp		$256, Nk
	je		DKeyHas8Words
	mov		$-1, r0				// Return error.
	jmp		9f


	.globl _aes_decrypt_key128
//	.private_extern	_aes_decrypt_key128
_aes_decrypt_key128:

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

			8 four-byte spaces for work.
	*/
	#define	LocalsSize	(8*16 + 8*4)

	// Define stack offset to storage space for local data.
	#define	Local	(8*16)

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

// Merge point for _aes_decrypt_key and _aes_decrypt_key128.
DKeyHas4Words:

	// First words of expanded key are copied from user key.
	movd	0*4(K), ve0
	movd	1*4(K), ve1
	movd	2*4(K), ve2
	movd	3*4(K), ve3

	movl	$10*16, ContextKeyLength(E)	// Set "key length."

	#if 0 != ContextKey
		add		$ContextKey, E
	#endif

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__

			call    0f          // Push program counter onto stack.
		0:	pop     STable      // Get program counter.
			lea     _AESRcon-0b(STable), R
			lea     _AESInvMixColumnTable-0b(STable), ITable
			lea     _AESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_AESRcon(%rip), R
		lea		_AESInvMixColumnTable(%rip), ITable
		lea		_AESSubBytesWordTable(%rip), STable

	#endif

	/*	With a four-word key, there are ten rounds (eleven 16-byte key blocks),
		nine of which have InvMixColumn applied.
	*/
	mov		$-9*4*4, offset
	sub		offset, E

	// Store initial words of expanded key, which are copies of user's key.
	movd	ve0, 0*4(E, offset)
	movd	ve1, 1*4(E, offset)
	movd	ve2, 2*4(E, offset)
	movd	ve3, 3*4(E, offset)

/*	Here is the first iteration of the key expansion.  It is separate from the
	main loop below because we need to apply InvMixColumn to each of the
	outputs, in ve0 through ve3.  In the main loop, the technique described at
	the top of this file is used to compute the proper outputs while using
	InvMixColumn only once.
*/
	add		$1, R					// Advance pointer.
	movd	ve3, dr					// Put previous word into work register.
	movzx	(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord
	pxor	vt0, ve0

	// Chain to successive words.
	pxor	ve0, ve1
	pxor	ve1, ve2
	pxor	ve2, ve3

	add		$4*4, offset

	/*	Apply InvMixColumn to each word.  The transformed values are stored in
		the expanded key.  The original values are retained in registers for
		further computation.
	*/
	movd	ve0, dr
	call	InvMixColumn
	movd	vt0, 0*4(E, offset)

	movd	ve1, dr
	call	InvMixColumn
	movd	vt0, 1*4(E, offset)

	movd	ve2, dr
	call	InvMixColumn
	movd	vt0, 2*4(E, offset)

	movd	ve3, dr
	call	InvMixColumn
	movd	vt0, 3*4(E, offset)

//	Here is the main loop.
1:
	add		$1, R					// Advance pointer.
	movd	ve3, dr					// Put previous word into work register.
	movzx	(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord
	pxor	vt0, ve0

	// Chain to successive words.
	pxor	ve0, ve1
	pxor	ve1, ve2
	pxor	ve2, ve3
		/*	Dr. Brian Gladman uses a technique with a single XOR here instead
			of the previous four.  There is some periodic behavior in the key
			expansion, and Gladman maintains E[4*i+3] for the latest four
			values of i.  XORing the value in vt0 with one of these yields its
			replacement.  However, using this technique requires additional
			instructions before the loop (to initialize the values) and after
			it (to extract the final values to be stored) and either some way
			to rotate or index four values in the loop or a four-fold unrolling
			of the loop to provide the indexing.  Experiment suggests the
			former is not worthwhile.  Unrolling the loop might give a small
			gain, at the cost of increased use of instruction cache, increased
			instructions loads the first time the routine is executed, and
			increased code complexity, so I decided against it.
		*/

	// Apply InvMixColumn to the difference.
	movd	vt0, dr
	call	InvMixColumn

	add		$4*4, offset

	// Chain the transformed difference to previously transformed outputs.
	movd	(0-4)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 0*4(E, offset)

	movd	(1-4)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 1*4(E, offset)

	movd	(2-4)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 2*4(E, offset)

	movd	(3-4)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 3*4(E, offset)

	jl		1b

// Here is the final iteration, which does not perform InvMixColumn.

	movd	ve3, dr					// Put previous word into work register.
	movzx	1(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord
	pxor	vt0, ve0

	// Chain to successive words.
	movd	ve0, 4*4(E, offset)
	pxor	ve0, ve1
	movd	ve1, 5*4(E, offset)
	pxor	ve1, ve2
	movd	ve2, 6*4(E, offset)
	pxor	ve2, ve3
	movd	ve3, 7*4(E, offset)

	xor		r0, r0				// Return success.

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


	.globl _aes_decrypt_key192
//	.private_extern	_aes_decrypt_key192
_aes_decrypt_key192:

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

			8 four-byte spaces for work.
	*/
	#define	LocalsSize	(8*16 + 8*4)

	// Define stack offset to storage space for local data.
	#define	Local	(8*16)

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

// Merge point for _aes_decrypt_key and _aes_decrypt_key192.
DKeyHas6Words:

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

			call    0f          // Push program counter onto stack.
		0:	pop     STable      // Get program counter.
			lea     _AESRcon-0b(STable), R
			lea     _AESInvMixColumnTable-0b(STable), ITable
			lea     _AESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_AESRcon(%rip), R
		lea		_AESInvMixColumnTable(%rip), ITable
		lea		_AESSubBytesWordTable(%rip), STable

	#endif

	/*	With a six-word key, there are twelve rounds (thirteen 16-byte key
		blocks), eleven of which have InvMixColumn applied.  The key expansion
		proceeds in iterations of six four-byte words, so the termination
		condition is a bit complicated.  We set offset to the negative of 10
		four four-byte words, and the loop branch does another iteration if
		offset is less than or equal to zero, meaning the number of iterations
		performed so far is less than or equal to 10.  Thus, after ten
		iterations, it branches again.  After the eleventh iteration, it
		stops.  Code after the end of the loop computes the twelfth key block,
		which does not have InvMixColumn applied.
	*/
	mov		$-10*4*4, offset
	sub		offset, E

	// Store initial words of expanded key, which are copies of user's key.
	movd	ve0, 0*4(E, offset)
	movd	ve1, 1*4(E, offset)
	movd	ve2, 2*4(E, offset)
	movd	ve3, 3*4(E, offset)

	/*	The first four words are stored untransformed.  After that, words in
		the expanded key are transformed by InvMixColumn.
	*/
	movd	ve4, dr
	call	InvMixColumn
	movd	vt0, 4*4(E, offset)

	movd	ve5, dr
	call	InvMixColumn
	movd	vt0, 5*4(E, offset)

/*	Here is the first iteration of the key expansion.  It is separate from the
	main loop below because we need to apply InvMixColumn to each of the
	outputs, in ve0 through ve5.  In the main loop, the technique described at
	the top of this file is used to compute the proper outputs while using
	InvMixColumn only once.
*/
	add		$1, R					// Advance pointer.
	movd	ve5, dr					// Put previous word into work register.
	movzx	(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord
	pxor	vt0, ve0

	// Chain to successive words.
	pxor	ve0, ve1
	pxor	ve1, ve2
	pxor	ve2, ve3
	pxor	ve3, ve4
	pxor	ve4, ve5

	add		$6*4, offset

	/*	Apply InvMixColumn to each word.  The transformed values are stored in
		the expanded key.  The original values are retained in registers for
		further computation.
	*/
	movd	ve0, dr
	call	InvMixColumn
	movd	vt0, 0*4(E, offset)

	movd	ve1, dr
	call	InvMixColumn
	movd	vt0, 1*4(E, offset)

	movd	ve2, dr
	call	InvMixColumn
	movd	vt0, 2*4(E, offset)

	movd	ve3, dr
	call	InvMixColumn
	movd	vt0, 3*4(E, offset)

	movd	(4-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 4*4(E, offset)

	movd	(5-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 5*4(E, offset)

//	Here is the main loop.
1:
	add		$1, R					// Advance pointer.
	movd	ve5, dr					// Put previous word into work register.
	movzx	(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord
	pxor	vt0, ve0

	// Chain to successive words.
	pxor	ve0, ve1
	pxor	ve1, ve2
	pxor	ve2, ve3
	pxor	ve3, ve4
	pxor	ve4, ve5

	// Apply InvMixColumn to the difference.
	movd	vt0, dr
	call	InvMixColumn

	add		$6*4, offset

	// Chain the transformed difference to previously transformed outputs.
	movd	(0-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 0*4(E, offset)

	movd	(1-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 1*4(E, offset)

	movd	(2-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 2*4(E, offset)

	movd	(3-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 3*4(E, offset)

	movd	(4-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 4*4(E, offset)

	movd	(5-6)*4(E, offset), vt1
	pxor	vt1, vt0
	movd	vt0, 5*4(E, offset)

	jle		1b

// Here is the final iteration, which does not perform InvMixColumn.

	movd	ve5, dr					// Put previous word into work register.
	movzx	1(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord
	pxor	vt0, ve0

	// Chain to successive words.
	movd	ve0, 6*4(E, offset)
	pxor	ve0, ve1
	movd	ve1, 7*4(E, offset)
	pxor	ve1, ve2
	movd	ve2, 8*4(E, offset)
	pxor	ve2, ve3
	movd	ve3, 9*4(E, offset)

	xor		r0, r0				// Return success.

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


	.globl _aes_decrypt_key256
//	.private_extern	_aes_decrypt_key256
_aes_decrypt_key256:

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

			8 four-byte spaces for work.
	*/
	#define	LocalsSize	(8*16 + 8*4)

	// Define stack offset to storage space for local data.
	#define	Local	(8*16)

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

// Merge point for _aes_decrypt_key and _aes_decrypt_key256.
DKeyHas8Words:

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

			call    0f          // Push program counter onto stack.
		0:	pop     STable      // Get program counter.
			lea     _AESRcon-0b(STable), R
			lea     _AESInvMixColumnTable-0b(STable), ITable
			lea     _AESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_AESRcon(%rip), R
		lea		_AESInvMixColumnTable(%rip), ITable
		lea		_AESSubBytesWordTable(%rip), STable

	#endif

	/*	With an eight-word key, there are fourteen rounds (fifteen 16-byte key
		blocks), thirteen of which have InvMixColumn applied.
	*/
	mov		$-12*4*4, offset
	sub		offset, E

	// Save untransformed values in stack area.
	movd	ve0, 4*4+Local(r4)
	movd	ve1, 5*4+Local(r4)
	movd	ve2, 6*4+Local(r4)
	movd	ve3, 7*4+Local(r4)

	/*	Apply InvMixColumn to words 4 through 7.  The transformed values are
		stored in the expanded key.  The original values are saved in the stack
		area for further computation.
	*/
	movd	ve0, dr
	call	InvMixColumn
	movd	vt0, 4*4(E, offset)

	movd	ve1, dr
	call	InvMixColumn
	movd	vt0, 5*4(E, offset)

	movd	ve2, dr
	call	InvMixColumn
	movd	vt0, 6*4(E, offset)

	movd	ve3, dr
	call	InvMixColumn
	movd	vt0, 7*4(E, offset)

/*	Here is the first iteration of the key expansion.  It is separate from the
	main loop below because we need to apply InvMixColumn to each of the
	outputs, in ve0 through ve3.  In the main loop, the technique described at
	the top of this file is used to compute the proper outputs while using
	InvMixColumn only once.
*/
	add		$1, R					// Advance pointer.
	movd	ve3, dr					// Put previous word into work register.
	movzx	(R), t0d				// Get round constant.
	movd	t0d, vt0

	SubWordRotWord

	add		$8*4, offset

	movd	(0-8)*4(E, offset), ve0		// Get old word.
	pxor	vt0, ve0
	movd	ve0, 0*4+Local(r4)			// Save on stack.
	movd	ve0, dr
	call	InvMixColumn
	movd	vt0, 0*4(E, offset)			// Write to expanded key.

	/*	 Chain to successive words and apply InvMixColumn to each word.  The
		 transformed values are stored in the expanded key.  The original
		 values are retained in local data for further computation.
	*/
	movd	(1-8)*4(E, offset), ve1		// Get old word.
	pxor	ve0, ve1					// Chain.
	movd	ve1, 1*4+Local(r4)			// Save on stack.
	movd	ve1, dr
	call	InvMixColumn
	movd	vt0, 1*4(E, offset)			// Write to expanded key.

	movd	(2-8)*4(E, offset), ve2		// Get old word.
	pxor	ve1, ve2					// Chain.
	movd	ve2, 2*4+Local(r4)			// Save on stack.
	movd	ve2, dr
	call	InvMixColumn
	movd	vt0, 2*4(E, offset)			// Write to expanded key.

	movd	(3-8)*4(E, offset), ve3		// Get old word.
	pxor	ve2, ve3					// Chain.
	movd	ve3, 3*4+Local(r4)			// Save on stack.
	movd	ve3, dr
	call	InvMixColumn
	movd	vt0, 3*4(E, offset)			// Write to expanded key.

	movd	ve3, dr						// Put previous word into work register.
	SubWord

	movd	4*4+Local(r4), ve0			// Get old word.
	pxor	vt0, ve0					// Chain.
	movd	ve0, 4*4+Local(r4)			// Save on stack.

	movd	5*4+Local(r4), ve1			// Get old word.
	pxor	ve0, ve1					// Chain.
	movd	ve1, 5*4+Local(r4)			// Save on stack.

	movd	6*4+Local(r4), ve2			// Get old word.
	pxor	ve1, ve2					// Chain.
	movd	ve2, 6*4+Local(r4)			// Save on stack.

	movd	7*4+Local(r4), ve3			// Get old word.
	pxor	ve2, ve3					// Chain.
	movd	ve3, 7*4+Local(r4)			// Save on stack.

	movd	vt0, dr						// Move change to work register.
	call	InvMixColumn

	movd	(4-8)*4(E, offset), vt1		// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, 4*4(E, offset)			// Write new word to expanded key.

	movd	(5-8)*4(E, offset), vt1		// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, 5*4(E, offset)			// Write new word to expanded key.

	movd	(6-8)*4(E, offset), vt1		// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, 6*4(E, offset)			// Write new word to expanded key.

	movd	(7-8)*4(E, offset), vt1		// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, 7*4(E, offset)			// Write new word to expanded key.

//	Here is the main loop.
1:
	add		$1, R						// Advance pointer.
	movd	ve3, dr						// Put previous word into work register.
	movzx	(R), t0d					// Get round constant.
	movd	t0d, vt0

	SubWordRotWord

	movd	0*4+Local(r4), ve0			// Get old word.
	pxor	vt0, ve0
	movd	ve0, 0*4+Local(r4)			// Save on stack.

	// Chain to successive words.
	movd	1*4+Local(r4), ve1			// Get old word.
	pxor	ve0, ve1					// Chain.
	movd	ve1, 1*4+Local(r4)			// Save on stack.

	movd	2*4+Local(r4), ve2			// Get old word.
	pxor	ve1, ve2					// Chain.
	movd	ve2, 2*4+Local(r4)			// Save on stack.

	movd	3*4+Local(r4), ve3			// Get old word.
	pxor	ve2, ve3					// Chain.
	movd	ve3, 3*4+Local(r4)			// Save on stack.

	movd	vt0, dr						// Move change to work register.
	call	InvMixColumn

	movd	0*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (0+8)*4(E, offset)		// Write new word to expanded key.

	movd	1*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (1+8)*4(E, offset)		// Write new word to expanded key.

	movd	2*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (2+8)*4(E, offset)		// Write new word to expanded key.

	movd	3*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (3+8)*4(E, offset)		// Write new word to expanded key.

	movd	ve3, dr						// Put previous word into work register.
	SubWord

	movd	4*4+Local(r4), ve0			// Get old word.
	pxor	vt0, ve0					// Chain.
	movd	ve0, 4*4+Local(r4)			// Save on stack.

	movd	5*4+Local(r4), ve1			// Get old word.
	pxor	ve0, ve1					// Chain.
	movd	ve1, 5*4+Local(r4)			// Save on stack.

	movd	6*4+Local(r4), ve2			// Get old word.
	pxor	ve1, ve2					// Chain.
	movd	ve2, 6*4+Local(r4)			// Save on stack.

	movd	7*4+Local(r4), ve3			// Get old word.
	pxor	ve2, ve3					// Chain.
	movd	ve3, 7*4+Local(r4)			// Save on stack.

	movd	vt0, dr						// Move change to work register.
	call	InvMixColumn

	movd	4*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (4+8)*4(E, offset)		// Write new word to expanded key.

	movd	5*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (5+8)*4(E, offset)		// Write new word to expanded key.

	movd	6*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (6+8)*4(E, offset)		// Write new word to expanded key.

	movd	7*4(E, offset), vt1			// Get old word.
	pxor	vt1, vt0					// Chain.
	movd	vt0, (7+8)*4(E, offset)		// Write new word to expanded key.

	add		$8*4, offset

	jl		1b

	movd	ve3, dr						// Put previous word into work register.
	movzx	1(R), t0d					// Get round constant.
	movd	t0d, vt0

	SubWordRotWord

	movd	0*4+Local(r4), ve0			// Get old word.
	pxor	vt0, ve0					// Chain.
	movd	ve0, (0+8)*4(E, offset)

	// Chain to successive words.
	movd	1*4+Local(r4), ve1			// Get old word.
	pxor	ve0, ve1					// Chain.
	movd	ve1, (1+8)*4(E, offset)

	movd	2*4+Local(r4), ve2			// Get old word.
	pxor	ve1, ve2					// Chain.
	movd	ve2, (2+8)*4(E, offset)

	movd	3*4+Local(r4), ve3			// Get old word.
	pxor	ve2, ve3					// Chain.
	movd	ve3, (3+8)*4(E, offset)

	xor		r0, r0				// Return success.

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
#undef	ITable
#undef	K
#undef	Local
#undef	LocalsSize
#undef	LookupI
#undef	LookupS
#undef	Nk
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
#undef	ve4
#undef	ve5
#undef	vt0
#undef	vt1
