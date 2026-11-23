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

#include "MD5.h"
#include <string.h>

// RFC 1321, 3.4: the 64 constants are floor(2^32 * abs(sin(i + 1))) for i = 0..63.
static const uint32_t kSine[64] = {
	0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
	0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
	0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
	0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
	0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
	0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
	0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
	0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
	0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
	0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
	0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
	0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
	0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
	0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
	0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
	0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u,
};

// RFC 1321, 3.4: the left rotation of each of the four rounds repeats every four steps.
static const uint8_t kRotation[4][4] = {
	{ 7, 12, 17, 22 },
	{ 5, 9, 14, 20 },
	{ 4, 11, 16, 23 },
	{ 6, 10, 15, 21 },
};

static uint32_t rotateLeft(uint32_t x, unsigned bits)
{
	return (x << bits) | (x >> (32 - bits));
}

// Runs the 64 steps of RFC 1321, 3.4 over one 64-byte block, whose words are little-endian.
static void processBlock(uint32_t state[4], const uint8_t block[64])
{
	uint32_t words[16];
	for (int i = 0; i < 16; i++)
		words[i] = (uint32_t) block[i * 4] | ((uint32_t) block[i * 4 + 1] << 8) | ((uint32_t) block[i * 4 + 2] << 16) | ((uint32_t) block[i * 4 + 3] << 24);

	uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
	for (unsigned step = 0; step < 64; step++)
	{
		const unsigned round = step / 16;
		uint32_t mix;
		unsigned wordIndex;
		switch (round)
		{
			case 0:
				mix = (b & c) | (~b & d);
				wordIndex = step;
				break;
			case 1:
				mix = (b & d) | (c & ~d);
				wordIndex = (5 * step + 1) % 16;
				break;
			case 2:
				mix = b ^ c ^ d;
				wordIndex = (3 * step + 5) % 16;
				break;
			default:
				mix = c ^ (b | ~d);
				wordIndex = (7 * step) % 16;
				break;
		}
		const uint32_t sum = a + mix + words[wordIndex] + kSine[step];
		a = d;
		d = c;
		c = b;
		b = b + rotateLeft(sum, kRotation[round][step % 4]);
	}
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

void MD5ContextInit(MD5Context* context)
{
	// RFC 1321, 3.3
	context->state[0] = 0x67452301u;
	context->state[1] = 0xefcdab89u;
	context->state[2] = 0x98badcfeu;
	context->state[3] = 0x10325476u;
	context->byteCount = 0;
}

void MD5ContextUpdate(MD5Context* context, const void* bytes, size_t length)
{
	const uint8_t* p = bytes;
	size_t used = (size_t) (context->byteCount % 64);
	context->byteCount += length;

	if (used > 0)
	{
		const size_t room = 64 - used;
		const size_t take = length < room ? length : room;
		memcpy(context->pending + used, p, take);
		p += take;
		length -= take;
		if (used + take < 64)
			return;
		processBlock(context->state, context->pending);
	}
	for (; length >= 64; p += 64, length -= 64)
		processBlock(context->state, p);
	memcpy(context->pending, p, length);
}

void MD5ContextFinal(MD5Context* context, uint8_t digest[MD5_DIGEST_SIZE])
{
	// RFC 1321, 3.1 and 3.2: a 1 bit, zero bits up to 56 mod 64 bytes, then the bit length as 64 bits.
	const uint64_t bitCount = context->byteCount * 8;
	uint8_t padding[64] = { 0x80 };
	const size_t used = (size_t) (context->byteCount % 64);
	MD5ContextUpdate(context, padding, used < 56 ? 56 - used : 120 - used);

	uint8_t lengthBytes[8];
	for (int i = 0; i < 8; i++)
		lengthBytes[i] = (uint8_t) (bitCount >> (8 * i));
	MD5ContextUpdate(context, lengthBytes, sizeof lengthBytes);

	for (int i = 0; i < 16; i++)
		digest[i] = (uint8_t) (context->state[i / 4] >> (8 * (i % 4)));
}
