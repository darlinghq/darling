
/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <QuickTime/QuickTime.h>
#include <CarbonCore/MacErrors.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned char bytes[2048];
static unsigned pos, checks, failures;
static PointerDataRefRecord record;
static PointerDataRefPtr rp = &record;

#define CHECK(x) do { ++checks; if (!(x)) { \
    ++failures; printf("FAIL %d %s\n",__LINE__,#x); \
} } while (0)

static void be32(unsigned off, uint32_t v)
{
    bytes[off]=v>>24;
    bytes[off+1]=v>>16;
    bytes[off+2]=v>>8;
    bytes[off+3]=v;
}

static unsigned atom(const char *type)
{
    unsigned p=pos;
    memcpy(bytes+pos+4,type,4);
    pos+=8;
    return p;
}

static void close_atom(unsigned p)
{
    be32(p,pos-p);
}

static unsigned payload(unsigned n)
{
    unsigned p=pos;
    pos+=n;
    return p;
}

static void track(uint32_t scale, uint32_t samples)
{
    unsigned t=atom("trak");
    unsigned k=atom("tkhd");
    payload(84);
    close_atom(k);

    unsigned m=atom("mdia");
    unsigned h=atom("mdhd");
    unsigned p=payload(24);
    be32(p+12,scale);
    be32(p+16,samples);
    close_atom(h);

    h=atom("hdlr");
    p=payload(24);
    memcpy(bytes+p+8,"soun",4);
    close_atom(h);

    unsigned f=atom("minf");
    unsigned s=atom("stbl");
    unsigned z=atom("stsz");
    p=payload(12);
    be32(p+4,2);
    be32(p+8,samples);
    close_atom(z);
    close_atom(s);
    close_atom(f);
    close_atom(m);
    close_atom(t);
}

static void fixture(int reference)
{
    memset(bytes,0,sizeof(bytes));
    pos=0;
    unsigned m=atom("moov");
    unsigned h=atom("mvhd");
    unsigned p=payload(100);
    be32(p+12,600);
    be32(p+16,1500);
    close_atom(h);

    track(48000,72000);
    track(44100,66150);

    if (reference) {
        unsigned r=atom("rmra");
        close_atom(r);
    }

    close_atom(m);
    record.data=bytes;
    record.dataLength=(Size)pos;
}

static OSErr import_movie(Movie *movie)
{
    return NewMovieFromDataRef(
        movie,0,NULL,(Handle)&rp,PointerDataHandlerSubType);
}

int main(void)
{
    Movie m=NULL;
    fixture(0);
    CHECK(EnterMovies()==noErr);

#ifdef QT_BASELINE
    CHECK(import_movie(&m)==unimpErr);
    CHECK(m==NULL);
    puts("BASELINE_TRACK_MOV_REFUSED");
#else
    CHECK(import_movie(&m)==noErr);
    CHECK(m!=NULL);

    if (m) {
        CHECK(GetMovieDuration(m)==1500);
        CHECK(GetMovieTimeScale(m)==600);
        CHECK(GetMovieTrackCount(m)==2);

        Track a=GetMovieIndTrack(m,1);
        Track b=GetMovieIndTrack(m,2);

        CHECK(a && b && a!=b);
        CHECK(GetMovieIndTrack(m,1)==a);
        CHECK(GetMovieIndTrack(m,0)==NULL);
        CHECK(GetMoviesError()==badTrackIndex);
        CHECK(GetMovieIndTrack(m,3)==NULL);
        CHECK(GetMoviesError()==badTrackIndex);

        Media x=GetTrackMedia(a);
        Media y=GetTrackMedia(b);

        CHECK(x && y && x!=y);
        CHECK(GetTrackMedia(a)==x);
        CHECK(GetMediaTimeScale(x)==48000);
        CHECK(GetMediaTimeScale(y)==44100);
        CHECK(GetMediaSampleCount(x)==72000);
        CHECK(GetMediaSampleCount(y)==66150);

        memset(bytes,0xa5,sizeof(bytes));

        CHECK(GetMediaSampleCount(x)==72000);
        CHECK(GetMediaTimeScale(y)==44100);

        DisposeMovie(m);
        CHECK(GetTrackMedia(a)==NULL);
        CHECK(GetMoviesError()==invalidTrack);
        CHECK(GetMediaSampleCount(x)==0);
        CHECK(GetMoviesError()==invalidMedia);
    }

    Movie empty=NewMovie(0);
    CHECK(empty!=NULL);
    CHECK(GetMovieTrackCount(empty)==0);
    DisposeMovie(empty);

    fixture(1);
    m=NULL;
    CHECK(import_movie(&m)==unimpErr);
    CHECK(m==NULL);
#endif

    ExitMovies();
    printf("QT_UP07_TRACKS checks=%u failures=%u\n",checks,failures);
    return failures!=0;
}
