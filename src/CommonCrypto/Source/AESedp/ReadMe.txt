This directory contains an implementation of AES intended to plug into the
CommonCrypto interface.  This main directory contains a C implementation which
is not particularly fast but which works and illustrates the techniques used.
The Intel subdirectory contains routines providing a fast implementation for
the i386 (IA-32) and x86_64 (EMT64) architectures.  The Intel routines replace
certain routines in AES.c when UseAESedp_IntelAssembly is defined in AES.h and
UseAESedp_GeneralC is not defined.

Below is a summary of some of the arithmetic used in AES.  This is not an
introduction to AES, just a note about why we use bitwise XOR for "addition" of
AES elements and various expressions for "multiplication."

AES defines a 256-element Galois field over the integers modulo 2 modulo the
polynomial p(x) = x**8 + x**4 + x**3 + x + 1.  This means:

	Only the residues modulo 2 of the coefficients are relevant, so each
	coefficient is effectively either 0 or 1.  1+1 yields zero.

	Only the residues modulo p(x) of the polynomials are relevant, so each
	polynomial is effectively a degree-seven or less polynomial.  (Any result,
	say from multiplication, that yields a polynomial with a term x**8 or
	greater is reduced by dividing it by p(x).)

	Each element of the field is a polynomial with eight coefficients (for
	each power of x from x**7 to x**0), and each coefficients is a single bit.
	So we can represent an element in an eight-bit byte.

	XORing two bytes is the same as adding two polynomials.

	To multiply a polynomial by x, shift the byte left one bit.  If the x**8
	bit is on, then subtract p(x), which is represented by 0x11b.  (No more
	than one subtraction is needed, because the x**8 coefficient is at most
	one.)  Equivalently:

		unsigned char MultiplyByX(unsigned char b)
		{ return b << 1 ^ (b & 0x80 ? 0x1b : 0); }

	Two polynomials can be multiplied by using the above operations to
	multiply by powers of x and add appropriate powers.

AES defines another field with polynomials whose coefficients are elements in
the previous Galois field.  This larger field has a characteristic polynomial
of x**4 + 1.  This means:

	Elements in this field have four coefficients, each of which can be
	represented by a byte.

	Elements are added by adding their coefficients, which are adding by XORing
	their byte representations.  So an XOR of two four-byte words containing
	the representations of two elements is the sum of the two elements.

	Because the characteristic polynomial is x**4 + 1 and b == -b in this
	field, multiplying a polynomial by x effectively rotates the four bytes
	that represent it left one byte.
