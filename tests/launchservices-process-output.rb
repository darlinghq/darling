#!/usr/bin/env ruby
# Executes the actual launch function with real host fork/exec/pipe calls.
# CF argument/environment APIs are unreachable adapters (both inputs are null).
require 'tmpdir'

path = ARGV[0] || File.expand_path('../src/frameworks/CoreServices/src/LaunchServices/LaunchServices.cpp', __dir__)
source = File.read(path)
helper = source[/static\s+int execvpe\(.*?\n\}/m] or abort 'exec helper missing'
launch = source[/OSStatus LSOpenApplication\(.*?\n\}/m] or abort 'launch function missing'
Dir.mktmpdir('ls-process-') do |dir|
  code = <<~'CPP'
    #include <unistd.h>
    #include <sys/wait.h>
    #include <fcntl.h>
    #include <cerrno>
    #include <cassert>
    #include <cstdlib>
    #include <cstring>
    #include <string>
    #include <vector>
    #include <memory>
    #include <algorithm>
    using OSStatus = int;
    using CFIndex = long;
    using CFStringRef = const void*;
    using CFArrayRef = const void*;
    using CFDictionaryRef = const void*;
    constexpr int noErr = 0, paramErr = -50, fnfErr = -43;
    constexpr int kCFStringEncodingUTF8 = 0;
    struct FSRef { const char* path; };
    struct ProcessSerialNumber { unsigned int highLongOfPSN, lowLongOfPSN; };
    struct LSApplicationParameters {
      const FSRef* application;
      CFArrayRef argv;
      CFDictionaryRef environment;
    };
    static bool FSRefMakePath(const FSRef* ref, std::string& out) {
      if (!ref || !ref->path) return false;
      out = ref->path; return true;
    }
    static int makeOSStatus(int error) { return -error; }
    static long CFArrayGetCount(CFArrayRef) { abort(); }
    static const void* CFArrayGetValueAtIndex(CFArrayRef, long) { abort(); }
    static long CFGetTypeID(const void*) { abort(); }
    static long CFStringGetTypeID() { abort(); }
    static const char* CFStringGetCStringPtr(CFStringRef, int) { abort(); }
    static long CFStringGetLength(CFStringRef) { abort(); }
    static long CFDictionaryGetCount(CFDictionaryRef) { abort(); }
    static void CFDictionaryApplyFunction(CFDictionaryRef, void (*)(const void*, const void*, void*), void*) { abort(); }
    static pid_t child;
    static int readMode, reads;
    static pid_t probe_fork() { child = fork(); return child; }
    static ssize_t probe_read(int fd, void* buffer, size_t size) {
      ++reads;
      if (reads == 1) {
        if (readMode == 1) { errno = EINTR; return -1; }
        if (readMode == 2) { errno = EAGAIN; return -1; }
        if (readMode == 3) { errno = EIO; return -1; }
        if (readMode == 4) { memset(buffer, 0, size); return 1; }
      }
      return read(fd, buffer, size);
    }
    #define fork probe_fork
    #define read probe_read
    #define execvpe probe_execvpe
  CPP
  code += helper + "\n" + launch + "\n"
  code += <<~'CPP'
    #undef fork
    #undef read
    #undef execvpe
    static void check(const char* path, int mode, int expected, bool output) {
      FSRef ref{path};
      LSApplicationParameters params{&ref, nullptr, nullptr};
      ProcessSerialNumber psn{0x1234, 0x5678};
      child = -1; readMode = mode; reads = 0;
      OSStatus status = LSOpenApplication(&params, output ? &psn : nullptr);
      assert(status == expected && child > 0);
      int waitStatus;
      assert(waitpid(child, &waitStatus, 0) == child);
      if (expected == noErr && output) {
        assert(psn.highLongOfPSN == 0 && psn.lowLongOfPSN == static_cast<unsigned int>(child));
      } else {
        assert(psn.highLongOfPSN == 0x1234 && psn.lowLongOfPSN == 0x5678);
      }
      if (mode == 1 || mode == 2) assert(reads == 2);
      if (mode == 0 && expected == noErr) assert(WIFEXITED(waitStatus) && WEXITSTATUS(waitStatus) == 0);
      if (mode == 0 && expected != noErr) assert(WIFEXITED(waitStatus) && WEXITSTATUS(waitStatus) == 1);
    }
    int main(int argc, char** argv) {
      assert(argc == 2);
      std::string missing = std::string(argv[1]) + "/missing";
      check("/bin/true", 0, noErr, true);
      check("/bin/true", 0, noErr, false);
      check(missing.c_str(), 0, -ENOENT, true);
      check(argv[1], 0, -EACCES, true); // A directory cannot be exec'd.
      check("/bin/true", 1, noErr, true);
      check("/bin/true", 2, noErr, true);
      check("/bin/true", 3, -EIO, true);
      check("/bin/true", 4, -EIO, true);
      ProcessSerialNumber psn{0x1234, 0x5678};
      assert(LSOpenApplication(nullptr, &psn) == paramErr);
      LSApplicationParameters params{nullptr, nullptr, nullptr};
      assert(LSOpenApplication(&params, &psn) == fnfErr);
      assert(psn.highLongOfPSN == 0x1234 && psn.lowLongOfPSN == 0x5678);
    }
  CPP
  File.write("#{dir}/probe.cpp", code)
  system(ENV.fetch('CXX', 'clang++'), '-std=c++11', '-g', '-fsanitize=address,undefined', "#{dir}/probe.cpp", '-o', "#{dir}/probe", exception: true)
  system("#{dir}/probe", dir, exception: true)
end
puts 'PASS: actual host launch PID output, exec failures and error-pipe handling'
