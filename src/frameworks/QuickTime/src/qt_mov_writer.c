/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "qt_mov_writer.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Atom layouts of the QuickTime File Format: file type, movie data, movie
 * header, track header, media header, handler reference, sound media
 * information header, data information and data reference, sample
 * description, time-to-sample, sample-to-chunk, sample size and chunk
 * offset atoms. All fields are big-endian.
 * https://developer.apple.com/documentation/quicktime-file-format */

#define FCC(a, b, c, d) (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | \
                         ((uint32_t)(c) << 8) | (uint32_t)(d))

enum {
    FTYP_SIZE = 20,
    MVHD_SIZE = 108,
    TKHD_SIZE = 92,
    MDHD_SIZE = 32,
    HDLR_SIZE = 33,
    SMHD_SIZE = 16,
    DREF_SIZE = 28,
    DINF_SIZE = 8 + DREF_SIZE,
    STSD_SIZE = 52,
    STTS_SIZE = 24,
    STSC_SIZE = 28,
    STSZ_SIZE = 20,
    STCO_SIZE = 20,
    STBL_SIZE = 8 + STSD_SIZE + STTS_SIZE + STSC_SIZE + STSZ_SIZE +
                STCO_SIZE,
    MINF_SIZE = 8 + SMHD_SIZE + DINF_SIZE + STBL_SIZE,
    MDIA_SIZE = 8 + MDHD_SIZE + HDLR_SIZE + MINF_SIZE,
    TRAK_SIZE = 8 + TKHD_SIZE + MDIA_SIZE
};

struct output {
    uint8_t *p;
    size_t n;
};

static void put16(struct output *o, uint32_t v)
{
    o->p[o->n++] = (uint8_t)(v >> 8);
    o->p[o->n++] = (uint8_t)v;
}

static void put32(struct output *o, uint32_t v)
{
    o->p[o->n++] = (uint8_t)(v >> 24);
    o->p[o->n++] = (uint8_t)(v >> 16);
    o->p[o->n++] = (uint8_t)(v >> 8);
    o->p[o->n++] = (uint8_t)v;
}

static void zero(struct output *o, size_t n)
{
    memset(o->p + o->n, 0, n);
    o->n += n;
}

static void header(struct output *o, uint32_t size, uint32_t type)
{
    put32(o, size);
    put32(o, type);
}

/* Identity transformation matrix: a, b, u, c, d, v, x, y, w. */
static void matrix(struct output *o)
{
    put32(o, 0x00010000); put32(o, 0); put32(o, 0);
    put32(o, 0); put32(o, 0x00010000); put32(o, 0);
    put32(o, 0); put32(o, 0); put32(o, 0x40000000);
}

static void sound_track(struct output *o, const struct qt_mov_write_track *t,
                        uint32_t id, uint32_t duration, uint32_t offset)
{
    uint32_t stride = (uint32_t)t->channels * 2;

    header(o, TRAK_SIZE, FCC('t', 'r', 'a', 'k'));

    /* Enabled, in movie and in preview; no creation or modification time. */
    header(o, TKHD_SIZE, FCC('t', 'k', 'h', 'd'));
    put32(o, 0x7);
    zero(o, 8);
    put32(o, id);
    put32(o, 0);
    put32(o, duration);
    zero(o, 12);                /* reserved, layer, alternate group */
    put16(o, t->volume);
    put16(o, 0);
    matrix(o);
    zero(o, 8);                 /* no width or height: sound only */

    header(o, MDIA_SIZE, FCC('m', 'd', 'i', 'a'));

    header(o, MDHD_SIZE, FCC('m', 'd', 'h', 'd'));
    zero(o, 12);
    put32(o, t->rate);
    put32(o, t->frames);
    zero(o, 4);                 /* language, quality */

    /* Media handler, sound; no manufacturer and an empty counted name. */
    header(o, HDLR_SIZE, FCC('h', 'd', 'l', 'r'));
    put32(o, 0);
    put32(o, FCC('m', 'h', 'l', 'r'));
    put32(o, FCC('s', 'o', 'u', 'n'));
    zero(o, 13);

    header(o, MINF_SIZE, FCC('m', 'i', 'n', 'f'));

    header(o, SMHD_SIZE, FCC('s', 'm', 'h', 'd'));
    zero(o, 8);                 /* version, flags, balance, reserved */

    /* One data reference: the file that holds the movie (flag 0x1). */
    header(o, DINF_SIZE, FCC('d', 'i', 'n', 'f'));
    header(o, DREF_SIZE, FCC('d', 'r', 'e', 'f'));
    put32(o, 0);
    put32(o, 1);
    header(o, 12, FCC('u', 'r', 'l', ' '));
    put32(o, 1);

    header(o, STBL_SIZE, FCC('s', 't', 'b', 'l'));

    /* Version 0 sound description, uncompressed 16-bit samples. */
    header(o, STSD_SIZE, FCC('s', 't', 's', 'd'));
    put32(o, 0);
    put32(o, 1);
    header(o, 36, t->format);
    zero(o, 6);
    put16(o, 1);                /* data reference index */
    zero(o, 8);                 /* version, revision, vendor */
    put16(o, t->channels);
    put16(o, 16);
    zero(o, 4);                 /* compression ID, packet size */
    put32(o, t->rate << 16);

    header(o, STTS_SIZE, FCC('s', 't', 't', 's'));
    put32(o, 0);
    put32(o, 1);
    put32(o, t->frames);
    put32(o, 1);

    header(o, STSC_SIZE, FCC('s', 't', 's', 'c'));
    put32(o, 0);
    put32(o, 1);
    put32(o, 1);
    put32(o, t->frames);
    put32(o, 1);

    header(o, STSZ_SIZE, FCC('s', 't', 's', 'z'));
    put32(o, 0);
    put32(o, stride);
    put32(o, t->frames);

    header(o, STCO_SIZE, FCC('s', 't', 'c', 'o'));
    put32(o, 0);
    put32(o, 1);
    put32(o, offset);
}

