// This file is plain wrong, broken and horrible.
// The problem is Apple's ld fails to resolve some global variables in the static libraries we link into dyld.
// I have confirmed the problem even on macOS.
//
// If the sought-after symbol is defined (really defined) in a static library, then ld fails to find it.
// When it is defined in an object file passed on the command line, ld has no problem.

void* __cleanup;
char __gdtoa_locks[32];
void* _libkernel_functions[16];

