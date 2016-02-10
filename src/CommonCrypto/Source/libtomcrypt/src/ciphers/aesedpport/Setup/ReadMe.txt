This directory contains a hybrid AES implementation.  The core AES routines
(the actual encryption, decryption, and key expansion) are in:

	AES.s
	Data.mk
	Data.s
	EncryptDecrypt.s
	ExpandKeyForDecryption.s
	ExpandKeyForEncryption.s
	MakeData.c

Although the above files do not explicitly include aes.h, they confirm to
certain things defined in it, notably the aes_rval type and the layout of the
aes_encrypt_ctx and aes_decrypt_ctx structures.  These must be kept
compatibility; the definitions of ContextKey and ContextKeyLength in AES.s must
match the offsets of the key ("ks") and key_length ("inf") members of
aes_encrypt_ctx and aes_decrypt_ctx.  (For some reason, aes_inf is a union that
is written as a 32-bit integer and read as an 8-bit integer.  I do not know
why but have reproduced that behavior in the new implementation.)

aes_modes.c extends the API, most notably by implementing CBC mode using the
basic AES block encryption.  It uses aesopt.h and edefs.h.
