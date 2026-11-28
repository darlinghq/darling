/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "qt_mov_probe.h"

#include <limits.h>
#include <stdlib.h>
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



/* Translate track-list parser errors into PCM parser errors. */
static int pcm_atom_status(int status)
{
    if (status == QT_MOV_LIST_OK)
        return QT_MOV_PCM_OK;
    if (status == QT_MOV_LIST_UNSUPPORTED)
        return QT_MOV_PCM_UNSUPPORTED;
    return QT_MOV_PCM_BAD_MOVIE;
}

static int pcm_read_atom(struct span s, size_t off,
                         const uint8_t **type, struct span *body,
                         size_t *total)
{
    return pcm_atom_status(read_atom(s,off,type,body,total));
}

static int pcm_one_child(struct span s, const char *type,
                         struct span *out)
{
    return pcm_atom_status(one_child(s,type,out));
}

static int pcm_find_children(struct span s, const char *type,
                             struct span *out, unsigned *count)
{
    return pcm_atom_status(find_children(s,type,out,count));
}

#define PCM_TRY(expr) do { int status_=(expr); \
    if (status_!=QT_MOV_PCM_OK) return status_; } while (0)




static uint16_t read_be16(const uint8_t *p)
{
    return ((uint16_t)p[0]<<8) | p[1];
}

static int table(struct span s, size_t entry_size, uint32_t *entries)
{
    if (s.n < 8)
        return QT_MOV_PCM_BAD_MOVIE;
    if (read_be32(s.p) != 0)
        return QT_MOV_PCM_UNSUPPORTED;
    *entries = read_be32(s.p + 4);
    if (entry_size &&
        ((s.n - 8) % entry_size != 0 ||
         (s.n - 8) / entry_size != *entries))
        return QT_MOV_PCM_BAD_MOVIE;
    return QT_MOV_PCM_OK;
}


/* The 1-based stsd data reference index must select a self-contained entry
 * of minf/dinf/dref (QTFF, Data Reference Atoms: self reference flag 0x0001,
 * media data in the same file as the movie atom). External aliases, URLs and
 * resources are never followed; their samples are not copied. */
static int check_local_reference(struct span minf, uint16_t index)
{
    struct span dinf, dref, entries, body;
    const uint8_t *type, *selected = NULL;
    uint32_t count, n = 0, version_flags = 0;
    size_t off = 0, total;

    if (index == 0)
        return QT_MOV_PCM_BAD_MOVIE;
    PCM_TRY(pcm_one_child(minf, "dinf", &dinf));
    PCM_TRY(pcm_one_child(dinf, "dref", &dref));
    if (dref.n < 8)
        return QT_MOV_PCM_BAD_MOVIE;
    if (read_be32(dref.p) != 0)
        return QT_MOV_PCM_UNSUPPORTED;
    count = read_be32(dref.p + 4);
    entries.p = dref.p + 8;
    entries.n = dref.n - 8;
    while (off < entries.n) {
        PCM_TRY(pcm_read_atom(entries, off, &type, &body, &total));
        if (body.n < 4)
            return QT_MOV_PCM_BAD_MOVIE;
        if (++n == index) {
            selected = type;
            version_flags = read_be32(body.p);
        }
        off += total;
    }
    if (n != count || !selected)
        return QT_MOV_PCM_BAD_MOVIE;
    if (version_flags != 0x000001)
        return QT_MOV_PCM_UNSUPPORTED;
    if (memcmp(selected, "alis", 4) != 0 && memcmp(selected, "url ", 4) != 0)
        return QT_MOV_PCM_UNSUPPORTED;
    return QT_MOV_PCM_OK;
}

static int find_sound_media(struct span moov, struct span *mdia)
{
    size_t off = 0;
    unsigned sound = 0;
    while (off < moov.n) {
        const uint8_t *t;
        struct span body, media, hdlr;
        size_t total;
        PCM_TRY(pcm_read_atom(moov, off, &t, &body, &total));
        off += total;
        if (memcmp(t, "trak", 4) != 0)
            continue;
        PCM_TRY(pcm_one_child(body, "mdia", &media));
        PCM_TRY(pcm_one_child(media, "hdlr", &hdlr));
        if (hdlr.n < 12)
            return QT_MOV_PCM_BAD_MOVIE;
        if (memcmp(hdlr.p + 8, "soun", 4) == 0) {
            *mdia = media;
            ++sound;
        }
    }
    if (sound == 0)
        return QT_MOV_PCM_NO_SOUND_TRACK;
    return sound == 1 ? QT_MOV_PCM_OK : QT_MOV_PCM_UNSUPPORTED;
}


