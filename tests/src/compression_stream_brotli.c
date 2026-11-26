// Copyright (C) 2026 Lubos Dolezel
//
// libcompression's stream functions for COMPRESSION_ZLIB, COMPRESSION_LZMA and COMPRESSION_BROTLI, and the buffer
// functions for COMPRESSION_BROTLI. Everything checked here was observed on macOS 26.7.3 x86_64, where this test
// passes:
//  - Brotli buffer encoding gives what macOS wrote for the input below (brotli_vector: quality 2, 4 MiB window), and
//    decoding it gives the input back; an empty input encodes to the one byte 0x06; encoding into a buffer that is too
//    small and decoding into one that is too small both return 0;
//  - a stream encoder fed in pieces through small output windows ends with COMPRESSION_STATUS_END; for ZLIB and BROTLI
//    its output is the buffer functions' output, for LZMA it is an .xz stream whose block header has no sizes;
//  - a stream decoder fed one byte at a time ends with COMPRESSION_STATUS_END and the input, also without
//    COMPRESSION_STREAM_FINALIZE; truncated input gives COMPRESSION_STATUS_ERROR with the flag and
//    COMPRESSION_STATUS_OK without it; corrupt input gives an error;
//  - after the end, the LZMA and Brotli decoders leave the input that follows to the caller, and the ZLIB decoder
//    takes it (QUIRK);
//  - a call after the end of an encoder is an error for ZLIB and COMPRESSION_STATUS_END for LZMA and BROTLI (QUIRK);
//  - COMPRESSION_LZ4_RAW and an unknown algorithm have no streams.
#include <compression.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(c, what) do { if (!(c)) { printf("FAIL line %d: %s: %s\n", __LINE__, what, #c); failures++; } } while (0)

static const uint8_t brotli_vector[] = {
    0x1b, 0x1f, 0x4e, 0x00, 0x80, 0xaa, 0xaa, 0xaa, 0xea, 0x9f, 0x0f, 0x27, 0x3f, 0x9c, 0xf9, 0x70,
    0xb2, 0xc3, 0x7d, 0x53, 0xdb, 0xdc, 0xcc, 0xd4, 0x76, 0x77, 0xb3, 0x9b, 0x99, 0x9b, 0x9a, 0x9b,
    0xaf, 0xea, 0x6e, 0x6e, 0xe6, 0xdb, 0x00, 0x1c, 0x8c, 0x05, 0x91, 0x98, 0x24, 0x06, 0xb8, 0xd3,
    0xe6, 0xa6, 0x2b, 0xb7, 0x9c, 0x14, 0x9f, 0xeb, 0xe9, 0x70, 0x61, 0x3f, 0xeb, 0xcf, 0x9d, 0xa3,
    0xfe, 0xf2, 0xbc, 0xde, 0x1e, 0x2f, 0xea, 0xb7, 0x9a, 0xb9, 0x4c, 0x8a, 0xd7, 0xee, 0xff, 0xe3,
    0xa0, 0x8f, 0x06, 0x75, 0xb8, 0xe9, 0xca, 0xb5, 0xb7, 0xb0, 0x72, 0xb9, 0x4b, 0xe0, 0xca, 0x3d,
    0xc2, 0x2a, 0x4e, 0x0b, 0xc8, 0x5a, 0xe4, 0x28, 0x7d, 0x81, 0xc8, 0x6c, 0x1b, 0x38, 0xd2, 0xdb,
    0x60, 0xf1, 0x05, 0xb2, 0x2a, 0x00, 0x8c, 0x41, 0xe0, 0xc0, 0xd8, 0x05, 0x3b, 0x86, 0x32, 0x85,
    0x8c, 0x01, 0x64, 0xec, 0xa0, 0xa1, 0x4c, 0x21, 0x63, 0x00, 0x19, 0x3b, 0x68, 0x28, 0x53, 0xc8,
    0x18, 0x40, 0xc6, 0x0e, 0x1a, 0xca, 0x14, 0x32, 0x06, 0x90, 0xb1, 0x83, 0x86, 0x32, 0x85, 0x8c,
    0x01, 0x64, 0xec, 0xa0, 0xa1, 0x4c, 0x21, 0x63, 0x00, 0x19, 0x3b, 0x68, 0x28, 0x53, 0xc8, 0x18,
    0x40, 0xc6, 0x0e, 0x1a, 0xca, 0x14, 0x32, 0x06, 0x90, 0xb1, 0x83, 0x86, 0x32, 0x85, 0x8c, 0x01,
    0x64, 0xec, 0xa0, 0xa1, 0x4c, 0x21, 0x63, 0x00, 0x19, 0x3b, 0x68, 0x28, 0x53, 0xc8, 0x18, 0x40,
    0xc6, 0x0e, 0x1a, 0xca, 0x14, 0x32, 0x06, 0x90, 0xb1, 0x83, 0x86, 0x32, 0x85, 0x8c, 0x01, 0x64,
    0xec, 0xa0, 0xa1, 0x4c, 0x21, 0x63, 0x00, 0x19, 0x3b, 0x68, 0x28, 0x53, 0xc8, 0x18, 0x40, 0xc6,
    0x0e, 0x1a, 0xca, 0x14, 0x1a, 0xc2, 0x1f, 0xaa, 0x18, 0xd2, 0x05, 0x1a, 0x83, 0xc0, 0x81, 0xb1,
    0x0b, 0x76, 0x0c, 0x65, 0x0a, 0x19, 0x03, 0xc8, 0x08, 0x20,
};

