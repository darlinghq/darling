/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Creating, saving and reopening single-fork PCM movies. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PCM_NATIVE
#include "qt_mov_probe.h"
#include "qt_mov_writer.h"
#else
#include <QuickTime/QuickTime.h>
#include <CarbonCore/Files.h>
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/MacErrors.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

static unsigned checks, failures;

#define CHECK(x) do { ++checks; if (!(x)) { \
    ++failures; printf("FAIL %d %s\n", __LINE__, #x); \
} } while (0)

#define TWOS 0x74776f73u
#define SOWT 0x736f7774u
#define NFRAMES 4800u

/* Stereo 16-bit little-endian pattern, distinct for each track k. */
static unsigned sample_value(unsigned k, unsigned i, unsigned c)
{
    i %= 160u;
    unsigned v = c ? (i * (k + 3u) + 11u * k) % 160u
                   : (i * (k + 1u) + 7u * k) % 160u;
    return (unsigned)(((int)v - 80) * 16) & 65535u;
}

static void fill_stereo(unsigned char *p, unsigned k)
{
    for (unsigned i = 0; i < NFRAMES; ++i)
        for (unsigned c = 0; c < 2; ++c) {
            unsigned v = sample_value(k, i, c);
            p[i * 4 + c * 2] = (unsigned char)v;
            p[i * 4 + c * 2 + 1] = (unsigned char)(v >> 8);
        }
}

static unsigned char pattern(unsigned seed, size_t i)
{
    return (unsigned char)((i * 37u + seed * 11u + (i >> 5)) & 255u);
}

struct format_case {
    uint32_t format;
    unsigned channels, rate;
    int movie_scale;            /* 0: the default movie time scale */
    unsigned blocks[3];         /* samples added per AddMediaSample call */
    int explicit_start;         /* second insertion at the computed end */
};

static const struct format_case format_cases[] = {
    { TWOS, 1, 8000, 0, { 1000, 2000, 3000 }, 0 },
    { SOWT, 2, 22050, 22050, { 4410, 4410, 1 }, 1 },
    { TWOS, 2, 44100, 44100, { 11025, 7, 100 }, 0 },
    { SOWT, 1, 48000, 1000, { 960, 960, 480 }, 1 },
    { TWOS, 1, 11025, 0, { 3001, 17, 1 }, 0 },
    { SOWT, 2, 32000, 600, { 32000, 16000, 3 }, 0 },
};

#define FORMAT_COUNT (sizeof(format_cases) / sizeof(*format_cases))

static unsigned case_frames(const struct format_case *c)
{
    return c->blocks[0] + c->blocks[1] + c->blocks[2];
}

#ifdef PCM_NATIVE

static void test_cp27_layout(void)
{
    static unsigned char pcm[2][NFRAMES * 4];
    struct qt_mov_write_track t[2];

    for (unsigned k = 0; k < 2; ++k) {
        fill_stereo(pcm[k], k);
        t[k] = (struct qt_mov_write_track){ pcm[k], NFRAMES, 48000,
                                            SOWT, 2, 0x100 };
    }

    for (unsigned count = 1; count <= 2; ++count) {
        uint8_t *out = NULL;
        size_t length = 0;

        CHECK(qt_mov_write(t, count, 48000, &out, &length)
              == QT_MOV_WRITE_OK);
        CHECK(length == 20 + 8 + count * NFRAMES * 4 + 8 + 108 + count * 385);

        struct qt_mov_header h;
        struct qt_mov_track_info info[2];
        size_t n = 0;

        CHECK(qt_mov_probe(out, length, &h) == QT_MOV_OK);
        CHECK(h.timescale == 48000 && h.duration == NFRAMES);
        CHECK(qt_mov_list_tracks(out, length, info, 2, &n)
              == QT_MOV_LIST_OK);
        CHECK(n == count);
        for (size_t i = 0; i < n; ++i) {
            CHECK(info[i].handler == 0x736f756eu);
            CHECK(info[i].media_time_scale == 48000);
            CHECK(info[i].sample_count == NFRAMES);
            CHECK(info[i].enabled == 1);
            CHECK(info[i].handler_manufacturer == 0);
            CHECK(info[i].handler_name[0] == 0);
        }

        if (count == 1) {
            static unsigned char back[NFRAMES * 4];
            struct qt_mov_pcm_info pi;

            CHECK(qt_mov_extract_pcm(out, length, back, sizeof(back), &pi)
                  == QT_MOV_PCM_OK);
            CHECK(pi.format == SOWT && pi.channels == 2 &&
                  pi.sample_rate == 48000 && pi.frame_count == NFRAMES);
            CHECK(memcmp(back, pcm[0], sizeof(back)) == 0);
        }
        free(out);
    }
}