/* Bounded local PCM import. Chunk offsets are absolute file offsets
 * (QTFF stco/co64); stsc describes runs of consecutive logical chunks.
 * https://developer.apple.com/documentation/quicktime-file-format
 *
 * Resource limits apply to this in-memory importer, not to the format.
 */
#define PCM_MAX_BYTES (64u * 1024u * 1024u)
#define PCM_MAX_CHUNKS 262144u

struct pcm_layout {
    struct qt_mov_pcm_info info;
    struct span mapping;
    struct span offsets;
    uint32_t runs;
    uint32_t chunks;
    unsigned wide;
};

struct pcm_chunk {
    uint64_t offset;
    size_t bytes;
    size_t destination;
};

static int locate(const void *data, size_t length,
                  struct pcm_layout *layout)
{
    struct span file = { data, length }, moov, mdia, mdhd, minf, stbl;
    struct span stsd, entries, entry, stts, stsc, stsz, stco;
    struct qt_mov_header header;
    struct qt_mov_pcm_info found;
    const uint8_t *format;
    size_t total;
    uint32_t count, timescale, duration, rate, frames, frame_bytes;
    uint64_t bytes;

    if (qt_mov_probe(data, length, &header) != QT_MOV_OK)
        return QT_MOV_PCM_BAD_MOVIE;
    PCM_TRY(pcm_one_child(file, "moov", &moov));
    PCM_TRY(find_sound_media(moov, &mdia));

    PCM_TRY(pcm_one_child(mdia, "mdhd", &mdhd));
    if (mdhd.n < 20)
        return QT_MOV_PCM_BAD_MOVIE;
    if (read_be32(mdhd.p) != 0)
        return QT_MOV_PCM_UNSUPPORTED;
    timescale = read_be32(mdhd.p + 12);
    duration = read_be32(mdhd.p + 16);
    PCM_TRY(pcm_one_child(mdia, "minf", &minf));
    PCM_TRY(pcm_one_child(minf, "stbl", &stbl));

    /* Sound description version 0: 36-byte entry (QTFF, Sound Media). */
    PCM_TRY(pcm_one_child(stbl, "stsd", &stsd));
    PCM_TRY(table(stsd, 0, &count));
    if (count != 1)
        return QT_MOV_PCM_UNSUPPORTED;
    entries.p = stsd.p + 8;
    entries.n = stsd.n - 8;
    PCM_TRY(pcm_read_atom(entries, 0, &format, &entry, &total));
    if (total != entries.n || entry.n < 28)
        return QT_MOV_PCM_BAD_MOVIE;
    if (entry.p[0] | entry.p[1] | entry.p[2] | entry.p[3] | entry.p[4] | entry.p[5])
        return QT_MOV_PCM_BAD_MOVIE; /* six reserved bytes must be 0 */
    if (memcmp(format, "twos", 4) != 0 && memcmp(format, "sowt", 4) != 0)
        return QT_MOV_PCM_UNSUPPORTED;
    if (entry.n != 28 || total != 36 ||
        read_be16(entry.p + 6) != 1 ||
        read_be16(entry.p + 10) != 0 ||
        read_be32(entry.p + 12) != 0 ||
        read_be16(entry.p + 22) != 0)
        return QT_MOV_PCM_UNSUPPORTED;

    found.format = read_be32(format);
    found.channels = read_be16(entry.p + 16);
    found.bits_per_sample = read_be16(entry.p + 18);
    rate = read_be32(entry.p + 24);
    if (read_be16(entry.p + 8) != 0 || read_be16(entry.p + 20) != 0 ||
        found.bits_per_sample != 16 || found.channels < 1 ||
        found.channels > 2 || (rate & 0xFFFF) != 0 || rate == 0)
        return QT_MOV_PCM_UNSUPPORTED;
    found.sample_rate = rate >> 16;
    frame_bytes = (uint32_t)found.channels * 2;
    PCM_TRY(check_local_reference(minf, read_be16(entry.p + 6)));


    PCM_TRY(pcm_one_child(stbl, "stts", &stts));
    PCM_TRY(table(stts, 8, &count));
    if (count != 1)
        return QT_MOV_PCM_UNSUPPORTED;
    frames = read_be32(stts.p + 8);

    PCM_TRY(pcm_one_child(stbl, "stsc", &stsc));
    PCM_TRY(table(stsc, 12, &count));
    uint32_t runs = count;

    PCM_TRY(pcm_one_child(stbl, "stsz", &stsz));
    if (stsz.n != 12)
        return QT_MOV_PCM_UNSUPPORTED;

