#ifndef __BOOTSTRAP_H__
#define __BOOTSTRAP_H__
/*
 * Copyright (c) 1999-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

/*
 * bootstrap -- fundamental service initiator and port server
 * Mike DeMoney, NeXT, Inc.
 * Copyright, 1990.  All rights reserved.
 */

/*
 *	Interface:	Bootstrap server
 *
 *  The bootstrap server is the first user-mode task initiated by the Mach
 *  kernel at system boot time.  The bootstrap server provides two services,
 *  it initiates other system tasks, and manages a table of name-port bindings
 *  for fundamental system services  (e.g. lookupd, Window Manager, etc...).
 *
 *  Name-port bindings can be established with the bootstrap server by either 
 *  of two mechanisms:
 *
 *  1.  The binding can be indicated, in advance of the service that backs it
 *  being available, via a "service create" request.  In this case, bootstrap
 *  will immediately create a port and bind the indicated name with that port.
 *  At a later time, a service may "checkin" for the name-port
 *  binding and will be returned receive rights for the bound port.  Lookup's
 *  on bindings created by this mechanism will return send rights to the port,
 *  even if no service has "checked-in".  In this case, requests sent to the
 *  bound port will be queued until a server has checked-in and can satisfy the
 *  request.
 *
 *  2.  Bindings can be established dynamically via a "register" request.  In
 *  this case, the register request provides bootstrap with a name and send
 *  rights for a port.  Bootstrap will provide send rights for the bound port
 *  to any requestor via the lookup request.
 *
 *  Bootstrap provides its service port to descendant tasks via the Mach
 *  "bootstrap" special task port.  All direct descendants of bootstrap receive
 *  a "privileged" bootstrap service port.  System services that initiate
 *  untrusted tasks should replace the Mach bootstrap task special port with
 *  a subset bootstrap port to prevent them from infecting the namespace.
 *
 *  The bootstrap server creates a "backup" port for each service that it
 *  creates.  This is used to detect when a checked out service is no longer
 *  being served.  The bootstrap server regains all rights to the port and
 *  it is marked available for check-out again.  This allows crashed servers to 
 *  resume service to previous clients.  Lookup's on this named port will 
 *  continue to be serviced by bootstrap while holding receive rights for the 
 *  bound port.  A client may detect that the service is inactive via the
 *  bootstrap status request.  If an inactive service re-registers rather
 *  than "checking-in" the original bound port is destroyed.
 *
 *  The status of a named service may be obtained via the "status" request.
 *  A service is "active" if a name-port binding exists and receive rights
 *  to the bound port are held by a task other than bootstrap.
 *
 *  The bootstrap server may also (re)start server processes associated with
 *  with a set of services. The definition of the server process is done
 *  through the "create server" request.  The server will be launched in the
 *  same bootstrap context in which it was registered.
 */
#include <AvailabilityMacros.h>
#include <mach/std_types.h>
#include <mach/message.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <stdbool.h>

__BEGIN_DECLS

#pragma GCC visibility push(default)

#define	BOOTSTRAP_MAX_NAME_LEN			128
#define	BOOTSTRAP_MAX_CMD_LEN			512

typedef char name_t[BOOTSTRAP_MAX_NAME_LEN];
typedef char cmd_t[BOOTSTRAP_MAX_CMD_LEN];
typedef name_t *name_array_t;
typedef int bootstrap_status_t;
typedef bootstrap_status_t *bootstrap_status_array_t;
typedef unsigned int bootstrap_property_t;
typedef bootstrap_property_t * bootstrap_property_array_t;

typedef boolean_t *bool_array_t;

#define	BOOTSTRAP_MAX_LOOKUP_COUNT		20

#define	BOOTSTRAP_SUCCESS				0
#define	BOOTSTRAP_NOT_PRIVILEGED		1100
#define	BOOTSTRAP_NAME_IN_USE			1101
#define	BOOTSTRAP_UNKNOWN_SERVICE		1102
#define	BOOTSTRAP_SERVICE_ACTIVE		1103
#define	BOOTSTRAP_BAD_COUNT				1104
#define	BOOTSTRAP_NO_MEMORY				1105
#define BOOTSTRAP_NO_CHILDREN			1106

