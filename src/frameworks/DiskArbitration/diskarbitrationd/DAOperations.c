/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DAOperations.h"

#include <Block.h>
#include <DiskArbitration/DiskArbitration.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#include "../DAProtocol.h"
#include "DAModel.h"
#include "DAServer.h"

// How long an approval round waits for its answers; a connection that has not answered by then approves.
#define DA_APPROVAL_TIMEOUT (10 * NSEC_PER_SEC)

typedef void (^DARoundDone)(DAReturn status, const char* string);

// An approval round: every connection that said hello is asked; the first dissent ends it (unless dissent is
// ignored), and so does the last approval or the timeout.
struct DARound
{
	int references;
	int pending;
	bool finished;
	bool ignoreDissent;
	DARoundDone done;
	dispatch_source_t timer;
};

static void roundRelease(struct DARound* round)
{
	if (--round->references > 0)
		return;
	Block_release(round->done);
	dispatch_release(round->timer);
	free(round);
}

static void roundFinish(struct DARound* round, DAReturn status, const char* string)
{
	if (round->finished)
		return;
	round->finished = true;
	dispatch_source_cancel(round->timer);
	round->done(status, string);
}

static void askApproval(const char* kind, const char* name, CFDictionaryRef description, bool ignoreDissent,
	DARoundDone done)
{
	struct DARound* round = calloc(1, sizeof(*round));
	if (round == NULL)
	{
		done(kDAReturnNoResources, NULL);
		return;
	}
	round->references = 2; // this function and the timer
	round->ignoreDissent = ignoreDissent;
	round->done = Block_copy(done);
	round->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, DAServerQueue());
	dispatch_source_set_timer(round->timer, dispatch_time(DISPATCH_TIME_NOW, DA_APPROVAL_TIMEOUT),
		DISPATCH_TIME_FOREVER, 0);
	dispatch_source_set_event_handler(round->timer, ^{
		roundFinish(round, kDAReturnSuccess, NULL);
	});
	dispatch_source_set_cancel_handler(round->timer, ^{
		roundRelease(round);
	});
	dispatch_resume(round->timer);

	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, DA_KEY_EVENT, DA_EVENT_APPROVE);
	xpc_dictionary_set_string(request, DA_KEY_KIND, kind);
	xpc_dictionary_set_string(request, DA_KEY_NAME, name);
	DAServerSetDescription(request, description);

	DAServerForEachSubscriber(^(xpc_connection_t peer) {
		round->pending++;
		round->references++;
		xpc_connection_send_message_with_reply(peer, request, DAServerQueue(), ^(xpc_object_t answer) {
			// a connection that went away approves
			DAReturn status = kDAReturnSuccess;
			const char* string = NULL;
			if (xpc_get_type(answer) == XPC_TYPE_DICTIONARY)
			{
				status = (DAReturn) xpc_dictionary_get_int64(answer, DA_KEY_STATUS);
				string = xpc_dictionary_get_string(answer, DA_KEY_STRING);
			}
			if (status != kDAReturnSuccess && !round->ignoreDissent)
				roundFinish(round, status, string);
			else if (--round->pending == 0)
				roundFinish(round, kDAReturnSuccess, NULL);
			roundRelease(round);
		});
	});
	xpc_release(request);

	if (round->pending == 0)
		roundFinish(round, kDAReturnSuccess, NULL);
	roundRelease(round);
}

static void sendReply(xpc_connection_t peer, xpc_object_t reply, DAReturn status, const char* string)
{
	xpc_dictionary_set_int64(reply, DA_KEY_STATUS, status);
	if (string != NULL)
		xpc_dictionary_set_string(reply, DA_KEY_STRING, string);
	xpc_connection_send_message(peer, reply);
}

// QUIRK: when unmount(2) fails, the request fails with 0xC000 | errno (0xC010 for EBUSY), not a kDAReturn code.
// The same status is returned for ejecting a whole disk whose volume is still mounted. Not documented. Evidence:
// black-box observation on macOS 26.7.3 (tier 2): an unmount with a file open in the volume, and an eject of a
// mounted image, both complete with 0xC010 (tests/diskarbitration.md).
// Test: diskarbitration.sh "ops: unmount with a busy file" and "ops: eject whole while mounted".
static DAReturn statusForErrno(int error)
{
	return 0xC000 | error;
}

// Only root and the user who mounted the volume may unmount or eject it.
static bool privileged(xpc_connection_t peer, const DAMedia* media)
{
	uid_t euid = xpc_connection_get_euid(peer);
	return euid == 0 || euid == media->owner;
}

