/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#include "internal.h"

#pragma mark Types
typedef struct _os_mach_port_disposition {
	mach_msg_bits_t ompd_bits;
	const char *const ompd_human;
} os_mach_port_disposition_t;

#define os_mach_port_disposition_init(d) [d] = { \
	.ompd_bits = (d), \
	.ompd_human = #d, \
}

#pragma mark Top-Level Statics
static const os_flagset_t _mach_msgh_bits = {
	os_flag_init(MACH_MSGH_BITS_COMPLEX),
	os_flag_init(MACH_MSGH_BITS_RAISEIMP),
	// MACH_MSGH_BITS_DENAP is deprecated
	os_flag_init(MACH_MSGH_BITS_IMPHOLDASRT),
	// MACH_MSGH_BITS_DENAPHOLDASRT is deprecated
	// MACH_MSGH_BITS_CIRCULAR is kernel-internal
};

static const os_mach_port_disposition_t _mach_port_dispositions[] = {
	os_mach_port_disposition_init(MACH_MSG_TYPE_MOVE_RECEIVE),
	os_mach_port_disposition_init(MACH_MSG_TYPE_MOVE_SEND),
	os_mach_port_disposition_init(MACH_MSG_TYPE_MOVE_SEND_ONCE),
	os_mach_port_disposition_init(MACH_MSG_TYPE_COPY_SEND),
	os_mach_port_disposition_init(MACH_MSG_TYPE_MAKE_SEND),
	os_mach_port_disposition_init(MACH_MSG_TYPE_MAKE_SEND_ONCE),
	// MACH_MSG_TYPE_COPY_RECEIVE is not a valid operation, so unclear why it's
	// even defined
	os_mach_port_disposition_init(MACH_MSG_TYPE_DISPOSE_RECEIVE),
	os_mach_port_disposition_init(MACH_MSG_TYPE_DISPOSE_SEND),
	os_mach_port_disposition_init(MACH_MSG_TYPE_DISPOSE_SEND_ONCE),
};

static inline const char *
_mach_port_disposition_string(mach_msg_bits_t d)
{
	if (d < MACH_MSG_TYPE_MOVE_RECEIVE) {
		return "[invalid]";
	}
	if (d > MACH_MSG_TYPE_DISPOSE_SEND_ONCE) {
		return "[invalid]";
	}
	return _mach_port_dispositions[d].ompd_human;
}

static const os_flagset_t _mach_port_rights = {
	os_flag_init(MACH_PORT_TYPE_SEND),
	os_flag_init(MACH_PORT_TYPE_RECEIVE),
	os_flag_init(MACH_PORT_TYPE_SEND_ONCE),
	os_flag_init(MACH_PORT_TYPE_PORT_SET),
	os_flag_init(MACH_PORT_TYPE_DEAD_NAME),
	// MACH_PORT_TYPE_LABELH is obsolete
	// MACH_PORT_TYPE_DNREQUEST->_mach_port_requests
	// MACH_PORT_TYPE_SPREQUEST->_mach_port_requests
	// MACH_PORT_TYPE_SPREQUEST_DELAYED->_mach_port_requests
	// MACH_PORT_RIGHT_NUMBER is obsolete
};

static const os_flagset_t _mach_port_requests = {
	os_flag_init(MACH_PORT_TYPE_DNREQUEST),
	os_flag_init(MACH_PORT_TYPE_SPREQUEST),
	os_flag_init(MACH_PORT_TYPE_SPREQUEST_DELAYED),
};

static const os_flagset_t _mach_port_status = {
	os_flag_init(MACH_PORT_STATUS_FLAG_TEMPOWNER),
	os_flag_init(MACH_PORT_STATUS_FLAG_GUARDED),
	os_flag_init(MACH_PORT_STATUS_FLAG_STRICT_GUARD),
	os_flag_init(MACH_PORT_STATUS_FLAG_IMP_DONATION),
	// MACH_PORT_STATUS_FLAG_REVIVE is obsolete
	os_flag_init(MACH_PORT_STATUS_FLAG_TASKPTR),
};

