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
    uint32_t handler_manufacturer; /* hdlr component manufacturer, 0 if absent */
    uint8_t handler_name[256];     /* hdlr counted component name, empty if absent */
    int enabled;               /* tkhd flag 0x1; 0 without a track header */
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
    QT_MOV_PCM_OUT_OF_BOUNDS = -6,  /* chunk range outside an mdat payload */
    QT_MOV_PCM_BUFFER_TOO_SMALL = -7,
    QT_MOV_PCM_NO_MEMORY = -8
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


/* Allocate and assemble the supported PCM into a caller-owned buffer.
 * The caller frees the returned buffer with free().
 * Outputs change only on success; the source must remain immutable.
 */
__attribute__((visibility("hidden")))
int qt_mov_load_pcm(const void *data, size_t length,
                    void **pcm, struct qt_mov_pcm_info *info);

enum qt_sound_extension_status {
    QT_SOUND_EXTENSION_OK = 0,
    QT_SOUND_EXTENSION_INVALID_ARGUMENT = -1,
    QT_SOUND_EXTENSION_BAD_DESCRIPTION = -2,
    QT_SOUND_EXTENSION_NOT_FOUND = -3
};

/* Find the extension atom of the given type that follows the fixed part of
 * an in-memory sound description (QTFF, Sound Sample Descriptions: 36 bytes
 * for version 0, 52 for version 1, 72 for version 2). desc_size and version
 * are the description's own fields, read by the caller in native byte
 * order; extension atoms are big-endian. Only [fixed part, desc_size) is
 * examined and desc_size must not exceed size. A tail shorter than an atom
 * header ends the list. On success *offset and *length give the payload of
 * the atom within desc; the outputs are unchanged on failure. */
__attribute__((visibility("hidden")))
int qt_sound_description_find_extension(const void *desc, size_t size,
                                        uint32_t desc_size, int version,
                                        uint32_t type, size_t *offset,
                                        size_t *length);

#endif
