
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

#if __arm64e__
    // arm64e uses chained binds which does not support lazy binding
    #define SUPPORTS_LAZY_BINDING 0
#else
    #define SUPPORTS_LAZY_BINDING 1
#endif


static bool        sSignalCaught = false;
static bool        sChildAbortInDyld = false;
static pid_t       sChildPid = 0;


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
    int procResult = proc_pidinfo(sChildPid, PROC_PIDEXITREASONINFO, 1, &info, PROC_PIDEXITREASONINFO_SIZE);
    if ( procResult != sizeof(struct proc_exitreasoninfo) ) {
        printf("bad return size from proc_pidinfo(), %d expected %lu\n", procResult, PROC_PIDEXITREASONINFO_SIZE);
        return;
    }
    if ( info.eri_namespace != OS_REASON_DYLD ) {
        printf("eri_namespace (%d) != OS_REASON_DYLD\n", info.eri_namespace);
        return;
    }

   sChildAbortInDyld = true;
}


bool runTest(const char* prog)
{
    sSignalCaught = false;
    sChildAbortInDyld = false;

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

    return sChildAbortInDyld;
}


int main(int argc, const char* argv[])
{
    printf("[BEGIN] lazy-symbol-missing and called\n");

#if SUPPORTS_LAZY_BINDING
    // set up signal handler for catching child terminations
    signal(SIGCHLD, childDied);

    // test launch program with missing library
    runTest(RUN_DIR "/lazy-symbol-missing-called.exe");

    if ( sSignalCaught && sChildAbortInDyld )
        printf("[PASS] lazy-symbol-missing and called\n");
    else
        printf("[FAIL] lazy-symbol-missing and called\n");
#else
    printf("[PASS] lazy-symbol-missing and called\n");
#endif
    return 0;
}

