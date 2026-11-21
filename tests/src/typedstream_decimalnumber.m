// Copyright (C) 2026 Lubos Dolezel
//
// NSDecimalNumber decodes archives written by NSArchiver (typedstream): the fields of NSDecimal
// (exponent, length, sign, compact flag, a reserved word) and the eight mantissa shorts.
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

static const char fx_dec_0[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "fe84015301840143009a018401490884045b38535d8139300000000000000086";

static const char fx_dec_1[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "ff84015301840143019a018401490884045b38535d050000000000000086";

static const char fx_dec_2[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015300840143009a008401490884045b38535d000000000000000086";

static const char fx_dec_3[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015301840143009a018401490884045b38535d010000000000000086";

static const char fx_dec_4[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015301840143009a018401490884045b38535dff0000000000000086";

static const char fx_dec_5[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015302840143009a018401490884045b38535d000100000000000086";

static const char fx_dec_6[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015308840143019a018401490884045b38535d81c0ba81dc7281714181ecea81de0f"
    "81d7bf81e3c61786";

static const char fx_dec_7[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015308840143009a018401490884045b38535d814ef38138de8150908149c4811333"
    "8102f081f6b081490986";

static const char fx_dec_8[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "6484015301840143009a018401490884045b38535d010000000000000086";

static const char fx_dec_9[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "9c84015301840143019a018401490884045b38535d010000000000000086";

static const char fx_dec_10[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "fa84015301840143009a018401490884045b38535d7b0000000000000086";

static const char fx_dec_11[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0a84015302840143009a018401490884045b38535d8187d61200000000000086";

static const char fx_dec_nan[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "0084015300840143019a008401490884045b38535d000000000000000086";

static const char fx_dec_max[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "7f84015308840143009a018401490884045b38535dffffffffffffffff86";

static const char fx_dec_min[] =
    "040b73747265616d747970656481e8038401408484841a4e53446563696d616c4e756d62"
    "6572506c616365686f6c6465720084840f4e53446563696d616c4e756d62657200848408"
    "4e534e756d626572008484074e5356616c7565008484084e534f626a6563740085840169"
    "7f84015308840143019a018401490884045b38535dffffffffffffffff86";


typedef struct
{
    const char *name;
    const char *hex;
    const char *input;     // decimal string the number was created from, NULL for NaN and the limits
    int exponent;
    unsigned length;
    unsigned negative;
    unsigned compact;
    unsigned short mantissa[8];
} Case;

static const Case cases[] = {
    {"dec-0", fx_dec_0, "123.45", -2, 1, 0, 1, {0x3039, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-1", fx_dec_1, "-0.5", -1, 1, 1, 1, {0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-2", fx_dec_2, "0", 0, 0, 0, 0, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-3", fx_dec_3, "1", 0, 1, 0, 1, {0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-4", fx_dec_4, "65535", 0, 1, 0, 1, {0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-5", fx_dec_5, "65536", 0, 2, 0, 1, {0x0000, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-6", fx_dec_6, "-123456789012345678901234567890123456", 0, 8, 1, 1, {0xbac0, 0x72dc, 0x4171, 0xeaec, 0x0fde, 0xbfd7, 0xc6e3, 0x0017}},
    {"dec-7", fx_dec_7, "12345678901234567890123456789012345678", 0, 8, 0, 1, {0xf34e, 0xde38, 0x9050, 0xc449, 0x3313, 0xf002, 0xb0f6, 0x0949}},
    {"dec-8", fx_dec_8, "1e100", 100, 1, 0, 1, {0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-9", fx_dec_9, "-1e-100", -100, 1, 1, 1, {0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-10", fx_dec_10, "0.000123", -6, 1, 0, 1, {0x007b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-11", fx_dec_11, "1234567e10", 10, 2, 0, 1, {0xd687, 0x0012, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-nan", fx_dec_nan, NULL, 0, 0, 1, 0, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}},
    {"dec-max", fx_dec_max, NULL, 127, 8, 0, 1, {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff}},
    {"dec-min", fx_dec_min, NULL, 127, 8, 1, 1, {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff}},
};

static void checkFields(const char *name, NSDecimalNumber *n, const Case *c)
{
    if (![n isKindOfClass:[NSDecimalNumber class]])
    {
        printf("FAIL %s: not an NSDecimalNumber\n", name);
        failures++;
        return;
    }
    NSDecimal d = [n decimalValue];
    CHECK(d._exponent == c->exponent);
    CHECK(d._length == c->length);
    CHECK(d._isNegative == c->negative);
    CHECK(d._isCompact == c->compact);
    CHECK(memcmp(d._mantissa, c->mantissa, sizeof(c->mantissa)) == 0);
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    @autoreleasepool
    {
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
        {
            const Case *c = &cases[i];
            NSDecimalNumber *n = decodeData(hexData(c->hex));
            checkFields(c->name, n, c);
            if (n == nil)
                continue;
            if (c->input != NULL)
            {
                NSDecimalNumber *expected = [NSDecimalNumber decimalNumberWithString:[NSString stringWithUTF8String:c->input] locale:nil];
                if ([n compare:expected] != NSOrderedSame)
                {
                    printf("FAIL %s: %s decoded as %s\n", c->name, c->input, [[n description] UTF8String]);
                    failures++;
                }
            }

            // encode with NSArchiver and decode again
            NSDecimalNumber *back = decodeData([NSArchiver archivedDataWithRootObject:n]);
            checkFields(c->name, back, c);
            if (back != nil && [n compare:back] != NSOrderedSame && ![[n description] isEqualToString:@"NaN"])
            {
                printf("FAIL %s: round trip %s\n", c->name, [[back description] UTF8String]);
                failures++;
            }
        }

        NSDecimalNumber *nan = decodeData(hexData(fx_dec_nan));
        CHECK([[nan description] isEqualToString:@"NaN"]);
        CHECK([nan isEqualToNumber:[NSDecimalNumber notANumber]]);
        CHECK([nan compare:[NSDecimalNumber zero]] != NSOrderedSame);

        NSDecimalNumber *max = decodeData(hexData(fx_dec_max));
        CHECK([max compare:[NSDecimalNumber maximumDecimalNumber]] == NSOrderedSame);
        NSDecimalNumber *min = decodeData(hexData(fx_dec_min));
        CHECK([min compare:[NSDecimalNumber minimumDecimalNumber]] == NSOrderedSame);
        NSDecimalNumber *mixed = [NSDecimalNumber decimalNumberWithString:@"-12.5" locale:nil];
        NSArray *array = decodeData([NSArchiver archivedDataWithRootObject:@[mixed, mixed, @"x"]]);
        CHECK([array count] == 3 && [array[0] compare:mixed] == NSOrderedSame && [array[2] isEqualToString:@"x"]);
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    return failures != 0;
}
