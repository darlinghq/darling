/* SPDX-License-Identifier: GPL-3.0-or-later
 * D4rling QuickTime: private, read-only MOV movie-header probe.
 * This is not the public Apple QuickTime ABI.
 */
#ifndef D4RL_QT_MOV_PROBE_H
#define D4RL_QT_MOV_PROBE_H

#include <stddef.h>
#include <stdint.h>

enum d4rl_qt_mov_probe_status {
    D4RL_QT_MOV_OK = 0,
    D4RL_QT_MOV_INVALID_ARGUMENT = -1,
    D4RL_QT_MOV_TRUNCATED = -2,
    D4RL_QT_MOV_BAD_ATOM = -3,
    D4RL_QT_MOV_NO_MOVIE_HEADER = -4,
    D4RL_QT_MOV_UNSUPPORTED_VERSION = -5,
    D4RL_QT_MOV_BAD_TIMESCALE = -6
};

struct d4rl_qt_mov_header {
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
int d4rl_qt_mov_probe(const void *data, size_t length,
                      struct d4rl_qt_mov_header *result);

enum d4rl_qt_mov_pcm_status {
    D4RL_QT_PCM_OK = 0,
    D4RL_QT_PCM_INVALID_ARGUMENT = -1,
    D4RL_QT_PCM_BAD_MOVIE = -2,      /* invalid, truncated or missing atoms */
    D4RL_QT_PCM_NO_SOUND_TRACK = -3,
    D4RL_QT_PCM_UNSUPPORTED = -4,    /* valid but outside the supported subset */
    D4RL_QT_PCM_INCONSISTENT = -5,   /* description and sample tables disagree */
    D4RL_QT_PCM_OUT_OF_BOUNDS = -6,  /* samples not inside one mdat payload */
    D4RL_QT_PCM_BUFFER_TOO_SMALL = -7
};

struct d4rl_qt_mov_track_info {
    uint32_t handler;          /* hdlr component subtype, 0 if absent */
    uint32_t media_time_scale; /* mdhd time scale, when has_media */
    uint32_t sample_count;     /* stsz sample count, when has_media */
    int has_media;
};

__attribute__((visibility("hidden")))
int d4rl_qt_mov_list_tracks(const void *data, size_t length,
                           struct d4rl_qt_mov_track_info *tracks,
                           size_t capacity, size_t *count);


#endif