// The media with that leaf or whole disk name, if it is still the one whose id was taken.
static DAMedia* findAgain(const char* name, unsigned long id)
{
	bool whole;
	DAMedia* media = DAModelFindName(name, &whole);
	return media != NULL && media->id == id ? media : NULL;
}

void DAOperationUnmount(xpc_connection_t peer, xpc_object_t message, xpc_object_t reply)
{
	const char* requested = xpc_dictionary_get_string(message, DA_KEY_NAME);
	uint64_t options = xpc_dictionary_get_uint64(message, DA_KEY_OPTIONS);
	bool whole = false;
	DAMedia* media = requested != NULL ? DAModelFindName(requested, &whole) : NULL;

	if (media == NULL)
		return sendReply(peer, reply, kDAReturnBadArgument, NULL);
	if (!privileged(peer, media))
		return sendReply(peer, reply, kDAReturnNotPrivileged, NULL);
	if (whole && !(options & kDADiskUnmountOptionWhole))
		return sendReply(peer, reply, kDAReturnNotMounted, NULL);
	if (!media->mounted)
		return sendReply(peer, reply, whole ? kDAReturnSuccess : kDAReturnNotMounted, NULL);
	if (media->root)
		return sendReply(peer, reply, kDAReturnNotPermitted, NULL);

	// a whole disk is unmounted by unmounting its leaf
	char name[32];
	unsigned long id = media->id;
	bool force = (options & kDADiskUnmountOptionForce) != 0;
	DAModelName(media, false, name, sizeof(name));
	char* leaf = strdup(name);
	if (leaf == NULL)
		return sendReply(peer, reply, kDAReturnNoResources, NULL);

	xpc_retain(peer);
	xpc_retain(reply);
	askApproval(DA_OP_UNMOUNT, leaf, media->leaf, force, ^(DAReturn status, const char* string) {
		// the volume may have gone away during the round
		DAMedia* current = findAgain(leaf, id);
		if (status == kDAReturnSuccess && current == NULL)
			status = kDAReturnBadArgument;
		else if (status == kDAReturnSuccess && !current->mounted)
			status = kDAReturnNotMounted;
		else if (status == kDAReturnSuccess && unmount(current->mountPoint, force ? MNT_FORCE : 0) != 0)
			status = statusForErrno(errno);
		else if (status == kDAReturnSuccess)
			DAModelUnmounted(current, DAServerEvents());

		sendReply(peer, reply, status, status != kDAReturnSuccess ? string : NULL);
		xpc_release(reply);
		xpc_release(peer);
		free(leaf);
	});
}

void DAOperationEject(xpc_connection_t peer, xpc_object_t message, xpc_object_t reply)
{
	const char* requested = xpc_dictionary_get_string(message, DA_KEY_NAME);
	bool whole = false;
	DAMedia* media = requested != NULL ? DAModelFindName(requested, &whole) : NULL;

	if (media == NULL)
		return sendReply(peer, reply, kDAReturnBadArgument, NULL);
	if (!whole)
		return sendReply(peer, reply, kDAReturnUnsupported, NULL);
	if (!privileged(peer, media))
		return sendReply(peer, reply, kDAReturnNotPrivileged, NULL);
	if (media->root)
		return sendReply(peer, reply, kDAReturnUnsupported, NULL);
	// a mounted volume fails the eject only after the approval round, as on macOS (a dissent comes first)

	char name[32];
	unsigned long id = media->id;
	DAModelName(media, true, name, sizeof(name));
	char* wholeName = strdup(name);
	if (wholeName == NULL)
		return sendReply(peer, reply, kDAReturnNoResources, NULL);

	xpc_retain(peer);
	xpc_retain(reply);
	askApproval(DA_OP_EJECT, wholeName, media->whole, false, ^(DAReturn status, const char* string) {
		DAMedia* current = findAgain(wholeName, id);
		if (status == kDAReturnSuccess && current == NULL)
			status = kDAReturnBadArgument;
		else if (status == kDAReturnSuccess && current->mounted)
			status = statusForErrno(EBUSY);
		else if (status == kDAReturnSuccess)
			DAModelRemove(current, DAServerEvents()); // the image's FUSE process exited when it was unmounted

		sendReply(peer, reply, status, status != kDAReturnSuccess ? string : NULL);
		xpc_release(reply);
		xpc_release(peer);
		free(wholeName);
	});
}
