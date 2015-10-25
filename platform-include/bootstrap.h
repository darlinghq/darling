#ifndef __XPC_BOOTSTRAP_H__
#define __XPC_BOOTSTRAP_H__

#ifndef __XPC_INDIRECT__
#define __XPC_INDIRECT__
#endif // __XPC_INDIRECT__

#if XPC_BUILDING_LAUNCHD
// Temporary hack to resolve conflicting availability with launchd's existing
// internal headers.
#pragma GCC diagnostic ignored "-Wavailability"
#endif // XPC_BUILDING_LAUNCHD

#include <xpc/base.h>
#include <Availability.h>

// <rdar://problem/13536274>
#include <AvailabilityMacros.h>

__BEGIN_DECLS;

#include <mach/std_types.h>
#include <mach/message.h>
#include <sys/types.h>
#include <stdbool.h>

#define	BOOTSTRAP_MAX_NAME_LEN 128
#define	BOOTSTRAP_MAX_CMD_LEN 512

typedef char name_t[BOOTSTRAP_MAX_NAME_LEN];
typedef char cmd_t[BOOTSTRAP_MAX_CMD_LEN];
typedef name_t *name_array_t;
typedef int bootstrap_status_t;
typedef bootstrap_status_t *bootstrap_status_array_t;
typedef unsigned int bootstrap_property_t;
typedef bootstrap_property_t * bootstrap_property_array_t;

typedef boolean_t *bool_array_t;

#define BOOTSTRAP_MAX_LOOKUP_COUNT 20

#define BOOTSTRAP_SUCCESS 0
#define BOOTSTRAP_NOT_PRIVILEGED 1100
#define BOOTSTRAP_NAME_IN_USE 1101
#define BOOTSTRAP_UNKNOWN_SERVICE 1102
#define BOOTSTRAP_SERVICE_ACTIVE 1103
#define BOOTSTRAP_BAD_COUNT 1104
#define BOOTSTRAP_NO_MEMORY 1105
#define BOOTSTRAP_NO_CHILDREN 1106

#define BOOTSTRAP_STATUS_INACTIVE 0
#define BOOTSTRAP_STATUS_ACTIVE 1
#define BOOTSTRAP_STATUS_ON_DEMAND 2

XPC_EXPORT
mach_port_t bootstrap_port;

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_10, __IPHONE_2_0, __IPHONE_7_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL5
kern_return_t
bootstrap_create_server(mach_port_t bp, cmd_t server_cmd, uid_t server_uid,
	boolean_t on_demand, mach_port_t *server_port);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_10, __IPHONE_2_0, __IPHONE_7_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL3
kern_return_t
bootstrap_subset(mach_port_t bp, mach_port_t requestor_port,
	mach_port_t *subset_port);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_5, __IPHONE_2_0, __IPHONE_7_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL2
kern_return_t
bootstrap_unprivileged(mach_port_t bp, mach_port_t *unpriv_port);

__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL2
kern_return_t
bootstrap_parent(mach_port_t bp, mach_port_t *parent_port);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_5, __IPHONE_2_0, __IPHONE_7_0)
XPC_EXPORT XPC_WARN_RESULT
kern_return_t
bootstrap_register(mach_port_t bp, name_t service_name, mach_port_t sp);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_6, __IPHONE_2_0, __IPHONE_7_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL3
kern_return_t
bootstrap_create_service(mach_port_t bp, name_t service_name, mach_port_t *sp);

__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL3
kern_return_t
bootstrap_check_in(mach_port_t bp, const name_t service_name, mach_port_t *sp);

// Once <rdar://problem/13542045> is fixed, we can add back in XPC_WARN_RESULT.
__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
XPC_EXPORT XPC_NONNULL3
kern_return_t
bootstrap_look_up(mach_port_t bp, const name_t service_name, mach_port_t *sp);

__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_4, __MAC_10_5, __IPHONE_2_0, __IPHONE_7_0)
XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL2
kern_return_t
bootstrap_status(mach_port_t bp, name_t service_name,
	bootstrap_status_t *service_active);

__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
XPC_EXPORT XPC_WARN_RESULT
const char *
bootstrap_strerror(kern_return_t r);

__END_DECLS;

#endif // __XPC_BOOTSTRAP_H__ 
