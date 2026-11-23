// Native test: compile with the MD5 code and run on the host.
//   cc -I src tests/MD5Test.c src/MD5.c -o MD5Test
// Also run it with -fsanitize=address,undefined.
// The vectors are the test suite of RFC 1321, appendix A.5 (https://www.rfc-editor.org/rfc/rfc1321), plus digests
// of 'x' runs around the padding boundaries and of one million 'a' characters, taken from the host's md5sum.
#include "MD5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

static void hexOf(const uint8_t digest[MD5_DIGEST_SIZE], char out[33])
{
	for (int i = 0; i < MD5_DIGEST_SIZE; i++)
		snprintf(out + i * 2, 3, "%02x", digest[i]);
}

static void expectDigest(const char* name, const void* bytes, size_t length, const char* expected)
{
	MD5Context context;
	uint8_t digest[MD5_DIGEST_SIZE];
	char hex[33];
	MD5ContextInit(&context);
	MD5ContextUpdate(&context, bytes, length);
	MD5ContextFinal(&context, digest);
	hexOf(digest, hex);
	if (strcmp(hex, expected) != 0)
	{
		printf("FAIL %s: got %s, expected %s\n", name, hex, expected);
		g_failures++;
	}
}

static void expectText(const char* text, const char* expected)
{
	expectDigest(text, text, strlen(text), expected);
}

int main(void)
{
	expectText("", "d41d8cd98f00b204e9800998ecf8427e");
	expectText("a", "0cc175b9c0f1b6a831c399e269772661");
	expectText("abc", "900150983cd24fb0d6963f7d28e17f72");
	expectText("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
	expectText("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
	expectText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f");
	expectText("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a");

	// Lengths around the padding boundaries: 55 bytes leave room for the length, 56 do not.
	char text[80];
	memset(text, 'x', sizeof text);
	expectDigest("55 bytes", text, 55, "04364420e25c512fd958a70738aa8f72");
	expectDigest("56 bytes", text, 56, "668a72d5ba17f08e62dabcafad6db14b");
	expectDigest("63 bytes", text, 63, "7dc2ca208106a2f703567bdff99d8981");
	expectDigest("64 bytes", text, 64, "c1bb4f81d892b2d57947682aeb252456");
	expectDigest("65 bytes", text, 65, "1bc932052302d074bdec39795fe00cf6");

	// The same input in pieces of every size gives the same digest as in one piece.
	uint8_t million[1000000];
	memset(million, 'a', sizeof million);
	expectDigest("one million a, one piece", million, sizeof million, "7707d6ae4e027c70eea2a935c2296f21");
	for (size_t piece = 1; piece <= 130; piece += 7)
	{
		MD5Context context;
		uint8_t digest[MD5_DIGEST_SIZE];
		char hex[33];
		MD5ContextInit(&context);
		for (size_t done = 0; done < sizeof million; done += piece)
			MD5ContextUpdate(&context, million + done, sizeof million - done < piece ? sizeof million - done : piece);
		MD5ContextFinal(&context, digest);
		hexOf(digest, hex);
		if (strcmp(hex, "7707d6ae4e027c70eea2a935c2296f21") != 0)
		{
			printf("FAIL pieces of %zu: got %s\n", piece, hex);
			g_failures++;
		}
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