int qt_mov_write(const struct qt_mov_write_track *tracks, size_t count,
                 int32_t movie_scale, uint8_t **out, size_t *length)
{
    uint64_t payload = 0, longest = 0;

    if (!out || !length || movie_scale <= 0 || (count && !tracks))
        return QT_MOV_WRITE_INVALID_ARGUMENT;
    if (count > QT_MOV_WRITE_MAX_BYTES / TRAK_SIZE)
        return QT_MOV_WRITE_TOO_LARGE;

    for (size_t i = 0; i < count; ++i) {
        const struct qt_mov_write_track *t = &tracks[i];
        uint64_t duration;

        if (!t->pcm)
            return QT_MOV_WRITE_INVALID_ARGUMENT;
        if (!t->frames || !t->rate || t->rate > 65535 ||
            (t->channels != 1 && t->channels != 2) ||
            (t->format != FCC('t', 'w', 'o', 's') &&
             t->format != FCC('s', 'o', 'w', 't')))
            return QT_MOV_WRITE_UNSUPPORTED;

        duration = (uint64_t)t->frames * (uint32_t)movie_scale / t->rate;
        if (!duration || duration > INT32_MAX)
            return QT_MOV_WRITE_UNSUPPORTED;
        if (duration > longest)
            longest = duration;

        payload += (uint64_t)t->frames * t->channels * 2;
        if (payload > QT_MOV_WRITE_MAX_BYTES)
            return QT_MOV_WRITE_TOO_LARGE;
    }

    uint64_t moov = 8 + MVHD_SIZE + (uint64_t)count * TRAK_SIZE;
    uint64_t total = FTYP_SIZE + (count ? 8 + payload : 0) + moov;

    if (total > QT_MOV_WRITE_MAX_BYTES)
        return QT_MOV_WRITE_TOO_LARGE;

    struct output o = { malloc((size_t)total), 0 };

    if (!o.p)
        return QT_MOV_WRITE_NO_MEMORY;

    header(&o, FTYP_SIZE, FCC('f', 't', 'y', 'p'));
    put32(&o, FCC('q', 't', ' ', ' '));
    put32(&o, 0);
    put32(&o, FCC('q', 't', ' ', ' '));

    if (count) {
        header(&o, (uint32_t)(8 + payload), FCC('m', 'd', 'a', 't'));
        for (size_t i = 0; i < count; ++i) {
            size_t bytes = (size_t)tracks[i].frames * tracks[i].channels * 2;
            memcpy(o.p + o.n, tracks[i].pcm, bytes);
            o.n += bytes;
        }
    }

    header(&o, (uint32_t)moov, FCC('m', 'o', 'o', 'v'));

    header(&o, MVHD_SIZE, FCC('m', 'v', 'h', 'd'));
    zero(&o, 12);               /* version, flags, creation, modification */
    put32(&o, (uint32_t)movie_scale);
    put32(&o, (uint32_t)longest);
    put32(&o, 0x00010000);      /* preferred rate */
    put16(&o, 0x0100);          /* preferred volume */
    zero(&o, 10);
    matrix(&o);
    zero(&o, 24);               /* preview, poster, selection, current time */
    put32(&o, (uint32_t)count + 1);

    uint32_t offset = FTYP_SIZE + 8;

    for (size_t i = 0; i < count; ++i) {
        const struct qt_mov_write_track *t = &tracks[i];
        uint32_t duration = (uint32_t)((uint64_t)t->frames *
                                       (uint32_t)movie_scale / t->rate);

        sound_track(&o, t, (uint32_t)i + 1, duration, offset);
        offset += t->frames * (uint32_t)t->channels * 2;
    }

    *out = o.p;
    *length = (size_t)total;
    return QT_MOV_WRITE_OK;
}
