/*	This file defines _AESExpandKeyForEncryption.  It is designed to be
	included in another assembly file with the preprocessor #include directive,
	to benefit from some assembly-time calculations.

	Written by Eric Postpischil, January 2008.

	The comments here do not say much about the algorithm; the code just
	follows the FIPS-197 specification.  I recommend reading the specification
	before working with this code or examining the C code in the parent
	directory that illustrates key expansion.
*/


/*	Routine:

		_AESExpandKeyForEncryption.

	Function:

		Expand the user's cipher key into the key schedule, as defined in
		Federal Information Processing Standards Publication 197 (FIPS-197),
		November 26, 2001.

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
	.globl _AESExpandKeyForEncryption
	.private_extern	_AESExpandKeyForEncryption
_AESExpandKeyForEncryption:

#define	dr		r0d				// Dissection register.
#define	drl		r0l				// Low 8 bits of dissection register.
#define	drh		r0h				// Second-lowest 8 bits of dissection register.

#define	t0		r1
#define	t0d		r1d				// Low 32 bits of t0.

#define	offset	r5				// Address offset and loop sentinel.

#define	R		r6				// Address of round constant.
#define	K		r6				// User key pointer, second x86_64 argument.
	// R and K overlap.

#define	E		r7				// Expanded key pointer, first x86_64 argument.

#define	ve0		%xmm0
#define	ve1		%xmm1
#define	ve2		%xmm2
#define	ve3		%xmm3
#define	vt3		%xmm4
#define	vt2		%xmm5
#define	vt1		%xmm6
#define	vt0		%xmm7

#define	LookupS(table, index)	(table)*TableSize(STable, index, 4)

	// Push new stack frame.
	push	r5

	// Save registers.
	#if defined __i386__
		push	r6
		push	r7
		#define	RegisterSaveSize	(2*4)
	#elif defined __x86_64__
		#define	RegisterSaveSize	(0*8)
		// Add pushes of r12 to r15 if used.
	#endif

#define	LocalsSize	0
#define	StackFrame	(LocalsSize+RegisterSaveSize)
	// Locals plus the registers we pushed after the new stack frame.

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
// Stop using r2 for Nk and start using it for STable.
#undef	Nk
#define	STable	r2

	#if 0 < LocalsSize
		sub		$LocalsSize, r4	// Allocate space on stack.
	#endif

	// First words of expanded key are copied from user key.
	movd	0*4(K), ve0
	movd	1*4(K), ve1
	movd	2*4(K), ve2
	movd	3*4(K), ve3
	je		EKeyHas6Words
	jg		EKeyHas8Words
	// Fall through to EKeyHas4Words.

EKeyHas4Words:

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__
		// Get address of 0 in R.
			call	0f			// Push program counter onto stack.
		0:
			pop		STable		// Get program counter.

		lea		_OLDAESRcon-0b(STable), R
		lea		_OLDAESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_OLDAESRcon(%rip), R
		lea		_OLDAESSubBytesWordTable(%rip), STable

	#endif

	// With a four-word key, there are ten rounds (eleven 16-byte key blocks).
	mov		$-10*4*4, offset
	sub		offset, E

	// Store initial words of expanded key, which are copies of user's key.
	movd	ve0, 0*4(E, offset)
	movd	ve1, 1*4(E, offset)
	movd	ve2, 2*4(E, offset)
	movd	ve3, 3*4(E, offset)

1:
	add		$1, R				// Advance pointer.
	movd	ve3, dr				// Put previous word into dissection register.
	movzx	(R), t0d			// Get round constant.
	movd	t0d, vt3
	pxor	vt3, ve0			// XOR with word from four words back.

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

	add		$4*4, offset

	// Chain to successive words.
	movd	ve0, 0*4(E, offset)
	pxor	ve0, ve1
	movd	ve1, 1*4(E, offset)
	pxor	ve1, ve2
	movd	ve2, 2*4(E, offset)
	pxor	ve2, ve3
	movd	ve3, 3*4(E, offset)

	jne		1b

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		// Add pops of r15 to r12 if used.
		pop		r7
		pop		r6
	#elif defined __x86_64
	#endif
	pop		r5

	ret


// Reset definitions for next case.
#undef	vt3
#undef	vt2
#define	ve4	%xmm4
#define	ve5	%xmm5


EKeyHas6Words:
	movd	4*4(K), ve4
	movd	5*4(K), ve5

	// K cannot be used after we write to R, since they use the same register.

	#if defined __i386__
		// Get address of 0 in R.
			call	0f			// Push program counter onto stack.
		0:
			pop		STable		// Get program counter.

		lea		_OLDAESRcon-0b(STable), R
		lea		_OLDAESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_OLDAESRcon(%rip), R
		lea		_OLDAESSubBytesWordTable(%rip), STable

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

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		// Add pops of r15 to r12 if used.
		pop		r7
		pop		r6
	#elif defined __x86_64
	#endif
	pop		r5

	ret


// Reset definitions for next case.
#undef	ve4
#undef	ve5
#define	vt3	%xmm4
#define	vt2	%xmm5


EKeyHas8Words:

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
		lea		_OLDAESSubBytesWordTable-0b(STable), STable

	#elif defined __x86_64__

		lea		_OLDAESRcon(%rip), R
		lea		_OLDAESSubBytesWordTable(%rip), STable

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

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		// Add pops of r15 to r12 if used.
		pop		r7
		pop		r6
	#elif defined __x86_64
	#endif
	pop		r5

	ret


#undef	Address
#undef	Argument
#undef	E
#undef	LocalsSize
#undef	LookupS
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
#undef	vt0
#undef	vt1
#undef	vt2
#undef	vt3
