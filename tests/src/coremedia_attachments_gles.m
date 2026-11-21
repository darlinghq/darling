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

// The CMAttachment functions on a CVOpenGLESTexture, which starts with a CVBuffer like a CVPixelBuffer does
// (an attachment set with CMSetAttachment is the CVBuffer attachment, and it propagates to another buffer).
// Needs an EAGL context, like corevideo_gles_texture_cache.m. Prints PASS when everything holds; the exit
// code is the number of failed checks.

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CVPixelBuffer.h>
#import <CoreVideo/CVOpenGLESTextureCache.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>

#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

int main(void)
{
	@autoreleasepool {
		EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
		if (context == nil)
			context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		CHECK(context != nil);
		if (context == nil)
			return 1;
		CVOpenGLESTextureCacheRef cache = NULL;
		CVPixelBufferRef buffer = NULL, other = NULL;
		CVOpenGLESTextureRef texture = NULL;
		CHECK(CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, context, NULL, &cache) == kCVReturnSuccess);
		NSDictionary *attrs = @{ (id)kCVPixelBufferBytesPerRowAlignmentKey: @64 };
		CHECK(CVPixelBufferCreate(kCFAllocatorDefault, 4, 2, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef)attrs, &buffer) == kCVReturnSuccess);
		CHECK(CVPixelBufferCreate(kCFAllocatorDefault, 4, 2, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef)attrs, &other) == kCVReturnSuccess);
		if (cache == NULL || buffer == NULL || other == NULL)
			return 1;
		CHECK(CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, cache, buffer, NULL,
			GL_TEXTURE_2D, GL_RGBA, 4, 2, GL_RGBA, GL_UNSIGNED_BYTE, 0, &texture) == kCVReturnSuccess);
		if (texture == NULL)
			return 1;

		CFStringRef key = CFSTR("test.attachment.texture");
		CMAttachmentMode mode = 99;
		CMSetAttachment(texture, key, CFSTR("value"), kCMAttachmentMode_ShouldPropagate);
		CFTypeRef found = CVBufferGetAttachment(texture, key, NULL);
		CHECK(found != NULL && CFEqual(found, CFSTR("value")));
		found = CMGetAttachment(texture, key, &mode);
		CHECK(found != NULL && CFEqual(found, CFSTR("value")) && mode == kCMAttachmentMode_ShouldPropagate);
		CMPropagateAttachments(texture, other);
		found = CMGetAttachment(other, key, NULL);
		CHECK(found != NULL && CFEqual(found, CFSTR("value")));
		CMRemoveAllAttachments(texture);
		CHECK(CMGetAttachment(texture, key, NULL) == NULL);

		CFRelease(texture);
		CFRelease(buffer);
		CFRelease(other);
		CFRelease(cache);
	}
	printf(failures ? "FAILED\n" : "PASS\n");
	return failures;
}