static const os_flagset_t _mach_special_bits = {
	os_flag_init(MACH_MSG_IPC_SPACE),
	os_flag_init(MACH_MSG_VM_SPACE),
	os_flag_init(MACH_MSG_IPC_KERNEL),
	os_flag_init(MACH_MSG_VM_KERNEL),
};

#pragma mark API
const mach_msg_trailer_t *
os_mach_msg_get_trailer(const mach_msg_header_t *hdr)
{
	// The mach_msg() documentation states that the trailer will follow the
	// message body on the next natural boundary. But when we moved to 64-bit,
	// we kept the trailer alignment on a 4-byte boundary for compatibility
	// reasons. Specifically, natural_t is still 32 bits on both 32- and 64-bit
	// platforms.
	return (mach_msg_trailer_t *)((uint8_t *)hdr + round_msg(hdr->msgh_size));
}

const mach_msg_audit_trailer_t *
os_mach_msg_get_audit_trailer(const mach_msg_header_t *hdr)
{
	const mach_msg_trailer_t *tlr = NULL;
	const mach_msg_audit_trailer_t *audit_tlr = NULL;

	tlr = os_mach_msg_get_trailer(hdr);
	if (tlr->msgh_trailer_type == MACH_MSG_TRAILER_FORMAT_0) {
		if (tlr->msgh_trailer_size >= sizeof(mach_msg_audit_trailer_t)) {
			audit_tlr = (mach_msg_audit_trailer_t *)tlr;
		}
	}

	return audit_tlr;
}

const mach_msg_context_trailer_t *
os_mach_msg_get_context_trailer(const mach_msg_header_t *hdr)
{
	const mach_msg_trailer_t *tlr = NULL;
	const mach_msg_context_trailer_t *ctx_tlr = NULL;

	tlr = os_mach_msg_get_trailer(hdr);
	if (tlr->msgh_trailer_type == MACH_MSG_TRAILER_FORMAT_0) {
		if (tlr->msgh_trailer_size >= sizeof(mach_msg_context_trailer_t)) {
			ctx_tlr = (mach_msg_context_trailer_t *)tlr;
		}
	}

	return ctx_tlr;
}

char *
os_mach_msg_copy_description(const mach_msg_header_t *msg)
{
	int ret = -1;
	mach_msg_bits_t local = MACH_MSGH_BITS_LOCAL(msg->msgh_bits);
	mach_msg_bits_t remote = MACH_MSGH_BITS_REMOTE(msg->msgh_bits);
	mach_msg_bits_t voucher = MACH_MSGH_BITS_VOUCHER(msg->msgh_bits);
	char *__os_free bits_desc = NULL;
	const char *local_desc = _mach_port_disposition_string(local);
	const char *remote_desc = _mach_port_disposition_string(remote);
	const char *voucher_desc = _mach_port_disposition_string(voucher);
	char *desc = NULL;
	mach_msg_size_t ool_cnt = 0;

	if (msg->msgh_bits & MACH_MSGH_BITS_COMPLEX) {
		ool_cnt = ((mach_msg_base_t *)msg)->body.msgh_descriptor_count;
	}

	bits_desc = os_flagset_copy_string(_mach_msgh_bits, msg->msgh_bits);
	ret = asprintf(&desc, "id = %#x, size = %u, bits = %s, "
			"local disp = %s, local port = %#x, "
			"remote disp = %s, remote port = %#x, "
		   	"voucher disp = %s, voucher port = %#x, "
			"out-of-line descriptor cnt = %u",
		   	msg->msgh_id, msg->msgh_size, bits_desc,
			local_desc, msg->msgh_local_port,
			remote_desc, msg->msgh_remote_port,
			voucher_desc, msg->msgh_voucher_port,
			ool_cnt);
	posix_assert_zero(ret);

	return desc;
}

