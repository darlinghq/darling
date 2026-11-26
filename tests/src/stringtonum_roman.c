/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <CarbonCore/NumberFormatting.h>
#include <limits.h>

static int test(ConstStr255Param input, long expected)
{
    long value = 777;
    StringToNum(input, &value);
    return value != expected;
}

int main(void)
{
    const unsigned char minimum[] = {11, '-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
    const unsigned char wrap[] = {10, '4', '2', '9', '4', '9', '6', '7', '2', '9', '6'};
    return test((const unsigned char[]){0}, 0) ||
           test((const unsigned char[]){3, '+', '4', '2'}, 42) ||
           test((const unsigned char[]){3, '1', ',', '2'}, 222) ||
           test(minimum, LONG_MIN) || test(wrap, 0);
}
