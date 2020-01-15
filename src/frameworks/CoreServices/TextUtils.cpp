#include "TextUtils.h"
#include <cstring>
#include <cstdlib>
#include <algorithm>

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

