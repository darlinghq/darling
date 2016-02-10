#include "AESAssembly.h"

// Generate object code iff this implementation is requested.
#if defined UseAESedp


/*	This module must not be compiled with -fstrict-aliasing.

	We are forced to do some aliasing in this module, because we must conform
	to an external API but need to do four-byte word manipulations for
	efficiency.  For example, chainBuf in the aes_cc_ctx structure is an array
	of char, but we operate in units of four-byte words (or bigger).
*/


#if defined UseAESedp_GeneralC


static void XorBlock(AESData *a, const AESData *b, const AESData *c)
{
	a->w[0] = b->w[0] ^ c->w[0];
	a->w[1] = b->w[1] ^ c->w[1];
	a->w[2] = b->w[2] ^ c->w[2];
	a->w[3] = b->w[3] ^ c->w[3];
}


/*	The code inside this preprocessor conditional clause is retained as an
	illustration of how the assembly implementation works.  For the most part,
	the code here implements AES in the same way the accompanying Intel
	assembly code does.

	While the assembly implementation performs well on an Intel processor, the
	code GCC generates for this C code is not particularly fast.

	Key expansion differs somewhat from the assembly implementation.  AES
	presents an Inverse Cipher for decryption that is not immediately suitable
	to table implementation.  AES also offers an Equivalent Inverse Cipher
	which is suitable for table implementation.  In the Equivalent Inverse
	Cipher, the InvMixColumn operation is switched with an XOR with the key.
	Fortunately, InvMixColumn distributes over XOR (it is a linear combination
	of its operands in a Galois field the AES defines, and the XOR is an
	addition in the field), so the swap can be made by applying InvMixColumn to
	the blocks of the key that will be used in the swapped operations.

	In the C code, InvMixColumn is applied in a separate step after expanded
	the key, in a for-loop inside the aes_cc_set_key routine.  In the assembly
	code, InvMixColumn is integrated into the key expansion code.
*/


#include "Data.c"	// Include tables with precalculated AES functions.


/*	This implements the InvMixColumn operation of the AES algorithm.  It is
	needed as a separate function during key expansion for decryption.
*/
static Word InvMixColumn(Word w)
{
	union { Word w; Byte b[4]; } b = { w };
	Byte 
		s0 = b.b[0],
		s1 = b.b[1],
		s2 = b.b[2],
		s3 = b.b[3];

	return
		  AESInvMixColumnTable[0][s0]
		^ AESInvMixColumnTable[1][s1]
		^ AESInvMixColumnTable[2][s2]
		^ AESInvMixColumnTable[3][s3];
}


// Expand the user's cipher key according to the AES key schedule.
static void AESExpandKey(Word *ExpandedKey, const AESKey *Key, long Nk)
{
	const Word (*T)[256] = AESSubBytesWordTable;
	const Byte *R = AESRcon;

	Word * const E = ExpandedKey;

	switch (Nk)
	{
		default:
		case 4:
		{
			const int Nr = 10;

			// The first words are just copies of the key.
			Word
				e0 = E[0] = Key->w[0],
				e1 = E[1] = Key->w[1],
				e2 = E[2] = Key->w[2],
				e3 = E[3] = Key->w[3];

			for (int i = Nk; i < Nb * (Nr + 1); i += Nk)
			{
				Word temp;
				{
					Byte * const b = (Byte *) &e3;

					Byte b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
					temp = T[0][b1] ^ T[1][b2] ^ T[2][b3] ^ T[3][b0];
				}
				temp ^= *++R;
				E[i+0] = e0 ^= temp;
				E[i+1] = e1 ^= e0;
				E[i+2] = e2 ^= e1;
				E[i+3] = e3 ^= e2;
			}
			break;
		}
		case 6:
		{
			const int Nr = 12;

			// The first words are just copies of the key.
			for (int i = 0; i < Nk; ++i)
				E[i] = Key->w[i];

			Word temp = E[Nk-1];
			for (int i = Nk; 1; i += Nk)
			{
				{
					Byte * const b = (Byte *) &temp;

					Byte b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
					temp = T[0][b1] ^ T[1][b2] ^ T[2][b3] ^ T[3][b0];
					temp ^= *++R;
				}
				E[i+0] = temp ^= E[i+0-Nk];
				E[i+1] = temp ^= E[i+1-Nk];
				E[i+2] = temp ^= E[i+2-Nk];
				E[i+3] = temp ^= E[i+3-Nk];

				if (Nb * Nr <= i)
					break;

				E[i+4] = temp ^= E[i+4-Nk];
				E[i+5] = temp ^= E[i+5-Nk];
			}
			break;
		}
		case 8:
		{
			const int Nr = 14;

			// The first words are just copies of the key.
			for (int i = 0; i < Nk; ++i)
				E[i] = Key->w[i];

			Word temp = E[Nk-1];
			for (int i = Nk; 1; i += Nk)
			{
				{
					Byte * const b = (Byte *) &temp;

					Byte b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
					temp = T[0][b1] ^ T[1][b2] ^ T[2][b3] ^ T[3][b0];
					temp ^= *++R;
				}
				E[i+0] = temp ^= E[i+0-Nk];
				E[i+1] = temp ^= E[i+1-Nk];
				E[i+2] = temp ^= E[i+2-Nk];
				E[i+3] = temp ^= E[i+3-Nk];

				if (Nb * Nr <= i)
					break;

				{
					Byte * const b = (Byte *) &temp;

					Byte b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
					temp = T[0][b0] ^ T[1][b1] ^ T[2][b2] ^ T[3][b3];
				}
				E[i+4] = temp ^= E[i+4-Nk];
				E[i+5] = temp ^= E[i+5-Nk];
				E[i+6] = temp ^= E[i+6-Nk];
				E[i+7] = temp ^= E[i+7-Nk];
			}
			break;
		}
	}
}


