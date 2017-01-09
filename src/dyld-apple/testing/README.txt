
When the dyld_tests target is built, all test cases are built into /AppleInternal/.
A test case is a directory in $SRCROOT/testing/test-cases/ whose name ends in ".dtest".
The build system scraps any .c or .cxx files in the .dtest directory looking for BUILD: or RUN: lines.
The BUILD: lines are use to build the test case binaries.
The RUN: lines are used to build the information needed for BATS to run the test cases.
Example, main.c may contain:

    // BUILD:  $CC main.c  -o $BUILD_DIR/example.exe
    // RUN:  ./example.exe
    int main() { return 0; }

When build lines are executed, the current directory is set to the test case's .dtest dir.
Build lines may contain the follow variables:
    $BUILD_DIR   - expands to the directory in $DSTROOT where this test case binaries are installed
    $RUN_DIR     - expands to the directory in /AppleInternal/ where this test case binaries will be run
    $TEMP_DIR    - expands to a temporary directory that will be delete after this test case is built
    $CC          - expands to the C compiler command line for the current platform.  It includes
                   the min-os-version option, then SDK option, and the architectures.
    $CXX         - expands to the C++ compiler plus standard options

When run lines are executed, the current directory is set to what $RUN_DIR was during build.
Run lines may contain the follow variables:
    $RUN_DIR       - expands to the directory in /AppleInternal/ where this test case binaries are installed
    $REQUIRE_CRASH - expands to the 'nocr' tool which is used when a program is expected to "crash" in order to pass


When a test program runs, it should initially print out the name of the test case. Ex:
 [BEGIN] dlfoo-thread-safe
Then it should print a pass or fail message with the same test name. Ex:
 [PASS] dlfoo-thread-safe


To support tests that dyld is supposed to terminate, use $REQUIRE_CRASH on the RUN: line
along with setting env var NOCR_TEST_NAME to the name of the test case.  When that env
var is set, the nocr tool wil print out the [BEGIN], then [PASS] if the test crashes
otherwise [FAIL].  Ex:
  // RUN:  NOCR_TEST_NAME="dylib-static-link missing" $REQUIRE_CRASH ./dylib-static-missing.exe


To support tests that are platform specific, add the BUILD_ONLY: line which specifies the platform.
Valid platforms are: MacOSX, iOS, watchOS, and tvOS.  When a specific platform is specified, a
new min OS version can also be specified via the BUILD_MIN_OS option.  For instance:
    // BUILD_ONLY: MacOSX
    // BUILD_MIN_OS: 10.5


