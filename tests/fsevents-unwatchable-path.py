#!/usr/bin/env python3
# Host test: FSEventStream's watch setup skips a path inotify cannot watch.
# usage: python3 tests/fsevents-unwatchable-path.py [REPO_ROOT]
# The for-in loop that registers the watches is cut out of FSEventsImpl.m and
# rewritten to plain C (the NSString/NSDictionary calls become a char* walk and a
# small map), then run against the real inotify API with one existing and one
# missing path. The map must hold only real watch descriptors: a -1 key would
# also be the wd inotify reports for IN_Q_OVERFLOW, which the event handler
# would then attribute to the unwatchable path.

import os
import re
import subprocess
import sys
import tempfile

def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    impl_path = os.path.join(root, "src/frameworks/CoreServices/src/FSEvents/FSEventsImpl.m")
    
    with open(impl_path, "r", encoding="utf-8") as f:
        content = f.read()
    
    match = re.search(r"(\tfor \(NSString\* path in _pathsToWatch\)\n\t\{.*?\n\t\}\n)", content, re.DOTALL)
    if not match:
        sys.exit("watch loop not found in FSEventsImpl.m")
    
    loop_src = match.group(1)
    c = loop_src.replace("for (NSString* path in _pathsToWatch)", "for (const char *path = next_path(); path; path = next_path())")
    c = c.replace("[path UTF8String]", "path")
    c = re.sub(r"\[wdmap setObject:\s*path\s+forKey:\s*\[NSNumber numberWithInt:\s*wd\]\];", "map_set(wd, path);", c)
    
    if re.search(r"\[[a-zA-Z]", c):
        sys.exit(f"unconverted ObjC left in the loop:\n{c}")
    
    with tempfile.TemporaryDirectory(prefix="fsevents-") as tmpdir:
        os.mkdir(os.path.join(tmpdir, "exists"))
        probe_path = os.path.join(tmpdir, "probe.c")
        bin_path = os.path.join(tmpdir, "probe")
        
        probe_src = f"""#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

static const char *paths[] = {{ "{tmpdir}/missing", "{tmpdir}/exists", NULL }};
static int pos;
static const char *next_path(void) {{ return paths[pos] ? paths[pos++] : NULL; }}
static int keys[8];
static int nkeys;
static void map_set(int wd, const char *path) {{ (void)path; keys[nkeys++] = wd; }}

int main(void) {{
    int _fd = inotify_init();
    assert(_fd >= 0);
{c}
    for (int i = 0; i < nkeys; i++) {{
        assert(keys[i] >= 0);
    }}
    assert(nkeys == 1);
    puts("PASS: unwatchable paths are skipped");
    close(_fd);
    return 0;
}}
"""
        with open(probe_path, "w", encoding="utf-8") as f:
            f.write(probe_src)
        
        cc = os.environ.get("CC", "gcc")
        subprocess.check_call([cc, "-Wall", "-Wextra", "-Werror", probe_path, "-o", bin_path])
        out = subprocess.check_output([bin_path]).decode("utf-8")
        print(out.strip())

if __name__ == "__main__":
    main()