// This is the main encryption routine.
static void AESEncryptWithExpandedKey(Byte *Ciphertext, const Byte *Plaintext,
	const AESData *ExpandedKey, long Nr)
{
	AESData State;

	XorBlock(&State, (const AESData *) Plaintext, &ExpandedKey[0]);

	{
		const Word (*T)[256] = AESEncryptTable;

		for (int round = 1; round < Nr; ++round)
		{
			const AESData *Key = &ExpandedKey[round];

			const union { Word w; Byte b[4]; }
				w0 = { State.w[0] },
				w1 = { State.w[1] },
				w2 = { State.w[2] },
				w3 = { State.w[3] };

			State.w[0] = Key->w[0]
				^ T[0][w0.b[0]] ^ T[1][w1.b[1]] ^ T[2][w2.b[2]] ^ T[3][w3.b[3]];
			State.w[1] = Key->w[1]
				^ T[0][w1.b[0]] ^ T[1][w2.b[1]] ^ T[2][w3.b[2]] ^ T[3][w0.b[3]];
			State.w[2] = Key->w[2]
				^ T[0][w2.b[0]] ^ T[1][w3.b[1]] ^ T[2][w0.b[2]] ^ T[3][w1.b[3]];
			State.w[3] = Key->w[3]
				^ T[0][w3.b[0]] ^ T[1][w0.b[1]] ^ T[2][w1.b[2]] ^ T[3][w2.b[3]];
		}
	}

	{
		const Word (*T)[256] = AESSubBytesWordTable;

		const AESData *Key = &ExpandedKey[Nr];

		const union { Word w; Byte b[4]; }
			w0 = { State.w[0] },
			w1 = { State.w[1] },
			w2 = { State.w[2] },
			w3 = { State.w[3] };

		State.w[0] = Key->w[0]
			^ T[0][w0.b[0]] ^ T[1][w1.b[1]] ^ T[2][w2.b[2]] ^ T[3][w3.b[3]];
		State.w[1] = Key->w[1]
			^ T[0][w1.b[0]] ^ T[1][w2.b[1]] ^ T[2][w3.b[2]] ^ T[3][w0.b[3]];
		State.w[2] = Key->w[2]
			^ T[0][w2.b[0]] ^ T[1][w3.b[1]] ^ T[2][w0.b[2]] ^ T[3][w1.b[3]];
		State.w[3] = Key->w[3]
			^ T[0][w3.b[0]] ^ T[1][w0.b[1]] ^ T[2][w1.b[2]] ^ T[3][w2.b[3]];
	}

	* (AESData *) Ciphertext = State;
}


