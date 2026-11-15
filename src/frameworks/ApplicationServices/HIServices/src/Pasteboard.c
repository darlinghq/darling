/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// The Pasteboard Manager over CoreFoundation's pasteboard store. A PasteboardRef is a reference to one pasteboard
// that remembers the change count it last saw (PasteboardSynchronize), whether it cleared the current contents, and
// its promise keeper. Everything else is the store's: the same pasteboards are what NSPasteboard uses, so both
// interfaces see the same items and change counts.
//
// Behaviour, as observed on macOS 26.7.3 and 10.14.4 (see the test tests/src/pasteboard_interop.c and the notes of
// the change). A reference remembers the change count it last saw. The process remembers, per pasteboard, the change
// count that the last PasteboardClear of any reference in the process produced: that is the process owning the
// contents, so all references to the pasteboard in the process may put flavors, and a change made that way is not a
// reason for another reference to be out of date.
//  - reading calls return badPasteboardSyncErr when the contents changed since the reference last saw them, unless
//    the change was a PasteboardClear in this process;
//  - putting a flavor also returns badPasteboardSyncErr then (on 26.7.3 that returns notPasteboardOwnerErr instead;
//    the older behaviour is followed), and notPasteboardOwnerErr when the process did not clear the current contents;
//  - putting a flavor that the item has already, with data, returns duplicatePasteboardFlavorErr, but a promised
//    flavor may be replaced (that is how a promise keeper supplies the data);
//  - PasteboardSynchronize returns kPasteboardModified when the contents changed since the reference last saw them,
//    otherwise kPasteboardClientIsOwner if this process cleared the current contents, otherwise 0.
// Not covered here: the translated flavors that macOS adds (for example UTF-16 and legacy text for UTF-8 text).

#include <HIServices/Pasteboard.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFPasteboard_Private.h>
#include <CoreFoundation/CFRuntime.h>
#include <pthread.h>
#include <stdlib.h>

struct OpaquePasteboardRef {
	CFRuntimeBase _base;
	CFPasteboardRef pasteboard;
	CFStringRef name;
	CFIndex seen;                   // change count at the last synchronization
	PasteboardPromiseKeeperProcPtr keeper;
	void *keeperContext;
};

// Serializes access to the fields above and to the list of references that have a keeper
static pthread_mutex_t gLock = PTHREAD_MUTEX_INITIALIZER;
static CFMutableDictionaryRef gCleared;  // pasteboard name -> change count produced by the last PasteboardClear in this process
static CFMutableArrayRef gKeeperRefs;   // unretained references with a keeper, for kPasteboardResolveAllPromises

static void keeperRefsRemove(PasteboardRef ref) {
	pthread_mutex_lock(&gLock);
	if (gKeeperRefs != NULL) {
		CFIndex i = CFArrayGetFirstIndexOfValue(gKeeperRefs, CFRangeMake(0, CFArrayGetCount(gKeeperRefs)), ref);
		if (i != kCFNotFound) CFArrayRemoveValueAtIndex(gKeeperRefs, i);
	}
	pthread_mutex_unlock(&gLock);
}

static void __PasteboardDeallocate(CFTypeRef cf) {
	PasteboardRef ref = (PasteboardRef)cf;
	keeperRefsRemove(ref);
	CFRelease(ref->pasteboard);
	CFRelease(ref->name);
}

// Does this process own the contents with the given change count? (gLock held)
static Boolean clearedHere(PasteboardRef ref, CFIndex current) {
	CFNumberRef value = gCleared ? (CFNumberRef)CFDictionaryGetValue(gCleared, ref->name) : NULL;
	CFIndex cleared = 0;
	return value != NULL && CFNumberGetValue(value, kCFNumberCFIndexType, &cleared) && cleared == current;
}

static CFTypeID gTypeID = _kCFRuntimeNotATypeID;
static const CFRuntimeClass gClass = { 0, "Pasteboard", NULL, NULL, __PasteboardDeallocate, NULL, NULL, NULL, NULL };

