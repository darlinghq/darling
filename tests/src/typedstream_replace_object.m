// Copyright (C) 2026 Lubos Dolezel
//
// NSUnarchiver: -replaceObject:withObject: redirects later references to a decoded object, and
// +decodeClassName:asClassName: maps archived class names to runtime classes.
// Fixtures: archives written by +[NSArchiver archivedDataWithRootObject:] on macOS 10.14.4 (i386
// and x86_64 produce identical bytes), see vd-upstream/mac-probes/v685-typedstream.m. The values
// checked are the ones macOS printed after unarchiving the same data with NSUnarchiver.
#import <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

static NSData *hexData(const char *hex)
{
    NSMutableData *d = [NSMutableData data];
    for (size_t i = 0; hex[i] != 0; i += 2)
    {
        unsigned b = 0;
        sscanf(hex + i, "%2x", &b);
        unsigned char c = (unsigned char)b;
        [d appendBytes:&c length:1];
    }
    return d;
}

static id decodeData(NSData *data)
{
    @try
    {
        return [NSUnarchiver unarchiveObjectWithData:data];
    }
    @catch (NSException *e)
    {
        printf("exception %s: %s\n", [[e name] UTF8String], [[e reason] UTF8String]);
        return nil;
    }
}

static const char fx_replace[] =
    "040b73747265616d747970656481e803840140848484074e534172726179008484084e53"
    "4f626a656374008584016903928484840b50726f6265486f6c6465720094928484840950"
    "726f62654e6f6465009492848484084e53537472696e67019484012b0673686172656486"
    "9501928586869297929786";

static const char fx_class_plain[] =
    "040b73747265616d747970656481e8038401408484840950726f62654e6f646500848408"
    "4e534f626a656374008592848484084e53537472696e67019484012b046e6f6465868401"
    "6907928586";

static const char fx_class_encoded[] =
    "040b73747265616d747970656481e8038401408484840c4f6c6450726f62654e6f646500"
    "8484084e534f626a656374008592848484084e53537472696e67019484012b046e6f6465"
    "8684016907928586";

@interface ProbeNode : NSObject <NSCoding>
{
@public
    NSString *tag;
    int n;
    id other;
}
@end

@implementation ProbeNode
- (void)encodeWithCoder:(NSCoder *)c
{
    [c encodeObject:tag];
    [c encodeValueOfObjCType:"i" at:&n];
    [c encodeObject:other];
}
- (id)initWithCoder:(NSCoder *)c
{
    tag = [[c decodeObject] retain];
    [c decodeValueOfObjCType:"i" at:&n];
    other = [[c decodeObject] retain];
    return self;
}
@end

@interface RenamedNode : ProbeNode
@end
@implementation RenamedNode
@end

// While decoding, replaces the shared object it holds by another one.
@interface ProbeHolder : NSObject <NSCoding>
{
@public
    id shared;
}
@end

@implementation ProbeHolder
- (void)encodeWithCoder:(NSCoder *)c
{
    [c encodeObject:shared];
}
- (id)initWithCoder:(NSCoder *)c
{
    shared = [[c decodeObject] retain];
    // the unarchiver does not retain the replacement (on macOS), so it is autoreleased
    ProbeNode *r = [[[ProbeNode alloc] init] autorelease];
    r->tag = @"replacement";
    r->n = 99;
    [(NSUnarchiver *)c replaceObject:shared withObject:r];
    return self;
}
@end

static void checkReplacement(NSArray *r, const char *what)
{
    printf("%s\n", what);
    CHECK([r count] == 3);
    if ([r count] != 3)
        return;
    ProbeHolder *holder = r[0];
    ProbeNode *held = holder->shared;
    CHECK([held isMemberOfClass:[ProbeNode class]]);
    // macOS: the holder keeps the object it decoded; the later references get the replacement
    CHECK([held->tag isEqualToString:@"shared"] && held->n == 1);
    CHECK([r[1] isMemberOfClass:[ProbeNode class]]);
    CHECK([((ProbeNode *)r[1])->tag isEqualToString:@"replacement"] && ((ProbeNode *)r[1])->n == 99);
    CHECK(r[1] == r[2]);
    CHECK(held != r[1]);
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    @autoreleasepool
    {
        // archive of @[holder, shared, shared], the holder decoding "shared" first
        checkReplacement(decodeData(hexData(fx_replace)), "mac archive");

        ProbeNode *shared = [[[ProbeNode alloc] init] autorelease];
        shared->tag = @"shared";
        shared->n = 1;
        ProbeHolder *h = [[[ProbeHolder alloc] init] autorelease];
        h->shared = shared;
        NSData *d = [NSArchiver archivedDataWithRootObject:@[h, shared, shared]];
        checkReplacement(decodeData(d), "round trip");

        // +decodeClassName:asClassName: applies to every unarchiver created afterwards
        [NSUnarchiver decodeClassName:@"ProbeNode" asClassName:@"RenamedNode"];
        ProbeNode *node = decodeData(hexData(fx_class_plain));
        CHECK([node isMemberOfClass:[RenamedNode class]]);
        CHECK([node->tag isEqualToString:@"node"] && node->n == 7);
        CHECK([[NSUnarchiver classNameDecodedForArchiveClassName:@"ProbeNode"] isEqualToString:@"RenamedNode"]);
        [NSUnarchiver decodeClassName:@"ProbeNode" asClassName:@"ProbeNode"];
        node = decodeData(hexData(fx_class_plain));
        CHECK([node isMemberOfClass:[ProbeNode class]]);

        // an archive that names a class the process does not have decodes through the mapping
        [NSUnarchiver decodeClassName:@"OldProbeNode" asClassName:@"RenamedNode"];
        node = decodeData(hexData(fx_class_encoded));
        CHECK([node isMemberOfClass:[RenamedNode class]]);
        CHECK([node->tag isEqualToString:@"node"] && node->n == 7);

        // the same through the archiver's own encodeClassName:intoClassName:
        NSMutableData *md = [NSMutableData data];
        NSArchiver *a = [[[NSArchiver alloc] initForWritingWithMutableData:md] autorelease];
        [a encodeClassName:@"ProbeNode" intoClassName:@"OldProbeNode"];
        ProbeNode *src = [[[ProbeNode alloc] init] autorelease];
        src->tag = @"node";
        src->n = 7;
        [a encodeRootObject:src];
        node = decodeData(md);
        CHECK([node isMemberOfClass:[RenamedNode class]]);
        CHECK([node->tag isEqualToString:@"node"] && node->n == 7);
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    return failures != 0;
}
