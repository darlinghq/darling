// Copyright (C) 2026 Lubos Dolezel
//
// Signals and thread suspensions that arrive while threads are blocked in
// kernel calls must not disturb those calls: each call returns its own
// result once, after the signal handler, and nothing else is affected.
//
// Worker threads keep making blocking calls on shared objects: a contended
// pthread mutex and condition variable, a Mach semaphore passed back and
// forth, and Mach messages with out-of-line memory sent to a shared port and
// received with a timeout. A disturber thread keeps sending SIGUSR1 (empty
// handler) to every worker and suspending and resuming them. At the end all
// counters must add up, every message must have arrived intact, and the
// process must still be alive (a call that got the wrong reply aborts it).
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WORKERS 6
#define SECONDS 20
#define OOL_SIZE 20000

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

typedef struct {
    mach_msg_header_t header;
    mach_msg_body_t body;
    mach_msg_ool_descriptor_t data;
    unsigned sequence;
} ool_message_t;

typedef struct {
    ool_message_t m;
    mach_msg_trailer_t trailer;
} ool_receive_t;

static volatile int stop;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static unsigned long shared_counter;
static unsigned long mutex_rounds[WORKERS];
static semaphore_t ping, pong;
static unsigned long ping_rounds, pong_rounds;
static mach_port_t port;
static unsigned long sent[WORKERS], received_ok, received_bad;
static unsigned char payload[OOL_SIZE];
static pthread_t workers[WORKERS];
static mach_port_t worker_ports[WORKERS];
static volatile unsigned long signals_sent, suspensions;

static void on_signal(int sig)
{
    (void)sig;
}

static double now(void)
{
    static mach_timebase_info_data_t timebase;
    if (timebase.denom == 0)
        mach_timebase_info(&timebase);
    return (double)mach_absolute_time() * timebase.numer / timebase.denom / 1e9;
}

// contended mutex plus a condition variable that wakes the next waiter
static void mutex_round(int id)
{
    pthread_mutex_lock(&mutex);
    shared_counter++;
    mutex_rounds[id]++;
    pthread_cond_signal(&cond);
    if ((shared_counter & 3) == 0) {
        struct timespec ts = { 0, 200000 };
        pthread_cond_timedwait_relative_np(&cond, &mutex, &ts);
    }
    pthread_mutex_unlock(&mutex);
}

static void send_one(int id)
{
    ool_message_t m;
    memset(&m, 0, sizeof m);
    m.header.msgh_bits = MACH_MSGH_BITS_COMPLEX | MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_MAKE_SEND);
    m.header.msgh_size = sizeof m;
    m.header.msgh_remote_port = port;
    m.header.msgh_id = id;
    m.body.msgh_descriptor_count = 1;
    m.data.address = payload + (sent[id] % 97);
    m.data.size = OOL_SIZE - 97;
    m.data.copy = MACH_MSG_VIRTUAL_COPY;
    m.data.deallocate = FALSE;
    m.data.type = MACH_MSG_OOL_DESCRIPTOR;
    m.sequence = (unsigned)sent[id];
    if (mach_msg(&m.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof m, 0, MACH_PORT_NULL, 1, MACH_PORT_NULL) == KERN_SUCCESS)
        sent[id]++;
}

static void receive_one(void)
{
    ool_receive_t r;
    memset(&r, 0, sizeof r);
    kern_return_t kr = mach_msg(&r.m.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof r, port, 1, MACH_PORT_NULL);
    if (kr == MACH_RCV_TIMED_OUT)
        return;
    if (kr != KERN_SUCCESS) {
        printf("FAIL: receive returned 0x%x\n", kr);
        __atomic_add_fetch(&received_bad, 1, __ATOMIC_RELAXED);
        return;
    }
    if (r.m.data.size != OOL_SIZE - 97 ||
        memcmp(r.m.data.address, payload + (r.m.sequence % 97), r.m.data.size) != 0) {
        printf("FAIL: message from worker %d (%u) has wrong contents\n", r.m.header.msgh_id, r.m.sequence);
        __atomic_add_fetch(&received_bad, 1, __ATOMIC_RELAXED);
    } else {
        __atomic_add_fetch(&received_ok, 1, __ATOMIC_RELAXED);
    }
    vm_deallocate(mach_task_self(), (vm_address_t)r.m.data.address, r.m.data.size);
}

