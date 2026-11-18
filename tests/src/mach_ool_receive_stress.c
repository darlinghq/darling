// Copyright (C) 2026 Lubos Dolezel
//
// Receiving a Mach message that carries an out-of-line descriptor needs the
// receiver's address space to be extended while the receive is in progress.
// That work must fail cleanly, not damage the receiver or anything else, when
// the receive is interrupted by a signal or when the receiving process dies.
//
// Part 1: one thread keeps receiving 8000-byte out-of-line messages while
// another thread bombards it with signals (handler installed without
// SA_RESTART) and a third thread sends. Every message that is received must
// carry the exact payload that was sent, and no mapping may be handed out
// twice.
// Part 2: a child task registers a service and receives out-of-line messages
// while this task floods it and then kills it with SIGKILL, repeatedly.
// Afterwards Mach IPC, memory allocation and process creation must still work.
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

#define PAYLOAD_SIZE 8000

typedef struct {
    mach_msg_header_t header;
    mach_msg_body_t body;
    mach_msg_ool_descriptor_t data;
    uint32_t seq;
} ool_send_t;

typedef struct {
    ool_send_t message;
    mach_msg_trailer_t trailer;
} ool_receive_t;

static void fill_payload(char *buf, uint32_t seq)
{
    for (size_t i = 0; i < PAYLOAD_SIZE; i++) buf[i] = (char)(i * 13 + seq);
}

static kern_return_t send_ool(mach_port_t port, uint32_t seq, mach_msg_timeout_t timeout)
{
    static __thread char payload[PAYLOAD_SIZE];
    fill_payload(payload, seq);

    ool_send_t out;
    memset(&out, 0, sizeof out);
    out.header.msgh_bits = MACH_MSGH_BITS_COMPLEX | MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    out.header.msgh_size = sizeof out;
    out.header.msgh_remote_port = port;
    out.header.msgh_id = 5150;
    out.body.msgh_descriptor_count = 1;
    out.data.address = payload;
    out.data.size = PAYLOAD_SIZE;
    out.data.deallocate = FALSE;
    out.data.copy = MACH_MSG_VIRTUAL_COPY;
    out.data.type = MACH_MSG_OOL_DESCRIPTOR;
    out.seq = seq;
    return mach_msg(&out.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof out, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
}

// Receives one message; returns the mach_msg result. A received payload is verified and released.
static kern_return_t receive_ool(mach_port_t port, mach_msg_timeout_t timeout, int *verified)
{
    ool_receive_t in;
    memset(&in, 0, sizeof in);
    kern_return_t kr = mach_msg(&in.message.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof in, port, timeout, MACH_PORT_NULL);
    if (kr != KERN_SUCCESS) return kr;
    if (in.message.header.msgh_id == 5150 && (in.message.header.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
        char expected[PAYLOAD_SIZE];
        fill_payload(expected, in.message.seq);
        if (in.message.data.size == PAYLOAD_SIZE && in.message.data.address && memcmp(in.message.data.address, expected, PAYLOAD_SIZE) == 0) {
            if (verified) (*verified)++;
        } else if (verified) {
            *verified = -1000000;
        }
        vm_deallocate(mach_task_self(), (vm_address_t)in.message.data.address, in.message.data.size);
    }
    return KERN_SUCCESS;
}

static void noop_handler(int sig)
{
    (void)sig;
}

static pthread_t receiver_thread;
static mach_port_t storm_port;
static volatile int storm_done;
static volatile int storm_verified;
static volatile int storm_bad;
static volatile int storm_bad_result;

static void *storm_receiver(void *arg)
{
    (void)arg;
    int verified = 0;
    while (!storm_done) {
        kern_return_t kr = receive_ool(storm_port, 50, &verified);
        if (kr != KERN_SUCCESS && kr != MACH_RCV_TIMED_OUT && kr != MACH_RCV_INTERRUPTED) {
            // any failure other than an interruption or a timeout must be a documented receive error
            kern_return_t base = kr & ~MACH_MSG_MASK;
            if (base != MACH_RCV_INVALID_DATA && base != MACH_RCV_TOO_LARGE && base != MACH_RCV_BODY_ERROR) {
                storm_bad++;
                storm_bad_result = kr;
            }
        }
        if (verified < 0) {
            storm_bad++;
            storm_bad_result = -1;
            verified = 0;
        }
    }
    storm_verified = verified;
    return NULL;
}

static void *storm_signaller(void *arg)
{
    (void)arg;
    while (!storm_done) {
        pthread_kill(receiver_thread, SIGUSR1);
        usleep(50);
    }
    return NULL;
}

static void *storm_sender(void *arg)
{
    (void)arg;
    uint32_t seq = 0;
    while (!storm_done) {
        send_ool(storm_port, seq++, 50);
    }
    return NULL;
}

static void part1_signal_storm(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = noop_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    CHECK(sigaction(SIGUSR1, &sa, NULL) == 0);

    CHECK(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &storm_port) == KERN_SUCCESS);
    CHECK(mach_port_insert_right(mach_task_self(), storm_port, storm_port, MACH_MSG_TYPE_MAKE_SEND) == KERN_SUCCESS);
    mach_port_limits_t limits = { .mpl_qlimit = 16 };
    mach_port_set_attributes(mach_task_self(), storm_port, MACH_PORT_LIMITS_INFO, (mach_port_info_t)&limits, MACH_PORT_LIMITS_INFO_COUNT);

    pthread_t signaller, sender;
    CHECK(pthread_create(&receiver_thread, NULL, storm_receiver, NULL) == 0);
    CHECK(pthread_create(&sender, NULL, storm_sender, NULL) == 0);
    CHECK(pthread_create(&signaller, NULL, storm_signaller, NULL) == 0);

    sleep(8);
    storm_done = 1;
    pthread_join(signaller, NULL);
    pthread_join(sender, NULL);
    pthread_join(receiver_thread, NULL);

    printf("part 1: %d messages received and verified, %d bad results (last: 0x%x)\n", storm_verified, storm_bad, storm_bad_result);
    CHECK(storm_bad == 0);
    CHECK(storm_verified >= 100); // a receiver that wedges after an interrupted receive gets nowhere near this
}

static void child_main(const char *service)
{
    mach_port_t port = MACH_PORT_NULL;
    if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS) _exit(2);
    if (mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) _exit(2);
    mach_port_limits_t limits = { .mpl_qlimit = 16 };
    mach_port_set_attributes(mach_task_self(), port, MACH_PORT_LIMITS_INFO, (mach_port_info_t)&limits, MACH_PORT_LIMITS_INFO_COUNT);
    mach_port_t bootstrap = MACH_PORT_NULL;
    task_get_bootstrap_port(mach_task_self(), &bootstrap);
    if (bootstrap_register(bootstrap, (char *)service, port) != KERN_SUCCESS) _exit(3);
    for (;;) {
        receive_ool(port, 1000, NULL);
    }
}

