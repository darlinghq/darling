// Copyright (C) 2026 Lubos Dolezel
//
// A process that execs another Darling image over and over, creating a thread in every image, keeps
// working: the first thread each new image creates must get its replies from the server, however the
// server orders the exec's completion against the new image's main-thread check-in. A thread whose
// replies never come stops the chain; the watchdog reports it.
// Usage: exec_thread_checkin [rounds]. Builds for x86_64 and i386 and runs on macOS.
#include <mach/mach.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void* worker(void* arg)
{
    // a call the server has to answer on this thread
    mach_port_t port = mach_thread_self();
    mach_port_deallocate(mach_task_self(), port);
    mach_port_t sem_port;
    if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &sem_port) == KERN_SUCCESS)
        mach_port_destroy(mach_task_self(), sem_port);
    return arg;
}

static void watchdog(int sig)
{
    (void)sig;
    static const char msg[] = "FAIL: a thread of an exec'd image got no reply from the server\n";
    write(1, msg, sizeof(msg) - 1);
    _exit(1);
}

int main(int argc, char** argv)
{
    int rounds = argc > 1 ? atoi(argv[1]) : 50;
    signal(SIGALRM, watchdog);
    alarm(20);

    // a series of threads, one after another: whichever the server's exec bookkeeping wrongly takes for the
    // exec's own check-in loses its replies
    for (int i = 0; i < 15; i++) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, worker, NULL) != 0) {
            printf("FAIL: pthread_create\n");
            return 1;
        }
        pthread_join(thread, NULL);
        usleep(20000);
    }

    if (rounds > 0) {
        char count[16];
        snprintf(count, sizeof(count), "%d", rounds - 1);
        execl(argv[0], argv[0], count, (char*)NULL);
        printf("FAIL: execl\n");
        return 1;
    }
    printf("PASS\n");
    return 0;
}
