/*	This file defines _AESExpandKeyForDecryption.  It is designed to be
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

		_AESExpandKeyForDecryption.

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

			static const Word _OLDAESSubBytesWordTable[4][256].

				_OLDAESSubBytesWordTable[i][j] = SubBytes(j) << 8*i, where
				SubBytes is defined in FIPS-197.  _OLDAESSubBytesWordTable
				differs from _OLDAESEncryptTable in that it does not include
				the MixColumn operation.  It is used in performing the last
				round, which differs fromm the previous rounds in that it
				does not include the MixColumn operation.

			static const Word _AESSInvMixColumnTable[4][256].

				_OLDAESInvMixColumnTable[i][j] contains the contribution of byte
				j to element i of the InvMixColumn operation.

				The four bytes of the word _OLDAESInvMixColumnTable[0][j] are:

					{0xe}*{j}, {0x9}*{j}, {0xd}*{j}, {0xb}*{j},

				listed in increasing address order, where multiplication is
				performed in the Galois field.  {j} designates the element of
				the Galois field represented by j.  _AESInvMixColumn[i][j] has
				the same bytes, rotated right in the order shown above.

			static const Byte _OLDAESRcon[].

				Round constants, beginning with OLDAESRcon[1] for the first round
				(OLDAESRcon[0] is padding.)
	
		Arguments:

			Word *ExpandedKey

				Address of output.

			const AESKey *Key

				Address of user's cipher key.

			long Nk

				Number of four-byte words in user's cipher key.

	Output:

		The expanded key is written to *ExpandedKey.
*/

#define	dr		r0d				// Dissection register.
#define	drl		r0l				// Low 8 bits of dissection register.
#define	drh		r0h				// Second-lowest 8 bits of dissection register.

#define	t0		r1
#define	t0d		r1d				// Low 32 bits of t0.

#define	STable	r2				// Address of SubBytes table.  Overlaps Nk.
#define	ITable	r3				// Address of InvMixColumn table.
#define	offset	r5				// Address offset and loop sentinel.

#define	R		r6				// Address of round constant.
#define	K		r6				// User key pointer, second x86_64 argument.
	// R and K overlap.

#define	E		r7				// Expanded key pointer, first x86_64 argument.

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


	.globl _AESExpandKeyForDecryption
	.private_extern	_AESExpandKeyForDecryption
_AESExpandKeyForDecryption:

	// Push new stack frame.
	push	r5

	// Save registers.
	push	r3
	#if defined __i386__
		push	r6
		push	r7
		#define	RegisterSaveSize	(3*4)
	#elif defined __x86_64__
		#define	RegisterSaveSize	(1*8)
		// Add pushes of r12 to r15 if used.
	#endif

#define	LocalsSize	0
#define	StackFrame	(LocalsSize+RegisterSaveSize)
	// Locals plus the registers we pushed after the new stack frame.

/*	Define stack offset to storage space for local data.  This is in the red
	zone.  We point far enough down to allow space for eight four-byte words
	plus a return address (4 or 8 bytes on i386 or x86_64) for our internal
	subroutine calls.
*/
#define	Local	(-8*4-8)

#if defined __i386__

	// Define location of argument i.
	#define	Argument(i)	StackFrame+8+4*(i)(r4)

	// Load arguments.
	mov		Argument(0), E
	mov		Argument(1), K
	#define	Nk	Argument(2)

#elif defined __x86_64__

	#define	Nk		r2			// Number of words in key.  Overlaps STable.

#endif

	cmp		$6,	 Nk

	#if 0 < LocalsSize
		sub		$LocalsSize, r4	// Allocate space on stack.
	#endif

	// First words of expanded key are copied from user key.
	movd	0*4(K), ve0
	movd	1*4(K), ve1
	movd	2*4(K), ve2
	movd	3*4(K), ve3
	je		DKeyHas6Words
	jg		DKeyHas8Words
	// Fall through to DKeyHas4Words.

DKeyHas4Words:

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__
		// Get address of 0 in R.
			call	0f			// Push program counter onto stack.
		0:
			pop		STable		// Get program counter.

		lea		_OLDAESRcon-0b(STable), R
		lea		_OLDAESInvMixColumnTable-0b(STable), ITable
		lea		_OLDAESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_OLDAESRcon(%rip), R
		lea		_OLDAESInvMixColumnTable(%rip), ITable
		lea		_OLDAESSubBytesWordTable(%rip), STable

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

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		// Add pops of r15 to r12 if used.
		pop		r7
		pop		r6
	#elif defined __x86_64__
	#endif
	pop		r3
	pop		r5

	ret


DKeyHas6Words:
	movd	4*4(K), ve4
	movd	5*4(K), ve5

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__
		// Get address of 0 in R.
			call	0f			// Push program counter onto stack.
		0:
			pop		STable		// Get program counter.

		lea		_OLDAESRcon-0b(STable), R
		lea		_OLDAESInvMixColumnTable-0b(STable), ITable
		lea		_OLDAESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_OLDAESRcon(%rip), R
		lea		_OLDAESInvMixColumnTable(%rip), ITable
		lea		_OLDAESSubBytesWordTable(%rip), STable

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

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		// Add pops of r15 to r12 if used.
		pop		r7
		pop		r6
	#elif defined __x86_64__
	#endif
	pop		r3
	pop		r5

	ret


DKeyHas8Words:

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
			call	0f			// Push program counter onto stack.
		0:
			pop		STable		// Get program counter.

		lea		_OLDAESRcon-0b(STable), R
		lea		_OLDAESInvMixColumnTable-0b(STable), ITable
		lea		_OLDAESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_OLDAESRcon(%rip), R
		lea		_OLDAESInvMixColumnTable(%rip), ITable
		lea		_OLDAESSubBytesWordTable(%rip), STable

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

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		// Add pops of r15 to r12 if used.
		pop		r7
		pop		r6
	#elif defined __x86_64__
	#endif
	pop		r3
	pop		r5

	ret


#undef	Address
#undef	Argument
#undef	E
#undef	ITable
#undef	K
#undef	LocalsSize
#undef	LookupI
#undef	LookupS
#undef	Nk
#undef	R
#undef	RegisterSaveSize
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
