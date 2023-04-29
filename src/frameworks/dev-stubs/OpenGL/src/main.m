/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
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

#import <objc/NSObject.h>

@interface NSString : NSObject
@end

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf


void* CGLBackDispatch(void) {
	if (verbose) LOG_FUNC("STUB: CGLBackDispatch called\n");
	return NULL;
};

void* CGLChoosePixelFormat(void) {
	if (verbose) LOG_FUNC("STUB: CGLChoosePixelFormat called\n");
	return NULL;
};

void* CGLCreateContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLCreateContext called\n");
	return NULL;
};

void* CGLDestroyContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLDestroyContext called\n");
	return NULL;
};

void* CGLDestroyPixelFormat(void) {
	if (verbose) LOG_FUNC("STUB: CGLDestroyPixelFormat called\n");
	return NULL;
};

void* CGLFrontDispatch(void) {
	if (verbose) LOG_FUNC("STUB: CGLFrontDispatch called\n");
	return NULL;
};

void* CGLGetCurrentContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLGetCurrentContext called\n");
	return NULL;
};

void* CGLGetParameter(void) {
	if (verbose) LOG_FUNC("STUB: CGLGetParameter called\n");
	return NULL;
};

void* CGLGetShareGroup(void) {
	if (verbose) LOG_FUNC("STUB: CGLGetShareGroup called\n");
	return NULL;
};

void* CGLGetVirtualScreen(void) {
	if (verbose) LOG_FUNC("STUB: CGLGetVirtualScreen called\n");
	return NULL;
};

void* CGLLockContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLLockContext called\n");
	return NULL;
};

void* CGLReleaseContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLReleaseContext called\n");
	return NULL;
};

void* CGLReleasePixelFormat(void) {
	if (verbose) LOG_FUNC("STUB: CGLReleasePixelFormat called\n");
	return NULL;
};

void* CGLRetainContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLRetainContext called\n");
	return NULL;
};

void* CGLSetCurrentContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLSetCurrentContext called\n");
	return NULL;
};

void* CGLSetParameter(void) {
	if (verbose) LOG_FUNC("STUB: CGLSetParameter called\n");
	return NULL;
};

void* CGLTexImageIOSurface2D(void) {
	if (verbose) LOG_FUNC("STUB: CGLTexImageIOSurface2D called\n");
	return NULL;
};

void* CGLUnlockContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLUnlockContext called\n");
	return NULL;
};

void* CGLUpdateContext(void) {
	if (verbose) LOG_FUNC("STUB: CGLUpdateContext called\n");
	return NULL;
};

void* glBegin(void) {
	if (verbose) LOG_FUNC("STUB: glBegin called\n");
	return NULL;
};

void* glBindFramebufferEXT(void) {
	if (verbose) LOG_FUNC("STUB: glBindFramebufferEXT called\n");
	return NULL;
};

void* glBindTexture(void) {
	if (verbose) LOG_FUNC("STUB: glBindTexture called\n");
	return NULL;
};

void* glBlendFunc(void) {
	if (verbose) LOG_FUNC("STUB: glBlendFunc called\n");
	return NULL;
};

void* glCheckFramebufferStatusEXT(void) {
	if (verbose) LOG_FUNC("STUB: glCheckFramebufferStatusEXT called\n");
	return NULL;
};

void* glClear(void) {
	if (verbose) LOG_FUNC("STUB: glClear called\n");
	return NULL;
};

void* glClearColor(void) {
	if (verbose) LOG_FUNC("STUB: glClearColor called\n");
	return NULL;
};

void* glColor4f(void) {
	if (verbose) LOG_FUNC("STUB: glColor4f called\n");
	return NULL;
};

void* glDisable(void) {
	if (verbose) LOG_FUNC("STUB: glDisable called\n");
	return NULL;
};

void* glDrawBuffer(void) {
	if (verbose) LOG_FUNC("STUB: glDrawBuffer called\n");
	return NULL;
};

void* glEnable(void) {
	if (verbose) LOG_FUNC("STUB: glEnable called\n");
	return NULL;
};

void* glEnd(void) {
	if (verbose) LOG_FUNC("STUB: glEnd called\n");
	return NULL;
};

void* glFinish(void) {
	if (verbose) LOG_FUNC("STUB: glFinish called\n");
	return NULL;
};

