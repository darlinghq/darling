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

#include <vmnet/vmnet.h>

const char * const vmnet_operation_mode_key = "vmnet_operation_mode";
const char * const vmnet_interface_id_key = "vmnet_interface_id";
const char * const vmnet_mac_address_key = "vmnet_mac_address";
const char * const vmnet_network_identifier_key = "vmnet_network_identifier";
const char * const vmnet_start_address_key = "vmnet_start_address";
const char * const vmnet_end_address_key = "vmnet_end_address";
const char * const vmnet_subnet_mask_key = "vmnet_subnet_mask";
const char * const vmnet_nat66_prefix_key = "vmnet_nat66_prefix";
const char * const vmnet_max_packet_size_key = "vmnet_max_packet_size";
const char * const vmnet_shared_interface_name_key = "vmnet_shared_interface_name";
const char * const vmnet_estimated_packets_available_key = "vmnet_estimated_packets";
const char * const vmnet_allocate_mac_address_key = "vmnet_allocate_mac_address";
const char * const vmnet_enable_checksum_offload_key = "vmnet_enable_checksum_offload";
const char * const vmnet_enable_isolation_key = "vmnet_enable_isolation";
const char * const vmnet_enable_tso_key = "vmnet_enable_tso";
const char * const vmnet_enable_virtio_header_key = "vmnet_enable_virtio_header";
const char * const vmnet_host_ip_address_key = "vmnet_host_ip_address";
const char * const vmnet_host_ipv6_address_key = "vmnet_host_ipv6_address";
const char * const vmnet_host_subnet_mask_key = "vmnet_host_subnet_mask";
const char * const vmnet_mtu_key = "vmnet_mtu";
const char * const vmnet_nat66_prefix_length_key = "vmnet_nat66_prefix_length";
const char * const vmnet_read_max_packets_key = "vmnet_read_max_packets";
const char * const vmnet_write_max_packets_key = "vmnet_write_max_packets";

// Darling has no host virtual network service. The handle returned by
// vmnet_start_interface() is a placeholder that never carries traffic; the
// start itself is reported as failed through the completion handler.
static struct vmnet_interface *vmnet_unavailable_interface(void)
{
	static char placeholder;
	return (struct vmnet_interface *) &placeholder;
}

interface_ref vmnet_start_interface(xpc_object_t interface_desc, dispatch_queue_t queue,
                                    vmnet_start_interface_completion_handler_t handler)
{
	if (!interface_desc || !queue || !handler) {
		return NULL;
	}

	dispatch_async(queue, ^{
		handler(VMNET_FAILURE, NULL);
	});
	return vmnet_unavailable_interface();
}

vmnet_return_t vmnet_stop_interface(interface_ref interface, dispatch_queue_t queue,
                                    vmnet_interface_completion_handler_t handler)
{
	if (!interface || !queue || !handler) {
		return VMNET_INVALID_ARGUMENT;
	}

	dispatch_async(queue, ^{
		handler(VMNET_SUCCESS);
	});
	return VMNET_SUCCESS;
}

vmnet_return_t vmnet_interface_set_event_callback(interface_ref interface,
                                                  interface_event_t event_mask,
                                                  dispatch_queue_t queue,
                                                  vmnet_interface_event_callback_t callback)
{
	if (!interface) {
		return VMNET_INVALID_ARGUMENT;
	}
	return VMNET_SUCCESS;
}

vmnet_return_t vmnet_read(interface_ref interface, struct vmpktdesc *packets, int *pktcnt)
{
	if (pktcnt) {
		*pktcnt = 0;
	}
	if (!interface || !packets || !pktcnt) {
		return VMNET_INVALID_ARGUMENT;
	}
	return VMNET_SETUP_INCOMPLETE;
}

vmnet_return_t vmnet_write(interface_ref interface, struct vmpktdesc *packets, int *pktcnt)
{
	if (pktcnt) {
		*pktcnt = 0;
	}
	if (!interface || !packets || !pktcnt) {
		return VMNET_INVALID_ARGUMENT;
	}
	return VMNET_SETUP_INCOMPLETE;
}