// This is the main decryption routine.
static void AESDecryptWithExpandedKey(Byte *Plaintext, const Byte *Ciphertext,
	const AESData *ExpandedKey, long Nr)
{
	AESData State;
	
	XorBlock(&State, (const AESData *) Ciphertext, &ExpandedKey[Nr]);

	{
		const Word (*T)[256] = AESDecryptTable;

		for (int round = Nr-1; 0 < round; --round)
		{
			const AESData *Key = &ExpandedKey[round];

			const union { Word w; Byte b[4]; }
				w0 = { State.w[0] },
				w1 = { State.w[1] },
				w2 = { State.w[2] },
				w3 = { State.w[3] };

			State.w[0] = Key->w[0]
				^ T[0][w0.b[0]] ^ T[1][w3.b[1]] ^ T[2][w2.b[2]] ^ T[3][w1.b[3]];
			State.w[1] = Key->w[1]
				^ T[0][w1.b[0]] ^ T[1][w0.b[1]] ^ T[2][w3.b[2]] ^ T[3][w2.b[3]];
			State.w[2] = Key->w[2]
				^ T[0][w2.b[0]] ^ T[1][w1.b[1]] ^ T[2][w0.b[2]] ^ T[3][w3.b[3]];
			State.w[3] = Key->w[3]
				^ T[0][w3.b[0]] ^ T[1][w2.b[1]] ^ T[2][w1.b[2]] ^ T[3][w0.b[3]];
		}
	}

	{
		const Word (*T)[256] = AESInvSubBytesWordTable;

		const AESData *Key = &ExpandedKey[0];

		const union { Word w; Byte b[4]; }
			w0 = { State.w[0] },
			w1 = { State.w[1] },
			w2 = { State.w[2] },
			w3 = { State.w[3] };

		State.w[0] = Key->w[0]
			^ T[0][w0.b[0]] ^ T[1][w3.b[1]] ^ T[2][w2.b[2]] ^ T[3][w1.b[3]];
		State.w[1] = Key->w[1]
			^ T[0][w1.b[0]] ^ T[1][w0.b[1]] ^ T[2][w3.b[2]] ^ T[3][w2.b[3]];
		State.w[2] = Key->w[2]
			^ T[0][w2.b[0]] ^ T[1][w1.b[1]] ^ T[2][w0.b[2]] ^ T[3][w3.b[3]];
		State.w[3] = Key->w[3]
			^ T[0][w3.b[0]] ^ T[1][w2.b[1]] ^ T[2][w1.b[2]] ^ T[3][w0.b[3]];
	}

	* (AESData *) Plaintext = State;
}


#else	// defined UseAESedp_GeneralC

	// Declare routines implemented elsewhere.
	void AESExpandKeyForEncryption(Word *ExpandedKey, const AESKey *Key,
		long Nk);
	void AESExpandKeyForDecryption(Word *ExpandedKey, const AESKey *Key,
		long Nk);
	void AESEncryptWithExpandedKey(Byte *OutputText, const Byte *InputText,
		const AESData *ExpandedKey, long Nr);
	void AESDecryptWithExpandedKey(Byte *OutputText, const Byte *InputText,
		const AESData *ExpandedKey, long Nr);
	void AESEncryptCBC(void *Output, const void *Input,
		void *ChainBuffer, void *Key, long Blocks, long Rounds);
	void AESDecryptCBC(void *Output, const void *Input,
		void *ChainBuffer, void *Key, long Blocks, long Rounds);

#endif	// defined UseAESedp_GeneralC


/*	Expand a key and store the expansion in the cryptor context.

	CommonCrypto calls this routine.
*/
int aes_cc_set_key(
		aes_cc_ctx *Context,	// Cryptor context.
		const void *Key,		// Key.
		aes_32t KeyLength,		// Number of bytes in key.
		int ForEncryption		// True for encryption, false for decryption.
	)
{
	// Test for invalid key length.
	if (KeyLength != 16 && KeyLength != 24 && KeyLength != 32)
		return -1;

	// Set Nk to number of four-byte words in key.
	const int Nk = KeyLength / 4;

	// Remember the number of rounds.
	Context->encrypt.rn = Nk + 6;

	#if defined UseAESedp_GeneralC

		AESExpandKey(Context->encrypt.ks, Key, Nk);

		if (!ForEncryption)
		{

			/*	Change the expanded key so we can swap the InvMixColumns and
				XorBlock operations during decryption.
			*/
			Word *E = Context->encrypt.ks;
			int Nr = Context->encrypt.rn;
			for (int i = Nb; i < Nr * Nb; ++i)
				E[i] = InvMixColumn(E[i]);
		}

	#else	// defined UseAESedp_GeneralC

		if (ForEncryption)
			AESExpandKeyForEncryption(Context->encrypt.ks, Key, Nk);
		else
			AESExpandKeyForDecryption(Context->encrypt.ks, Key, Nk);

	#endif	// defined UseAESedp_GeneralC

	// Indicate there is no initial value stored.
	Context->encrypt.cbcEnable = 0;

	return 0;
}


