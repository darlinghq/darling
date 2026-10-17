# Host test: UCKeyTranslate indexes the modifier table with the documented
# modifier byte, (EventRecord.modifiers >> 8) & 0xFF, without shifting it again.
# usage: ruby tests/uckeytranslate-modifier-state.rb [REPO_ROOT]
# The UCKeyTranslate function is cut out of UnicodeUtilities.cpp and built with
# the real UCKeyboardLayout structs against a two-table layout: table 0 maps
# key 0 to 'a', table 1 maps it to 'A', and modifier index 2 (shift, 0x02) selects table 1.
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
cpp = File.read("#{root}/src/frameworks/CoreServices/src/CarbonCore/UnicodeUtilities.cpp")
fn = cpp[/^OSStatus UCKeyTranslate\(.*?^\}\n/m] or abort 'UCKeyTranslate not found'
Dir.mktmpdir('uckey-') do |dir|
  File.write("#{dir}/MacTypes.h", <<~H)
    #pragma once
    #include <stdint.h>
    typedef uint8_t UInt8; typedef uint16_t UInt16; typedef uint32_t UInt32; typedef int32_t OSStatus;
    typedef unsigned char Boolean; typedef UInt16 UniChar; typedef unsigned long UniCharCount; typedef UInt32 OptionBits;
    enum { noErr = 0 };
  H
  Dir.mkdir("#{dir}/CoreFoundation")
  File.write("#{dir}/CoreFoundation/CFBase.h", "#include <stdint.h>\n")
  File.write("#{dir}/probe.cpp", <<~CPP)
    #include <assert.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <CarbonCore/UnicodeUtilities.h>
    #{fn}
    int main() {
      /* layout: header | modifier map | key-to-char index | two 4-key tables */
      alignas(8) static unsigned char buf[512]; memset(buf, 0, sizeof buf);
      UCKeyboardLayout *l = (UCKeyboardLayout *) buf;
      const size_t modOff = 128, idxOff = 192, t0 = 256, t1 = 288;
      l->keyboardTypeCount = 1;
      l->keyboardTypeList[0].keyModifiersToTableNumOffset = modOff;
      l->keyboardTypeList[0].keyToCharTableIndexOffset = idxOff;
      UCKeyModifiersToTableNum *m = (UCKeyModifiersToTableNum *) (buf + modOff);
      m->defaultTableNum = 0; m->modifiersCount = 4;
      m->tableNum[0] = 0; m->tableNum[1] = 0; m->tableNum[2] = 1; m->tableNum[3] = 1;
      UCKeyToCharTableIndex *x = (UCKeyToCharTableIndex *) (buf + idxOff);
      x->keyToCharTableSize = 4; x->keyToCharTableCount = 2;
      x->keyToCharTableOffsets[0] = t0; x->keyToCharTableOffsets[1] = t1;
      ((UCKeyOutput *) (buf + t0))[0] = 'a'; ((UCKeyOutput *) (buf + t1))[0] = 'A';
      UniChar out[4]; UniCharCount n; UInt32 dead = 0;
      auto tr = [&](UInt32 mods) { out[0] = 0; assert(UCKeyTranslate(l, 0, 3, mods, 0, 0, &dead, 4, &n, out) == noErr && n == 1); return out[0]; };
      assert(tr(0x00) == 'a');            /* no modifiers */
      assert(tr(0x02) == 'A');            /* shift, as the documented byte */
      assert(tr(0x03) == 'A');
      assert(tr(0x200) == 'a');           /* the raw EventRecord flag is outside the table: default */
      assert(tr(0xFFFFFFFFu) == 'a');     /* far out of range: default table, no overread */
      puts("PASS: UCKeyTranslate uses the documented modifier byte");
      return 0;
    }
  CPP
  out, st = Open3.capture2e('clang++', '-std=c++17', '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-w',
    "-I#{dir}", "-I#{root}/src/frameworks/CoreServices/include", "#{dir}/probe.cpp", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e("#{dir}/probe", rlimit_core: 0)
  puts out
  abort 'UCKeyTranslate modifier regression' unless st.success?
end
