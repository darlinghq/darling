#if !defined sha1edp_h
#define	sha1edp_h


/*	This file is included in sha1edpLittleEndian.s and sha1edpBigEndian.s to
	define the symbols below for use in assembly code.

	It is also included in sha1_locl.h and compiled in C to test that the
	hard-coded values here match the values used in C.  CC_SHA1_BLOCK_BYTES
	is defined in another header, so an error will be generated if its
	definition here conflicts.  The other symbols are tested below, with
	the CheckAssertion definition.
*/


// Number of bytes in a SHA-1 block.
#define	CC_SHA1_BLOCK_BYTES	64

// Offset of h0 to h4 members in SHA-1 context structure.
#define	Contexth0	(0*4)
#define	Contexth1	(1*4)
#define	Contexth2	(2*4)
#define	Contexth3	(3*4)
#define	Contexth4	(4*4)


#if !defined __ASSEMBLER__

	#include <stddef.h>	// Get offsetof macro.

	/*	Declare CheckAssertion so that if any of the declarations below
		differ from it, the compiler will report an error.
	*/
	extern char CheckAssertion[1];

	/*	Ensure that Contexth0 through Contexth4 are the byte offsets of the
		h0 through h4 members of the SHA-1 context structure.
	*/
	extern char CheckAssertion[Contexth0 == offsetof(SHA_CTX, h0)];
	extern char CheckAssertion[Contexth1 == offsetof(SHA_CTX, h1)];
	extern char CheckAssertion[Contexth2 == offsetof(SHA_CTX, h2)];
	extern char CheckAssertion[Contexth3 == offsetof(SHA_CTX, h3)];
	extern char CheckAssertion[Contexth4 == offsetof(SHA_CTX, h4)];
		/*	If these assertions fail, change the definitions of Contexth0 to
			Contexth4 to match the offsets of the members.
		*/

#endif	// !defined __ASSEMBLER__


#endif	// !defined sha1edp_h
