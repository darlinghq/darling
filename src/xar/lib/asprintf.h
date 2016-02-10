//==============================================================================
//
// Copyright (C) 2005 Jason Evans <jasone@canonware.com>.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice(s),
//    this list of conditions and the following disclaimer unmodified other than
//    the allowable addition of one or more copyright notices.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice(s), this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) `AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
// NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//==============================================================================
//
// Emulate vasprintf() and asprintf().
//
//==============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static inline int
vasprintf(char **rResult, const char *aFormat, va_list aAp)
{
    int rVal;
    char *result;
    va_list ap;
#define XarAsprintfStartLen 16

    result = (char *) malloc(XarAsprintfStartLen);
    if (result == NULL)
    {
	rVal = -1;
	goto RETURN;
    }

    va_copy(ap, aAp);
    rVal = vsnprintf(result, XarAsprintfStartLen, aFormat, ap);
    va_end(ap);

    if (rVal == -1)
    {
	goto RETURN;
    }
    else if (rVal >= XarAsprintfStartLen)
    {
	free(result);
	result = (char *) malloc(rVal + 1);
	if (result == NULL)
	{
	    rVal = -1;
	    goto RETURN;
	}

	va_copy(ap, aAp);
	rVal = vsnprintf(result, rVal + 1, aFormat, aAp);
	va_end(ap);
    }

    *rResult = result;
    RETURN:
#undef XarAsprintfStartLen
    return rVal;
}

static int
asprintf(char **rResult, const char *aFormat, ...)
{
    int rVal;
    va_list ap;

    va_start(ap, aFormat);
    rVal = vasprintf(rResult, aFormat, ap);
    va_end(ap);

    return rVal;
}