static uint8_t text[20000];

static const char *name(compression_algorithm a) {
    return a == COMPRESSION_ZLIB ? "zlib" : a == COMPRESSION_LZMA ? "lzma" : "brotli";
}

// Encodes text in 1000-byte pieces through 37-byte output windows; returns the length, 0 on failure.
static size_t stream_encode(compression_algorithm a, uint8_t *out, size_t cap, compression_status *after_end) {
    compression_stream s;
    if (compression_stream_init(&s, COMPRESSION_STREAM_ENCODE, a) != COMPRESSION_STATUS_OK)
        return 0;
    size_t fed = 0, got = 0;
    compression_status status;
    for (int calls = 0; calls < 100000; calls++) {
        size_t chunk = sizeof text - fed < 1000 ? sizeof text - fed : 1000;
        size_t window = cap - got < 37 ? cap - got : 37;
        s.src_ptr = text + fed;
        s.src_size = chunk;
        s.dst_ptr = out + got;
        s.dst_size = window;
        status = compression_stream_process(&s, fed + chunk == sizeof text ? COMPRESSION_STREAM_FINALIZE : 0);
        fed += chunk - s.src_size;
        got += window - s.dst_size;
        if (status != COMPRESSION_STATUS_OK)
            break;
    }
    uint8_t junk[16];
    s.src_size = 0;
    s.dst_ptr = junk;
    s.dst_size = sizeof junk;
    *after_end = compression_stream_process(&s, COMPRESSION_STREAM_FINALIZE);
    CHECK(s.dst_size == sizeof junk, "nothing is written after the end");
    CHECK(compression_stream_destroy(&s) == COMPRESSION_STATUS_OK, name(a));
    return status == COMPRESSION_STATUS_END && fed == sizeof text ? got : 0;
}

struct decoded { compression_status last; size_t out, consumed; int matches; };

// Decodes in pieces of in_step bytes through out_step-byte windows; stops at the first status that is not OK or when a
// call makes no progress.
static struct decoded stream_decode(compression_algorithm a, const uint8_t *in, size_t in_len, size_t in_step,
                                    size_t out_step, int finalize) {
    struct decoded r = { COMPRESSION_STATUS_ERROR, 0, 0, 0 };
    compression_stream s;
    if (compression_stream_init(&s, COMPRESSION_STREAM_DECODE, a) != COMPRESSION_STATUS_OK)
        return r;
    static uint8_t out[sizeof text + 64];
    for (int calls = 0; calls < 200000; calls++) {
        size_t chunk = in_len - r.consumed < in_step ? in_len - r.consumed : in_step;
        size_t window = sizeof out - r.out < out_step ? sizeof out - r.out : out_step;
        s.src_ptr = in + r.consumed;
        s.src_size = chunk;
        s.dst_ptr = out + r.out;
        s.dst_size = window;
        r.last = compression_stream_process(&s, finalize && r.consumed + chunk == in_len ? COMPRESSION_STREAM_FINALIZE : 0);
        size_t consumed = chunk - s.src_size, produced = window - s.dst_size;
        r.consumed += consumed;
        r.out += produced;
        if (r.last != COMPRESSION_STATUS_OK || (consumed == 0 && produced == 0))
            break;
    }
    r.matches = r.out == sizeof text && memcmp(out, text, sizeof text) == 0;
    compression_stream_destroy(&s);
    return r;
}

