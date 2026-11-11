// Copyright (C) 2026 Lubos Dolezel
//
// task_restartable_ranges_register() and task_restartable_ranges_synchronize()
// (the task_restartable MIG subsystem, 8000) succeed for the caller's own task.
// libsystem_kernel does not export the routines and the SDK does not declare
// them, so the test sends the MIG messages itself. Before darlingserver stubbed
// them, the first request aborted the server.
#include <mach/mach.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL %s: %s\n", what, #cond); failures++; } else printf("PASS %s\n", what); } while (0)

#define REGISTER_ID 8000
#define SYNCHRONIZE_ID 8001

#pragma pack(push, 4)
typedef struct {
	mach_msg_header_t head;
	NDR_record_t ndr;
	mach_msg_type_number_t count;
	uint64_t ranges[64][2];
} register_request_t;

typedef struct {
	mach_msg_header_t head;
} synchronize_request_t;

typedef struct {
	mach_msg_header_t head;
	NDR_record_t ndr;
	kern_return_t ret;
	mach_msg_trailer_t trailer;
} reply_t;
#pragma pack(pop)

// the range format of <kern/restartable.h>: location, then length, recovery offset and flags
static void make_range(uint64_t out[2], uint64_t location, uint16_t length, uint16_t recovery_offs)
{
	out[0] = location;
	out[1] = (uint64_t)length | ((uint64_t)recovery_offs << 16);
}

static kern_return_t call(mach_msg_header_t* request, mach_msg_size_t size, mach_msg_id_t id)
{
	mach_port_t reply_port = mig_get_reply_port();
	reply_t reply;
	memset(&reply, 0, sizeof(reply));

	request->msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	request->msgh_remote_port = mach_task_self();
	request->msgh_local_port = reply_port;
	request->msgh_id = id;
	request->msgh_size = size;

	mach_msg_return_t mr = mach_msg(request, MACH_SEND_MSG | MACH_RCV_MSG, size, sizeof(reply), reply_port,
		MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (mr != MACH_MSG_SUCCESS) {
		printf("mach_msg failed: %#x\n", mr);
		return mr;
	}
	memcpy(&reply, request, sizeof(reply));
	if (reply.head.msgh_id != id + 100) {
		printf("unexpected reply id %d\n", reply.head.msgh_id);
		return MIG_REPLY_MISMATCH;
	}
	return reply.ret;
}

static void restartable_code(void) {}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);

	union {
		register_request_t reg;
		synchronize_request_t sync;
		reply_t reply;
	} msg;

	memset(&msg, 0, sizeof(msg));
	msg.reg.ndr = NDR_record;
	msg.reg.count = 1;
	make_range(msg.reg.ranges[0], (uint64_t)(uintptr_t)&restartable_code, 4, 0);
	mach_msg_size_t size = (mach_msg_size_t)(sizeof(register_request_t) - sizeof(msg.reg.ranges) + sizeof(msg.reg.ranges[0]));
	CHECK(call(&msg.reg.head, size, REGISTER_ID) == KERN_SUCCESS, "task_restartable_ranges_register with one range");

	memset(&msg, 0, sizeof(msg));
	CHECK(call(&msg.sync.head, sizeof(synchronize_request_t), SYNCHRONIZE_ID) == KERN_SUCCESS, "task_restartable_ranges_synchronize");

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
