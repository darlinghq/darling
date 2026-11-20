// Copyright (C) 2026 Lubos Dolezel
//
// Receiving a message that carries out-of-line memory must not fail because
// the receiving thread is interrupted by a signal (or suspended) while the
// kernel maps the out-of-line region into the receiver. mach_msg() reports
// interruptions of a receive only as MACH_RCV_INTERRUPTED (with
// MACH_RCV_INTERRUPT), never as a copyout error, and without that option it
// retries internally. A message once dequeued is never lost.
//
// One thread sends messages with an out-of-line region (virtual copy, not
// deallocated) and a port descriptor; the main thread receives them with a
// page-sized buffer, a zero timeout, an audit trailer and MACH_RCV_VOUCHER,
// polling; a third thread keeps signalling (SIGUSR1, empty handler) and
// suspending the receiver. Every message must arrive intact and in order.
//
// Phase 1 disturbs every 500 us and must deliver all messages. Phase 2 is a
// storm (every 50 us) that must still make progress: at least
// STORM_MIN_MESSAGES within STORM_SECONDS, none lost or failed. (Under
// Darling every signal costs the receiver two calls to the server, so the
// storm throughput varies a lot with the machine and its load; the minimum
// only tells progress from a receive that is restarted forever.)
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <mach/mach.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach_time.h>
#include <unistd.h>

#define MESSAGES 3000
#define STORM_SECONDS 20
#define STORM_MIN_MESSAGES 10

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

typedef struct {
    mach_msg_header_t header;
    mach_msg_body_t body;
    mach_msg_port_descriptor_t port;
    mach_msg_ool_descriptor_t data;
} ool_message_t;

static mach_port_t port;
static volatile int done;
static volatile unsigned interval_us;
static pthread_t receiver_pthread;
static mach_port_t receiver_thread = MACH_PORT_NULL;
static unsigned char *payload;
static int messages_to_send;
static const size_t sizes[] = { 5000, 65536, 300000, 1 << 20 };
#define NSIZES (sizeof sizes / sizeof *sizes)

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

static void *sender(void *arg)
{
    (void)arg;
    for (int i = 0; i < messages_to_send && !done; i++) {
        ool_message_t m;
        memset(&m, 0, sizeof m);
        m.header.msgh_bits = MACH_MSGH_BITS_COMPLEX | MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_COPY_SEND);
        m.header.msgh_size = sizeof m;
        m.header.msgh_remote_port = port;
        m.header.msgh_id = i;
        m.body.msgh_descriptor_count = 2;
        m.port.name = port;
        m.port.disposition = MACH_MSG_TYPE_MAKE_SEND;
        m.port.type = MACH_MSG_PORT_DESCRIPTOR;
        m.data.address = payload + (i % 251);
        m.data.size = (mach_msg_size_t)sizes[i % NSIZES];
        m.data.copy = MACH_MSG_VIRTUAL_COPY;
        m.data.deallocate = FALSE;
        m.data.type = MACH_MSG_OOL_DESCRIPTOR;
        // a full queue: wait and retry, as a sender with a send timeout does
        kern_return_t kr;
        while ((kr = mach_msg(&m.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof m, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL)) == MACH_SEND_TIMED_OUT && !done)
            usleep(100);
        if (kr != KERN_SUCCESS && !done) {
            printf("FAIL: send %d returned 0x%x\n", i, kr);
            failures++;
            done = 1;
        }
    }
    return NULL;
}

static void *disturber(void *arg)
{
    (void)arg;
    unsigned n = 0;
    while (!done) {
        pthread_kill(receiver_pthread, SIGUSR1);
        if ((n++ & 7) == 0) {
            if (thread_suspend(receiver_thread) == KERN_SUCCESS)
                thread_resume(receiver_thread);
        }
        usleep(interval_us);
    }
    return NULL;
}