static void test_formats(void)
{
    for (size_t i = 0; i < FORMAT_COUNT; ++i) {
        const struct format_case *c = &format_cases[i];
        unsigned frames = case_frames(c);
        size_t bytes = (size_t)frames * c->channels * 2;
        unsigned char *pcm = malloc(bytes);
        unsigned char *back = malloc(bytes);
        int32_t scale = c->movie_scale ? c->movie_scale : 600;

        CHECK(pcm && back);
        if (!pcm || !back) {
            free(pcm);
            free(back);
            continue;
        }
        for (size_t j = 0; j < bytes; ++j)
            pcm[j] = pattern((unsigned)i, j);

        struct qt_mov_write_track t = { pcm, frames, c->rate, c->format,
                                        (uint16_t)c->channels, 0x100 };
        uint8_t *out = NULL;
        size_t length = 0;
        struct qt_mov_header h;
        struct qt_mov_pcm_info pi;

        CHECK(qt_mov_write(&t, 1, scale, &out, &length) == QT_MOV_WRITE_OK);
        CHECK(qt_mov_probe(out, length, &h) == QT_MOV_OK);
        CHECK(h.duration == (uint64_t)frames * (uint32_t)scale / c->rate);
        CHECK(qt_mov_extract_pcm(out, length, back, bytes, &pi)
              == QT_MOV_PCM_OK);
        CHECK(pi.format == c->format && pi.channels == c->channels &&
              pi.sample_rate == c->rate && pi.frame_count == frames);
        CHECK(memcmp(back, pcm, bytes) == 0);
        free(out);
        free(pcm);
        free(back);
    }
}

static void test_invalid(void)
{
    static unsigned char pcm[400];
    const struct qt_mov_write_track good = { pcm, 100, 8000, TWOS, 1, 0x100 };
    struct qt_mov_write_track t;
    uint8_t *out = (uint8_t *)pcm;
    size_t length = 77;

    CHECK(qt_mov_write(&good, 1, 600, NULL, &length)
          == QT_MOV_WRITE_INVALID_ARGUMENT);
    CHECK(qt_mov_write(&good, 1, 600, &out, NULL)
          == QT_MOV_WRITE_INVALID_ARGUMENT);
    CHECK(qt_mov_write(NULL, 1, 600, &out, &length)
          == QT_MOV_WRITE_INVALID_ARGUMENT);
    CHECK(qt_mov_write(&good, 1, 0, &out, &length)
          == QT_MOV_WRITE_INVALID_ARGUMENT);

    t = good; t.pcm = NULL;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length)
          == QT_MOV_WRITE_INVALID_ARGUMENT);
    t = good; t.frames = 0;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_UNSUPPORTED);
    t = good; t.rate = 0;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_UNSUPPORTED);
    t = good; t.rate = 65536;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_UNSUPPORTED);
    t = good; t.channels = 3;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_UNSUPPORTED);
    t = good; t.format = 0x72617720u;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_UNSUPPORTED);
    /* One sample at 48 kHz lasts no whole unit of a 600 time scale. */
    t = good; t.frames = 1; t.rate = 48000;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_UNSUPPORTED);
    /* Too large: rejected before any allocation or copy. */
    t = good; t.frames = 0x7fffffffu;
    CHECK(qt_mov_write(&t, 1, 600, &out, &length) == QT_MOV_WRITE_TOO_LARGE);
    CHECK(qt_mov_write(&good, 0x7fffffff, 600, &out, &length)
          == QT_MOV_WRITE_TOO_LARGE);
    CHECK(out == (uint8_t *)pcm && length == 77);

    /* A movie without tracks: file type and movie atoms only. */
    struct qt_mov_header h;
    size_t n = 9;

    CHECK(qt_mov_write(NULL, 0, 600, &out, &length) == QT_MOV_WRITE_OK);
    CHECK(length == 20 + 8 + 108);
    CHECK(qt_mov_probe(out, length, &h) == QT_MOV_OK);
    CHECK(h.timescale == 600 && h.duration == 0 && !h.has_tracks);
    CHECK(qt_mov_list_tracks(out, length, NULL, 0, &n) == QT_MOV_LIST_OK);
    CHECK(n == 0);
    free(out);
}

