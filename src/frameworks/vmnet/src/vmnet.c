#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <sys/uio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
	VMNET_SUCCESS = 1000,
	VMNET_FAILURE = 1001,
	VMNET_MEM_FAILURE = 1002,
	VMNET_INVALID_ARGUMENT = 1003,
	VMNET_SETUP_INCOMPLETE = 1004,
	VMNET_UNSUPPORTED = 1005,
	VMNET_BUFFER_EXHAUSTED = 1006,
	VMNET_TOO_MANY_PACKETS = 1007,
} vmnet_return_t;

struct vmpktdesc {
	struct iovec *vm_pkt_iov;
	uint32_t      vm_pkt_iovcnt;
	uint32_t      vm_flags;
};

typedef struct interface_s* interface_ref;

const char* const vmnet_operation_mode_key = "vmnet_operation_mode_key";
const char* const vmnet_interface_id_key = "vmnet_interface_id_key";
const char* const vmnet_mac_address_key = "vmnet_mac_address_key";
const char* const vmnet_network_identifier_key = "vmnet_network_identifier_key";
const char* const vmnet_start_address_key = "vmnet_start_address_key";
const char* const vmnet_end_address_key = "vmnet_end_address_key";
const char* const vmnet_subnet_mask_key = "vmnet_subnet_mask_key";
const char* const vmnet_nat66_prefix_key = "vmnet_nat66_prefix_key";
const char* const vmnet_max_packet_size_key = "vmnet_max_packet_size_key";
const char* const vmnet_shared_interface_name_key = "vmnet_shared_interface_name_key";
const char* const vmnet_estimated_packets_available_key = "vmnet_estimated_packets_available_key";
const char* const vmnet_allocated_mac_address_key = "vmnet_allocated_mac_address_key";

interface_ref vmnet_start_interface(xpc_object_t interface_desc,
                                    dispatch_queue_t queue,
                                    void (^handler)(vmnet_return_t status, xpc_object_t interface_param))
{
	if (handler) {
		if (queue) {
			dispatch_async(queue, ^{
				handler(VMNET_FAILURE, NULL);
			});
		} else {
			handler(VMNET_FAILURE, NULL);
		}
	}
	return NULL;
}

vmnet_return_t vmnet_stop_interface(interface_ref interface,
                                    dispatch_queue_t queue,
                                    void (^handler)(vmnet_return_t status))
{
	if (handler) {
		if (queue) {
			dispatch_async(queue, ^{
				handler(VMNET_SUCCESS);
			});
		} else {
			handler(VMNET_SUCCESS);
		}
	}
	return VMNET_SUCCESS;
}

vmnet_return_t vmnet_interface_set_event_callback(interface_ref interface,
                                                  uint32_t event_mask,
                                                  dispatch_queue_t queue,
                                                  void (^handler)(uint32_t event_mask, xpc_object_t event_param))
{
	return VMNET_SUCCESS;
}

vmnet_return_t vmnet_read(interface_ref interface, struct vmpktdesc *packets, int *pktcnt) {
	if (pktcnt) *pktcnt = 0;
	return VMNET_UNSUPPORTED;
}

vmnet_return_t vmnet_write(interface_ref interface, struct vmpktdesc *packets, int *pktcnt) {
	if (pktcnt) *pktcnt = 0;
	return VMNET_UNSUPPORTED;
}
