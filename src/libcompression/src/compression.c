// The buffer functions for LZFSE (the reference lzfse library), ZLIB (a raw
// DEFLATE stream, RFC 1951, without the zlib header and checksum), LZ4 (the
// "bv4" block stream, see lz4.c) and LZMA (an .xz stream without a check,
// through liblzma). The raw LZ4 variant and the stream functions are not
// implemented: the buffer functions return 0 and the stream functions an error
// for them.

#include <compression.h>
#include <lzfse.h>
#include <lzma.h>
#include <stdlib.h>
#include <zlib.h>

#include "lz4.h"

size_t
compression_encode_scratch_buffer_size(compression_algorithm algorithm) {
    if (algorithm == COMPRESSION_LZFSE)
        return lzfse_encode_scratch_size();
    return 0;
}

size_t
compression_decode_scratch_buffer_size(compression_algorithm algorithm) {
    if (algorithm == COMPRESSION_LZFSE)
        return lzfse_decode_scratch_size();
    return 0;
}

// Returns the number of bytes written, or 0 when the output does not fit.
static size_t
zlib_encode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    z_stream stream = { 0 };
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        return 0;
    stream.next_in = (Bytef *) src;
    stream.avail_in = (uInt) src_size;
    stream.next_out = dst;
    stream.avail_out = (uInt) dst_size;
    int status = deflate(&stream, Z_FINISH);
    size_t written = stream.total_out;
    deflateEnd(&stream);
    return status == Z_STREAM_END ? written : 0;
}

// Returns the number of bytes written: all of the output, or dst_size when it
// does not fit; 0 for invalid or incomplete input.
static size_t
zlib_decode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    z_stream stream = { 0 };
    if (inflateInit2(&stream, -15) != Z_OK)
        return 0;
    stream.next_in = (Bytef *) src;
    stream.avail_in = (uInt) src_size;
    stream.next_out = dst;
    stream.avail_out = (uInt) dst_size;
    int status = inflate(&stream, Z_FINISH);
    size_t written = stream.total_out;
    inflateEnd(&stream);
    if (status == Z_STREAM_END || (stream.avail_out == 0 && (status == Z_OK || status == Z_BUF_ERROR)))
        return written;
    return 0;
}

// An .xz stream with one block at preset 6 and no integrity check, the form macOS writes (observed: stream flags 0,
// an LZMA2 filter with an 8 MiB dictionary). Returns the number of bytes written, 0 when the output does not fit.
static size_t
lzma_encode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    size_t out_pos = 0;
    if (lzma_easy_buffer_encode(6, LZMA_CHECK_NONE, NULL, src, src_size, dst, &out_pos, dst_size) != LZMA_OK)
        return 0;
    return out_pos;
}

// Returns all of the output, 0 for invalid or incomplete input and also when the output does not fit: unlike the other
// algorithms, LZMA gives nothing for a buffer that is too small (QUIRK, observed on macOS 10.14.4 and 26.7.3, tier 2:
// probe trunc.m of batch11b). Test: tests/src/compression_lz4_lzma.c.
static size_t
lzma_decode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    lzma_stream stream = LZMA_STREAM_INIT;
    if (lzma_stream_decoder(&stream, UINT64_MAX, 0) != LZMA_OK)
        return 0;
    stream.next_in = src;
    stream.avail_in = src_size;
    stream.next_out = dst;
    stream.avail_out = dst_size;
    lzma_ret status = lzma_code(&stream, LZMA_FINISH);
    size_t written = dst_size - stream.avail_out;
    lzma_end(&stream);
    return status == LZMA_STREAM_END ? written : 0;
}

size_t
compression_encode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
                          const uint8_t * __restrict src_buffer, size_t src_size,
                          void * __restrict __nullable scratch_buffer,
                          compression_algorithm algorithm) {
    // Empty input still encodes to a valid (empty) stream.
    if (dst_size == 0)
        return 0;
    switch (algorithm) {
    case COMPRESSION_LZFSE:
        return lzfse_encode_buffer(dst_buffer, dst_size, src_buffer, src_size, scratch_buffer);
    case COMPRESSION_ZLIB:
        return zlib_encode(dst_buffer, dst_size, src_buffer, src_size);
    case COMPRESSION_LZ4:
        return bv4_encode(dst_buffer, dst_size, src_buffer, src_size);
    case COMPRESSION_LZMA:
        return lzma_encode(dst_buffer, dst_size, src_buffer, src_size);
    default:
        return 0;
    }
}

size_t
compression_decode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
                          const uint8_t * __restrict src_buffer, size_t src_size,
                          void * __restrict __nullable scratch_buffer,
                          compression_algorithm algorithm) {
    if (dst_size == 0 || src_size == 0)
        return 0;
    switch (algorithm) {
    case COMPRESSION_LZFSE:
        return lzfse_decode_buffer(dst_buffer, dst_size, src_buffer, src_size, scratch_buffer);
    case COMPRESSION_ZLIB:
        return zlib_decode(dst_buffer, dst_size, src_buffer, src_size);
    case COMPRESSION_LZ4:
        return bv4_decode(dst_buffer, dst_size, src_buffer, src_size);
    case COMPRESSION_LZMA:
        return lzma_decode(dst_buffer, dst_size, src_buffer, src_size);
    default:
        return 0;
    }
}

compression_status
compression_stream_init(compression_stream * stream,
                        compression_stream_operation operation,
                        compression_algorithm algorithm) {
    return COMPRESSION_STATUS_ERROR;
}

compression_status
compression_stream_process(compression_stream * stream,
                           int flags) {
    return COMPRESSION_STATUS_ERROR;
}

compression_status
compression_stream_destroy(compression_stream * stream) {
    return COMPRESSION_STATUS_ERROR;
}