int main(void)
{
    test_cp27_layout();
    test_formats();
    test_invalid();
    printf("QT_UP13_NATIVE checks=%u failures=%u\n", checks, failures);
    return failures != 0;
}

#else

static char out_dir[1024];
static unsigned cp27_ok, format_ok, empty_ok, errors_ok;

static void path_for(char *path, size_t size, const char *name)
{
    snprintf(path, size, "%s/%s", out_dir, name);
}

static OSErr spec_for(const char *name, FSSpec *spec)
{
    char path[1100];
    unsigned char counted[256];

    path_for(path, sizeof(path), name);
    size_t n = strlen(path);
    if (n > 255)
        return bdNamErr;
    counted[0] = (unsigned char)n;
    memcpy(counted + 1, path, n);
    OSErr err = FSMakeFSSpec(0, 0, counted, spec);
    return err == fnfErr ? noErr : err;
}

static unsigned char *read_file(const char *name, size_t *length)
{
    char path[1100];
    path_for(path, sizeof(path), name);
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    unsigned char *p = malloc(n > 0 ? (size_t)n : 1);
    if (p && n > 0 && fread(p, 1, (size_t)n, f) != (size_t)n) {
        free(p);
        p = NULL;
    }
    fclose(f);
    *length = n > 0 ? (size_t)n : 0;
    return p;
}

static long file_size(const char *name)
{
    char path[1100];
    struct stat st;
    path_for(path, sizeof(path), name);
    return stat(path, &st) == 0 ? (long)st.st_size : -1;
}

static void remove_file(const char *name)
{
    char path[1100];
    path_for(path, sizeof(path), name);
    unlink(path);
}

static SoundDescriptionHandle new_description(uint32_t format,
                                              unsigned channels,
                                              unsigned rate)
{
    SoundDescriptionHandle d =
        (SoundDescriptionHandle)NewHandleClear(sizeof(SoundDescription));
    if (d && *d) {
        (*d)->descSize = sizeof(SoundDescription);
        (*d)->dataFormat = format;
        (*d)->dataRefIndex = 1;
        (*d)->numChannels = (SInt16)channels;
        (*d)->sampleSize = 16;
        (*d)->sampleRate = (UInt32)rate << 16;
    }
    return d;
}

static OSErr open_movie(const char *name, Movie *movie, short *ref)
{
    FSSpec spec;
    OSErr err = spec_for(name, &spec);
    *movie = NULL;
    *ref = -1;
    if (err == noErr)
        err = OpenMovieFile(&spec, ref, fsRdPerm);
    if (err == noErr)
        err = NewMovieFromFile(movie, *ref, NULL, NULL, 0, NULL);
    return err;
}

