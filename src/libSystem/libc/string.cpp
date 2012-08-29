#include "string.h"
#include <cstdlib>
#include <ctype.h>

// The following three functions are slow.
// But they're no faster on Darwin either.

void memset_pattern4(char* b, const char* pattern4, size_t len)
{
	for (size_t i = 0; i < len; i++)
		b[i] = pattern4[i % 4];
}

void memset_pattern8(char* b, const char* pattern4, size_t len)
{
	for (size_t i = 0; i < len; i++)
		b[i] = pattern4[i % 8];
}

void memset_pattern16(char* b, const char* pattern4, size_t len)
{
	for (size_t i = 0; i < len; i++)
		b[i] = pattern4[i % 16];
}

int ___mb_cur_max()
{
	return MB_CUR_MAX;
}

int __toupper(int c)
{
	return toupper(c);
}

int __tolower(int c)
{
	return tolower(c);
}

