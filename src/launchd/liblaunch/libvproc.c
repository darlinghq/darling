/*
 * Copyright (c) 1999-2012 Apple Inc. All rights reserved.
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
#include "vproc.h"
#include "vproc_priv.h"
#include "vproc_internal.h"

#include <dispatch/dispatch.h>
#include <libproc.h>
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <libkern/OSAtomic.h>
#include <sys/syscall.h>
#include <sys/event.h>
#include <System/sys/fileport.h>
#include <os/assumes.h>

#if HAVE_QUARANTINE
#include <quarantine.h>
#endif

#include "launch.h"
#include "launch_priv.h"
#include "launch_internal.h"
#include "ktrace.h"

#include "job.h"

#include "helper.h"
#include "helperServer.h"

#include "reboot2.h"

#define likely(x) __builtin_expect((bool)(x), true)
#define unlikely(x) __builtin_expect((bool)(x), false)

#define _vproc_set_crash_log_message(x)

void _vproc_transactions_enable_internal(void *arg);
void _vproc_transaction_begin_internal(void *arg __unused);
void _vproc_transaction_end_internal(void *arg __unused);

#pragma mark vproc Object
struct vproc_s {
	int32_t refcount;
	mach_port_t j_port;
};

vproc_t
vprocmgr_lookup_vproc(const char *label)
{
	struct vproc_s *vp = NULL;

	mach_port_t mp = MACH_PORT_NULL;
	kern_return_t kr = vproc_mig_port_for_label(bootstrap_port, (char *)label, &mp);
	if (kr == BOOTSTRAP_SUCCESS) {
		vp = (struct vproc_s *)calloc(1, sizeof(struct vproc_s));
		if (vp) {
			vp->refcount = 1;
			mach_port_mod_refs(mach_task_self(), mp, MACH_PORT_RIGHT_SEND, 1);
			vp->j_port = mp;
		}
		(void)mach_port_deallocate(mach_task_self(), mp);
	}

	return vp;
}

vproc_t
vproc_retain(vproc_t vp)
{
	int32_t orig = OSAtomicAdd32(1, &vp->refcount) - 1;	
	if (orig <= 0) {
		_vproc_set_crash_log_message("Under-retain / over-release of vproc_t.");
		abort();
	}

	return vp;
}

void
vproc_release(vproc_t vp)
{
	int32_t newval = OSAtomicAdd32(-1, &vp->refcount);
	if (newval < 0) {
		_vproc_set_crash_log_message("Over-release of vproc_t.");
		abort();
	} else if (newval == 0) {
		mach_port_deallocate(mach_task_self(), vp->j_port);
		free(vp);
	}
}

#pragma mark Transactions
static void
_vproc_transaction_init_once(void *arg __unused)
{
	launch_globals_t globals = _launch_globals();

	int64_t enable_transactions = 0;
	(void)vproc_swap_integer(NULL, VPROC_GSK_TRANSACTIONS_ENABLED, 0, &enable_transactions);
	if (enable_transactions != 0) {
		(void)os_assumes_zero(proc_track_dirty(getpid(), PROC_DIRTY_TRACK));
		globals->_vproc_transaction_enabled = 1;
	}
	globals->_vproc_transaction_queue = dispatch_queue_create("com.apple.idle-exit-queue", NULL);
}

void
_vproc_transactions_enable_internal(void *arg __unused)
{
	launch_globals_t globals = _launch_globals();

	if (!globals->_vproc_transaction_enabled) {
		(void)os_assumes_zero(proc_track_dirty(getpid(), PROC_DIRTY_TRACK));
		globals->_vproc_transaction_enabled = 1;
	}

	if (globals->_vproc_transaction_cnt > 0) {
		(void)os_assumes_zero(proc_set_dirty(getpid(), true));
	}
}

void
_vproc_transactions_enable(void)
{
	launch_globals_t globals = _launch_globals();

	dispatch_once_f(&globals->_vproc_transaction_once, NULL, _vproc_transaction_init_once);
	dispatch_sync_f(globals->_vproc_transaction_queue, NULL, _vproc_transactions_enable_internal);
}

void
_vproc_transaction_begin_internal(void *ctx __unused)
{
	launch_globals_t globals = _launch_globals();

	int64_t new = ++globals->_vproc_transaction_cnt;
	if (!globals->_vproc_transaction_enabled || new > 1) {
		return;
	}

	if (new < 1) {
		_vproc_set_crash_log_message("Underflow of transaction count.");
		abort();
	}

	(void)os_assumes_zero(proc_set_dirty(getpid(), true));
}

void
_vproc_transaction_begin(void)
{
	launch_globals_t globals = _launch_globals();

	dispatch_once_f(&globals->_vproc_transaction_once, NULL, _vproc_transaction_init_once);
	dispatch_sync_f(globals->_vproc_transaction_queue, NULL, _vproc_transaction_begin_internal);
}

vproc_transaction_t
vproc_transaction_begin(vproc_t vp __unused)
{
	_vproc_transaction_begin();

	/* Return non-NULL on success. Originally, there were dreams of returning
	 * an object or something, but those never panned out.
	 */
	return (vproc_transaction_t)vproc_transaction_begin;
}