/* The public sequence of the CHECKPOINT27 writer qualification. */
static void test_cp27_sequence(unsigned count, const char *name)
{
    unsigned before = failures;
    FSSpec spec;
    Movie movie = NULL;
    short ref = -1;
    unsigned char empty[1] = { 0 };
    Track tracks[2] = { NULL, NULL };
    Media media[2] = { NULL, NULL };

    remove_file(name);
    CHECK(spec_for(name, &spec) == noErr);
    CHECK(CreateMovieFile(&spec, 0, 0, createMovieFileDontCreateResFile,
                          &ref, &movie) == noErr);
    CHECK(movie != NULL && ref > 0);
    if (!movie)
        return;
    SetMovieTimeScale(movie, 48000);
    CHECK(GetMoviesError() == noErr);

    for (unsigned k = 0; k < count; ++k) {
        tracks[k] = NewMovieTrack(movie, 0, 0, 0x100);
        CHECK(tracks[k] != NULL && GetMoviesError() == noErr);
        media[k] = tracks[k] ? NewTrackMedia(tracks[k], SoundMediaType,
                                             48000, NULL, 0) : NULL;
        CHECK(media[k] != NULL && GetMoviesError() == noErr);
        if (!media[k])
            break;

        if (k == count - 1) {
            short id = movieInDataForkResID;
            /* A track without samples cannot be saved: nothing is written. */
            CHECK(AddMovieResource(movie, ref, &id, empty) != noErr);
            CHECK(file_size(name) == 0);
        }

        Handle data = NewHandle(NFRAMES * 4);
        SoundDescriptionHandle desc = new_description(SOWT, 2, 48000);
        TimeValue at = -1;

        CHECK(data && *data && desc && *desc);
        if (!data || !desc)
            break;
        fill_stereo((unsigned char *)*data, k);
        CHECK(BeginMediaEdits(media[k]) == noErr);
        CHECK(AddMediaSample(media[k], data, 0, NFRAMES * 4, 1,
                             (SampleDescriptionHandle)desc, NFRAMES, 0,
                             &at) == noErr);
        CHECK(at == 0);
        CHECK(EndMediaEdits(media[k]) == noErr);
        CHECK(InsertMediaIntoTrack(tracks[k], 0, 0, NFRAMES, 0x10000)
              == noErr);
        DisposeHandle(data);
        DisposeHandle((Handle)desc);
    }

    CHECK(GetMovieTrackCount(movie) == (SInt32)count);
    CHECK(GetMovieDuration(movie) == NFRAMES);
    for (unsigned k = 0; k < count; ++k) {
        CHECK(GetMovieIndTrack(movie, (SInt32)k + 1) == tracks[k]);
        CHECK(GetTrackMedia(tracks[k]) == media[k]);
    }

    short id = movieInDataForkResID;
    CHECK(AddMovieResource(movie, ref, &id, empty) == noErr);
    CHECK(id == movieInDataForkResID);
    CHECK(UpdateMovieResource(movie, ref, movieInDataForkResID, empty)
          == noErr);

    size_t before_size = 0, after_size = 0;
    unsigned char *saved = read_file(name, &before_size);

    CHECK(saved && before_size == 20 + 8 + count * NFRAMES * 4 + 8 + 108 +
                                  count * 385);
    /* A new track without media makes the movie unsavable: unchanged. */
    CHECK(NewMovieTrack(movie, 0, 0, 0x100) != NULL);
    CHECK(UpdateMovieResource(movie, ref, movieInDataForkResID, empty)
          != noErr);
    unsigned char *again = read_file(name, &after_size);
    CHECK(again && after_size == before_size &&
          memcmp(saved, again, before_size) == 0);
    free(saved);
    free(again);

    CHECK(CloseMovieFile(ref) == noErr);
    DisposeMovie(movie);
    CHECK(GetMoviesError() == noErr);

    /* Reopen through the File Manager and the movie reader. */
    Movie back = NULL;
    short rref = -1;

    CHECK(open_movie(name, &back, &rref) == noErr);
    if (back) {
        CHECK(GetMovieTrackCount(back) == (SInt32)count);
        CHECK(GetMovieTimeScale(back) == 48000);
        CHECK(GetMovieDuration(back) == NFRAMES);
        for (unsigned k = 0; k < count; ++k) {
            Media m = GetTrackMedia(GetMovieIndTrack(back, (SInt32)k + 1));
            CHECK(GetMediaSampleCount(m) == NFRAMES);
            CHECK(GetMediaTimeScale(m) == 48000);
        }
        if (count == 1) {
            Media m = GetTrackMedia(GetMovieIndTrack(back, 1));
            Handle h = NewHandle(0);
            SInt32 size = -1, got = -1, index = -1;
            TimeValue t = -1, d = -1;
            SInt16 flags = -1;
            static unsigned char expected[NFRAMES * 4];

            fill_stereo(expected, 0);
            CHECK(GetMediaSample(m, h, 0, &size, 0, &t, &d, NULL, &index,
                                 NFRAMES, &got, &flags) == noErr);
            CHECK(size == NFRAMES * 4 && got == NFRAMES && t == 0 && d == 1);
            CHECK(GetHandleSize(h) == NFRAMES * 4 &&
                  memcmp(*h, expected, NFRAMES * 4) == 0);
            DisposeHandle(h);
        }
        DisposeMovie(back);
    }
    if (rref > 0)
        CloseMovieFile(rref);

    if (failures == before)
        ++cp27_ok;
}

