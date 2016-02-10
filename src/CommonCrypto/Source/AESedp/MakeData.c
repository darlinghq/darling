#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	UseAESedp	// Tell AES.h to define things for AESedp.
#include "AES.h"


/*	In comments below, {n} designates the Galois field element represented by
	the byte n.  See notes about Galois field multiplication in ReadMe.txt.

	So 3+5 is addition of ordinary integers, and 3+5 == 8, while {3}+{5} is
	addition in the field, and {3} + {5} = {3 XOR 5} = {6}.)
*/


// Define constants for languages.
typedef enum { C, IntelAssembly } Language;


/*	LogBase3[i] will contain the base-three logarithm of i in the 256-element
	Galois field defined by AES.  That is, {3}**LogBase3[i] == {3}**i.
*/
static Byte LogBase3[256];

/*	AntilogBase3[i] will contain {3}**i in the 256-element Galois field defined
	by AES.  It contains extra elements so that the antilog of a+b can be found
	by looking up a+b directly, without having to reduce modulo the period, for
	0 <= a, b < 255.

	(254 is the greatest value we encounter.  Each a or b we use is the
	base-three logarithm of some element.  As a primitive root, the powers of
	three cycle through all non-zero elements of the field, of which there are
	255, so the exponents cover 0 to 254 before the powers repeat.)
*/
static Byte AntilogBase3[254+254+1];


static void InitializeLogTables(void)
{
	// log({1}) is zero, so start {p} (power) at {1} and l (logarithm) at 0.
	Byte p = 1;
	int l = 0;
	do
	{
		// Record table entries.
		LogBase3[p] = l;
		AntilogBase3[l] = p;

		/*	Observe that {2}*{p} is {p << 1 ^ (a & 0x80 ? 0x1b : 0)}, per notes
			in ReadMe.txt.  We produce {3}*{p}:

				{3}*{p}
					= {1}*{p} + {2}*{p}
					= {1}*{p} + {p << 1 ^ (a & 0x80 ? 0x1b : 0)}
					= {p ^ p << 1 ^ (p & 0x80 ? 0x1b : 0)}.
		*/
		p ^= p << 1 ^ (p & 0x80 ? 0x1b : 0);
		++l;

	} while (p != 1);	// Stop when we have gone around completely.

	/*	The antilogarithms are periodic with a period of 255, and we want to
		look up elements as high as 254+254 (the largest that a sum of two
		logarithms could be), so we replicate the table beyond the first
		period.
	*/
	for (l = 255; l < 254+254; ++l)
		AntilogBase3[l] = AntilogBase3[l-255];
}


/*	MultiplyByte(Byte b, Byte c) returns {b}*{c}.  It requires tables that must
	be initialized before this routine is used.
*/
static Byte MultiplyByte(Byte b, Byte c)
{
	// Calculate product by adding logarithms, but avoid logarithms of zero.
	return b == 0 || c == 0 ? 0 : AntilogBase3[LogBase3[b] + LogBase3[c]];
}


// Return {0} if {b} is {0} and the multiplicative inverse of {b} otherwise.
static Byte InverseByte(Byte b)
{
	return b == 0 ? 0 : AntilogBase3[255 - LogBase3[b]];
}


// Perform AES' SubBytes operation on a single byte.
static Byte SubByte(Byte b)
{
	unsigned int r = InverseByte(b);

	// Duplicate r as a proxy for a rotate operation.
	r = r | r<<8;

	// Apply the standard's affine transformation.
	return r ^ r>>4 ^ r>>5 ^ r>>6 ^ r>>7 ^ 0x63;
}


// Define and populate tables for the SubBytes and InvSubBytes operations.
static Byte SubBytesTable[256];
static Byte InvSubBytesTable[256];


static void InitializeSubBytesTable(void)
{
	for (int i = 0; i < 256; ++i)
		SubBytesTable[i] = SubByte((Byte) i);
}


static void InitializeInvSubBytesTable(void)
{
	for (int i = 0; i < 256; ++i)
		InvSubBytesTable[SubByte((Byte) i)] = i;
}


/*	Print tables for SubBytes function providing the output byte embedded in
	various places in a word, so that the table entries can be used with
	fewer byte manipulations.
*/
static void PrintSubBytesWordTable(Language language)
{
	switch (language)
	{
		case C:
			printf("\n\n"
				"// SubBytes embedded in words tables.\n"
				"const Word AESSubBytesWordTable[4][256] =\n"
				"{\n");
			for (int j = 0; j < 4; ++j)
			{
				printf("\t{\n");
				for (int i = 0; i < 256; ++i)
					printf("\t\t0x%08x,\n", SubBytesTable[i] << j*8);
				printf("\t},\n");
			}
			printf("};\n");
			break;

		case IntelAssembly:
			printf("\n\n"
				"// SubBytes embedded in words tables.\n"
				"\t.globl\t_AESSubBytesWordTable\n"
				"\t.private_extern\t_AESSubBytesWordTable\n"
				"\t.align\t2\n"
				"_AESSubBytesWordTable:\n");
			for (int j = 0; j < 4; ++j)
			{
				printf("\t// Table %d.\n", j);
				for (int i = 0; i < 256; ++i)
					printf("\t.long\t0x%08x\n", SubBytesTable[i] << j*8);
			}
			break;
	}
}


