/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _COLORSYNC_MD5_H_
#define _COLORSYNC_MD5_H_

#include <stddef.h>
#include <stdint.h>

// The MD5 message digest of RFC 1321 (https://www.rfc-editor.org/rfc/rfc1321), written from that text.
// A 128-bit digest of any number of bytes, fed in pieces if wanted. Not for security: MD5 is broken as a hash.

#define MD5_DIGEST_SIZE 16

typedef struct MD5Context {
	uint32_t state[4];
	uint64_t byteCount;
	uint8_t pending[64];
} MD5Context;

void MD5ContextInit(MD5Context* context);
void MD5ContextUpdate(MD5Context* context, const void* bytes, size_t length);
void MD5ContextFinal(MD5Context* context, uint8_t digest[MD5_DIGEST_SIZE]);

#endif
