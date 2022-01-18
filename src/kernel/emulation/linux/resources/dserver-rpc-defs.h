#include "../network/recvmsg.h"
#include "../network/sendmsg.h"
#include "../network/getsockopt.h"
#include <stddef.h>
#include <sys/_types/_iovec_t.h>
#include <linux-syscalls/linux.h>
#include "../base.h"
#include "../duct_errno.h"
#include "../mach/lkm.h"
#include "../elfcalls_wrapper.h"

extern void* memcpy(void* dest, const void* src, __SIZE_TYPE__ n);

struct linux_sockaddr_un {
	unsigned short int sun_family;
	char sun_path[108];
};

#define dserver_rpc_hooks_msghdr_t struct linux_msghdr
#define dserver_rpc_hooks_iovec_t struct iovec
#define dserver_rpc_hooks_cmsghdr_t struct linux_cmsghdr
#define DSERVER_RPC_HOOKS_CMSG_SPACE LINUX_CMSG_SPACE
#define DSERVER_RPC_HOOKS_CMSG_FIRSTHDR(msghdr) ({ \
		struct linux_msghdr* _msghdr = (msghdr); \
		(_msghdr->msg_controllen >= sizeof(struct linux_cmsghdr)) ? ((struct linux_cmsghdr*)(_msghdr->msg_control)) : NULL; \
	})
#define DSERVER_RPC_HOOKS_SOL_SOCKET LINUX_SOL_SOCKET
#define DSERVER_RPC_HOOKS_SCM_RIGHTS 1
#define DSERVER_RPC_HOOKS_CMSG_LEN LINUX_CMSG_LEN
#define DSERVER_RPC_HOOKS_CMSG_DATA(cmsghdr) (&(cmsghdr)->cmsg_data[0])
#define DSERVER_RPC_HOOKS_ATTRIBUTE static

#define dserver_rpc_hooks_get_pid() ((pid_t)LINUX_SYSCALL0(__NR_getpid))

#define dserver_rpc_hooks_get_tid() ((pid_t)LINUX_SYSCALL(__NR_gettid))


#define dserver_rpc_hooks_get_server_address() ((void*)__dserver_socket_address())

#define dserver_rpc_hooks_get_server_address_length() sizeof(struct linux_sockaddr_un)

#define dserver_rpc_hooks_memcpy memcpy

static long int dserver_rpc_hooks_send_message(int socket, const dserver_rpc_hooks_msghdr_t* message) {
#ifdef __NR_socketcall
	return LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDMSG, ((long[6]) { socket, message, 0 }));
#else
	return LINUX_SYSCALL(__NR_sendmsg, socket, message, 0);
#endif
};

static long int dserver_rpc_hooks_receive_message(int socket, dserver_rpc_hooks_msghdr_t* out_message) {
#ifdef __NR_socketcall
	return LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_RECVMSG, ((long[6]) { socket, out_message, 0 }));
#else
	return LINUX_SYSCALL(__NR_recvmsg, socket, out_message, 0);
#endif
};

#define dserver_rpc_hooks_get_bad_message_status() LINUX_EBADMSG

#define dserver_rpc_hooks_get_communication_error_status() LINUX_ECOMM

#define dserver_rpc_hooks_get_broken_pipe_status() LINUX_EPIPE

#define dserver_rpc_hooks_close_fd(fd) ((int)LINUX_SYSCALL1(__NR_close, fd))

#define dserver_rpc_hooks_get_socket mach_driver_get_fd
