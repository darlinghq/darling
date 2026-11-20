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

#ifndef _CMATTACHMENT_H_
#define _CMATTACHMENT_H_

#include <CoreMedia/CMBase.h>

CF_IMPLICIT_BRIDGING_ENABLED

// Any CF object that supports the attachment functions below. CMSampleBuffer, CMBlockBuffer and the
// CVBuffer classes CVPixelBuffer, CVOpenGLTexture and CVOpenGLESTexture do;
// on any other object the functions do nothing and report no attachments.
typedef CFTypeRef CMAttachmentBearerRef;

// Whether an attachment is copied to another bearer by CMPropagateAttachments. The values are those of
// CVAttachmentMode, which the CMAttachment functions are documented to be equivalent to on a CVBuffer.
typedef CF_ENUM(uint32_t, CMAttachmentMode) {
	kCMAttachmentMode_ShouldNotPropagate = 0,
	kCMAttachmentMode_ShouldPropagate = 1,
};

CM_EXPORT void CMSetAttachment(CMAttachmentBearerRef target, CFStringRef key, CFTypeRef value,
	CMAttachmentMode attachmentMode);
CM_EXPORT CFTypeRef CMGetAttachment(CMAttachmentBearerRef target, CFStringRef key,
	CMAttachmentMode *attachmentModeOut);
CM_EXPORT void CMRemoveAttachment(CMAttachmentBearerRef target, CFStringRef key);
CM_EXPORT void CMRemoveAllAttachments(CMAttachmentBearerRef target);
CM_EXPORT CF_RETURNS_RETAINED CFDictionaryRef CMCopyDictionaryOfAttachments(CFAllocatorRef allocator,
	CMAttachmentBearerRef target, CMAttachmentMode attachmentMode);
CM_EXPORT void CMSetAttachments(CMAttachmentBearerRef target, CFDictionaryRef theAttachments,
	CMAttachmentMode attachmentMode);
CM_EXPORT void CMPropagateAttachments(CMAttachmentBearerRef source, CMAttachmentBearerRef destination);

CF_IMPLICIT_BRIDGING_DISABLED

#endif
