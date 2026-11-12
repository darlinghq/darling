// Copyright (C) 2026 Lubos Dolezel
//
// Out-of-line memory and out-of-line port descriptors, and the MIG calls that
// return port arrays, must work for 32-bit and 64-bit tasks alike. A message
// with both kinds of descriptor is sent to a port of the same task and read
// back; task_threads() (reply with an out-of-line port array) must list the
// threads; mach_ports_lookup() likewise returns an array of ports.
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <mach/mach.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

typedef struct {
    mach_msg_header_t header;
    mach_msg_body_t body;
    mach_msg_ool_descriptor_t data;
    mach_msg_ool_ports_descriptor_t ports;
    mach_msg_port_descriptor_t port;
    uint32_t tag;
} complex_send_t;

typedef struct {
    complex_send_t message;
    mach_msg_trailer_t trailer;
} complex_receive_t;

static void *idle(void *arg)
{
    (void)arg;
    sleep(2);
    return NULL;
}

int main(void)
{
    mach_port_t port = MACH_PORT_NULL;
    kern_return_t kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    CHECK(kr == KERN_SUCCESS);
    kr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
    CHECK(kr == KERN_SUCCESS);

    static char payload[5000];
    for (size_t i = 0; i < sizeof payload; i++) payload[i] = (char)(i * 7 + 3);
    mach_port_t carried[2] = { port, port };

    complex_send_t out;
    memset(&out, 0, sizeof out);
    out.header.msgh_bits = MACH_MSGH_BITS_COMPLEX | MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    out.header.msgh_size = sizeof out;
    out.header.msgh_remote_port = port;
    out.header.msgh_id = 4242;
    out.body.msgh_descriptor_count = 3;
    out.data.address = payload;
    out.data.size = sizeof payload;
    out.data.deallocate = FALSE;
    out.data.copy = MACH_MSG_VIRTUAL_COPY;
    out.data.type = MACH_MSG_OOL_DESCRIPTOR;
    out.ports.address = carried;
    out.ports.count = 2;
    out.ports.deallocate = FALSE;
    out.ports.copy = MACH_MSG_VIRTUAL_COPY;
    out.ports.disposition = MACH_MSG_TYPE_COPY_SEND;
    out.ports.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
    out.port.name = port;
    out.port.disposition = MACH_MSG_TYPE_COPY_SEND;
    out.port.type = MACH_MSG_PORT_DESCRIPTOR;
    out.tag = 0xA5A5A5A5u;

    kr = mach_msg(&out.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof out, 0, MACH_PORT_NULL, 2000, MACH_PORT_NULL);
    CHECK(kr == MACH_MSG_SUCCESS);

    complex_receive_t in;
    memset(&in, 0, sizeof in);
    kr = mach_msg(&in.message.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof in, port, 2000, MACH_PORT_NULL);
    CHECK(kr == MACH_MSG_SUCCESS);
    if (kr == MACH_MSG_SUCCESS) {
        CHECK(in.message.header.msgh_id == 4242);
        CHECK(in.message.header.msgh_size == sizeof in.message);
        CHECK((in.message.header.msgh_bits & MACH_MSGH_BITS_COMPLEX) != 0);
        CHECK(in.message.body.msgh_descriptor_count == 3);
        CHECK(in.message.data.type == MACH_MSG_OOL_DESCRIPTOR);
        CHECK(in.message.data.size == sizeof payload);
        CHECK(in.message.data.address != NULL && memcmp(in.message.data.address, payload, sizeof payload) == 0);
        CHECK(in.message.ports.type == MACH_MSG_OOL_PORTS_DESCRIPTOR);
        CHECK(in.message.ports.count == 2);
        if (in.message.ports.address != NULL && in.message.ports.count == 2) {
            mach_port_t *names = (mach_port_t *)in.message.ports.address;
            CHECK(names[0] == port && names[1] == port);
        }
        CHECK(in.message.port.type == MACH_MSG_PORT_DESCRIPTOR);
        CHECK(in.message.port.name == port);
        CHECK(in.message.tag == 0xA5A5A5A5u);
    }

    // A reply that carries an out-of-line port array.
    pthread_t thread;
    CHECK(pthread_create(&thread, NULL, idle, NULL) == 0);
    thread_act_array_t threads = NULL;
    mach_msg_type_number_t count = 0;
    kr = task_threads(mach_task_self(), &threads, &count);
    CHECK(kr == KERN_SUCCESS);
    CHECK(count >= 2);
    if (kr == KERN_SUCCESS) {
        int mine = 0;
        for (mach_msg_type_number_t i = 0; i < count; i++) {
            if (threads[i] == mach_thread_self()) mine++;
            mach_port_deallocate(mach_task_self(), threads[i]);
        }
        CHECK(mine >= 1);
        vm_deallocate(mach_task_self(), (vm_address_t)threads, count * sizeof(threads[0]));
    }

    mach_port_array_t ports = NULL;
    mach_msg_type_number_t portCount = 0;
    kr = mach_ports_lookup(mach_task_self(), &ports, &portCount);
    CHECK(kr == KERN_SUCCESS);
    if (kr == KERN_SUCCESS && ports != NULL) {
        vm_deallocate(mach_task_self(), (vm_address_t)ports, portCount * sizeof(ports[0]));
    }

    if (failures != 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    printf("PASS: mach out-of-line descriptors\n");
    return 0;
}
