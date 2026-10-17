# Host test: FSEventStream's watch setup skips a path inotify cannot watch.
# usage: ruby tests/fsevents-unwatchable-path.rb [REPO_ROOT]
# The for-in loop that registers the watches is cut out of FSEventsImpl.m and
# rewritten to plain C (the NSString/NSDictionary calls become a char* walk and a
# small map), then run against the real inotify API with one existing and one
# missing path. The map must hold only real watch descriptors: a -1 key would
# also be the wd inotify reports for IN_Q_OVERFLOW, which the event handler
# would then attribute to the unwatchable path.
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
m = File.read("#{root}/src/frameworks/CoreServices/src/FSEvents/FSEventsImpl.m")
loop_src = m[/^\tfor \(NSString\* path in _pathsToWatch\)\n\t\{.*?^\t\}\n/m] or abort 'watch loop not found'
c = loop_src
  .sub('for (NSString* path in _pathsToWatch)', 'for (const char *path = next_path(); path; path = next_path())')
  .gsub('[path UTF8String]', 'path')
  .gsub(/\[wdmap setObject: path\s+forKey: \[NSNumber numberWithInt: wd\]\];/, 'map_set(wd, path);')
abort "unconverted ObjC left in the loop:\n#{c}" if c =~ /\[[a-zA-Z]/
Dir.mktmpdir('fsevents-') do |dir|
  Dir.mkdir("#{dir}/exists")
  File.write("#{dir}/probe.c", <<~C)
    #define _GNU_SOURCE
    #include <assert.h>
    #include <stdio.h>
    #include <string.h>
    #include <sys/inotify.h>
    #include <unistd.h>
    static const char *paths[] = { "#{dir}/missing", "#{dir}/exists", NULL };
    static int pos;
    static const char *next_path(void) { return paths[pos] ? paths[pos++] : NULL; }
    static int keys[8]; static int nkeys;
    static void map_set(int wd, const char *path) { keys[nkeys++] = wd; }
    int main(void) {
      int _fd = inotify_init();
      assert(_fd >= 0);
    #{c}
      for (int i = 0; i < nkeys; i++) assert(keys[i] >= 0);   /* no -1 key */
      assert(nkeys == 1);                                     /* only the existing path */
      puts("PASS: unwatchable paths are skipped");
      return 0;
    }
  C
  out, st = Open3.capture2e('clang', '-w', "#{dir}/probe.c", '-o', "#{dir}/probe")
  abort out unless st.success?
  out, st = Open3.capture2e("#{dir}/probe")
  puts out
  abort 'fsevents unwatchable path regression' unless st.success?
end
