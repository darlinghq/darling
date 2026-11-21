// Copyright (C) 2026 Lubos Dolezel
//
// A Mach RPC (mach_msg with MACH_SEND_MSG | MACH_RCV_MSG, as MIG client stubs
// do) must get its reply while the caller is being signalled, and while the
// server inspects the caller the way a crash reporter does: it suspends the
// calling thread or task, reads thread states and memory of the caller and
// lists its threads and regions before replying.
//
// A signal that interrupts the receive half makes mach_msg() receive again
// (MACH_RCV_INTERRUPT is not set); the request must be delivered exactly once
// and the reply must reach the caller. The first call is made from a
// dispatch_once block. Meanwhile other threads of the caller map memory
// entries (calls that may have to be restarted after a signal) and spawn
// short-lived children (SIGCHLD).
//
// Phase 1: the server is a thread of the calling task (thread_suspend).
// Phase 2: the server is a child process (task_suspend), found through the
// bootstrap server.
// Builds for x86_64 and i386 (-target i386-apple-macos10.13). A call that hangs
// is reported after 15 seconds.
#include <dispatch/dispatch.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <pthread.h>
#include <servers/bootstrap.h>
#include <signal.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

extern char **environ;
extern mach_port_t mig_get_reply_port(void);

// printf() to the terminal can fail with EINTR while the signals arrive; write everything out
static void say(const char *format, ...)
{
    char text[512];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    if (length > (int)sizeof(text) - 1) {
        length = sizeof(text) - 1;
    }
    for (int done = 0; done < length;) {
        ssize_t n = write(STDOUT_FILENO, text + done, length - done);
        if (n > 0) {
            done += n;
        } else if (n < 0 && errno != EINTR && errno != EAGAIN) {
            break;
        }
    }
}

#define REQUEST_ID 0x52490
#define BUFFER_SIZE 0x3000
#define PHASE_SECONDS 6
#define HANG_SECONDS 15

typedef struct {
    mach_msg_header_t head;
    mach_msg_body_t body;
    mach_msg_port_descriptor_t thread;
    mach_msg_port_descriptor_t task;
    NDR_record_t ndr;
    uint64_t buffer;
    uint32_t length;
    uint32_t seq;
} request_t;

typedef struct {
    mach_msg_header_t head;
    NDR_record_t ndr;
    kern_return_t ret;
    uint32_t seq;
    uint32_t sum;
    uint32_t threads;
} reply_t;

typedef struct {
    request_t req;
    mach_msg_max_trailer_t trailer;
} request_buffer_t;

typedef struct {
    reply_t rep;
    mach_msg_max_trailer_t trailer;
} reply_buffer_t;

static uint32_t checksum(const unsigned char *p, size_t len)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum = sum * 31 + p[i];
    }
    return sum;
}

// --- server side ---

// what the server (thread) is doing, reported when a call hangs
static const char *volatile server_step = "receiving";

static kern_return_t inspect(mach_port_t task, mach_port_t thread, mach_vm_address_t buffer, uint32_t length,
                             int whole_task, uint32_t *sum_out, uint32_t *threads_out)
{
    server_step = "suspending the caller";
    kern_return_t kr = whole_task ? task_suspend(task) : thread_suspend(thread);
    if (kr != KERN_SUCCESS) {
        return kr;
    }

    x86_thread_state_t state;
    mach_msg_type_number_t count = x86_THREAD_STATE_COUNT;
    server_step = "getting the caller's thread state";
    kern_return_t result = thread_get_state(thread, x86_THREAD_STATE, (thread_state_t)&state, &count);

    thread_act_array_t list = NULL;
    mach_msg_type_number_t list_count = 0;
    if (result == KERN_SUCCESS) {
        server_step = "listing threads";
        result = task_threads(task, &list, &list_count);
    }
    if (result == KERN_SUCCESS) {
        for (mach_msg_type_number_t i = 0; i < list_count; i++) {
            count = x86_THREAD_STATE_COUNT;
            thread_get_state(list[i], x86_THREAD_STATE, (thread_state_t)&state, &count);
            mach_port_deallocate(mach_task_self(), list[i]);
        }
        vm_deallocate(mach_task_self(), (vm_address_t)list, list_count * sizeof(*list));
        *threads_out = list_count;
    }

    if (result == KERN_SUCCESS) {
        vm_offset_t data = 0;
        mach_msg_type_number_t data_count = 0;
        server_step = "reading memory";
        result = mach_vm_read(task, buffer, length, &data, &data_count);
        if (result == KERN_SUCCESS) {
            *sum_out = (data_count == length) ? checksum((const unsigned char *)data, data_count) : 0;
            vm_deallocate(mach_task_self(), data, data_count);
        }
    }

    if (result == KERN_SUCCESS) {
        mach_vm_address_t address = buffer;
        mach_vm_size_t size = 0;
        natural_t depth = 0;
        vm_region_submap_info_data_64_t info;
        mach_msg_type_number_t info_count = VM_REGION_SUBMAP_INFO_COUNT_64;
        server_step = "looking up a region";
        result = mach_vm_region_recurse(task, &address, &size, &depth, (vm_region_recurse_info_t)&info, &info_count);
    }

    server_step = "resuming the caller";
    kr = whole_task ? task_resume(task) : thread_resume(thread);
    server_step = "replying";
    return (result != KERN_SUCCESS) ? result : kr;
}

