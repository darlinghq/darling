// Copyright (C) 2026 Lubos Dolezel
//
// vmnet public contract: return-code and mode values, the interface key
// strings, argument validation of vmnet_read/write/stop with a NULL interface
// and of vmnet_start_interface with a NULL description, and the completion
// handler of a start that cannot succeed.
//
// The expected values were observed with this same program on macOS 27.0.1
// (arm64), where an unentitled start reports VMNET_FAILURE through its handler.
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <string.h>
#include <vmnet/vmnet.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_KEY(key, want) do { if (strcmp(key, want) != 0) { printf("FAIL %d: %s is \"%s\", want \"%s\"\n", __LINE__, #key, key, want); g_failures++; } } while (0)

static volatile vmnet_return_t g_start_status;

int main(void)
{
	CHECK((int) VMNET_SUCCESS == 1000);
	CHECK((int) VMNET_FAILURE == 1001);
	CHECK((int) VMNET_MEM_FAILURE == 1002);
	CHECK((int) VMNET_INVALID_ARGUMENT == 1003);
	CHECK((int) VMNET_SETUP_INCOMPLETE == 1004);
	CHECK((int) VMNET_INVALID_ACCESS == 1005);
	CHECK((int) VMNET_PACKET_TOO_BIG == 1006);
	CHECK((int) VMNET_BUFFER_EXHAUSTED == 1007);
	CHECK((int) VMNET_TOO_MANY_PACKETS == 1008);
	CHECK((int) VMNET_SHARING_SERVICE_BUSY == 1009);
	CHECK((int) VMNET_NOT_AUTHORIZED == 1010);
	CHECK((int) VMNET_HOST_MODE == 1000);
	CHECK((int) VMNET_SHARED_MODE == 1001);
	CHECK((int) VMNET_BRIDGED_MODE == 1002);
	CHECK((int) VMNET_INTERFACE_PACKETS_AVAILABLE == 1);

	CHECK_KEY(vmnet_allocate_mac_address_key, "vmnet_allocate_mac_address");
	CHECK_KEY(vmnet_enable_checksum_offload_key, "vmnet_enable_checksum_offload");
	CHECK_KEY(vmnet_enable_isolation_key, "vmnet_enable_isolation");
	CHECK_KEY(vmnet_enable_tso_key, "vmnet_enable_tso");
	CHECK_KEY(vmnet_enable_virtio_header_key, "vmnet_enable_virtio_header");
	CHECK_KEY(vmnet_end_address_key, "vmnet_end_address");
	CHECK_KEY(vmnet_estimated_packets_available_key, "vmnet_estimated_packets");
	CHECK_KEY(vmnet_host_ip_address_key, "vmnet_host_ip_address");
	CHECK_KEY(vmnet_host_ipv6_address_key, "vmnet_host_ipv6_address");
	CHECK_KEY(vmnet_host_subnet_mask_key, "vmnet_host_subnet_mask");
	CHECK_KEY(vmnet_interface_id_key, "vmnet_interface_id");
	CHECK_KEY(vmnet_mac_address_key, "vmnet_mac_address");
	CHECK_KEY(vmnet_max_packet_size_key, "vmnet_max_packet_size");
	CHECK_KEY(vmnet_mtu_key, "vmnet_mtu");
	CHECK_KEY(vmnet_nat66_prefix_key, "vmnet_nat66_prefix");
	CHECK_KEY(vmnet_nat66_prefix_length_key, "vmnet_nat66_prefix_length");
	CHECK_KEY(vmnet_network_identifier_key, "vmnet_network_identifier");
	CHECK_KEY(vmnet_operation_mode_key, "vmnet_operation_mode");
	CHECK_KEY(vmnet_read_max_packets_key, "vmnet_read_max_packets");
	CHECK_KEY(vmnet_shared_interface_name_key, "vmnet_shared_interface_name");
	CHECK_KEY(vmnet_start_address_key, "vmnet_start_address");
	CHECK_KEY(vmnet_subnet_mask_key, "vmnet_subnet_mask");
	CHECK_KEY(vmnet_write_max_packets_key, "vmnet_write_max_packets");

	CHECK(sizeof(struct vmpktdesc) == sizeof(size_t) + sizeof(void *) + 2 * sizeof(uint32_t));

	int cnt = 5;
	struct vmpktdesc pd;
	memset(&pd, 0, sizeof pd);
	CHECK(vmnet_read(NULL, &pd, &cnt) == VMNET_INVALID_ARGUMENT);
	CHECK(vmnet_write(NULL, &pd, &cnt) == VMNET_INVALID_ARGUMENT);
	CHECK(vmnet_stop_interface(NULL, dispatch_get_global_queue(0, 0), ^(vmnet_return_t s) { (void) s; }) == VMNET_INVALID_ARGUMENT);
	CHECK(vmnet_interface_set_event_callback(NULL, VMNET_INTERFACE_PACKETS_AVAILABLE, dispatch_get_global_queue(0, 0),
		^(interface_event_t e, xpc_object_t p) { (void) e; (void) p; }) == VMNET_INVALID_ARGUMENT);

	// A NULL interface description fails synchronously with a NULL handle.
	CHECK(vmnet_start_interface(NULL, dispatch_get_global_queue(0, 0), ^(vmnet_return_t s, xpc_object_t p) { (void) s; (void) p; }) == NULL);

	// A start that cannot succeed still returns a handle and reports the
	// failure through the completion handler.
	xpc_object_t desc = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_uint64(desc, vmnet_operation_mode_key, VMNET_SHARED_MODE);
	dispatch_semaphore_t sem = dispatch_semaphore_create(0);
	interface_ref iface = vmnet_start_interface(desc, dispatch_get_global_queue(0, 0), ^(vmnet_return_t status, xpc_object_t param) {
		(void) param;
		g_start_status = status;
		dispatch_semaphore_signal(sem);
	});
	CHECK(iface != NULL);
	CHECK(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0);
	CHECK(g_start_status == VMNET_FAILURE);

	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