void _vproc_transaction_end_flush(void);

void
_vproc_transaction_end_internal2(void *ctx)
{
	launch_globals_t globals = _launch_globals();

	globals->_vproc_gone2zero_callout(ctx);
	_vproc_transaction_end_flush();
}

void
_vproc_transaction_end_internal(void *arg)
{
	launch_globals_t globals = _launch_globals();

	int64_t new = --globals->_vproc_transaction_cnt;
	if (!globals->_vproc_transaction_enabled || new > 0) {
		return;
	}

	if (new < 0) {
		_vproc_set_crash_log_message("Underflow of transaction count.");
		abort();
	}

	if (globals->_vproc_gone2zero_callout && !arg) {
		globals->_vproc_transaction_cnt = 1;
		dispatch_async_f(globals->_vproc_gone2zero_queue, globals->_vproc_gone2zero_ctx, _vproc_transaction_end_internal2);
	} else {
		(void)os_assumes_zero(proc_set_dirty(getpid(), false));
	}
}

void
_vproc_transaction_end_flush2(void *ctx __unused)
{
	_vproc_transaction_end_internal((void *)1);
}

void
_vproc_transaction_end_flush(void)
{
	launch_globals_t globals = _launch_globals();

	dispatch_sync_f(globals->_vproc_transaction_queue, NULL, _vproc_transaction_end_flush2);
}

void
_vproc_transaction_end(void)
{
	launch_globals_t globals = _launch_globals();

	dispatch_once_f(&globals->_vproc_transaction_once, NULL, _vproc_transaction_init_once);
	dispatch_sync_f(globals->_vproc_transaction_queue, NULL, _vproc_transaction_end_internal);
}

void
vproc_transaction_end(vproc_t vp __unused, vproc_transaction_t vpt __unused)
{
	_vproc_transaction_end();
}

size_t
_vproc_transaction_count(void)
{
	launch_globals_t globals = _launch_globals();

	return globals->_vproc_transaction_cnt;
}

size_t
_vproc_standby_count(void)
{
	return 0;
} 

size_t
_vproc_standby_timeout(void)
{
	return 0;
}

bool
_vproc_pid_is_managed(pid_t p)
{
	boolean_t result = false;
	vproc_mig_pid_is_managed(bootstrap_port, p, &result);

	return result;
}

kern_return_t
_vproc_transaction_count_for_pid(pid_t p, int32_t *count, bool *condemned)
{
	/* Activity Monitor relies on us returning this error code when the process
	 * is not opted into Instant Off.
	 */
	kern_return_t error = BOOTSTRAP_NO_MEMORY;

	if (condemned) {
		*condemned = false;
	}

	if (count) {
		uint32_t flags;
		int ret = proc_get_dirty(p, &flags);
		if (ret == 0) {
			if (flags & PROC_DIRTY_TRACKED) {
				*count = (flags & PROC_DIRTY_IS_DIRTY) ? 1 : 0;
				error = BOOTSTRAP_SUCCESS;
			} else {
				error = BOOTSTRAP_NO_MEMORY;
			}
		} else if (ret == ENOTSUP) {
			error = BOOTSTRAP_NO_MEMORY;
		} else if (ret == ESRCH) {
			error = BOOTSTRAP_UNKNOWN_SERVICE;
		} else if (ret == EPERM) {
			error = BOOTSTRAP_NOT_PRIVILEGED;
		} else {
			error = ret;
		}
	}
	return error;
}
void
_vproc_transaction_try_exit(int status)
{
#if !TARGET_OS_EMBEDDED
	launch_globals_t globals = _launch_globals();
	if (globals->_vproc_transaction_cnt == 0) {
		_exit(status);
	}
#else
	_exit(status);
#endif
}