static void *worker(void *arg)
{
    int id = (int)(long)arg;
    worker_ports[id] = mach_thread_self();
    while (!stop) {
        switch (id % 3) {
        case 0:
            mutex_round(id);
            send_one(id);
            break;
        case 1:
            mutex_round(id);
            receive_one();
            break;
        case 2:
            // ping-pong between the two semaphores: workers 2 and 5 take turns
            if (id == 2) {
                if (semaphore_signal(ping) == KERN_SUCCESS)
                    __atomic_add_fetch(&ping_rounds, 1, __ATOMIC_RELAXED);
                mach_timespec_t ts = { 0, 1000000 };
                semaphore_timedwait(pong, ts);
            } else {
                mach_timespec_t ts = { 0, 1000000 };
                if (semaphore_timedwait(ping, ts) == KERN_SUCCESS) {
                    __atomic_add_fetch(&pong_rounds, 1, __ATOMIC_RELAXED);
                    semaphore_signal(pong);
                }
            }
            receive_one();
            break;
        }
    }
    return NULL;
}

static void *disturber(void *arg)
{
    (void)arg;
    unsigned n = 0;
    while (!stop) {
        int id = n % WORKERS;
        pthread_kill(workers[id], SIGUSR1);
        signals_sent++;
        if ((n % 5) == 0 && worker_ports[id] != MACH_PORT_NULL) {
            if (thread_suspend(worker_ports[id]) == KERN_SUCCESS) {
                suspensions++;
                thread_resume(worker_ports[id]);
            }
        }
        n++;
        usleep(30);
    }
    return NULL;
}

int main(void)
{
    // a hang should still show how far the test got
    setvbuf(stdout, NULL, _IOLBF, 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGUSR1, &sa, NULL);

    for (size_t i = 0; i < sizeof payload; i++)
        payload[i] = (unsigned char)(i * 7 + 3);

    CHECK(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) == KERN_SUCCESS);
    CHECK(semaphore_create(mach_task_self(), &ping, SYNC_POLICY_FIFO, 0) == KERN_SUCCESS);
    CHECK(semaphore_create(mach_task_self(), &pong, SYNC_POLICY_FIFO, 0) == KERN_SUCCESS);

    for (long i = 0; i < WORKERS; i++)
        pthread_create(&workers[i], NULL, worker, (void *)i);
    while (1) {
        int ready = 1;
        for (int i = 0; i < WORKERS; i++)
            if (worker_ports[i] == MACH_PORT_NULL)
                ready = 0;
        if (ready)
            break;
        usleep(1000);
    }

    pthread_t d;
    pthread_create(&d, NULL, disturber, NULL);

    const double end = now() + SECONDS;
    while (now() < end)
        usleep(100000);
    stop = 1;
    pthread_join(d, NULL);
    for (int i = 0; i < WORKERS; i++)
        pthread_join(workers[i], NULL);

    // drain what is still queued
    for (;;) {
        unsigned long before = received_ok + received_bad;
        receive_one();
        if (received_ok + received_bad == before)
            break;
    }

    unsigned long mutex_total = 0, sent_total = 0;
    for (int i = 0; i < WORKERS; i++) {
        mutex_total += mutex_rounds[i];
        sent_total += sent[i];
    }
    printf("signals %lu, suspensions %lu; mutex rounds %lu; semaphore %lu/%lu; messages sent %lu, received %lu, bad %lu\n",
        signals_sent, suspensions, mutex_total, ping_rounds, pong_rounds, sent_total, received_ok, received_bad);

    CHECK(shared_counter == mutex_total);
    CHECK(mutex_total > 0);
    CHECK(received_bad == 0);
    CHECK(received_ok == sent_total);
    CHECK(sent_total > 0);
    // every pong answered a ping
    CHECK(pong_rounds <= ping_rounds);

    if (failures) {
        printf("FAIL (%d)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
