
// BUILD:  $CC main.c            -o $BUILD_DIR/crt-vars-libSystem.exe

// RUN:  ./crt-vars-libSystem.exe

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <crt_externs.h>
#include <mach-o/ldsyms.h>

// This struct is passed as fifth parameter to libSystem.dylib's initializer so it record
// the address of crt global variables.
struct ProgramVars
{
    const void*        mh;
    int*               NXArgcPtr;
    char***            NXArgvPtr;
    char***            environPtr;
    char**             __prognamePtr;
};


// global variables defeined in crt1.o
extern char**  NXArgv;
extern int     NXArgc;
extern char**  environ;
extern char*   __progname;


static const struct ProgramVars* sVars;

void __attribute__((constructor))
myInit(int argc, const char* argv[], const char* envp[], const char* apple[], const struct ProgramVars* vars)
{
    sVars = vars;
}


int main(int argc, const char* argv[])
{
    printf("[BEGIN] crt-vars-libSystem\n");
    bool success = true;

    if ( _NSGetArgv() != &NXArgv ) {
        printf("[FAIL] crt-libSystem: _NSGetArgv() != &NXArgv (%p!=%p) for %s", _NSGetArgv(), &NXArgv, argv[0]);
        success = false;
    }

    if ( _NSGetArgc() != &NXArgc ) {
        printf("[FAIL] crt-libSystem: _NSGetArgc() != &NXArgc (%p!=%p) for %s", _NSGetArgc(), &NXArgc, argv[0]);
        success = false;
    }

    if ( _NSGetEnviron() != &environ ) {
        printf("[FAIL] crt-libSystem: _NSGetEnviron() != &environv (%p!=%p) for %s", _NSGetEnviron(), &environ, argv[0]);
        success = false;
    }

    if ( _NSGetProgname() != &__progname ) {
        printf("[FAIL] crt-libSystem: _NSGetProgname() != &__progname (%p!=%p) for %s", _NSGetProgname(), &__progname, argv[0]);
        success = false;
    }

    if ( _NSGetMachExecuteHeader() != &_mh_execute_header ) {
        printf("[FAIL] crt-libSystem: _NSGetMachExecuteHeader() != &_mh_execute_headerv (%p!=%p) for %s", _NSGetMachExecuteHeader(), &_mh_execute_header, argv[0]);
        success = false;
    }

    if ( sVars->NXArgvPtr != &NXArgv ) {
        printf("[FAIL] crt-libSystem: sVars->NXArgvPtr != &NXArg (%p!=%p) for %s", sVars->NXArgvPtr, &NXArgv, argv[0]);
        success = false;
    }

    if ( sVars->NXArgcPtr != &NXArgc ) {
        printf("[FAIL] crt-libSystem: sVars->NXArgcPtr != &NXArgc (%p!=%p) for %s", sVars->NXArgcPtr, &NXArgc, argv[0]);
        success = false;
    }

    if ( sVars->environPtr != &environ ) {
        printf("[FAIL] crt-libSystem: sVars->environPtr != &environ (%p!=%p) for %s", sVars->environPtr, &environ, argv[0]);
        success = false;
    }

    if ( sVars->__prognamePtr != &__progname ) {
        printf("[FAIL] crt-libSystem: sVars->__prognamePtr != &__progname (%p!=%p) for %s", sVars->__prognamePtr, &__progname, argv[0]);
        success = false;
    }

    if ( sVars->mh != &_mh_execute_header ) {
        printf("[FAIL] crt-libSystem: sVars->mh != &_mh_execute_header (%p!=%p) for %s", sVars->mh, &_mh_execute_header, argv[0]);
        success = false;
    }

    if ( success )
        printf("[PASS] crt-vars-libSystem\n");

    return 0;
}

