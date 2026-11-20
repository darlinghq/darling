/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * The CMAttachment functions: key, value and propagation mode attachments on CF objects.
 *
 * A CVBuffer is a bearer too, and on one each function is documented to be equivalent to the
 * CVBuffer attachment function, so they delegate to CoreVideo (AGENTS section 2). CMSampleBuffer and
 * CMBlockBuffer keep their attachments in a CMAttachmentStore. The documentation says the functions
 * fail on any other object. They return no status, so on one the setters and removers do nothing and
 * the getters return NULL.
 */

#include <CoreMedia/CMAttachment.h>
#include "CMAttachmentStore.h"
#include <CoreVideo/CoreVideo.h>
#include <pthread.h>
#include <stdbool.h>

// One lock covers every store; attachments are touched rarely and briefly. A value's last release can
// run arbitrary code (a finalizer that calls back into these functions), so values leaving a store are
// kept alive under the lock and released after it, never while it is held.
static pthread_mutex_t storeLock = PTHREAD_MUTEX_INITIALIZER;

// The documentation defines an invalid mode only for CMCopyDictionaryOfAttachments, which returns NULL.
static bool modeIsValid(CMAttachmentMode mode)
{
	return mode == kCMAttachmentMode_ShouldNotPropagate || mode == kCMAttachmentMode_ShouldPropagate;
}

// CoreVideo declares this only for iOS (CVOpenGLESTexture.h needs the OpenGLES headers), but Darling's
// CoreVideo builds and exports it, and the texture starts with a CVBuffer like the other classes here.
extern CFTypeID CVOpenGLESTextureGetTypeID(void);

// The CoreVideo classes that start with a CVBuffer and so share its attachment functions.
static bool isCVBuffer(CFTypeRef object)
{
	CFTypeID type = CFGetTypeID(object);
	return type == CVPixelBufferGetTypeID() || type == CVOpenGLTextureGetTypeID()
		|| type == CVOpenGLESTextureGetTypeID();
}

static void releaseAll(CFTypeRef *values, int count)
{
	for (int i = 0; i < count; i++) {
		if (values[i] != NULL)
			CFRelease(values[i]);
	}
}

static CFMutableDictionaryRef *slotFor(CMAttachmentStore *store, CMAttachmentMode mode)
{
	return mode == kCMAttachmentMode_ShouldPropagate ? &store->propagating : &store->local;
}

// GUESS: a mode other than ShouldPropagate counts as ShouldNotPropagate, as CVBufferSetAttachment treats
// it, so the same call behaves the same on every kind of bearer. The documentation does not cover it.
// The values it replaces or moves are returned retained in `dropped`, for the caller to release unlocked.
static void storeSet(CMAttachmentStore *store, CFStringRef key, CFTypeRef value, CMAttachmentMode mode,
	CFTypeRef dropped[2])
{
	CMAttachmentMode effective = mode == kCMAttachmentMode_ShouldPropagate
		? kCMAttachmentMode_ShouldPropagate : kCMAttachmentMode_ShouldNotPropagate;
	CMAttachmentMode other = effective == kCMAttachmentMode_ShouldPropagate
		? kCMAttachmentMode_ShouldNotPropagate : kCMAttachmentMode_ShouldPropagate;

	CFMutableDictionaryRef *slot = slotFor(store, effective);
	if (*slot == NULL)
		*slot = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
	if (*slot == NULL)
		return;
	CFTypeRef old = CFDictionaryGetValue(*slot, key);
	dropped[0] = old != NULL ? CFRetain(old) : NULL;
	// Retain the new value before dropping the old one: the caller may not own `value` (it can be what
	// CMGetAttachment returned for this key in the other mode, which the removal below would free).
	CFDictionarySetValue(*slot, key, value);
	// A key lives in one mode at a time.
	CFMutableDictionaryRef *otherSlot = slotFor(store, other);
	if (*otherSlot != NULL) {
		old = CFDictionaryGetValue(*otherSlot, key);
		dropped[1] = old != NULL ? CFRetain(old) : NULL;
		CFDictionaryRemoveValue(*otherSlot, key);
	}
}

void CMSetAttachment(CMAttachmentBearerRef target, CFStringRef key, CFTypeRef value, CMAttachmentMode attachmentMode)
{
	// A NULL value is documented as an error.
	if (target == NULL || key == NULL || value == NULL)
		return;
	if (isCVBuffer(target)) {
		// CVBufferSetAttachment drops the key's old attachment before it retains the new value, which
		// frees a value that the caller got from CMGetAttachment and does not own. Hold it meanwhile.
		CFRetain(value);
		CVBufferSetAttachment((CVBufferRef)target, key, value, attachmentMode);
		CFRelease(value);
		return;
	}
	CMAttachmentStore *store = CMCoreGetAttachmentStore(target);
	if (store == NULL)
		return;
	CFTypeRef dropped[2] = { NULL, NULL };
	pthread_mutex_lock(&storeLock);
	storeSet(store, key, value, attachmentMode, dropped);
	pthread_mutex_unlock(&storeLock);
	releaseAll(dropped, 2);
}