#include <string.h>	// For memcpy.


/*	Store an initial value (or lack thereof) in the cryptor context.

	CommonCrypto calls this routine.
*/
void aes_cc_set_iv(aes_cc_ctx *Context, int ForEncryption, const void *IV)
{
	if (IV == 0)
		// Indicate there is no initial value stored.
		Context->encrypt.cbcEnable = 0;
	else
	{
		// Indicate there is an initial value stored.
		Context->encrypt.cbcEnable = 1;
		memcpy(Context->encrypt.chainBuf, IV, sizeof Context->encrypt.chainBuf);
	}
}


/*	Encrypt blocks of data.

	CommonCrypto calls this routine.
*/
void aes_cc_encrypt(
		aes_cc_ctx *Context,	// Cryptor context.
		const void *Input,		// Input.
		aes_32t Blocks,			// Number of 16-byte blocks to process.
		void *Output			// Output.
	)
{
	// Alias to more convenient pointers for referring to blocks.
	const AESData *I = Input;
	      AESData *O = Output;

	// If we have an initial value, use Cipher Block Chaining (CBC) mode.
	if (Context->encrypt.cbcEnable)
	{
		#if defined UseAESedp_GeneralC
			// Get chain value.
			AESData State = * (AESData *) Context->encrypt.chainBuf;

			// Chain and encrypt.
			while (Blocks--)
			{
				XorBlock(&State, &State, I++);
				AESEncryptWithExpandedKey(State.b, State.b,
					(const void *) Context->encrypt.ks,
					Context->encrypt.rn);
				*O++ = State;
			}

			// Save updated chain value.
			* (AESData *) Context->encrypt.chainBuf = State;
		#else	// defined UseAESedp_GeneralC
			AESEncryptCBC(O, I, Context->encrypt.chainBuf,
				Context->encrypt.ks, Blocks, Context->encrypt.rn);
		#endif	// defined UseAESedp_GeneralC
	}

	// If we have no initial value, use Electronic Code Book (ECB) mode.
	else
	{
		aes_32t i;
		for (i = 0; i < Blocks; ++i)
			AESEncryptWithExpandedKey(O[i].b, I[i].b,
				(const void *) Context->encrypt.ks, Context->encrypt.rn);
	}
}


/*	Alternate interface to encryption, same as aes_cc_encrypt, except that if
	InitialValue is non-null, it points to an initial value which is used for
	CBC mode, regardless of the cbcEnable flag and the initial/chain value in
	the context structure.  The updated chain value is written to written to
	the context structure.
*/
aes_rval aes_encrypt_cbc(
	const unsigned char *Input,
	const unsigned char *InitialValue,
	unsigned int Blocks,
	unsigned char *Output,
	aes_encrypt_ctx *Context)
{
	// Alias to more convenient pointers for referring to blocks.
	const AESData *I = (const AESData *) Input;
	      AESData *O = (      AESData *) Output;

	// If we have an initial value, use Cipher Block Chaining (CBC) mode.
	if (Context->cbcEnable || InitialValue)
	{
		#if defined UseAESedp_GeneralC
			// Get chain value.
			AESData State = InitialValue
				? * (const AESData *) InitialValue
				: * (const AESData *) Context->chainBuf;

			// Chain and encrypt.
			while (Blocks--)
			{
				XorBlock(&State, &State, I++);
				AESEncryptWithExpandedKey(State.b, State.b,
					(const void *) Context->ks,
					Context->rn);
				*O++ = State;
			}

			// Save updated chain value.
			* (AESData *) Context->chainBuf = State;
		#else	// defined UseAESedp_GeneralC
			AESEncryptCBC(O, I, Context->chainBuf,
				Context->ks, Blocks, Context->rn);
		#endif	// defined UseAESedp_GeneralC
	}

	// If we have no initial value, use Electronic Code Book (ECB) mode.
	else
	{
		aes_32t i;
		for (i = 0; i < Blocks; ++i)
			AESEncryptWithExpandedKey(O[i].b, I[i].b,
				(const void *) Context->ks, Context->rn);
	}
}


