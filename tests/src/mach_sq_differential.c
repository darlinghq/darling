// Copyright (C) 2026 Lubos Dolezel
//
// Simple messages a task sends to its own ports must be received exactly as
// darlingserver delivers them, whether they were queued in the task's shared
// region or in the server. The test runs one matrix of sends and receives
// twice, in child processes, once with DARLING_MACH_SERVER=1 (every message
// through the server) and once without, and compares what each child saw byte
// for byte: return codes, the received header and body, every trailer level
// (seqno, security and audit tokens, context), MACH_RCV_LARGE and
// LARGE_IDENTITY answers, the effect of a full queue on the sender's rights,
// the port's status (make-send count, seqno and queue length after each step
// of a mix of MAKE_SEND and COPY_SEND traffic, also once the queue has moved
// back to the port, and the no-senders notification it ends in), and messages the server delivered (reply ports,
// descriptors) interleaved with simple ones.
//
// Port names and the process ID (in the audit token) differ between the two
// runs and are printed symbolically.
//
// The run without the switch must also have the shared region mapped and the
// one with it must not, so that the comparison is between the two paths.

#include "mach_sq_common.h"

#include <mach-o/dyld.h>
#include <spawn.h>
#include <sys/wait.h>

extern char** environ;

static mach_port_t names[16];
static const char* name_labels[16];
static int name_count;

static void label(mach_port_t name, const char* text)
{
	SQ_CHECK(name_count < 16, "too many labels");
	names[name_count] = name;
	name_labels[name_count] = text;
	name_count++;
}

// stands for this process's pid in the audit token of a received trailer, which differs between runs
#define PID_MARK 0x7e50d1d0u

static void print_word(uint32_t value)
{
	for (int i = 0; i < name_count; i++)
	{
		if (value == names[i] && value != 0)
		{
			printf(" <%s>", name_labels[i]);
			return;
		}
	}
	if (value == PID_MARK)
	{
		printf(" <pid>");
		return;
	}
	printf(" %08x", value);
}

// the first `size` bytes of a received buffer, as 32-bit words
static void dump(const char* what, mach_msg_return_t mr, const void* data, uint32_t size)
{
	const uint8_t* bytes = data;
	printf("%s: mr=%08x", what, mr);
	for (uint32_t i = 0; i + 4 <= size; i += 4)
	{
		uint32_t word;
		memcpy(&word, bytes + i, 4);
		print_word(word);
	}
	printf("\n");
}

static uint32_t received_length(const sq_buffer_t* buffer)
{
	const mach_msg_trailer_t* trailer =
		(const mach_msg_trailer_t*) ((const uint8_t*) buffer + buffer->header.msgh_size);
	uint32_t length = buffer->header.msgh_size + trailer->msgh_trailer_size;
	return length > sizeof(*buffer) ? sizeof(*buffer) : length;
}

static void dump_received(const char* what, mach_msg_return_t mr, const sq_buffer_t* buffer)
{
	if (mr == MACH_MSG_SUCCESS)
	{
		// the audit token's pid (its sixth word, at trailer offset 40) printed symbolically, if it is ours
		sq_buffer_t copy = *buffer;
		uint32_t length = received_length(buffer);
		uint32_t offset = buffer->header.msgh_size + 40;
		const mach_msg_trailer_t* trailer = (const mach_msg_trailer_t*) ((const uint8_t*) buffer + buffer->header.msgh_size);
		uint32_t pid;
		if (trailer->msgh_trailer_size >= 44 && offset + 4 <= length)
		{
			memcpy(&pid, (uint8_t*) &copy + offset, 4);
			if (pid == (uint32_t) getpid())
			{
				pid = PID_MARK;
				memcpy((uint8_t*) &copy + offset, &pid, 4);
			}
		}
		dump(what, mr, &copy, length);
	}
	else
		dump(what, mr, buffer, sizeof(mach_msg_header_t));
}

static void dump_status(const char* what, mach_port_t port)
{
	mach_port_status_t status;
	mach_msg_type_number_t count = MACH_PORT_RECEIVE_STATUS_COUNT;
	kern_return_t kr = mach_port_get_attributes(mach_task_self(), port, MACH_PORT_RECEIVE_STATUS,
		(mach_port_info_t) &status, &count);
	mach_port_urefs_t send = 0;
	mach_port_get_refs(mach_task_self(), port, MACH_PORT_RIGHT_SEND, &send);
	printf("%s: kr=%x seqno=%u mscount=%u qlimit=%u msgcount=%u sorights=%u srights=%d pd=%d ns=%d flags=%x urefs=%u\n",
		what, kr, status.mps_seqno, status.mps_mscount, status.mps_qlimit, status.mps_msgcount,
		status.mps_sorights, status.mps_srights, status.mps_pdrequest, status.mps_nsrequest, status.mps_flags, send);
}