char *
os_mach_msg_trailer_copy_description(const mach_msg_trailer_t *tlr)
{
	union {
		int r;
		size_t n;
	} ret = {
		.r = -1,
	};
	char *desc = NULL;
	char buff[512];
	char *cursor = buff;
	size_t left = sizeof(buff);
	// Yes we do not know the actual size of the trailer yet, so this is
	// technically unsafe, but we only dereference members after determining
	// that they are safe to dereference. Just us chickens and all that.
	const mach_msg_mac_trailer_t *max = (const mach_msg_mac_trailer_t *)tlr;

	if (tlr->msgh_trailer_type != MACH_MSG_TRAILER_FORMAT_0) {
		ret.r = asprintf(&desc, "type = %u, size = %u",
				tlr->msgh_trailer_type, tlr->msgh_trailer_size);
		os_assert_zero(ret.r);
		goto __out;
	}

	if (tlr->msgh_trailer_size >= sizeof(mach_msg_trailer_t)) {
		ret.r = snprintf(cursor, left, "format = %u, size = %u",
				tlr->msgh_trailer_type, tlr->msgh_trailer_size);
		os_assert_sprintf(ret.r, left);

		// Safe since the above assert has verified that ret is both positive
		// and less than or equal to the size of the buffer.
		cursor += ret.n;
		left -= ret.n;
	}

	if (tlr->msgh_trailer_size >= sizeof(mach_msg_seqno_trailer_t)) {
		ret.r = snprintf(cursor, left, ", seqno = %u", max->msgh_seqno);
		os_assert_sprintf(ret.r, left);
		cursor += ret.n;
		left -= ret.n;
	}

	if (tlr->msgh_trailer_size >= sizeof(mach_msg_security_trailer_t)) {
		ret.r = snprintf(cursor, left, ", security.uid = %u, security.gid = %u",
				max->msgh_sender.val[0], max->msgh_sender.val[1]);
		os_assert_sprintf(ret.r, left);
		cursor += ret.n;
		left -= ret.n;
	}

	if (tlr->msgh_trailer_size >= sizeof(mach_msg_audit_trailer_t)) {
		ret.r = snprintf(cursor, left, ", audit.auid = %u, "
			   	"audit.euid = %u, audit.egid = %u, "
			   	"audit.ruid = %u, audit.rgid = %u, "
			   	"audit.pid = %u, audit.asid = %u, audit.pidvers = %u",
				max->msgh_audit.val[0], max->msgh_audit.val[1],
			   	max->msgh_audit.val[2], max->msgh_audit.val[3],
			   	max->msgh_audit.val[4], max->msgh_audit.val[5],
			   	max->msgh_audit.val[6], max->msgh_audit.val[7]);
		os_assert_sprintf(ret.r, left);
		cursor += ret.n;
		left -= ret.n;
	}

	if (tlr->msgh_trailer_size >= sizeof(mach_msg_context_trailer_t)) {
		uint64_t ctx = max->msgh_context;
		ret.r = snprintf(cursor, left, ", context = %#llx", ctx);
		os_assert_sprintf(ret.r, left);
		cursor += ret.n;
		left -= ret.n;
	}

	if (tlr->msgh_trailer_size >= sizeof(mach_msg_mac_trailer_t)) {
		ret.r = snprintf(cursor, left, ", labels.sender = %#x",
				max->msgh_labels.sender);
		os_assert_sprintf(ret.r, left);
		cursor += ret.n;
		left -= ret.n;
	}

	desc = os_strdup(buff);

__out:
	return desc;
}