    if (read_be32(stsz.p) != 0 ||
        read_be32(stsz.p + 4) != frame_bytes)
        return QT_MOV_PCM_UNSUPPORTED;

    unsigned narrow_count, wide_count;

    PCM_TRY(pcm_find_children(stbl, "stco", &stco, &narrow_count));
    PCM_TRY(pcm_find_children(stbl, "co64", &stco, &wide_count));

    if (narrow_count + wide_count != 1)
        return QT_MOV_PCM_BAD_MOVIE;

    PCM_TRY(table(stco, wide_count ? 8 : 4, &count));
    uint32_t chunks = count;

    bytes = (uint64_t)frames * frame_bytes;

    if (!frames || frames > INT32_MAX ||
        bytes > PCM_MAX_BYTES || bytes > SIZE_MAX ||
        !chunks || chunks > PCM_MAX_CHUNKS ||
        !runs || runs > chunks)
        return QT_MOV_PCM_UNSUPPORTED;

    if (read_be32(stts.p + 12) != 1 ||
        timescale != found.sample_rate ||
        duration != frames ||
        read_be32(stsz.p + 8) != frames)
        return QT_MOV_PCM_INCONSISTENT;

    /* Check all stsc runs before expanding any chunk. */
    uint64_t sample_total = 0;

    for (uint32_t i = 0; i < runs; ++i) {
        const uint8_t *row = stsc.p + 8 + (size_t)i * 12;

        uint32_t first = read_be32(row);
        uint32_t samples = read_be32(row + 4);
        uint32_t description = read_be32(row + 8);

        uint32_t next = i + 1 < runs
            ? read_be32(row + 12) : chunks + 1;

        if ((i == 0 && first != 1) ||
            first == 0 || first > chunks ||
            next <= first || next > chunks + 1 ||
            samples == 0)
            return QT_MOV_PCM_INCONSISTENT;

        if (description != 1)
            return QT_MOV_PCM_UNSUPPORTED;

        uint64_t run_samples =
            (uint64_t)(next - first) * samples;

        if (run_samples > frames - sample_total)
            return QT_MOV_PCM_INCONSISTENT;

        sample_total += run_samples;
    }

    if (sample_total != frames)
        return QT_MOV_PCM_INCONSISTENT;

    found.frame_count = frames;
    found.byte_count = (size_t)bytes;

    layout->info = found;
    layout->mapping = stsc;
    layout->offsets = stco;
    layout->runs = runs;
    layout->chunks = chunks;
    layout->wide = wide_count != 0;

    return QT_MOV_PCM_OK;
}


static int chunk_order(const void *a, const void *b)
{
    const struct pcm_chunk *x = a, *y = b;

    if (x->offset < y->offset)
        return -1;
    if (x->offset > y->offset)
        return 1;
    return 0;
}

/* Validate the complete set of source ranges before copying.
 * Sort by physical position for a single forward scan of the file.
 * destination retains the original logical sample order.
 */
static int build_pcm_plan(struct span file,
                          const struct pcm_layout *layout,
                          struct pcm_chunk **result)
{
    size_t n = layout->chunks;
    size_t frame_bytes = (size_t)layout->info.channels * 2;

    if (n > SIZE_MAX / sizeof(struct pcm_chunk))
        return QT_MOV_PCM_UNSUPPORTED;

    struct pcm_chunk *chunks = malloc(n * sizeof(*chunks));
    if (!chunks)
        return QT_MOV_PCM_NO_MEMORY;

    size_t destination = 0;
    uint32_t run = 0;
    const uint8_t *row = layout->mapping.p + 8;
    size_t width = layout->wide ? 8 : 4;

    for (uint32_t i = 0; i < layout->chunks; ++i) {
        if (run + 1 < layout->runs &&
            i + 1 == read_be32(row + 12)) {
            ++run;
            row += 12;
        }

        const uint8_t *entry =
            layout->offsets.p + 8 + (size_t)i * width;

        size_t bytes = (size_t)read_be32(row + 4) * frame_bytes;

        if (bytes > layout->info.byte_count - destination) {
            free(chunks);
            return QT_MOV_PCM_INCONSISTENT;
        }

        chunks[i].offset = layout->wide
            ? read_be64(entry) : read_be32(entry);

        chunks[i].bytes = bytes;
        chunks[i].destination = destination;
        destination += bytes;
    }

    if (destination != layout->info.byte_count) {
        free(chunks);
        return QT_MOV_PCM_INCONSISTENT;
    }

    qsort(chunks, n, sizeof(*chunks), chunk_order);