static void test_format(size_t i)
{
    unsigned before = failures;
    const struct format_case *c = &format_cases[i];
    unsigned frames = case_frames(c);
    unsigned stride = c->channels * 2;
    size_t bytes = (size_t)frames * stride;
    char name[64];
    FSSpec spec;
    Movie movie = NULL;
    short ref = -1;

    snprintf(name, sizeof(name), "format-%u.mov", (unsigned)i);
    remove_file(name);
    CHECK(spec_for(name, &spec) == noErr);
    /* Delete-current-file on a missing file creates it. */
    CHECK(CreateMovieFile(&spec, 0x44726c67, 0,
                          createMovieFileDeleteCurFile |
                          createMovieFileDontCreateResFile,
                          &ref, &movie) == noErr);
    if (!movie)
        return;
    if (c->movie_scale)
        SetMovieTimeScale(movie, c->movie_scale);
    TimeScale scale = GetMovieTimeScale(movie);
    CHECK(scale == (c->movie_scale ? c->movie_scale : 600));

    Track track = NewMovieTrack(movie, 0, 0, 0x100);
    Media media = track ? NewTrackMedia(track, SoundMediaType,
                                        (TimeScale)c->rate, NULL, 0) : NULL;
    SoundDescriptionHandle desc = new_description(c->format, c->channels,
                                                  c->rate);
    /* All samples in one handle, after a guard, added in three blocks. */
    Handle data = NewHandle((Size)(bytes + 64));

    CHECK(track && media && desc && data && *data);
    if (!track || !media || !desc || !data) {
        DisposeMovie(movie);
        CloseMovieFile(ref);
        return;
    }
    memset(*data, 0xee, bytes + 64);
    for (size_t j = 0; j < bytes; ++j)
        ((unsigned char *)*data)[32 + j] = pattern((unsigned)i, j);

    CHECK(BeginMediaEdits(media) == noErr);
    unsigned done = 0;
    for (unsigned b = 0; b < 3; ++b) {
        TimeValue at = -1;
        CHECK(AddMediaSample(media, data, (SInt32)(32 + done * stride),
                             c->blocks[b] * stride, 1,
                             (SampleDescriptionHandle)desc,
                             (SInt32)c->blocks[b], 0, &at) == noErr);
        CHECK(at == (TimeValue)done);
        done += c->blocks[b];
    }
    CHECK(EndMediaEdits(media) == noErr);
    CHECK(GetMediaSampleCount(media) == (SInt32)frames);

    /* Two contiguous insertions: the first block, then the rest. */
    CHECK(InsertMediaIntoTrack(track, 0, 0, c->blocks[0], 0x10000) == noErr);
    TimeValue end = (TimeValue)((uint64_t)c->blocks[0] * (uint32_t)scale /
                                c->rate);
    CHECK(InsertMediaIntoTrack(track, c->explicit_start ? end : -1,
                               c->blocks[0], frames - c->blocks[0],
                               0x10000) == noErr);
    TimeValue duration = (TimeValue)((uint64_t)frames * (uint32_t)scale /
                                     c->rate);
    CHECK(GetMovieDuration(movie) == duration);

    /* The media is readable before the movie is saved. */
    Handle h = NewHandle(0);
    SInt32 size = -1, got = -1, index = -1;
    TimeValue t = -1, d = -1;
    SInt16 flags = -1;

    CHECK(GetMediaSample(media, h, 0, &size, 0, &t, &d, NULL, &index,
                         (SInt32)frames, &got, &flags) == noErr);
    CHECK(size == (SInt32)bytes && got == (SInt32)frames);
    CHECK(GetHandleSize(h) == (Size)bytes &&
          memcmp(*h, (unsigned char *)*data + 32, bytes) == 0);

    short id = movieInDataForkResID;
    CHECK(AddMovieResource(movie, ref, &id, NULL) == noErr);
    CHECK(CloseMovieFile(ref) == noErr);
    DisposeMovie(movie);

    Movie back = NULL;
    short rref = -1;

    CHECK(open_movie(name, &back, &rref) == noErr);
    if (back) {
        Track bt = GetMovieIndTrackType(back, 1, SoundMediaType,
                                        movieTrackMediaType |
                                        movieTrackEnabledOnly);
        Media bm = bt ? GetTrackMedia(bt) : NULL;
        Handle bd = NewHandle(0);
        OSType type = 0;

        CHECK(GetMovieTrackCount(back) == 1);
        CHECK(GetMovieTimeScale(back) == scale);
        CHECK(GetMovieDuration(back) == duration);
        CHECK(bm != NULL);
        if (bm) {
            GetMediaHandlerDescription(bm, &type, NULL, NULL);
            CHECK(type == SoundMediaType);
            CHECK(GetMediaTimeScale(bm) == (TimeScale)c->rate);
            CHECK(GetMediaSampleCount(bm) == (SInt32)frames);
            CHECK(GetMediaDuration(bm) == (TimeValue)frames);
            GetMediaSampleDescription(bm, 1, (SampleDescriptionHandle)bd);
            CHECK(GetMoviesError() == noErr);
            if (GetHandleSize(bd) == sizeof(SoundDescription)) {
                SoundDescription *sd = (SoundDescription *)*bd;
                CHECK(sd->dataFormat == c->format);
                CHECK(sd->numChannels == (SInt16)c->channels);
                CHECK(sd->sampleSize == 16);
                CHECK(sd->sampleRate == (UInt32)c->rate << 16);
            } else {
                CHECK(!"sound description size");
            }
            size = got = -1;
            CHECK(GetMediaSample(bm, h, 0, &size, 0, &t, &d, NULL, &index,
                                 (SInt32)frames, &got, &flags) == noErr);
            CHECK(size == (SInt32)bytes && got == (SInt32)frames);
            CHECK(GetHandleSize(h) == (Size)bytes &&
                  memcmp(*h, (unsigned char *)*data + 32, bytes) == 0);
            /* Imported media cannot be edited. */
            CHECK(BeginMediaEdits(bm) == unimpErr);
        }
        DisposeHandle(bd);
        DisposeMovie(back);
    }
    if (rref > 0)
        CloseMovieFile(rref);

    DisposeHandle(h);
    DisposeHandle(data);
    DisposeHandle((Handle)desc);
    if (failures == before)
        ++format_ok;
}

