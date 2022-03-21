#include "dserver-rpc-defs.h"
#include <darlingserver/rpc.h>

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
	dserver_rpc_hooks_msghdr_t callmsg = {
		.msg_name = dserver_rpc_hooks_get_server_address(),
		.msg_namelen = dserver_rpc_hooks_get_server_address_length(),
		.msg_iov = &call_data,
		.msg_iovlen = 1,
		.msg_control = reply->msg_control,
		.msg_controllen = reply->msg_controllen,
	};

	long int long_status = dserver_rpc_hooks_send_message(socket, &callmsg);

	if (long_status < 0) {
		dserver_rpc_hooks_printf("*** %d:%d: %s: BAD SEND STATUS: %ld ***\n", dserver_rpc_hooks_get_pid(), dserver_rpc_hooks_get_tid(), __func__, long_status);
		__simple_abort();
	}

	if (long_status != sizeof(call)) {
		dserver_rpc_hooks_printf("*** %d:%d: %s: BAD SEND LENGTH: %ld (expected %zu) ***\n", dserver_rpc_hooks_get_pid(), dserver_rpc_hooks_get_tid(), __func__, long_status, sizeof(call));
		__simple_abort();
	}
};
