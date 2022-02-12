#include "for-libkqueue.h"
#include <darlingserver/rpc.h>

int _dserver_rpc_kqchan_mach_port_open_4libkqueue(uint32_t port_name, void* receive_buffer, uint64_t receive_buffer_size, uint64_t saved_filter_flags, int* out_socket) {
	return dserver_rpc_kqchan_mach_port_open(port_name, receive_buffer, receive_buffer_size, saved_filter_flags, out_socket);
};

int _dserver_rpc_kqchan_proc_open_4libkqueue(int32_t pid, uint32_t flags, int* out_socket) {
	return dserver_rpc_kqchan_proc_open(pid, flags, out_socket);
};