void
_vproc_standby_begin(void)
{

}

vproc_standby_t
vproc_standby_begin(vproc_t vp __unused)
{
	return (vproc_standby_t)vproc_standby_begin;
}

void
_vproc_standby_end(void)
{

}

void
_vproc_transaction_set_clean_callback(dispatch_queue_t targetq, void *ctx, dispatch_function_t func)
{
	launch_globals_t globals = _launch_globals();

	globals->_vproc_gone2zero_queue = targetq;
	dispatch_retain(targetq);

	globals->_vproc_gone2zero_callout = func;
	globals->_vproc_gone2zero_ctx = ctx;
}

void
vproc_standby_end(vproc_t vp __unused, vproc_standby_t vpt __unused)
{

}

#pragma mark Miscellaneous SPI
kern_return_t
_vproc_grab_subset(mach_port_t bp, mach_port_t *reqport, mach_port_t *rcvright,
	launch_data_t *outval, mach_port_array_t *ports, 
	mach_msg_type_number_t *portCnt)
{
	mach_msg_type_number_t outdata_cnt;
	vm_offset_t outdata = 0;
	size_t data_offset = 0;
	launch_data_t out_obj;
	kern_return_t kr;

	if ((kr = vproc_mig_take_subset(bp, reqport, rcvright, &outdata, &outdata_cnt, ports, portCnt))) {
		goto out;
	}

	if ((out_obj = launch_data_unpack((void *)outdata, outdata_cnt, NULL, 0, &data_offset, NULL))) {
		*outval = launch_data_copy(out_obj);
	} else {
		kr = 1;
	}

out:
	if (outdata) {
		mig_deallocate(outdata, outdata_cnt);
	}

	return kr;
}

vproc_err_t
_vprocmgr_move_subset_to_user(uid_t target_user, const char *session_type, uint64_t flags)
{
	kern_return_t kr = 0;
	bool is_bkgd = (strcmp(session_type, VPROCMGR_SESSION_BACKGROUND) == 0);
	int64_t ldpid, lduid;

	if (vproc_swap_integer(NULL, VPROC_GSK_MGR_PID, 0, &ldpid) != 0) {
		return (vproc_err_t)_vprocmgr_move_subset_to_user;
	}

	if (vproc_swap_integer(NULL, VPROC_GSK_MGR_UID, 0, &lduid) != 0) {
		return (vproc_err_t)_vprocmgr_move_subset_to_user;
	}

	if (!is_bkgd && ldpid != 1) {
		if (lduid == getuid()) {
			return NULL;
		}
		/*
		 * Not all sessions can be moved.
		 * We should clean up this mess someday.
		 */
		return (vproc_err_t)_vprocmgr_move_subset_to_user;
	}

	mach_port_t puc = 0;
	mach_port_t rootbs = MACH_PORT_NULL;
	(void)bootstrap_get_root(bootstrap_port, &rootbs);

	if (vproc_mig_lookup_per_user_context(rootbs, target_user, &puc) != 0) {
		return (vproc_err_t)_vprocmgr_move_subset_to_user;
	}

	if (is_bkgd) {		
		task_set_bootstrap_port(mach_task_self(), puc);
		mach_port_deallocate(mach_task_self(), bootstrap_port);
		bootstrap_port = puc;
	} else {
		kr = vproc_mig_move_subset(puc, bootstrap_port, (char *)session_type, _audit_session_self(), flags);
		mach_port_deallocate(mach_task_self(), puc);
	}

	if (kr) {
		return (vproc_err_t)_vprocmgr_move_subset_to_user;
	}

	return _vproc_post_fork_ping();
}

