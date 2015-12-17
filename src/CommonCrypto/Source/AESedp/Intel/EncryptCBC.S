#include "../AESAssembly.h"


// Generate object code only if this implementation has been requested.
#if defined UseAESedp_IntelAssembly


/*	AESEncryptCBC.s -- Encrypt blocks with AES in Cipher Block Chaining mode.

	Written by Eric Postpischil, January 24, 2008.
*/


/*	Define a macro to select a value based on architecture.  This reduces
	some of the architecture conditionalization later in the source.
*/
#if defined __i386__
	#define	Arch(i386, x86_64)	i386
#elif defined __x86_64__
	#define	Arch(i386, x86_64)	x86_64
#endif


/*	Rename the general registers.  This makes it easier to keep track of them
	and provides names for the "whole register" that are uniform between i386
	and x86_64.
*/
#if defined __i386__
	#define	r0	%eax	// Available for any use.
	#define	r1	%ecx	// Available for any use, some special purposes (loop).
	#define	r2	%edx	// Available for any use.
	#define	r3	%ebx	// Must be preserved by called routine.
	#define	r4	%esp	// Stack pointer.
	#define	r5	%ebp	// Frame pointer, must preserve, no bare indirect.
	#define	r6	%esi	// Must be preserved by called routine.
	#define	r7	%edi	// Must be preserved by called routine.
#elif defined __x86_64__
	#define	r0	%rax	// Available for any use.
	#define	r1	%rcx	// Available for any use.
	#define	r2	%rdx	// Available for any use.
	#define	r3	%rbx	// Must be preserved by called routine.
	#define	r4	%rsp	// Stack pointer.
	#define	r5	%rbp	// Frame pointer.  Must be preserved by called routine.
	#define	r6	%rsi	// Available for any use.
	#define	r7	%rdi	// Available for any use.
	#define	r8	%r8		// Available for any use.
	#define	r9	%r9		// Available for any use.
	#define	r10	%r10	// Available for any use.
	#define	r11	%r11	// Available for any use.
	#define	r12	%r12	// Must be preserved by called routine.
	#define	r13	%r13	// Must be preserved by called routine.
	#define	r14	%r14	// Must be preserved by called routine.
	#define	r15	%r15	// Must be preserved by called routine.
#else
	#error "Unknown architecture."
#endif


/*	Routine:

		_AESEncryptCBC.

	Function:

		This routine uses _AESEncryptWithExpandedKey to encrypt blocks in
		Cipher Block Chaining mode, which requires chaining the AES state
		from block to block.  In CBC mode, an initial block is XORed with the
		first input block, and then each output block is XORed with the next
		input block.

	Input:

		void *O				// Output
		const void *I		// Input
		void *ChainBuffer	// Chain buffer / initial value.
		void *Key			// Expanded Key.
		long Blocks			// Number of 16-byte blocks to process.
		long Rounds			// Number of rounds.

	Output:

		Encrypted text is written to *O.

		The final output block is written to *ChainBuffer.
*/
	.globl _AESEncryptCBC
	.private_extern	_AESEncryptCBC
_AESEncryptCBC:

	// Push new stack frame.
	push	r5

	// Save registers.
	push	r3
	#if defined __i386__
		push	r6
		push	r7
		#define	RegisterSaveSize	(3*4)
	#elif defined __x86_64__
		push	r12
		push	r13
		push	r14
		push	r15
		#define	RegisterSaveSize	(5*8)
	#endif

/*	B is the number of bytes from the top of stack just before the instruction
	that called this routine to the top of stack after we push the frame
	pointer and other registers.  It provides information needed to align our
	stack frame.
*/
#define	B	(RegisterSaveSize + 2*Arch(4, 8))

/*	Allocate space on the stack for 16 bytes for the AES state and, on i386,
	16 bytes for four four-byte arguments, and padding needed to produce
	16-byte alignment.
*/
#define	LocalsSize	((16 + Arch(16, 0) + B + 15 & -16) - B)
#define	StackFrame	(LocalsSize + B)

/*	LocalState is the offset from the stack pointer to where we store the AES
 	state.
*/
#define	LocalState	Arch(16, 0)

	#if 0 < LocalsSize
		sub		$LocalsSize, r4	// Allocate space on stack.
	#endif

// Non-volatile registers.
#define	I			r3
#define	O			r5
#define	Blocks		Arch(r6, r12)
#define	ChainBuffer	Arch(r7, r13)
#define	Rounds		Arch(Not used, r14)
#define	Key			Arch(Not used, r15)

