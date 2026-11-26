// The buffer functions for LZFSE (the reference lzfse library), ZLIB (a raw
// DEFLATE stream, RFC 1951, without the zlib header and checksum), LZ4 (the
// "bv4" block stream, see lz4.c), LZMA (an .xz stream without a check,
// through liblzma) and BROTLI (RFC 7932, through the brotli library), and the
// stream functions for ZLIB, LZMA and BROTLI. The raw LZ4 variant has no
// buffer or stream functions, and LZFSE and LZ4 have no stream functions yet:
// the buffer functions return 0 and the stream functions an error for them.

#include <brotli/decode.h>
#include <brotli/encode.h>
#include <compression.h>
#include <limits.h>
#include <lzfse.h>
#include <lzma.h>
#include <stdbool.h>
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

// The Brotli encoder settings macOS uses, for the buffer and the stream functions: quality 2 and a 4 MiB window
// (observed on macOS 26.7.3: its output for a 20000-byte text is the one these settings give).
#define BROTLI_QUALITY 2
#define BROTLI_WINDOW_BITS 22

// Returns the number of bytes written, or 0 when the output does not fit.
static size_t
brotli_encode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    size_t written = dst_size;
    if (!BrotliEncoderCompress(BROTLI_QUALITY, BROTLI_WINDOW_BITS, BROTLI_MODE_GENERIC, src_size, src, &written, dst))
        return 0;
    return written;
}

// Returns all of the output, 0 for invalid or incomplete input and also when the output does not fit, as for LZMA
// (observed on macOS 26.7.3). Test: tests/src/compression_stream_brotli.c.
static size_t
brotli_decode(uint8_t *dst, size_t dst_size, const uint8_t *src, size_t src_size) {
    size_t written = dst_size;
    if (BrotliDecoderDecompress(src_size, src, &written, dst) != BROTLI_DECODER_RESULT_SUCCESS)
        return 0;
    return written;
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
    case COMPRESSION_BROTLI:
        return brotli_encode(dst_buffer, dst_size, src_buffer, src_size);
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
    case COMPRESSION_BROTLI:
        return brotli_decode(dst_buffer, dst_size, src_buffer, src_size);
    default:
        return 0;
    }
}

// The state of a stream (compression_stream.state).
struct stream_state {
    compression_algorithm algorithm;
    compression_stream_operation operation;
    bool ended; // the last call returned COMPRESSION_STATUS_END
    union {
        z_stream zlib;
        lzma_stream lzma;
        BrotliEncoderState *brotli_encoder;
        BrotliDecoderState *brotli_decoder;
    };
};

compression_status
compression_stream_init(compression_stream * stream,
                        compression_stream_operation operation,
                        compression_algorithm algorithm) {
    if (operation != COMPRESSION_STREAM_ENCODE && operation != COMPRESSION_STREAM_DECODE)
        return COMPRESSION_STATUS_ERROR;
    struct stream_state *state = calloc(1, sizeof(*state));
    if (state == NULL)
        return COMPRESSION_STATUS_ERROR;
    state->algorithm = algorithm;
    state->operation = operation;
    bool encode = operation == COMPRESSION_STREAM_ENCODE;
    bool ok;

    switch (algorithm) {
    case COMPRESSION_ZLIB:
        // a raw DEFLATE stream at the default level, as the buffer functions write and read it
        ok = (encode ? deflateInit2(&state->zlib, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY)
                     : inflateInit2(&state->zlib, -15)) == Z_OK;
        break;
    case COMPRESSION_LZMA:
        // an .xz stream at preset 6 without a check; the block header has no sizes, as macOS writes it
        // (observed on macOS 26.7.3), and the decoder reads one stream and stops at its end
        state->lzma = (lzma_stream) LZMA_STREAM_INIT;
        ok = (encode ? lzma_easy_encoder(&state->lzma, 6, LZMA_CHECK_NONE)
                     : lzma_stream_decoder(&state->lzma, UINT64_MAX, 0)) == LZMA_OK;
        break;
    case COMPRESSION_BROTLI:
        if (encode) {
            state->brotli_encoder = BrotliEncoderCreateInstance(NULL, NULL, NULL);
            ok = state->brotli_encoder != NULL
                && BrotliEncoderSetParameter(state->brotli_encoder, BROTLI_PARAM_QUALITY, BROTLI_QUALITY)
                && BrotliEncoderSetParameter(state->brotli_encoder, BROTLI_PARAM_LGWIN, BROTLI_WINDOW_BITS);
            if (!ok && state->brotli_encoder != NULL)
                BrotliEncoderDestroyInstance(state->brotli_encoder);
        } else {
            state->brotli_decoder = BrotliDecoderCreateInstance(NULL, NULL, NULL);
            ok = state->brotli_decoder != NULL;
        }
        break;
    default:
        ok = false;
        break;
    }

    if (!ok) {
        free(state);
        return COMPRESSION_STATUS_ERROR;
    }
    stream->state = state;
    return COMPRESSION_STATUS_OK;
}

