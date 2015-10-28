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

#include "config.h"
#include "launch.h"
#include "launch_priv.h"
#include "bootstrap.h"
#include "bootstrap_priv.h"
#include "vproc.h"
#include "vproc_priv.h"

#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>

#include "job.h"

void
bootstrap_init(void)
{
	kern_return_t kr = task_get_special_port(task_self_trap(), TASK_BOOTSTRAP_PORT, &bootstrap_port);
	if (kr != KERN_SUCCESS) {
		abort();
	}
}

kern_return_t
bootstrap_create_server(mach_port_t bp, cmd_t server_cmd, uid_t server_uid, boolean_t on_demand, mach_port_t *server_port)
{
	kern_return_t kr;

	kr = vproc_mig_create_server(bp, server_cmd, server_uid, on_demand, server_port);

	if (kr == VPROC_ERR_TRY_PER_USER) {
		mach_port_t puc;

		if (vproc_mig_lookup_per_user_context(bp, 0, &puc) == 0) {
			kr = vproc_mig_create_server(puc, server_cmd, server_uid, on_demand, server_port);
			mach_port_deallocate(mach_task_self(), puc);
		}
	}

	return kr;
}

kern_return_t
bootstrap_subset(mach_port_t bp, mach_port_t requestor_port, mach_port_t *subset_port)
{
	return vproc_mig_subset(bp, requestor_port, subset_port);
}

kern_return_t
bootstrap_unprivileged(mach_port_t bp, mach_port_t *unpriv_port)
{
	kern_return_t kr;

	*unpriv_port = MACH_PORT_NULL;

	kr = mach_port_mod_refs(mach_task_self(), bp, MACH_PORT_RIGHT_SEND, 1);

	if (kr == KERN_SUCCESS) {
		*unpriv_port = bp;
	}

	return kr;
}

kern_return_t
bootstrap_parent(mach_port_t bp, mach_port_t *parent_port)
{
	return vproc_mig_parent(bp, parent_port);
}

kern_return_t
bootstrap_register(mach_port_t bp, name_t service_name, mach_port_t sp)
{
	return bootstrap_register2(bp, service_name, sp, 0);
}

kern_return_t
bootstrap_register2(mach_port_t bp, name_t service_name, mach_port_t sp, uint64_t flags)
{
	kern_return_t kr = vproc_mig_register2(bp, service_name, sp, flags);

	if (kr == VPROC_ERR_TRY_PER_USER) {
		mach_port_t puc;

		if (vproc_mig_lookup_per_user_context(bp, 0, &puc) == 0) {
			kr = vproc_mig_register2(puc, service_name, sp, flags);
			mach_port_deallocate(mach_task_self(), puc);
		}
	}

	return kr;
}

kern_return_t
bootstrap_create_service(mach_port_t bp, name_t service_name, mach_port_t *sp)
{
	kern_return_t kr;

	if ((kr = bootstrap_check_in(bp, service_name, sp))) {
		return kr;
	}

	if ((kr = mach_port_mod_refs(mach_task_self(), *sp, MACH_PORT_RIGHT_RECEIVE, -1))) {
		return kr;
	}

	return bootstrap_look_up(bp, service_name, sp);
}

kern_return_t
bootstrap_check_in(mach_port_t bp, const name_t service_name, mach_port_t *sp)
{
	uuid_t junk;
	(void)bzero(junk, sizeof(junk));
	return vproc_mig_check_in2(bp, (char *)service_name, sp, junk, 0);
}

kern_return_t
bootstrap_check_in2(mach_port_t bp, const name_t service_name, mach_port_t *sp, uint64_t flags)
{
	uuid_t junk;
	(void)bzero(junk, sizeof(junk));
	return vproc_mig_check_in2(bp, (char *)service_name, sp, junk, flags);
}

kern_return_t
bootstrap_look_up_per_user(mach_port_t bp, const name_t service_name, uid_t target_user, mach_port_t *sp)
{
	audit_token_t au_tok;
	kern_return_t kr;
	mach_port_t puc;

	/* See rdar://problem/4890134. */

	if ((kr = vproc_mig_lookup_per_user_context(bp, target_user, &puc)) != 0) {
		return kr;
	}

	if (!service_name) {
		*sp = puc;
	} else {
		uuid_t junk;
		kr = vproc_mig_look_up2(puc, (char *)service_name, sp, &au_tok, 0, junk, 0);
		mach_port_deallocate(mach_task_self(), puc);
	}

	return kr;
}

kern_return_t
bootstrap_lookup_children(mach_port_t bp, mach_port_array_t *children, name_array_t *names, bootstrap_property_array_t *properties, mach_msg_type_number_t *n_children)
{
	mach_msg_type_number_t junk = 0;
	return vproc_mig_lookup_children(bp, children, &junk, names, n_children, properties, &junk);
}

