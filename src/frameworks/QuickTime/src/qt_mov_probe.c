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
        return QT_MOV_TRUNCATED;
    small_length = read_be32(data);
    if (small_length == 0) {
        atom_length = remaining; /* extends to end of containing atom/file */
    } else if (small_length == 1) {
        if (remaining < 16)
            return QT_MOV_TRUNCATED;
        large_length = read_be64(data + 8);
        header_length = 16;
        if (large_length < header_length)
            return QT_MOV_BAD_ATOM;
        if (large_length > (uint64_t)remaining)
            return QT_MOV_TRUNCATED;
        atom_length = (size_t)large_length;
    } else {
        if (small_length < 8)
            return QT_MOV_BAD_ATOM;
        if ((uint64_t)small_length > (uint64_t)remaining)
            return QT_MOV_TRUNCATED;
        atom_length = (size_t)small_length;
    }
    out->length = atom_length;
    out->header_length = header_length;
    out->type = data + 4;
    return QT_MOV_OK;
}

static int parse_mvhd(const uint8_t *data, size_t length,
                      struct qt_mov_header *info)
{
    uint8_t version;
    struct qt_mov_header found = {0};
    uint32_t timescale;

    if (length < 4)
        return QT_MOV_TRUNCATED;
    version = data[0];
    if (version == 0) {
        if (length < 100) /* flags + ctime + mtime + scale + duration */
            return QT_MOV_TRUNCATED;
        timescale = read_be32(data + 12);
        found.duration = read_be32(data + 16);
    } else if (version == 1) {
        if (length < 112) /* flags + 64-bit ctime/mtime + scale + duration */
            return QT_MOV_TRUNCATED;
        timescale = read_be32(data + 20);
        found.duration = read_be64(data + 24);
    } else {
        return QT_MOV_UNSUPPORTED_VERSION;
    }
    if (timescale == 0 || timescale > INT32_MAX)
        return QT_MOV_BAD_TIMESCALE;
    found.timescale = timescale;
    found.version = version;
    *info = found;
    return QT_MOV_OK;
}

static int parse_moov(const uint8_t *data, size_t length,
                      struct qt_mov_header *info)
{
    size_t offset = 0;
    int seen_header = 0, unsupported = 0, tracks = 0;
    struct qt_mov_header found = {0};

    while (offset < length) {
        struct qt_atom atom;
        int status = parse_atom(data + offset, length - offset, &atom);

        if (status != QT_MOV_OK)
            return status;

        if (memcmp(atom.type, "mvhd", 4) == 0) {
            if (seen_header)
                return QT_MOV_BAD_ATOM;
            status = parse_mvhd(data + offset + atom.header_length,
                                atom.length - atom.header_length, &found);
            if (status != QT_MOV_OK)
                return status;
            seen_header = 1;
        } else if (memcmp(atom.type, "trak", 4) == 0) {
            tracks = 1;
        } else if (
                   memcmp(atom.type, "rmra", 4) == 0 ||
                   memcmp(atom.type, "cmov", 4) == 0) {
            unsupported = 1;
        }

        offset += atom.length;
    }

    if (!seen_header)
        return QT_MOV_NO_MOVIE_HEADER;

    found.has_tracks = tracks;
    found.has_unsupported = unsupported;
    *info = found;
    return QT_MOV_OK;
}

int qt_mov_probe(const void *data, size_t length,
                      struct qt_mov_header *result)
{
    const uint8_t *bytes = data;
    size_t offset = 0;

    if (!data || !result)
        return QT_MOV_INVALID_ARGUMENT;
    while (offset < length) {
        struct qt_atom atom;
        int status = parse_atom(bytes + offset, length - offset, &atom);
        if (status != QT_MOV_OK)
            return status;
        if (memcmp(atom.type, "moov", 4) == 0)
            return parse_moov(bytes + offset + atom.header_length,
                              atom.length - atom.header_length, result);
        offset += atom.length;
    }
    return QT_MOV_NO_MOVIE_HEADER;
}


struct span {
    const uint8_t *p;
    size_t n;
};

/* Adapt the existing atom reader, rather than maintaining
 * a second atom-size and bounds implementation. */
static int read_atom(struct span s, size_t off,
                     const uint8_t **type, struct span *body,
                     size_t *total)
{
    struct qt_atom atom;
    if (off >= s.n ||
        parse_atom(s.p+off,s.n-off,&atom)!=QT_MOV_OK)
        return QT_MOV_LIST_BAD_MOVIE;
    *type=atom.type;
    body->p=s.p+off+atom.header_length;
    body->n=atom.length-atom.header_length;
    *total=atom.length;
    return QT_MOV_LIST_OK;
}