CFTypeRef CMGetAttachment(CMAttachmentBearerRef target, CFStringRef key, CMAttachmentMode *attachmentModeOut)
{
	if (target == NULL || key == NULL)
		return NULL;
	// GUESS, for a maintainer to decide: the result is not retained for the caller. The documentation
	// says nothing on ownership but calls this equivalent to CVBufferCopyAttachment (retaining) on a
	// CVBuffer. The Core Foundation memory management rules give a Get function a result the caller does
	// not own, and the function is named Get, so the code follows that and matches CVBufferGetAttachment.
	if (isCVBuffer(target)) {
		CMAttachmentMode mode = kCMAttachmentMode_ShouldNotPropagate;
		CFTypeRef value = CVBufferGetAttachment((CVBufferRef)target, key, &mode);
		if (value != NULL && attachmentModeOut != NULL)
			*attachmentModeOut = mode;
		return value;
	}
	CMAttachmentStore *store = CMCoreGetAttachmentStore(target);
	if (store == NULL)
		return NULL;
	pthread_mutex_lock(&storeLock);
	CFTypeRef value = NULL;
	if (store->propagating != NULL && (value = CFDictionaryGetValue(store->propagating, key)) != NULL) {
		if (attachmentModeOut != NULL)
			*attachmentModeOut = kCMAttachmentMode_ShouldPropagate;
	} else if (store->local != NULL && (value = CFDictionaryGetValue(store->local, key)) != NULL) {
		if (attachmentModeOut != NULL)
			*attachmentModeOut = kCMAttachmentMode_ShouldNotPropagate;
	}
	pthread_mutex_unlock(&storeLock);
	return value;
}

void CMRemoveAttachment(CMAttachmentBearerRef target, CFStringRef key)
{
	if (target == NULL || key == NULL)
		return;
	if (isCVBuffer(target)) {
		CVBufferRemoveAttachment((CVBufferRef)target, key);
		return;
	}
	CMAttachmentStore *store = CMCoreGetAttachmentStore(target);
	if (store == NULL)
		return;
	CFTypeRef dropped[2] = { NULL, NULL };
	pthread_mutex_lock(&storeLock);
	CFMutableDictionaryRef dictionaries[2] = { store->propagating, store->local };
	for (int i = 0; i < 2; i++) {
		CFTypeRef old = dictionaries[i] != NULL ? CFDictionaryGetValue(dictionaries[i], key) : NULL;
		if (old != NULL) {
			dropped[i] = CFRetain(old);
			CFDictionaryRemoveValue(dictionaries[i], key);
		}
	}
	pthread_mutex_unlock(&storeLock);
	releaseAll(dropped, 2);
}

void CMRemoveAllAttachments(CMAttachmentBearerRef target)
{
	if (target == NULL)
		return;
	if (isCVBuffer(target)) {
		CVBufferRemoveAllAttachments((CVBufferRef)target);
		return;
	}
	CMAttachmentStore *store = CMCoreGetAttachmentStore(target);
	if (store == NULL)
		return;
	// Snapshots keep the values alive until the lock is gone.
	CFDictionaryRef dropped[2] = { NULL, NULL };
	pthread_mutex_lock(&storeLock);
	CFMutableDictionaryRef dictionaries[2] = { store->propagating, store->local };
	for (int i = 0; i < 2; i++) {
		if (dictionaries[i] != NULL) {
			dropped[i] = CFDictionaryCreateCopy(kCFAllocatorDefault, dictionaries[i]);
			// Out of memory: keep the attachments rather than release them under the lock.
			if (dropped[i] != NULL)
				CFDictionaryRemoveAllValues(dictionaries[i]);
		}
	}
	pthread_mutex_unlock(&storeLock);
	releaseAll((CFTypeRef *)dropped, 2);
}

CFDictionaryRef CMCopyDictionaryOfAttachments(CFAllocatorRef allocator, CMAttachmentBearerRef target,
	CMAttachmentMode attachmentMode)
{
	if (target == NULL || !modeIsValid(attachmentMode))
		return NULL;
	if (isCVBuffer(target))
		return CVBufferCopyAttachments((CVBufferRef)target, attachmentMode); // ignores the allocator
	CMAttachmentStore *store = CMCoreGetAttachmentStore(target);
	if (store == NULL)
		return NULL;
	pthread_mutex_lock(&storeLock);
	CFMutableDictionaryRef source = *slotFor(store, attachmentMode);
	// A snapshot: later changes to the attachments do not show in it. No attachments is an empty dictionary.
	CFDictionaryRef copy = source != NULL
		? CFDictionaryCreateCopy(allocator, source)
		: CFDictionaryCreate(allocator, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
	pthread_mutex_unlock(&storeLock);
	return copy;
}

static void setPair(const void *key, const void *value, void *context)
{
	void **args = context;
	CMSetAttachment((CMAttachmentBearerRef)args[0], (CFStringRef)key, (CFTypeRef)value,
		*(CMAttachmentMode *)args[1]);
}

void CMSetAttachments(CMAttachmentBearerRef target, CFDictionaryRef theAttachments, CMAttachmentMode attachmentMode)
{
	if (target == NULL || theAttachments == NULL)
		return;
	if (isCVBuffer(target)) {
		CVBufferSetAttachments((CVBufferRef)target, theAttachments, attachmentMode);
		return;
	}
	// One CMSetAttachment per pair, as documented.
	void *args[2] = { (void *)target, &attachmentMode };
	CFDictionaryApplyFunction(theAttachments, setPair, args);
}

void CMPropagateAttachments(CMAttachmentBearerRef source, CMAttachmentBearerRef destination)
{
	if (source == NULL || destination == NULL)
		return;
	if (isCVBuffer(source) && isCVBuffer(destination)) {
		CVBufferPropagateAttachments((CVBufferRef)source, (CVBufferRef)destination);
		return;
	}
	CFDictionaryRef propagating = CMCopyDictionaryOfAttachments(kCFAllocatorDefault, source,
		kCMAttachmentMode_ShouldPropagate);
	if (propagating == NULL)
		return;
	CMSetAttachments(destination, propagating, kCMAttachmentMode_ShouldPropagate);
	CFRelease(propagating);
}