vproc_err_t
_vprocmgr_switch_to_session(const char *target_session, vproc_flags_t flags __attribute__((unused)))
{
	mach_port_t new_bsport = MACH_PORT_NULL;
	kern_return_t kr = KERN_FAILURE;

	mach_port_t tnp = MACH_PORT_NULL;
	task_name_for_pid(mach_task_self(), getpid(), &tnp);
	if ((kr = vproc_mig_switch_to_session(bootstrap_port, tnp, (char *)target_session, _audit_session_self(), &new_bsport)) != KERN_SUCCESS) {
		_vproc_log(LOG_NOTICE, "_vprocmgr_switch_to_session(): kr = 0x%x", kr);
		return (vproc_err_t)_vprocmgr_switch_to_session;
	}

	task_set_bootstrap_port(mach_task_self(), new_bsport);
	mach_port_deallocate(mach_task_self(), bootstrap_port);
	bootstrap_port = new_bsport;

	return !issetugid() ? _vproc_post_fork_ping() : NULL;
}

vproc_err_t 
_vprocmgr_detach_from_console(vproc_flags_t flags __attribute__((unused)))
{
	return _vprocmgr_switch_to_session(VPROCMGR_SESSION_BACKGROUND, 0);
}

vproc_err_t
_vproc_post_fork_ping(void)
{
	mach_port_t session = MACH_PORT_NULL;
	kern_return_t kr = vproc_mig_post_fork_ping(bootstrap_port, mach_task_self(), &session);
	if (kr) {
		return _vproc_post_fork_ping;
	}

	if (session) {
		(void)_audit_session_join(session);
		(void)mach_port_deallocate(mach_task_self(), session);
	}

	return NULL;
}

vproc_err_t
_vprocmgr_init(const char *session_type)
{
	if (vproc_mig_init_session(bootstrap_port, (char *)session_type, _audit_session_self()) == 0) {
		return NULL;
	}

	return (vproc_err_t)_vprocmgr_init;
}

pid_t
_spawn_via_launchd(const char *label, const char *const *argv, const struct spawn_via_launchd_attr *spawn_attrs, int struct_version)
{
	size_t i, good_enough_size = 10*1024*1024;
	mach_msg_type_number_t indata_cnt = 0;
	vm_offset_t indata = 0;
	mach_port_t obsvr_port = MACH_PORT_NULL;
	launch_data_t tmp, tmp_array, in_obj;
	const char *const *tmpp;
	kern_return_t kr = 1;
	void *buf = NULL;
	pid_t p = -1;

	if ((in_obj = launch_data_alloc(LAUNCH_DATA_DICTIONARY)) == NULL) {
		goto out;
	}

	if ((tmp = launch_data_new_string(label)) == NULL) {
		goto out;
	}

	launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_LABEL);

	if ((tmp_array = launch_data_alloc(LAUNCH_DATA_ARRAY)) == NULL) {
		goto out;
	}

	for (i = 0; *argv; i++, argv++) {
		tmp = launch_data_new_string(*argv);
		if (tmp == NULL) {
			goto out;
		}

		launch_data_array_set_index(tmp_array, tmp, i);
	}

	launch_data_dict_insert(in_obj, tmp_array, LAUNCH_JOBKEY_PROGRAMARGUMENTS);

	if (spawn_attrs) switch (struct_version) {
	case 3:
	case 2:
#if HAVE_QUARANTINE
		if (spawn_attrs->spawn_quarantine) {
			char qbuf[QTN_SERIALIZED_DATA_MAX];
			size_t qbuf_sz = QTN_SERIALIZED_DATA_MAX;

			if (qtn_proc_to_data(spawn_attrs->spawn_quarantine, qbuf, &qbuf_sz) == 0) {
				tmp = launch_data_new_opaque(qbuf, qbuf_sz);
				launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_QUARANTINEDATA);
			}
		}
