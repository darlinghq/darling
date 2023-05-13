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


void* AudioFileClose(void) {
	if (verbose) LOG_FUNC("STUB: AudioFileClose called\n");
	return NULL;
};

void* AudioFileInitializeWithCallbacks(void) {
	if (verbose) LOG_FUNC("STUB: AudioFileInitializeWithCallbacks called\n");
	return NULL;
};

void* AudioFileOpenWithCallbacks(void) {
	if (verbose) LOG_FUNC("STUB: AudioFileOpenWithCallbacks called\n");
	return NULL;
};

void* AudioServicesPlaySystemSound(void) {
	if (verbose) LOG_FUNC("STUB: AudioServicesPlaySystemSound called\n");
	return NULL;
};

void* AudioUnitGetProperty(void) {
	if (verbose) LOG_FUNC("STUB: AudioUnitGetProperty called\n");
	return NULL;
};

void* AudioUnitSetParameter(void) {
	if (verbose) LOG_FUNC("STUB: AudioUnitSetParameter called\n");
	return NULL;
};

void* AudioUnitSetProperty(void) {
	if (verbose) LOG_FUNC("STUB: AudioUnitSetProperty called\n");
	return NULL;
};

void* ExtAudioFileCreateWithURL(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileCreateWithURL called\n");
	return NULL;
};

void* ExtAudioFileDispose(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileDispose called\n");
	return NULL;
};

void* ExtAudioFileGetProperty(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileGetProperty called\n");
	return NULL;
};

void* ExtAudioFileGetPropertyInfo(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileGetPropertyInfo called\n");
	return NULL;
};

void* ExtAudioFileRead(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileRead called\n");
	return NULL;
};

void* ExtAudioFileSetProperty(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileSetProperty called\n");
	return NULL;
};

void* ExtAudioFileWrapAudioFileID(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileWrapAudioFileID called\n");
	return NULL;
};

void* ExtAudioFileWrite(void) {
	if (verbose) LOG_FUNC("STUB: ExtAudioFileWrite called\n");
	return NULL;
};


