#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if !defined(_WIN32)
# include <sys/wait.h>
# if !defined(NO_CONFIG_H)
#  include "config.h"
# endif
# include <semaphore.h>
#else
# define inline _inline
# include "../../src/windows/platform.h"
# include "posix_semaphore.h"
#endif
#include "../../src/private.h"

#if HAVE_ERR_H
# include <err.h>
#else
# define err(rc,msg,...) do { perror(msg); exit(rc); } while (0)
# define errx(rc,msg,...) do { puts(msg); exit(rc); } while (0)
#endif

#include <pthread_workqueue.h>

static int work_cnt;

/* If non-zero, extra debugging statements will be printed */
static int dbg = 0;

static sem_t test_complete;
static int test_rounds;
#undef dbg_puts
#define dbg_puts(s) if (dbg) puts(s)
#undef dbg_printf
#define dbg_printf(fmt,...) if (dbg) fprintf(stderr, fmt, __VA_ARGS__)

void additem(pthread_workqueue_t wq, void (*func)(void *), 
             void * arg)
{

    int rv;
    
    rv = pthread_workqueue_additem_np(wq, *func, arg, NULL, NULL);
    if (rv != 0)
        errx(1, "unable to add item: %s", strerror(rv));
    dbg_puts("added item\n");
}

void 
mark_progress(void)
{
    static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mtx);
    test_rounds--;
    pthread_mutex_unlock(&mtx);

    dbg_printf("rounds = %d\n", test_rounds);
    if (test_rounds == 0) {
        sem_post(&test_complete);
    }
}

void
sem_up(void *arg)
{
    dbg_puts("semaphore UP\n");
    sem_post((sem_t *) arg);
    mark_progress();
}

void
sem_down(void *arg)
{
    dbg_puts("semaphore DOWN\n");
    sem_wait((sem_t *) arg);
    dbg_puts("semaphore UP\n");
    sem_post((sem_t *) arg);
    mark_progress();
}

void
compute(void *arg)
{
    int *count = (int *) arg;
#define nval 5000
    int val[nval];
    int i,j;

    /* Do some useless computation */
    for (i = 0; i < nval; i++) {
        val[i] = INT_MAX;
    }
    for (j = 0; j < nval; j++) {
        for (i = 0; i < nval; i++) {
            val[i] /= 3;
            val[i] *= 2;
            val[i] /= 4;
            val[i] *= 5;
        }
    }

    if (count != NULL) 
        (*count)--;
}


void
sleepy(void *msg)
{
    printf("%s\n", (char *) msg);
    if (strcmp(msg, "done") == 0)
        exit(0);
    sleep(random() % 6);
}

void
lazy(void *arg)
{
    sleep(3);
    dbg_printf("item %lu complete\n", (unsigned long) arg);
	work_cnt--;
}

void
run_blocking_test(pthread_workqueue_t wq, int rounds)
{
	long i = 0;
	work_cnt = rounds;
    for (i = 0; i < rounds; i++) {
        additem(wq, lazy, (void *) i);
    }
	while (work_cnt > 0)
		sleep(1);
}

void
run_cond_wait_test(pthread_workqueue_t wq)
{
	const int rounds = 10;
	long i = 0;

	sleep(3);	/* Allow time for the workers to enter pthread_cond_wait() */
	work_cnt = rounds;
    for (i = 0; i < rounds; i++) {
        additem(wq, lazy, (void *) i);
		sleep(1);
    }
	while (work_cnt > 0)
		sleep(1);
}

void
run_load_test(pthread_workqueue_t wq)
{
    char buf[16];
	int i = 0;
    for (i = 0; i < 1024; i++) {
        sprintf(buf, "%d", i);
        additem(wq, sleepy, strdup(buf));
        additem(wq, compute, NULL);
    }
    additem(wq, sleepy, "done");
}

/* Try to overwhelm the CPU with computation requests */
void
run_stress_test(pthread_workqueue_t wq, int rounds)
{
	int i = 0;
	work_cnt = rounds;
    for (i = 0; i < rounds; i++) {
        additem(wq, compute, &work_cnt);
    }
	while (work_cnt > 0)
		sleep(1);
}

/*
 * Ensure that the library is reinitialized after fork(2) is called.
 */
void
run_fork_test(pthread_workqueue_t wq)
{
#if !defined(_WIN32)
    pid_t pid;
    int   rv, status, timeout;

    puts("fork test... ");
    pid = fork();
    if (pid < 0) 
        err(1, "fork");

    if (pid == 0) {
        /* Child */
        wq = NULL;
        rv = pthread_workqueue_create_np(&wq, NULL);
        if (rv < 0)
            errx(1, "pthread_workqueue_create_np");
        work_cnt = 1;
        timeout = 5;
        additem(wq, compute, &work_cnt);
        while (work_cnt > 0) {
            sleep(1);
            if (--timeout == 0)
                errx(1, "work was not completed");
        }
        exit(0);
    } else {
        /* Parent */
        if (wait(&status) != pid) 
            err(1, "waitpid");
        if (WEXITSTATUS(status) != 0) 
            errx(1, "fork test failed");
        puts("ok\n");
    }
#else
    puts("fork test... N/A\n");
#endif
}

void
run_overcommit_test(pthread_workqueue_t wq)
{
    sem_t sem;
    pthread_workqueue_t ocwq;
    pthread_workqueue_attr_t attr;
    int i, rv;

    (void)wq;
    sem_init(&sem, 0, 0);

    printf("pthread_workqueue_create_np() - overcommit enabled ");
    pthread_workqueue_attr_init_np(&attr);
    pthread_workqueue_attr_setovercommit_np(&attr, 1);
    rv = pthread_workqueue_create_np(&ocwq, &attr);
    if (rv != 0)
        err(1, "failed");

    puts("ok\n");

    printf("stress test - overcommit enabled ");
    run_stress_test(ocwq, 25);
    puts("ok\n");

    /* FIXME: should use a multiple of the number of CPUs instead of magic number */
    printf("deadlock test - overcommit enabled ");
    test_rounds = 41;
    for (i = 0; i < 40; i++) {
        additem(ocwq, sem_down, &sem);
    }
    additem(ocwq, sem_up, &sem);
    sem_wait(&test_complete);
    puts("ok\n");
}

int main() {
    pthread_workqueue_t wq;
    int rv;

#ifdef MAKE_STATIC
	pthread_workqueue_init_np();
#endif

    sem_init(&test_complete, 0, 0);

    run_overcommit_test(NULL);

    printf("pthread_workqueue_create_np().. ");
    rv = pthread_workqueue_create_np(&wq, NULL);
    if (rv != 0)
        err(1, "failed");
    printf("ok\n");

    printf("stress test.. ");
    run_stress_test(wq, 25);
    printf("ok\n");

    run_fork_test(wq);

    //run_deadlock_test();
//    run_cond_wait_test();
//    run_blocking_test();
    //run_load_test();


	puts("All tests completed.\n");
    exit(0);
}