#endif

		if (spawn_attrs->spawn_seatbelt_profile) {
			tmp = launch_data_new_string(spawn_attrs->spawn_seatbelt_profile);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_SANDBOXPROFILE);
		}

		if (spawn_attrs->spawn_seatbelt_flags) {
			tmp = launch_data_new_integer(*spawn_attrs->spawn_seatbelt_flags);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_SANDBOXFLAGS);
		}

		/* fall through */
	case 1:
		if (spawn_attrs->spawn_binpref) {
			tmp_array = launch_data_alloc(LAUNCH_DATA_ARRAY);
			for (i = 0; i < spawn_attrs->spawn_binpref_cnt; i++) {
				tmp = launch_data_new_integer(spawn_attrs->spawn_binpref[i]);
				launch_data_array_set_index(tmp_array, tmp, i);
			}
			launch_data_dict_insert(in_obj, tmp_array, LAUNCH_JOBKEY_BINARYORDERPREFERENCE);
		}
		/* fall through */
	case 0:
		if (spawn_attrs->spawn_flags & SPAWN_VIA_LAUNCHD_STOPPED) {
			tmp = launch_data_new_bool(true);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_WAITFORDEBUGGER);
		}
		if (spawn_attrs->spawn_flags & SPAWN_VIA_LAUNCHD_TALAPP) {
			tmp = launch_data_new_string(LAUNCH_KEY_POSIXSPAWNTYPE_TALAPP);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_POSIXSPAWNTYPE);
		}
		if (spawn_attrs->spawn_flags & SPAWN_VIA_LAUNCHD_DISABLE_ASLR) {
			tmp = launch_data_new_bool(true);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_DISABLEASLR);
		}

		if (spawn_attrs->spawn_env) {
			launch_data_t tmp_dict = launch_data_alloc(LAUNCH_DATA_DICTIONARY);

			for (tmpp = spawn_attrs->spawn_env; *tmpp; tmpp++) {
				char *eqoff, tmpstr[strlen(*tmpp) + 1];

				strcpy(tmpstr, *tmpp);

				eqoff = strchr(tmpstr, '=');

				if (!eqoff) {
					goto out;
				}

				*eqoff = '\0';

				launch_data_dict_insert(tmp_dict, launch_data_new_string(eqoff + 1), tmpstr);
			}

			launch_data_dict_insert(in_obj, tmp_dict, LAUNCH_JOBKEY_ENVIRONMENTVARIABLES);
		}

		if (spawn_attrs->spawn_path) {
			tmp = launch_data_new_string(spawn_attrs->spawn_path);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_PROGRAM);
		}

		if (spawn_attrs->spawn_chdir) {
			tmp = launch_data_new_string(spawn_attrs->spawn_chdir);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_WORKINGDIRECTORY);
		}

		if (spawn_attrs->spawn_umask) {
			tmp = launch_data_new_integer(*spawn_attrs->spawn_umask);
			launch_data_dict_insert(in_obj, tmp, LAUNCH_JOBKEY_UMASK);
		}

		break;
	default:
		break;
	}

	if (!(buf = malloc(good_enough_size))) {
		goto out;
	}

	if ((indata_cnt = launch_data_pack(in_obj, buf, good_enough_size, NULL, NULL)) == 0) {
		goto out;
	}

	indata = (vm_offset_t)buf;

	if (struct_version == 3) {
		kr = vproc_mig_spawn2(bootstrap_port, indata, indata_cnt, _audit_session_self(), &p, &obsvr_port); 
	} else {
		_vproc_set_crash_log_message("Bogus version passed to _spawn_via_launchd(). For this release, the only valid version is 3.");
	}

	if (kr == VPROC_ERR_TRY_PER_USER) {
		mach_port_t puc;

		if (vproc_mig_lookup_per_user_context(bootstrap_port, 0, &puc) == 0) {
			if (struct_version == 3) {
				kr = vproc_mig_spawn2(puc, indata, indata_cnt, _audit_session_self(), &p, &obsvr_port);
			}
			mach_port_deallocate(mach_task_self(), puc);
		}
	}