// receives until `count` messages arrived or `seconds` passed; returns how many arrived
static int run_phase(const char *name, unsigned interval, int count, double seconds, vm_address_t buffer, vm_size_t buffer_size)
{
    done = 0;
    interval_us = interval;
    messages_to_send = count;

    pthread_t s, d;
    pthread_create(&d, NULL, disturber, NULL);
    pthread_create(&s, NULL, sender, NULL);

    const double deadline = now() + seconds;
    int received = 0;
    while (received < count && !done) {
        mach_msg_header_t *h = (mach_msg_header_t *)buffer;
        memset(h, 0, sizeof *h);
        h->msgh_size = (mach_msg_size_t)buffer_size;
        h->msgh_local_port = port;
        const mach_msg_option_t options = MACH_RCV_MSG | MACH_RCV_TIMEOUT |
            MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) |
            MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT) | MACH_RCV_VOUCHER;
        kern_return_t kr = mach_msg(h, options, 0, h->msgh_size, port, 0, MACH_PORT_NULL);
        if (kr == MACH_RCV_TIMED_OUT) {
            if (now() > deadline)
                break;
            continue;
        }
        if (kr != KERN_SUCCESS) {
            printf("FAIL: %s: receive of message %d returned 0x%x\n", name, received, kr);
            failures++;
            break;
        }
        ool_message_t *m = (ool_message_t *)h;
        int i = m->header.msgh_id;
        if (i != received) {
            printf("FAIL: %s: got message %d, expected %d (a message was lost)\n", name, i, received);
            failures++;
            break;
        }
        CHECK(m->body.msgh_descriptor_count == 2);
        CHECK(m->data.size == sizes[i % NSIZES]);
        if (m->data.address == NULL || memcmp(m->data.address, payload + (i % 251), m->data.size) != 0) {
            printf("FAIL: %s: message %d has wrong out-of-line contents\n", name, i);
            failures++;
        }
        mach_msg_audit_trailer_t *t = (mach_msg_audit_trailer_t *)((char *)h + round_msg(h->msgh_size));
        CHECK(t->msgh_trailer_size == sizeof(mach_msg_audit_trailer_t));
        vm_deallocate(mach_task_self(), (vm_address_t)m->data.address, m->data.size);
        mach_port_deallocate(mach_task_self(), m->port.name);
        if (h->msgh_voucher_port != MACH_PORT_NULL)
            mach_port_deallocate(mach_task_self(), h->msgh_voucher_port);
        received++;
        if (now() > deadline)
            break;
    }

    done = 1;
    pthread_join(s, NULL);
    pthread_join(d, NULL);

    // drop what the sender queued after we stopped
    for (;;) {
        mach_msg_header_t *h = (mach_msg_header_t *)buffer;
        memset(h, 0, sizeof *h);
        if (mach_msg(h, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, (mach_msg_size_t)buffer_size, port, 0, MACH_PORT_NULL) != KERN_SUCCESS)
            break;
        mach_msg_destroy(h);
    }

    printf("%s: received %d/%d\n", name, received, count);
    return received;
}

int main(void)
{
    // a hang should still show how far the test got
    setvbuf(stdout, NULL, _IOLBF, 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigaction(SIGUSR1, &sa, NULL);

    payload = malloc((1 << 20) + 256);
    for (size_t i = 0; i < (1 << 20) + 256; i++)
        payload[i] = (unsigned char)(i * 13 + 5);

    kern_return_t kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    CHECK(kr == KERN_SUCCESS);
    kr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
    CHECK(kr == KERN_SUCCESS);

    vm_address_t buffer = 0;
    const vm_size_t buffer_size = getpagesize();
    kr = vm_allocate(mach_task_self(), &buffer, buffer_size, VM_FLAGS_ANYWHERE);
    CHECK(kr == KERN_SUCCESS);

    receiver_pthread = pthread_self();
    receiver_thread = mach_thread_self();

    int got = run_phase("disturbed", 500, MESSAGES, 120, buffer, buffer_size);
    if (got != MESSAGES && !failures) {
        printf("FAIL: disturbed: only %d of %d messages in 120 s\n", got, MESSAGES);
        failures++;
    }

    if (!failures) {
        got = run_phase("storm", 50, MESSAGES, STORM_SECONDS, buffer, buffer_size);
        if (got < STORM_MIN_MESSAGES && !failures) {
            printf("FAIL: storm: only %d messages in %d s (need %d)\n", got, STORM_SECONDS, STORM_MIN_MESSAGES);
            failures++;
        }
    }

    if (failures) {
        printf("FAIL (%d)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