char *
os_mach_port_copy_description(mach_port_t p)
{
	kern_return_t kr = KERN_FAILURE;
	mach_port_right_t right = 0;
	mach_port_type_t type = 0;
	mach_port_urefs_t urefs = 0;
	mach_port_status_t status;
	mach_msg_type_number_t status_size = MACH_PORT_RECEIVE_STATUS_COUNT;
	char *desc = NULL;
	char *__os_free rightdesc = NULL;
	char *__os_free requestdesc = NULL;
	char *__os_free statusdesc = NULL;
	char *__os_free urefsdesc = NULL;
	char *which_urefs = "";
	int ret = -1;

	if (p == MACH_PORT_NULL) {
		return os_strdup("null");
	}
	if (p == MACH_PORT_DEAD) {
		return os_strdup("dead-name");
	}

	kr = mach_port_type(mach_task_self(), p, &type);
	switch (kr) {
	case KERN_SUCCESS:
		rightdesc = os_flagset_copy_string(_mach_port_rights, type);
		requestdesc = os_flagset_copy_string(_mach_port_requests, type);
		break;
	default:
		ret = asprintf(&rightdesc, "[%#x]", kr);
		posix_assert_zero(ret);
	}

	kr = mach_port_get_attributes(mach_task_self(), p,
			MACH_PORT_RECEIVE_STATUS, (mach_port_info_t)&status, &status_size);
	switch (kr) {
	case KERN_SUCCESS:
		if (status.mps_flags) {
			statusdesc = os_flagset_copy_string(_mach_port_status,
					status.mps_flags);
		} else {
			statusdesc = os_strdup("[none]");
		}

		break;
	case KERN_INVALID_RIGHT:
		if (!(type & MACH_PORT_TYPE_RECEIVE)) {
			statusdesc = os_strdup("[none]");
			break;
		}
	default:
		ret = asprintf(&statusdesc, "[%#x]", kr);
		posix_assert_zero(ret);
	}

	if (type & MACH_PORT_TYPE_SEND) {
		right = MACH_PORT_RIGHT_SEND;
		which_urefs = "send";
	} else if (type & MACH_PORT_TYPE_DEAD_NAME) {
		right = MACH_PORT_RIGHT_DEAD_NAME;
		which_urefs = "dead name";
	}

	if (which_urefs) {
		kr = mach_port_get_refs(mach_task_self(), p, right, &urefs);
		switch (kr) {
		case KERN_SUCCESS:
			ret = asprintf(&urefsdesc, ", %s urefs = %u", which_urefs, urefs);
			break;
		default:
			ret = asprintf(&urefsdesc, ", %s urefs = [%#x]",
					which_urefs, kr);
			break;
		}
	}

	ret = asprintf(&desc, "name = %#x, rights = %s, requests = %s, "
			"status = %s%s",
			p, rightdesc, requestdesc, statusdesc, urefsdesc);
	posix_assert_zero(ret);

	return desc;
}

#pragma mark API from <os/assumes.h>
// These live here because the implementations uses functionality from
// libdarwin, and we don't want to have a circular dependency between Libc and
// libsystem_darwin. The long-term plan is to move assumes() and assert()
// functionality into libdarwin anyway.
void
(os_assert_mach)(const char *op, kern_return_t kr)
{
	kern_return_t real_kr = (kern_return_t)(kr & (~MACH_MSG_MASK));
	kern_return_t extra = (kern_return_t)(kr & MACH_MSG_MASK);
	const char *err_string = NULL;
	const char *err_type_string = NULL;
	char err_buff[64];
	char code_buff[16];
	const char *special_desc = NULL;
	int sys = err_get_system(real_kr);
	int sub = err_get_sub(real_kr);
	int code = err_get_code(real_kr);

	if (kr == KERN_SUCCESS) {
		return;
	}

	if (kr >= BOOTSTRAP_NOT_PRIVILEGED && kr <= BOOTSTRAP_NO_CHILDREN) {
		err_string = bootstrap_strerror(kr);
		snprintf(code_buff, sizeof(code_buff), "%d", kr);
		err_type_string = "bootstrap";
	} else {
		err_string = mach_error_string(real_kr);
		if (strcmp(err_string, "unknown error code") == 0) {
			snprintf(err_buff, sizeof(err_buff), "[%#x|%#x|%#x]",
					sys, sub, code);
			err_string = err_buff;
			err_type_string = "unrecognized";
		} else {
			err_type_string = "mach";
		}

		if (kr <= MIG_TYPE_ERROR && kr >= MIG_TRAILER_ERROR) {
			snprintf(code_buff, sizeof(code_buff), "%d", kr);
		} else {
			snprintf(code_buff, sizeof(code_buff), "%#x", kr);
			special_desc = os_flagset_copy_string(_mach_special_bits, extra);
		}
	}

	if (special_desc) {
		os_crash("%s failed: %s error = %s [%s], special bits = %s",
				op, err_type_string, err_string, code_buff, special_desc);
	} else {
		os_crash("%s failed: %s error = %s [%s]",
				op, err_type_string, err_string, code_buff);
	}
}