/*	Decrypt blocks of data.

	CommonCrypto calls this routine.
*/
void aes_cc_decrypt(
		aes_cc_ctx *Context,	// Cryptor context.
		const void *Input,		// Input.
		aes_32t Blocks,			// Number of 16-byte blocks to process.
		void *Output			// Output.
	)
{
	// Alias to more convenient pointers for referring to blocks.
	const AESData *I = Input;
	      AESData *O = Output;

	// If we have an initial value, use Cipher Block Chaining (CBC) mode.
	if (Context->encrypt.cbcEnable)
	{
		#if defined UseAESedp_GeneralC
			// Get chain value.
			AESData NextChainValue = * (AESData *) Context->encrypt.chainBuf;

			// Decrypt and chain.
			while (Blocks--)
			{
				AESData ChainValue = NextChainValue, State;
				NextChainValue = *I++;
				AESDecryptWithExpandedKey(State.b, NextChainValue.b,
					(const void *) Context->encrypt.ks, Context->encrypt.rn);
				XorBlock(O++, &State, &ChainValue);
			}

			// Save updated chain value.
			* (AESData *) Context->encrypt.chainBuf = NextChainValue;
		#else	// defined UseAESedp_GeneralC
			AESDecryptCBC(O, I, Context->encrypt.chainBuf,
				Context->encrypt.ks, Blocks, Context->encrypt.rn);
		#endif	// defined UseAESedp_GeneralC
	}

	// If we have no initial value, use Electronic Code Book (ECB) mode.
	else
	{
		aes_32t i;
		for (i = 0; i < Blocks; ++i)
			AESDecryptWithExpandedKey(O[i].b, I[i].b,
				(const void *) Context->encrypt.ks, Context->encrypt.rn);
	}
}


/*	Alternate interface to decryption, same as aes_cc_decrypt, except that if
	InitialValue is non-null, it points to an initial value which is used for
	CBC mode, regardless of the cbcEnable flag and the initial/chain value in
	the context structure.  The updated chain value is written to written to
	the context structure.
*/
aes_rval aes_decrypt_cbc(
	const unsigned char *Input,
	const unsigned char *InitialValue,
	unsigned int Blocks,
	unsigned char *Output,
	aes_decrypt_ctx *Context)
{
	// Alias to more convenient pointers for referring to blocks.
	const AESData *I = (const AESData *) Input;
	      AESData *O = (      AESData *) Output;

	// If we have an initial value, use Cipher Block Chaining (CBC) mode.
	if (Context->cbcEnable || InitialValue)
	{
		#if defined UseAESedp_GeneralC
			// Get chain value.
			AESData NextChainValue = InitialValue
				? * (const AESData *) InitialValue
				: * (const AESData *) Context->chainBuf;

			// Decrypt and chain.
			while (Blocks--)
			{
				AESData ChainValue = NextChainValue, State;
				NextChainValue = *I++;
				AESDecryptWithExpandedKey(State.b, NextChainValue.b,
					(const void *) Context->ks, Context->rn);
				XorBlock(O++, &State, &ChainValue);
			}

			// Save updated chain value.
			* (AESData *) Context->chainBuf = NextChainValue;
		#else	// defined UseAESedp_GeneralC
			AESDecryptCBC(O, I, Context->chainBuf,
				Context->ks, Blocks, Context->rn);
		#endif	// defined UseAESedp_GeneralC
	}

	// If we have no initial value, use Electronic Code Book (ECB) mode.
	else
	{
		aes_32t i;
		for (i = 0; i < Blocks; ++i)
			AESDecryptWithExpandedKey(O[i].b, I[i].b,
				(const void *) Context->ks, Context->rn);
	}
}



#endif	// defined UseAESedp