/*	Print tables for InvSubBytes function providing the output byte embedded in
	various places in a word, so that the table entries can be used with
	fewer byte manipulations.
*/
static void PrintInvSubBytesWordTable(Language language)
{
	switch (language)
	{
		case C:
			printf("\n\n"
				"// InvSubBytes embedded in words tables.\n"
				"const Word AESInvSubBytesWordTable[4][256] =\n"
				"{\n");
			for (int j = 0; j < 4; ++j)
			{
				printf("\t{\n");
				for (int i = 0; i < 256; ++i)
					printf("\t\t0x%08x,\n", InvSubBytesTable[i] << j*8);
				printf("\t},\n");
			}
			printf("};\n");
			break;

		case IntelAssembly:
			printf("\n\n"
				"// InvSubBytes embedded in words tables.\n"
				"\t.globl\t_AESInvSubBytesWordTable\n"
				"\t.private_extern\t_AESInvSubBytesWordTable\n"
				"\t.align\t2\n"
				"_AESInvSubBytesWordTable:\n");
			for (int j = 0; j < 4; ++j)
			{
				printf("\t// Table %d.\n", j);
				for (int i = 0; i < 256; ++i)
					printf("\t.long\t0x%08x\n", InvSubBytesTable[i] << j*8);
			}
			break;
	}
}


// Print the round constants.
static void PrintRcon(Language language)
{
	union { Byte c[4]; Word w; } t = { { 1, 0, 0, 0 } };

	switch (language)
	{
		case C:
			printf("\n\n"
				"// Round constants.\n"
				"const Byte AESRcon[] =\n"
				"{\n"
				"\t0,\t// Not used, included for indexing simplicity.\n");
			for (int i = 1; i < MaxRcon; ++i)
			{
				printf("\t0x%02x,\n", t.w);
				t.c[0] = MultiplyByte(0x2, t.c[0]);
			}
			printf("};\n");
			break;

		case IntelAssembly:
			printf("\n\n"
				"// Round constants.\n"
				"\t.globl\t_AESRcon\n"
				"\t.private_extern\t_AESRcon\n"
				"_AESRcon:\n"
				"\t.byte\t0\t// Not used, included for indexing simplicity.\n");
			for (int i = 1; i < MaxRcon; ++i)
			{
				printf("\t.byte\t0x%02x\n", t.w);
				t.c[0] = MultiplyByte(0x2, t.c[0]);
			}
			break;
	}
}


// Print tables for the InvMixColumn operation.
static void PrintInvMixColumnTable(Language language)
{
	Word T[4][256];

	for (int i = 0; i < 256; ++i)
	{
		union { Byte b[4]; Word w; } c;

		Byte s9 = MultiplyByte(0x9, i);
		Byte sb = MultiplyByte(0xb, i);
		Byte sd = MultiplyByte(0xd, i);
		Byte se = MultiplyByte(0xe, i);

		c.b[0] = se;
		c.b[1] = s9;
		c.b[2] = sd;
		c.b[3] = sb;
		T[0][i] = c.w;

		c.b[0] = sb;
		c.b[1] = se;
		c.b[2] = s9;
		c.b[3] = sd;
		T[1][i] = c.w;

		c.b[0] = sd;
		c.b[1] = sb;
		c.b[2] = se;
		c.b[3] = s9;
		T[2][i] = c.w;

		c.b[0] = s9;
		c.b[1] = sd;
		c.b[2] = sb;
		c.b[3] = se;
		T[3][i] = c.w;
	}

	switch (language)
	{
		case C:
			printf("\n\n"
				"// Tables for InvMixColumn.\n"
				"const Word AESInvMixColumnTable[4][256] =\n"
				"{\n");
			for (int i = 0; i < 4; ++i)
			{
				printf("\t{\n");
				for (int j = 0; j < 256; ++j)
					printf("\t\t0x%08x,\n", T[i][j]);
				printf("\t},\n");
			}
			printf("};\n");
			break;

		case IntelAssembly:
			printf("\n\n"
				"// Tables for InvMixColumn.\n"
				"\t.globl\t_AESInvMixColumnTable\n"
				"\t.private_extern\t_AESInvMixColumnTable\n"
				"\t.align\t2\n"
				"_AESInvMixColumnTable:\n");
			for (int i = 0; i < 4; ++i)
			{
				printf("\t// Table %d.\n", i);
				for (int j = 0; j < 256; ++j)
					printf("\t.long\t0x%08x\n", T[i][j]);
			}
			break;
	}
}


