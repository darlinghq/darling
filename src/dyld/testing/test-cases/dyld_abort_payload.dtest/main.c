
// BUILD:  $CC foo.c -dynamiclib -install_name /cant/find/me.dylib -o $BUILD_DIR/libmissing.dylib
// BUILD:  $CC emptyMain.c $BUILD_DIR/libmissing.dylib  -o $BUILD_DIR/prog_missing_dylib.exe
// BUILD:  $CC defSymbol.c -dynamiclib -install_name libMissingSymbols.dylib -o $BUILD_DIR/libMissingSymbols.dylib
// BUILD:  $CC defSymbol.c -dynamiclib -install_name libMissingSymbols.dylib -o $BUILD_DIR/libHasSymbols.dylib -DHAS_SYMBOL
// BUILD:  $CC useSymbol.c $BUILD_DIR/libHasSymbols.dylib -o $BUILD_DIR/prog_missing_symbol.exe
// BUILD:  $CC main.c -o $BUILD_DIR/dyld_abort_tests.exe

// RUN:  ./dyld_abort_tests.exe 

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <err.h>
#include <System/sys/reason.h>
#include <System/sys/proc_info.h>
#include <System/kern/kern_cdata.h>
#include <libproc.h>
#include <mach-o/dyld_priv.h>


static bool        sSignalCaught = false;
static bool        sChildAbortInfoCorrect = false;
static pid_t       sChildPid = 0;
static uint64_t    sExpectedDyldReason = 0;
static const char* sExpectedDylibPath = NULL;
static const char* sExpectedSymbol = NULL;


static void childDied(int sig)
{
    sSignalCaught = true;
    //printf("sigchld for pid=%d\n", sChildPid);

    struct proc_exitreasoninfo info;
    bzero(&info, sizeof(info));
    uint8_t packReasonData[OS_REASON_BUFFER_MAX_SIZE];
    bzero(packReasonData, OS_REASON_BUFFER_MAX_SIZE);
    info.eri_reason_buf_size = OS_REASON_BUFFER_MAX_SIZE;
    info.eri_kcd_buf = (user_addr_t)packReasonData;
    //fprintf(stderr, "info=%p\n", &info);
    if ( proc_pidinfo(sChildPid, PROC_PIDEXITREASONINFO, 1, &info, PROC_PIDEXITREASONINFO_SIZE) != sizeof(struct proc_exitreasoninfo) ) {
        printf("bad return size from proc_pidinfo()\n");
        return;
    }
    if ( info.eri_namespace != OS_REASON_DYLD ) {
        printf("eri_namespace != OS_REASON_DYLD\n");
        return;
    }
    if ( info.eri_code != sExpectedDyldReason ) {
        printf("eri_code != %lld\n", sExpectedDyldReason);
        return;
    }
    kcdata_iter_t iter = kcdata_iter(packReasonData, info.eri_reason_buf_size);

    if ( !kcdata_iter_valid(iter) ) {
        printf("invalid kcdata iterator from payload data\n");
        return;
    }

    if ( kcdata_iter_type(iter) != KCDATA_BUFFER_BEGIN_OS_REASON ){
        printf("first kcdata from payload data is not KCDATA_BUFFER_BEGIN_OS_REASON\n");
        return;
    }

    kcdata_iter_t payloadIter = kcdata_iter_find_type(iter, EXIT_REASON_USER_PAYLOAD);
    if ( !kcdata_iter_valid(payloadIter) ) {
        printf("invalid kcdata payload iterator from payload data\n");
        return;
    }
    const dyld_abort_payload* dyldInfo = (dyld_abort_payload*)kcdata_iter_payload(payloadIter);

    if ( dyldInfo->version != 1 ) {
        printf("dyld payload is not version 1\n");
        return;
    }

    if ( (dyldInfo->flags & 1) == 0 ) {
        printf("dyld flags should have low bit set to me process terminated at launch\n");
        return;
    }

    if ( sExpectedDylibPath != NULL ) {
        if ( dyldInfo->targetDylibPathOffset != 0 ) {
            const char* targetDylib = (char*)dyldInfo + dyldInfo->targetDylibPathOffset;
            if ( strcmp(sExpectedDylibPath, targetDylib) != 0 ) {
                printf("dylib path (%s) not what expected (%s)\n", targetDylib, sExpectedDylibPath);
                return;
            }
        }
        else {
            printf("dylib path (%s) not provided by dyld\n", sExpectedDylibPath);
            return;
        }
    }

   if ( sExpectedSymbol != NULL ) {
        if ( dyldInfo->targetDylibPathOffset != 0 ) {
            const char* missingSymbol = (char*)dyldInfo + dyldInfo->symbolOffset;
            if ( strcmp(sExpectedSymbol, missingSymbol) != 0 ) {
                printf("symbol (%s) not what expected (%s)\n", missingSymbol, sExpectedSymbol);
                return;
            }
        }
        else {
            printf("symbol (%s) not provided by dyld\n", sExpectedSymbol);
            return;
        }
    }

   sChildAbortInfoCorrect = true;
}


bool runTest(const char* prog, uint64_t dyldReason, const char* expectedDylibPath, const char* expectedSymbol)
{
    sSignalCaught = false;
    sChildAbortInfoCorrect = false;
    sExpectedDyldReason = dyldReason;
    sExpectedDylibPath = expectedDylibPath;
    sExpectedSymbol = expectedSymbol;

    // fork and exec child
	sChildPid = fork();
	if ( sChildPid < 0 )
		err(EXIT_FAILURE, "fork");
	if ( sChildPid == 0 ) {
		// child side
        char* childArgv[] = { (char*)prog, NULL };
		int result = execvp(prog, childArgv);
        err(EXIT_FAILURE, "exec(\"%s\",...)", prog);
	}
    for(int i=0; i < 10; ++i) {
        if ( sSignalCaught )
            break;
        sleep(1);
    }

    return sChildAbortInfoCorrect;
}


int main(int argc, const char* argv[])
{
    bool someTestFailed = false;
    printf("[BEGIN] dyld_abort_payload\n");

    // set up signal handler for catching child terminations
    signal(SIGCHLD, childDied);

    // test launch program with missing library
    if ( !runTest("./prog_missing_dylib.exe", DYLD_EXIT_REASON_DYLIB_MISSING, "/cant/find/me.dylib", NULL) ) {
        printf("[FAIL] dyld_abort_payload DYLD_EXIT_REASON_DYLIB_MISSING\n");
        someTestFailed = true;
    }

    // test launch program with missing symbol
    if ( !runTest("./prog_missing_symbol.exe", DYLD_EXIT_REASON_SYMBOL_MISSING, "libMissingSymbols.dylib", "_slipperySymbol") ) {
        printf("[FAIL] dyld_abort_payload DYLD_EXIT_REASON_SYMBOL_MISSING\n");
        someTestFailed = true;
    }

    if ( !someTestFailed )
        printf("[PASS] dyld_abort_payload\n");

    return 0;
}

