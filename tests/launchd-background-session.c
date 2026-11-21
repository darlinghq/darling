// Copyright (C) 2026 Lubos Dolezel
//
// Helper for tests/launchd-background-session.sh.
//
//   serve <service> <file>  checks in to a MachServices name, waits for one
//                           message and writes its id to <file>
//   send <service>          looks the service up and sends one message to it
//   lookup <service>        exits 0 if bootstrap_look_up finds the service
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_ID 0x4d5347

typedef struct { mach_msg_header_t head; } send_msg_t;
typedef struct { mach_msg_header_t head; char pad[64]; mach_msg_trailer_t trailer; } recv_msg_t;

int main(int argc, char** argv)
{
	if (argc < 3) return 2;
	mach_port_t port = MACH_PORT_NULL;
	kern_return_t kr;
	if (strcmp(argv[1], "serve") == 0 && argc == 4)
	{
		kr = bootstrap_check_in(bootstrap_port, argv[2], &port);
		if (kr != KERN_SUCCESS) { fprintf(stderr, "check_in: %d\n", kr); return 1; }
		recv_msg_t msg;
		kr = mach_msg(&msg.head, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(msg), port, 20000, MACH_PORT_NULL);
		if (kr != KERN_SUCCESS) { fprintf(stderr, "receive: %d\n", kr); return 1; }
		FILE* f = fopen(argv[3], "w");
		if (!f) return 1;
		fprintf(f, "%d\n", msg.head.msgh_id);
		fclose(f);
		return 0;
	}
	kr = bootstrap_look_up(bootstrap_port, argv[2], &port);
	if (kr != KERN_SUCCESS) { fprintf(stderr, "look_up: %d\n", kr); return 1; }
	if (strcmp(argv[1], "lookup") == 0) return 0;
	if (strcmp(argv[1], "send") != 0) return 2;
	send_msg_t msg = { { MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0), sizeof(msg), port, MACH_PORT_NULL, MACH_PORT_NULL, MSG_ID } };
	kr = mach_msg(&msg.head, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(msg), 0, MACH_PORT_NULL, 10000, MACH_PORT_NULL);
	if (kr != KERN_SUCCESS) { fprintf(stderr, "send: %d\n", kr); return 1; }
	return 0;
}
