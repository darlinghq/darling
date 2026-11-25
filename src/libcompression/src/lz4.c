// Copyright (C) 2026 Lubos Dolezel
// The LZ4 stream layout of libcompression (COMPRESSION_LZ4): a sequence of blocks, each introduced by a four byte tag.
//   "bv41", uint32 uncompressed size, uint32 compressed size, an LZ4 block of that many bytes
//   "bv4-", uint32 size, that many bytes stored as they are
//   "bv4$"  the end of the stream
// All sizes are little endian. The layout was found by looking at streams written by macOS (black-box observation of
// compression_encode_buffer, probe data1.m of batch11b: 64 KiB blocks, an incompressible block stored with "bv4-",
// the end tag alone for empty input). The LZ4 block coding itself is the public format description
// (https://github.com/lz4/lz4/blob/dev/doc/lz4_Block_format.md): sequences of a token (high nibble the literal count,
// low nibble the match length minus four, a nibble of 15 continues in following bytes of 255 that add up), the
// literals, a two byte little endian offset and the continued match length; the last sequence holds literals only.
// This file is an original implementation of that description.

#include "lz4.h"

#include <stdint.h>
#include <string.h>

#define BV4_BLOCK_SIZE 65536
#define HASH_BITS 12
#define MIN_MATCH 4
// A format rule of the block coding: the last match starts at least 12 bytes before the end and the last 5 bytes are
// literals.
#define LAST_LITERALS 5
#define MATCH_FLIMIT 12

static void
store32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t) v;
    p[1] = (uint8_t) (v >> 8);
    p[2] = (uint8_t) (v >> 16);
    p[3] = (uint8_t) (v >> 24);
}

static uint32_t
load32(const uint8_t *p) {
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

static unsigned
hash4(uint32_t sequence) {
    return (sequence * 2654435761u) >> (32 - HASH_BITS);
}

// Writes the extension bytes of a length whose nibble is 15; returns the new output position or 0 when it does not fit.
static uint8_t *
write_length(uint8_t *out, const uint8_t *out_end, size_t length) {
    while (length >= 255) {
        if (out >= out_end)
            return NULL;
        *out++ = 255;
        length -= 255;
    }
    if (out >= out_end)
        return NULL;
    *out++ = (uint8_t) length;
    return out;
}

// Writes a sequence of literals followed by a match (match_length 0: the last sequence, literals only).
static uint8_t *
write_sequence(uint8_t *out, const uint8_t *out_end, const uint8_t *literals, size_t literal_count,
               size_t offset, size_t match_length) {
    if (out >= out_end)
        return NULL;
    uint8_t *token = out++;
    size_t match_code = match_length ? match_length - MIN_MATCH : 0;
    *token = (uint8_t) ((literal_count >= 15 ? 15 : literal_count) << 4);
    if (literal_count >= 15 && !(out = write_length(out, out_end, literal_count - 15)))
        return NULL;
    if ((size_t) (out_end - out) < literal_count)
        return NULL;
    memcpy(out, literals, literal_count);
    out += literal_count;
    if (match_length == 0)
        return out;
    if (out_end - out < 2)
        return NULL;
    *out++ = (uint8_t) offset;
    *out++ = (uint8_t) (offset >> 8);
    *token |= (uint8_t) (match_code >= 15 ? 15 : match_code);
    if (match_code >= 15 && !(out = write_length(out, out_end, match_code - 15)))
        return NULL;
    return out;
}

// Codes one block with a greedy search for matches inside the block. Returns the coded size, 0 when it does not fit.
static size_t
encode_block(uint8_t *out, size_t out_size, const uint8_t *in, size_t in_size) {
    uint8_t *out_pos = out;
    const uint8_t *out_end = out + out_size;
    const uint8_t *anchor = in;
    if (in_size >= MATCH_FLIMIT + 1) {
        uint16_t table[1 << HASH_BITS];
        memset(table, 0, sizeof table);
        // Positions are stored plus one so that 0 means "none".
        size_t limit = in_size - MATCH_FLIMIT;
        size_t pos = 0;
        while (pos < limit) {
            uint32_t sequence = load32(in + pos);
            unsigned h = hash4(sequence);
            size_t candidate = table[h];
            table[h] = (uint16_t) (pos + 1);
            if (candidate == 0 || load32(in + candidate - 1) != sequence) {
                pos++;
                continue;
            }
            size_t match_start = candidate - 1;
            size_t length = MIN_MATCH;
            size_t max_length = in_size - LAST_LITERALS - pos;
            while (length < max_length && in[match_start + length] == in[pos + length])
                length++;
            if (!(out_pos = write_sequence(out_pos, out_end, anchor, (size_t) (in + pos - anchor), pos - match_start, length)))
                return 0;
            pos += length;
            anchor = in + pos;
        }
    }
    out_pos = write_sequence(out_pos, out_end, anchor, (size_t) (in + in_size - anchor), 0, 0);
    return out_pos ? (size_t) (out_pos - out) : 0;
}

size_t
bv4_encode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    uint8_t *out = dst;
    uint8_t *out_end = dst + dst_size;
    while (src_size > 0) {
        size_t block = src_size < BV4_BLOCK_SIZE ? src_size : BV4_BLOCK_SIZE;
        if (out_end - out < 12)
            return 0;
        size_t room = (size_t) (out_end - out) - 12;
        size_t coded = encode_block(out + 12, room < block ? room : block, src, block);
        if (coded != 0 && coded < block) {
            memcpy(out, "bv41", 4);
            store32(out + 4, (uint32_t) block);
            store32(out + 8, (uint32_t) coded);
            out += 12 + coded;
        } else {
            // Not smaller: stored as it is.
            if ((size_t) (out_end - out) < 8 + block)
                return 0;
            memcpy(out, "bv4-", 4);
            store32(out + 4, (uint32_t) block);
            memcpy(out + 8, src, block);
            out += 8 + block;
        }
        src += block;
        src_size -= block;
    }
    if (out_end - out < 4)
        return 0;
    memcpy(out, "bv4$", 4);
    return (size_t) (out + 4 - dst);
}

