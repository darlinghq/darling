# Host test: Gestalt's system-version selectors report the macOS product
# version (kern.osproductversion), the same one SystemVersion.plist carries,
# and never the Darwin kernel release.
# usage: ruby tests/gestalt-system-version.rb [REPO_ROOT]
# Gestalt.cpp is compiled unmodified against stand-in sysctl/CoreFoundation
# headers. The fake kernel answers kern.osproductversion with the plist's
# ProductVersion and any KERN_OSRELEASE query with a different Darwin string,
# so a selector that reads the wrong source fails.
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
plist = File.read("#{root}/src/frameworks/CoreServices/SystemVersion.plist")
product = plist[%r{<key>ProductVersion</key>\s*<string>([\d.]+)</string>}, 1] or abort 'ProductVersion missing'
Dir.mktmpdir('gestalt-') do |dir|
  Dir.mkdir("#{dir}/sys"); Dir.mkdir("#{dir}/CoreFoundation")
  File.write("#{dir}/sys/sysctl.h", <<~H)
    #include <stddef.h>
    #define CTL_KERN 1
    #define KERN_OSRELEASE 2
    #ifdef __cplusplus
    extern "C" {
    #endif
    int sysctl(int*, unsigned, void*, size_t*, void*, size_t);
    int sysctlbyname(const char*, void*, size_t*, void*, size_t);
    #ifdef __cplusplus
    }
    #endif
  H
  File.write("#{dir}/CoreFoundation/CFBase.h", "#include <stdint.h>\n")
  File.write("#{dir}/MacTypes.h", "#include <stdint.h>\ntypedef int32_t OSStatus;\n")
  File.write("#{dir}/probe.cpp", <<~CPP)
    #include <assert.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/sysctl.h>
    #include <CarbonCore/MacErrors.h>
    #include <CarbonCore/Gestalt.h>
    static const char *product = "#{product}";
    static int bycall;
    extern "C" int sysctl(int *name, unsigned n, void *buf, size_t *len, void *nw, size_t nl) {
      if (n == 2 && name[0] == CTL_KERN && name[1] == KERN_OSRELEASE) { strncpy((char *)buf, "25.0.0", *len); return 0; }
      return -1;
    }
    extern "C" int sysctlbyname(const char *name, void *buf, size_t *len, void *nw, size_t nl) {
      bycall++;
      if (strcmp(name, "kern.osproductversion") || !product) return -1;
      strncpy((char *)buf, product, *len); return 0;
    }
    static int get(uint32_t t, int *v) { return Gestalt(t, v); }
    int main(int argc, char **argv) {
      int maj = 0, min = 0, patch = 0;
      sscanf("#{product}", "%d.%d.%d", &maj, &min, &patch);
      int v;
      assert(get(gestaltSystemVersionMajor, &v) == 0 && v == maj);
      assert(get(gestaltSystemVersionMinor, &v) == 0 && v == min);
      assert(get(gestaltSystemVersionPatch, &v) == 0 && v == patch);
      assert(maj != 25 && "stand-in kernel release must differ from the product version");
      int packed;
      assert(get(gestaltSystemVersion, &packed) == 0);
      assert(packed == ((((maj / 10) << 4) | (maj % 10)) << 8 | ((min > 9 ? 9 : min) << 4) | (patch > 9 ? 9 : patch)));
      assert(bycall > 0);
      product = "10.9.5";
      assert(get(gestaltSystemVersion, &v) == 0 && v == 0x1095);
      product = "14.6";   /* two components: patch must be 0, not uninitialised */
      assert(get(gestaltSystemVersionPatch, &v) == 0 && v == 0);
      product = "11.7.4";
      assert(get(gestaltSystemVersion, &v) == 0 && v == 0x1174);
      product = "14.6.1";
      assert(get(gestaltSystemVersionMajor, &v) == 0 && v == 14);
      assert(get(gestaltSystemVersionMinor, &v) == 0 && v == 6);
      assert(get(gestaltSystemVersionPatch, &v) == 0 && v == 1);
      assert(get(gestaltSystemVersion, &v) == 0 && v == 0x1461);
      product = "10.15.7";   /* minor saturates at 9 in the packed form */
      assert(get(gestaltSystemVersion, &v) == 0 && v == 0x1097);
      product = "10.9.15";  /* patch saturates at 9 too */
      assert(get(gestaltSystemVersion, &v) == 0 && v == 0x1099);
      product = NULL;     /* sysctl failure: error, not a guessed version */
      v = 123; assert(get(gestaltSystemVersionMajor, &v) == gestaltUnknownErr && v == 0);
      product = "garbage";
      assert(get(gestaltSystemVersionMajor, &v) == gestaltUnknownErr);
      assert(Gestalt(gestaltSystemVersionMajor, NULL) == paramErr);
      puts("PASS: Gestalt system version follows kern.osproductversion");
      return 0;
    }
  CPP
  # Without the target arch, a missing break before gestaltSysArchitecture is invisible on a non-x86 host.
  out, st = Open3.capture2e('clang++', '-fsanitize=address,undefined', '-w', '-D__x86_64__', "-I#{dir}",
    "-I#{root}/src/frameworks/CoreServices/include", "#{dir}/probe.cpp",
    "#{root}/src/frameworks/CoreServices/src/CarbonCore/Gestalt.cpp", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e("#{dir}/probe", rlimit_core: 0)
  puts out
  abort 'gestalt system version regression' unless st.success?
end