void
os_assert_mach_port_status(const char *desc, mach_port_t p,
		mach_port_status_t *expected)
{
	kern_return_t kr = KERN_FAILURE;
	mach_port_status_t status;
	mach_msg_type_number_t status_cnt = MACH_PORT_RECEIVE_STATUS_COUNT;

	kr = mach_port_get_attributes(mach_task_self(), p, MACH_PORT_RECEIVE_STATUS,
			(mach_port_info_t)&status, &status_cnt);
	os_assert_mach("get status", kr);

	if (expected->mps_pset != UINT32_MAX) {
		if (expected->mps_pset != status.mps_pset) {
			os_crash("port set mismatch: actual = %u, expected = %u",
					status.mps_pset, expected->mps_pset);
		}
	}
	if (expected->mps_seqno != UINT32_MAX) {
		if (expected->mps_seqno != status.mps_seqno) {
			os_crash("sequence number mismatch: actual = %u, expected = %u",
					status.mps_seqno, expected->mps_seqno);
		}
	}
	if (expected->mps_mscount != UINT32_MAX) {
		if (expected->mps_mscount != status.mps_mscount) {
			os_crash("make-send count mismatch: actual = %u, expected = %u",
					status.mps_mscount, expected->mps_mscount);
		}
	}
	if (expected->mps_qlimit != UINT32_MAX) {
		if (expected->mps_qlimit != status.mps_qlimit) {
			os_crash("queue limit mismatch: actual = %u, expected = %u",
					status.mps_qlimit, expected->mps_qlimit);
		}
	}
	if (expected->mps_msgcount != UINT32_MAX) {
		if (expected->mps_msgcount != status.mps_msgcount) {
			os_crash("message count mismatch: actual = %u, expected = %u",
					status.mps_msgcount, expected->mps_msgcount);
		}
	}
	if (expected->mps_sorights != UINT32_MAX) {
		if (expected->mps_sorights != status.mps_sorights) {
			os_crash("send-once rights mismatch: actual = %u, expected = %u",
					status.mps_sorights, expected->mps_sorights);
		}
	}
	if (expected->mps_srights != INT32_MAX) {
		if (expected->mps_srights != status.mps_srights) {
			os_crash("send rights mismatch: actual = %d, expected = %d",
					status.mps_srights, expected->mps_srights);
		}
	}
	if (expected->mps_pdrequest != INT32_MAX) {
		if (expected->mps_pdrequest != status.mps_pdrequest) {
			os_crash("port-destroyed mismatch: actual = %d, expected = %d",
					status.mps_pdrequest, expected->mps_pdrequest);
		}
	}
	if (expected->mps_nsrequest != INT32_MAX) {
		if (expected->mps_nsrequest != status.mps_nsrequest) {
			os_crash("no-senders mismatch: actual = %d, expected = %d",
					status.mps_nsrequest, expected->mps_nsrequest);
		}
	}
	if (expected->mps_flags) {
		if (expected->mps_flags != status.mps_flags) {
			os_crash("flags mismatch: actual = %#x, expected = %#x",
					status.mps_flags, expected->mps_flags);
		}
	}
}
