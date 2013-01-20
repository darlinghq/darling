#ifndef TEXTUTILS_H
#define TEXTUTILS_H
#include "MacTypes.h"

extern "C"
{

// There are many functions missing, but will be added as-needed.
// There is no point in spending time implementing all this ancient stuff.

StringHandle NewString(const Str255 str);
void SetString(StringHandle handle, const Str255 str);

void UpperString(Str255 str, Boolean diac);
void upperstring(char* str, Boolean diac);

void c2pstrcpy(Str255 dst, const char* src);
void p2cstrcpy(char* dst, const Str255 src);
void CopyPascalStringToC(const Str255 src, char* dst);
void CopyCStringToPascal(const char* src, Str255 dst);
StringPtr c2pstr(char* str);
StringPtr C2PStr(Ptr str);
char* p2cstr(StringPtr str);
Ptr P2CStr(StringPtr str);

}

#endif
