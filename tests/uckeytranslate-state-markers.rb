# Host test: UCKeyTranslate does not return state or sequence indexes as characters,
# and still returns the plain UCS values Darling's keyboard layouts hold.
# usage: ruby tests/uckeytranslate-state-markers.rb [REPO_ROOT]
# The UCKeyTranslate function is cut out of UnicodeUtilities.cpp and built with
# the real UCKeyboardLayout structs against a one-table layout with 16 keys. The probe runs
# the same keys against three layouts: no state or sequence tables (as the cocotron
# builders write), a state-records table, and a sequence table with two entries.
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
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <CarbonCore/UnicodeUtilities.h>
    #{fn}
    alignas(8) static unsigned char buf[512];
    static UCKeyboardLayout *build(bool state, bool seq) {
      memset(buf, 0, sizeof buf);
      UCKeyboardLayout *l = (UCKeyboardLayout *) buf;
      const size_t modOff = 128, idxOff = 192, t0 = 256, seqOff = 400;
      l->keyboardTypeCount = 1;
      l->keyboardTypeList[0].keyModifiersToTableNumOffset = modOff;
      l->keyboardTypeList[0].keyToCharTableIndexOffset = idxOff;
      if (state) l->keyboardTypeList[0].keyStateRecordsIndexOffset = 380;
      if (seq) l->keyboardTypeList[0].keySequenceDataIndexOffset = seqOff;
      UCKeyModifiersToTableNum *m = (UCKeyModifiersToTableNum *) (buf + modOff);
      m->defaultTableNum = 0; m->modifiersCount = 1; m->tableNum[0] = 0;
      UCKeyToCharTableIndex *x = (UCKeyToCharTableIndex *) (buf + idxOff);
      x->keyToCharTableSize = 16; x->keyToCharTableCount = 1; x->keyToCharTableOffsets[0] = t0;
      if (seq) ((UCKeySequenceDataIndex *) (buf + seqOff))->charSequenceCount = 2;
      UCKeyOutput *t = (UCKeyOutput *) (buf + t0);
      t[0] = 'a';
      t[1] = 0x4001;           /* dead-key state index */
      t[2] = 0x8001;           /* sequence index 1 */
      t[3] = 0xC000 | 'z';     /* top bits 11: a character */
      t[4] = 0xAC00;           /* HANGUL SYLLABLE GA: top bits 10 */
      t[5] = 0x9999;           /* a CJK ideograph: top bits 10 */
      t[6] = 0x4E00;           /* CJK ideograph one: top bits 01 */
      t[7] = 0x8005;           /* sequence index beyond a two-entry list */
      t[8] = 0xFFFE; t[9] = 0xFFFF;
      t[10] = 0x00E9;          /* e acute */
      return l;
    }
    static int fails = 0;
    /* want < 0: no text; otherwise the one character expected */
    static void check(const char *what, UCKeyboardLayout *l, UInt16 key, long want) {
      UniChar out[4] = {0}; UniCharCount n = 99; UInt32 dead = 0;
      OSStatus st = UCKeyTranslate(l, key, 3, 0, 0, 0, &dead, 4, &n, out);
      bool ok = st == noErr && (want < 0 ? n == 0 : (n == 1 && out[0] == want));
      if (!ok) { printf("FAIL: %s: key %u gave n=%lu out0=%#x, wanted %ld\\n", what, key, (unsigned long) n, out[0], want); fails++; }
    }
    int main() {
      UCKeyboardLayout *l = build(false, false);
      check("plain", l, 0, 'a');
      check("11 prefix", l, 3, 0xC000 | 'z');
      check("no tables: U+AC00", l, 4, 0xAC00);
      check("no tables: U+9999", l, 5, 0x9999);
      check("no tables: U+4E00 (guess)", l, 6, 0x4E00);
      check("no tables: 10 prefix index", l, 2, 0x8001);
      check("no tables: 0xFFFE", l, 8, -1);
      check("no tables: 0xFFFF", l, 9, -1);
      check("no tables: e acute", l, 10, 0x00E9);
      l = build(true, false);
      check("state table: dead-key index", l, 1, -1);
      check("state table: U+4E00 is an index", l, 6, -1);
      check("state table, no sequences: U+AC00", l, 4, 0xAC00);
      l = build(false, true);
      check("sequence table: index 1", l, 2, -1);
      check("sequence table: U+AC00 is beyond the list", l, 4, 0xAC00);
      check("sequence table: U+9999 is beyond the list", l, 5, 0x9999);
      check("sequence table: index 5 beyond the list", l, 7, 0x8005);
      check("sequence table: 11 prefix", l, 3, 0xC000 | 'z');
      if (fails) { fflush(stdout); exit(1); }
      puts("PASS: UCKeyTranslate state and sequence indexes");
      return 0;
    }
  CPP
  out, st = Open3.capture2e('clang++', '-std=c++17', '-fsanitize=address,undefined', '-fno-sanitize-recover=undefined', '-w',
    "-I#{dir}", "-I#{root}/src/frameworks/CoreServices/include", "#{dir}/probe.cpp", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e({ 'ASAN_OPTIONS' => 'disable_coredump=1' }, 'timeout', '60', "#{dir}/probe", rlimit_core: 0)
  puts out
  abort 'UCKeyTranslate state marker regression' unless st.success?
end
