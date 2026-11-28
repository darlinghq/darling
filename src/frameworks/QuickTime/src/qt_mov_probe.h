/* SPDX-License-Identifier: GPL-3.0-or-later
 * QuickTime: private, read-only MOV movie-header and track probe.
 * This is not the public Apple QuickTime ABI.
 */
#ifndef QT_MOV_PROBE_H
#define QT_MOV_PROBE_H

#include <stddef.h>
#include <stdint.h>

enum qt_mov_probe_status {
    QT_MOV_OK = 0,
    QT_MOV_INVALID_ARGUMENT = -1,
    QT_MOV_TRUNCATED = -2,
    QT_MOV_BAD_ATOM = -3,
    QT_MOV_NO_MOVIE_HEADER = -4,
    QT_MOV_UNSUPPORTED_VERSION = -5,
    QT_MOV_BAD_TIMESCALE = -6
};

struct qt_mov_header {
    uint32_t timescale;       /* ticks per second; strictly positive */
    uint64_t duration;        /* raw mvhd ticks; UINT32_MAX / UINT64_MAX can mean unknown */
    int has_tracks;
    int has_unsupported;
    uint8_t version;          /* mvhd version: 0 or 1 */
};

/*
 * Read the movie header (moov/mvhd) from a complete, in-memory MOV buffer.
 * Skips other atoms, supports 32-bit, extended 64-bit and size-zero atom lengths.
 * Does no file I/O, allocation, decoding, or arbitrary atom recursion.
 * The output is changed only on success. The caller retains ownership of data.
 * The buffer must remain valid and immutable for the duration of the call.
 */
__attribute__((visibility("hidden")))
int qt_mov_probe(const void *data, size_t length,
                      struct qt_mov_header *result);

enum qt_mov_list_status {
    QT_MOV_LIST_OK = 0,
    QT_MOV_LIST_INVALID_ARGUMENT = -1,
    QT_MOV_LIST_BAD_MOVIE = -2,      /* invalid, truncated or missing atoms */
    QT_MOV_LIST_UNSUPPORTED = -3,    /* valid but outside the supported subset */
    QT_MOV_LIST_BUFFER_TOO_SMALL = -4
};

struct qt_mov_track_info {
    uint32_t handler;          /* hdlr component subtype, 0 if absent */
    uint32_t media_time_scale; /* mdhd time scale, when has_media */
    uint32_t sample_count;     /* stsz sample count, when has_media */
    int has_media;
};

__attribute__((visibility("hidden")))
int qt_mov_list_tracks(const void *data, size_t length,
                           struct qt_mov_track_info *tracks,
                           size_t capacity, size_t *count);


enum qt_mov_pcm_status {
    QT_MOV_PCM_OK = 0,
    QT_MOV_PCM_INVALID_ARGUMENT = -1,
    QT_MOV_PCM_BAD_MOVIE = -2,      /* invalid, truncated or missing atoms */
    QT_MOV_PCM_NO_SOUND_TRACK = -3,
    QT_MOV_PCM_UNSUPPORTED = -4,    /* valid but outside the supported subset */
    QT_MOV_PCM_INCONSISTENT = -5,   /* description and sample tables disagree */
    QT_MOV_PCM_OUT_OF_BOUNDS = -6,  /* samples not inside one mdat payload */
    QT_MOV_PCM_BUFFER_TOO_SMALL = -7
};

struct qt_mov_pcm_info {
    uint32_t format;          /* 'twos' (big-endian) or 'sowt' (little-endian) */
    uint32_t sample_rate;     /* Hz, integer part of the 16.16 description rate */
    uint16_t channels;        /* 1 or 2, interleaved */
    uint16_t bits_per_sample; /* 16 */
    uint32_t frame_count;     /* media samples, one sample frame each */
    size_t byte_count;        /* bytes copied to the destination */
};

__attribute__((visibility("hidden")))
int qt_mov_find_pcm(const void *data, size_t length,
                         struct qt_mov_pcm_info *info,
                         size_t *offset);

__attribute__((visibility("hidden")))
int qt_mov_extract_pcm(const void *data, size_t length,
                            void *pcm, size_t capacity,
                            struct qt_mov_pcm_info *info);


#endif
