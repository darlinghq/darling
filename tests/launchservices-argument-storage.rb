#!/usr/bin/env ruby
# Execute the actual pre-launch argument-building block with CF adapters.
# This tests storage and conversion, not the supplied argv[0] API convention.
require 'tmpdir'
path = ARGV[0] || File.expand_path('../src/frameworks/CoreServices/src/LaunchServices/LaunchServices.cpp', __dir__)
source = File.read(path)
function = source[/OSStatus LSOpenApplication\(.*?\n\}/m] or abort 'launch missing'
block = function.split("\tstd::string exePath;", 2)[1]&.split("\tif (appParams->environment", 2)&.first or abort 'argument block missing'
Dir.mktmpdir('ls-arguments-') do |dir|
  code = <<~'CPP'
    #include <cassert>
    #include <cstring>
    #include <string>
    #include <vector>
    #include <memory>
    #include <sys/types.h>
    using OSStatus = int;
    using CFIndex = long;
    constexpr int noErr = 0, paramErr = -50, fnfErr = -43, kCFStringEncodingUTF8 = 1;
    struct String {
      std::string bytes;
      bool direct = false, convertible = true, validCapacity = true;
      int type = 1;
      String(std::string text) : bytes(text) {}
    };
    using CFStringRef = const String*;
    using CFArrayRef = const std::vector<String>*;
    struct FSRef { const char* path; };
    struct LSApplicationParameters { const FSRef* application; CFArrayRef argv; };
    static bool FSRefMakePath(const FSRef* ref, std::string& result) {
      if (!ref || !ref->path) return false;
      result = ref->path; return true;
    }
    static CFIndex CFArrayGetCount(CFArrayRef array) { return array->size(); }
    static const void* CFArrayGetValueAtIndex(CFArrayRef array, CFIndex i) { return &array->at(i); }
    static int CFGetTypeID(CFStringRef s) { return s->type; }
    static int CFStringGetTypeID() { return 1; }
    static CFIndex CFStringGetLength(CFStringRef s) { return s->validCapacity ? s->bytes.size() : -1; }
    static CFIndex CFStringGetMaximumSizeForEncoding(CFIndex length, int encoding) {
      assert(encoding == kCFStringEncodingUTF8);
      return length < 0 ? -1 : length * 3;
    }
    static const char* CFStringGetCStringPtr(CFStringRef s, int) { return s->direct ? s->bytes.c_str() : nullptr; }
    static bool CFStringGetCString(CFStringRef s, char* output, CFIndex size, int encoding) {
      assert(encoding == kCFStringEncodingUTF8);
      if (!s->convertible || size <= (CFIndex)s->bytes.size()) return false;
      memcpy(output, s->bytes.c_str(), s->bytes.size() + 1);
      return true;
    }
    static OSStatus build(const LSApplicationParameters* appParams, std::vector<std::string>& result) {
      std::string exePath;
  CPP
  code += block
  code += <<~'CPP'
      // Consume pointers only after the actual block finished growing storage.
      for (size_t i = 0; argv[i]; ++i) result.emplace_back(argv[i]);
      return noErr;
    }
    int main() {
      FSRef ref{"/fixture/Runner"};
      LSApplicationParameters params{&ref, nullptr};
      std::vector<std::string> output;
      assert(build(&params, output) == noErr);
      assert(output == std::vector<std::string>{"/fixture/Runner"});
      std::vector<String> strings;
      params.argv = &strings; output.clear();
      assert(build(&params, output) == noErr && output.empty());
      // Preserve this PR's existing supplied-argv convention; no executable prepend.
      strings = {String("program"), String("ASCII"), String("é 🐈"), String(""), String("after")};
      strings[0].direct = true;
      output.clear(); assert(build(&params, output) == noErr);
      assert((output == std::vector<std::string>{"program", "ASCII", "é 🐈", "", "after"}));
      strings.clear();
      for (int i = 0; i < 2000; ++i) strings.emplace_back(std::to_string(i) + std::string(i % 100, 'x'));
      output.clear(); assert(build(&params, output) == noErr);
      assert(output.size() == strings.size());
      for (size_t i = 0; i < strings.size(); ++i) assert(output[i] == strings[i].bytes);
      for (int failure = 0; failure < 3; ++failure) {
        strings = {String("first"), String("bad"), String("last")};
        if (failure == 0) strings[1].type = 2;
        if (failure == 1) strings[1].convertible = false;
        if (failure == 2) strings[1].validCapacity = false;
        output.clear(); assert(build(&params, output) == paramErr && output.empty());
      }
      params.application = nullptr;
      assert(build(&params, output) == fnfErr && output.empty());
    }
  CPP
  File.write("#{dir}/probe.cpp", code)
  system('clang++', '-std=c++11', '-fsanitize=address,undefined', '-g', "#{dir}/probe.cpp", '-o', "#{dir}/probe", exception: true)
  system("#{dir}/probe", exception: true)
end
puts 'PASS: actual argument conversion, indirect strings, empty values, growth and failures'
