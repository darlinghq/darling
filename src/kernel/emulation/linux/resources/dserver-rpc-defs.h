#include "../network/recvmsg.h"
#include "../network/sendmsg.h"
#include "../network/getsockopt.h"
#include "../network/sendto.h"
#include <stddef.h>
#include <sys/_types/_iovec_t.h>
#include <linux-syscalls/linux.h>
#include "../base.h"
#include "../duct_errno.h"
#include "../mach/lkm.h"
#include "../elfcalls_wrapper.h"
#include "../simple.h"
#include "../signal/sigprocmask.h"
#include "../unistd/close.h"

#include <darlingserver/rpc-supplement.h>

#include <rtsig.h>

#ifndef DSERVER_RPC_HOOKS_ARCHITECTURE
#define DSERVER_RPC_HOOKS_ARCHITECTURE 1
enum dserver_rpc_architecture {
	dserver_rpc_architecture_invalid,
	dserver_rpc_architecture_i386,
	dserver_rpc_architecture_x86_64,
	dserver_rpc_architecture_arm32,
	dserver_rpc_architecture_arm64,
};

typedef enum dserver_rpc_architecture dserver_rpc_architecture_t;
#endif

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

#define dserver_rpc_hooks_get_pid() ((int)LINUX_SYSCALL0(__NR_getpid))

#define dserver_rpc_hooks_get_tid() ((int)LINUX_SYSCALL(__NR_gettid))

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
	long int ret = 0;

retry:
#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_RECVMSG, ((long[6]) { socket, out_message, 0 }));
#else
	ret = LINUX_SYSCALL(__NR_recvmsg, socket, out_message, 0);
#endif
	if (ret < 0) {
		return ret;
	}

	if (ret >= sizeof(dserver_s2c_callhdr_t)) {
		dserver_s2c_callhdr_t* callhdr = out_message->msg_iov->iov_base;

		if (callhdr->call_number == 0x52cca11) {
			// this is an S2C call
			// handle it
			switch (callhdr->s2c_number) {
				case dserver_s2c_msgnum_mmap: {
					dserver_s2c_call_mmap_t* mmap_call = out_message->msg_iov->iov_base;
					uint64_t call_ret;
					dserver_s2c_reply_mmap_t reply = {
						.header.call_number = 0x52cca11,
						.header.pid = dserver_rpc_hooks_get_pid(),
						.header.tid = dserver_rpc_hooks_get_tid(),
						.header.architecture = dserver_rpc_hooks_get_architecture(),
						.header.s2c_number = dserver_s2c_msgnum_mmap,
						.address = 0,
						.errno_result = 0,
					};

					if (mmap_call->fd == 0) {
						dserver_rpc_hooks_cmsghdr_t* reply_cmsg = DSERVER_RPC_HOOKS_CMSG_FIRSTHDR(out_message);
						if (!reply_cmsg || reply_cmsg->cmsg_level != DSERVER_RPC_HOOKS_SOL_SOCKET || reply_cmsg->cmsg_type != DSERVER_RPC_HOOKS_SCM_RIGHTS || reply_cmsg->cmsg_len != DSERVER_RPC_HOOKS_CMSG_LEN(sizeof(int))) {
							__simple_printf("Bad S2C call: no FD, but expected one");
							__simple_abort();
						}
						dserver_rpc_hooks_memcpy(&mmap_call->fd, DSERVER_RPC_HOOKS_CMSG_DATA(reply_cmsg), sizeof(int));
					} else if (mmap_call->fd != -1) {
						__simple_printf("Bad S2C call: invalid FD number");
						__simple_abort();
					}

#ifdef __NR_mmap2
					call_ret = (void*)LINUX_SYSCALL(__NR_mmap2, mmap_call->address, mmap_call->length, mmap_call->protection, mmap_call->flags, mmap_call->fd, mmap_call->offset / 4096);
#else
					call_ret = (void*)LINUX_SYSCALL(__NR_mmap, mmap_call->address, mmap_call->length, mmap_call->protection, mmap_call->flags, mmap_call->fd, mmap_call->offset);
#endif

					if (mmap_call->fd >= 0) {
						close_internal(mmap_call->fd);
					}

					if ((unsigned long)call_ret > (unsigned long)-4096) {
						// this is actually an errno
						reply.errno_result = -(int)call_ret;
						reply.address = (void*)(-1);
					} else {
						reply.address = call_ret;
					}

#ifdef __NR_socketcall
					ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDTO, ((long[6]) { socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length() }));
#else
					ret = LINUX_SYSCALL(__NR_sendto, socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length());
#endif
					if (ret < 0) {
						return ret;
					}
				} break;

				case dserver_s2c_msgnum_munmap: {
					dserver_s2c_call_munmap_t* munmap_call = out_message->msg_iov->iov_base;
					int call_ret;
					dserver_s2c_reply_munmap_t reply = {
						.header.call_number = 0x52cca11,
						.header.pid = dserver_rpc_hooks_get_pid(),
						.header.tid = dserver_rpc_hooks_get_tid(),
						.header.architecture = dserver_rpc_hooks_get_architecture(),
						.header.s2c_number = dserver_s2c_msgnum_munmap,
						.return_value = 0,
						.errno_result = 0,
					};

					call_ret = LINUX_SYSCALL2(__NR_munmap, munmap_call->address, munmap_call->length);

					if (call_ret < 0) {
						reply.return_value = -1;
						reply.errno_result = -call_ret;
					} else {
						reply.return_value = call_ret;
					}

#ifdef __NR_socketcall
					ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDTO, ((long[6]) { socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length() }));
#else
					ret = LINUX_SYSCALL(__NR_sendto, socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length());
#endif
					if (ret < 0) {
						return ret;
					}
				} break;

				case dserver_s2c_msgnum_mprotect: {
					dserver_s2c_call_mprotect_t* mprotect_call = out_message->msg_iov->iov_base;
					int call_ret;
					dserver_s2c_reply_mprotect_t reply = {
						.header.call_number = 0x52cca11,
						.header.pid = dserver_rpc_hooks_get_pid(),
						.header.tid = dserver_rpc_hooks_get_tid(),
						.header.architecture = dserver_rpc_hooks_get_architecture(),
						.header.s2c_number = dserver_s2c_msgnum_mprotect,
						.return_value = 0,
						.errno_result = 0,
					};

					call_ret = LINUX_SYSCALL3(__NR_mprotect, mprotect_call->address, mprotect_call->length, mprotect_call->protection);

					if (call_ret < 0) {
						reply.return_value = -1;
						reply.errno_result = -call_ret;
					} else {
						reply.return_value = call_ret;
					}

#ifdef __NR_socketcall
					ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDTO, ((long[6]) { socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length() }));
#else
					ret = LINUX_SYSCALL(__NR_sendto, socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length());
#endif
					if (ret < 0) {
						return ret;
					}
				} break;

				case dserver_s2c_msgnum_msync: {
					dserver_s2c_call_msync_t* msync_call = out_message->msg_iov->iov_base;
					int call_ret;
					dserver_s2c_reply_msync_t reply = {
						.header.call_number = 0x52cca11,
						.header.pid = dserver_rpc_hooks_get_pid(),
						.header.tid = dserver_rpc_hooks_get_tid(),
						.header.architecture = dserver_rpc_hooks_get_architecture(),
						.header.s2c_number = dserver_s2c_msgnum_msync,
						.return_value = 0,
						.errno_result = 0,
					};

					call_ret = LINUX_SYSCALL3(__NR_msync, msync_call->address, msync_call->size, msync_call->sync_flags);

					if (call_ret < 0) {
						reply.return_value = -1;
						reply.errno_result = -call_ret;
					} else {
						reply.return_value = call_ret;
					}

#ifdef __NR_socketcall
					ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_SENDTO, ((long[6]) { socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length() }));