out:
	if (in_obj) {
		launch_data_free(in_obj);
	}

	if (buf) {
		free(buf);
	}

	switch (kr) {
	case BOOTSTRAP_SUCCESS:
		if (spawn_attrs && spawn_attrs->spawn_observer_port) {
			*spawn_attrs->spawn_observer_port = obsvr_port;
		} else {
			if (struct_version == 3) {
				mach_port_mod_refs(mach_task_self(), obsvr_port, MACH_PORT_RIGHT_RECEIVE, -1);
			} else {
				mach_port_deallocate(mach_task_self(), obsvr_port);
			}
		}
		return p;
	case BOOTSTRAP_NOT_PRIVILEGED:
		errno = EPERM; break;
	case BOOTSTRAP_NO_MEMORY:
		errno = ENOMEM; break;
	case BOOTSTRAP_NAME_IN_USE:
		errno = EEXIST; break;
	case 1:
		errno = EIO; break;
	default:
		errno = EINVAL; break;
	}

	return -1;
}

kern_return_t
mpm_wait(mach_port_t ajob __attribute__((unused)), int *wstatus)
{
	*wstatus = 0;
	return 0;
}

kern_return_t
mpm_uncork_fork(mach_port_t ajob __attribute__((unused)))
{
	return KERN_FAILURE;
}

kern_return_t
_vprocmgr_getsocket(name_t sockpath)
{
	return vproc_mig_getsocket(bootstrap_port, sockpath);
}

vproc_err_t
_vproc_get_last_exit_status(int *wstatus)
{
	int64_t val;

	if (vproc_swap_integer(NULL, VPROC_GSK_LAST_EXIT_STATUS, 0, &val) == 0) {
		*wstatus = (int)val;
		return NULL;
	}

	return (vproc_err_t)_vproc_get_last_exit_status;
}

vproc_err_t
_vproc_send_signal_by_label(const char *label, int sig)
{
	if (vproc_mig_send_signal(bootstrap_port, (char *)label, sig) == 0) {
		return NULL;
	}

	return _vproc_send_signal_by_label;
}

vproc_err_t
_vprocmgr_log_forward(mach_port_t mp, void *data, size_t len)
{
	if (vproc_mig_log_forward(mp, (vm_offset_t)data, len) == 0) {
		return NULL;
	}

	return _vprocmgr_log_forward;
}

vproc_err_t
_vprocmgr_log_drain(vproc_t vp __attribute__((unused)), pthread_mutex_t *mutex, _vprocmgr_log_drain_callback_t func)
{
	mach_msg_type_number_t outdata_cnt, tmp_cnt;
	vm_offset_t outdata = 0;
	struct timeval tv;
	struct logmsg_s *lm;

	if (!func) {
		return _vprocmgr_log_drain;
	}

	if (vproc_mig_log_drain(bootstrap_port, &outdata, &outdata_cnt) != 0) {
		return _vprocmgr_log_drain;
	}

	tmp_cnt = outdata_cnt;

	if (mutex) {
		pthread_mutex_lock(mutex);
	}

	for (lm = (struct logmsg_s *)outdata; tmp_cnt > 0; lm = ((void *)lm + lm->obj_sz)) {
		lm->from_name = (char *)lm + lm->from_name_offset;
		lm->about_name = (char *)lm + lm->about_name_offset;
		lm->msg = (char *)lm + lm->msg_offset;
		lm->session_name = (char *)lm + lm->session_name_offset;

		tv.tv_sec = lm->when / USEC_PER_SEC;
		tv.tv_usec = lm->when % USEC_PER_SEC;

		func(&tv, lm->from_pid, lm->about_pid, lm->sender_uid, lm->sender_gid, lm->pri,
				lm->from_name, lm->about_name, lm->session_name, lm->msg);

		tmp_cnt -= lm->obj_sz;
	}

	if (mutex) {
		pthread_mutex_unlock(mutex);
	}

	if (outdata) {
		mig_deallocate(outdata, outdata_cnt);
	}

	return NULL;
}