CFTypeID PasteboardGetTypeID(void) {
	pthread_mutex_lock(&gLock);
	if (gTypeID == _kCFRuntimeNotATypeID) gTypeID = _CFRuntimeRegisterClass(&gClass);
	pthread_mutex_unlock(&gLock);
	return gTypeID;
}

static OSStatus translateStatus(CFPasteboardStatus status, OSStatus flavorError) {
	switch (status) {
		case kCFPasteboardSuccess: return noErr;
		case kCFPasteboardBadItem: return badPasteboardItemErr;
		case kCFPasteboardBadFlavor: return badPasteboardFlavorErr;
		case kCFPasteboardBadIndex: return badPasteboardIndexErr;
		case kCFPasteboardNotOwner: return notPasteboardOwnerErr;
		case kCFPasteboardNoProvider: return flavorError;
		case kCFPasteboardTooLarge: return memFullErr;
		case kCFPasteboardInvalid: return paramErr;
		default: return ioErr;
	}
}

OSStatus PasteboardCreate(CFStringRef name, PasteboardRef *out) {
	if (out == NULL) return paramErr;
	*out = NULL;
	CFPasteboardRef pasteboard;
	if (name == NULL) {
		pasteboard = CFPasteboardCreateUnique(kCFAllocatorDefault);
	} else {
		// the Pasteboard Manager's names for the standard pasteboards are aliases of CoreFoundation's names
		if (CFEqual(name, kPasteboardClipboard)) name = kCFPasteboardGeneral;
		else if (CFEqual(name, kPasteboardFind)) name = kCFPasteboardFind;
		pasteboard = CFPasteboardCreate(kCFAllocatorDefault, name);
	}
	if (pasteboard == NULL) return name == NULL ? ioErr : paramErr;

	PasteboardRef ref = (PasteboardRef)_CFRuntimeCreateInstance(kCFAllocatorDefault, PasteboardGetTypeID(),
		sizeof(struct OpaquePasteboardRef) - sizeof(CFRuntimeBase), NULL);
	if (ref == NULL) {
		CFRelease(pasteboard);
		return memFullErr;
	}
	ref->pasteboard = pasteboard;
	ref->name = CFPasteboardCopyName(pasteboard);
	ref->seen = CFPasteboardGetChangeCount(pasteboard);
	ref->keeper = NULL;
	ref->keeperContext = NULL;
	*out = ref;
	return noErr;
}

PasteboardSyncFlags PasteboardSynchronize(PasteboardRef ref) {
	CFIndex current = CFPasteboardGetChangeCount(ref->pasteboard);
	PasteboardSyncFlags flags = 0;
	pthread_mutex_lock(&gLock);
	if (current != ref->seen) flags = kPasteboardModified;
	else if (clearedHere(ref, current)) flags = kPasteboardClientIsOwner;
	ref->seen = current;
	pthread_mutex_unlock(&gLock);
	return flags;
}

// Reading needs a reference that has seen the current contents
static OSStatus requireSynchronized(PasteboardRef ref) {
	CFIndex current = CFPasteboardGetChangeCount(ref->pasteboard);
	if (current == 0) return ioErr;
	pthread_mutex_lock(&gLock);
	Boolean stale = current != ref->seen && !clearedHere(ref, current);
	pthread_mutex_unlock(&gLock);
	return stale ? badPasteboardSyncErr : noErr;
}

OSStatus PasteboardClear(PasteboardRef ref) {
	CFIndex count = CFPasteboardBeginGeneration(ref->pasteboard);
	if (count == 0) return ioErr;
	pthread_mutex_lock(&gLock);
	ref->seen = count;
	if (gCleared == NULL) gCleared = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFNumberRef value = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &count);
	CFDictionarySetValue(gCleared, ref->name, value);
	CFRelease(value);
	pthread_mutex_unlock(&gLock);
	return noErr;
}

OSStatus PasteboardCopyName(PasteboardRef ref, CFStringRef *name) {
	if (name == NULL) return paramErr;
	*name = CFPasteboardCopyName(ref->pasteboard);
	return noErr;
}

