// CFLAGS: -framework Foundation -fobjc-arc -Isrc/Finder src/Finder/FinderDSStore.m
// ARGS: tests/fixtures/dsstore
// Copyright (C) 2026 Lubos Dolezel
//
// Reading .DS_Store files (src/Finder/FinderDSStore): the fixtures tests/fixtures/make-dsstore-fixtures.py writes
// (every record type, a multi-level tree, an empty store, malformed stores), every truncation and 2000 random
// mutations of the well-formed ones (nil or a store, never a crash), and optionally stores Finder wrote, whose
// records are printed. See tests/finder.md.
// Usage: finder_dsstore <fixture dir> [<extra store>...]. Prints PASS/FAIL lines; exits 1 on a failure.
#import <Foundation/Foundation.h>
#import "FinderDSStore.h"
#include <stdio.h>
#include <stdlib.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSString *fourcc(FourCharCode code)
{
    char text[5] = { (char) (code >> 24), (char) (code >> 16), (char) (code >> 8), (char) code, 0 };
    return [NSString stringWithFormat: @"%s", text];
}

static NSData *iloc(uint32_t x, uint32_t y)
{
    unsigned char bytes[16] = { x >> 24, x >> 16, x >> 8, x, y >> 24, y >> 16, y >> 8, y,
                                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0 };
    return [NSData dataWithBytes: bytes length: sizeof bytes];
}

static NSData *fixture(NSString *dir, NSString *name)
{
    return [NSData dataWithContentsOfFile: [dir stringByAppendingPathComponent: [name stringByAppendingString: @".DS_Store"]]];
}

