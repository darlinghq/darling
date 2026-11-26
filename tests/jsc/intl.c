// Copyright (C) 2026 Lubos Dolezel
//
// JavaScriptCore's Intl objects, which it implements with ICU and its data:
// number and date formatting, collation, plural rules and case mapping give
// the results the ECMAScript Internationalization API specifies for the
// "en-US" and "de-DE" locales.
//   clang -o /tmp/jsc-intl tests/jsc/intl.c -framework JavaScriptCore   (run inside Darling; i386 or x86_64)
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
    check(context, "new Intl.NumberFormat('en-US').format(1234567.891)", "1,234,567.891");
    check(context, "new Intl.NumberFormat('de-DE').format(1234567.891)", "1.234.567,891");
    check(context, "new Intl.NumberFormat('en-US', {style: 'currency', currency: 'EUR'}).format(12.5)", "\xe2\x82\xac" "12.50");
    check(context, "new Intl.DateTimeFormat('en-US', {timeZone: 'UTC', year: 'numeric', month: 'long', day: 'numeric'}).format(Date.UTC(2020, 1, 29))", "February 29, 2020");
    check(context, "new Intl.DateTimeFormat('de-DE', {timeZone: 'UTC', year: 'numeric', month: '2-digit', day: '2-digit'}).format(Date.UTC(2020, 1, 29))", "29.02.2020");
    check(context, "['b', 'a', '\xc3\xa4', 'z'].sort(new Intl.Collator('de').compare).join()", "a,\xc3\xa4,b,z");
    check(context, "new Intl.PluralRules('en-US').select(1) + new Intl.PluralRules('en-US').select(2)", "oneother");
    check(context, "'\xc3\x9f'.toLocaleUpperCase('de-DE')", "SS");
    check(context, "Intl.NumberFormat.supportedLocalesOf(['de-DE']).join()", "de-DE");
    JSGlobalContextRelease(context);
    return fail;
}
