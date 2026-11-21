/*
 Copyright (C) 2026 Lubos Dolezel

 Licensed under the MIT license, the same as the rest of this project.
*/
// Cost of creating a thread in a process that already has many threads (a browser process has hundreds).
// Starts IDLE threads that wait, then creates and joins SHORT threads one after another and prints the
// time per creation for several idle-thread counts. The cost of a creation should not grow with the number
// of threads the process already has.
// Usage: thread_create_cost [short-threads-per-step]
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int release;

static void *idle(void *arg) {
    pthread_mutex_lock(&mu);
    while (!release)
        pthread_cond_wait(&cv, &mu);
    pthread_mutex_unlock(&mu);
    return NULL;
}
static void *shortThread(void *arg) { return NULL; }

static double now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}

int main(int argc, char **argv) {
    int n = argc > 1 ? atoi(argv[1]) : 100;
    int counts[] = {0, 50, 100, 200, 400};
    int have = 0;
    double first = 0, last = 0;
    for (unsigned s = 0; s < sizeof counts / sizeof *counts; s++) {
        for (; have < counts[s]; have++) {
            pthread_t t;
            if (pthread_create(&t, NULL, idle, NULL) != 0) { printf("FAIL: cannot create thread %d\n", have); return 1; }
            pthread_detach(t);
        }
        double t0 = now();
        for (int i = 0; i < n; i++) {
            pthread_t t;
            pthread_create(&t, NULL, shortThread, NULL);
            pthread_join(t, NULL);
        }
        double per = (now() - t0) / n * 1000;
        printf("%3d idle threads: %.2f ms per create+join\n", have, per);
        fflush(stdout);
        if (s == 0) first = per;
        last = per;
    }
    pthread_mutex_lock(&mu); release = 1; pthread_cond_broadcast(&cv); pthread_mutex_unlock(&mu);
    // tolerance: 400 idle threads may cost up to 10x of 0 idle threads, plus 2 ms
    if (last > first * 10 + 2) { printf("FAIL: creation cost grows with the thread count (%.2f ms vs %.2f ms)\n", last, first); return 1; }
    printf("PASS\n");
    return 0;
}