OSStatus PasteboardGetItemCount(PasteboardRef ref, ItemCount *count) {
	OSStatus err = requireSynchronized(ref);
	if (err != noErr) return err;
	CFIndex n = 0;
	CFPasteboardStatus status = CFPasteboardGetItemCount(ref->pasteboard, &n);
	if (status == kCFPasteboardSuccess && count != NULL) *count = (ItemCount)n;
	return translateStatus(status, paramErr);
}

OSStatus PasteboardGetItemIdentifier(PasteboardRef ref, CFIndex index, PasteboardItemID *item) {
	OSStatus err = requireSynchronized(ref);
	if (err != noErr) return err;
	CFPasteboardItemID id = 0;
	CFPasteboardStatus status = index < 1 ? kCFPasteboardBadIndex : CFPasteboardGetItemAtIndex(ref->pasteboard, index - 1, &id);
	if (status == kCFPasteboardSuccess && item != NULL) *item = (PasteboardItemID)(uintptr_t)id;
	return translateStatus(status, paramErr);
}

OSStatus PasteboardCopyItemFlavors(PasteboardRef ref, PasteboardItemID item, CFArrayRef *types) {
	OSStatus err = requireSynchronized(ref);
	if (err != noErr) return err;
	if (types == NULL) return paramErr;
	*types = NULL;
	return translateStatus(CFPasteboardCopyFlavorsForItem(ref->pasteboard, (CFPasteboardItemID)(uintptr_t)item, types), paramErr);
}

OSStatus PasteboardGetItemFlavorFlags(PasteboardRef ref, PasteboardItemID item, CFStringRef type, PasteboardFlavorFlags *flags) {
	OSStatus err = requireSynchronized(ref);
	if (err != noErr) return err;
	CFPasteboardFlavorFlags value = 0;
	CFPasteboardStatus status = CFPasteboardGetDataFlags(ref->pasteboard, (CFPasteboardItemID)(uintptr_t)item, type, &value);
	if (status == kCFPasteboardSuccess && flags != NULL) *flags = (PasteboardFlavorFlags)value;
	return translateStatus(status, paramErr);
}

OSStatus PasteboardCopyItemFlavorData(PasteboardRef ref, PasteboardItemID item, CFStringRef type, CFDataRef *data) {
	OSStatus err = requireSynchronized(ref);
	if (err != noErr) return err;
	if (data == NULL) return paramErr;
	*data = NULL;
	return translateStatus(CFPasteboardCopyData(ref->pasteboard, (CFPasteboardItemID)(uintptr_t)item, type, data), badPasteboardFlavorErr);
}

// ---- promises ----

// Asks the promise keeper of the reference that made the promise
static CFPasteboardStatus callKeeper(CFPasteboardRef pasteboard, CFPasteboardItemID item, CFStringRef flavor, void *info) {
	PasteboardRef ref = (PasteboardRef)info;
	pthread_mutex_lock(&gLock);
	PasteboardPromiseKeeperProcPtr keeper = ref->keeper;
	void *context = ref->keeperContext;
	pthread_mutex_unlock(&gLock);
	if (keeper == NULL) return kCFPasteboardNoProvider;
	return keeper(ref, (PasteboardItemID)(uintptr_t)item, flavor, context) == noErr ? kCFPasteboardSuccess : kCFPasteboardNoProvider;
}

static void releaseKeeperRef(void *info) {
	CFRelease((PasteboardRef)info);
}

static const CFPasteboardProviderCallbacks gKeeperProvider = { 0, callKeeper, NULL, NULL, releaseKeeperRef };

OSStatus PasteboardSetPromiseKeeper(PasteboardRef ref, PasteboardPromiseKeeperProcPtr keeper, void *context) {
	pthread_mutex_lock(&gLock);
	ref->keeper = keeper;
	ref->keeperContext = context;
	if (keeper != NULL) {
		if (gKeeperRefs == NULL) gKeeperRefs = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
		if (CFArrayGetFirstIndexOfValue(gKeeperRefs, CFRangeMake(0, CFArrayGetCount(gKeeperRefs)), ref) == kCFNotFound) CFArrayAppendValue(gKeeperRefs, ref);
	}
	pthread_mutex_unlock(&gLock);
	if (keeper == NULL) keeperRefsRemove(ref);
	return noErr;
}

