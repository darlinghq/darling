// mach_semaphore.c

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <mach/mach.h>

#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error(msg ":" , kr); goto out; }

#define PTHID() (unsigned long)(pthread_self())

#define SEMAPHORE_WAIT(s, n) \
    { int i; for (i = 0; i < (n); i++) { semaphore_wait((s)); } }

semaphore_t g_sem[2];

void *
start_routine(void *id)
{
    semaphore_signal(g_sem[1]);
    fprintf(stderr, "thread: %lx about to decrement semaphore count\n", id);
    semaphore_wait(g_sem[0]);
    fprintf(stderr, "thread: %lx succeeded in decrementing semaphore count\n", id);
    semaphore_signal(g_sem[1]);
    return NULL;
}

int
main(void)
{
    pthread_t     pthread1, pthread2, pthread3;
    semaphore_t*   sem = g_sem;
    kern_return_t kr;

    setbuf(stdout, NULL);

    kr = semaphore_create(mach_task_self(), &sem[0], SYNC_POLICY_FIFO, 0);
    OUT_ON_MACH_ERROR("semaphore_create", kr);

    kr = semaphore_create(mach_task_self(), &sem[1], SYNC_POLICY_FIFO, 0);
    OUT_ON_MACH_ERROR("semaphore_create", kr);

    (void)pthread_create(&pthread1, (const pthread_attr_t *)0,
                         start_routine, (void *) 0);
    printf("created thread1=%lx\n", 0);

    (void)pthread_create(&pthread2, (const pthread_attr_t *)0,
                         start_routine, (void *) 1);
    printf("created thread2=%lx\n", 1);

    (void)pthread_create(&pthread3, (const pthread_attr_t *)0,
                         start_routine, (void *) 2);
    printf("created thread3=%lx\n", 2);

    // wait until all three threads are ready
    SEMAPHORE_WAIT(sem[1], 3);

//    printf("main: about to signal thread3\n");
//    semaphore_signal_thread(sem[0], pthread_mach_thread_np(pthread3));

    // wait for thread3 to sem_signal()
//    semaphore_wait(sem[1]);

    sleep(1);
    printf("main: about to signal all threads\n");
    semaphore_signal_all(sem[0]);

    // wait for thread1 and thread2 to sem_signal()
    SEMAPHORE_WAIT(sem[1], 3);

out:
    if (sem[0])
        semaphore_destroy(mach_task_self(), sem[0]);
    if (sem[1])
        semaphore_destroy(mach_task_self(), sem[1]);

    exit(kr);
}

