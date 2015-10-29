/* srm */
/* Copyright (c) 2000 Matthew D. Gauthier
 * Portions copyright (c) 2007 Apple Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the contributors shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fts.h>

#include "removefile.h"
#include "removefile_priv.h"

int
__removefile_process_file(FTS* stream, FTSENT* current_file, removefile_state_t state) {
	int res = 0;
	char* path = current_file->fts_path;

	int recursive = state->unlink_flags & REMOVEFILE_RECURSIVE;
	int keep_parent = state->unlink_flags & REMOVEFILE_KEEP_PARENT;
	int secure = state->unlink_flags & (REMOVEFILE_SECURE_7_PASS | REMOVEFILE_SECURE_35_PASS | REMOVEFILE_SECURE_1_PASS | REMOVEFILE_SECURE_3_PASS | REMOVEFILE_SECURE_1_PASS_ZERO);

	switch (current_file->fts_info) {
		// attempt to unlink the directory on pre-order in case it is
		// a directory hard-link.  If we succeed, it was a hard-link
		// and we should not descend any further.
		case FTS_D:
			if (unlink(path) == 0) {
				fts_set(stream, current_file, FTS_SKIP);
			}
			break;
		case FTS_DC:
			state->error_num = ELOOP;
			res = -1;
			break;
		case FTS_DNR:
		case FTS_ERR:
		case FTS_NS:
			state->error_num = current_file->fts_errno;
			res = -1;
			break;
		case FTS_DP:
			if (recursive &&
			    (!keep_parent ||
			     current_file->fts_level != FTS_ROOTLEVEL)) {
				if (secure) {
					res = __removefile_rename_unlink(path,
						state);
				} else {
					if (geteuid() == 0 &&
						(current_file->fts_statp->st_flags & (UF_APPEND|UF_IMMUTABLE)) &&
						!(current_file->fts_statp->st_flags & (SF_APPEND|SF_IMMUTABLE)) &&
						chflags(path, current_file->fts_statp->st_flags &= ~(UF_APPEND|UF_IMMUTABLE)) < 0) {
						errno = EACCES;
						res = -1;
					} else {
						res = rmdir(path);
					}
				}
				if (res == -1) state->error_num = errno;
			}
			break;
		case FTS_F:
		case FTS_SL:
		case FTS_SLNONE:
		case FTS_DEFAULT:
			if (secure) {
				res = __removefile_sunlink(path, state);
			} else if (geteuid() == 0 &&
				(current_file->fts_statp->st_flags & (UF_APPEND|UF_IMMUTABLE)) &&
				!(current_file->fts_statp->st_flags & (SF_APPEND|SF_IMMUTABLE)) &&
				chflags(path, current_file->fts_statp->st_flags &= ~(UF_APPEND|UF_IMMUTABLE)) < 0) {
				errno = EACCES;
				res = -1;
			} else {
				res = unlink(path);
			}
			if (res == -1) state->error_num = errno;
			break;
		case FTS_DOT:
		default:
			break;
	}
	return res;
}

int
__removefile_tree_walker(char **trees, removefile_state_t state) {
	FTSENT *current_file;
	FTS *stream;
	int rval = 0;

	removefile_callback_t cb_confirm = NULL;
	removefile_callback_t cb_status = NULL;
	removefile_callback_t cb_error = NULL;

	cb_confirm = state->confirm_callback;
	cb_status = state->status_callback;
	cb_error = state->error_callback;

	stream = fts_open(trees, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
	if (stream == NULL) return -1;

	while ((current_file = fts_read(stream)) != NULL) {
		int res = REMOVEFILE_PROCEED;

		/*
		 * fts_number is set to REMOVEFILE_SKIP for directories where
		 * the confirmation callback has asked to skip it.  We check
		 * this on post-order, so we don't remove an empty directory that
		 * the caller wanted preserved.
		 */
		if (current_file->fts_info == FTS_DP &&
		    current_file->fts_number == REMOVEFILE_SKIP) {
			current_file->fts_number = 0;
			continue;
		}

		// don't process the file if a cancel has been requested
		if (__removefile_state_test_cancel(state)) break;

		// confirm regular files and directories in pre-order 
		if (cb_confirm && current_file->fts_info != FTS_DP) {
			res = cb_confirm(state,
				current_file->fts_path, state->confirm_context);
		}

		// don't process the file if a cancel has been requested
		// by the callback
		if (__removefile_state_test_cancel(state)) break;

		if (res == REMOVEFILE_PROCEED) {
			state->error_num = 0;
			rval = __removefile_process_file(stream, current_file,
					state);

			if (state->error_num != 0) {
				// Since removefile(3) is abstracting the
				// traversal of the directory tree, suppress
				// all ENOENT and ENOTDIR errors from the
				// calling process.
				// However, these errors should still be
				// reported for the ROOTLEVEL since those paths
				// were explicitly provided by the application.
				if ((state->error_num != ENOENT &&
				     state->error_num != ENOTDIR) ||
				    current_file->fts_level == FTS_ROOTLEVEL) {
					if (cb_error) {
						res = cb_error(state,
							current_file->fts_path,
							state->error_context);
						if (res == REMOVEFILE_PROCEED ||
							res == REMOVEFILE_SKIP) {
							rval = 0;
						} else if (res == REMOVEFILE_STOP) {
							rval = -1;
						}
					} else {
						res = REMOVEFILE_STOP;
					}
				}
			// show status for regular files and directories
			// in post-order
			} else if (cb_status &&
				   current_file->fts_info != FTS_D) {
				res = cb_status(state, current_file->fts_path,
					state->status_context);
			}
		}

		if (current_file->fts_info == FTS_D && res == REMOVEFILE_SKIP) {
			current_file->fts_number = REMOVEFILE_SKIP;
		}

		if (res == REMOVEFILE_SKIP ||
    		    !(state->unlink_flags & REMOVEFILE_RECURSIVE))
			fts_set(stream, current_file, FTS_SKIP);
		if (res == REMOVEFILE_STOP ||
		    __removefile_state_test_cancel(state))
			break;
	}

	if (__removefile_state_test_cancel(state)) {
		errno = ECANCELED;
		rval = -1;
	}

	fts_close(stream);
	return rval;
}
