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

// The CMAttachment functions on a CVOpenGLTexture, which starts with a CVBuffer like a CVPixelBuffer does:
// an attachment set with CMSetAttachment is the CVBuffer attachment, and it propagates to another buffer.
// Needs an offscreen OpenGL context, like corevideo_opengl_texture_cache.c. Prints PASS when everything
// holds; the exit code is the number of failed checks.

#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <OpenGL/OpenGL.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static void onAlarm(int sig)
{
	(void)sig;
	static const char message[] = "FAIL: timed out\n";
	write(1, message, sizeof(message) - 1);
	_exit(1);
}

int main(void)
{
	signal(SIGALRM, onAlarm);
	alarm(60);

	CGLPixelFormatAttribute formatAttributes[] = { 0 };
	CGLPixelFormatObj pixelFormat = NULL;
	GLint screens = 0;
	CGLContextObj context = NULL;
	if (CGLChoosePixelFormat(formatAttributes, &pixelFormat, &screens) != kCGLNoError
		|| CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError
		|| CGLSetCurrentContext(context) != kCGLNoError) {
		printf("FAIL: no offscreen OpenGL context\n");
		return 1;
	}
	CVOpenGLTextureCacheRef cache = NULL;
	CVPixelBufferRef buffer = NULL, other = NULL;
	CVOpenGLTextureRef texture = NULL;
	CHECK(CVOpenGLTextureCacheCreate(NULL, NULL, context, pixelFormat, NULL, &cache) == kCVReturnSuccess);
	CHECK(CVPixelBufferCreate(NULL, 8, 4, kCVPixelFormatType_32BGRA, NULL, &buffer) == kCVReturnSuccess);
	CHECK(CVPixelBufferCreate(NULL, 8, 4, kCVPixelFormatType_32BGRA, NULL, &other) == kCVReturnSuccess);
	if (!cache || !buffer || !other)
		return 1;
	CHECK(CVOpenGLTextureCacheCreateTextureFromImage(NULL, cache, buffer, NULL, &texture) == kCVReturnSuccess);
	if (!texture)
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
	CVPixelBufferRelease(buffer);
	CVPixelBufferRelease(other);
	CVOpenGLTextureCacheRelease(cache);
	CGLSetCurrentContext(NULL);
	CGLReleasePixelFormat(pixelFormat);
	CGLDestroyContext(context);

	printf(failures ? "FAILED\n" : "PASS\n");
	return failures;
}