    size_t atom_offset = 0;
    uint64_t previous_end = 0;
    int status = QT_MOV_PCM_OK;

    for (size_t i = 0; i < n; ++i) {
        uint64_t start = chunks[i].offset;
        uint64_t bytes = chunks[i].bytes;

        if (start > file.n || bytes > file.n - start) {
            status = QT_MOV_PCM_OUT_OF_BOUNDS;
            break;
        }

        /* Overlapping or reused ranges are outside this bounded subset. */
        if (i && start < previous_end) {
            status = QT_MOV_PCM_UNSUPPORTED;
            break;
        }

        int found = 0;

        while (atom_offset < file.n) {
            const uint8_t *type;
            struct span body;
            size_t total;

            status = pcm_read_atom(
                file, atom_offset, &type, &body, &total);

            if (status != QT_MOV_PCM_OK)
                break;

            uint64_t end = (uint64_t)atom_offset + total;

            if (start >= end) {
                atom_offset += total;
                continue;
            }

            uint64_t payload = (uint64_t)(body.p - file.p);

            if (memcmp(type, "mdat", 4) == 0 &&
                start >= payload && bytes <= end - start)
                found = 1;

            break;
        }

        if (status != QT_MOV_PCM_OK)
            break;

        if (!found) {
            status = QT_MOV_PCM_OUT_OF_BOUNDS;
            break;
        }

        previous_end = start + bytes;
    }

    if (status != QT_MOV_PCM_OK) {
        free(chunks);
        return status;
    }

    *result = chunks;
    return QT_MOV_PCM_OK;
}

static void copy_pcm_plan(const uint8_t *source, uint8_t *destination,
                          const struct pcm_chunk *chunks, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        memcpy(destination + chunks[i].destination,
               source + (size_t)chunks[i].offset, chunks[i].bytes);
}

static int prepare_pcm(const void *data, size_t length,
                       struct pcm_layout *layout,
                       struct pcm_chunk **chunks)
{
    PCM_TRY(locate(data, length, layout));

    struct span file = { data, length };

    return build_pcm_plan(file, layout, chunks);
}

/* Preserve the private contiguous-region query's original meaning. */
int qt_mov_find_pcm(const void *data, size_t length,
                    struct qt_mov_pcm_info *info, size_t *offset)
{
    struct pcm_layout layout;
    struct pcm_chunk *chunks;

    if (!data || !info || !offset)
        return QT_MOV_PCM_INVALID_ARGUMENT;

    PCM_TRY(prepare_pcm(data, length, &layout, &chunks));

    uint64_t first = chunks[0].offset;
    int contiguous = 1;

    for (size_t i = 0; i < layout.chunks; ++i) {
        if (chunks[i].offset - first != chunks[i].destination) {
            contiguous = 0;
            break;
        }
    }

    free(chunks);

    if (!contiguous)
        return QT_MOV_PCM_UNSUPPORTED;

    *info = layout.info;
    *offset = (size_t)first;

    return QT_MOV_PCM_OK;
}

int qt_mov_extract_pcm(const void *data, size_t length,
                       void *pcm, size_t capacity,
                       struct qt_mov_pcm_info *info)
{
    struct pcm_layout layout;
    struct pcm_chunk *chunks;

    if (!data || !info || (!pcm && capacity))
        return QT_MOV_PCM_INVALID_ARGUMENT;

    PCM_TRY(prepare_pcm(data, length, &layout, &chunks));

    if (layout.info.byte_count > capacity) {
        free(chunks);
        return QT_MOV_PCM_BUFFER_TOO_SMALL;
    }

    copy_pcm_plan(data, pcm, chunks, layout.chunks);

    free(chunks);
    *info = layout.info;

    return QT_MOV_PCM_OK;
}

/* Transfer ownership of the assembled PCM only after full validation. */
int qt_mov_load_pcm(const void *data, size_t length,
                    void **pcm, struct qt_mov_pcm_info *info)
{
    struct pcm_layout layout;
    struct pcm_chunk *chunks;

    if (!data || !pcm || !info)
        return QT_MOV_PCM_INVALID_ARGUMENT;

    PCM_TRY(prepare_pcm(data, length, &layout, &chunks));

    void *buffer = malloc(layout.info.byte_count);

    if (!buffer) {
        free(chunks);
        return QT_MOV_PCM_NO_MEMORY;
    }

    copy_pcm_plan(data, buffer, chunks, layout.chunks);

    free(chunks);

    *pcm = buffer;
    *info = layout.info;

    return QT_MOV_PCM_OK;
}