static void prefill(sq_buffer_t* buffer)
{
	memset(buffer, 0xa5, sizeof(*buffer));
}

static void matrix(void)
{
	static const mach_msg_size_t sizes[] = { 24, 28, 64, 236, 240, 244, 300 };
	mach_port_t port = sq_port(true);
	sq_buffer_t buffer;
	char what[64];

	label(port, "P");
	SQ_KR(mach_port_set_context(mach_task_self(), port, 0x1122334455667788ull), "set_context");

	for (unsigned s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++)
	{
		for (int make = 0; make < 2; make++)
		{
			for (int elements = 0; elements <= 8; elements++)
			{
				mach_msg_option_t option = 0;
				if (elements > 0)
					option = MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | MACH_RCV_TRAILER_ELEMENTS(elements - 1);
				if (elements == 8)
					option = MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | MACH_RCV_TRAILER_ELEMENTS(7);

				mach_msg_return_t mr = sq_send_ex(port, make ? MACH_MSG_TYPE_MAKE_SEND : MACH_MSG_TYPE_COPY_SEND,
					sizes[s], 1000 + s * 100 + make * 10 + elements, 0, 0);
				snprintf(what, sizeof(what), "send size=%u make=%d el=%d", sizes[s], make, elements);
				dump(what, mr, NULL, 0);

				prefill(&buffer);
				mr = sq_receive_ex(port, &buffer, sizeof(buffer), option | MACH_RCV_TIMEOUT, 1000);
				snprintf(what, sizeof(what), "recv size=%u make=%d el=%d", sizes[s], make, elements);
				dump_received(what, mr, &buffer);
			}
		}
	}

	dump_status("matrix status", port);
	sq_destroy(port);
}

static void status_with_queue(void)
{
	mach_port_t port = sq_port(true);
	sq_buffer_t buffer;

	label(port, "S");
	dump_status("status empty", port);
	dump("send 1", sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 32, 1, 0, 0), NULL, 0);
	dump("send 2", sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 32, 2, 0, 0), NULL, 0);
	dump("send 3", sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 32, 3, 0, 0), NULL, 0);
	dump_status("status 3 queued", port);

	// without the task's own send right, only the queued messages hold send rights
	mach_port_deallocate(mach_task_self(), port);
	dump_status("status no own send", port);

	for (int i = 0; i < 3; i++)
	{
		prefill(&buffer);
		mach_msg_return_t mr = sq_receive_ex(port, &buffer, sizeof(buffer),
			MACH_RCV_TIMEOUT | MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), 1000);
		dump_received("status recv", mr, &buffer);
	}
	dump_status("status drained", port);
	sq_destroy(port);
}

static void large(void)
{
	mach_port_t port = sq_port(true);
	sq_buffer_t buffer;

	label(port, "L");
	sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 100, 7, 0, 0);

	prefill(&buffer);
	dump_received("large 64", sq_receive_ex(port, &buffer, 64, MACH_RCV_LARGE | MACH_RCV_TIMEOUT, 1000), &buffer);
	prefill(&buffer);
	dump_received("large identity 64", sq_receive_ex(port, &buffer, 64,
		MACH_RCV_LARGE | MACH_RCV_LARGE_IDENTITY | MACH_RCV_TIMEOUT, 1000), &buffer);
	prefill(&buffer);
	dump_received("large 12", sq_receive_ex(port, &buffer, 12, MACH_RCV_LARGE | MACH_RCV_TIMEOUT, 1000), &buffer);
	prefill(&buffer);
	// exactly the message and the minimal trailer
	dump_received("large fits", sq_receive_ex(port, &buffer, 108, MACH_RCV_LARGE | MACH_RCV_TIMEOUT, 1000), &buffer);

	// too small without MACH_RCV_LARGE: the message is destroyed
	sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 100, 8, 0, 0);
	prefill(&buffer);
	dump_received("small", sq_receive_ex(port, &buffer, 64, MACH_RCV_TIMEOUT, 1000), &buffer);
	prefill(&buffer);
	dump_received("small after", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 0), &buffer);

	// a trailer that does not fit
	sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 100, 9, 0, 0);
	prefill(&buffer);
	dump_received("large trailer", sq_receive_ex(port, &buffer, 110, MACH_RCV_LARGE | MACH_RCV_TIMEOUT |
		MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT), 1000), &buffer);
	prefill(&buffer);
	dump_received("voucher", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_VOUCHER | MACH_RCV_TIMEOUT, 1000),
		&buffer);

	dump_status("large status", port);
	sq_destroy(port);
}