#define BOOTSTRAP_STATUS_INACTIVE		0
#define BOOTSTRAP_STATUS_ACTIVE			1
#define BOOTSTRAP_STATUS_ON_DEMAND		2

/*
 * After main() starts, it is safe to assume that this variable is always set.
 */
extern mach_port_t bootstrap_port;

/*
 * bootstrap_create_server()
 *
 * Declares a server that mach_init will re-spawn within the specified
 * bootstrap context.  The server is considered already "active"
 * (i.e. will not be re-spawned) until the returned server_port is
 * deallocated.
 *
 * In the meantime, services can be declared against the server,
 * by using the server_port as the privileged bootstrap target of
 * subsequent bootstrap_create_service() calls.
 *
 * When mach_init re-spawns the server, its task bootstrap port
 * is set to the privileged sever_port.  Through this special
 * bootstrap port, it can access all of parent bootstrap's context
 * (and all services are created in the parent's namespace). But
 * all additional service declarations (and declaration removals)
 * will be associated with this particular server.
 *
 * Only a holder of the server_port privilege bootstrap port can
 * check in or register over those services.  
 *
 * When all services associated with a server are deleted, and the server
 * exits, it will automatically be deleted itself.
 *
 * If the server is declared "on_demand," then a non-running server
 * will be re-launched on first use of one of the service ports
 * registered against it.  Otherwise, it will be re-launched
 * immediately upon exiting (whether any client is actively using
 * any of the service ports or not).
 *
 * Errors:	Returns appropriate kernel errors on rpc failure.
 *		Returns BOOTSTRAP_NOT_PRIVILEGED, bootstrap or uid invalid.
 */
kern_return_t bootstrap_create_server(
		mach_port_t bp,
		cmd_t server_cmd,
		uid_t server_uid,
		boolean_t on_demand,
		mach_port_t *server_port);

/*
 * bootstrap_subset()
 *
 * Returns a new port to use as a bootstrap port.  This port behaves
 * exactly like the previous bootstrap_port, except that ports dynamically
 * registered via bootstrap_register() are available only to users of this
 * specific subset_port.  Lookups on the subset_port will return ports
 * registered with this port specifically, and ports registered with
 * ancestors of this subset_port.  Duplications of services already
 * registered with an ancestor port may be registered with the subset port
 * are allowed.  Services already advertised may then be effectively removed
 * by registering PORT_NULL for the service.
 * When it is detected that the requestor_port is destroyed the subset
 * port and all services advertized by it are destroyed as well.
 *
 * Errors:	Returns appropriate kernel errors on rpc failure.
 */
kern_return_t bootstrap_subset(
		mach_port_t bp,
		mach_port_t requestor_port,
		mach_port_t *subset_port);

/*
 * bootstrap_unprivileged()
 *
 * Given a bootstrap port, return its unprivileged equivalent.  If
 * the port is already unprivileged, another reference to the same
 * port is returned.
 *
 * This is most often used by servers, which are launched with their
 * bootstrap port set to the privileged port for the server, to get
 * an unprivileged version of the same port for use by its unprivileged
 * children (or any offspring that it does not want to count as part
 * of the "server" for mach_init registration and re-launch purposes).
 *
 * Native launchd jobs are always started with an unprivileged port.
 */
kern_return_t bootstrap_unprivileged(
		mach_port_t bp,
		mach_port_t *unpriv_port)
		AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_5;

/*
 * bootstrap_parent()
 *
 * Given a bootstrap subset port, return the parent bootstrap port.
 * If the specified bootstrap port is already the root subset,
 * the same port will be returned. Much like "." and ".." are the same
 * in the file system name space for the root directory ("/").
 *
 * Errors:
 *	Returns BOOTSTRAP_NOT_PRIVILEGED if the caller is not running
 *	with an effective user id of root (as determined by the security
 *	token in the message trailer).
 */
kern_return_t bootstrap_parent(
		mach_port_t bp,
		mach_port_t *parent_port);