// Decodes one LZ4 block into dst + *written (dst_size is the whole buffer, earlier output may be referenced by
// matches), for exactly expected bytes. Returns 1 when done, 0 when malformed, 2 when the buffer ended first (what fits
// has been written and *written is dst_size).
static int
decode_block(uint8_t *dst, size_t dst_size, size_t *written, const uint8_t *in, size_t in_size, size_t expected) {
    const uint8_t *in_end = in + in_size;
    size_t start = *written;
    size_t pos = start;
    while (in < in_end) {
        uint8_t token = *in++;
        size_t literals = token >> 4;
        if (literals == 15) {
            uint8_t more;
            do {
                if (in >= in_end)
                    return 0;
                more = *in++;
                literals += more;
            } while (more == 255);
        }
        if ((size_t) (in_end - in) < literals)
            return 0;
        size_t room = dst_size - pos;
        if (literals > room) {
            memcpy(dst + pos, in, room);
            *written = dst_size;
            return 2;
        }
        memcpy(dst + pos, in, literals);
        pos += literals;
        in += literals;
        if (in == in_end)
            break;
        if (in_end - in < 2)
            return 0;
        size_t offset = (size_t) in[0] | ((size_t) in[1] << 8);
        in += 2;
        if (offset == 0 || offset > pos)
            return 0;
        size_t match = (size_t) (token & 15);
        if (match == 15) {
            uint8_t more;
            do {
                if (in >= in_end)
                    return 0;
                more = *in++;
                match += more;
            } while (more == 255);
        }
        match += MIN_MATCH;
        size_t copy = match > dst_size - pos ? dst_size - pos : match;
        // The copy is byte by byte: a match may overlap its own output.
        for (size_t i = 0; i < copy; i++)
            dst[pos + i] = dst[pos + i - offset];
        pos += copy;
        if (copy < match) {
            *written = dst_size;
            return 2;
        }
    }
    if (pos - start != expected)
        return 0;
    *written = pos;
    return 1;
}

size_t
bv4_decode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    size_t written = 0;
    const uint8_t *in = src;
    const uint8_t *in_end = src + src_size;
    for (;;) {
        if (in_end - in < 4)
            return 0;
        if (memcmp(in, "bv4$", 4) == 0)
            return written;
        if (in_end - in < 8)
            return 0;
        uint32_t usize = load32(in + 4);
        if (memcmp(in, "bv4-", 4) == 0) {
            if ((size_t) (in_end - in) - 8 < usize)
                return 0;
            size_t room = dst_size - written;
            size_t copy = usize > room ? room : usize;
            memcpy(dst + written, in + 8, copy);
            written += copy;
            if (copy < usize)
                return dst_size;
            in += 8 + usize;
        } else if (memcmp(in, "bv41", 4) == 0) {
            if (in_end - in < 12)
                return 0;
            uint32_t csize = load32(in + 8);
            if ((size_t) (in_end - in) - 12 < csize)
                return 0;
            int status = decode_block(dst, dst_size, &written, in + 12, csize, usize);
            if (status == 0)
                return 0;
            if (status == 2)
                return dst_size;
            in += 12 + csize;
        } else {
            return 0;
        }
    }
}
