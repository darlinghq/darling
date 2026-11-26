/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <CarbonCore/NumberFormatting.h>
#include <limits.h>
#include <string.h>

static int test(long value, const char *expected)
{
    Str255 text;
    size_t len = strlen(expected);
    memset(text, 0xa5, sizeof(text));
    NumToString(value, text);
    return text[0] != len || memcmp(text + 1, expected, len) ||
           text[len + 1] != 0xa5;
}

int main(void)
{
    return test(0, "0") || test(48, "48") || test(-50, "-50") ||
           test(LONG_MIN, "-2147483648") || test(LONG_MAX, "2147483647");
}
