/*	This file defines _AESEncryptWithExpandedKey or _AESDecryptWithExpandedKey,
	according to the value of the Select preprocessor symbol.  This file is
	designed to be included in another assembly file using the preprocessor
	#include directive, to benefit from some assembly-time calculations.

	These two routines are nearly identical.  They differ only in the tables
	they use, the direction they iterate through the key, and the permutation
	performed on part of the state.

	Written by Eric Postpischil, January 2008.
*/


#if Select == 0
	#define	Name		_AESEncryptWithExpandedKey	// Routine name.
	#define	MTable		_OLDAESEncryptTable			// Main table.
	#define	FTable		_OLDAESSubBytesWordTable		// Final table.
	#define	P0			S0							// State permutation.
	#define	P1			S1
	#define	P2			S2
	#define	P3			S3
	#define	Increment	+16							// ExpandedKey increment.
#elif Select == 1
	#define	Name		_AESDecryptWithExpandedKey	// Routine name.
	#define	MTable		_OLDAESDecryptTable			// Main table.
	#define	FTable		_OLDAESInvSubBytesWordTable	// Final table.
	#define	P0			S2							// State permutation.
	#define	P1			S3
	#define	P2			S0
	#define	P3			S1
	#define	Increment	-16							// ExpandedKey increment.
#endif	// Select


/*	Routine:

		_AESEncryptWithExpandedKey (if Select is 0) or
		_AESDecryptWithExpandedKey (if Select is 1).

	Function:

		Perform the AES cipher or its inverse as defined in Federal Information
		Processing Standards Publication 197 (FIPS-197), November 26, 2001.

		The inverse cipher here is the "Equivalent Inverse Cipher" in FIPS-197.

	Input:

		Constant data:
				
			For encryption:

				The following names must be locally defined so the assembler
				can calculate certain offsets.

				static const Word _OLDAESEncryptTable[4][256].

					_OLDAESEncryptTable[i] contains the tables T[i] defined in AES
					Proposal: Rijndael, version 2, 03/09/99, by Joan Daemen and
					Vincent Rijmen, section 5.2.1, page 18.  These tables
					combine the SubBytes and MixColumns operations.

				static const Word _OLDAESSubBytesWordTable[256].

					_OLDAESSubBytesWordTable[i][j] = SubBytes(j) << 8*i, where
					SubBytes is defined in FIPS-197.  _OLDAESSubBytesWordTable
					differs from _OLDAESEncryptTable in that it does not include
					the MixColumn operation.  It is used in performing the last
					round, which differs fromm the previous rounds in that it
					does not include the MixColumn operation.

			For decryption:

				static const Word _OLDAESDecryptTable[4][256].

					The analog of _OLDAESEncryptTable for decryption.

				static const Word _OLDAESSubBytesWordTable[256].

					_OLDAESInvSubBytesWordTable[i][j] = InvSubBytes(j) << 8*i,
					where InvSubBytes is defined in FIPS-197.
					_OLDAESInvSubBytesWordTable differs from _OLDAESDecryptTable in
					that it does not include the InvMixColumn operation.  It is
					used in performing the last round, which differs fromm the
					previous rounds in that it does not include the
					InvMixColumn operation.

		Arguments:

			Byte *OutputText.

				Address of output, 16 bytes.  Best if four-byte aligned.

			const Byte *InputText.

				Address of input, 16 bytes.  Best if four-byte aligned.

			const Byte *ExpandedKey.

				Address of expanded key, which has 4 * (Nr+1) bytes.  Best if
				four-byte aligned.

			int Nr

				Number of rounds.

	Output:

		Encrypted or decrypted data is written to *OutputText.
*/
	.globl Name
	.private_extern	Name
Name:

	// Push new stack frame.
	push	r5

	// Save registers and set RegisterSave size to the number of bytes used.
	push	r3
	#if defined __i386__
		push	r6
		push	r7
		#define	RegisterSaveSize	(3*4)
	#elif defined __x86_64__
		#define	RegisterSaveSize	(1*8)
	#endif

#define	LocalsSize	Arch(4, 0)	// Number of bytes used for local variables.

	#if 0 < LocalsSize
		sub		$LocalsSize, r4	// Allocate space on stack.
	#endif

// Number of bytes from the stack pointer to the return address.
#define	StackFrame	(LocalsSize+RegisterSaveSize)

#if defined __i386__

	// Define location of argument i (presuming 4-byte arguments).
	#define	Argument(i)	StackFrame+8+4*(i)(%esp)

	#define	ArgOutputText	Argument(0)
	#define	ArgInputText	Argument(1)
	#define	ArgExpandedKey	Argument(2)
	#define	ArgNr			Argument(3)

#elif defined __x86_64__

	// Arguments.
	#define	OutputText		r7	// Needed near end of routine.
	#define	InputText		r6	// Used early then overwritten for other use.
	#define	ArgExpandedKey	r2
	#define	ArgNr			r1
		/*	The arguments passed in r1 and r2 overlaps registers we need for
		 	other work, so they must be moved early in the routine.
		*/

