# Host test: the handler LaunchServices reports for a web or mail URL.
# usage: ruby tests/launchservices-host-handler.rb [REPO_ROOT]
# LSHostOpen.c and LSHostOpenBus.c are built unmodified against stand-in CoreFoundation, FSRef and
# elfcalls pieces. The fake host libc answers access() for xdg-open, so the test
# checks that a handler is reported only when the opener exists on the host.
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
ls = "#{root}/src/frameworks/CoreServices/src/LaunchServices"
Dir.mktmpdir('ls-handler-') do |dir|
  %w[CoreFoundation LaunchServices CarbonCore].each { |d| Dir.mkdir("#{dir}/#{d}") }
  File.write("#{dir}/CoreFoundation/CoreFoundation.h", <<~H)
    #pragma once
    #include <stdint.h>
    #include <string.h>
    typedef unsigned char Boolean; typedef unsigned char UInt8; typedef long CFIndex; typedef int32_t OSStatus;
    #define TRUE 1
    #define FALSE 0
    typedef const char *CFStringRef;
    typedef struct { const char *scheme; const char *string; } *CFURLRef;
    typedef const void *CFAllocatorRef;
    typedef int CFComparisonResult;
    enum { kCFCompareCaseInsensitive = 1, kCFCompareEqualTo = 0, kCFStringEncodingUTF8 = 1 };
    typedef int CFStringEncoding;
    #define CFSTR(x) ((CFStringRef) x)
    CFStringRef CFURLCopyScheme(CFURLRef);
    CFComparisonResult CFStringCompare(CFStringRef, CFStringRef, int);
    void CFRelease(const void *);
  H
  File.write("#{dir}/LaunchServices/LaunchServices.h", <<~H)
    #pragma once
    #include <CoreFoundation/CoreFoundation.h>
    typedef struct { char path[256]; } FSRef;
    enum { kLSApplicationNotFoundErr = -10814, kLSServerCommunicationErr = -10822 };
    OSStatus FSPathMakeRef(const UInt8 *, FSRef *, Boolean *);
    CFURLRef CFURLCreateFromFileSystemRepresentation(CFAllocatorRef, const UInt8 *, CFIndex, Boolean);
    CFURLRef CFURLCopyAbsoluteURL(CFURLRef);
    CFStringRef CFURLGetString(CFURLRef);
    CFIndex CFStringGetLength(CFStringRef);
    CFIndex CFStringGetMaximumSizeForEncoding(CFIndex, int);
    Boolean CFStringGetCString(CFStringRef, char *, CFIndex, int);
  H
  File.write("#{dir}/CarbonCore/MacErrors.h", "enum { paramErr = -50, noErr = 0 };\n")
  File.write("#{dir}/probe.c", <<~'C')
    #include <assert.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <elfcalls.h>
    #include <LaunchServices/LaunchServices.h>
    #include <CarbonCore/MacErrors.h>
    #include "LSHostOpen.h"
    struct elf_calls *_elfcalls;
    static const char *present;           /* the xdg-open path that exists on the fake host, or NULL */
    static int bus_calls, bus_result;
    static const char *missing;           /* dlsym name that fails, or NULL */
    static int no_systemd, no_libc;
    static int fake_access(const char *p, int m) { return present && !strcmp(p, present) ? 0 : -1; }
    static unsigned fake_uid(void) { return 1000; }
    static int fake_ok(void) { return 0; }
    static int fake_new(void **r) { *r = malloc(1); return 0; }
    static void *fake_close(void *b) { free(b); return NULL; }
    static int fake_msg(void *b, void **m, const char *d, const char *p, const char *i, const char *mem) { *m = malloc(1); return 0; }
    static void *fake_unref(void *m) { free(m); return NULL; }
    static int fake_call(void *b, void *m, uint64_t u, void *e, void **r) { bus_calls++; *r = NULL; return bus_result; }
    static void fake_efree(void *e) {}
    static void *fake_dlopen(const char *n) {
      if (no_systemd && !strcmp(n, "libsystemd.so.0")) return NULL;
      if (no_libc && !strcmp(n, "libc.so.6")) return NULL;
      return (void *) n;
    }
    static int fake_dlclose(void *l) { return 0; }
    static void *fake_dlsym(void *l, const char *n) {
      if (missing && !strcmp(n, missing)) return NULL;
      if (!strcmp(n, "access")) return (void *) fake_access;
      if (!strcmp(n, "getuid")) return (void *) fake_uid;
      if (!strcmp(n, "sd_bus_new")) return (void *) fake_new;
      if (!strcmp(n, "sd_bus_flush_close_unref")) return (void *) fake_close;
      if (!strcmp(n, "sd_bus_message_new_method_call")) return (void *) fake_msg;
      if (!strcmp(n, "sd_bus_message_unref")) return (void *) fake_unref;
      if (!strcmp(n, "sd_bus_call")) return (void *) fake_call;
      if (!strcmp(n, "sd_bus_error_free")) return (void *) fake_efree;
      return (void *) fake_ok;
    }
    CFStringRef CFURLCopyScheme(CFURLRef u) { return u->scheme; }
    CFComparisonResult CFStringCompare(CFStringRef a, CFStringRef b, int f) { return strcasecmp(a, b); }
    void CFRelease(const void *p) {}
    OSStatus FSPathMakeRef(const UInt8 *p, FSRef *r, Boolean *d) { strncpy(r->path, (const char *) p, 255); return 0; }
    typedef struct { const char *scheme; const char *string; } URLObj;
    static URLObj made;
    CFURLRef CFURLCreateFromFileSystemRepresentation(CFAllocatorRef a, const UInt8 *p, CFIndex n, Boolean d) {
      made.scheme = "file"; made.string = strndup((const char *) p, n); return (CFURLRef) &made; }
    CFURLRef CFURLCopyAbsoluteURL(CFURLRef u) { return u; }
    CFStringRef CFURLGetString(CFURLRef u) { return u->string; }
    CFIndex CFStringGetLength(CFStringRef s) { return strlen(s); }
    CFIndex CFStringGetMaximumSizeForEncoding(CFIndex n, int e) { return n; }
    Boolean CFStringGetCString(CFStringRef s, char *b, CFIndex n, int e) { if ((CFIndex) strlen(s) >= n) return 0; strcpy(b, s); return 1; }
    int main(int argc, char **argv) {
      static struct elf_calls calls;
      const char *mode = argc > 1 ? argv[1] : "handler";
      calls.dlopen = fake_dlopen; calls.dlclose = fake_dlclose; calls.dlsym = fake_dlsym;
      _elfcalls = &calls;
      setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/x/bus", 1);
      URLObj https = { "https", "https://example.org/a" }, mail = { "mailto", "mailto:a@example.org" },
        ftp = { "ftp", "ftp://example.org/" }, spaced = { "https", "https://e.org/a b --new-window" },
        empty = { "https", "" };
      FSRef ref; CFURLRef app = NULL;
      if (!strcmp(mode, "handler")) {
        /* no xdg-open on the host: no handler may be reported, with or without an out FSRef */
        present = NULL;
        assert(_LSGetHostHandlerForURL((CFURLRef) &https, NULL, &app) == kLSApplicationNotFoundErr && app == NULL);
        assert(_LSGetHostHandlerForURL((CFURLRef) &https, &ref, NULL) == kLSApplicationNotFoundErr);
        /* xdg-open present: the path appears under the host root, for web and mail URLs */
        present = "/usr/bin/xdg-open";
        assert(_LSGetHostHandlerForURL((CFURLRef) &https, &ref, &app) == noErr);
        assert(!strcmp(ref.path, "/Volumes/SystemRoot/usr/bin/xdg-open"));
        assert(app && !strcmp(app->string, "/Volumes/SystemRoot/usr/bin/xdg-open"));
        assert(_LSGetHostHandlerForURL((CFURLRef) &mail, &ref, NULL) == noErr);
        /* other schemes are not handled here */
        assert(_LSGetHostHandlerForURL((CFURLRef) &ftp, &ref, &app) == kLSApplicationNotFoundErr);
        assert(_LSIsHostOpenURL((CFURLRef) &https) && _LSIsHostOpenURL((CFURLRef) &mail) && !_LSIsHostOpenURL((CFURLRef) &ftp));
        assert(!_LSIsHostOpenURL(NULL));
        /* no handler for a URL that opening would reject */
        assert(_LSGetHostHandlerForURL((CFURLRef) &spaced, &ref, &app) == kLSApplicationNotFoundErr);
        assert(_LSGetHostHandlerForURL((CFURLRef) &empty, &ref, &app) == kLSApplicationNotFoundErr);
      } else if (!strcmp(mode, "open")) {
        present = "/usr/bin/xdg-open";
        /* success: one portal call */
        assert(_LSOpenURLOnHost((CFURLRef) &https) == noErr && bus_calls == 1);
        assert(_LSOpenURLOnHost((CFURLRef) &mail) == noErr && bus_calls == 2);
        /* a URL the validator rejects, an empty one and NULL make no bus call */
        assert(_LSOpenURLOnHost((CFURLRef) &spaced) == paramErr);
        assert(_LSOpenURLOnHost((CFURLRef) &ftp) == paramErr);
        assert(_LSOpenURLOnHost((CFURLRef) &empty) == paramErr);
        assert(_LSOpenURLOnHost(NULL) == paramErr);
        assert(bus_calls == 2);
        /* a bus failure is reported, not turned into success */
        bus_result = -5;
        assert(_LSOpenURLOnHost((CFURLRef) &https) == kLSServerCommunicationErr);
        { static char big[6000]; memset(big, 'a', sizeof big - 1); memcpy(big, "https://e.org/", 14);
          URLObj huge = { "https", big };
          assert(_LSOpenURLOnHost((CFURLRef) &huge) == paramErr); }
      } else if (!strcmp(mode, "nosystemd")) {
        no_systemd = 1;
        assert(_LSOpenURLOnHost((CFURLRef) &https) == kLSApplicationNotFoundErr && bus_calls == 0);
      } else if (!strcmp(mode, "nolibc")) {
        no_libc = 1;
        assert(_LSOpenURLOnHost((CFURLRef) &https) == kLSApplicationNotFoundErr && bus_calls == 0);
      } else if (!strcmp(mode, "nosymbol")) {
        missing = "sd_bus_call";
        assert(_LSOpenURLOnHost((CFURLRef) &https) == kLSApplicationNotFoundErr && bus_calls == 0);
        present = "/usr/bin/xdg-open";
        assert(_LSGetHostHandlerForURL((CFURLRef) &https, &ref, NULL) == kLSApplicationNotFoundErr);
      } else if (!strcmp(mode, "noelfcalls")) {
        _elfcalls = NULL;
        assert(_LSOpenURLOnHost((CFURLRef) &https) == kLSApplicationNotFoundErr);
      } else abort();
      puts("PASS");
      return 0;
    }
  C
  out, st = Open3.capture2e('clang', '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-fno-sanitize=function', '-w', '-D_GNU_SOURCE',
    "-I#{dir}", "-I#{ls}", "-I#{root}/src/startup/mldr/elfcalls", "#{dir}/probe.c", "#{ls}/LSHostOpen.c", "#{ls}/LSHostOpenBus.c", '-o', "#{dir}/probe")
  abort out unless st.success?
  %w[handler open nosystemd nolibc nosymbol noelfcalls].each do |mode|
    out, st = Open3.capture2e("#{dir}/probe", mode, rlimit_core: 0)
    puts "#{mode}: #{out.lines.last}"
    abort "host handler regression (#{mode})" unless st.success?
  end
end
