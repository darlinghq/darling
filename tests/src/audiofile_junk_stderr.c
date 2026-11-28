/*
 * Opening data that is not audio, or is damaged audio, through AudioFileOpenWithCallbacks, AudioFileOpenURL and
 * ExtAudioFileOpenURL returns the status macOS returns, and writes nothing to stderr: a library reports problems
 * in its results, not on the process's error stream (the FFmpeg libraries that read the MPEG-4 family used to print
 * "moov atom not found" and the like).
 *
 * Also the statuses of AudioFileGetGlobalInfo(Size) for types and properties that do not exist, and of the
 * functions given no file.
 *
 * The expected values were observed on macOS 26.7.3 through these public functions; stderr was empty there too.
 *
 * Exit status is 0 when every check passed.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef paramErr
#define paramErr (-50)
#endif

static int failures;
static char dir[64];
static int errfd, savefd;

static void cap_begin(void) { fflush(stderr); ftruncate(errfd, 0); lseek(errfd, 0, SEEK_SET); dup2(errfd, 2); }
static long cap_end(void) { fflush(stderr); dup2(savefd, 2); return (long)lseek(errfd, 0, SEEK_END); }

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

typedef struct { const unsigned char *data; SInt64 size; } Mem;
static OSStatus rd(void *c, SInt64 pos, UInt32 req, void *buf, UInt32 *act)
{
	Mem *m = c;
	SInt64 n = pos >= m->size ? 0 : (m->size - pos < req ? m->size - pos : req);
	if (n)
		memcpy(buf, m->data + pos, (size_t)n);
	*act = (UInt32)n;
	return noErr;
}
static OSStatus wr(void *c, SInt64 p, UInt32 r, const void *b, UInt32 *a) { return kAudioFilePermissionsError; }
static SInt64 sz(void *c) { return ((Mem *)c)->size; }
static OSStatus ssz(void *c, SInt64 s) { return kAudioFilePermissionsError; }
static OSStatus failing_status = kAudioFileUnspecifiedError;
static OSStatus rd_fail(void *c, SInt64 pos, UInt32 req, void *buf, UInt32 *act) { *act = 0; return failing_status; }

#define ANY 1 /* not checked: the status is not pinned for this entry point */

