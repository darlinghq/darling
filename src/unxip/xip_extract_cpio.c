//
//  main.c
//  pbzx
//
//  Created by PHPdev32 on 6/20/14.
//  Licensed under GPLv3, full text at http://www.gnu.org/licenses/gpl-3.0.txt
//
//  This code originated here: http://www.tonymacx86.com/threads/pbzx-stream-parser.135458/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <xar/xar.h>

#define min(A,B) ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
#define err(c, m) if (c) { fprintf(stderr, m"\n"); exit(__COUNTER__ + 1); }
#define XBSZ 1024*4
#define ZBSZ 1024

static inline void xar_read(char *buffer, uint32_t size, xar_stream *stream) {
    stream->next_out = buffer;
    stream->avail_out = size;
    while (stream->avail_out)
        err(xar_extract_tostream(stream) != XAR_STREAM_OK, "XAR extraction failure");
}

static inline size_t cpio_out(char *buffer, size_t size) {
    size_t c = 0;
    while (c < size)
        c+= fwrite(buffer + c, 1, size - c, stdout);
    return c;
}

static inline uint64_t xar_read_64(xar_stream *stream) {
    char t[8];
    xar_read(t, 8, stream);
    return __builtin_bswap64(*(uint64_t *)t);
}

int main(int argc, const char * argv[])
{
    char xbuf[XBSZ], *zbuf = malloc(ZBSZ);
    xar_t x;
    err(argc < 2, "No file specified");
    err(!(x = xar_open(argv[1], READ)), "XAR open failure");
    xar_iter_t i = xar_iter_new();
    xar_file_t f = xar_file_first(x, i);
    char *path;
    while (strncmp((path = xar_get_path(f)), "Content", 7) && (f = xar_file_next(i)))
        free(path);
    free(path);
    xar_iter_free(i);
    err(!f, "No payload");
    err(xar_verify(x, f) != XAR_STREAM_OK, "File verification failed");
    xar_stream xs;
    err(xar_extract_tostream_init(x, f, &xs) != XAR_STREAM_OK, "XAR init failed");
    xar_read(xbuf, 4, &xs);
    err(strncmp(xbuf, "pbzx", 4), "Not a pbzx stream");
    uint64_t length = 0, flags = xar_read_64(&xs), last = 0;
    while (flags & 1 << 24) {
        flags = xar_read_64(&xs);
        length = xar_read_64(&xs);
        
        while (length) {
            xar_read(xbuf, min(XBSZ, (uint32_t)length), &xs);
            cpio_out(xbuf, min(XBSZ, length));
            length -= last = min(XBSZ, length);
        }
    }
    xar_extract_tostream_end(&xs);
    free(zbuf);
    xar_close(x);
    return 0;
}