#endif

#define	BaseP		Arch(r7, r9)	// Base pointer for addressing global data.
#define	ExpandedKey	Arch(t0, r10)	// Address of expanded key.

/*	The Work registers defined below are used to hold parts of the AES state
	while we dissect or assemble it.  They must be assigned to the A, B, C, and
	D registers so that we can access the bytes in %al, %ah, and so on.
*/
#define	Work0d	r0d
#define	Work0l	r0l
#define	Work0h	r0h
#define	Work1d	r3d
#define	Work1l	r3l
#define	Work1h	r3h
#define	Work2d	r1d
#define	Work2l	r1l
#define	Work2h	r1h
#define	Work3d	r2d
#define	Work3l	r2l
#define	Work3h	r2h

#define	t0		r5
#define	t0d		r5d		// Low 32 bits of t0.
#define	t0l		r5l		// Low byte of t0.

#define	t1		r6

/*	S0, S1, S2, and S3 are where we assemble the new AES state when computing
	a regular round.  S1, S2, and S3 are assigned to the Work registers, but
	S0 needs to go somewhere else because Work0 holds part of the old state.
*/
#define	S0		Arch(t1, r8d)
#define	S1		Work1d
#define	S2		Work2d
#define	S3		Work3d

/*	These XMM registers are used as holding space, because it is faster to
	spill to these registers than to the stack.  (On x86_64, we do not need
	to spill, because there are additional general registers available.
	However, using more general registers requires saving them to the stack
	and restoring them.  I timed it, and no time was saved.)
*/
#define	vS1		%xmm1
#define	vS2		%xmm2
#define	vS3		%xmm3
#if defined __i386__
	#define	vExpandedKey	%xmm4
	#define	vIncrement		%xmm5
#endif

	// Get argument.
	mov	ArgExpandedKey, ExpandedKey

// Store sentinel value of ExpandedKey on stack on i386, a register on x86_64.
#define	ExpandedKeyEnd	Arch((r4), r11)

	/*	Convert ArgNr from rounds to number of bytes to move through expanded
		key to get to (but not beyond) last 16-byte block.
	*/
	mov		ArgNr, r0
	shl		$4, r0

	#if Select == 0
		// For encryption, prepare to iterate forward through expanded key.
		add		ExpandedKey, r0
		mov		r0, ExpandedKeyEnd
	#else
		// For decryption, prepare to iterate backward through expanded key.
		mov		ExpandedKey, ExpandedKeyEnd
		add		r0, ExpandedKey
	#endif

	// Initialize State from input text.
	#if defined __i386__
		mov		ArgInputText, BaseP
		#define	InputText	BaseP
	#endif
	mov		0*4(InputText), Work0d
	mov		1*4(InputText), S1
	mov		2*4(InputText), S2
	mov		3*4(InputText), S3
#undef	InputText	// Register is reused after this for other purposes.

	// Add round key and save results.
	xor		0*4(ExpandedKey), Work0d	// S0 is in dissection register.
	xor		1*4(ExpandedKey), S1
	movd	S1, vS1						// Save S1 to S3 in vector registers.
	xor		2*4(ExpandedKey), S2
	movd	S2, vS2
	xor		3*4(ExpandedKey), S3
	movd	S3, vS3

	add		$Increment, ExpandedKey		 // Advance to next round key.

	#if defined __i386__
		// Save expanded key address and increment in vector registers.
		mov		$Increment, t1
		movp	ExpandedKey, vExpandedKey
		movp	t1, vIncrement
	#endif

	// Set up relative addressing.
	#if defined __i386__

		// Get address of 0 in BaseP.
			call	0f				// Push program counter onto stack.
		0:
			pop		BaseP			// Get program counter.

		// Define macros to help address data.
#define	LookupM(table, index)	MTable-0b+(table)*TableSize(BaseP, index, 4)
#define LookupF(table, index)	FTable-0b+(table)*TableSize(BaseP, index, 4)

	#elif defined __x86_64__

		lea	MTable(%rip), BaseP

		// Define macros to help address data.
		#define	LookupM(table, index)	(table)*TableSize(BaseP, index, 4)
		#define	LookupF(table, index)	(table)*TableSize(BaseP, index, 4)

/*	With these definitions of LookupM and LookupF, BaseP must be loaded with
	the address of the table at the point where it is used.  So we need an
	instruction to change BaseP after we are done with MTable and before we
	start using FTable.  I would prefer to use something like:

		.set	FMinusM, FTable - MTable
		#define LookupF(table, index)	\
			FMinusM+(table)*TableSize(BaseP, index, 4)

	Then BaseP would not need to change.  However, this fails due to an
	assembler/linker bug, <rdar://problem/5683882>.
*/

	#endif

	// Get round key.
	mov		0*4(ExpandedKey), S0
	mov		1*4(ExpandedKey), S1
	mov		2*4(ExpandedKey), S2
	mov		3*4(ExpandedKey), S3