static void full(void)
{
	mach_port_t port = sq_port(true);
	sq_buffer_t buffer;

	label(port, "F");
	sq_set_qlimit(port, 1);
	dump_status("full empty", port);
	dump("full send 1", sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 32, 1, MACH_SEND_TIMEOUT, 0), NULL, 0);
	dump("full send 2", sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 32, 2, MACH_SEND_TIMEOUT, 0), NULL, 0);
	dump("full send 3", sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 32, 3, MACH_SEND_TIMEOUT, 0), NULL, 0);
	dump("full send 4", sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 32, 4, MACH_SEND_TIMEOUT, 20), NULL, 0);
	dump_status("full status", port);

	prefill(&buffer);
	dump_received("full recv", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 1000), &buffer);
	prefill(&buffer);
	dump_received("full empty recv 0", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 0), &buffer);
	prefill(&buffer);
	dump_received("full empty recv 20", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 20), &buffer);
	dump_status("full drained", port);
	sq_destroy(port);
}

static void errors(void)
{
	mach_port_t port = sq_port(false);
	sq_buffer_t buffer;

	label(port, "E");
	// COPY_SEND without a send right
	dump("no send right", sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 32, 1, MACH_SEND_TIMEOUT, 0), NULL, 0);
	// MAKE_SEND from the receive right
	dump("make send", sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 32, 2, MACH_SEND_TIMEOUT, 0), NULL, 0);

	// a receive buffer that can't be written loses the message
	mach_msg_return_t mr = mach_msg_overwrite(NULL, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, 256, port, 1000,
		MACH_PORT_NULL, (mach_msg_header_t*) 16, 0);
	dump("bad buffer", mr, NULL, 0);
	prefill(&buffer);
	dump_received("bad buffer after", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 0), &buffer);

	// a send buffer that can't be read
	mr = mach_msg((mach_msg_header_t*) 16, MACH_SEND_MSG | MACH_SEND_TIMEOUT, 32, 0, MACH_PORT_NULL, 0,
		MACH_PORT_NULL);
	dump("bad send buffer", mr, NULL, 0);

	// size not a multiple of 4, too small
	dump("odd size", sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 30, 3, MACH_SEND_TIMEOUT, 0), NULL, 0);
	dump("tiny size", sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 20, 4, MACH_SEND_TIMEOUT, 0), NULL, 0);
	dump_status("errors status", port);
	sq_destroy(port);
}

// messages darlingserver delivers (with a reply port, with a descriptor) between simple ones keep their order
static void interleaved(void)
{
	mach_port_t port = sq_port(true);
	mach_port_t reply = sq_port(false);
	sq_buffer_t buffer;

	label(port, "I");
	label(reply, "R");
	// all twelve messages stay queued (the default limit is 5)
	sq_set_qlimit(port, 16);

	for (int round = 0; round < 3; round++)
	{
		sq_send(port, 100 + round * 10);

		struct
		{
			mach_msg_header_t header;
			uint32_t data;
		} with_reply = {
			.header = {
				.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE),
				.msgh_size = sizeof(with_reply),
				.msgh_remote_port = port,
				.msgh_local_port = reply,
				.msgh_id = 101 + round * 10,
			},
			.data = 0x12345678,
		};
		dump("send reply", mach_msg(&with_reply.header, MACH_SEND_MSG, sizeof(with_reply), 0, MACH_PORT_NULL, 0,
			MACH_PORT_NULL), NULL, 0);

		sq_send(port, 102 + round * 10);

		struct
		{
			mach_msg_header_t header;
			mach_msg_body_t body;
			mach_msg_port_descriptor_t descriptor;
		} complex = {
			.header = {
				.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0) | MACH_MSGH_BITS_COMPLEX,
				.msgh_size = sizeof(complex),
				.msgh_remote_port = port,
				.msgh_id = 103 + round * 10,
			},
			.body = { .msgh_descriptor_count = 1 },
			.descriptor = {
				.name = port,
				.disposition = MACH_MSG_TYPE_MAKE_SEND,
				.type = MACH_MSG_PORT_DESCRIPTOR,
			},
		};
		dump("send complex", mach_msg(&complex.header, MACH_SEND_MSG, sizeof(complex), 0, MACH_PORT_NULL, 0,
			MACH_PORT_NULL), NULL, 0);
	}
	dump_status("interleaved status", port);

	for (int i = 0; i < 12; i++)
	{
		prefill(&buffer);
		mach_msg_return_t mr = sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT |
			MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX), 1000);
		if (mr == MACH_MSG_SUCCESS && (buffer.header.msgh_bits & MACH_MSGH_BITS_COMPLEX))
		{
			// the descriptor names this port again: an extra uref of its send right
			mach_port_deallocate(mach_task_self(), port);
		}
		bool reply_right = mr == MACH_MSG_SUCCESS &&
			MACH_MSGH_BITS_REMOTE(buffer.header.msgh_bits) == MACH_MSG_TYPE_PORT_SEND_ONCE;
		// a new name for the send-once right: names are not the same in two processes
		if (reply_right)
			label(buffer.header.msgh_remote_port, "SO");
		dump_received("interleaved recv", mr, &buffer);
		if (reply_right)
		{
			name_count--;
			mach_port_deallocate(mach_task_self(), buffer.header.msgh_remote_port);
		}
	}
	dump_status("interleaved drained", port);
	sq_destroy(port);
	sq_destroy(reply);
}