// zlib and liblzma count in unsigned int and size_t; a call gives zlib at most UINT_MAX bytes of either buffer and
// the caller calls again for the rest.
static uInt
zlib_size(size_t size) {
    return size > UINT_MAX ? UINT_MAX : (uInt) size;
}

static compression_status
zlib_process(struct stream_state *state, compression_stream *stream, bool finalize) {
    z_stream *z = &state->zlib;
    uInt in_size = zlib_size(stream->src_size), out_size = zlib_size(stream->dst_size);
    z->next_in = (Bytef *) stream->src_ptr;
    z->avail_in = in_size;
    z->next_out = stream->dst_ptr;
    z->avail_out = out_size;
    // the caller's input may be only part of what it has (more than UINT_MAX bytes), so it is finished only when all
    // of it is here
    bool all_input = in_size == stream->src_size;
    int status;
    if (state->operation == COMPRESSION_STREAM_ENCODE)
        status = deflate(z, finalize && all_input ? Z_FINISH : Z_NO_FLUSH);
    else
        status = inflate(z, Z_NO_FLUSH);
    size_t consumed = in_size - z->avail_in, produced = out_size - z->avail_out;
    stream->src_ptr += consumed;
    stream->src_size -= consumed;
    stream->dst_ptr += produced;
    stream->dst_size -= produced;

    if (status == Z_STREAM_END) {
        // QUIRK: at the end of a DEFLATE stream the decoder also takes whatever input follows it, unlike the LZMA
        // and Brotli decoders, which leave it to the caller. Not documented; black-box observation on macOS 26.7.3
        // (tier 2). Test: tests/src/compression_stream_brotli.c ("zlib: the decoder takes the input after the end").
        if (state->operation == COMPRESSION_STREAM_DECODE) {
            stream->src_ptr += stream->src_size;
            stream->src_size = 0;
        }
        return COMPRESSION_STATUS_END;
    }
    if (status != Z_OK && status != Z_BUF_ERROR)
        return COMPRESSION_STATUS_ERROR;
    return COMPRESSION_STATUS_OK;
}

static compression_status
lzma_process(struct stream_state *state, compression_stream *stream, bool finalize) {
    lzma_stream *x = &state->lzma;
    x->next_in = stream->src_ptr;
    x->avail_in = stream->src_size;
    x->next_out = stream->dst_ptr;
    x->avail_out = stream->dst_size;
    bool encode = state->operation == COMPRESSION_STREAM_ENCODE;
    lzma_ret status = lzma_code(x, encode && finalize ? LZMA_FINISH : LZMA_RUN);
    stream->src_ptr = x->next_in;
    stream->src_size = x->avail_in;
    stream->dst_ptr = x->next_out;
    stream->dst_size = x->avail_out;

    if (status == LZMA_STREAM_END)
        return COMPRESSION_STATUS_END;
    if (status != LZMA_OK && status != LZMA_BUF_ERROR)
        return COMPRESSION_STATUS_ERROR;
    return COMPRESSION_STATUS_OK;
}

