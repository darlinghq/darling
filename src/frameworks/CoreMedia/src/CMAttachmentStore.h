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

#ifndef _CMATTACHMENTSTORE_H_
#define _CMATTACHMENTSTORE_H_

#include <CoreFoundation/CoreFoundation.h>

// Attachments of a CoreMedia object, kept in one dictionary per propagation mode. An object that can
// bear attachments embeds a zero-initialised store, releases it in its finalizer, and hands it to
// CMCoreGetAttachmentStore. Not part of the public interface.
typedef struct {
	CFMutableDictionaryRef propagating;
	CFMutableDictionaryRef local;
} CMAttachmentStore;

__attribute__((visibility("hidden"))) void CMAttachmentStoreRelease(CMAttachmentStore *store);

// The store of a CMSampleBuffer or CMBlockBuffer, or NULL for any other object.
__attribute__((visibility("hidden"))) CMAttachmentStore *CMCoreGetAttachmentStore(CFTypeRef object);

#endif
