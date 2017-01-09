#include "execserverServer.h"

#include <mach/mach.h>
#include <mach/vm_map.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <pthread.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include <errno.h>
#include <signal.h>
#include <libproc.h>
#include <System/sys/reason.h>
#include <System/sys/proc_info.h>



static pid_t                sChildPid;
static dispatch_semaphore_t sServerRunning;
static bool                 sChildCrashed = false;
static bool                 sChildTerminatedByDyld = false;

/*
 * setup exception handling port for EXC_CRASH and EXC_CORPSE_NOTIFY.
 * runs mach_msg_server once for receiving exception messages from kernel.
 */
static void* serverCode(void* arg)
{
	mach_port_t exception_port;

	kern_return_t kret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &exception_port);
	if (kret != KERN_SUCCESS)
		errx(1, "mach_port_allocate: %s (%d)", mach_error_string(kret), kret);

	kret = mach_port_insert_right(mach_task_self(), exception_port, exception_port, MACH_MSG_TYPE_MAKE_SEND);
	if (kret != KERN_SUCCESS)
		errx(1, "mach_port_insert_right: %s (%d)", mach_error_string(kret), kret);

	kret = task_set_exception_ports(mach_task_self(), EXC_MASK_CRASH | EXC_MASK_CORPSE_NOTIFY, exception_port,
	                                EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES, 0);
	if (kret != KERN_SUCCESS)
		errx(1, "task_set_exception_ports: %s (%d)", mach_error_string(kret), kret);

	dispatch_semaphore_signal(sServerRunning);

	kret = mach_msg_server(mach_exc_server, MACH_MSG_SIZE_RELIABLE, exception_port, 0);
	if (kret != KERN_SUCCESS)
		errx(1, "mach_msg_server: %s (%d)", mach_error_string(kret), kret);

	return NULL;
}


static void childDied(int sig)
{
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
    sChildTerminatedByDyld = (info.eri_namespace == OS_REASON_DYLD);
 }


int main(int argc, const char* argv[])
{
	if ( argc < 2 ) {
		fprintf(stderr, "usage: nocr [-require_crash] prog args...\n");
		return EXIT_FAILURE;
	}
    unsigned progArgIndex = 1;
    bool requireCrash = false;
    const char* testName = NULL;
    if ( strcmp(argv[1], "-require_crash") == 0 ) {
        progArgIndex = 2;
        requireCrash = true;
        testName = getenv("NOCR_TEST_NAME");
        if ( testName )
            printf("[BEGIN] %s\n", testName);
    }

    signal(SIGCHLD, childDied);

	sServerRunning = dispatch_semaphore_create(0);

    // start up thread for mach server which handles mach exception ports
	pthread_t serverThread;
	int result = pthread_create(&serverThread, NULL, serverCode, NULL);
    if ( result )
        err(EXIT_FAILURE, "pthread_create");

    // wait until server is up before starting child
	dispatch_semaphore_wait(sServerRunning, DISPATCH_TIME_FOREVER);

    // fork and exec child
	sChildPid = fork();
	if ( sChildPid < 0 )
		err(EXIT_FAILURE, "fork");
	if ( sChildPid == 0 ) {
		// child side
		result = execvp(argv[progArgIndex], (char**)&argv[progArgIndex]);
        err(EXIT_FAILURE, "exec(\"%s\",...)", argv[progArgIndex]);
	}

    // wait for child to finish (including crash)
    int status;
	int waitResult;
	int childResult = EXIT_FAILURE;
	do {
        waitResult = waitpid(sChildPid, &status, 0);
    } while ( (waitResult == -1) && (errno == EINTR) );
    if ( waitResult != -1 ) {
        if ( WIFEXITED(status) ) {
			childResult = WEXITSTATUS(status);
        }
    }

    if ( requireCrash ) {
        if ( testName ) {
            if ( sChildCrashed || sChildTerminatedByDyld )
                printf("[PASS] %s\n", testName);
            else
                printf("[FAIL] %s\n", testName);
        }
        return sChildCrashed ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else
        return childResult;
}




// Mach exception handler routines needed by execserverServer.c

kern_return_t
catch_mach_exception_raise(mach_port_t exception_port,
                           mach_port_t thread,
                           mach_port_t task,
                           exception_type_t exception,
                           mach_exception_data_t code,
                           mach_msg_type_number_t codeCnt)
{
	//fprintf(stderr, "child crashed\n");
    sChildCrashed = true;
    return KERN_SUCCESS;
}

kern_return_t
catch_mach_exception_raise_state(mach_port_t exception_port,
                                 exception_type_t exception,
                                 const mach_exception_data_t code,
                                 mach_msg_type_number_t codeCnt,
                                 int * flavor,
                                 const thread_state_t old_state,
                                 mach_msg_type_number_t old_stateCnt,
                                 thread_state_t new_state,
                                 mach_msg_type_number_t * new_stateCnt)
{
	errx(1, "Unsupported catch_mach_exception_raise_state");
	return KERN_NOT_SUPPORTED;
}

kern_return_t
catch_mach_exception_raise_state_identity(mach_port_t exception_port,
                                          mach_port_t thread,
                                          mach_port_t task,
                                          exception_type_t exception,
                                          mach_exception_data_t code,
                                          mach_msg_type_number_t codeCnt,
                                          int * flavor,
                                          thread_state_t old_state,
                                          mach_msg_type_number_t old_stateCnt,
                                          thread_state_t new_state,
                                          mach_msg_type_number_t * new_stateCnt)
{
	errx(1, "Unsupported catch_mach_exception_raise_state_identity");
	return KERN_NOT_SUPPORTED;
}