static void part2_kill_loop(const char *self)
{
    mach_port_t bootstrap = MACH_PORT_NULL;
    CHECK(task_get_bootstrap_port(mach_task_self(), &bootstrap) == KERN_SUCCESS);

    for (int round = 0; round < 40; round++) {
        char service[64];
        snprintf(service, sizeof service, "org.darlinghq.test.oolstress.%d.%d", getpid(), round);

        pid_t child = 0;
        char *argv[] = { (char *)self, (char *)"child", service, NULL };
        if (posix_spawn(&child, self, NULL, NULL, argv, environ) != 0) {
            CHECK(!"posix_spawn failed");
            return;
        }

        mach_port_t port = MACH_PORT_NULL;
        kern_return_t kr = KERN_FAILURE;
        for (int attempt = 0; attempt < 200 && kr != KERN_SUCCESS; attempt++) {
            kr = bootstrap_look_up(bootstrap, service, &port);
            if (kr != KERN_SUCCESS) usleep(20000);
        }
        if (kr != KERN_SUCCESS) {
            // child could not register; do not count the round
            kill(child, SIGKILL);
            waitpid(child, NULL, 0);
            continue;
        }

        int count = 20 + (round * 37) % 200;
        for (int i = 0; i < count; i++) send_ool(port, (uint32_t)i, 20);
        // let the child get into the middle of receiving, then kill it
        usleep((useconds_t)(round % 7) * 300);
        kill(child, SIGKILL);
        for (int i = 0; i < 10; i++) send_ool(port, (uint32_t)i, 5);

        int status = 0;
        CHECK(waitpid(child, &status, 0) == child);
        mach_port_deallocate(mach_task_self(), port);
    }
    printf("part 2: 40 receivers killed\n");
}

static void check_system_alive(const char *self)
{
    // memory allocation, a Mach round trip with an out-of-line payload and process creation must all still work
    vm_address_t address = 0;
    CHECK(vm_allocate(mach_task_self(), &address, 3 * 4096, VM_FLAGS_ANYWHERE) == KERN_SUCCESS);
    CHECK(vm_deallocate(mach_task_self(), address, 3 * 4096) == KERN_SUCCESS);

    mach_port_t port = MACH_PORT_NULL;
    CHECK(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) == KERN_SUCCESS);
    CHECK(mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND) == KERN_SUCCESS);
    int verified = 0;
    CHECK(send_ool(port, 7, 1000) == KERN_SUCCESS);
    CHECK(receive_ool(port, 1000, &verified) == KERN_SUCCESS);
    CHECK(verified == 1);

    pid_t child = 0;
    char *argv[] = { (char *)"/usr/bin/true", NULL };
    CHECK(posix_spawn(&child, "/usr/bin/true", NULL, NULL, argv, environ) == 0);
    int status = 0;
    CHECK(waitpid(child, &status, 0) == child);
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    (void)self;
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "child") == 0) {
        child_main(argv[2]);
        return 0;
    }

    alarm(50);
    part1_signal_storm();
    check_system_alive(argv[0]);
    part2_kill_loop(argv[0]);
    check_system_alive(argv[0]);

    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
