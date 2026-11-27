// Copyright (C) 2026 Lubos Dolezel
// NSSound plays through the audio output of the private PipeWire session and tells its delegate on the main thread
// when it ends (YES) or is stopped (NO). Sounds are generated in memory (WAV) or written to a temporary directory
// (AIFF, CAF); the tone has an amplitude of 0.5, and `nssound_run.sh` records the null sink to check the level.
//
//   nssound_play <dir> [levels]    <dir>: a directory for the AIFF and CAF files
// With "levels" the program only plays two tones (volume 1 and 0.25) for the level check.
#import <AppKit/AppKit.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int failures;
static double T0;
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL line %d: %s -- ", __LINE__, #cond); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } } while (0)
#define LOG(...) do { printf("[%6.3f] ", now() - T0); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)

static void put32le(NSMutableData *d, uint32_t v) { [d appendBytes: &v length: 4]; }
static void put16le(NSMutableData *d, uint16_t v) { [d appendBytes: &v length: 2]; }
static void put32be(NSMutableData *d, uint32_t v) { v = __builtin_bswap32(v); [d appendBytes: &v length: 4]; }
static void put16be(NSMutableData *d, uint16_t v) { v = __builtin_bswap16(v); [d appendBytes: &v length: 2]; }

// Sine samples as 16 bit values
static int16_t sample(int i, int rate, double hz, double amp) { return (int16_t) lrint(32767.0 * amp * sin(2 * M_PI * hz * i / rate)); }

// 16-bit mono WAV
static NSData *toneWAV(double seconds, int rate, double amp) {
    uint32_t n = (uint32_t) (seconds * rate), len = n * 2;
    NSMutableData *d = [NSMutableData data];
    [d appendBytes: "RIFF" length: 4]; put32le(d, 36 + len); [d appendBytes: "WAVEfmt " length: 8];
    put32le(d, 16); put16le(d, 1); put16le(d, 1); put32le(d, rate); put32le(d, rate * 2); put16le(d, 2); put16le(d, 16);
    [d appendBytes: "data" length: 4]; put32le(d, len);
    for (uint32_t i = 0; i < n; i++) put16le(d, (uint16_t) sample(i, rate, 440, amp));
    return d;
}

// 16-bit stereo AIFF: IEEE extended sample rate (only the rates below 65536 are needed)
static NSData *toneAIFF(double seconds, int rate, double amp) {
    uint32_t n = (uint32_t) (seconds * rate), len = n * 4;
    NSMutableData *d = [NSMutableData data];
    [d appendBytes: "FORM" length: 4]; put32be(d, 4 + 8 + 18 + 8 + 8 + len); [d appendBytes: "AIFF" length: 4];
    [d appendBytes: "COMM" length: 4]; put32be(d, 18); put16be(d, 2); put32be(d, n); put16be(d, 16);
    int e = 0; double m = rate; while (m >= 2) { m /= 2; e++; }          // rate = m * 2^e, 1 <= m < 2
    put16be(d, (uint16_t) (16383 + e)); uint64_t mant = (uint64_t) (m * 9223372036854775808.0);
    put32be(d, (uint32_t) (mant >> 32)); put32be(d, (uint32_t) mant);
    [d appendBytes: "SSND" length: 4]; put32be(d, 8 + len); put32be(d, 0); put32be(d, 0);
    for (uint32_t i = 0; i < n; i++) { int16_t v = sample(i, rate, 440, amp); put16be(d, v); put16be(d, v); }
    return d;
}

static void putf64be(NSMutableData *d, double v) { uint64_t b; memcpy(&b, &v, 8); b = __builtin_bswap64(b); [d appendBytes: &b length: 8]; }

// 16-bit big endian linear PCM CAF, mono
static NSData *toneCAF(double seconds, int rate, double amp) {
    uint32_t n = (uint32_t) (seconds * rate);
    NSMutableData *d = [NSMutableData data];
    [d appendBytes: "caff" length: 4]; put16be(d, 1); put16be(d, 0);
    [d appendBytes: "desc" length: 4]; put32be(d, 0); put32be(d, 32);
    putf64be(d, rate); [d appendBytes: "lpcm" length: 4]; put32be(d, 0 /* big endian integer */); put32be(d, 2); put32be(d, 1);
    put32be(d, 1); put32be(d, 16);
    [d appendBytes: "data" length: 4]; put32be(d, 0); put32be(d, 0); // the 64-bit chunk size is patched below
    NSUInteger sizeAt = [d length] - 4;
    put32be(d, 1); // edit count
    for (uint32_t i = 0; i < n; i++) put16be(d, (uint16_t) sample(i, rate, 440, amp));
    uint32_t size = __builtin_bswap32(4 + n * 2);
    [d replaceBytesInRange: NSMakeRange(sizeAt, 4) withBytes: &size];
    return d;
}