kern_return_t
bootstrap_look_up(mach_port_t bp, const name_t service_name, mach_port_t *sp)
{
	return bootstrap_look_up2(bp, service_name, sp, 0, 0);
}

kern_return_t
bootstrap_look_up2(mach_port_t bp, const name_t service_name, mach_port_t *sp, pid_t target_pid, uint64_t flags)
{
	uuid_t instance_id;
	return bootstrap_look_up3(bp, service_name, sp, target_pid, instance_id, flags);
}

kern_return_t
bootstrap_look_up3(mach_port_t bp, const name_t service_name, mach_port_t *sp, pid_t target_pid, const uuid_t instance_id, uint64_t flags)
{
	audit_token_t au_tok;
	bool privileged_server_lookup = flags & BOOTSTRAP_PRIVILEGED_SERVER;
	kern_return_t kr = 0;
	mach_port_t puc;

	// We have to cast instance_id here because the MIG-generated method
	// doesn't expect a const parameter.
	if ((kr = vproc_mig_look_up2(bp, (char *)service_name, sp, &au_tok, target_pid, (unsigned char*)instance_id, flags)) != VPROC_ERR_TRY_PER_USER) {
		goto out;
	}

	if ((kr = vproc_mig_lookup_per_user_context(bp, 0, &puc)) != 0) {
		goto out;
	}

	kr = vproc_mig_look_up2(puc, (char *)service_name, sp, &au_tok, target_pid, (unsigned char*)instance_id, flags);
	mach_port_deallocate(mach_task_self(), puc);

out:
	if ((kr == 0) && privileged_server_lookup) {
		uid_t server_euid;

		/*
		 * The audit token magic is dependent on the per-user launchd
		 * forwarding MIG requests to the root launchd when it cannot
		 * find the answer locally.
		 */

		/* This API should be in Libsystem, but is not */
		//audit_token_to_au32(au_tok, NULL, &server_euid, NULL, NULL, NULL, NULL, NULL, NULL);

		server_euid = au_tok.val[1];

		if (server_euid) {
			mach_port_deallocate(mach_task_self(), *sp);
			*sp = MACH_PORT_NULL;
			kr = BOOTSTRAP_NOT_PRIVILEGED;
		}
	}

	return kr;
}

kern_return_t
bootstrap_check_in3(mach_port_t bp, const name_t service_name, mach_port_t *sp, uuid_t instance_id, uint64_t flags)
{
	return vproc_mig_check_in2(bp, (char *)service_name, sp, instance_id, flags);
}

kern_return_t
bootstrap_get_root(mach_port_t bp, mach_port_t *root)
{
	return vproc_mig_get_root_bootstrap(bp, root);
}

kern_return_t
bootstrap_status(mach_port_t bp, name_t service_name, bootstrap_status_t *service_active)
{
	kern_return_t kr;
	mach_port_t p;

	if ((kr = bootstrap_look_up(bp, service_name, &p))) {
		return kr;
	}

	mach_port_deallocate(mach_task_self(), p);
	*service_active = BOOTSTRAP_STATUS_ACTIVE;

	if (bootstrap_check_in(bp, service_name, &p) == BOOTSTRAP_SUCCESS) {
		mach_port_mod_refs(mach_task_self(), p, MACH_PORT_RIGHT_RECEIVE, -1);
		*service_active = BOOTSTRAP_STATUS_ON_DEMAND;
	}

	return BOOTSTRAP_SUCCESS;
}

kern_return_t
bootstrap_info(mach_port_t bp,
			   name_array_t *service_names, mach_msg_type_number_t *service_namesCnt,
			   name_array_t *service_jobs, mach_msg_type_number_t *service_jobsCnt,
			   bootstrap_status_array_t *service_active, mach_msg_type_number_t *service_activeCnt,
			   uint64_t flags)
{
	return vproc_mig_info(bp, service_names, service_namesCnt, service_jobs, service_jobsCnt, service_active, service_activeCnt, flags);
}

const char *
bootstrap_strerror(kern_return_t r)
{
	switch (r) {
	case BOOTSTRAP_SUCCESS:
		return "Success";
	case BOOTSTRAP_NOT_PRIVILEGED:
		return "Permission denied";
	case BOOTSTRAP_NAME_IN_USE:
	case BOOTSTRAP_SERVICE_ACTIVE:
		return "Service name already exists";
	case BOOTSTRAP_UNKNOWN_SERVICE:
		return "Unknown service name";
	case BOOTSTRAP_BAD_COUNT:
		return "Too many lookups were requested in one request";
	case BOOTSTRAP_NO_MEMORY:
		return "Out of memory";
	default:
		return mach_error_string(r);
	}
}