void* glFramebufferTexture2DEXT(void) {
	if (verbose) LOG_FUNC("STUB: glFramebufferTexture2DEXT called\n");
	return NULL;
};

void* glGenFramebuffersEXT(void) {
	if (verbose) LOG_FUNC("STUB: glGenFramebuffersEXT called\n");
	return NULL;
};

void* glGenTextures(void) {
	if (verbose) LOG_FUNC("STUB: glGenTextures called\n");
	return NULL;
};

void* glGetBooleanv(void) {
	if (verbose) LOG_FUNC("STUB: glGetBooleanv called\n");
	return NULL;
};

void* glGetError(void) {
	if (verbose) LOG_FUNC("STUB: glGetError called\n");
	return NULL;
};

void* glGetIntegerv(void) {
	if (verbose) LOG_FUNC("STUB: glGetIntegerv called\n");
	return NULL;
};

void* glLineWidth(void) {
	if (verbose) LOG_FUNC("STUB: glLineWidth called\n");
	return NULL;
};

void* glLoadIdentity(void) {
	if (verbose) LOG_FUNC("STUB: glLoadIdentity called\n");
	return NULL;
};

void* glMatrixMode(void) {
	if (verbose) LOG_FUNC("STUB: glMatrixMode called\n");
	return NULL;
};

void* glOrtho(void) {
	if (verbose) LOG_FUNC("STUB: glOrtho called\n");
	return NULL;
};

void* glPixelStorei(void) {
	if (verbose) LOG_FUNC("STUB: glPixelStorei called\n");
	return NULL;
};

void* glPointSize(void) {
	if (verbose) LOG_FUNC("STUB: glPointSize called\n");
	return NULL;
};

void* glPopMatrix(void) {
	if (verbose) LOG_FUNC("STUB: glPopMatrix called\n");
	return NULL;
};

void* glPushMatrix(void) {
	if (verbose) LOG_FUNC("STUB: glPushMatrix called\n");
	return NULL;
};

void* glReadPixels(void) {
	if (verbose) LOG_FUNC("STUB: glReadPixels called\n");
	return NULL;
};

void* glRotatef(void) {
	if (verbose) LOG_FUNC("STUB: glRotatef called\n");
	return NULL;
};

void* glSwapAPPLE(void) {
	if (verbose) LOG_FUNC("STUB: glSwapAPPLE called\n");
	return NULL;
};

void* glTexCoord2f(void) {
	if (verbose) LOG_FUNC("STUB: glTexCoord2f called\n");
	return NULL;
};

void* glTexImage2D(void) {
	if (verbose) LOG_FUNC("STUB: glTexImage2D called\n");
	return NULL;
};

void* glTexParameterfv(void) {
	if (verbose) LOG_FUNC("STUB: glTexParameterfv called\n");
	return NULL;
};

void* glTexParameteri(void) {
	if (verbose) LOG_FUNC("STUB: glTexParameteri called\n");
	return NULL;
};

void* glTranslatef(void) {
	if (verbose) LOG_FUNC("STUB: glTranslatef called\n");
	return NULL;
};

void* glUniform1f(void) {
	if (verbose) LOG_FUNC("STUB: glUniform1f called\n");
	return NULL;
};

void* glUniform1i(void) {
	if (verbose) LOG_FUNC("STUB: glUniform1i called\n");
	return NULL;
};

void* glUniform2f(void) {
	if (verbose) LOG_FUNC("STUB: glUniform2f called\n");
	return NULL;
};

void* glUniform3f(void) {
	if (verbose) LOG_FUNC("STUB: glUniform3f called\n");
	return NULL;
};

void* glUniform4f(void) {
	if (verbose) LOG_FUNC("STUB: glUniform4f called\n");
	return NULL;
};

void* glUniformMatrix2x3fv(void) {
	if (verbose) LOG_FUNC("STUB: glUniformMatrix2x3fv called\n");
	return NULL;
};

void* glUniformMatrix4x3fv(void) {
	if (verbose) LOG_FUNC("STUB: glUniformMatrix4x3fv called\n");
	return NULL;
};

void* glVertex2f(void) {
	if (verbose) LOG_FUNC("STUB: glVertex2f called\n");
	return NULL;
};

void* glVertex3f(void) {
	if (verbose) LOG_FUNC("STUB: glVertex3f called\n");
	return NULL;
};

void* glViewport(void) {
	if (verbose) LOG_FUNC("STUB: glViewport called\n");
	return NULL;
};