int main(void) {
    for (size_t i = 0; i < sizeof text; i++)
        text[i] = "The quick brown fox jumps over the lazy dog. "[i % 45] ^ ((i / 997) & 1 ? 0x20 : 0);
    static uint8_t buf[40000], sbuf[40000], dec[40000];

    // Brotli buffer functions
    size_t n = compression_encode_buffer(buf, sizeof buf, text, sizeof text, NULL, COMPRESSION_BROTLI);
    CHECK(n == sizeof brotli_vector && memcmp(buf, brotli_vector, n) == 0, "brotli: the encoder writes what macOS writes");
    CHECK(compression_decode_buffer(dec, sizeof dec, brotli_vector, sizeof brotli_vector, NULL, COMPRESSION_BROTLI) == sizeof text
          && memcmp(dec, text, sizeof text) == 0, "brotli: macOS's output decodes");
    CHECK(compression_encode_buffer(buf, sizeof buf, text, 0, NULL, COMPRESSION_BROTLI) == 1 && buf[0] == 0x06, "brotli: empty input");
    CHECK(compression_encode_buffer(buf, 8, text, sizeof text, NULL, COMPRESSION_BROTLI) == 0, "brotli: output too small to encode");
    CHECK(compression_decode_buffer(dec, 100, brotli_vector, sizeof brotli_vector, NULL, COMPRESSION_BROTLI) == 0, "brotli: output too small to decode");
    CHECK(compression_decode_buffer(dec, sizeof dec, brotli_vector, sizeof brotli_vector - 5, NULL, COMPRESSION_BROTLI) == 0, "brotli: truncated input");

    compression_algorithm algorithms[] = { COMPRESSION_ZLIB, COMPRESSION_LZMA, COMPRESSION_BROTLI };
    for (int k = 0; k < 3; k++) {
        compression_algorithm a = algorithms[k];
        const char *what = name(a);
        compression_status after_end;
        size_t sn = stream_encode(a, sbuf, sizeof sbuf, &after_end);
        CHECK(sn > 0, what);
        if (sn == 0)
            continue;
        if (a == COMPRESSION_LZMA) {
            // .xz magic, stream flags 0 (no check), then a block header of 2 * 4 bytes whose flags give no sizes
            CHECK(memcmp(sbuf, "\xfd" "7zXZ\0\0\0", 8) == 0 && sbuf[12] == 0x02 && sbuf[13] == 0x00, "lzma: the stream form");
        } else {
            n = compression_encode_buffer(buf, sizeof buf, text, sizeof text, NULL, a);
            CHECK(n == sn && memcmp(buf, sbuf, n) == 0, "the stream encoder writes what the buffer encoder writes");
        }
        CHECK(after_end == (a == COMPRESSION_ZLIB ? COMPRESSION_STATUS_ERROR : COMPRESSION_STATUS_END), "a call after the end");

        struct decoded d = stream_decode(a, sbuf, sn, 1, 1, 1);
        CHECK(d.last == COMPRESSION_STATUS_END && d.matches && d.consumed == sn, "one byte at a time");
        d = stream_decode(a, sbuf, sn, 333, 4096, 0);
        CHECK(d.last == COMPRESSION_STATUS_END && d.matches, "without COMPRESSION_STREAM_FINALIZE");
        d = stream_decode(a, sbuf, sn - 5, 333, 4096, 1);
        CHECK(d.last == COMPRESSION_STATUS_ERROR && d.consumed == sn - 5, "truncated, with COMPRESSION_STREAM_FINALIZE");
        d = stream_decode(a, sbuf, sn - 5, 333, 4096, 0);
        CHECK(d.last == COMPRESSION_STATUS_OK && d.consumed == sn - 5, "truncated, without COMPRESSION_STREAM_FINALIZE");

        uint8_t *more = malloc(sn + 10);
        memcpy(more, sbuf, sn);
        memset(more + sn, 0x55, 10);
        d = stream_decode(a, more, sn + 10, sn + 10, sizeof text + 64, 1);
        CHECK(d.last == COMPRESSION_STATUS_END && d.matches, "input after the end");
        if (a == COMPRESSION_ZLIB)
            CHECK(d.consumed == sn + 10, "zlib: the decoder takes the input after the end");
        else
            CHECK(d.consumed == sn, "the decoder leaves the input after the end");
        memset(more, 0xEE, 8);
        d = stream_decode(a, more, sn, sn, sizeof text + 64, 1);
        CHECK(d.last == COMPRESSION_STATUS_ERROR && d.out == 0, "corrupt input");
        free(more);
    }

    compression_stream s;
    CHECK(compression_stream_init(&s, COMPRESSION_STREAM_ENCODE, COMPRESSION_LZ4_RAW) == COMPRESSION_STATUS_ERROR, "lz4 raw");
    CHECK(compression_stream_init(&s, COMPRESSION_STREAM_ENCODE, (compression_algorithm) 0x999) == COMPRESSION_STATUS_ERROR, "an unknown algorithm");

    printf("%s (%d failures)\n", failures ? "FAILED" : "PASS", failures);
    return failures != 0;
}
