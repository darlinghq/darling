// Copyright (C) 2026 Lubos Dolezel
//
// A signal handler, and a thread_suspend()/thread_resume() pair, must leave the
// interrupted thread's floating-point state as it was: the x87 register stack
// and its tag word, the x87 control word (rounding mode), MXCSR and the XMM
// registers. On i386, doubles are returned from functions on the x87 stack, so
// a corrupted tag word turns every later x87 load into a NaN.
//
// The main thread repeatedly calls functions returning double and long double
// and checks the results, while another thread signals it (SIGUSR1, a handler
// that uses floating point itself) and suspends and resumes it.
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <fenv.h>
#include <mach/mach.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ITERATIONS 200000

static pthread_t main_pthread;
static mach_port_t main_thread;
static volatile int stop;
static volatile int mode;
static volatile double handler_sink;

static void on_signal(int sig)
{
    (void)sig;
    // the handler's own floating-point work must not leak into the interrupted code
    volatile long double x = 7.25L;
    volatile double y = 1.0 / 3.0;
    handler_sink = (double)(x * 3.0L) + y;
}

static void *disturber(void *arg)
{
    (void)arg;
    unsigned n = 0;
    while (!stop) {
        if (mode & 1)
            pthread_kill(main_pthread, SIGUSR1);
        if ((mode & 2) && (n++ & 3) == 0) {
            if (thread_suspend(main_thread) == KERN_SUCCESS)
                thread_resume(main_thread);
        }
        usleep(50);
    }
    return NULL;
}

__attribute__((noinline)) static double twice(double x)
{
    return x * 2.0;
}

__attribute__((noinline)) static long double third(long double x)
{
    return x / 3.0L;
}

// returns the number of iterations that saw a wrong value
__attribute__((noinline)) static long run(double base, mach_port_t port)
{
    const long double expected_third = (long double)base / 3.0L;
    long bad = 0;
    for (long i = 0; i < ITERATIONS && !(bad > 0 && i > 1000); i++) {
        double a = base;
        long double b = base;
        // a system call, which is where the signals tend to arrive
        mach_msg_header_t h;
        memset(&h, 0, sizeof h);
        mach_msg(&h, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof h, port, 0, MACH_PORT_NULL);
        if (twice(a) != base * 2.0 || third(b) != expected_third)
            bad++;
        // 1/3 rounds differently upwards and downwards
        if (fegetround() != FE_UPWARD)
            bad++;
    }
    return bad;
}

int main(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGUSR1, &sa, NULL);

    main_pthread = pthread_self();
    main_thread = mach_thread_self();

    mach_port_t port;
    if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS) {
        printf("FAIL: mach_port_allocate\n");
        return 1;
    }

    static const char *names[] = { "undisturbed", "signals", "suspend/resume", "both" };
    volatile double base = 123456.789;
    int failures = 0;
    for (int m = 0; m <= 3; m++) {
        mode = m;
        stop = 0;
        fesetround(FE_UPWARD);
        pthread_t d;
        pthread_create(&d, NULL, disturber, NULL);
        long bad = run(base, port);
        stop = 1;
        pthread_join(d, NULL);
        fesetround(FE_TONEAREST);
        printf("%s: %ld wrong results\n", names[m], bad);
        if (bad)
            failures++;
    }

    if (failures) {
        printf("FAIL (%d)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