static void run(const char *name, const void *data, size_t size, AudioFileTypeID hint, OSStatus callbacks, OSStatus url, OSStatus ext)
{
	char path[300];
	snprintf(path, sizeof path, "%s/%s", dir, name);
	FILE *f = fopen(path, "wb");
	if (size)
		fwrite(data, 1, size, f);
	fclose(f);

	Mem m = { data, (SInt64)size };
	AudioFileID af = NULL;
	cap_begin();
	OSStatus a = AudioFileOpenWithCallbacks(&m, rd, wr, sz, ssz, hint, &af);
	OSStatus closed = noErr;
	if (!a)
		closed = AudioFileClose(af);
	long e1 = cap_end();

	CFURLRef u = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)path, strlen(path), false);
	af = NULL;
	cap_begin();
	OSStatus b = AudioFileOpenURL(u, kAudioFileReadPermission, hint, &af);
	if (!b)
		AudioFileClose(af);
	long e2 = cap_end();

	ExtAudioFileRef ef = NULL;
	cap_begin();
	OSStatus c = ExtAudioFileOpenURL(u, &ef);
	if (!c)
		ExtAudioFileDispose(ef);
	long e3 = cap_end();
	CFRelease(u);

	CHECK((callbacks == ANY || a == callbacks) && (url == ANY || b == url) && (ext == ANY || c == ext),
		"%s: callbacks %d (expected %d), URL %d (expected %d), Ext %d (expected %d)", name, (int)a, (int)callbacks, (int)b, (int)url, (int)c, (int)ext);
	CHECK(e1 == 0 && e2 == 0 && e3 == 0, "%s: %ld, %ld and %ld bytes on stderr", name, e1, e2, e3);
	/* closing a file that was only read succeeds, although the write function of the caller cannot write */
	CHECK(closed == noErr, "%s: AudioFileClose %d", name, (int)closed);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	snprintf(dir, sizeof dir, "/tmp/afjunk.XXXXXX");
	if (!mkdtemp(dir)) {
		printf("FAIL mkdtemp\n");
		return 1;
	}
	char errp[100];
	snprintf(errp, sizeof errp, "%s/err", dir);
	errfd = open(errp, O_RDWR | O_CREAT | O_TRUNC, 0600);
	savefd = dup(2);

	unsigned char noise[30000];
	unsigned x = 12345;
	for (size_t i = 0; i < sizeof noise; i++) {
		x = x * 1103515245u + 12345u;
		noise[i] = (unsigned char)(x >> 16);
	}
	/* 16-bit stereo 44.1 kHz WAVE with 8 bytes of samples */
	static const unsigned char wav[] = { 'R','I','F','F', 0x2c,0,0,0, 'W','A','V','E','f','m','t',' ',16,0,0,0,1,0,2,0,
		0x44,0xac,0,0,0x10,0xb1,2,0,4,0,16,0,'d','a','t','a',8,0,0,0, 1,2,3,4,5,6,7,8 };
	static const unsigned char mp4[] = { 0,0,0,0x18,'f','t','y','p','M','4','A',' ',0,0,0,0,'M','4','A',' ','m','p','4','2',
		0,0,0,0x10,'m','d','a','t','1','2','3','4','5','6','7','8' };
	static const unsigned char mp4ftyp[] = { 0,0,0,0x18,'f','t','y','p','M','4','A',' ',0,0,0,0,'M','4','A',' ','m','p','4','2' };

	const OSStatus typ = kAudioFileUnsupportedFileTypeError, dta = kAudioFileInvalidFileError, wht = kAudioFileUnspecifiedError;

	run("empty", "", 0, 0, typ, wht, wht);
	run("random4", noise, 4, 0, typ, typ, typ);
	run("random100", noise, 100, 0, typ, typ, typ);
	run("random30000", noise, sizeof noise, 0, typ, typ, typ);
	run("random-hint-wave", noise, sizeof noise, kAudioFileWAVEType, typ, typ, typ);
	run("wav4", wav, 4, 0, typ, typ, typ);
	run("wav12", wav, 12, 0, dta, dta, dta);
	run("wav20", wav, 20, 0, dta, dta, dta);
	run("wav-whole-header-no-data-chunk", wav, 36, 0, noErr, noErr, noErr);
	run("wav-data-chunk-cut", wav, 48 - 8, 0, ANY, noErr, noErr);
	run("wav", wav, sizeof wav, 0, noErr, noErr, noErr);
	/* a type hint does not make a file another kind of file */
	run("wav-hint-wave", wav, sizeof wav, kAudioFileWAVEType, noErr, noErr, noErr);
	run("wav-hint-mp3", wav, sizeof wav, kAudioFileMP3Type, noErr, noErr, noErr);
	run("wav-hint-aiff", wav, sizeof wav, kAudioFileAIFFType, noErr, noErr, noErr);
	run("wav-hint-caf", wav, sizeof wav, kAudioFileCAFType, noErr, noErr, noErr);
	run("wav-hint-bogus", wav, sizeof wav, 0x78787878, noErr, noErr, noErr);
#ifdef __i386__
	printf("SKIP MPEG-4 files without a movie box: the MPEG-4 reader is the host's FFmpeg, which 32-bit code does not have\n");
#else
	run("mp4-no-moov", mp4, sizeof mp4, 0, ANY, dta, dta);
	run("mp4-no-moov-hint-m4a", mp4, sizeof mp4, kAudioFileM4AType, ANY, dta, dta);
	run("mp4-no-moov-hint-wave", mp4, sizeof mp4, kAudioFileWAVEType, ANY, dta, dta);
	run("mp4-ftyp-only", mp4ftyp, sizeof mp4ftyp, 0, ANY, dta, dta);
