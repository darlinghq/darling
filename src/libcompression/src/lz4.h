// Copyright (C) 2026 Lubos Dolezel
#ifndef LIBCOMPRESSION_LZ4_H
#define LIBCOMPRESSION_LZ4_H

#include <stddef.h>
#include <stdint.h>

// The "bv4" block stream of COMPRESSION_LZ4. Both return the number of bytes written; bv4_encode 0 when the output does
// not fit, bv4_decode 0 for a malformed stream and dst_size when the output is cut off.
size_t bv4_encode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size);
size_t bv4_decode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size);

#endif