// Volatile registers.
#define	t0			r0
#define	v0			%xmm0
#define	v1			%xmm1
#define	v2			%xmm2
#define	v3			%xmm3
#define	vState0		%xmm4
#define	vState1		%xmm5
#define	vState2		%xmm6
#define	vState3		%xmm7

// Arguments passed to us.
#if defined __i386__
	// Define location of argument i.
	#define	Argument(i)	StackFrame+4*(i)(r4)
#endif
#define	ArgO			Arch(Argument(0), r7)
#define	ArgI			Arch(Argument(1), r6)
#define	ArgChainBuffer	Arch(Argument(2), r2)
#define	ArgKey			Arch(Argument(3), r1)
#define	ArgBlocks		Arch(Argument(4), r8)
#define	ArgRounds		Arch(Argument(5), r9)

	/*	Get some arguments.  We need to move these from the stack (on i386)
		or volatile registers (on x86_64) to non-volatile registers where we
		can use them and keep them during calls to a subroutine.
	*/
	mov		ArgO, O
	mov		ArgI, I
	mov		ArgChainBuffer, ChainBuffer
	mov		ArgBlocks, Blocks

	// Read the initial value from the chain buffer.
	movd	0*4(ChainBuffer), vState0
	movd	1*4(ChainBuffer), vState1
	movd	2*4(ChainBuffer), vState2
	movd	3*4(ChainBuffer), vState3

	/*	Convert Blocks from number of blocks to displacement in bytes from
		end of input to current input location.  (We will increment it from
		iteration to iteration.  When it reaches zero, we are done.)
	*/
	imul	$-16, Blocks
	je		done	// Leave if we were given zero blocks.

	// Adjust input and output pointers to use ends as base addresses.
	sub		Blocks, I
	sub		Blocks, O

	#if defined __i386__

		// Put arguments we will pass on stack.
		mov		ArgRounds, t0
		mov		t0,    3*4(r4)

		mov		ArgKey, t0
		mov		t0,    2*4(r4)

		lea		LocalState(r4), t0
		mov		t0,    1*4(r4)
		mov		t0,    0*4(r4)

	#else

		// Put arguments we will pass into non-volatile registers.
		mov		ArgRounds, Rounds
		mov		ArgKey,    Key

	#endif

1:
	// Read next input block.
	movd	0*4(I, Blocks), v0
	movd	1*4(I, Blocks), v1
	movd	2*4(I, Blocks), v2
	movd	3*4(I, Blocks), v3

	// Chain block with state.
	pxor	v0, vState0
	pxor	v1, vState1
	pxor	v2, vState2
	pxor	v3, vState3

	// Store state for passing to encryption routine.
	movd	vState0, 0*4+LocalState(r4)
	movd	vState1, 1*4+LocalState(r4)
	movd	vState2, 2*4+LocalState(r4)
	movd	vState3, 3*4+LocalState(r4)

	#if defined __x86_64__

		// Pass arguments to subroutine.
		#define	PassedRounds	r1
		#define	PassedKey		r2
		#define	PassedInput		r6
		#define	PassedOutput	r7
		mov		Rounds,         PassedRounds
		mov		Key,            PassedKey
		lea		LocalState(r4), PassedInput
		lea		(O, Blocks),    PassedOutput

	#endif

	// Encrypt state.
	call	_AESEncryptWithExpandedKey

	#if defined __i386__

		// Get encrypted state.
		movd	0*4+LocalState(r4), vState0
		movd	1*4+LocalState(r4), vState1
		movd	2*4+LocalState(r4), vState2
		movd	3*4+LocalState(r4), vState3

		// Write to output.
		movd	vState0, 0*4(O, Blocks)
		movd	vState1, 1*4(O, Blocks)
		movd	vState2, 2*4(O, Blocks)
		movd	vState3, 3*4(O, Blocks)

	#else

		// Get output for chaining.
		movd	0*4(O, Blocks), vState0
		movd	1*4(O, Blocks), vState1
		movd	2*4(O, Blocks), vState2
		movd	3*4(O, Blocks), vState3

	#endif

	add		$16, Blocks

	jl		1b

	// Save state for chaining in future calls.
	movd	vState0, 0*4(ChainBuffer)
	movd	vState1, 1*4(ChainBuffer)
	movd	vState2, 2*4(ChainBuffer)
	movd	vState3, 3*4(ChainBuffer)

done:
	// Pop stack and restore registers.
	#if 0 < LocalsSize
		add		$LocalsSize, r4
	#endif
	#if defined __i386__
		pop		r7
		pop		r6
	#elif defined __x86_64__
		pop		r15
		pop		r14
		pop		r13
		pop		r12
	#endif
	pop		r3
	pop		r5

	ret


#endif	// defined UseAESedp_IntelAssembly
