/* SPDX-License-Identifier: GPL-3.0-or-later
 * QuickTime: private serializer for single-fork PCM movie files.
 * This is not the public Apple QuickTime ABI.
 */
#ifndef QT_MOV_WRITER_H
#define QT_MOV_WRITER_H

#include <stddef.h>
#include <stdint.h>

/* Largest movie image: the bound of the PCM reader. */
#define QT_MOV_WRITE_MAX_BYTES (64u * 1024u * 1024u)

struct qt_mov_write_track {
    const void *pcm;   /* frames * channels * 2 bytes, in the byte order of format */
    uint32_t frames;   /* media samples, one media time unit each */
    uint32_t rate;     /* media time scale and integer sample rate, 1..65535 */
    uint32_t format;   /* 'twos' (big-endian) or 'sowt' (little-endian) */
    uint16_t channels; /* 1 or 2 */
    uint16_t volume;   /* track volume, 8.8 fixed point */
};

enum qt_mov_write_status {
    QT_MOV_WRITE_OK = 0,
    QT_MOV_WRITE_INVALID_ARGUMENT = -1,
    QT_MOV_WRITE_UNSUPPORTED = -2, /* outside the supported subset */
    QT_MOV_WRITE_TOO_LARGE = -3,
    QT_MOV_WRITE_NO_MEMORY = -4
};

/* Serialize a complete single-fork movie: a file type atom, one movie data
 * atom with the samples of each track in track order, then the movie atom.
 * Every track maps its whole media from time zero at normal rate, so no
 * edit list is written. A track lasts frames * movie_scale / rate movie
 * time units, rounded down, which must not be zero. Each track has one
 * version 0 sound description, one chunk and a constant sample size. On
 * success *out is a malloc'd buffer that the caller frees; the outputs are
 * unchanged on failure. */
__attribute__((visibility("hidden")))
int qt_mov_write(const struct qt_mov_write_track *tracks, size_t count,
                 int32_t movie_scale, uint8_t **out, size_t *length);

#endif