/*
 * bootstrap_register()
 *
 * Registers a send right for service_port with the service identified by
 * service_name.  Attempts to register a service where an active binding
 * already exists are rejected.
 *
 * If the service was previously declared with bootstrap_create_service(),
 * but is not currently active, this call can be used to undeclare the
 * service. The bootstrap port used must have sufficient privilege to
 * do so.  (Registering MACH_PORT_NULL is especially useful for shutting
 * down declared services).
 *
 * This API is deprecated. Old scenarios and recommendations:
 *
 * 1) Code that used to call bootstrap_check_in() and then bootstrap_register()
 *    can now always call bootstrap_check_in().
 *
 * 2) If the code was registering a well known name, please switch to launchd.
 *
 * 3) If the code was registering a dynamically generated string and passing
 *    the string to other applications, please rewrite the code to send a Mach
 *    send-right directly.
 *
 * 4) If the launchd job maintained an optional Mach service, please reserve
 *    the name with launchd and control the presense of the service through
 *    ownership of the Mach receive right like so.
 *
 *	<key>MachServices</key>
 *	<dict>
 *		<key>com.apple.windowserver</key>
 *		<true/>
 *		<key>com.apple.windowserver.active</key>
 *		<dict>
 *			<key>HideUntilCheckIn</key>
 *			<true/>
 *		</dict>
 *	</dict>
 *
 *
 * Errors:	Returns appropriate kernel errors on rpc failure.
 *		Returns BOOTSTRAP_NOT_PRIVILEGED, if request directed to
 *			bootstrap port without privilege.
 *		Returns BOOTSTRAP_NAME_IN_USE, if service has already been
 *			register or checked-in.
 */
AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_5
kern_return_t
bootstrap_register(mach_port_t bp, name_t service_name, mach_port_t sp);

/*
 * bootstrap_create_service()
 *
 * Creates a service named "service_name" and returns a send right to that
 * port in "service_port."  The port may later be checked in as if this
 * port were configured in the bootstrap configuration file.
 *
 * This API is deprecated. Please call bootstrap_check_in() instead.
 *
 * Errors:	Returns appropriate kernel errors on rpc failure.
 *		Returns BOOTSTRAP_SERVICE_ACTIVE, if service already exists.
 */
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6
AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6
#endif
kern_return_t
bootstrap_create_service(mach_port_t bp, name_t service_name, mach_port_t *sp);

/*
 * bootstrap_check_in()
 *
 * Returns the receive right for the service named by service_name. The
 * service must have been declared in the launchd.plist(5) file associated 
 * with the job.  Attempts to check_in a service which is already active 
 * are not allowed.
 *
 * If the service was declared as being associated with a server, the
 * check_in must come from the server's privileged port (server_port).
 *
 * Errors:	Returns appropriate kernel errors on rpc failure.
 *		Returns BOOTSTRAP_UNKNOWN_SERVICE, if service does not exist.
 *		Returns BOOTSTRAP_NOT_PRIVILEGED, if request directed to
 *			bootstrap port without privilege.
 *		Returns BOOTSTRAP_SERVICE_ACTIVE, if service has already been
 *			registered or checked-in.
 */
kern_return_t bootstrap_check_in(
		mach_port_t bp,
		const name_t service_name,
		mach_port_t *sp);

/*
 * bootstrap_look_up()
 *
 * Returns a send right for the service port declared/registered under the
 * name service_name. The service is not guaranteed to be active.  Use the
 * bootstrap_status call to determine the status of the service.
 *
 * Errors:	Returns appropriate kernel errors on rpc failure.
 *		Returns BOOTSTRAP_UNKNOWN_SERVICE, if service does not exist.
 */
kern_return_t bootstrap_look_up(
		mach_port_t bp,
		const name_t service_name,
		mach_port_t *sp);

/*
 * bootstrap_status()
 *
 * In practice, this call was used to preflight whether the following two
 * APIs would succeed.
 *
 * bootstrap_look_up()
 * bootstrap_check_in()
 *
 * Please don't bother. Just call the above two APIs directly and check
 * for failure.
 */
kern_return_t bootstrap_status(
		mach_port_t bp,
		name_t service_name,
		bootstrap_status_t *service_active)
		AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_5;

/* bootstrap_strerror()
 *
 * Translate a return value from the bootstrap_*() APIs to a string.
 */
const char *bootstrap_strerror(kern_return_t r) __attribute__((__nothrow__, __pure__, __warn_unused_result__));

#pragma GCC visibility pop

__END_DECLS

#endif /* __BOOTSTRAP_H__ */
