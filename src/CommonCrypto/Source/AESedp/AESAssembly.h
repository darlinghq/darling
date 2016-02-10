#if !defined AES_h
#define AES_h


// Include aesopt.h to get the UseAESedp symbol.  We use nothing else from it.
#include <CommonCrypto/aesopt.h>


// Generate object code iff UseAESedp is defined.
#if defined UseAESedp


// Select which implementation to use.
#if 1
	#define	UseAESedp_IntelAssembly
#else
	#define	UseAESedp_GeneralC
#endif


/*	MaxNb is the maximum value of Nb, the number of four-byte words in one data
	block.
*/
#define	MaxNb	4

/*	Nb is the number of four-byte words in one data block.  AES fixes Nb at 4,
	although Rijndael allows up to 8.
*/
#define	Nb		4

/*	MaxNk is the maximum value of Nk, the number of four-byte words in a key.
	AES and Rijndael allow up to 8.
*/
#define	MaxNk	8

/*	Nk is not defined here because different key sizes are supported
	dynamically.
*/

/*	MaxRcon is the maximum number of round constants that might be needed.
	The number needed is (Nb*(Nr+1)-1) / Nk + 1.  For AES, Nr is Nk + 6.  (For
	Rijndael, Nr is max(Nk, Nb) + 6.)  For AES, we have:

		(Nb*(Nr+1)-1) / Nk + 1.
		(Nb*(Nk+6+1)-1) / Nk + 1.
		(Nb*Nk + Nb*7 - 1) / Nk + 1.
		Nb + (Nb*7-1)/Nk + 1.

	Clearly this is greatest when Nk is smallest.  Nk is at least 4.  In AES,
	Nb is 4, so we have 4 + 27/4 + 1 = 11.

	(In Rijndael, the maximum is 30, occurring when Nb is 8 and Nk is 4.)
*/
#define	MaxRcon	11


#if !__ASSEMBLER__

	#include <stdint.h>

	typedef uint8_t Byte;
	typedef uint32_t Word;
	typedef union { Byte b[MaxNb*4]; Word w[MaxNb]; } AESData;
	typedef union { Byte b[MaxNk*4]; Word w[MaxNk]; } AESKey ;

#endif


#endif	// defined UseAESedp


#endif	// !defined AES_h
