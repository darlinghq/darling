/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "qt_mov_probe.h"

#include <limits.h>
#include <string.h>

struct qt_atom {
    size_t length;
    size_t header_length;
    const uint8_t *type;
};

static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint64_t read_be64(const uint8_t *p)
{
    return ((uint64_t)read_be32(p) << 32) | read_be32(p + 4);
}

/* Atom lengths are bounded by the containing buffer, before any pointer advance. */
static int parse_atom(const uint8_t *data, size_t remaining, struct qt_atom *out)
{
    uint32_t small_length;
    uint64_t large_length;
    size_t atom_length, header_length = 8;

    if (remaining < 8)
        return D4RL_QT_MOV_TRUNCATED;
    small_length = read_be32(data);
    if (small_length == 0) {
        atom_length = remaining; /* extends to end of containing atom/file */
    } else if (small_length == 1) {
        if (remaining < 16)
            return D4RL_QT_MOV_TRUNCATED;
        large_length = read_be64(data + 8);
        header_length = 16;
        if (large_length < header_length)
            return D4RL_QT_MOV_BAD_ATOM;
        if (large_length > (uint64_t)remaining)
            return D4RL_QT_MOV_TRUNCATED;
        atom_length = (size_t)large_length;
    } else {
        if (small_length < 8)
            return D4RL_QT_MOV_BAD_ATOM;
        if ((uint64_t)small_length > (uint64_t)remaining)
            return D4RL_QT_MOV_TRUNCATED;
        atom_length = (size_t)small_length;
    }
    out->length = atom_length;
    out->header_length = header_length;
    out->type = data + 4;
    return D4RL_QT_MOV_OK;
}

static int parse_mvhd(const uint8_t *data, size_t length,
                      struct d4rl_qt_mov_header *info)
{
    uint8_t version;
    struct d4rl_qt_mov_header found = {0};
    uint32_t timescale;

    if (length < 4)
        return D4RL_QT_MOV_TRUNCATED;
    version = data[0];
    if (version == 0) {
        if (length < 100) /* flags + ctime + mtime + scale + duration */
            return D4RL_QT_MOV_TRUNCATED;
        timescale = read_be32(data + 12);
        found.duration = read_be32(data + 16);
    } else if (version == 1) {
        if (length < 112) /* flags + 64-bit ctime/mtime + scale + duration */
            return D4RL_QT_MOV_TRUNCATED;
        timescale = read_be32(data + 20);
        found.duration = read_be64(data + 24);
    } else {
        return D4RL_QT_MOV_UNSUPPORTED_VERSION;
    }
    if (timescale == 0 || timescale > INT32_MAX)
        return D4RL_QT_MOV_BAD_TIMESCALE;
    found.timescale = timescale;
    found.version = version;
    *info = found;
    return D4RL_QT_MOV_OK;
}

static int parse_moov(const uint8_t *data, size_t length,
                      struct d4rl_qt_mov_header *info)
{
    size_t offset = 0;
    int seen_header = 0, unsupported = 0;
    struct d4rl_qt_mov_header found = {0};

    while (offset < length) {
        struct qt_atom atom;
        int status = parse_atom(data + offset, length - offset, &atom);

        if (status != D4RL_QT_MOV_OK)
            return status;

        if (memcmp(atom.type, "mvhd", 4) == 0) {
            if (seen_header)
                return D4RL_QT_MOV_BAD_ATOM;
            status = parse_mvhd(data + offset + atom.header_length,
                                atom.length - atom.header_length, &found);
            if (status != D4RL_QT_MOV_OK)
                return status;
            seen_header = 1;
        } else if (memcmp(atom.type, "trak", 4) == 0 ||
                   memcmp(atom.type, "rmra", 4) == 0 ||
                   memcmp(atom.type, "cmov", 4) == 0) {
            unsupported = 1;
        }

        offset += atom.length;
    }

    if (!seen_header)
        return D4RL_QT_MOV_NO_MOVIE_HEADER;

    found.has_tracks = unsupported;
    *info = found;
    return D4RL_QT_MOV_OK;
}

int d4rl_qt_mov_probe(const void *data, size_t length,
                      struct d4rl_qt_mov_header *result)
{
    const uint8_t *bytes = data;
    size_t offset = 0;

    if (!data || !result)
        return D4RL_QT_MOV_INVALID_ARGUMENT;
    while (offset < length) {
        struct qt_atom atom;
        int status = parse_atom(bytes + offset, length - offset, &atom);
        if (status != D4RL_QT_MOV_OK)
            return status;
        if (memcmp(atom.type, "moov", 4) == 0)
            return parse_moov(bytes + offset + atom.header_length,
                              atom.length - atom.header_length, result);
        offset += atom.length;
    }
    return D4RL_QT_MOV_NO_MOVIE_HEADER;
}
