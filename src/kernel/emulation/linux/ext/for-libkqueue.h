#ifndef _DARLING_EMULATION_FOR_LIBKQUEUE_H_
#define _DARLING_EMULATION_FOR_LIBKQUEUE_H_

#include <stdint.h>

#include <darling/emulation/base.h>

VISIBLE
int _dserver_rpc_kqchan_mach_port_open_4libkqueue(uint32_t port_name, void* receive_buffer, uint64_t receive_buffer_size, uint64_t saved_filter_flags, int* out_socket);

#endif // _DARLING_EMULATION_FOR_LIBKQUEUE_H_