// Parses every prefix of data, and count copies with 1-8 random bytes changed: each gives nil or a store
static void survives(NSString *name, NSData *data, int count)
{
    int parsed = 0;
    for (NSUInteger length = 0; length < [data length]; length++) {
        @autoreleasepool {
            parsed += [FinderDSStore storeWithData: [data subdataWithRange: NSMakeRange(0, length)]] != nil;
        }
    }
    expect(YES, [NSString stringWithFormat: @"%@: every truncation gives nil or a store (%d stores)", name, parsed]);

    parsed = 0;
    srand48(7);
    for (int i = 0; i < count; i++) {
        @autoreleasepool {
            NSMutableData *copy = [data mutableCopy];
            unsigned char *bytes = [copy mutableBytes];
            int flips = 1 + (int) (drand48() * 8);
            for (int j = 0; j < flips; j++)
                bytes[(NSUInteger) (drand48() * [copy length])] ^= 1 + (int) (drand48() * 255);
            parsed += [FinderDSStore storeWithData: copy] != nil;
        }
    }
    expect(YES, [NSString stringWithFormat: @"%@: %d mutations give nil or a store (%d stores)", name, count, parsed]);
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: %s <fixture dir> [<extra store>...]\n", argv[0]);
            return 2;
        }
        NSString *dir = [NSString stringWithUTF8String: argv[1]];

        // all-types: one record of each type, in the order Finder keeps them
        FinderDSStore *store = [FinderDSStore storeWithData: fixture(dir, @"all-types")];
        expect(store != nil, @"all-types parses");
        NSArray *want = @[
            @[ @".", @"ICVO", @"bool", @YES ],
            @[ @".", @"vSrn", @"long", @1 ],
            @[ @"a", @"lg1S", @"comp", @123456789012ULL ],
            @[ @"a", @"modD", @"dutc", @0x0123456789ABCDEFULL ],
            @[ @"ü", @"type", @"type", @"icnv" ],
            @[ @"日本", @"cmmt", @"ustr", @"Kommentar 😀" ],
            @[ @"😀", @"Iloc", @"blob", iloc(10, 20) ],
            @[ @"😀", @"dscl", @"shor", @7 ],
        ];
        expect([store.records count] == [want count],
               [NSString stringWithFormat: @"all-types has 8 records (%lu)", (unsigned long) [store.records count]]);
        for (NSUInteger i = 0; i < MIN([want count], [store.records count]); i++) {
            FinderDSStoreRecord *record = store.records[i];
            NSArray *expected = want[i];
            BOOL same = [record.name isEqual: expected[0]] && [fourcc(record.code) isEqual: expected[1]]
                && [fourcc(record.type) isEqual: expected[2]] && [record.value isEqual: expected[3]];
            expect(same, [NSString stringWithFormat: @"record %lu is %@ %@ %@ %@ (%@ %@ %@ %@)", (unsigned long) i,
                          expected[0], expected[1], expected[2], expected[3],
                          record.name, fourcc(record.code), fourcc(record.type), record.value]);
        }
        FinderDSStoreRecord *decomposed = [store recordForName: @"ü" code: 'type'];
        FinderDSStoreRecord *precomposed = [store recordForName: @"ü" code: 'type'];
        expect(decomposed != nil && decomposed == precomposed, @"ü is found by its decomposed and its precomposed name");
        expect([store recordForName: @"Ü" code: 'type'] == nil, @"names match case-sensitively");
        expect([store recordForName: @"a" code: 'Iloc'] == nil, @"no record for a code the name doesn't have");

        // multi-level: internal nodes
        store = [FinderDSStore storeWithData: fixture(dir, @"multi-level")];
        expect([store.records count] == 3000,
               [NSString stringWithFormat: @"multi-level has 3000 records (%lu)", (unsigned long) [store.records count]]);
        FinderDSStoreRecord *last = [store recordForName: @"item 2999.txt" code: 'Iloc'];
        expect([last.value isEqual: iloc(2999, 2999 * 2)], @"item 2999.txt's Iloc is (2999, 5998)");
        BOOL ordered = YES;
        for (NSUInteger i = 1; i < [store.records count]; i++)
            ordered = ordered && [store.records[i - 1].name caseInsensitiveCompare: store.records[i].name] != NSOrderedDescending;
        expect(ordered && [store.records count] > 0, @"multi-level's records are in tree order");

        store = [FinderDSStore storeWithData: fixture(dir, @"empty")];
        expect(store != nil && [store.records count] == 0, @"empty parses, with no records");

        for (NSString *name in @[ @"cycle", @"block-out-of-range", @"bad-type", @"huge-count", @"long-name", @"bad-magic" ]) {
            NSData *data = fixture(dir, name);
            expect(data != nil && [FinderDSStore storeWithData: data] == nil, [NSString stringWithFormat: @"%@ is rejected", name]);
        }
        expect([FinderDSStore storeWithData: [NSData data]] == nil, @"no data is rejected");

        survives(@"all-types", fixture(dir, @"all-types"), 2000);
        survives(@"multi-level", fixture(dir, @"multi-level"), 2000);

        // storeForFolder: reads <folder>/.DS_Store
        NSString *folder = [NSTemporaryDirectory() stringByAppendingPathComponent: [[NSUUID UUID] UUIDString]];
        [[NSFileManager defaultManager] createDirectoryAtPath: folder withIntermediateDirectories: YES attributes: nil error: NULL];
        expect([FinderDSStore storeForFolder: [NSURL fileURLWithPath: folder]] == nil, @"a folder without .DS_Store has no store");
        [fixture(dir, @"all-types") writeToFile: [folder stringByAppendingPathComponent: @".DS_Store"] atomically: NO];
        expect([[FinderDSStore storeForFolder: [NSURL fileURLWithPath: folder]].records count] == 8,
               @"storeForFolder: reads the folder's .DS_Store");
        [[NSFileManager defaultManager] removeItemAtPath: folder error: NULL];

        for (int i = 2; i < argc; i++) {
            NSString *path = [NSString stringWithUTF8String: argv[i]];
            store = [FinderDSStore storeWithData: [NSData dataWithContentsOfFile: path]];
            expect(store != nil, [NSString stringWithFormat: @"%@ parses", [path lastPathComponent]]);
            for (FinderDSStoreRecord *record in store.records)
                printf("  record %s %s %s\n", [record.name UTF8String], [fourcc(record.code) UTF8String],
                       [fourcc(record.type) UTF8String]);
        }

        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
