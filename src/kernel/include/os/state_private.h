// This header has been made from scratch by:
// Andrew Hyatt <ahyattdev@icloud.com>
//
// SOSCloudKeychainClient.c needs this

#ifndef _OS_STATE_PRIVATE_H_
#define _OS_STATE_PRIVATE_H_

#define OS_STATE_DATA_SERIALIZED_NSCF_OBJECT 1

#define OS_STATE_API_FAULT 0
#define OS_STATE_API_REQUEST 1

#define OS_STATE_DATA_SIZE_NEEDED(statelen) statelen

typedef struct {
	int osd_type;
	int osd_size;
	char *osd_title;
	void *osd_data;
	int osd_data_size;
} *os_state_data_t;

typedef struct {
	int osh_api;
} *os_state_hints_t;

typedef os_state_data_t(^os_state_block_t)(os_state_hints_t hints);

// not sure what the real type for this is
typedef int os_state_handle_t;

os_state_handle_t os_state_add_handler(dispatch_queue_t arg0, os_state_block_t arg1);

void os_state_remove_handler(os_state_handle_t handler);

#endif