@interface Delegate : NSObject <NSSoundDelegate> {
@public
    int calls; BOOL last; BOOL onMain; double at; NSSound *who; BOOL playingInside;
}
- (void) reset;
@end
@implementation Delegate
- (void) reset { calls = 0; who = nil; }
- (void) sound: (NSSound *) sound didFinishPlaying: (BOOL) ok {
    calls++; last = ok; onMain = [NSThread isMainThread]; at = now(); who = sound; playingInside = [sound isPlaying];
    LOG("delegate didFinishPlaying:%d", ok);
}
@end

static void spin(double seconds) { [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: seconds]]; }
static BOOL waitFor(Delegate *d, int calls, double timeout) {
    double end = now() + timeout;
    while (d->calls < calls && now() < end) spin(0.02);
    return d->calls >= calls;
}

static NSSound *make(NSData *data, Delegate *d) {
    NSSound *s = [[NSSound alloc] initWithData: data];
    s.delegate = d;
    return s;
}

int main(int argc, char **argv) {
    @autoreleasepool {
        T0 = now();
        NSString *dir = [NSString stringWithUTF8String: argc > 1 ? argv[1] : "/tmp"];
        BOOL levelsOnly = argc > 2 && !strcmp(argv[2], "levels");
        Delegate *d = [Delegate new];

        if (levelsOnly) {
            // Two tones of amplitude 0.5 with a gap: at full volume and at a quarter of it
            NSSound *loud = make(toneWAV(1.0, 22050, 0.5), d);
            LOG("loud play=%d", [loud play]);
            waitFor(d, 1, 3);
            spin(0.5);
            NSSound *quiet = make(toneWAV(1.0, 22050, 0.5), d);
            quiet.volume = 0.25f;
            [d reset];
            LOG("quiet play=%d", [quiet play]);
            waitFor(d, 1, 3);
            spin(0.3);
            printf(d->calls == 1 && d->last ? "PASS\n" : "FAILED\n");
            return d->calls == 1 && d->last ? 0 : 1;
        }

        // 1. A sound plays to its end: the delegate hears YES, on the main thread, about one duration (and the
        //    output latency) after the start
        NSSound *s = make(toneWAV(1.0, 22050, 0.25), d);
        double t0 = now();
        CHECK([s play], "play");
        CHECK([s isPlaying], "playing after play");
        CHECK(![s play], "play of a playing sound returns NO");
        spin(0.4);
        double ct = [s currentTime];
        LOG("currentTime after 0.4 s: %.3f", ct);
        CHECK(ct > 0.15 && ct < 0.5, "currentTime %.3f", ct);
        CHECK(d->calls == 0, "no early delegate message");
        CHECK(waitFor(d, 1, 4), "the delegate is told that the sound ended");
        double took = d->at - t0;
        LOG("ended after %.3f s", took);
        CHECK(d->last && d->onMain && d->who == s, "YES on the main thread for the sound");
        CHECK(took > 0.9 && took < 1.8, "end %.3f s after the start of a 1 s sound", took);
        CHECK(![s isPlaying] && !d->playingInside, "not playing after the end");
        CHECK([s currentTime] == 0, "currentTime back at 0 after the end: %.3f", [s currentTime]);
        spin(0.3);
        CHECK(d->calls == 1, "exactly one message: %d", d->calls);

        // 2. The sound plays again from the start; pause holds the position, resume goes on
        [d reset];
        t0 = now();
        CHECK([s play], "play again");
        spin(0.3);
        CHECK([s pause], "pause");
        CHECK(![s isPlaying], "not playing while paused");
        CHECK(![s pause], "pause of a paused sound returns NO");
        double p1 = [s currentTime]; spin(0.4); double p2 = [s currentTime];
        LOG("paused position %.3f %.3f", p1, p2);
        CHECK(fabs(p1 - p2) < 0.02 && p1 > 0.15 && p1 < 0.45, "the position is held");
        CHECK(d->calls == 0, "no message for a pause");
        CHECK([s resume], "resume");
        CHECK([s isPlaying], "playing after resume");
        CHECK(![s resume], "resume of a playing sound returns NO");
        CHECK(waitFor(d, 1, 4) && d->last, "the sound ends after the resume");
        took = d->at - t0;
        LOG("ended after %.3f s with a 0.4 s pause", took);
        CHECK(took > 1.3 && took < 2.3, "pause lengthens the sound: %.3f", took);

        // 3. stop: NO, and the next play starts at the beginning
        [d reset];
        CHECK([s play], "play");
        spin(0.3);
        CHECK([s stop], "stop");
        CHECK(![s isPlaying] && [s currentTime] == 0, "stopped at the start");
        CHECK(waitFor(d, 1, 1) && !d->last && d->onMain, "stop tells the delegate NO");
        CHECK([s stop], "stop of a stopped sound");
        spin(0.2);
        CHECK(d->calls == 1, "one message for a stop: %d", d->calls);
        [d reset];
        CHECK([s play], "play after stop");
        spin(0.2);
        CHECK([s currentTime] < 0.45, "restarted at the beginning: %.3f", [s currentTime]);
        CHECK([s pause] && [s stop], "stop of a paused sound");
        CHECK(waitFor(d, 1, 1) && !d->last, "stop of a paused sound tells NO");

        // 4. loops: no message while it plays, and the position wraps; stop tells NO
        [d reset];
        NSSound *l = make(toneWAV(0.5, 22050, 0.25), d);
        l.loops = YES;
        CHECK([l play], "play a looping sound");
        spin(1.7);
        CHECK(d->calls == 0 && [l isPlaying], "still playing after 3 durations");
        ct = [l currentTime];
        CHECK(ct >= 0 && ct < 0.5, "position within the sound: %.3f", ct);
        l.loops = NO;
        CHECK(waitFor(d, 1, 2) && d->last, "the sound ends once looping is turned off");
        [d reset];
        l.loops = YES;
        [l play]; spin(0.7); CHECK([l stop], "stop a looping sound");
        CHECK(waitFor(d, 1, 1) && !d->last, "stop of a looping sound tells NO");

        // 5. currentTime while playing moves the position
        [d reset];
        NSSound *seek = make(toneWAV(1.5, 22050, 0.25), d);
        [seek play]; spin(0.2);
        seek.currentTime = 1.0;
        ct = [seek currentTime];
        CHECK(ct >= 0.99 && ct < 1.2, "seek: %.3f", ct);
        t0 = now();
        CHECK(waitFor(d, 1, 3) && d->last, "ends");
        took = d->at - t0;
        LOG("after a seek to 1.0 of 1.5 s the end came after %.3f s", took);
        CHECK(took > 0.35 && took < 1.2, "end after the seek %.3f", took);

        // 6. The delegate hears even when nobody holds the sound
        [d reset];
        @autoreleasepool {
            NSSound *tmp = make(toneWAV(0.4, 22050, 0.25), d);
            [tmp play];
            [tmp release];
        }
        CHECK(waitFor(d, 1, 3) && d->last, "a sound that nobody else holds plays to its end");

        // 7. Other file types, 48 kHz and 22.05 kHz
        NSString *aiff = [dir stringByAppendingPathComponent: @"tone.aiff"], *caf = [dir stringByAppendingPathComponent: @"tone.caf"];
        [toneAIFF(0.5, 48000, 0.25) writeToFile: aiff atomically: NO];
        [toneCAF(0.5, 44100, 0.25) writeToFile: caf atomically: NO];
        for (NSString *path in @[aiff, caf]) {
            [d reset];
            NSSound *f = [[NSSound alloc] initWithContentsOfFile: path byReference: YES];
            f.delegate = d;
            CHECK(f != nil && fabs([f duration] - 0.5) < 0.005, "%s opens, duration %.4f", [[path lastPathComponent] UTF8String], [f duration]);
            if (!f) continue;
            t0 = now();
            CHECK([f play], "play %s", [[path lastPathComponent] UTF8String]);
            CHECK(waitFor(d, 1, 3) && d->last, "%s ends", [[path lastPathComponent] UTF8String]);
            LOG("%s ended after %.3f s", [[path lastPathComponent] UTF8String], d->at - t0);
        }
        unlink([aiff fileSystemRepresentation]); unlink([caf fileSystemRepresentation]);
    }
    printf(failures ? "FAILED (%d)\n" : "PASS\n", failures);
    return failures != 0;
}
