
/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <QuickTime/QuickTime.h>
#include <CarbonCore/MacErrors.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned char b[256];
static unsigned checks, failures;
static PointerDataRefRecord rec;
static PointerDataRefPtr rp = &rec;

#define CHECK(x) do { ++checks; if (!(x)) { \
    ++failures; printf("FAIL %d: %s\n", __LINE__, #x); \
} } while (0)

static void be32(unsigned char *p, uint32_t n)
{
    p[0]=n>>24; p[1]=n>>16; p[2]=n>>8; p[3]=n;
}

static Size fixture(unsigned version, uint32_t scale,
                    uint32_t duration, int track)
{
    unsigned payload = version ? 112 : 100;
    unsigned n = 16 + payload + (track ? 8 : 0);

    memset(b, 0, sizeof(b));
    be32(b,n); memcpy(b+4,"moov",4);
    be32(b+8,8+payload); memcpy(b+12,"mvhd",4);
    b[16]=version;
    be32(b+16+(version ? 20 : 12),scale);
    be32(b+16+(version ? 28 : 16),duration);

    if (track) {
        be32(b+16+payload,8);
        memcpy(b+20+payload,"trak",4);
    }
    return (Size)n;
}

static OSErr open_movie(Movie *m, short flags, OSType type)
{
    return NewMovieFromDataRef(m,flags,NULL,(Handle)&rp,type);
}

static void reject(OSErr expected)
{
    Movie m=(Movie)1;
    CHECK(open_movie(&m,0,PointerDataHandlerSubType)==expected);
    CHECK(m==NULL);
    CHECK(GetMoviesError()==expected);
    CHECK(GetMoviesError()==noErr);
}

int main(void)
{
    Movie m=NULL;
    rec.data=b;
    rec.dataLength=fixture(0,600,1500,0);

    CHECK(sizeof(void*)==4);
    CHECK(sizeof(PointerDataRefRecord)==8);
    reject(movieToolboxUninitialized);
    CHECK(EnterMovies()==noErr);

    for (unsigned v=0; v<2; ++v) {
        rec.dataLength=fixture(v,1000,2400,0);
        m=NULL;
        CHECK(open_movie(&m,0,PointerDataHandlerSubType)==noErr);
        CHECK(m!=NULL);
        if (m) {
            CHECK(GetMovieTimeScale(m)==1000);
            CHECK(GetMovieDuration(m)==2400);
            memset(b,0xa5,sizeof(b));
            CHECK(GetMovieDuration(m)==2400);
            DisposeMovie(m);
        }
    }

    rec.dataLength=fixture(0,600,1000,1);
    reject(unimpErr);

    rec.dataLength=fixture(0,0,1000,0);
    reject(badPublicMovieAtom);

    rec.dataLength=fixture(0,600,UINT32_MAX,0);
    reject(invalidDuration);

    rec.dataLength=fixture(0,600,1000,0)-1;
    reject(badPublicMovieAtom);

    rec.dataLength=8;
    be32(b,8); memcpy(b+4,"free",4);
    reject(noMovieFound);

    rec.dataLength=0;
    reject(invalidDataRef);

    rec.dataLength=fixture(0,600,0,0);
    CHECK(open_movie(&m,1,PointerDataHandlerSubType)==unimpErr);
    CHECK(m==NULL);
    CHECK(open_movie(&m,0,0)==cantFindHandler);
    CHECK(m==NULL);

    ExitMovies();

    printf("QUICKTIME_DATAREF checks=%u failures=%u\n",
           checks,failures);

    return failures!=0;
}