// operations that take a port out of shared-queue mode keep its queue, seqno and counts
static void leave(void)
{
	sq_buffer_t buffer;
	mach_port_t port = sq_port(true);
	label(port, "Q");

	sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 40, 1, 0, 0);
	sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 40, 2, 0, 0);
	prefill(&buffer);
	dump_received("leave recv 1", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT |
		MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), 1000), &buffer);

	mach_port_seqno_t seqno = 0;
	mach_msg_size_t size = 0;
	mach_msg_id_t id = 0;
	mach_msg_max_trailer_t trailer;
	mach_msg_type_number_t trailer_size = sizeof(trailer);
	kern_return_t kr = mach_port_peek(mach_task_self(), port, MACH_RCV_TRAILER_NULL, &seqno, &size, &id,
		(mach_msg_trailer_info_t) &trailer, &trailer_size);
	printf("peek: kr=%x seqno=%u size=%u id=%d\n", kr, seqno, size, id);
	dump_status("leave peeked", port);

	sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 40, 3, 0, 0);
	for (int i = 0; i < 2; i++)
	{
		prefill(&buffer);
		dump_received("leave recv", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT |
			MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), 1000), &buffer);
	}
	dump_status("leave status", port);

	SQ_KR(mach_port_set_seqno(mach_task_self(), port, 100), "set_seqno");
	sq_send(port, 4);
	prefill(&buffer);
	dump_received("seqno recv", sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT |
		MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), 1000), &buffer);
	sq_destroy(port);
}

// the make-send count, seqno and queue length step by step, with MAKE_SEND messages queued, received, and moved
// back to the port by a no-senders request, until the notification
static void mscount_steps(void)
{
	sq_buffer_t buffer;
	char what[64];
	mach_port_t port = sq_port(false);
	mach_port_t notify = sq_port(false);
	mach_port_t previous = MACH_PORT_NULL;
	static const char steps[] = "MMrMMMrrMrIccMrrrMrMMr";
	int id = 0;

	label(port, "M");
	label(notify, "N");
	sq_set_qlimit(port, 16);
	dump_status("mscount start", port);
	for (unsigned i = 0; steps[i] != '\0'; i++)
	{
		mach_msg_return_t mr = MACH_MSG_SUCCESS;
		switch (steps[i])
		{
		case 'M':
			mr = sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 32, id++, 0, 0);
			break;
		case 'c':
			mr = sq_send_ex(port, MACH_MSG_TYPE_COPY_SEND, 32, id++, 0, 0);
			break;
		case 'I':
			mr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
			break;
		case 'r':
			prefill(&buffer);
			mr = sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT | MACH_RCV_TRAILER_TYPE(0) |
				MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), 1000);
			dump_received("mscount recv", mr, &buffer);
			break;
		}
		snprintf(what, sizeof(what), "mscount step %u %c (mr %x)", i, steps[i], mr);
		dump_status(what, port);
	}

	// queued MAKE_SEND messages and the own send right hold send rights; then only the messages
	dump_status("mscount before request", port);
	printf("request: kr=%x\n", mach_port_request_notification(mach_task_self(), port, MACH_NOTIFY_NO_SENDERS,
		7, notify, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous));
	dump_status("mscount after request", port);
	sq_send_ex(port, MACH_MSG_TYPE_MAKE_SEND, 32, id++, 0, 0);
	mach_port_deallocate(mach_task_self(), port);
	dump_status("mscount own right dropped", port);
	for (;;)
	{
		prefill(&buffer);
		mach_msg_return_t mr = sq_receive_ex(port, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT |
			MACH_RCV_TRAILER_TYPE(0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SEQNO), 100);
		dump_received("mscount drain recv", mr, &buffer);
		if (mr != MACH_MSG_SUCCESS)
			break;
	}
	prefill(&buffer);
	dump_received("mscount notification", sq_receive_ex(notify, &buffer, sizeof(buffer), MACH_RCV_TIMEOUT, 1000),
		&buffer);
	dump_status("mscount end", port);
	sq_destroy(port);
	sq_destroy(notify);
}

