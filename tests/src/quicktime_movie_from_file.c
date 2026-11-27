
/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <QuickTime/QuickTime.h>
#include <CarbonCore/MacErrors.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <CarbonCore/Files.h>
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


static OSErr open_ref(const FSRef *ref, short *fork)
{
#ifdef QT_COMBINED
    FSSpec spec;
    OSErr e=FSGetCatalogInfo(ref,0,NULL,NULL,&spec,NULL);
    if (e!=noErr)
        return e;
    return OpenMovieFile(&spec,fork,fsRdPerm);
#else
    return FSOpenFork(ref,0,NULL,fsRdPerm,fork);
#endif
}

static OSErr close_ref(short fork)
{
#ifdef QT_COMBINED
    return CloseMovieFile(fork);
#else
    return FSCloseFork(fork);
#endif
}

int main(void)
{
    Movie m=NULL, second=NULL;
    short fork=-1, id=0;
    Str255 name;
    Boolean changed=1;
    SInt64 mark=-1;
    FSRef ref;
    char path[]="/tmp/qt-up08-XXXXXX";

    fixture(0);
    int fd=mkstemp(path);
    if (fd<0)
        return 2;

    if (write(fd,bytes,pos)!=(ssize_t)pos) {
        close(fd);
        unlink(path);
        return 2;
    }
    close(fd);

    CHECK(EnterMovies()==noErr);
    if (FSPathMakeRef((const UInt8*)path,&ref,NULL)!=noErr ||
        open_ref(&ref,&fork)!=noErr) {
        unlink(path);
        return 2;
    }

    CHECK(FSSetForkPosition(fork,fsFromStart,5)==noErr);
    memset(name,0xa5,sizeof(name));

    OSErr err=NewMovieFromFile(&m,fork,&id,name,0,&changed);
    if (!m && err==noErr)
        puts("QT_UP08_BASELINE_NO_MOVIE");

    CHECK(err==noErr);
    CHECK(m!=NULL);
    CHECK(id==movieInDataForkResID);
    CHECK(name[0]==0);
    CHECK(changed==0);

    CHECK(FSGetForkPosition(fork,&mark)==noErr);
    CHECK(mark==5);

    if (m) {
        CHECK(GetMovieDuration(m)==1500);
        CHECK(GetMovieTimeScale(m)==600);
        CHECK(GetMovieTrackCount(m)==2);
        Track t=GetMovieIndTrack(m,1);
        CHECK(t!=NULL);
        if (t) {
            Media md=GetTrackMedia(t);
            CHECK(md!=NULL);
            if (md) {
                CHECK(GetMediaTimeScale(md)==48000);
                CHECK(GetMediaSampleCount(md)==72000);
            }
        }
    }

    id=movieInDataForkResID;
    CHECK(NewMovieFromFile(&second,fork,&id,NULL,0,NULL)==noErr);
    CHECK(second!=NULL && second!=m);

    Movie invalid=NULL;
    short resourceId=128;
    CHECK(NewMovieFromFile(&invalid,fork,&resourceId,NULL,0,NULL)==unimpErr);
    CHECK(invalid==NULL);
    CHECK(NewMovieFromFile(NULL,fork,NULL,NULL,0,NULL)==paramErr);

    CHECK(close_ref(fork)==noErr);

    CHECK(NewMovieFromFile(&invalid,fork,NULL,NULL,0,NULL)!=noErr);
    CHECK(invalid==NULL);

    if (m) {
        CHECK(GetMovieDuration(m)==1500);
        CHECK(GetMovieTrackCount(m)==2);
        DisposeMovie(m);
    }
    if (second && second!=m)
        DisposeMovie(second);

    unlink(path);
    ExitMovies();

    printf("QT_UP08_FILE checks=%u failures=%u\n",checks,failures);
    return failures!=0;
}
