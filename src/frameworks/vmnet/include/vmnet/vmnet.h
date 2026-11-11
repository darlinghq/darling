/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _VMNET_H_
#define _VMNET_H_

#include <dispatch/dispatch.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/uio.h>
#include <xpc/xpc.h>

#ifdef __cplusplus
extern "C" {
#endif

// Darling has no host virtual network service: vmnet_start_interface() always
// reports failure through its completion handler.

typedef enum {
	VMNET_HOST_MODE = 1000,
	VMNET_SHARED_MODE = 1001,
	VMNET_BRIDGED_MODE = 1002,
} operating_modes_t;

typedef operating_modes_t vmnet_mode_t;

typedef enum {
	VMNET_INTERFACE_PACKETS_AVAILABLE = 1,
} interface_event_t;

typedef enum {
	VMNET_SUCCESS = 1000,
	VMNET_FAILURE = 1001,
	VMNET_MEM_FAILURE = 1002,
	VMNET_INVALID_ARGUMENT = 1003,
	VMNET_SETUP_INCOMPLETE = 1004,
	VMNET_INVALID_ACCESS = 1005,
	VMNET_PACKET_TOO_BIG = 1006,
	VMNET_BUFFER_EXHAUSTED = 1007,
	VMNET_TOO_MANY_PACKETS = 1008,
	VMNET_SHARING_SERVICE_BUSY = 1009,
	VMNET_NOT_AUTHORIZED = 1010,
} vmnet_return_t;

struct vmpktdesc {
	size_t vm_pkt_size;
	struct iovec *vm_pkt_iov;
	uint32_t vm_pkt_iovcnt;
	uint32_t vm_flags;
};

typedef struct vmnet_interface *interface_ref;

typedef void (^vmnet_start_interface_completion_handler_t)(vmnet_return_t status, xpc_object_t interface_param);
typedef void (^vmnet_interface_event_callback_t)(interface_event_t event_mask, xpc_object_t event);
typedef void (^vmnet_interface_completion_handler_t)(vmnet_return_t status);

// operation mode, a vmnet_mode_t value, given as a uint64
extern const char * const vmnet_operation_mode_key;
// UUID string of the interface; optional input, reported on output
extern const char * const vmnet_interface_id_key;
// MAC address of the interface, as text, on output
extern const char * const vmnet_mac_address_key;
// identifier of the virtual network to attach to
extern const char * const vmnet_network_identifier_key;
// first address of the DHCP range
extern const char * const vmnet_start_address_key;
// last address of the DHCP range
extern const char * const vmnet_end_address_key;
// subnet mask of the virtual network
extern const char * const vmnet_subnet_mask_key;
// IPv6 prefix of the NAT66 network
extern const char * const vmnet_nat66_prefix_key;
// largest packet the interface carries, on output
extern const char * const vmnet_max_packet_size_key;
// host interface to bridge to in bridged mode
extern const char * const vmnet_shared_interface_name_key;
// packets pending, in event parameters
extern const char * const vmnet_estimated_packets_available_key;
// whether vmnet picks the MAC address
extern const char * const vmnet_allocate_mac_address_key;
// checksum offload
extern const char * const vmnet_enable_checksum_offload_key;
// isolate interfaces on the same network from each other
extern const char * const vmnet_enable_isolation_key;
// TCP segmentation offload
extern const char * const vmnet_enable_tso_key;
// use virtio-net headers
extern const char * const vmnet_enable_virtio_header_key;
// IPv4 address of the host on the network
extern const char * const vmnet_host_ip_address_key;
// IPv6 address of the host on the network
extern const char * const vmnet_host_ipv6_address_key;
// subnet mask of the host address
extern const char * const vmnet_host_subnet_mask_key;
// MTU of the interface
extern const char * const vmnet_mtu_key;
// deprecated companion of the NAT66 prefix
extern const char * const vmnet_nat66_prefix_length_key;
// largest packet count for one vmnet_read
extern const char * const vmnet_read_max_packets_key;
// largest packet count for one vmnet_write
extern const char * const vmnet_write_max_packets_key;

interface_ref vmnet_start_interface(xpc_object_t interface_desc, dispatch_queue_t queue,
	vmnet_start_interface_completion_handler_t handler);

vmnet_return_t vmnet_interface_set_event_callback(interface_ref interface,
	interface_event_t event_mask, dispatch_queue_t queue,
	vmnet_interface_event_callback_t callback);

vmnet_return_t vmnet_write(interface_ref interface, struct vmpktdesc *packets, int *pktcnt);
vmnet_return_t vmnet_read(interface_ref interface, struct vmpktdesc *packets, int *pktcnt);

vmnet_return_t vmnet_stop_interface(interface_ref interface, dispatch_queue_t queue,
	vmnet_interface_completion_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif
