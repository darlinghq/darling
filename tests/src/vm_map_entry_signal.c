// Copyright (C) 2026 Lubos Dolezel
//
// mach_vm_map() of a memory entry (a named entry made with
// mach_make_memory_entry_64) is not interruptible: a signal that arrives while
// the mapping is being made must neither make the call fail nor leave stray
// mappings behind. The call has to succeed, the mapping has to share memory
// with the region the entry was made from, and a read-only entry must refuse a
// read/write mapping with KERN_INVALID_RIGHT (what macOS 26 returns).
//
// One thread maps entries in a loop while another thread keeps signalling it
// (the handler is installed once without and once with SA_RESTART).
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

#define ENTRY_SIZE 0x20000

static volatile sig_atomic_t signals_handled;
static volatile int phase_done;
static pthread_t mapper_thread;

static void handler(int sig)
{
    (void)sig;
    signals_handled++;
}

static void *signaller(void *arg)
{
    (void)arg;
    while (!phase_done) {
        pthread_kill(mapper_thread, SIGUSR1);
        usleep(30);
    }
    return NULL;
}

static unsigned long long map_attempts;
static unsigned long long map_failures;
static kern_return_t last_failure;

static void *mapper(void *arg)
{
    (void)arg;
    mach_vm_address_t source = 0;
    kern_return_t kr = mach_vm_allocate(mach_task_self(), &source, ENTRY_SIZE, VM_FLAGS_ANYWHERE);
    if (kr != KERN_SUCCESS) {
        map_failures++;
        last_failure = kr;
        return NULL;
    }
    memset((void *)source, 0x5a, ENTRY_SIZE);

    mach_port_t entry = MACH_PORT_NULL;
    memory_object_size_t size = ENTRY_SIZE;
    kr = mach_make_memory_entry_64(mach_task_self(), &size, source, VM_PROT_READ | VM_PROT_WRITE, &entry, MACH_PORT_NULL);
    if (kr != KERN_SUCCESS || size != ENTRY_SIZE) {
        map_failures++;
        last_failure = kr;
        return NULL;
    }

    unsigned long long round = 0;
    while (!phase_done) {
        mach_vm_address_t mapped = 0;
        kr = mach_vm_map(mach_task_self(), &mapped, ENTRY_SIZE, 0, VM_FLAGS_ANYWHERE, entry, 0, FALSE,
                         VM_PROT_READ | VM_PROT_WRITE, VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_NONE);
        map_attempts++;
        if (kr != KERN_SUCCESS) {
            map_failures++;
            last_failure = kr;
            continue;
        }
        // shared with the source region, not a copy
        volatile unsigned char *p = (volatile unsigned char *)mapped;
        p[0] = (unsigned char)round;
        if (((volatile unsigned char *)source)[0] != (unsigned char)round || p[ENTRY_SIZE - 1] != 0x5a) {
            map_failures++;
            last_failure = KERN_FAILURE;
        }
        mach_vm_deallocate(mach_task_self(), mapped, ENTRY_SIZE);
        round++;
    }

    mach_port_deallocate(mach_task_self(), entry);
    mach_vm_deallocate(mach_task_self(), source, ENTRY_SIZE);
    return NULL;
}

static void run_phase(const char *name, int flags)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = flags;
    CHECK(sigaction(SIGUSR1, &sa, NULL) == 0);

    phase_done = 0;
    signals_handled = 0;
    map_attempts = 0;
    map_failures = 0;
    last_failure = KERN_SUCCESS;

    pthread_t sig_thread;
    CHECK(pthread_create(&mapper_thread, NULL, mapper, NULL) == 0);
    CHECK(pthread_create(&sig_thread, NULL, signaller, NULL) == 0);
    sleep(5);
    phase_done = 1;
    pthread_join(sig_thread, NULL);
    pthread_join(mapper_thread, NULL);

    printf("%s: %llu mappings, %llu failures (last kr %d), %d signals handled\n", name, map_attempts, map_failures,
           (int)last_failure, (int)signals_handled);
    CHECK(map_failures == 0);
    CHECK(map_attempts > 100);
    CHECK(signals_handled > 0);
}

// A read-only entry cannot be mapped writable (the entry's permission limits the mapping's maximum protection).
static void read_only_entry(void)
{
    mach_vm_address_t source = 0;
    CHECK(mach_vm_allocate(mach_task_self(), &source, ENTRY_SIZE, VM_FLAGS_ANYWHERE) == KERN_SUCCESS);
    memset((void *)source, 0x33, ENTRY_SIZE);

    mach_port_t entry = MACH_PORT_NULL;
    memory_object_size_t size = ENTRY_SIZE;
    CHECK(mach_make_memory_entry_64(mach_task_self(), &size, source, VM_PROT_READ, &entry, MACH_PORT_NULL) == KERN_SUCCESS);

    mach_vm_address_t mapped = 0;
    kern_return_t kr = mach_vm_map(mach_task_self(), &mapped, ENTRY_SIZE, 0, VM_FLAGS_ANYWHERE, entry, 0, FALSE,
                                   VM_PROT_READ | VM_PROT_WRITE, VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_NONE);
    printf("read-only entry mapped read/write: kr %d\n", (int)kr);
    CHECK(kr == KERN_INVALID_RIGHT);
    if (kr == KERN_SUCCESS) mach_vm_deallocate(mach_task_self(), mapped, ENTRY_SIZE);

    mapped = 0;
    kr = mach_vm_map(mach_task_self(), &mapped, ENTRY_SIZE, 0, VM_FLAGS_ANYWHERE, entry, 0, FALSE,
                     VM_PROT_READ, VM_PROT_READ, VM_INHERIT_NONE);
    CHECK(kr == KERN_SUCCESS);
    if (kr == KERN_SUCCESS) {
        CHECK(*(volatile unsigned char *)mapped == 0x33);
        mach_vm_deallocate(mach_task_self(), mapped, ENTRY_SIZE);
    }
    mach_port_deallocate(mach_task_self(), entry);
    mach_vm_deallocate(mach_task_self(), source, ENTRY_SIZE);
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    alarm(55);
    read_only_entry();
    run_phase("signals without SA_RESTART", 0);
    run_phase("signals with SA_RESTART", SA_RESTART);
    if (failures) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
