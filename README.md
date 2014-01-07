This is a userspace compatibility layer for running Darwin/OS X binaries on Linux.                                                                                                                                                                                             

# General information

For more information (such as **build instructions**), visit http://www.darlinghq.org

# Hacking tutorial

Want to help? Visit the developer zone at http://www.darlinghq.org/developer-zone

## Directory tree

<dl>

<dt>benchmarks/</dt>
<dd>No serious stuff in here, used only once for a simple ObjC msg sending benchmark.</dd>
<dt>etc/</dt>
<dd>Contains dylib.conf which is used to map OS X library (framework) paths to Linux/Darling ones. `/dev/null` = load nothing.</dd>
<dt>include/</dt>
<dd>Header files taken from Darwin/OS X (APSL license). The plan is to get rid of these eventually.</dd>
<dt>misc/</dt>
<dd>Random files, nothing that really matters.</dd>
<dt>src/</dt>
<dd>Source code, see below.</dd>
<dt>tests/</dt>
<dd>Test runner. It is specifically designed to work on my testing setup. See below.</dd>
<dt>tests/src/</dt>
<dd>The source code for tests.</dd>
<dt>tools/</dt>
<dd>Various utilities used during the development.</dd>

</dl>

### Source tree

The structure under `src/`.

<dl>
<dt>src/libSystem/</dt>
<dd>Wrappers or implementation of libc funcions, BSD system calls and Mach system calls.</dd>

<dt>src/libobjcdarwin/</dt>
<dd>Loader of ObjC classes/protocols in Mach-O ObjC applications. Contains code for selector fixups and other techniques needed to "make it work" with GNUstep's libobjc2.</dd>

<dt>src/util/</dt>
<dd>Various utility functions/classes common to all parts of Darling.</dd>

<dt>src/libmach-o/</dt>
<dd>Mach-O parsing/loading library.</dd>

<dt>src/dyld/</dt>
<dd>The dynamic loader.</dd>

<dt>src/motool/</dt>
<dd>A very simple tool for Mach-O file examination. (An allusion to "otool" available on OS X.)</dd>

<dt>src/crash/</dt>
<dd>A crash ("force quit") dialog app for Cocoa apps. Not really complete yet.</dd>

<dt>...</dt>
<dd>The rest is code or wrappers in various stages of completion.</dd>

</dl>

### Tests tree

The testing procedure implemented in `src/tests/runtest.cpp` is as follows:

1. It copies the source file to the OS X machine.
2. It remotely builds the source file. If the current binary name is `runtest32`, then `-m32` is added and `dyld32` is used later on. A similar `runtest64` symlink is needed if your 64-bit dyld is called `dyld64`. If the first line in the source file is `// CFLAGS:`, then the rest is used as CFLAGS.
3. It remotely runs the program and keeps its stdout.
4. It copies the binary over to the local machine.
5. It runs the binary via dyld/dyld32/dyld64.
6. It compares the stdout contents of dyld with that of the remotely run binary.
7. Should the stdout contents differ or should the process exit with a non-zero code on either of the systems, the test has failed.

## Debugging

Find out how Darling can help you with debugging at http://www.darlinghq.org/for-developers/debugging-in-darling

## What NOT to do

To avoid mistakes:

* DO NOT use opencflite and similar Apple CFLite forks, unless you know what you're doing. They lack bridging support with gnustep-base. Bridging between gnustep-corebase and -base is at least work in progress. Should CFLite forks ever fix a bug for you, please help fix the problem in gnustep-corebase.