static int run_dump(void)
{
	// line by line, so that the output of a run that crashes ends where it crashed
	setvbuf(stdout, NULL, _IOLBF, 0);
	matrix();
	status_with_queue();
	large();
	full();
	errors();
	interleaved();
	leave();
	mscount_steps();
	fflush(stdout);
	return sq_region_mapped() == 1 ? 10 : (sq_region_mapped() == 0 ? 11 : 12);
}

// runs this program with --dump, and the switch on or off; returns its output
static char* run_child(bool server_only, int* code)
{
	char path[4096];
	uint32_t length = sizeof(path);
	SQ_CHECK(_NSGetExecutablePath(path, &length) == 0, "executable path");

	int fds[2];
	SQ_CHECK(pipe(fds) == 0, "pipe");

	char** env = calloc(512, sizeof(char*));
	int n = 0;
	for (char** e = environ; *e && n < 500; e++)
		if (strncmp(*e, "DARLING_MACH_SERVER=", 20) != 0)
			env[n++] = *e;
	if (server_only)
		env[n++] = "DARLING_MACH_SERVER=1";
	env[n] = NULL;

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_adddup2(&actions, fds[1], 1);
	posix_spawn_file_actions_addclose(&actions, fds[0]);

	char* argv[] = { path, "--dump", NULL };
	pid_t pid;
	SQ_CHECK(posix_spawn(&pid, path, &actions, NULL, argv, env) == 0, "posix_spawn");
	close(fds[1]);

	size_t capacity = 1 << 20, used = 0;
	char* output = malloc(capacity);
	ssize_t r;
	while ((r = read(fds[0], output + used, capacity - 1 - used)) > 0)
	{
		used += r;
		if (used + 1 >= capacity)
		{
			capacity *= 2;
			output = realloc(output, capacity);
		}
	}
	output[used] = '\0';
	close(fds[0]);

	int status;
	waitpid(pid, &status, 0);
	*code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	free(env);
	return output;
}

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "--dump") == 0)
		return run_dump();

	sq_watchdog(120);

	int code_server, code_shared;
	char* server = run_child(true, &code_server);
	char* shared = run_child(false, &code_shared);

	if (argc > 1 && strcmp(argv[1], "--print") == 0)
		printf("%s", shared);

	// the two outputs, line by line
	int line = 1, differences = 0;
	const char* a = server;
	const char* b = shared;
	while (*a || *b)
	{
		const char* ea = strchr(a, '\n');
		const char* eb = strchr(b, '\n');
		size_t la = ea ? (size_t) (ea - a) : strlen(a);
		size_t lb = eb ? (size_t) (eb - b) : strlen(b);
		if (la != lb || memcmp(a, b, la) != 0)
		{
			if (differences < 20)
				printf("line %d differs:\n  server: %.*s\n  shared: %.*s\n", line, (int) la, a, (int) lb, b);
			differences++;
		}
		a += la + (ea ? 1 : 0);
		b += lb + (eb ? 1 : 0);
		line++;
	}

	printf("%d lines, %d differ; region mapped: server-only run %s, default run %s\n", line - 1, differences,
		code_server == 10 ? "yes" : code_server == 11 ? "no" : "?",
		code_shared == 10 ? "yes" : code_shared == 11 ? "no" : "?");

	SQ_CHECK(line > 100, "the dumps are too short (%d lines)", line);
	SQ_CHECK(differences == 0, "the runs differ");
	SQ_CHECK(code_server == 11 || code_server == 12, "the server-only run has a shared region");
	SQ_CHECK(code_shared == 10 || code_shared == 12, "the default run has no shared region");
	printf("PASS\n");
	return 0;
}
