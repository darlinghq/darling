// Copyright (C) 2026 Lubos Dolezel
//
// NSAttributedString decodes archives written by NSArchiver (typedstream): the string followed
// by one entry per attribute run, and encodes them in the same layout.
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

static const char fx_as_empty[] =
    "040b73747265616d747970656481e803840140848484124e534174747269627574656453"
    "7472696e67008484084e534f626a656374008592848484084e53537472696e6701948401"
    "2b008686";

static const char fx_as_noattrs[] =
    "040b73747265616d747970656481e803840140848484124e534174747269627574656453"
    "7472696e67008484084e534f626a656374008592848484084e53537472696e6701948401"
    "2b0568656c6c6f86840269490105928484840c4e5344696374696f6e6172790094840169"
    "008686";

static const char fx_as_one[] =
    "040b73747265616d747970656481e803840140848484124e534174747269627574656453"
    "7472696e67008484084e534f626a656374008592848484084e53537472696e6701948401"
    "2b0568656c6c6f86840269490105928484840c4e5344696374696f6e6172790094840169"
    "0192849696026b418692849696027641868686";

static const char fx_as_one_emptydict[] =
    "040b73747265616d747970656481e803840140848484124e534174747269627574656453"
    "7472696e67008484084e534f626a656374008592848484084e53537472696e6701948401"
    "2b0568656c6c6f86840269490105928484840c4e5344696374696f6e6172790094840169"
    "008686";

static const char fx_as_one_twokeys[] =
    "040b73747265616d747970656481e803840140848484124e534174747269627574656453"
    "7472696e67008484084e534f626a656374008592848484084e53537472696e6701948401"
    "2b0568656c6c6f86840269490105928484840c4e5344696374696f6e6172790094840169"
    "0292849696026b4186928496960276418692849696026b428692848484084e534e756d62"
    "6572008484074e5356616c7565009484012a84999902868686";

static const char fx_as_two[] =
    "040b73747265616d747970656481e803840140848484194e534d757461626c6541747472"
    "696275746564537472696e67008484124e5341747472696275746564537472696e670084"
    "84084e534f626a6563740085928484840f4e534d757461626c65537472696e6701848408"
    "4e53537472696e67019584012b02616286840269490101928484840c4e5344696374696f"
    "6e61727900958401690192849898026b318692848484084e534e756d626572008484074e"
    "5356616c7565009584012a849b9b01868699020192849a9b0192849898026b328692849d"
    "9e9f9b02868686";

static const char fx_as_middle[] =
    "040b73747265616d747970656481e803840140848484194e534d757461626c6541747472"
    "696275746564537472696e67008484124e5341747472696275746564537472696e670084"
    "84084e534f626a6563740085928484840f4e534d757461626c65537472696e6701848408"
    "4e53537472696e67019584012b046162636486840269490101928484840c4e5344696374"
    "696f6e6172790095840169008699020292849a9b0192849898026b318692848484084e53"
    "4e756d626572008484074e5356616c7565009584012a849b9b01868699010186";

static const char fx_as_three_last_plain[] =
    "040b73747265616d747970656481e803840140848484194e534d757461626c6541747472"
    "696275746564537472696e67008484124e5341747472696275746564537472696e670084"
    "84084e534f626a6563740085928484840f4e534d757461626c65537472696e6701848408"
    "4e53537472696e67019584012b08616161626262636386840269490103928484840c4e53"
    "44696374696f6e61727900958401690192849898026b4186928498980276418686990203"
    "92849a9b0292849898026b438692848484084e534e756d626572008484074e5356616c75"
    "65009584012a849b9b038692849898026b428692849898027642868699030292849a9b00"
    "8686";

static const char fx_as_three_aba[] =
    "040b73747265616d747970656481e803840140848484194e534d757461626c6541747472"
    "696275746564537472696e67008484124e5341747472696275746564537472696e670084"
    "84084e534f626a6563740085928484840f4e534d757461626c65537472696e6701848408"
    "4e53537472696e67019584012b08616161626262636386840269490103928484840c4e53"
    "44696374696f6e61727900958401690192849898026b4186928498980276418686990203"
    "92849a9b0192849898026b428692849898027642868699010286";

static const char fx_as_three_distinct[] =
    "040b73747265616d747970656481e803840140848484194e534d757461626c6541747472"
    "696275746564537472696e67008484124e5341747472696275746564537472696e670084"
    "84084e534f626a6563740085928484840f4e534d757461626c65537472696e6701848408"
    "4e53537472696e67019584012b06787879797a7a86840269490102928484840c4e534469"
    "6374696f6e61727900958401690192849898026b318692848484084e534e756d62657200"
    "8484074e5356616c7565009584012a849b9b01868699020292849a9b0192849898026b32"
    "8692849d9e9f9b02868699030292849a9b0192849898026b338692849d9e9f9b03868686";

static const char fx_as_unicode[] =
    "040b73747265616d747970656481e803840140848484124e534174747269627574656453"
    "7472696e67008484084e534f626a656374008592848484084e53537472696e6701948401"
    "2b0968c3a96c6c6fe282ac86840269490106928484840c4e5344696374696f6e61727900"
    "948401690192849696026b418692849696027641868686";