OSStatus PasteboardPutItemFlavor(PasteboardRef ref, PasteboardItemID item, CFStringRef type, CFDataRef data, PasteboardFlavorFlags flags) {
	if (type == NULL) return paramErr;
	CFPasteboardItemID id = (CFPasteboardItemID)(uintptr_t)item;
	CFIndex current = CFPasteboardGetChangeCount(ref->pasteboard);
	if (current == 0) return ioErr;
	pthread_mutex_lock(&gLock);
	Boolean stale = current != ref->seen;
	Boolean owner = clearedHere(ref, current);
	PasteboardPromiseKeeperProcPtr keeper = ref->keeper;
	pthread_mutex_unlock(&gLock);
	if (stale) return badPasteboardSyncErr;
	if (!owner) return notPasteboardOwnerErr;

	// a flavor that has data is not replaced; a promise may be (the keeper supplies the data this way)
	CFPasteboardFlavorFlags existing;
	if (CFPasteboardGetDataFlags(ref->pasteboard, id, type, &existing) == kCFPasteboardSuccess && !(existing & kCFPasteboardFlavorPromised)) {
		return duplicatePasteboardFlavorErr;
	}

	if (data != NULL) {
		return translateStatus(CFPasteboardSetData(ref->pasteboard, id, type, data, (CFPasteboardFlavorFlags)flags), paramErr);
	}
	if (keeper == NULL) return noPasteboardPromiseKeeperErr;
	// (the promise is made visible: macOS reports promises of the Pasteboard Manager as flagged Promised, unlike
	// NSPasteboard's)
	CFRetain(ref);   // released with the provider
	CFPasteboardStatus status = CFPasteboardPromiseData(ref->pasteboard, id, type, (CFPasteboardFlavorFlags)flags | kCFPasteboardFlavorPromiseVisible, &gKeeperProvider, ref);
	if (status != kCFPasteboardSuccess) CFRelease(ref);
	return translateStatus(status, noPasteboardPromiseKeeperErr);
}

OSStatus PasteboardResolvePromises(PasteboardRef ref) {
	if (ref != NULL) return translateStatus(CFPasteboardResolveAllPromisedData(ref->pasteboard), paramErr);

	// kPasteboardResolveAllPromises: every pasteboard this process made promises on
	CFArrayRef refs;
	pthread_mutex_lock(&gLock);
	refs = gKeeperRefs ? CFArrayCreateCopy(kCFAllocatorDefault, gKeeperRefs) : NULL;
	if (refs != NULL) {
		for (CFIndex i = 0; i < CFArrayGetCount(refs); ++i) CFRetain(CFArrayGetValueAtIndex(refs, i));
	}
	pthread_mutex_unlock(&gLock);
	if (refs == NULL) return noErr;
	for (CFIndex i = 0; i < CFArrayGetCount(refs); ++i) {
		PasteboardRef each = (PasteboardRef)CFArrayGetValueAtIndex(refs, i);
		CFPasteboardResolveAllPromisedData(each->pasteboard);
		CFRelease(each);
	}
	CFRelease(refs);
	return noErr;
}

// ---- paste location ----

OSStatus PasteboardCopyPasteLocation(PasteboardRef ref, CFURLRef *location) {
	if (location == NULL) return paramErr;
	*location = NULL;
	return translateStatus(CFPasteboardCopyPasteLocation(ref->pasteboard, location), paramErr);
}

OSStatus PasteboardSetPasteLocation(PasteboardRef ref, CFURLRef location) {
	return translateStatus(CFPasteboardSetPasteLocation(ref->pasteboard, location), paramErr);
}

// The CoreFoundation pasteboard behind the reference (not retained)
CFPasteboardRef PasteboardGetCFPasteboard(PasteboardRef ref) {
	return ref->pasteboard;
}
