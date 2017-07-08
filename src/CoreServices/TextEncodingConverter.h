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

#ifndef _TEXTENCODINGCONVERTER_H
#define _TEXTENCODINGCONVERTER_H
#include <MacTypes.h>
#include "TextCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueTECObjectRef* TECObjectRef;

OSStatus TECCreateConverter(TECObjectRef *newEncodingConverter, TextEncoding inputEncoding, TextEncoding outputEncoding);

OSStatus TECConvertText(TECObjectRef encodingConverter, ConstTextPtr inputBuffer, ByteCount inputBufferLength, ByteCount *actualInputLength, TextPtr outputBuffer, ByteCount outputBufferLength, ByteCount *actualOutputLength);

OSStatus TECFlushText(TECObjectRef encodingConverter, TextPtr outputBuffer, ByteCount outputBufferLength, ByteCount *actualOutputLength);

OSStatus TECDisposeConverter(TECObjectRef newEncodingConverter);

#ifdef __cplusplus
}
#endif

#endif

