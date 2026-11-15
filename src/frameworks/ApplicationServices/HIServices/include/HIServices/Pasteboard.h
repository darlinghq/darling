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

// The Pasteboard Manager: the Carbon-era interface to the pasteboards (clipboard, find, drag and named
// pasteboards). Declarations follow the documented Pasteboard Manager reference.

#ifndef _HISERVICES_PASTEBOARD_H_
#define _HISERVICES_PASTEBOARD_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

CF_ASSUME_NONNULL_BEGIN

typedef struct CF_BRIDGED_TYPE(id) OpaquePasteboardRef *PasteboardRef;

// An item is identified by an opaque value that is unique within the pasteboard's current contents.
typedef void *PasteboardItemID;

enum {
	badPasteboardSyncErr = -25130,          // the pasteboard changed and must be synchronized first
	badPasteboardIndexErr = -25131,         // no item at that index
	badPasteboardItemErr = -25132,          // no such item
	badPasteboardFlavorErr = -25133,        // the item has no such flavor
	duplicatePasteboardFlavorErr = -25134,  // the item has the flavor already
	notPasteboardOwnerErr = -25135,         // the caller did not clear the pasteboard
	noPasteboardPromiseKeeperErr = -25136   // a promise was made but no promise keeper is set
};

typedef CF_OPTIONS(OptionBits, PasteboardSyncFlags) {
	kPasteboardModified = (1 << 0),         // the contents changed since the last synchronization
	kPasteboardClientIsOwner = (1 << 1)     // this reference cleared the pasteboard and nothing changed it since
};

#define kPasteboardTypeFileURLPromise CFSTR("com.apple.pasteboard.promised-file-url")
#define kPasteboardTypeFilePromiseContent CFSTR("com.apple.pasteboard.promised-file-content-type")

typedef CF_OPTIONS(OptionBits, PasteboardFlavorFlags) {
	kPasteboardFlavorNoFlags = 0,
	kPasteboardFlavorSenderOnly = (1 << 0),
	kPasteboardFlavorSenderTranslated = (1 << 1),
	kPasteboardFlavorNotSaved = (1 << 2),
	kPasteboardFlavorRequestOnly = (1 << 3),
	kPasteboardFlavorSystemTranslated = (1 << 8),
	kPasteboardFlavorPromised = (1 << 9)
};

typedef CF_ENUM(OSType, PasteboardStandardLocation) {
	kPasteboardStandardLocationTrash = 'trsh',
	kPasteboardStandardLocationUnknown = 'unkn'
};

extern CFTypeID PasteboardGetTypeID(void);

#define kPasteboardClipboard CFSTR("com.apple.pasteboard.clipboard")
#define kPasteboardFind CFSTR("com.apple.pasteboard.find")
#define kPasteboardUniqueName ((CFStringRef)NULL)

// A NULL name (kPasteboardUniqueName) creates a pasteboard of its own.
extern OSStatus PasteboardCreate(CFStringRef _Nullable inName, PasteboardRef _Nullable * _Nonnull outPasteboard CF_RETURNS_RETAINED);
extern PasteboardSyncFlags PasteboardSynchronize(PasteboardRef inPasteboard);
// Removes all items and makes the caller the owner: only then may flavors be put.
extern OSStatus PasteboardClear(PasteboardRef inPasteboard);
extern OSStatus PasteboardCopyName(PasteboardRef inPasteboard, CFStringRef _Nullable * _Nonnull outName CF_RETURNS_RETAINED);
extern OSStatus PasteboardGetItemCount(PasteboardRef inPasteboard, ItemCount *outItemCount);
// Indices start at 1.
extern OSStatus PasteboardGetItemIdentifier(PasteboardRef inPasteboard, CFIndex inIndex, PasteboardItemID _Nullable * _Nonnull outItem);
extern OSStatus PasteboardCopyItemFlavors(PasteboardRef inPasteboard, PasteboardItemID inItem, CFArrayRef _Nullable * _Nonnull outFlavorTypes CF_RETURNS_RETAINED);
extern OSStatus PasteboardGetItemFlavorFlags(PasteboardRef inPasteboard, PasteboardItemID inItem, CFStringRef inFlavorType, PasteboardFlavorFlags *outFlags);
extern OSStatus PasteboardCopyItemFlavorData(PasteboardRef inPasteboard, PasteboardItemID inItem, CFStringRef inFlavorType, CFDataRef _Nullable * _Nonnull outData CF_RETURNS_RETAINED);

// Passed as the data of PasteboardPutItemFlavor to promise the flavor.
#define kPasteboardPromisedData ((CFDataRef)NULL)

// Adds a flavor to an item (the item is created by the first flavor put under its identifier).
extern OSStatus PasteboardPutItemFlavor(PasteboardRef inPasteboard, PasteboardItemID inItem, CFStringRef inFlavorType, CFDataRef _Nullable inData, PasteboardFlavorFlags inFlags);

extern OSStatus PasteboardCopyPasteLocation(PasteboardRef inPasteboard, CFURLRef _Nullable * _Nonnull outPasteLocation CF_RETURNS_RETAINED);
extern OSStatus PasteboardSetPasteLocation(PasteboardRef inPasteboard, CFURLRef inPasteLocation);

typedef OSStatus (*PasteboardPromiseKeeperProcPtr)(PasteboardRef pasteboard, PasteboardItemID item, CFStringRef flavorType, void * _Nullable context);
extern OSStatus PasteboardSetPromiseKeeper(PasteboardRef inPasteboard, PasteboardPromiseKeeperProcPtr inPromiseKeeper, void * _Nullable inContext);

#define kPasteboardResolveAllPromises ((PasteboardRef)NULL)
extern OSStatus PasteboardResolvePromises(PasteboardRef inPasteboard);

CF_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif

#endif