1:
	/*	Word 0 of the current state must be in Work0 now, and the next round
		key must be in S0 to S3.
	*/

	// Process previous S0.
	movzx	Work0l, t0
	xor		LookupM(0, t0), S0
	movzx	Work0h, t0d
	xor		LookupM(1, t0), P3
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupM(2, t0), S2
	movzx	Work0h, t0d
	xor		LookupM(3, t0), P1

	// Process previous S1.
	movd	vS1, Work0d
	movzx	Work0l, t0d
	xor		LookupM(0, t0), S1
	movzx	Work0h, t0d
	xor		LookupM(1, t0), P0
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupM(2, t0), S3
	movzx	Work0h, t0d
	xor		LookupM(3, t0), P2

	// Process previous S2.
	movd	vS2, Work0d
	movzx	Work0l, t0d
	xor		LookupM(0, t0), S2
	movzx	Work0h, t0d
	xor		LookupM(1, t0), P1
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupM(2, t0), S0
	movzx	Work0h, t0d
	xor		LookupM(3, t0), P3

	// Process previous S3.
	movd	vS3, Work0d
	movzx	Work0l, t0d
	xor		LookupM(0, t0), S3
	movzx	Work0h, t0d
	xor		LookupM(1, t0), P2
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupM(2, t0), S1
	movzx	Work0h, t0d
	xor		LookupM(3, t0), P0

	#if defined __i386__
		paddd	vIncrement, vExpandedKey
		movp	vExpandedKey, ExpandedKey
	#else
		add		$Increment, ExpandedKey
	#endif

	// Save state for next iteration and load next round key.
	mov		S0, Work0d
	mov		0*4(ExpandedKey), S0
	movd	S1, vS1
	mov		1*4(ExpandedKey), S1
	movd	S2, vS2
	mov		2*4(ExpandedKey), S2
	movd	S3, vS3
	mov		3*4(ExpandedKey), S3

	cmp		ExpandedKeyEnd, ExpandedKey
	jne		1b

	/*	Word 0 of the current state must be in Work0 now, and the next round
		key must be in S0 to S3.
	*/

	// Work around assembler bug.  See comments above about Radar 5683882.
	#if defined __x86_64__
		lea	FTable(%rip), BaseP
	#endif

	// Process previous S0.
	movzx	Work0l, t0
	xor		LookupF(0, t0), S0
	movzx	Work0h, t0d
	xor		LookupF(1, t0), P3
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupF(2, t0), S2
	movzx	Work0h, t0d
	xor		LookupF(3, t0), P1

	// Process previous S1.
	movd	vS1, Work0d
	movzx	Work0l, t0d
	xor		LookupF(0, t0), S1
	movzx	Work0h, t0d
	xor		LookupF(1, t0), P0
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupF(2, t0), S3
	movzx	Work0h, t0d
	xor		LookupF(3, t0), P2

	// Process previous S2.
	movd	vS2, Work0d
	movzx	Work0l, t0d
	xor		LookupF(0, t0), S2
	movzx	Work0h, t0d
	xor		LookupF(1, t0), P1
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupF(2, t0), S0
	movzx	Work0h, t0d
	xor		LookupF(3, t0), P3

	// Process previous S3.
	movd	vS3, Work0d
	movzx	Work0l, t0d
	xor		LookupF(0, t0), S3
	movzx	Work0h, t0d
	xor		LookupF(1, t0), P2
	shr		$16, Work0d
	movzx	Work0l, t0d
	xor		LookupF(2, t0), S1
	movzx	Work0h, t0d
	xor		LookupF(3, t0), P0

	#if defined __i386__	// Architecture.
		// Get OutputText address.
		#define	OutputText	BaseP
		mov		ArgOutputText, OutputText
	#endif	// Architecture.

	// Write output.
	mov		S0, 0*4(OutputText)
	mov		S1, 1*4(OutputText)
	mov		S2, 2*4(OutputText)
	mov		S3, 3*4(OutputText)

	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		pop		r7
		pop		r6
	#elif defined __x86_64__
	#endif
	pop		r3
	pop		r5

	ret


#undef	ArgExpandedKey
#undef	ArgInputText
#undef	ArgNr
#undef	ArgOutputText
#undef	Argument
#undef	BaseP
#undef	ExpandedKey
#undef	ExpandedKeyEnd
#undef	FTable
#undef	InputText
#undef	LocalsSize
#undef	LookupM
#undef	LookupF
#undef	MTable
#undef	OutputText
#undef	RegisterSaveSize
#undef	S0
#undef	S1
#undef	S2
#undef	S3
#undef	StackFrame
#undef	Work0d
#undef	Work0h
#undef	Work0l
#undef	Work1d
#undef	Work1h
#undef	Work1l
#undef	Work2d
#undef	Work2h
#undef	Work2l
#undef	Work3d
#undef	Work3h
#undef	Work3l
#undef	t0
#undef	t0d
#undef	t0l
#undef	t1
#undef	vExpandedKey
#undef	vS1
#undef	vS2
#undef	vS3

#undef	Name
#undef	MTable
#undef	FTable
#undef	P0
#undef	P1
#undef	P2
#undef	P3
#undef	Increment