vproc_err_t
vproc_swap_integer(vproc_t vp, vproc_gsk_t key, int64_t *inval, int64_t *outval)
{
	kern_return_t kr = KERN_FAILURE;
	int64_t dummyval = 0;
	mach_port_t mp = vp ? vp->j_port : bootstrap_port;
	if ((kr = vproc_mig_swap_integer(mp, inval ? key : 0, outval ? key : 0, inval ? *inval : 0, outval ? outval : &dummyval)) == 0) {
		switch (key) {
		case VPROC_GSK_PERUSER_SUSPEND:
			if (dummyval) {
				/* Wait for the per-user launchd to exit before returning. */
				int kq = kqueue();
				struct kevent kev;
				EV_SET(&kev, dummyval, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, 0);
				int r = kevent(kq, &kev, 1, &kev, 1, NULL);
				(void)close(kq);
				if (r != 1) {
					return NULL;
				}
				break;
			}
		default:
			break;
		}
		return NULL;
	}

	return (vproc_err_t)vproc_swap_integer;
}

vproc_err_t
vproc_swap_complex(vproc_t vp, vproc_gsk_t key, launch_data_t inval, launch_data_t *outval)
{
	size_t data_offset = 0, good_enough_size = 10*1024*1024;
	mach_msg_type_number_t indata_cnt = 0, outdata_cnt;
	vm_offset_t indata = 0, outdata = 0;
	launch_data_t out_obj;
	void *rval = vproc_swap_complex;
	void *buf = NULL;

	if (inval) {
		if (!(buf = malloc(good_enough_size))) {
			goto out;
		}

		if ((indata_cnt = launch_data_pack(inval, buf, good_enough_size, NULL, NULL)) == 0) {
			goto out;
		}

		indata = (vm_offset_t)buf;
	}

	mach_port_t mp = vp ? vp->j_port : bootstrap_port;
	if (vproc_mig_swap_complex(mp, inval ? key : 0, outval ? key : 0, indata, indata_cnt, &outdata, &outdata_cnt) != 0) {
		goto out;
	}

	if (outval) {
		if (!(out_obj = launch_data_unpack((void *)outdata, outdata_cnt, NULL, 0, &data_offset, NULL))) {
			goto out;
		}

		if (!(*outval = launch_data_copy(out_obj))) {
			goto out;
		}
	}

	rval = NULL;
out:
	if (buf) {
		free(buf);
	}

	if (outdata) {
		mig_deallocate(outdata, outdata_cnt);
	}

	return rval;
}

vproc_err_t
vproc_swap_string(vproc_t vp, vproc_gsk_t key, const char *instr, char **outstr)
{
	launch_data_t instr_data = instr ? launch_data_new_string(instr) : NULL;
	launch_data_t outstr_data = NULL;

	vproc_err_t verr = vproc_swap_complex(vp, key, instr_data, &outstr_data);
	if (!verr && outstr) {
		if (launch_data_get_type(outstr_data) == LAUNCH_DATA_STRING) {
			*outstr = strdup(launch_data_get_string(outstr_data));
		} else {
			verr = (vproc_err_t)vproc_swap_string;
		}
		launch_data_free(outstr_data);
	}
	if (instr_data) {
		launch_data_free(instr_data);
	}

	return verr;
}

void *
reboot2(uint64_t flags)
{
	mach_port_t rootbs = MACH_PORT_NULL;
	(void)bootstrap_get_root(bootstrap_port, &rootbs);
	if (vproc_mig_reboot2(rootbs, flags) == 0) {
		(void)mach_port_deallocate(mach_task_self(), rootbs);
		return NULL;
	}

	return reboot2;
}

vproc_err_t
_vproc_kickstart_by_label(const char *label, pid_t *out_pid, 
	mach_port_t *out_port_name __unused, mach_port_t *out_obsrvr_port __unused,
	vproc_flags_t flags)
{
	/* Ignore the two port parameters. This SPI isn't long for this world, and
	 * all the current clients just leak them anyway.
	 */
	kern_return_t kr = vproc_mig_kickstart(bootstrap_port, (char *)label, out_pid, flags);
	if (kr == KERN_SUCCESS) {
		return NULL;
	}

	return (vproc_err_t)_vproc_kickstart_by_label;
}

