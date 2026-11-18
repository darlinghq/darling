/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Named memory entries (mach_make_memory_entry_64) shared between mappings
// and between processes, the way a shared-memory region is built on them:
//   1. an entry created with MAP_MEM_NAMED_CREATE is mapped with
//      mach_vm_map(VM_FLAGS_ANYWHERE); every mapping sees the same pages;
//   2. a read-only entry made from a mapped range with MAP_MEM_VM_SHARE
//      shares that range (writes through the writable mapping are visible
//      through the read-only mapping) and refuses writable mappings;
//   3. entries sent in a Mach message to another process (a forked child
//      that learns the parent's port through its bootstrap port) map to the
//      same pages in both directions;
//   4. unmapping one mapping leaves the object alive for the others.
// Run for several sizes.  Prints one line per check; exit status 0 if all pass.

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) { printf("PASS: %s\n", #cond); } \
	else { failures++; printf("FAIL: %s  ", #cond); printf(__VA_ARGS__); printf("\n"); } \
	fflush(stdout); \
} while (0)

#define PAGE 4096ull

static const uint64_t sizes[] = { 4096, 65536, 2ull << 20, 256ull << 20 };

static kern_return_t make_rw(mach_vm_size_t size, mach_port_t* port) {
	memory_object_size_t s = size;
	return mach_make_memory_entry_64(mach_task_self(), &s, 0, MAP_MEM_NAMED_CREATE | VM_PROT_READ | VM_PROT_WRITE, port, MACH_PORT_NULL);
}

static kern_return_t map_entry(mach_port_t port, mach_vm_size_t size, vm_prot_t prot, mach_vm_address_t* addr) {
	*addr = 0;
	return mach_vm_map(mach_task_self(), addr, size, 0, VM_FLAGS_ANYWHERE, port, 0, FALSE, prot, prot, VM_INHERIT_NONE);
}

static kern_return_t make_ro_share(mach_vm_address_t mapped, mach_vm_size_t size, mach_port_t* port) {
	memory_object_size_t s = size;
	return mach_make_memory_entry_64(mach_task_self(), &s, mapped, MAP_MEM_VM_SHARE | VM_PROT_READ, port, MACH_PORT_NULL);
}

static void fill(volatile uint8_t* p, uint64_t size, uint8_t seed) {
	// first, middle and last page (except the first 64 bytes of each, where the control words live); the rest is left untouched so 256 MiB stays cheap
	uint64_t spots[3] = { 0, (size / 2) & ~(PAGE - 1), size - PAGE };
	for (int s = 0; s < 3; s++)
		for (uint64_t i = 64; i < PAGE; i++)
			p[spots[s] + i] = (uint8_t)(seed + i);
}

static int check(volatile uint8_t* p, uint64_t size, uint8_t seed) {
	uint64_t spots[3] = { 0, (size / 2) & ~(PAGE - 1), size - PAGE };
	for (int s = 0; s < 3; s++)
		for (uint64_t i = 64; i < PAGE; i++)
			if (p[spots[s] + i] != (uint8_t)(seed + i))
				return 0;
	return 1;
}

static int zero_filled(volatile uint8_t* p, uint64_t size) {
	uint64_t spots[3] = { 0, (size / 2) & ~(PAGE - 1), size - PAGE };
	for (int s = 0; s < 3; s++)
		for (uint64_t i = 64; i < PAGE; i++)
			if (p[spots[s] + i])
				return 0;
	return 1;
}

static void in_process(uint64_t size) {
	mach_port_t port = MACH_PORT_NULL, ro = MACH_PORT_NULL, bad = MACH_PORT_NULL;
	mach_vm_address_t a = 0, b = 0, r = 0, w = 0;
	kern_return_t kr;

	printf("-- in-process, size %llu\n", (unsigned long long)size);
	kr = make_rw(size, &port);
	CHECK(kr == KERN_SUCCESS && port != MACH_PORT_NULL, "kr=%d", kr);
	if (kr != KERN_SUCCESS) return;

	kr = map_entry(port, size, VM_PROT_READ | VM_PROT_WRITE, &a);
	CHECK(kr == KERN_SUCCESS, "first mapping kr=%d", kr);
	kr = map_entry(port, size, VM_PROT_READ | VM_PROT_WRITE, &b);
	CHECK(kr == KERN_SUCCESS, "second mapping kr=%d", kr);
	if (a == 0 || b == 0 || kr != KERN_SUCCESS) return;
	CHECK(a != b, "a=%llx b=%llx", (unsigned long long)a, (unsigned long long)b);
	CHECK(zero_filled((uint8_t*)(uintptr_t)a, size), "new entry is zero filled");
	fill((uint8_t*)(uintptr_t)a, size, 3);
	CHECK(check((uint8_t*)(uintptr_t)b, size, 3), "write via a visible via b");
	fill((uint8_t*)(uintptr_t)b, size, 9);
	CHECK(check((uint8_t*)(uintptr_t)a, size, 9), "write via b visible via a");

	// read-only duplicate of the writable mapping
	kr = make_ro_share(a, size, &ro);
	CHECK(kr == KERN_SUCCESS, "ro share kr=%d", kr);
	if (kr == KERN_SUCCESS) {
		kr = map_entry(ro, size, VM_PROT_READ, &r);
		CHECK(kr == KERN_SUCCESS, "ro mapping kr=%d", kr);
		if (kr == KERN_SUCCESS) {
			CHECK(check((uint8_t*)(uintptr_t)r, size, 9), "ro mapping sees existing data");
			fill((uint8_t*)(uintptr_t)a, size, 21);
			CHECK(check((uint8_t*)(uintptr_t)r, size, 21), "write via rw mapping visible via ro mapping");
			fill((uint8_t*)(uintptr_t)b, size, 33);
			CHECK(check((uint8_t*)(uintptr_t)r, size, 33), "write via second rw mapping visible via ro mapping");
		}
		kr = map_entry(ro, size, VM_PROT_READ | VM_PROT_WRITE, &w);
		CHECK(kr != KERN_SUCCESS, "writable mapping of read-only entry must fail, got kr=%d", kr);
		if (kr == KERN_SUCCESS) mach_vm_deallocate(mach_task_self(), w, size);
		printf("INFO: writable map of ro entry kr=%d\n", kr);
		if (r) mach_vm_deallocate(mach_task_self(), r, size);
		mach_port_deallocate(mach_task_self(), ro);
	}

	// unmapping one mapping leaves the object alive
	kr = mach_vm_deallocate(mach_task_self(), a, size);
	CHECK(kr == KERN_SUCCESS, "deallocate a kr=%d", kr);
	{
		mach_vm_address_t q = a;
		mach_vm_size_t qs = 0;
		vm_region_basic_info_data_64_t info;
		mach_msg_type_number_t cnt = VM_REGION_BASIC_INFO_COUNT_64;
		mach_port_t obj;
		kr = mach_vm_region(mach_task_self(), &q, &qs, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &cnt, &obj);
		CHECK(kr != KERN_SUCCESS || q != a, "unmapped address is gone (region now at %llx kr=%d)", (unsigned long long)q, kr);
	}
	CHECK(check((uint8_t*)(uintptr_t)b, size, 33), "data intact via b after unmapping a");
	kr = map_entry(port, size, VM_PROT_READ | VM_PROT_WRITE, &a);
	CHECK(kr == KERN_SUCCESS && check((uint8_t*)(uintptr_t)a, size, 33), "remap after unmap sees data, kr=%d", kr);

	// a read-only entry made from the entry directly cannot be widened
	{
		memory_object_size_t s = size;
		kr = mach_make_memory_entry_64(mach_task_self(), &s, 0, VM_PROT_READ, &bad, port);
		CHECK(kr == KERN_SUCCESS, "sub-entry read-only of entry kr=%d", kr);
		if (kr == KERN_SUCCESS) {
			mach_vm_address_t x = 0;
			kr = map_entry(bad, size, VM_PROT_READ | VM_PROT_WRITE, &x);
			CHECK(kr != KERN_SUCCESS, "writable map of read-only sub-entry must fail, got kr=%d", kr);
			if (kr == KERN_SUCCESS) mach_vm_deallocate(mach_task_self(), x, size);
			mach_port_deallocate(mach_task_self(), bad);
		}
	}

	if (a) mach_vm_deallocate(mach_task_self(), a, size);
	if (b) mach_vm_deallocate(mach_task_self(), b, size);
	mach_port_deallocate(mach_task_self(), port);
}

typedef struct {
	mach_msg_header_t head;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t ports[2];
	uint64_t size;
} entries_msg_t;

typedef struct {
	mach_msg_header_t head;
	mach_msg_trailer_t trailer;
} hello_msg_t;

typedef struct {
	mach_msg_header_t head;
	mach_msg_body_t body;
	mach_msg_port_descriptor_t ports[2];
	uint64_t size;
	mach_msg_trailer_t trailer;
} entries_recv_msg_t;

// Shared control words live in the first 64 bytes of the region; fill()/check() leave them alone.
#define CTL_CHILD_STATE 0
#define CTL_PARENT_STATE 1

static int wait_word(volatile uint32_t* w, uint32_t want) {
	struct timespec ts = { 0, 1000000 };
	for (int i = 0; i < 20000; i++) {
		if (*w == want) return 1;
		nanosleep(&ts, NULL);
	}
	return 0;
}

static int child_main(uint64_t size) {
	int bad = 0;
	mach_port_t bp = MACH_PORT_NULL, reply = MACH_PORT_NULL;
	kern_return_t kr = task_get_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, &bp);
	if (kr != KERN_SUCCESS) return 90;
	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &reply);
	if (kr != KERN_SUCCESS) return 91;

	hello_msg_t hello;
	memset(&hello, 0, sizeof(hello));
	hello.head.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	hello.head.msgh_size = sizeof(mach_msg_header_t);
	hello.head.msgh_remote_port = bp;
	hello.head.msgh_local_port = reply;
	kr = mach_msg(&hello.head, MACH_SEND_MSG, sizeof(mach_msg_header_t), 0, MACH_PORT_NULL, 5000, MACH_PORT_NULL);
	if (kr != KERN_SUCCESS) return 92;

	entries_recv_msg_t rm;
	memset(&rm, 0, sizeof(rm));
	kr = mach_msg(&rm.head, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(rm), reply, 10000, MACH_PORT_NULL);
	if (kr != KERN_SUCCESS) { printf("child: receive kr=%d\n", kr); return 93; }
	mach_port_t rw = rm.ports[0].name, ro = rm.ports[1].name;

	mach_vm_address_t a = 0, r = 0, w = 0;
	kr = map_entry(rw, size, VM_PROT_READ | VM_PROT_WRITE, &a);
	if (kr != KERN_SUCCESS) { printf("child: map rw kr=%d\n", kr); return 94; }
	kr = map_entry(ro, size, VM_PROT_READ, &r);
	if (kr != KERN_SUCCESS) { printf("child: map ro kr=%d\n", kr); return 95; }
	volatile uint32_t* ctl = (volatile uint32_t*)(uintptr_t)a;
	kr = map_entry(ro, size, VM_PROT_READ | VM_PROT_WRITE, &w);
	if (kr == KERN_SUCCESS) { printf("child: writable map of ro entry succeeded\n"); bad |= 1; }

	// handshake: parent signals 1 when its pattern (seed 40) is in place
	if (!wait_word(ctl + CTL_PARENT_STATE, 1)) { printf("child: parent never signalled\n"); return 96; }
	if (!check((volatile uint8_t*)(uintptr_t)a, size, 40)) { printf("child: parent data not visible via rw mapping\n"); bad |= 2; }
	if (!check((volatile uint8_t*)(uintptr_t)r, size, 40)) { printf("child: parent data not visible via ro mapping\n"); bad |= 4; }
	fill((volatile uint8_t*)(uintptr_t)a, size, 77);
	ctl[CTL_CHILD_STATE] = 1;
	// parent then checks child data, rewrites seed 90, signals 2
	if (!wait_word(ctl + CTL_PARENT_STATE, 2)) { printf("child: parent never signalled 2\n"); return 97; }
	if (!check((volatile uint8_t*)(uintptr_t)r, size, 90)) { printf("child: later parent write not visible via ro mapping\n"); bad |= 8; }
	// unmap and remap: the object outlives the mapping
	mach_vm_deallocate(mach_task_self(), a, size);
	kr = map_entry(rw, size, VM_PROT_READ | VM_PROT_WRITE, &a);
	if (kr != KERN_SUCCESS || !check((volatile uint8_t*)(uintptr_t)a, size, 90)) { printf("child: remap lost data kr=%d\n", kr); bad |= 16; }
	((volatile uint32_t*)(uintptr_t)a)[CTL_CHILD_STATE] = 2;
	return bad;
}

static void across_fork(uint64_t size) {
	mach_port_t rw = MACH_PORT_NULL, ro = MACH_PORT_NULL, recv = MACH_PORT_NULL, saved_bp = MACH_PORT_NULL;
	mach_vm_address_t a = 0;
	kern_return_t kr;

	printf("-- across fork, size %llu\n", (unsigned long long)size);
	kr = make_rw(size, &rw);
	if (kr != KERN_SUCCESS) { CHECK(0, "make_rw kr=%d", kr); return; }
	kr = map_entry(rw, size, VM_PROT_READ | VM_PROT_WRITE, &a);
	if (kr != KERN_SUCCESS) { CHECK(0, "map kr=%d", kr); return; }
	kr = make_ro_share(a, size, &ro);
	CHECK(kr == KERN_SUCCESS, "ro share kr=%d", kr);
	if (kr != KERN_SUCCESS) return;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &recv);
	CHECK(kr == KERN_SUCCESS, "allocate recv kr=%d", kr);
	kr = mach_port_insert_right(mach_task_self(), recv, recv, MACH_MSG_TYPE_MAKE_SEND);
	CHECK(kr == KERN_SUCCESS, "insert send kr=%d", kr);
	task_get_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, &saved_bp);
	kr = task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, recv);
	CHECK(kr == KERN_SUCCESS, "set bootstrap special port kr=%d", kr);
	if (kr != KERN_SUCCESS) return;

	volatile uint32_t* ctl = (volatile uint32_t*)(uintptr_t)a;
	ctl[CTL_CHILD_STATE] = 0;
	ctl[CTL_PARENT_STATE] = 0;
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0) {
		_exit(child_main(size));
	}
	task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, saved_bp);
	CHECK(pid > 0, "fork");

	struct { mach_msg_header_t head; char body[64]; mach_msg_trailer_t trailer; } hello;
	memset(&hello, 0, sizeof(hello));
	kr = mach_msg(&hello.head, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(hello), recv, 10000, MACH_PORT_NULL);
	CHECK(kr == KERN_SUCCESS, "receive hello kr=%d", kr);
	if (kr == KERN_SUCCESS) {
		entries_msg_t m;
		memset(&m, 0, sizeof(m));
		m.head.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MOVE_SEND_ONCE, 0, 0, MACH_MSGH_BITS_COMPLEX);
		m.head.msgh_size = sizeof(m);
		m.head.msgh_remote_port = hello.head.msgh_remote_port;
		m.body.msgh_descriptor_count = 2;
		m.ports[0].name = rw;
		m.ports[0].disposition = MACH_MSG_TYPE_COPY_SEND;
		m.ports[0].type = MACH_MSG_PORT_DESCRIPTOR;
		m.ports[1].name = ro;
		m.ports[1].disposition = MACH_MSG_TYPE_COPY_SEND;
		m.ports[1].type = MACH_MSG_PORT_DESCRIPTOR;
		m.size = size;
		kr = mach_msg(&m.head, MACH_SEND_MSG, sizeof(m), 0, MACH_PORT_NULL, 10000, MACH_PORT_NULL);
		CHECK(kr == KERN_SUCCESS, "send entries kr=%d", kr);

		fill((volatile uint8_t*)(uintptr_t)a, size, 40);
		ctl[CTL_PARENT_STATE] = 1;
		CHECK(wait_word((volatile uint32_t*)ctl + CTL_CHILD_STATE, 1), "child wrote and signalled");
		CHECK(check((volatile uint8_t*)(uintptr_t)a, size, 77), "child's write visible in parent");
		fill((volatile uint8_t*)(uintptr_t)a, size, 90);
		ctl[CTL_PARENT_STATE] = 2;
		CHECK(wait_word((volatile uint32_t*)ctl + CTL_CHILD_STATE, 2), "child remapped and signalled");
	}
	int status = 0;
	for (int i = 0; i < 100; i++) {
		pid_t w = waitpid(pid, &status, WNOHANG);
		if (w == pid) break;
		usleep(100000);
		if (i == 99) { kill(pid, SIGKILL); waitpid(pid, &status, 0); status = -1; }
	}
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "child exit status %d (bits: 1 ro-writable, 2/4 parent data, 8 later write, 16 remap; 9x setup)", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
	mach_vm_deallocate(mach_task_self(), a, size);
}

int main(void) {
	for (size_t i = 0; i < sizeof(sizes) / sizeof(*sizes); i++)
		in_process(sizes[i]);
	for (size_t i = 0; i < sizeof(sizes) / sizeof(*sizes); i++)
		across_fork(sizes[i]);
	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures ? 1 : 0;
}
