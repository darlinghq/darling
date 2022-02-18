#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <darlingserver/rpc-supplement.h>

#define dserver_rpc_hooks_msghdr_t struct msghdr
#define dserver_rpc_hooks_iovec_t struct iovec
#define dserver_rpc_hooks_cmsghdr_t struct cmsghdr
#define DSERVER_RPC_HOOKS_CMSG_SPACE CMSG_SPACE
#define DSERVER_RPC_HOOKS_CMSG_FIRSTHDR CMSG_FIRSTHDR
#define DSERVER_RPC_HOOKS_SOL_SOCKET SOL_SOCKET
#define DSERVER_RPC_HOOKS_SCM_RIGHTS SCM_RIGHTS
#define DSERVER_RPC_HOOKS_CMSG_LEN CMSG_LEN
#define DSERVER_RPC_HOOKS_CMSG_DATA CMSG_DATA
#define DSERVER_RPC_HOOKS_ATTRIBUTE static

#define dserver_rpc_hooks_get_pid getpid

#define dserver_rpc_hooks_get_tid() ((pid_t)syscall(SYS_gettid))

#if __x86_64__
	#define dserver_rpc_hooks_get_architecture() dserver_rpc_architecture_x86_64
#elif __i386__
	#define dserver_rpc_hooks_get_architecture() dserver_rpc_architecture_i386
#elif __aarch64__
	#define dserver_rpc_hooks_get_architecture() dserver_rpc_architecture_arm64
#elif __arm__
	#define dserver_rpc_hooks_get_architecture() dserver_rpc_architecture_arm32
#else
	#define dserver_rpc_hooks_get_architecture() dserver_rpc_architecture_invalid
#endif

extern struct sockaddr_un __dserver_socket_address_data;

#define dserver_rpc_hooks_get_server_address() ((void*)&__dserver_socket_address_data)

#define dserver_rpc_hooks_get_server_address_length() sizeof(__dserver_socket_address_data)

#define dserver_rpc_hooks_memcpy memcpy

static long int dserver_rpc_hooks_send_message(int socket, const dserver_rpc_hooks_msghdr_t* message) {
	ssize_t ret = sendmsg(socket, message, 0);
	if (ret < 0) {
		return -errno;
	}
	return ret;
};

static long int dserver_rpc_hooks_receive_message(int socket, dserver_rpc_hooks_msghdr_t* out_message) {
	ssize_t ret = recvmsg(socket, out_message, 0);
	if (ret < 0) {
		return -errno;
	}

	if (ret >= sizeof(dserver_s2c_callhdr_t)) {
		dserver_s2c_callhdr_t* callhdr = out_message->msg_iov->iov_base;
		if (callhdr->call_number == 0x52cca11) {
			// this is an S2C call
			// mldr shouldn't need to be doing S2C calls
			fprintf(stderr, "mldr darlingserver RPC hooks received S2C call\n");
			abort();
		}
	}

	return ret;
};

#define dserver_rpc_hooks_get_bad_message_status() (-EBADMSG)

#define dserver_rpc_hooks_get_communication_error_status() (-ECOMM)

#define dserver_rpc_hooks_get_broken_pipe_status() (-EPIPE)

#define dserver_rpc_hooks_close_fd close

extern int __dserver_main_thread_socket_fd;

#define dserver_rpc_hooks_get_socket() __dserver_main_thread_socket_fd
