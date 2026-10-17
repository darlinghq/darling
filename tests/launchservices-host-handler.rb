# Host test: the handler LaunchServices reports for a web or mail URL.
# usage: ruby tests/launchservices-host-handler.rb [REPO_ROOT]
# LSHostOpen.c is built unmodified against stand-in CoreFoundation, FSRef and
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
    typedef struct { const char *s; } *CFStringRef;
    typedef struct { const char *scheme; } *CFURLRef;
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
    static int fake_access(const char *p, int m) { return present && !strcmp(p, present) ? 0 : -1; }
    static unsigned fake_uid(void) { return 1000; }
    static int fake_sd(void) { return 0; }
    static void *fake_dlopen(const char *n) { return (void *) n; }
    static int fake_dlclose(void *l) { return 0; }
    static void *fake_dlsym(void *l, const char *n) {
      if (!strcmp(n, "access")) return (void *) fake_access;
      if (!strcmp(n, "getuid")) return (void *) fake_uid;
      return (void *) fake_sd;
    }
    CFStringRef CFURLCopyScheme(CFURLRef u) { return u->scheme ? (CFStringRef) u->scheme : NULL; }
    CFComparisonResult CFStringCompare(CFStringRef a, CFStringRef b, int f) { return strcasecmp((const char *) a, (const char *) b); }
    void CFRelease(const void *p) {}
    OSStatus FSPathMakeRef(const UInt8 *p, FSRef *r, Boolean *d) { strncpy(r->path, (const char *) p, 255); return 0; }
    static struct { const char *scheme; } url_obj;
    CFURLRef CFURLCreateFromFileSystemRepresentation(CFAllocatorRef a, const UInt8 *p, CFIndex n, Boolean d) {
      url_obj.scheme = strndup((const char *) p, n); return (CFURLRef) &url_obj; }
    CFURLRef CFURLCopyAbsoluteURL(CFURLRef u) { return u; }
    CFStringRef CFURLGetString(CFURLRef u) { return NULL; }
    CFIndex CFStringGetLength(CFStringRef s) { return 0; }
    CFIndex CFStringGetMaximumSizeForEncoding(CFIndex n, int e) { return n; }
    Boolean CFStringGetCString(CFStringRef s, char *b, CFIndex n, int e) { return 0; }
    int main(void) {
      static struct elf_calls calls;
      calls.dlopen = fake_dlopen; calls.dlclose = fake_dlclose; calls.dlsym = fake_dlsym;
      _elfcalls = &calls;
      struct { const char *scheme; } https = { "https" }, ftp = { "ftp" };
      FSRef ref; CFURLRef app = NULL;
      /* no xdg-open on the host: no handler may be reported, with or without an out FSRef */
      present = NULL;
      assert(_LSGetHostHandlerForURL((CFURLRef) &https, NULL, &app) == kLSApplicationNotFoundErr && app == NULL);
      assert(_LSGetHostHandlerForURL((CFURLRef) &https, &ref, NULL) == kLSApplicationNotFoundErr);
      /* xdg-open present: the path appears under the host root */
      present = "/usr/bin/xdg-open";
      assert(_LSGetHostHandlerForURL((CFURLRef) &https, &ref, &app) == noErr);
      assert(!strcmp(ref.path, "/Volumes/SystemRoot/usr/bin/xdg-open"));
      assert(app && !strcmp(((struct { const char *s; } *) app)->s, "/Volumes/SystemRoot/usr/bin/xdg-open"));
      /* other schemes are not handled here */
      assert(_LSGetHostHandlerForURL((CFURLRef) &ftp, &ref, &app) == kLSApplicationNotFoundErr);
      puts("PASS: host handler reported only when xdg-open exists");
      return 0;
    }
  C
  out, st = Open3.capture2e('clang', '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-w', '-D_GNU_SOURCE',
    "-I#{dir}", "-I#{ls}", "-I#{root}/src/startup/mldr/elfcalls", "#{dir}/probe.c", "#{ls}/LSHostOpen.c", "#{ls}/LSHostOpenBus.c", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e("#{dir}/probe", rlimit_core: 0)
  puts out
  abort 'host handler regression' unless st.success?
end
