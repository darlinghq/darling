// TECGetWebTextEncodings (CarbonCore, Text Encoding Converter): Darling has
// no tables of the encodings each region's web uses, so the call reports a
// missing table (kTECMissingTableErr) and no encodings, and leaves the
// caller's array alone.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s\n", #cond); failures++; } else printf("PASS: %s\n", #cond); } while (0)

int main(void) {
    TextEncoding encodings[4] = { 0x1234, 0x1234, 0x1234, 0x1234 };
    ItemCount count = 99;
    OSStatus status = TECGetWebTextEncodings(0 /* verUS */, encodings, 4, &count);
    CHECK(status == kTECMissingTableErr);
    CHECK(count == 0);
    CHECK(encodings[0] == 0x1234);
    CHECK(TECGetWebTextEncodings(0, encodings, 4, NULL) == kTECMissingTableErr);
    printf("%s\n", failures ? "FAILED" : "ALL PASSED");
    return failures != 0;
}
