#include "getuid.h"
#include "../base.h"
#include <stddef.h>
#include <darlingserver/rpc.h>
#include "../simple.h"
#include <libsimple/lock.h>
#include "../errno.h"

// the UID and GID related calls are called frequently, so making a server call for them each time is expensive.
// instead, since the server never changes our UID and GID on its own (we're always the ones that tell it to change it),
// we can just cache the UID and GID and only inform the server when we want to change them.

static libsimple_rwlock_t uidgid_rwlock = LIBSIMPLE_RWLOCK_INITIALIZER;
static int32_t stored_uid = -1;
static int32_t stored_gid = -1;

void __getuidgid(int32_t* uid, int32_t* gid) {
	int32_t tmp_uid = -1;
	int32_t tmp_gid = -1;

	libsimple_rwlock_lock_read(&uidgid_rwlock);
	tmp_uid = stored_uid;
	tmp_gid = stored_gid;
	libsimple_rwlock_unlock_read(&uidgid_rwlock);

	// if they're uninitialized, initialize them
	if (tmp_uid == -1 && tmp_gid == -1) {
		libsimple_rwlock_lock_write(&uidgid_rwlock);
		// check again if they're still uninitialized
		if (stored_uid == -1 && stored_gid == -1) {
			// fetch them from the server

			// NOTE: there's no possibility of deadlock here. the server needs the task lock to read/write
			//       the UID and GID, but the task lock is only held while in a server call (i.e. it doesn't
			//       remain locked after returning from a syscall). since the server doesn't even know about
			//       the `uidgid_rwlock` here, there's no chance of someone holding the task lock in the
			//       server while wanting the `uidgid_lock`.
			if (dserver_rpc_uidgid(-1, -1, &stored_uid, &stored_gid) < 0) {
				__simple_abort();
			}
		}
		tmp_uid = stored_uid;
		tmp_gid = stored_gid;
		libsimple_rwlock_unlock_write(&uidgid_rwlock);
	}

	if (uid) {
		*uid = tmp_uid;
	}
	if (gid) {
		*gid = tmp_gid;
	}
};

long __setuidgid(int32_t uid, int32_t gid) {
	int ret = 0;

	libsimple_rwlock_lock_write(&uidgid_rwlock);
	// see note about impossibility of deadlocks in __getuidgid
	ret = dserver_rpc_uidgid(uid, gid, NULL, NULL);
	if (ret == 0) {
		if (uid >= 0) {
			stored_uid = uid;
		}
		if (gid >= 0) {
			stored_gid = gid;
		}
	} else {
		ret = errno_linux_to_bsd(ret);
	}
	libsimple_rwlock_unlock_write(&uidgid_rwlock);

	return ret;
};

long sys_getuid(void)
{
	int32_t uid;
	__getuidgid(&uid, NULL);
	return uid;
}

