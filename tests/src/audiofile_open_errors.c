/*
 * The status AudioFileOpenURL, AudioFileOpenWithCallbacks and ExtAudioFileOpenURL return for what cannot be
 * opened: a file that is missing, empty or a directory is 'wht?' (kAudioFileUnspecifiedError); contents that are
 * not any kind of audio file are 'typ?' (kAudioFileUnsupportedFileTypeError), whatever the name of the file says; a file
 * that is recognized as a kind of file but is broken is 'dta?' (kAudioFileInvalidFileError). A file that is a
 * valid header with no samples opens.
 *
 * The expected values were observed on macOS (10.14 and 26) through these public functions.
 *
 * Exit status is 0 when every check passed.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

static char dir[64];

static void write_file(const char *name, const void *data, size_t size, char *path)
{
	snprintf(path, 256, "%s/%s", dir, name);
	FILE *f = fopen(path, "wb");
	if (f) {
		fwrite(data, 1, size, f);
		fclose(f);
	}
}

static CFURLRef url_for(const char *path)
{
	return CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)path, strlen(path), false);
}

static OSStatus open_url(const char *path)
{
	CFURLRef url = url_for(path);
	AudioFileID file = NULL;
	OSStatus status = AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file);
	if (status == noErr)
		AudioFileClose(file);
	CFRelease(url);
	return status;
}

static OSStatus open_ext(const char *path)
{
	CFURLRef url = url_for(path);
	ExtAudioFileRef file = NULL;
	OSStatus status = ExtAudioFileOpenURL(url, &file);
	if (status == noErr)
		ExtAudioFileDispose(file);
	CFRelease(url);
	return status;
}

typedef struct { const unsigned char *data; SInt64 size; } Memory;

static OSStatus mem_read(void *c, SInt64 pos, UInt32 request, void *buffer, UInt32 *actual)
{
	Memory *m = c;
	SInt64 n = pos >= m->size ? 0 : (m->size - pos < request ? m->size - pos : request);
	if (n)
		memcpy(buffer, m->data + pos, (size_t)n);
	*actual = (UInt32)n;
	return noErr;
}

static OSStatus mem_write(void *c, SInt64 p, UInt32 r, const void *b, UInt32 *a) { return kAudioFilePermissionsError; }
static SInt64 mem_size(void *c) { return ((Memory *)c)->size; }
static OSStatus mem_set_size(void *c, SInt64 s) { return kAudioFilePermissionsError; }

static OSStatus open_memory(const void *data, size_t size)
{
	Memory m = { data, (SInt64)size };
	AudioFileID file = NULL;
	OSStatus status = AudioFileOpenWithCallbacks(&m, mem_read, mem_write, mem_size, mem_set_size, 0, &file);
	if (status == noErr)
		AudioFileClose(file);
	return status;
}

#define BOTH(path, expected, what) do { \
	OSStatus a = open_url(path), b = open_ext(path); \
	CHECK(a == (expected) && b == (expected), "%s: AudioFileOpenURL %d, ExtAudioFileOpenURL %d, expected %d", what, (int)a, (int)b, (int)(expected)); \
} while (0)

int main(void)
{
	char path[256];

	snprintf(dir, sizeof(dir), "/tmp/afopen.XXXXXX");
	if (!mkdtemp(dir)) {
		printf("FAIL mkdtemp\n");
		return 1;
	}

	snprintf(path, sizeof(path), "%s/missing.wav", dir);
	BOTH(path, kAudioFileUnspecifiedError, "a missing file");

	write_file("empty.wav", "", 0, path);
	BOTH(path, kAudioFileUnspecifiedError, "an empty file");

	BOTH(dir, kAudioFileUnspecifiedError, "a directory");

	static const char text[] = "this is not audio, just some text";
	write_file("text.wav", text, sizeof(text) - 1, path);
	BOTH(path, kAudioFileUnsupportedFileTypeError, "text named .wav");

	unsigned char noise[30000];
	unsigned x = 12345;
	for (size_t i = 0; i < sizeof(noise); i++) {
		x = x * 1103515245u + 12345u;
		noise[i] = (unsigned char)(x >> 16);
	}
	write_file("noise.wav", noise, sizeof(noise), path);
	BOTH(path, kAudioFileUnsupportedFileTypeError, "noise named .wav");
	write_file("noise.m4a", noise, sizeof(noise), path);
	BOTH(path, kAudioFileUnsupportedFileTypeError, "noise named .m4a");

	// A file of the MPEG-4 kind (it has the type box) with no movie box in it
	static const unsigned char mp4[] = { 0, 0, 0, 0x18, 'f', 't', 'y', 'p', 'M', '4', 'A', ' ', 0, 0, 0, 0, 'M', '4', 'A', ' ', 'm', 'p', '4', '2',
		0, 0, 0, 0x10, 'm', 'd', 'a', 't', '1', '2', '3', '4', '5', '6', '7', '8' };
	write_file("ftyp-only.m4a", mp4, sizeof(mp4), path);
#ifdef __i386__
	// The MPEG-4 container reader is the host's FFmpeg, which 32-bit code does not have
	printf("SKIP an MPEG-4 file without a movie\n");
#else
	BOTH(path, kAudioFileInvalidFileError, "an MPEG-4 file without a movie");
#endif

	// A WAVE header (16-bit stereo, 44.1 kHz) with an empty data chunk is a sound of no length
	static const unsigned char wav[] = { 'R', 'I', 'F', 'F', 0x24, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 2, 0,
		0x44, 0xac, 0, 0, 0x10, 0xb1, 2, 0, 4, 0, 16, 0, 'd', 'a', 't', 'a', 0, 0, 0, 0 };
	write_file("nodata.wav", wav, sizeof(wav), path);
	BOTH(path, noErr, "a WAVE header without samples");

	// The same contents handed over as memory: nothing there is not a type either
	CHECK(open_memory("", 0) == kAudioFileUnsupportedFileTypeError, "empty memory");
	CHECK(open_memory(text, sizeof(text) - 1) == kAudioFileUnsupportedFileTypeError, "text in memory");
	CHECK(open_memory(noise, sizeof(noise)) == kAudioFileUnsupportedFileTypeError, "noise in memory");
	CHECK(open_memory(wav, sizeof(wav)) == noErr, "a WAVE header in memory");

	printf("%s (%d failed)\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 1 : 0;
}