#define TRY(expr) do { int s_=(expr); \
    if (s_!=QT_MOV_LIST_OK) return s_; } while (0)

static int find_children(struct span s, const char *type, struct span *out,
                         unsigned *count)
{
    size_t off = 0;
    *count = 0;
    while (off < s.n) {
        const uint8_t *t;
        struct span body;
        size_t total;
        int status = read_atom(s, off, &t, &body, &total);
        if (status != QT_MOV_LIST_OK)
            return status;
        if (memcmp(t, type, 4) == 0) {
            if (out)
                *out = body;
            ++*count;
        }
        off += total;
    }
    return QT_MOV_LIST_OK;
}

static int one_child(struct span s, const char *type, struct span *out)
{
    unsigned count;
    int status = find_children(s, type, out, &count);
    if (status != QT_MOV_LIST_OK)
        return status;
    if (count == 0)
        return QT_MOV_LIST_BAD_MOVIE;
    return count == 1 ? QT_MOV_LIST_OK : QT_MOV_LIST_UNSUPPORTED;
}

static void describe_track(struct span trak, struct qt_mov_track_info *info)
{
    struct span mdia, mdhd, hdlr, minf, stbl, stsz;
    uint32_t scale, size, samples;
    size_t table;

    memset(info, 0, sizeof(*info));
    if (one_child(trak, "mdia", &mdia) != QT_MOV_LIST_OK ||
        one_child(mdia, "hdlr", &hdlr) != QT_MOV_LIST_OK || hdlr.n < 12)
        return;
    info->handler = read_be32(hdlr.p + 8);
    if (one_child(mdia, "mdhd", &mdhd) != QT_MOV_LIST_OK)
        return;
    if (mdhd.n >= 20 && read_be32(mdhd.p) == 0)
        scale = read_be32(mdhd.p + 12);
    else if (mdhd.n >= 36 && read_be32(mdhd.p) == 0x01000000U)
        scale = read_be32(mdhd.p + 20);
    else
        return;
    if (scale == 0 || scale > INT32_MAX ||
        one_child(mdia, "minf", &minf) != QT_MOV_LIST_OK ||
        one_child(minf, "stbl", &stbl) != QT_MOV_LIST_OK ||
        one_child(stbl, "stsz", &stsz) != QT_MOV_LIST_OK)
        return;
    /* stsz (QTFF, Sample Size Atoms): version and flags 0, sample size,
     * number of entries, then a 4-byte size per sample only when the sample
     * size is 0. Compared by division: no overflow, no table entry read. */
    if (stsz.n < 12 || read_be32(stsz.p) != 0)
        return;
    size = read_be32(stsz.p + 4);
    samples = read_be32(stsz.p + 8);
    table = stsz.n - 12;
    if (size != 0 ? table != 0 : (table % 4 != 0 || table / 4 != samples))
        return;
    if (samples > INT32_MAX)
        return;
    info->media_time_scale = scale;
    info->sample_count = samples;
    info->has_media = 1;
}

static int walk_tracks(struct span moov, struct qt_mov_track_info *tracks,
                       size_t *count)
{
    size_t off = 0, n = 0;
    while (off < moov.n) {
        const uint8_t *t;
        struct span body;
        size_t total;
        TRY(read_atom(moov, off, &t, &body, &total));
        off += total;
        if (memcmp(t, "trak", 4) != 0)
            continue;
        if (tracks)
            describe_track(body, &tracks[n]);
        ++n;
    }
    *count = n;
    return QT_MOV_LIST_OK;
}

int qt_mov_list_tracks(const void *data, size_t length,
                            struct qt_mov_track_info *tracks,
                            size_t capacity, size_t *count)
{
    struct span file = { data, length }, moov;
    struct qt_mov_header header;
    size_t n;

    if (!data || !count || (!tracks && capacity))
        return QT_MOV_LIST_INVALID_ARGUMENT;
    if (qt_mov_probe(data, length, &header) != QT_MOV_OK)
        return QT_MOV_LIST_BAD_MOVIE;
    TRY(one_child(file, "moov", &moov));
    TRY(walk_tracks(moov, NULL, &n));
    if (tracks) {
        if (capacity < n)
            return QT_MOV_LIST_BUFFER_TOO_SMALL;
        TRY(walk_tracks(moov, tracks, &n));
    }
    *count = n;
    return QT_MOV_LIST_OK;
}
