#ifndef THREAD_H
#define THREAD_H
#include <stdint.h>
#include "task.h"

#define THREAD_BASIC_INFO		3
#define THREAD_IDENTIFIER_INFO	4

typedef natural_t thread_act_t;
typedef natural_t thread_flavor_t;

struct thread_identifier_info
{
	uint64_t thread_id;
	uint64_t thread_handle;
	uint64_t dispatch_qaddr;
};

#define THREAD_IDENTIFIER_INFO_COUNT (sizeof(struct thread_identifier_info) / sizeof(natural_t))

#ifdef __cplusplus
extern "C" {
#endif

kern_return_t thread_info(thread_act_t target_act, thread_flavor_t flavor, void* infoOut, mach_msg_type_number_t* count);

#ifdef __cplusplus
}
#endif

#endif