static void checkString(const char *name, NSAttributedString *a, NSString *string, NSUInteger length, NSArray *runs)
{
    if (a == nil)
    {
        printf("FAIL %s: not decoded\n", name);
        failures++;
        return;
    }
    CHECK([a isKindOfClass:[NSAttributedString class]]);
    if (![[a string] isEqualToString:string] || [a length] != length)
    {
        printf("FAIL %s: string [%s]\n", name, [[a string] UTF8String]);
        failures++;
        return;
    }
    NSUInteger i = 0, found = 0;
    while (i < [a length])
    {
        NSRange r;
        NSDictionary *d = [a attributesAtIndex:i effectiveRange:&r];
        if (found >= [runs count])
        {
            printf("FAIL %s: more runs than expected, at %lu\n", name, (unsigned long)i);
            failures++;
            return;
        }
        NSArray *e = runs[found];
        if ([e[0] unsignedIntegerValue] != r.location || [e[1] unsignedIntegerValue] != r.length || ![e[2] isEqualToDictionary:d])
        {
            printf("FAIL %s: run %lu is (%lu,%lu) %s\n", name, (unsigned long)found, (unsigned long)r.location,
                   (unsigned long)r.length, [[d description] UTF8String]);
            failures++;
            return;
        }
        found++;
        i = NSMaxRange(r);
    }
    if (found != [runs count])
    {
        printf("FAIL %s: %lu runs, expected %lu\n", name, (unsigned long)found, (unsigned long)[runs count]);
        failures++;
    }
}

static NSAttributedString *build(NSString *string, NSArray *runs)
{
    NSMutableAttributedString *m = [[[NSMutableAttributedString alloc] initWithString:string] autorelease];
    for (NSArray *e in runs)
    {
        [m setAttributes:e[2] range:NSMakeRange([e[0] unsignedIntegerValue], [e[1] unsignedIntegerValue])];
    }
    return m;
}

static void checkCase(const char *name, const char *hex, NSString *string, NSUInteger length, NSArray *runs)
{
    // the archive written by macOS
    NSAttributedString *a = decodeData(hexData(hex));
    checkString(name, a, string, length, runs);

    // encode with NSArchiver and decode again, for strings with and without attributes
    NSAttributedString *source = build(string, runs);
    NSData *d = [NSArchiver archivedDataWithRootObject:source];
    NSAttributedString *back = decodeData(d);
    checkString(name, back, string, length, runs);
    if (a != nil && back != nil && ![a isEqualToAttributedString:back])
    {
        printf("FAIL %s: round trip differs from the decoded macOS archive\n", name);
        failures++;
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    @autoreleasepool
    {
        checkCase("as-empty", fx_as_empty, @"", 0, @[]);
        checkCase("as-noattrs", fx_as_noattrs, @"hello", 5, @[@[@0, @5, @{}]]);
        checkCase("as-one", fx_as_one, @"hello", 5, @[@[@0, @5, @{@"kA": @"vA"}]]);
        checkCase("as-one-emptydict", fx_as_one_emptydict, @"hello", 5, @[@[@0, @5, @{}]]);
        checkCase("as-one-twokeys", fx_as_one_twokeys, @"hello", 5, @[@[@0, @5, @{@"kA": @"vA", @"kB": @2}]]);
        checkCase("as-two", fx_as_two, @"ab", 2, @[@[@0, @1, @{@"k1": @1}], @[@1, @1, @{@"k2": @2}]]);
        checkCase("as-middle", fx_as_middle, @"abcd", 4, @[@[@0, @1, @{}], @[@1, @2, @{@"k1": @1}], @[@3, @1, @{}]]);
        checkCase("as-three-last-plain", fx_as_three_last_plain, @"aaabbbcc", 8, @[@[@0, @3, @{@"kA": @"vA"}], @[@3, @3, @{@"kB": @"vB", @"kC": @3}], @[@6, @2, @{}]]);
        checkCase("as-three-aba", fx_as_three_aba, @"aaabbbcc", 8, @[@[@0, @3, @{@"kA": @"vA"}], @[@3, @3, @{@"kB": @"vB"}], @[@6, @2, @{@"kA": @"vA"}]]);
        checkCase("as-three-distinct", fx_as_three_distinct, @"xxyyzz", 6, @[@[@0, @2, @{@"k1": @1}], @[@2, @2, @{@"k2": @2}], @[@4, @2, @{@"k3": @3}]]);
        checkCase("as-unicode", fx_as_unicode, [NSString stringWithUTF8String:"h\xc3\xa9llo\xe2\x82\xac"], 6, @[@[@0, @6, @{@"kA": @"vA"}]]);

        // an archive with a first run that spans the whole string decodes as that single run
        NSMutableAttributedString *m = [[[NSMutableAttributedString alloc] initWithString:@"abc" attributes:@{@"k": @1}] autorelease];
        NSAttributedString *back = decodeData([NSArchiver archivedDataWithRootObject:m]);
        CHECK([back isEqualToAttributedString:m]);

        // runs with equal attributes that are not adjacent share one archived dictionary
        m = [[[NSMutableAttributedString alloc] initWithString:@"aabbcc"] autorelease];
        [m setAttributes:@{@"k": @"v"} range:NSMakeRange(0, 2)];
        [m setAttributes:@{@"j": @"w"} range:NSMakeRange(2, 2)];
        [m setAttributes:@{@"k": @"v"} range:NSMakeRange(4, 2)];
        back = decodeData([NSArchiver archivedDataWithRootObject:m]);
        CHECK([back isEqualToAttributedString:m]);
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    return failures != 0;
}