#endif

	AudioFileID af = NULL;
	OSStatus s;
	long e;
	(void)e;

	Mem w = { wav, sizeof wav };
	s = AudioFileOpenWithCallbacks(&w, rd, wr, sz, ssz, 0, NULL);
	CHECK(s == paramErr, "no place for the file: %d", (int)s);

	/* AudioFileGetGlobalInfo and its Size variant */
	UInt32 size = 0;
	AudioFileTypeID t = 0x78787878;
	s = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ExtensionsForType, 0, NULL, &size);
	(void)s; /* the status for a missing specifier is not pinned */
	s = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size);
	CHECK(s == noErr && size > 0 && size % sizeof(UInt32) == 0, "ReadableTypes size: %d, %u", (int)s, (unsigned)size);
	s = AudioFileGetGlobalInfoSize(0x78787878, 0, NULL, &size);
	CHECK(s == wht, "a property that does not exist: %d", (int)s);

	CFArrayRef arr = NULL;
	CFStringRef str = NULL;
	size = sizeof arr;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, &size, &arr);
	CHECK(s == typ, "extensions of a type that does not exist: %d", (int)s);
	size = sizeof str;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_FileTypeName, sizeof t, &t, &size, &str);
	CHECK(s == typ, "name of a type that does not exist: %d", (int)s);
	t = kAudioFileWAVEType;
	size = 2;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, &size, &arr);
	CHECK(s == kAudioFileBadPropertySizeError, "extensions with a buffer that is too small: %d", (int)s);
	size = sizeof arr;
	arr = NULL;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, &size, &arr);
	CHECK(s == noErr && arr && CFArrayGetCount(arr) > 0, "extensions of WAVE: %d", (int)s);
	if (arr)
		CFRelease(arr);

	/* a failing read function: its error is the result, whatever it is */
	{
		static const OSStatus errs[] = { kAudioFileUnspecifiedError, kAudioFilePermissionsError, kAudioFileInvalidFileError,
			kAudioFileUnsupportedFileTypeError, paramErr, -36, 1234 };
		for (size_t i = 0; i < sizeof errs / sizeof errs[0]; i++) {
			Mem fm = { wav, sizeof wav };
			failing_status = errs[i];
			af = NULL;
			s = AudioFileOpenWithCallbacks(&fm, rd_fail, wr, sz, ssz, 0, &af);
			CHECK(s == errs[i], "read function failing with %d: %d", (int)errs[i], (int)s);
			s = AudioFileOpenWithCallbacks(&fm, rd_fail, wr, sz, ssz, kAudioFileWAVEType, &af);
			CHECK(s == errs[i], "read function failing with %d, type hint: %d", (int)errs[i], (int)s);
		}
	}

	/* a size that is not credible (a size too large for the data is not tested: the read function would have to supply it) */
	{
		Mem neg = { wav, -1 };
		s = AudioFileOpenWithCallbacks(&neg, rd, wr, sz, ssz, 0, &af);
		CHECK(s == typ, "size -1: %d", (int)s);
	}

	/* the global information: the size query ignores a specifier that does not matter, the query for the answer does not */
	size = 0;
	s = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_FileTypeName, 1, &t, &size);
	CHECK(s == noErr && size == sizeof(CFStringRef), "FileTypeName size, specifier size 1: %d, %u", (int)s, (unsigned)size);
	s = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ExtensionsForType, 1, &t, &size);
	CHECK(s == noErr && size == sizeof(CFArrayRef), "ExtensionsForType size, specifier size 1: %d, %u", (int)s, (unsigned)size);
	s = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, NULL);
	CHECK(s == paramErr, "size query with nowhere to put the size: %d", (int)s);
	t = kAudioFileWAVEType;
	size = 2;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_FileTypeName, sizeof t, &t, &size, &str);
	CHECK(s == kAudioFileBadPropertySizeError, "FileTypeName with a buffer that is too small: %d", (int)s);
	size = 0;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, &size, &arr);
	CHECK(s == kAudioFileBadPropertySizeError, "extensions with a zero-size buffer: %d", (int)s);
	size = sizeof arr;
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, 1, &t, &size, &arr);
	CHECK(s == kAudioFileBadPropertySizeError, "extensions with specifier size 1: %d", (int)s);
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, &size, NULL);
	CHECK(s == paramErr, "extensions with no place for the answer: %d", (int)s);
	s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof t, &t, NULL, &arr);
	CHECK(s == paramErr, "extensions with no size: %d", (int)s);
	{
		/* a list of types that is longer than the buffer is cut at its end; the size is the whole list */
		UInt32 whole = 0, buf[100], guard[2] = { 0xeeeeeeee, 0xeeeeeeee };
		s = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &whole);
		CHECK(s == noErr && whole >= 2 * sizeof(UInt32) && whole <= sizeof buf, "ReadableTypes size %u", (unsigned)whole);
		memset(buf, 0xee, sizeof buf);
		size = 2 * sizeof(UInt32);
		s = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size, buf);
		CHECK(s == noErr && size == whole && buf[0] != guard[0] && buf[1] != guard[0] && buf[2] == guard[0],
			"ReadableTypes in a buffer for two: %d, size %u of %u, %08x %08x %08x", (int)s, (unsigned)size, (unsigned)whole, buf[0], buf[1], buf[2]);
	}

	/* functions given no file: the ones that act on the file say it is not open, the ones on its properties that the parameter is bad */
	UInt32 v = 0, psz = sizeof v, nitems = 0;
	AudioStreamPacketDescription pd;
	AudioStreamBasicDescription desc;
	SInt64 pos;

