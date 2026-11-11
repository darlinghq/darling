/* Public-API smoke test. Run in a fresh process so the JIT option is set before
 * JavaScriptCore initializes. Availability annotations are also omitted by
 * Darling's JavaScriptCore config.h. */
#ifndef JSC_API_AVAILABLE
#define JSC_API_AVAILABLE(...)
#endif
#include <JavaScriptCore/JavaScript.h>
#include <stdio.h>
#include <stdlib.h>

static JSValueRef evaluate(JSGlobalContextRef context, const char *source,
                          JSValueRef *exception)
{
    JSStringRef script = JSStringCreateWithUTF8CString(source);
    JSValueRef value = JSEvaluateScript(context, script, NULL, NULL, 1, exception);
    JSStringRelease(script);
    return value;
}

int main(void)
{
    if (setenv("JSC_useJIT", "false", 1))
        return 1;
    JSGlobalContextRef context = JSGlobalContextCreate(NULL);
    if (!context)
        return 2;
    JSValueRef exception = NULL;
    JSValueRef value = evaluate(context,
        "(function(){let sum=0; for(let i=0;i<100;i++) sum+=i; return sum;})()",
        &exception);
    if (!value || exception || JSValueToNumber(context, value, NULL) != 4950)
        return 3;
    value = evaluate(context, "throw new Error('expected')", &exception);
    if (value || !exception)
        return 4;
    exception = NULL;
    value = evaluate(context, "21*2", &exception);
    if (!value || exception || JSValueToNumber(context, value, NULL) != 42)
        return 5;
    JSGlobalContextRelease(context);
    puts("PASS: interpreter evaluation, exception and recovery");
    return 0;
}