#else
					ret = LINUX_SYSCALL(__NR_sendto, socket, &reply, sizeof(reply), 0, dserver_rpc_hooks_get_server_address(), dserver_rpc_hooks_get_server_address_length());
#endif
					if (ret < 0) {
						return ret;
					}
				} break;

				default:
					__simple_printf("Invalid S2C call number: %d", callhdr->s2c_number);
					__simple_abort();
			}

			// go back and wait for another message
			// we'll stop once we receive a normal message (i.e. not an S2C call)
			goto retry;
		}
	}

	return ret;
};

#define dserver_rpc_hooks_get_bad_message_status() (-LINUX_EBADMSG)

#define dserver_rpc_hooks_get_communication_error_status() (-LINUX_ECOMM)

#define dserver_rpc_hooks_get_broken_pipe_status() (-LINUX_EPIPE)

#define dserver_rpc_hooks_close_fd(fd) ((int)LINUX_SYSCALL1(__NR_close, fd))

#define dserver_rpc_hooks_get_socket mach_driver_get_fd

__attribute__((format(printf, 1, 2)))
void __dserver_rpc_hooks_printf(const char* format, ...);
#define dserver_rpc_hooks_printf __dserver_rpc_hooks_printf

#define dserver_rpc_hooks_atomic_save_t sigset_t

static void dserver_rpc_hooks_atomic_begin(dserver_rpc_hooks_atomic_save_t* atomic_save) {
	// block standard unix signals (not real-time signals, though)
	sigset_t set = 0x7fffffff;
	sys_sigprocmask(SIG_BLOCK, &set, atomic_save);
};

static void dserver_rpc_hooks_atomic_end(dserver_rpc_hooks_atomic_save_t* atomic_save) {
	sys_sigprocmask(SIG_SETMASK, atomic_save, NULL);
};

#define dserver_rpc_hooks_get_interrupt_status() (-LINUX_EINTR)

extern void __dserver_rpc_hooks_push_reply(int socket, const dserver_rpc_hooks_msghdr_t* reply, size_t size);
#define dserver_rpc_hooks_push_reply __dserver_rpc_hooks_push_reply
