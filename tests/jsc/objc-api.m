// Copyright (C) 2026 Lubos Dolezel
//
// JavaScriptCore's Objective-C API (64-bit only; the i386 framework has the C
// API only, as on macOS): a JSContext evaluates scripts, JSValue converts
// between JavaScript and Foundation values, a block becomes a callable
// function, exceptions reach the exceptionHandler, and a JSContext made from a
// JSGlobalContextRef shares its global object with the C API.
//   clang -fobjc-arc -o /tmp/jsc-objc-api tests/jsc/objc-api.m -framework JavaScriptCore -framework Foundation
//   (run inside Darling; x86_64)
// Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>
#include <stdio.h>

static int fail;

static void expect(BOOL ok, const char *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    fail |= !ok;
}

int main(void)
{
    @autoreleasepool {
        JSContext *context = [[JSContext alloc] init];
        expect(context != nil, "JSContext is created");
        expect([[context evaluateScript:@"1 + 2"] toInt32] == 3, "evaluateScript returns the value");

        context[@"twice"] = ^(int x) { return x * 2; };
        expect([[context evaluateScript:@"twice(21)"] toInt32] == 42, "a block is callable from JavaScript");

        NSDictionary *object = [[context evaluateScript:@"({a: 1, b: [true, 'x']})"] toDictionary];
        expect([object[@"a"] isEqual:@1] && [object[@"b"] isEqual:(@[@YES, @"x"])], "toDictionary converts objects and arrays");

        JSValue *fromArray = [JSValue valueWithObject:@[@1, @2, @3] inContext:context];
        context[@"list"] = fromArray;
        expect([[context evaluateScript:@"list.length + list[2]"] toInt32] == 6, "an NSArray becomes a JavaScript array");

        __block NSString *message = nil;
        context.exceptionHandler = ^(JSContext *c, JSValue *exception) { message = [exception toString]; };
        [context evaluateScript:@"throw new TypeError('bad')"];
        expect([message isEqualToString:@"TypeError: bad"], "the exceptionHandler receives the exception");

        JSGlobalContextRef global = JSGlobalContextCreate(NULL);
        JSContext *wrapped = [JSContext contextWithJSGlobalContextRef:global];
        [wrapped evaluateScript:@"var shared = 7"];
        JSStringRef name = JSStringCreateWithUTF8CString("shared");
        JSValueRef value = JSObjectGetProperty(global, JSContextGetGlobalObject(global), name, NULL);
        JSStringRelease(name);
        expect(JSValueToNumber(global, value, NULL) == 7, "a JSContext from a JSGlobalContextRef shares the global object");
        expect(wrapped.JSGlobalContextRef == global, "JSGlobalContextRef returns the wrapped context");
        JSGlobalContextRelease(global);
    }
    return fail;
}