static compression_status
brotli_process(struct stream_state *state, compression_stream *stream, bool finalize) {
    size_t in_size = stream->src_size, out_size = stream->dst_size;
    const uint8_t *in = stream->src_ptr;
    uint8_t *out = stream->dst_ptr;
    compression_status result;

    if (state->operation == COMPRESSION_STREAM_ENCODE) {
        BrotliEncoderOperation op = finalize ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
        if (!BrotliEncoderCompressStream(state->brotli_encoder, op, &in_size, &in, &out_size, &out, NULL))
            result = COMPRESSION_STATUS_ERROR;
        else if (BrotliEncoderIsFinished(state->brotli_encoder))
            result = COMPRESSION_STATUS_END;
        else
            result = COMPRESSION_STATUS_OK;
    } else {
        switch (BrotliDecoderDecompressStream(state->brotli_decoder, &in_size, &in, &out_size, &out, NULL)) {
        case BROTLI_DECODER_RESULT_SUCCESS:
            result = COMPRESSION_STATUS_END;
            break;
        case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT:
        case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT:
            result = COMPRESSION_STATUS_OK;
            break;
        default:
            result = COMPRESSION_STATUS_ERROR;
            break;
        }
    }

    stream->src_ptr = in;
    stream->src_size = in_size;
    stream->dst_ptr = out;
    stream->dst_size = out_size;
    return result;
}

compression_status
compression_stream_process(compression_stream * stream,
                           int flags) {
    struct stream_state *state = stream->state;
    if (state == NULL)
        return COMPRESSION_STATUS_ERROR;
    bool finalize = (flags & COMPRESSION_STREAM_FINALIZE) != 0;

    if (state->ended) {
        // QUIRK: once a ZLIB encoder has ended, a further call is an error, where the LZMA and Brotli encoders answer
        // that the stream has ended again. Not documented; black-box observation on macOS 26.7.3 (tier 2).
        // Test: tests/src/compression_stream_brotli.c ("a call after the end").
        if (state->algorithm == COMPRESSION_ZLIB && state->operation == COMPRESSION_STREAM_ENCODE)
            return COMPRESSION_STATUS_ERROR;
        return COMPRESSION_STATUS_END;
    }

    compression_status status;
    switch (state->algorithm) {
    case COMPRESSION_ZLIB:
        status = zlib_process(state, stream, finalize);
        break;
    case COMPRESSION_LZMA:
        status = lzma_process(state, stream, finalize);
        break;
    case COMPRESSION_BROTLI:
        status = brotli_process(state, stream, finalize);
        break;
    default:
        status = COMPRESSION_STATUS_ERROR;
        break;
    }

    // A decoder that was told the input is complete, has used all of it and still has room for output, but has not
    // reached the end of the compressed data, was given truncated data: an error. Without the flag it waits for more.
    if (status == COMPRESSION_STATUS_OK && finalize && state->operation == COMPRESSION_STREAM_DECODE
        && stream->src_size == 0 && stream->dst_size > 0)
        status = COMPRESSION_STATUS_ERROR;

    if (status == COMPRESSION_STATUS_END)
        state->ended = true;
    return status;
}

compression_status
compression_stream_destroy(compression_stream * stream) {
    struct stream_state *state = stream->state;
    if (state == NULL)
        return COMPRESSION_STATUS_ERROR;
    bool encode = state->operation == COMPRESSION_STREAM_ENCODE;
    switch (state->algorithm) {
    case COMPRESSION_ZLIB:
        if (encode)
            deflateEnd(&state->zlib);
        else
            inflateEnd(&state->zlib);
        break;
    case COMPRESSION_LZMA:
        lzma_end(&state->lzma);
        break;
    case COMPRESSION_BROTLI:
        if (encode)
            BrotliEncoderDestroyInstance(state->brotli_encoder);
        else
            BrotliDecoderDestroyInstance(state->brotli_decoder);
        break;
    default:
        break;
    }
    free(state);
    stream->state = NULL;
    return COMPRESSION_STATUS_OK;
}
