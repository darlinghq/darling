/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <CarbonCore/TextUtils.h>
#include <CarbonCore/NumberFormatting.h>
#include <stdint.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#if !__LP64__
// Signed decimal Pascal string, no terminator; LONG_MIN is formatted through its magnitude.
void NumToString(long theNum, Str255 theString)
{
	if (!theString)
		return;

	unsigned long magnitude = (unsigned long) theNum;
	const bool negative = theNum < 0;
	if (negative)
		magnitude = 0ul - magnitude;

	unsigned char digits[3 * sizeof(long) + 1];
	unsigned count = 0;
	do
	{
		digits[count++] = (unsigned char) ('0' + magnitude % 10);
		magnitude /= 10;
	}
	while (magnitude);

	unsigned out = 1;
	theString[0] = (unsigned char) (count + negative);
	if (negative)
		theString[out++] = '-';
	while (count)
		theString[out++] = digits[--count];
}

// Roman rules: an optional leading sign, then each character contributes its low four bits as a
// digit, with no validation; the value wraps modulo 2^32.
void StringToNum(ConstStr255Param theString, long* theNum)
{
	if (!theNum)
		return;
	if (!theString)
	{
		*theNum = 0;
		return;
	}

	uint32_t value = 0;
	unsigned i = 1;
	bool negative = false;
	if (theString[0] && (theString[1] == '-' || theString[1] == '+'))
	{
		negative = theString[1] == '-';
		i = 2;
	}
	for (; i <= theString[0]; i++)
		value = value * 10u + (theString[i] & 15u);
	if (negative)
		value = 0u - value;

	int32_t result;
	std::memcpy(&result, &value, sizeof(result));
	*theNum = (long) result;
}
#endif

StringHandle NewString(const Str255 str)
{
	StringHandle handle = (StringHandle) malloc(sizeof(StringPtr));
	*handle = nullptr;
	
	SetString(handle, str);
	
	return handle;
}

void SetString(StringHandle handle, const Str255 str)
{
	free(*handle);
	
	*handle = (StringPtr) malloc(str[0]);
	memcpy(*handle, &str[1], str[0]);
}

void UpperString(Str255 str, Boolean diac)
{
	for (int i = 0; i < str[0]; i++)
		str[i+1] = toupper(str[i+1]);
}

void upperstring(char* str, Boolean diac)
{
	while (*str)
	{
		*str = toupper(*str);
		str++;
	}
}

void c2pstrcpy(Str255 dst, const char* src)
{
	size_t len = std::min<size_t>(strlen(src), 255);
	dst[0] = len;
	memcpy(&dst[1], src, len);
}

void p2cstrcpy(char* dst, const Str255 src)
{
	size_t len = src[0];
	memcpy(dst, &src[1], len);
	dst[len] = 0;
}

void CopyPascalStringToC(const Str255 src, char* dst)
{
	p2cstrcpy(dst, src);
}

void CopyCStringToPascal(const char* src, Str255 dst)
{
	c2pstrcpy(dst, src);
}

StringPtr c2pstr(char* str)
{
	size_t len = strlen(str);
	StringPtr ptr = (StringPtr) malloc(len+1);
	
	ptr[0] = len;
	memcpy(ptr+1, str, len);
	return ptr;
}

StringPtr C2PStr(Ptr str)
{
	return c2pstr(str);
}

char* p2cstr(StringPtr str)
{
	size_t len = str[0];
	char* ptr = (char*) malloc(len+1);
	
	memcpy(ptr, &str[1], len);
	ptr[len] = 0;
	
	return ptr;
}

Ptr P2CStr(StringPtr str)
{
	return p2cstr(str);
}

// QUIRK: each byte of the type that is not printable ASCII (0x20...0x7f) is stored as a space,
// so the result is always four characters long.
// Not documented. Evidence: black-box observation on macOS 27.0.1, arm64 and x86_64 (tier 2),
// vd-upstream/mac-probes/v124-ostype-probe.c.
// Test: test_ostype_to_cstring in tests/src/private_spi_stubs.c
void OSTypeToCString(OSType theType, char* theString)
{
	for (int i = 0; i < 4; i++)
	{
		unsigned char byte = (theType >> (24 - 8 * i)) & 0xFF;
		theString[i] = (byte >= 0x20 && byte < 0x80) ? byte : ' ';
	}
	theString[4] = '\0';
}
