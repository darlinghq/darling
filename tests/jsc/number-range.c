// Copyright (C) 2026 Lubos Dolezel
//
// JavaScript numbers outside the int32 range keep their values (ECMAScript
// numbers are IEEE 754 doubles): integral results of Math functions, Date
// times after January 2038 in milliseconds (2^31 ms is in January 1970), and
// values next to the int32 limits; -0 stays negative zero.
//   clang -o /tmp/jsc-number-range tests/jsc/number-range.c -framework JavaScriptCore   (run inside Darling; i386 or x86_64)
// Exits non-zero on failure.
#include <JavaScriptCore/JavaScript.h>
#include <stdio.h>
#include <string.h>

static int fail;

static void check(JSGlobalContextRef context, const char *source, const char *expected)
{
    JSStringRef script = JSStringCreateWithUTF8CString(source);
    JSValueRef exception = NULL;
    JSValueRef value = JSEvaluateScript(context, script, NULL, NULL, 1, &exception);
    JSStringRelease(script);
    char result[256] = "(exception)";
    if (value && !exception) {
        JSStringRef string = JSValueToStringCopy(context, value, NULL);
        JSStringGetUTF8CString(string, result, sizeof result);
        JSStringRelease(string);
    }
    int ok = strcmp(result, expected) == 0;
    printf("%s: %s = \"%s\"\n", ok ? "PASS" : "FAIL", source, result);
    fail |= !ok;
}

int main(void)
{
    JSGlobalContextRef context = JSGlobalContextCreate(NULL);
    if (!context)
        return 2;
    check(context, "Math.trunc(3e9)", "3000000000");
    check(context, "Math.floor(-3e9 - 0.5)", "-3000000001");
    check(context, "Math.round(4294967296.4)", "4294967296");
    check(context, "2147483647 + 1", "2147483648");
    check(context, "-2147483648 - 1", "-2147483649");
    check(context, "[2147483648, -2147483649, 1e15].map(x => Math.trunc(x)).join()", "2147483648,-2147483649,1000000000000000");
    check(context, "Date.UTC(2020, 1, 29)", "1582934400000");
    check(context, "new Date(Date.UTC(2020, 1, 29)).toISOString()", "2020-02-29T00:00:00.000Z");
    check(context, "new Date(8.64e15).getTime()", "8640000000000000");
    check(context, "1 / Math.trunc(-0.5)", "-Infinity");
    check(context, "(3e9 | 0)", "-1294967296");
    JSGlobalContextRelease(context);
    return fail;
}