#define NOFILE(call, expected) do { s = (call); CHECK(s == (expected), "%s: %d", #call, (int)s); } while (0)
	NOFILE(AudioFileClose(NULL), kAudioFileNotOpenError);
	NOFILE(AudioFileOptimize(NULL), kAudioFileNotOpenError);
	NOFILE(AudioFileReadBytes(NULL, false, 0, &psz, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileWriteBytes(NULL, false, 0, &psz, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileReadPacketData(NULL, false, &psz, &pd, 0, &psz, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileReadPackets(NULL, false, &psz, &pd, 0, &psz, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileWritePackets(NULL, false, 4, &pd, 0, &psz, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileCountUserData(NULL, 'abcd', &nitems), kAudioFileNotOpenError);
	NOFILE(AudioFileGetUserDataSize(NULL, 'abcd', 0, &nitems), kAudioFileNotOpenError);
	NOFILE(AudioFileGetUserData(NULL, 'abcd', 0, &psz, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileSetUserData(NULL, 'abcd', 0, 4, &v), kAudioFileNotOpenError);
	NOFILE(AudioFileRemoveUserData(NULL, 'abcd', 0), kAudioFileNotOpenError);
	NOFILE(AudioFileGetPropertyInfo(NULL, kAudioFilePropertyFileFormat, &nitems, NULL), paramErr);
	NOFILE(AudioFileGetProperty(NULL, kAudioFilePropertyFileFormat, &psz, &v), paramErr);
	NOFILE(AudioFileSetProperty(NULL, kAudioFilePropertyFileFormat, 4, &v), paramErr);
	NOFILE(ExtAudioFileGetProperty(NULL, kExtAudioFileProperty_FileDataFormat, &psz, &desc), paramErr);
	NOFILE(ExtAudioFileSetProperty(NULL, kExtAudioFileProperty_FileDataFormat, sizeof desc, &desc), paramErr);
	NOFILE(ExtAudioFileGetPropertyInfo(NULL, kExtAudioFileProperty_FileDataFormat, &nitems, NULL), paramErr);
	NOFILE(ExtAudioFileSeek(NULL, 0), paramErr);
	NOFILE(ExtAudioFileTell(NULL, &pos), paramErr);
	NOFILE(ExtAudioFileRead(NULL, &nitems, NULL), paramErr);
	NOFILE(ExtAudioFileWrite(NULL, 0, NULL), paramErr);
	NOFILE(ExtAudioFileDispose(NULL), paramErr);

	char cmd[200];
	snprintf(cmd, sizeof cmd, "/bin/rm -rf %s", dir);
	(void)system(cmd);
	printf("%s (%d failed)\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 1 : 0;
}