/*	Print the tables defined AES Proposal: Rijndael, amended, 9/04/2003,
	section 5.2.1.  These combine the MixColumn and SubBytes operations.
*/
static void PrintEncryptTable(Language language)
{
	Word T[4][256];

	for (int i = 0; i < 256; ++i)
	{
		union { Byte b[4]; Word w; } c;

		Byte s1 = SubBytesTable[i];
		Byte s2 = MultiplyByte(0x2, s1);
		Byte s3 = s1 ^ s2;

		c.b[0] = s2;
		c.b[1] = s1;
		c.b[2] = s1;
		c.b[3] = s3;
		T[0][i] = c.w;

		c.b[0] = s3;
		c.b[1] = s2;
		//c.b[2] = s1;
		c.b[3] = s1;
		T[1][i] = c.w;

		c.b[0] = s1;
		c.b[1] = s3;
		c.b[2] = s2;
		//c.b[3] = s1;
		T[2][i] = c.w;

		//c.b[0] = s1;
		c.b[1] = s1;
		c.b[2] = s3;
		c.b[3] = s2;
		T[3][i] = c.w;
	}

	switch (language)
	{
		case C:
			printf("\n\n"
				"// Tables for main encryption iterations.\n"
				"const Word AESEncryptTable[4][256] =\n"
				"{\n");
			for (int i = 0; i < 4; ++i)
			{
				printf("\t{\n");
				for (int j = 0; j < 256; ++j)
					printf("\t\t0x%08x,\n", T[i][j]);
				printf("\t},\n");
			}
			printf("};\n");
			break;

		case IntelAssembly:
			printf("\n\n"
				"// Tables for main encryption iterations.\n"
				"\t.globl\t_AESEncryptTable\n"
				"\t.private_extern\t_AESEncryptTable\n"
				"\t.align\t2\n"
				"_AESEncryptTable:\n");
			for (int i = 0; i < 4; ++i)
			{
				printf("\t// Table %d.\n", i);
				for (int j = 0; j < 256; ++j)
					printf("\t.long\t0x%08x\n", T[i][j]);
			}
			break;
	}
}


/*	Print the inverse tables.  These correspond to the tables above, but for
	decyrption.  These combine the InvSubBytes and InvMixColumn operations.
*/
static void PrintDecryptTable(Language language)
{
	Word T[4][256];

	for (int i = 0; i < 256; ++i)
	{
		union { Byte b[4]; Word w; } c;

		Byte si = InvSubBytesTable[i];

		Byte s9 = MultiplyByte(0x9, si);
		Byte sb = MultiplyByte(0xb, si);
		Byte sd = MultiplyByte(0xd, si);
		Byte se = MultiplyByte(0xe, si);

		c.b[0] = se;
		c.b[1] = s9;
		c.b[2] = sd;
		c.b[3] = sb;
		T[0][i] = c.w;

		c.b[0] = sb;
		c.b[1] = se;
		c.b[2] = s9;
		c.b[3] = sd;
		T[1][i] = c.w;

		c.b[0] = sd;
		c.b[1] = sb;
		c.b[2] = se;
		c.b[3] = s9;
		T[2][i] = c.w;

		c.b[0] = s9;
		c.b[1] = sd;
		c.b[2] = sb;
		c.b[3] = se;
		T[3][i] = c.w;
	}

	switch (language)
	{
		case C:
			printf("\n\n"
				"// Tables for main decryption iterations.\n"
				"const Word AESDecryptTable[4][256] =\n"
				"{\n");
			for (int i = 0; i < 4; ++i)
			{
				printf("\t{\n");
				for (int j = 0; j < 256; ++j)
					printf("\t\t0x%08x,\n", T[i][j]);
				printf("\t},\n");
			}
			printf("};\n");
			break;

		case IntelAssembly:
			printf("\n\n"
				"// Tables for main decryption iterations.\n"
				"\t.globl\t_AESDecryptTable\n"
				"\t.private_extern\t_AESDecryptTable\n"
				"\t.align\t2\n"
				"_AESDecryptTable:\n");
			for (int i = 0; i < 4; ++i)
			{
				printf("\t// Table %d.\n", i);
				for (int j = 0; j < 256; ++j)
					printf("\t.long\t0x%08x\n", T[i][j]);
			}
			break;
	}
}


static void Usage(const char *ProgramName)
{
	fprintf(stderr,
		"%s:  This program must have exactly one argument, \"C\" to generate\n"
		"C or \"Intel\" to generate GCC i386/x86_64 assembly.\n", ProgramName);
	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
	if (argc != 2)
		Usage(argv[0]);

	Language language;

	// Figure out which language to generate, C or Intel assembly.
	if (0 == strcmp(argv[1], "C"))
		language = C;
	else if (0 == strcmp(argv[1], "Intel"))
		language = IntelAssembly;
	else
		Usage(argv[0]);

	printf("// This file was generated by " __FILE__ ".\n");

	if (language == C)
		printf("\n\n#include \"AES.h\"\n");

	if (language == IntelAssembly)
		printf("\n\n\t.const\n");

	InitializeLogTables();
	InitializeSubBytesTable();
	InitializeInvSubBytesTable();

	PrintRcon(language);
	PrintInvMixColumnTable(language);
	PrintEncryptTable(language);
	PrintDecryptTable(language);
	PrintSubBytesWordTable(language);
	PrintInvSubBytesWordTable(language);

	return 0;
}