vproc_err_t
_vproc_set_global_on_demand(bool state)
{
	int64_t val = state ? ~0 : 0;

	if (vproc_swap_integer(NULL, VPROC_GSK_GLOBAL_ON_DEMAND, &val, NULL) == 0) {
		return NULL;
	}

	return (vproc_err_t)_vproc_set_global_on_demand;
}

void
_vproc_logv(int pri, int err, const char *msg, va_list ap)
{
	char flat_msg[3000];

	vsnprintf(flat_msg, sizeof(flat_msg), msg, ap);

	vproc_mig_log(bootstrap_port, pri, err, flat_msg);
}

void
_vproc_log(int pri, const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	_vproc_logv(pri, 0, msg, ap);
	va_end(ap);
}

void
_vproc_log_error(int pri, const char *msg, ...)
{
	int saved_errno = errno;
	va_list ap;

	va_start(ap, msg);
	_vproc_logv(pri, saved_errno, msg, ap);
	va_end(ap);
}

/* The type naming convention is as follows:
 * For requests...
 *     union __RequestUnion__<userprefix><subsystem>_subsystem
 * For replies...
 *     union __ReplyUnion__<userprefix><subsystem>_subsystem
 */
union maxmsgsz {
	union __RequestUnion__helper_downcall_launchd_helper_subsystem req;
	union __ReplyUnion__helper_downcall_launchd_helper_subsystem rep;
};

const size_t vprocmgr_helper_maxmsgsz = sizeof(union maxmsgsz);

kern_return_t
helper_recv_wait(mach_port_t p, int status)
{
#if __LAUNCH_MACH_PORT_CONTEXT_T_DEFINED__
	mach_port_context_t ctx = status;
#else
	mach_vm_address_t ctx = status;
#endif

	return (errno = mach_port_set_context(mach_task_self(), p, ctx));
}

int
launch_wait(mach_port_t port)
{
	int status = -1;
	errno = mach_msg_server_once(launchd_helper_server, vprocmgr_helper_maxmsgsz, port, 0);
	if (errno == MACH_MSG_SUCCESS) {
#if __LAUNCH_MACH_PORT_CONTEXT_T_DEFINED__
		mach_port_context_t ctx = 0;
#else
		mach_vm_address_t ctx = 0;
#endif
		if ((errno = mach_port_get_context(mach_task_self(), port, &ctx)) == KERN_SUCCESS) {
			status = ctx;
		}
	}

	return status;
}

launch_data_t
launch_socket_service_check_in(void)
{
	launch_data_t reply = NULL;

	size_t big_enough = 10 * 1024;
	void *buff = malloc(big_enough);
	if (buff) {
		launch_data_t req = launch_data_new_string(LAUNCH_KEY_CHECKIN);
		if (req) {
			size_t sz = launch_data_pack(req, buff, big_enough, NULL, NULL);
			if (sz) {
				vm_address_t sreply = 0;
				mach_msg_size_t sreplyCnt = 0;
				mach_port_array_t fdps = NULL;
				mach_msg_size_t fdpsCnt = 0;
				kern_return_t kr = vproc_mig_legacy_ipc_request(bootstrap_port, (vm_address_t)buff, sz, NULL, 0, &sreply, &sreplyCnt, &fdps, &fdpsCnt, _audit_session_self());
				if (kr == BOOTSTRAP_SUCCESS) {
					int fds[128];

					size_t i = 0;
					size_t nfds = fdpsCnt / sizeof(fdps[0]);
					for (i = 0; i < nfds; i++) {
						fds[i] = fileport_makefd(fdps[i]);
						(void)mach_port_deallocate(mach_task_self(), fdps[i]);
					}

					size_t dataoff = 0;
					size_t fdoff = 0;
					reply = launch_data_unpack((void *)sreply, sreplyCnt, fds, nfds, &dataoff, &fdoff);
					reply = launch_data_copy(reply);

					mig_deallocate(sreply, sreplyCnt);
					mig_deallocate((vm_address_t)fdps, fdpsCnt);
				}
			}

			launch_data_free(req);
		}

		free(buff);
	}

	return reply;
}
