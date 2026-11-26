// The sandbox extension class names that libsandbox exports
// (APP_SANDBOX_READ and the others) are pointers to the names, as callers
// such as WebKit declare them, and a token can be issued for one. Observed on
// macOS 26.7.3 (x86_64) with the same declarations.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char *const APP_SANDBOX_READ;
extern const char *const APP_SANDBOX_READ_WRITE;
extern const char *const APP_SANDBOX_MACH;
extern const char *const APP_SANDBOX_IOKIT_CLIENT;
extern const char *const IOS_SANDBOX_APPLICATION_GROUP;
extern const char *const IOS_SANDBOX_CONTAINER;
extern char *sandbox_extension_issue_file(const char *extension_class, const char *path, uint32_t flags);

static int failures;

static void check(const char *name, const char *value, const char *expected)
{
    int ok = value != NULL && strcmp(value, expected) == 0;
    printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok)
        failures++;
}

int main(void)
{
    check("APP_SANDBOX_READ", APP_SANDBOX_READ, "com.apple.app-sandbox.read");
    check("APP_SANDBOX_READ_WRITE", APP_SANDBOX_READ_WRITE, "com.apple.app-sandbox.read-write");
    check("APP_SANDBOX_MACH", APP_SANDBOX_MACH, "com.apple.app-sandbox.mach");
    check("APP_SANDBOX_IOKIT_CLIENT", APP_SANDBOX_IOKIT_CLIENT, "com.apple.app-sandbox.iokit-client");
    check("IOS_SANDBOX_APPLICATION_GROUP", IOS_SANDBOX_APPLICATION_GROUP, "com.apple.sandbox.application-group");
    check("IOS_SANDBOX_CONTAINER", IOS_SANDBOX_CONTAINER, "com.apple.sandbox.container");

    char *token = sandbox_extension_issue_file(APP_SANDBOX_READ, "/tmp", 0);
    printf("%s: a read extension is issued\n", token ? "PASS" : "FAIL");
    if (!token)
        failures++;
    free(token);
    return failures != 0;
}