static void test_empty(void)
{
    unsigned before = failures;
    FSSpec spec;
    Movie movie = NULL;
    short ref = -1, id = movieInDataForkResID;

    remove_file("empty.mov");
    CHECK(spec_for("empty.mov", &spec) == noErr);
    CHECK(CreateMovieFile(&spec, 0, 0, createMovieFileDontCreateResFile,
                          &ref, &movie) == noErr);
    if (!movie)
        return;
    CHECK(AddMovieResource(movie, ref, &id, NULL) == noErr);
    CHECK(CloseMovieFile(ref) == noErr);
    DisposeMovie(movie);
    CHECK(file_size("empty.mov") == 20 + 8 + 108);

    Movie back = NULL;
    short rref = -1;
    CHECK(open_movie("empty.mov", &back, &rref) == noErr);
    if (back) {
        CHECK(GetMovieTrackCount(back) == 0);
        CHECK(GetMovieDuration(back) == 0);
        CHECK(GetMovieTimeScale(back) == 600);
        DisposeMovie(back);
    }
    if (rref > 0)
        CloseMovieFile(rref);
    if (failures == before)
        ++empty_ok;
}

static void test_errors(void)
{
    unsigned before = failures;
    FSSpec spec;
    Movie movie = NULL;
    short ref = -1, id;
    const SInt32 data_fork = createMovieFileDontCreateResFile;

    /* Arguments and unsupported file layouts; nothing is created. */
    remove_file("errors.mov");
    CHECK(spec_for("errors.mov", &spec) == noErr);
    CHECK(CreateMovieFile(NULL, 0, 0, data_fork, &ref, &movie) == paramErr);
    CHECK(CreateMovieFile(&spec, 0, 0, data_fork, NULL, &movie) == paramErr);
    CHECK(CreateMovieFile(&spec, 0, 0, 0, &ref, &movie) == unimpErr);
    CHECK(CreateMovieFile(&spec, 0, 0, data_fork |
                          createMovieFileDontCreateMovie, &ref, &movie)
          == unimpErr);
    CHECK(CreateMovieFile(&spec, 0, 0, data_fork |
                          createMovieFileDontOpenFile, &ref, &movie)
          == unimpErr);
    CHECK(file_size("errors.mov") == -1 && movie == NULL);

    /* An existing file is kept unless it is to be deleted. */
    char path[1100];
    path_for(path, sizeof(path), "errors.mov");
    FILE *f = fopen(path, "wb");
    CHECK(f != NULL);
    if (f) {
        fputs("keep these bytes", f);
        fclose(f);
    }
    CHECK(CreateMovieFile(&spec, 0, 0, data_fork, &ref, &movie) == unimpErr);
    size_t kept_size = 0;
    unsigned char *kept = read_file("errors.mov", &kept_size);
    CHECK(kept && kept_size == 16 && memcmp(kept, "keep these bytes", 16) == 0);
    free(kept);
    CHECK(CreateMovieFile(&spec, 0, 0, data_fork |
                          createMovieFileDeleteCurFile, &ref, &movie)
          == noErr);
    CHECK(file_size("errors.mov") == 0);
    if (!movie)
        return;

    /* Tracks and media. */
    Movie bogus = (Movie)&ref;
    CHECK(NewMovieTrack(bogus, 0, 0, 0x100) == NULL);
    CHECK(GetMoviesError() == invalidMovie);
    CHECK(NewMovieTrack(movie, 0x1400000, 0xf00000, 0) == NULL);
    CHECK(GetMoviesError() == unimpErr);
    Track track = NewMovieTrack(movie, 0, 0, 0x100);
    CHECK(track != NULL);
    if (!track) {
        DisposeMovie(movie);
        CloseMovieFile(ref);
        return;
    }
    CHECK(GetTrackMedia(track) == NULL);
    CHECK(NewTrackMedia(track, 0x76696465, 600, NULL, 0) == NULL);
    CHECK(GetMoviesError() == unimpErr);
    CHECK(NewTrackMedia(track, SoundMediaType, 0, NULL, 0) == NULL);
    CHECK(GetMoviesError() == paramErr);
    CHECK(NewTrackMedia(track, SoundMediaType, 96000, NULL, 0) == NULL);
    CHECK(GetMoviesError() == unimpErr);
    Handle ref_handle = NewHandle(4);
    CHECK(NewTrackMedia(track, SoundMediaType, 8000, ref_handle,
                        0x616c6973) == NULL);
    CHECK(GetMoviesError() == unimpErr);
    DisposeHandle(ref_handle);
    Media media = NewTrackMedia(track, SoundMediaType, 8000, NULL, 0);
    CHECK(media != NULL);
    CHECK(NewTrackMedia(track, SoundMediaType, 8000, NULL, 0) == NULL);
    CHECK(GetMoviesError() == paramErr);
    if (!media) {
        DisposeMovie(movie);
        CloseMovieFile(ref);
        return;
    }

    /* Samples. */
    Handle data = NewHandle(400);
    SoundDescriptionHandle desc = new_description(TWOS, 1, 8000);
    TimeValue at = -1;
    memset(*data, 0x11, 400);

    CHECK(AddMediaSample(media, data, 0, 200, 1,
                         (SampleDescriptionHandle)desc, 100, 0, &at)
          == paramErr);                          /* outside BeginMediaEdits */
    CHECK(BeginMediaEdits(media) == noErr);
    CHECK(BeginMediaEdits(media) == paramErr);

    SoundDescription saved_desc = **desc;
    (**desc).descSize = 52;
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    **desc = saved_desc; (**desc).sampleSize = 8;
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    **desc = saved_desc; (**desc).sampleRate = 44100u << 16;
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    **desc = saved_desc; (**desc).dataFormat = 0x72617720;
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    **desc = saved_desc; (**desc).numChannels = 3;
    CHECK(AddMediaSample(media, data, 0, 600, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    **desc = saved_desc; (**desc).compressionID = -2;
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    **desc = saved_desc;
    CHECK(AddMediaSample(media, data, 0, 200, 2, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 1, &at) == unimpErr);
    CHECK(AddMediaSample(media, data, 0, 199, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == paramErr);
    CHECK(AddMediaSample(media, data, 300, 200, 1,
                         (SampleDescriptionHandle)desc, 100, 0, &at)
          == paramErr);
    CHECK(AddMediaSample(media, data, 0, 0, 1, (SampleDescriptionHandle)desc,
                         0, 0, &at) == paramErr);
    CHECK(AddMediaSample(media, NULL, 0, 200, 1,
                         (SampleDescriptionHandle)desc, 100, 0, &at)
          == paramErr);
    CHECK(GetMediaSampleCount(media) == 0 && at == -1);

    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == noErr);
    CHECK(at == 0);
    (**desc).dataFormat = SOWT;
    CHECK(AddMediaSample(media, data, 0, 200, 1, (SampleDescriptionHandle)desc,
                         100, 0, &at) == unimpErr);   /* one description */
    **desc = saved_desc;
    CHECK(AddMediaSample(media, data, 200, 200, 1,
                         (SampleDescriptionHandle)desc, 100, 0, &at) == noErr);
    CHECK(at == 100 && GetMediaSampleCount(media) == 200);

    /* Edits: only the next media samples, at the end, at normal rate. */
    CHECK(InsertMediaIntoTrack(track, 0, 0, 100, 0x10000) == paramErr);
    CHECK(EndMediaEdits(media) == noErr);
    CHECK(EndMediaEdits(media) == paramErr);
    CHECK(InsertMediaIntoTrack(track, 0, 0, 100, 0x20000) == unimpErr);
    CHECK(InsertMediaIntoTrack(track, 0, 5, 100, 0x10000) == unimpErr);
    CHECK(InsertMediaIntoTrack(track, 10, 0, 100, 0x10000) == unimpErr);
    CHECK(InsertMediaIntoTrack(track, 0, 0, 201, 0x10000) == paramErr);
    CHECK(InsertMediaIntoTrack(track, 0, 0, 0, 0x10000) == paramErr);
    CHECK(InsertMediaIntoTrack(track, 0, 0, 100, 0x10000) == noErr);
    CHECK(GetMovieDuration(movie) == 7);        /* 100 * 600 / 8000 */

    /* Saving. */
    id = 128;
    CHECK(AddMovieResource(movie, ref, &id, NULL) == unimpErr);
    CHECK(AddMovieResource(movie, ref, NULL, NULL) == paramErr);
    id = movieInDataForkResID;
    CHECK(AddMovieResource(movie, ref, &id, NULL) == unimpErr); /* 100/200 */
    CHECK(file_size("errors.mov") == 0);
    CHECK(UpdateMovieResource(movie, ref, movieInDataForkResID, NULL)
          == paramErr);                          /* never added */
    CHECK(InsertMediaIntoTrack(track, -1, 100, 100, 0x10000) == noErr);
    CHECK(AddMovieResource(movie, ref, &id, NULL) == noErr);
    CHECK(AddMovieResource(movie, ref, &id, NULL) == paramErr);
    CHECK(UpdateMovieResource(movie, (short)(ref + 1000),
                              movieInDataForkResID, NULL) == paramErr);
    CHECK(UpdateMovieResource(movie, ref, 0, NULL) == unimpErr);
    CHECK(UpdateMovieResource(movie, ref, movieInDataForkResID, NULL)
          == noErr);
    CHECK(file_size("errors.mov") == 20 + 8 + 400 + 8 + 108 + 385);

    DisposeHandle(data);
    DisposeHandle((Handle)desc);
    CHECK(CloseMovieFile(ref) == noErr);
    DisposeMovie(movie);
    CHECK(NewMovieTrack(movie, 0, 0, 0x100) == NULL);
    CHECK(GetMoviesError() == invalidMovie);

    if (failures == before)
        ++errors_ok;
}

int main(int argc, char **argv)
{
    /* Files go next to the witness; host paths appear under SystemRoot. */
    const char *self = argc > 0 ? argv[0] : "";
    const char *slash = strrchr(self, '/');
    size_t dir = slash ? (size_t)(slash - self) : 0;
    const char *root = strncmp(self, "/Volumes/SystemRoot/", 20) == 0 ?
                       "" : "/Volumes/SystemRoot";

    snprintf(out_dir, sizeof(out_dir), "%s%.*s/qt-up13-out", root,
             (int)dir, self);
    mkdir(out_dir, 0755);
    printf("OUTPUT_DIRECTORY=%s\n", out_dir);

    CHECK(CreateMovieFile(NULL, 0, 0, 0, NULL, NULL)
          == movieToolboxUninitialized);
    CHECK(EnterMovies() == noErr);

    test_cp27_sequence(1, "cp27-1.mov");
    test_cp27_sequence(2, "cp27-2.mov");
    for (size_t i = 0; i < FORMAT_COUNT; ++i)
        test_format(i);
    test_empty();
    test_errors();

    ExitMovies();

    printf("QT_UP13_WRITER checks=%u failures=%u cp27=%u/2 formats=%u/%u "
           "empty=%u/1 errors=%u/1\n", checks, failures, cp27_ok,
           format_ok, (unsigned)FORMAT_COUNT, empty_ok, errors_ok);
    return failures != 0;
}

#endif
