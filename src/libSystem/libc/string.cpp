#include "string.h"
#include <cstdlib>
#include <ctype.h>

size_t strlcpy(char* dst, const char* src, size_t size)
{
	dst[size - 1] = '\0';
	strncpy(dst, src, size - 1);
	return strlen(dst);
}

size_t strlcat(char* dst, const char* src, size_t size)
{
	dst[size - 1] = '\0';
	strncat(dst, src, size - strlen(dst) - 1);
	return strlen(dst);
}

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

int __mb_cur_max()
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