// receives and answers requests on `port` until a request with seq 0 arrives
static void serve(mach_port_t port, int whole_task)
{
    for (;;) {
        request_buffer_t in;
        memset(&in, 0, sizeof(in));
        server_step = "receiving";
        kern_return_t kr = mach_msg(&in.req.head, MACH_RCV_MSG, 0, sizeof(in), port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        if (kr != KERN_SUCCESS) {
            say("FAIL: server receive: 0x%x\n", kr);
            exit(1);
        }

        reply_t out;
        memset(&out, 0, sizeof(out));
        out.head.msgh_bits = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(in.req.head.msgh_bits), 0);
        out.head.msgh_remote_port = in.req.head.msgh_remote_port;
        out.head.msgh_size = sizeof(out);
        out.head.msgh_id = in.req.head.msgh_id + 100;
        out.ndr = NDR_record;
        out.seq = in.req.seq;

        int last = in.req.seq == 0;
        if (in.req.head.msgh_id != REQUEST_ID || !(in.req.head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||
            in.req.body.msgh_descriptor_count != 2) {
            out.ret = MIG_BAD_ARGUMENTS;
        } else if (!last) {
            out.ret = inspect(in.req.task.name, in.req.thread.name, in.req.buffer, in.req.length, whole_task,
                              &out.sum, &out.threads);
        }
        if (in.req.head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
            mach_port_deallocate(mach_task_self(), in.req.thread.name);
            mach_port_deallocate(mach_task_self(), in.req.task.name);
        }

        kr = mach_msg(&out.head, MACH_SEND_MSG, sizeof(out), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        if (kr != KERN_SUCCESS) {
            say("FAIL: server reply: 0x%x\n", kr);
            exit(1);
        }
        if (last) {
            return;
        }
    }
}

// the test's signals go to the caller (and the mapper), not to the server or the helper threads
static void block_test_signals(void)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

static void *server_thread(void *arg)
{
    block_test_signals();
    serve((mach_port_t)(uintptr_t)arg, 0);
    return NULL;
}

// --- client side ---

static int failures;
#define CHECK(cond) do { if (!(cond)) { say("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static volatile int phase_done;
static volatile int phase;
static volatile int all_done;
static volatile unsigned long progress;
static volatile sig_atomic_t signals_handled;
static pthread_t main_thread;
static unsigned char *buffer;

static void handler(int sig)
{
    (void)sig;
    signals_handled++;
}

// one request/reply exchange, like a MIG routine
static kern_return_t call(mach_port_t server, uint32_t seq, uint32_t *sum, uint32_t *threads)
{
    union {
        request_t req;
        reply_buffer_t rep;
    } msg;
    memset(&msg, 0, sizeof(msg));

    mach_port_t thread = mach_thread_self();
    mach_port_t reply_port = mig_get_reply_port();

    msg.req.head.msgh_bits = MACH_MSGH_BITS_COMPLEX | MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
    msg.req.head.msgh_remote_port = server;
    msg.req.head.msgh_local_port = reply_port;
    msg.req.head.msgh_size = sizeof(msg.req);
    msg.req.head.msgh_id = REQUEST_ID;
    msg.req.body.msgh_descriptor_count = 2;
    msg.req.thread.name = thread;
    msg.req.thread.disposition = MACH_MSG_TYPE_COPY_SEND;
    msg.req.thread.type = MACH_MSG_PORT_DESCRIPTOR;
    msg.req.task.name = mach_task_self();
    msg.req.task.disposition = MACH_MSG_TYPE_COPY_SEND;
    msg.req.task.type = MACH_MSG_PORT_DESCRIPTOR;
    msg.req.ndr = NDR_record;
    msg.req.buffer = (uint64_t)(uintptr_t)buffer;
    msg.req.length = BUFFER_SIZE;
    msg.req.seq = seq;

    kern_return_t kr = mach_msg(&msg.req.head, MACH_SEND_MSG | MACH_RCV_MSG, sizeof(msg.req), sizeof(msg.rep),
                                reply_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    mach_port_deallocate(mach_task_self(), thread);
    if (kr != KERN_SUCCESS) {
        return kr;
    }
    if (msg.rep.rep.head.msgh_id != REQUEST_ID + 100 || msg.rep.rep.head.msgh_size != sizeof(reply_t) ||
        msg.rep.rep.seq != seq) {
        say("FAIL: unexpected reply id %d size %u seq %u (expected seq %u)\n", msg.rep.rep.head.msgh_id,
               msg.rep.rep.head.msgh_size, msg.rep.rep.seq, seq);
        return MIG_REPLY_MISMATCH;
    }
    *sum = msg.rep.rep.sum;
    *threads = msg.rep.rep.threads;
    return msg.rep.rep.ret;
}

static void *signaller(void *arg)
{
    (void)arg;
    block_test_signals();
    while (!phase_done) {
        pthread_kill(main_thread, SIGUSR1);
        usleep(200);
    }
    return NULL;
}

static void *spawner(void *arg)
{
    (void)arg;
    char *argv[] = { "/usr/bin/true", NULL };
    block_test_signals();
    while (!phase_done) {
        pid_t pid;
        if (posix_spawn(&pid, "/usr/bin/true", NULL, NULL, argv, environ) == 0) {
            while (waitpid(pid, NULL, 0) < 0) {
            }
        }
        usleep(20000);
    }
    return NULL;
}

// maps memory entries as shared memory code does; these calls may be interrupted by the signals
static void *mapper(void *arg)
{
    (void)arg;
    while (!phase_done) {
        mach_vm_address_t source = 0;
        if (mach_vm_allocate(mach_task_self(), &source, 0x10000, VM_FLAGS_ANYWHERE) != KERN_SUCCESS) {
            continue;
        }
        memset((void *)(uintptr_t)source, 0x33, 0x10000);
        memory_object_size_t size = 0x10000;
        mach_port_t entry = MACH_PORT_NULL;
        if (mach_make_memory_entry_64(mach_task_self(), &size, source, VM_PROT_READ | VM_PROT_WRITE, &entry, MACH_PORT_NULL) == KERN_SUCCESS) {
            mach_vm_address_t mapped = 0;
            if (mach_vm_map(mach_task_self(), &mapped, size, 0, VM_FLAGS_ANYWHERE, entry, 0, FALSE,
                            VM_PROT_READ | VM_PROT_WRITE, VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_NONE) == KERN_SUCCESS) {
                mach_vm_deallocate(mach_task_self(), mapped, size);
            }
            mach_port_deallocate(mach_task_self(), entry);
        }
        mach_vm_deallocate(mach_task_self(), source, 0x10000);
    }
    return NULL;
}

static void *watchdog(void *arg)
{
    (void)arg;
    block_test_signals();
    unsigned long last = progress;
    time_t since = time(NULL);
    while (!all_done) {
        // (sleep() returns early when a signal is handled on this thread)
        sleep(1);
        if (progress != last) {
            last = progress;
            since = time(NULL);
        } else if (time(NULL) - since >= HANG_SECONDS) {
            say("FAIL: no reply for %d seconds (call %lu, phase %d; server thread: %s)\n", HANG_SECONDS, last,
                   phase, server_step);
            fflush(stdout);
            _exit(1);
        }
    }
    return NULL;
}

static void install_handlers(int flags)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = flags;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

static void run_phase(const char *name, mach_port_t server)
{
    pthread_t sig_thread, spawn_thread, map_thread;
    phase_done = 0;
    phase++;
    pthread_create(&sig_thread, NULL, signaller, NULL);
    pthread_create(&spawn_thread, NULL, spawner, NULL);
    pthread_create(&map_thread, NULL, mapper, NULL);

    struct itimerval timer = { { 0, 1000 }, { 0, 1000 } };
    setitimer(ITIMER_REAL, &timer, NULL);

    static dispatch_once_t once;
    static kern_return_t once_kr;
    static uint32_t once_sum, once_threads;
    static uint32_t seq = 1;

    // the first call is made from a dispatch_once block (other threads calling the same once block would wait for it)
    memset(buffer, 0x11, BUFFER_SIZE);
    dispatch_once(&once, ^{
        once_kr = call(server, seq, &once_sum, &once_threads);
    });
    CHECK(once_kr == KERN_SUCCESS);
    CHECK(once_sum == checksum(buffer, BUFFER_SIZE));
    seq++;

    unsigned long calls = 0;
    time_t end = time(NULL) + PHASE_SECONDS;
    while (time(NULL) < end && failures < 5) {
        memset(buffer, (int)(seq & 0xff), BUFFER_SIZE);
        buffer[seq % BUFFER_SIZE] ^= 0x5a;
        uint32_t sum = 0, threads = 0;
        kern_return_t kr = call(server, seq, &sum, &threads);
        if (kr != KERN_SUCCESS) {
            say("FAIL: %s call %u: 0x%x\n", name, seq, kr);
            failures++;
        } else {
            CHECK(sum == checksum(buffer, BUFFER_SIZE));
            CHECK(threads >= 4);
        }
        seq++;
        calls++;
        progress++;
    }

    struct itimerval off = { { 0, 0 }, { 0, 0 } };
    setitimer(ITIMER_REAL, &off, NULL);
    phase_done = 1;
    pthread_join(sig_thread, NULL);
    pthread_join(spawn_thread, NULL);
    pthread_join(map_thread, NULL);

    // stop the server
    uint32_t sum, threads;
    CHECK(call(server, 0, &sum, &threads) == KERN_SUCCESS);

    say("%s: %lu calls, %d signals handled\n", name, calls, (int)signals_handled);
    CHECK(calls >= 20);
}

static int server_main(const char *service)
{
    block_test_signals();
    mach_port_t port = MACH_PORT_NULL;
    mach_port_t bootstrap = MACH_PORT_NULL;
    if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS ||
        mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
        return 2;
    }
    task_get_bootstrap_port(mach_task_self(), &bootstrap);
    if (bootstrap_register(bootstrap, (char *)service, port) != KERN_SUCCESS) {
        return 3;
    }
    serve(port, 1);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "server") == 0) {
        return server_main(argv[2]);
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    main_thread = pthread_self();
    buffer = malloc(BUFFER_SIZE);

    pthread_t watch_thread;
    pthread_create(&watch_thread, NULL, watchdog, NULL);

    // phase 1: a server thread in this task
    install_handlers(0);
    mach_port_t port = MACH_PORT_NULL;
    CHECK(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port) == KERN_SUCCESS);
    CHECK(mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND) == KERN_SUCCESS);
    pthread_t serve_thread;
    pthread_create(&serve_thread, NULL, server_thread, (void *)(uintptr_t)port);
    run_phase("thread server", port);
    pthread_join(serve_thread, NULL);

    // phase 2: a server process that suspends this task while it inspects it
    install_handlers(SA_RESTART);
    char service[64];
    snprintf(service, sizeof(service), "org.darlinghq.test.rpc-inspect.%d", (int)getpid());
    char *child_argv[] = { argv[0], "server", service, NULL };
    pid_t child;
    CHECK(posix_spawn(&child, argv[0], NULL, NULL, child_argv, environ) == 0);

    mach_port_t bootstrap = MACH_PORT_NULL;
    task_get_bootstrap_port(mach_task_self(), &bootstrap);
    mach_port_t server = MACH_PORT_NULL;
    for (int i = 0; i < 500 && server == MACH_PORT_NULL; i++) {
        if (bootstrap_look_up(bootstrap, service, &server) != KERN_SUCCESS) {
            server = MACH_PORT_NULL;
            usleep(10000);
        }
    }
    CHECK(server != MACH_PORT_NULL);
    if (server != MACH_PORT_NULL) {
        run_phase("process server", server);
    }
    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
    }
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);

    all_done = 1;
    if (failures) {
        say("FAILED (%d)\n", failures);
        return 1;
    }
    say("PASS\n");
    return 0;
}
