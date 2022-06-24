#include "dserver-rpc-defs.h"
#include <darlingserver/rpc.h>

#include <libsimple/lock.h>
#include "../fcntl/open.h"

#include "../unistd/pipe.h"
#include "../unistd/read.h"

#ifndef DSERVER_RPC_USE_LOG_FILE
	// CHANGE TO 0 BEFORE COMMITTING
	#define DSERVER_RPC_USE_LOG_FILE 1
#endif

void __dserver_rpc_hooks_push_reply(int socket, const dserver_rpc_hooks_msghdr_t* reply, size_t size) {
	dserver_rpc_call_push_reply_t call = {
		.header = {
			.architecture = dserver_rpc_hooks_get_architecture(),
			.pid = dserver_rpc_hooks_get_pid(),
			.tid = dserver_rpc_hooks_get_tid(),
			.number = dserver_callnum_push_reply,
		},
		.reply = reply->msg_iov->iov_base,
		.reply_size = size,
	};
	dserver_rpc_hooks_iovec_t call_data = {
		.iov_base = &call,
		.iov_len = sizeof(call),
	};
	char controlbuf[DSERVER_RPC_HOOKS_CMSG_SPACE(sizeof(int) * (DSERVER_RPC_REPLY_MAX_FD_COUNT + 1))];
	size_t fd_count = ((reply->msg_control && reply->msg_controllen >= sizeof(dserver_rpc_hooks_cmsghdr_t)) ? (((dserver_rpc_hooks_cmsghdr_t*)reply->msg_control)->cmsg_len - LINUX_CMSG_ALIGN(sizeof(dserver_rpc_hooks_cmsghdr_t))) : 0) / sizeof(int);
	dserver_rpc_hooks_msghdr_t callmsg = {
		.msg_name = dserver_rpc_hooks_get_server_address(),
		.msg_namelen = dserver_rpc_hooks_get_server_address_length(),
		.msg_iov = &call_data,
		.msg_iovlen = 1,
		.msg_control = controlbuf,
		.msg_controllen = DSERVER_RPC_HOOKS_CMSG_LEN(sizeof(int) * (fd_count + 1)),
	};
	int pipe_ends[2];
	long int long_status;
	char tmp;
	dserver_rpc_hooks_cmsghdr_t* cmsg = (void*)&controlbuf[0];
	int* fds = (void*)DSERVER_RPC_HOOKS_CMSG_DATA(cmsg);

	cmsg->cmsg_level = DSERVER_RPC_HOOKS_SOL_SOCKET;
	cmsg->cmsg_type = DSERVER_RPC_HOOKS_SCM_RIGHTS;
	cmsg->cmsg_len = DSERVER_RPC_HOOKS_CMSG_LEN(sizeof(int) * (fd_count + 1));

	// copy over the FDs from the reply
	memcpy(fds, DSERVER_RPC_HOOKS_CMSG_DATA((dserver_rpc_hooks_cmsghdr_t*)reply->msg_control), sizeof(int) * fd_count);

	long_status = sys_pipe(pipe_ends);
	if (long_status < 0) {
		dserver_rpc_hooks_printf("*** sys_pipe failed: %ld ***\n", long_status);
		__simple_abort();
	}

	// give the write end to the server and keep the read end
	fds[fd_count] = pipe_ends[1];

	dserver_rpc_hooks_printf("*** pushing reply for call #%d with code %d (size = %llu); pointer = %p\n", ((dserver_rpc_replyhdr_t*)call.reply)->number, ((dserver_rpc_replyhdr_t*)call.reply)->code, call.reply_size, (void*)call.reply);

	long_status = dserver_rpc_hooks_send_message(socket, &callmsg);

	if (long_status < 0) {
		dserver_rpc_hooks_printf("*** %d:%d: %s: BAD SEND STATUS: %ld ***\n", dserver_rpc_hooks_get_pid(), dserver_rpc_hooks_get_tid(), __func__, long_status);
		__simple_abort();
	}

	if (long_status != sizeof(call)) {
		dserver_rpc_hooks_printf("*** %d:%d: %s: BAD SEND LENGTH: %ld (expected %zu) ***\n", dserver_rpc_hooks_get_pid(), dserver_rpc_hooks_get_tid(), __func__, long_status, sizeof(call));
		__simple_abort();
	}

	// close the write end
	close_internal(pipe_ends[1]);

	// and wait for a byte from the server
	sys_read_nocancel(pipe_ends[0], &tmp, sizeof(tmp));

	close_internal(pipe_ends[0]);
};

#if !defined(VARIANT_DYLD) && DSERVER_RPC_USE_LOG_FILE
static int printf_file = -1;
static libsimple_once_t printf_file_token = LIBSIMPLE_ONCE_INITIALIZER;

static void printf_file_init(void* context) {
	printf_file = LINUX_SYSCALL(__NR_open, "/tmp/dserver-client-rpc.log", LINUX_O_WRONLY | LINUX_O_CREAT | LINUX_O_APPEND, 0644);
};
#endif

void __dserver_rpc_hooks_printf(const char* format, ...) {
	va_list args;
	va_start(args, format);
#if defined(VARIANT_DYLD) || !DSERVER_RPC_USE_LOG_FILE
	__simple_vprintf(format, args);
#else
	libsimple_once(&printf_file_token, printf_file_init, NULL);
	__simple_vfprintf(printf_file, format, args);
#endif
	va_end(args);
};
