/*	AES.s -- Core AES routines for Intel processors.

	Written by Eric Postpischil, January 30, 2008.
*/
#if defined __i386__ || defined __x86_64__


/*	We build these AES routines as a single module because the routines refer
	to labels in Data.s and it is easier and faster to refer to them as local
	labels.  In my implementations of AES for CommonCrypto, both i386 and
	x86_64 use position-independent code.  For this in-kernel implementation,
	i386 has been converted to absolute addressing, but x86_64 still uses PIC.

	A local label can be referred to with position-independent assembler
	expressions such as "label-base(register)", where <base> is a local label
	whose address has been loaded into <register>.  (On i386, this is typically
	done with the idiom of a call to the next instruction and a pop of that
	return address into a register.)  Without local labels, the references must
	be done using spaces for addresses of "lazy symbols" that are filled in by
	the dynamic loader and loaded by the code that wants the address.

	So the various routines in other files are assembled here via #include
	directives.
*/
#include "Data.s"


#define	TableSize	(256*4)
	/*	Each of the arrays defined in Data.s except for the round constants
		in _AESRcon is composed of four tables of 256 entries of four bytes
		each.  TableSize is the number of bytes in one of those four tables.
	*/


// Include constants describing the AES context structures.
#include "Context.h"


/*	Define a macro to select a value based on architecture.  This reduces
	some of the architecture conditionalization later in the source.
*/
#if defined __i386__
	#define	Arch(i386, x86_64)	i386
#elif defined __x86_64__
	#define	Arch(i386, x86_64)	x86_64
#endif


// Define an instruction for moving pointers.
#define	movp	Arch(movd, movd)
	// Latter argument should be "movq", but the assembler uses "movd".


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

// Define names for parts of registers.

#define	r0d		%eax	// Low 32 bits of r0.
#define	r1d		%ecx	// Low 32 bits of r1.
#define	r2d		%edx	// Low 32 bits of r2.
#define	r3d		%ebx	// Low 32 bits of r3.
#define	r5d		%ebp	// Low 32 bits of r5.
#define	r6d		%esi	// Low 32 bits of r6.
#define	r7d		%edi	// Low 32 bits of r7.
#define	r8d		%r8d	// Low 32 bits of r8.
#define	r9d		%r9d	// Low 32 bits of r9.
#define	r11d	%r11d	// Low 32 bits of r11.

#define	r0l		%al		// Low byte of r0.
#define	r1l		%cl		// Low byte of r1.
#define	r2l		%dl		// Low byte of r2.
#define	r3l		%bl		// Low byte of r3.
#define	r5l		%bpl	// Low byte of r5.

#define	r0h		%ah		// Second lowest byte of r0.
#define	r1h		%ch		// Second lowest byte of r1.
#define	r2h		%dh		// Second lowest byte of r2.
#define	r3h		%bh		// Second lowest byte of r3.


	.text


// Define encryption routine, _AESEncryptWithExpandedKey
#define	Select	0
#include "EncryptDecrypt.s"
#undef	Select


// Define decryption routine, _AESDecryptWithExpandedKey
#define	Select	1
#include "EncryptDecrypt.s"
#undef	Select

// Define encryption routine, _AESEncryptWithExpandedKey
#define	Select	2
#include "EncryptDecrypt.s"
#undef	Select


// Define decryption routine, _AESDecryptWithExpandedKey
#define	Select	3
#include "EncryptDecrypt.s"
#undef	Select


// Define key expansion routine for encryption, _AESExpandKeyForEncryption.
#include "ExpandKeyForEncryption.s"


// Define key expansion for decryption routine, _AESExpandKeyForDecryption.
#include "ExpandKeyForDecryption.s"
#endif /* x86 based build */
