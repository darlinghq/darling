# Execute the production environment builder with controlled CF adapters.
require 'tmpdir'
source = File.read(File.expand_path('../src/frameworks/CoreServices/src/LaunchServices/LaunchServices.cpp', __dir__))
block = source[/\tif \(appParams->environment != nullptr\).*?(?=\n\t\/\/ https:)/m]
abort 'environment block missing' unless block
Dir.mktmpdir('ls-environment') do |dir|
  code = <<~'CPP'
    #include <cassert>
    #include <cstring>
    #include <string>
    #include <vector>
    #include <memory>
    #include <utility>
    using CFIndex = long;
    constexpr int kCFStringEncodingUTF8 = 1;
    struct String { std::string bytes; bool converts = true; int type = 1; };
    using CFStringRef = const String*;
    using Dictionary = std::vector<std::pair<String,String>>;
    struct Parameters { const Dictionary *environment; };
    int CFGetTypeID(CFStringRef value) { return value->type; }
    int CFStringGetTypeID() { return 1; }
    CFIndex CFStringGetLength(CFStringRef value) { return value->bytes.size(); }
    CFIndex CFStringGetMaximumSizeForEncoding(CFIndex length, int) { return length*3; }
    bool CFStringGetCString(CFStringRef value, char *buffer, CFIndex capacity, int) {
      if (!value->converts || capacity <= (CFIndex)value->bytes.size()) return false;
      memcpy(buffer, value->bytes.c_str(), value->bytes.size()+1); return true;
    }
    CFIndex CFDictionaryGetCount(const Dictionary *dict) { return dict->size(); }
    void CFDictionaryApplyFunction(const Dictionary *dict, void (*callback)(const void*,const void*,void*), void *context) {
      for (const auto &entry : *dict) callback(&entry.first, &entry.second, context);
    }
    std::vector<std::string> build(const Dictionary *dict, bool &inherits) {
      Parameters params{dict}; auto *appParams = &params;
      std::unique_ptr<std::vector<char*>> envp;
  CPP
  code += block
  code += <<~'CPP'
      inherits = !envp;
      std::vector<std::string> output;
      if (envp) {
        assert(!envp->empty() && envp->back() == nullptr);
        for (size_t i=0; i+1<envp->size(); ++i) {
          assert((*envp)[i]); output.emplace_back((*envp)[i]);
        }
        for (char *entry : *envp) delete[] entry;
      }
      return output;
    }
    int main() {
      bool inherits;
      assert(build(nullptr,inherits).empty() && inherits);
      Dictionary dict;
      assert(build(&dict,inherits).empty() && !inherits);
      String key, value; key.bytes="KEY"; value.bytes="value";
      dict.push_back({key,value});
      key.bytes="猫"; value.bytes="é 🐈"; dict.push_back({key,value});
      key.bytes="EMPTY"; value.bytes=""; dict.push_back({key,value});
      auto result=build(&dict,inherits);
      assert(!inherits && (result == std::vector<std::string>{"KEY=value","猫=é 🐈","EMPTY="}));
      for (int failure=0; failure<4; ++failure) {
        dict.clear(); key.bytes="KEY"; value.bytes="value";
        key.converts=value.converts=true; key.type=value.type=1;
        if(failure==0) key.converts=false;
        if(failure==1) value.converts=false;
        if(failure==2) key.type=2;
        if(failure==3) value.type=2;
        dict.push_back({key,value});
        assert(build(&dict,inherits).empty() && !inherits);
      }
    }
  CPP
  File.write("#{dir}/probe.cpp",code)
  system('clang++','-std=c++11','-fsanitize=address,undefined',"#{dir}/probe.cpp",'-o',"#{dir}/probe",exception:true)
  system("#{dir}/probe",exception:true)
end
puts 'PASS: environment builder, absent/empty dictionaries, multibyte text, skipped entries and cleanup'
