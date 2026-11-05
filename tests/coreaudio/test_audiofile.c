// Copyright (C) 2026 Lubos Dolezel
//
// Exercises AudioFile and ExtAudioFile through their public API: writes known samples to
// each writable container, reads them back, and checks conversion and seeking.
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FRAMES 44100
#define RATE 44100.0

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_OK(expr) do { OSStatus s_ = (expr); if (s_ != noErr) { printf("FAIL %s:%d: %s = %d\n", __FILE__, __LINE__, #expr, (int) s_); g_failures++; } } while (0)

static CFURLRef urlFor(const char* path)
{
	return CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), false);
}

static AudioStreamBasicDescription pcm(Float64 rate, UInt32 channels, UInt32 bits, UInt32 flags)
{
	AudioStreamBasicDescription f;
	memset(&f, 0, sizeof(f));
	f.mSampleRate = rate;
	f.mFormatID = kAudioFormatLinearPCM;
	f.mFormatFlags = flags | kAudioFormatFlagIsPacked;
	f.mChannelsPerFrame = channels;
	f.mBitsPerChannel = bits;
	f.mFramesPerPacket = 1;
	f.mBytesPerFrame = f.mBytesPerPacket = bits / 8 * channels;
	return f;
}

// Every frame differs, so that lost or reordered frames show up
static SInt16 sampleAt(int frame, int channel)
{
	return (SInt16) (((frame * 7) % 20011) - 10005 + channel);
}

static void swap16(SInt16* samples, int count)
{
	for (int i = 0; i < count; i++)
		samples[i] = (SInt16) __builtin_bswap16((UInt16) samples[i]);
}

static SInt16* makeSamples(int frames)
{
	SInt16* samples = malloc(frames * 2 * sizeof(SInt16));
	for (int i = 0; i < frames; i++)
	{
		samples[i * 2] = sampleAt(i, 0);
		samples[i * 2 + 1] = sampleAt(i, 1);
	}
	return samples;
}

static void testContainer(const char* path, AudioFileTypeID type, UInt32 flags)
{
	const AudioStreamBasicDescription format = pcm(RATE, 2, 16, kAudioFormatFlagIsSignedInteger | flags);
	const bool swapped = flags & kAudioFormatFlagIsBigEndian;
	SInt16* samples = makeSamples(FRAMES);
	CFURLRef url = urlFor(path);
	AudioFileID file = NULL;

	if (swapped)
		swap16(samples, FRAMES * 2);

	CHECK_OK(AudioFileCreateWithURL(url, type, &format, kAudioFileFlags_EraseFile, &file));
	if (!file)
		return;

	// Written in two parts, as an application would
	UInt32 packets = 1000;
	CHECK_OK(AudioFileWritePackets(file, false, packets * 4, NULL, 0, &packets, samples));
	packets = FRAMES - 1000;
	CHECK_OK(AudioFileWritePackets(file, false, packets * 4, NULL, 1000, &packets, samples + 2000));

	// Replacing what's already there must not change the length
	SInt16 first[2] = { samples[0], samples[1] };
	packets = 1;
	CHECK_OK(AudioFileWritePackets(file, false, 4, NULL, 0, &packets, first));

	CHECK_OK(AudioFileClose(file));

	// No type hint, so that it needs to find the type itself
	file = NULL;
	CHECK_OK(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file));
	if (file)
	{
		AudioStreamBasicDescription read;
		UInt32 size = sizeof(read);
		CHECK_OK(AudioFileGetProperty(file, kAudioFilePropertyDataFormat, &size, &read));
		CHECK(read.mFormatID == kAudioFormatLinearPCM);
		CHECK(read.mSampleRate == RATE);
		CHECK(read.mChannelsPerFrame == 2);
		CHECK(read.mBitsPerChannel == 16);
		CHECK(read.mBytesPerPacket == 4);

		UInt64 count = 0;
		size = sizeof(count);
		CHECK_OK(AudioFileGetProperty(file, kAudioFilePropertyAudioDataPacketCount, &size, &count));
		CHECK(count == FRAMES);

		SInt16* back = malloc(FRAMES * 4);
		UInt32 bytes = FRAMES * 4;
		packets = FRAMES;
		CHECK_OK(AudioFileReadPacketData(file, false, &bytes, NULL, 0, &packets, back));
		CHECK(packets == FRAMES);
		CHECK(memcmp(back, samples, FRAMES * 4) == 0);

		// Reading from the middle, and past the end
		bytes = 40;
		packets = 10;
		CHECK_OK(AudioFileReadPacketData(file, false, &bytes, NULL, 500, &packets, back));
		CHECK(packets == 10);
		CHECK(memcmp(back, samples + 1000, 40) == 0);

		bytes = 40;
		packets = 10;
		OSStatus status = AudioFileReadPacketData(file, false, &bytes, NULL, FRAMES, &packets, back);
		CHECK(status == kAudioFileEndOfFileError);

		free(back);
		CHECK_OK(AudioFileClose(file));
	}

	CFRelease(url);
	free(samples);
}

static void testErrors(const char* dir)
{
	char path[1024];
	const AudioStreamBasicDescription format = pcm(RATE, 2, 16, kAudioFormatFlagIsSignedInteger);
	AudioFileID file = NULL;

	snprintf(path, sizeof(path), "%s/does-not-exist.wav", dir);
	CFURLRef url = urlFor(path);
	CHECK(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file) == fnfErr);
	CFRelease(url);

	// Existing files are only replaced when asked to
	snprintf(path, sizeof(path), "%s/exists.wav", dir);
	url = urlFor(path);
	CHECK_OK(AudioFileCreateWithURL(url, kAudioFileWAVEType, &format, kAudioFileFlags_EraseFile, &file));
	CHECK_OK(AudioFileClose(file));
	CHECK(AudioFileCreateWithURL(url, kAudioFileWAVEType, &format, 0, &file) != noErr);
	CFRelease(url);

	// Not audio, whatever the name says
	snprintf(path, sizeof(path), "%s/not-audio.wav", dir);
	FILE* junk = fopen(path, "wb");
	for (int i = 0; i < 4096; i++)
		fputc(i * 31, junk);
	fclose(junk);
	url = urlFor(path);
	CHECK(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file) != noErr);
	CFRelease(url);

	// Big-endian samples don't exist in WAV files
	snprintf(path, sizeof(path), "%s/bad-format.wav", dir);
	url = urlFor(path);
	const AudioStreamBasicDescription big = pcm(RATE, 2, 16, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian);
	CHECK(AudioFileCreateWithURL(url, kAudioFileWAVEType, &big, kAudioFileFlags_EraseFile, &file) == kAudioFileUnsupportedDataFormatError);
	CFRelease(url);

	// Formats that can only be read
	snprintf(path, sizeof(path), "%s/read-only.flac", dir);
	url = urlFor(path);
	CHECK(AudioFileCreateWithURL(url, kAudioFileFLACType, &format, kAudioFileFlags_EraseFile, &file) != noErr);
	CFRelease(url);
}

static void testWrongExtension(const char* dir)
{
	char from[1024], to[1024];
	snprintf(from, sizeof(from), "%s/test.wav", dir);
	snprintf(to, sizeof(to), "%s/wav-named.aiff", dir);
	CHECK(rename(from, to) == 0);

	AudioFileID file = NULL;
	CFURLRef url = urlFor(to);
	CHECK_OK(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file));
	if (file)
	{
		AudioFileTypeID type = 0;
		UInt32 size = sizeof(type);
		CHECK_OK(AudioFileGetProperty(file, kAudioFilePropertyFileFormat, &size, &type));
		CHECK(type == kAudioFileWAVEType);
		CHECK_OK(AudioFileClose(file));
	}
	CFRelease(url);
	CHECK(rename(to, from) == 0);
}

typedef struct
{
	const UInt8* data;
	SInt64 size;
} MemoryFile;

static OSStatus memoryRead(void* opaque, SInt64 position, UInt32 count, void* buffer, UInt32* actual)
{
	MemoryFile* memory = opaque;
	*actual = 0;
	if (position >= memory->size)
		return noErr;
	*actual = (UInt32) ((position + count > memory->size) ? memory->size - position : count);
	memcpy(buffer, memory->data + position, *actual);
	return noErr;
}

static SInt64 memorySize(void* opaque)
{
	return ((MemoryFile*) opaque)->size;
}

static void testCallbacks(const char* path)
{
	FILE* f = fopen(path, "rb");
	CHECK(f != NULL);
	if (!f)
		return;

	fseek(f, 0, SEEK_END);
	MemoryFile memory;
	memory.size = ftell(f);
	rewind(f);
	UInt8* data = malloc(memory.size);
	CHECK(fread(data, 1, memory.size, f) == (size_t) memory.size);
	fclose(f);
	memory.data = data;

	AudioFileID file = NULL;
	CHECK_OK(AudioFileOpenWithCallbacks(&memory, memoryRead, NULL, memorySize, NULL, 0, &file));
	if (file)
	{
		UInt64 count = 0;
		UInt32 size = sizeof(count);
		CHECK_OK(AudioFileGetProperty(file, kAudioFilePropertyAudioDataPacketCount, &size, &count));
		CHECK(count == FRAMES);

		SInt16 frame[2];
		UInt32 bytes = sizeof(frame);
		UInt32 packets = 1;
		CHECK_OK(AudioFileReadPacketData(file, false, &bytes, NULL, 3, &packets, frame));
		CHECK(frame[0] == sampleAt(3, 0));
		CHECK_OK(AudioFileClose(file));
	}
	free(data);
}

static void testExtRead(const char* path)
{
	CFURLRef url = urlFor(path);
	ExtAudioFileRef ext = NULL;
	CHECK_OK(ExtAudioFileOpenURL(url, &ext));
	CFRelease(url);
	if (!ext)
		return;

	SInt64 length = 0;
	UInt32 size = sizeof(length);
	CHECK_OK(ExtAudioFileGetProperty(ext, kExtAudioFileProperty_FileLengthFrames, &size, &length));
	CHECK(length == FRAMES);

	// Non-PCM isn't something a client can ask for
	AudioStreamBasicDescription bad = pcm(RATE, 2, 16, kAudioFormatFlagIsSignedInteger);
	bad.mFormatID = kAudioFormatMPEGLayer3;
	CHECK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(bad), &bad) == kExtAudioFileError_NonPCMClientFormat);

	// Without a client format this reads the file's own samples
	SInt16 raw[8];
	AudioBufferList list;
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 2;
	list.mBuffers[0].mData = raw;
	list.mBuffers[0].mDataByteSize = sizeof(raw);
	UInt32 frames = 4;
	CHECK_OK(ExtAudioFileRead(ext, &frames, &list));
	CHECK(frames == 4 && raw[2] == sampleAt(1, 0));

	SInt64 position = -1;
	CHECK_OK(ExtAudioFileTell(ext, &position));
	CHECK(position == 4);

	CHECK_OK(ExtAudioFileSeek(ext, 1000));
	frames = 4;
	list.mBuffers[0].mDataByteSize = sizeof(raw);
	CHECK_OK(ExtAudioFileRead(ext, &frames, &list));
	CHECK(frames == 4 && raw[0] == sampleAt(1000, 0) && raw[1] == sampleAt(1000, 1));
	CHECK(ExtAudioFileSeek(ext, -1) == kExtAudioFileError_InvalidSeek);

	// Float samples through the converter
	AudioStreamBasicDescription floats = pcm(RATE, 2, 32, kAudioFormatFlagIsFloat);
	CHECK_OK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(floats), &floats));
	CHECK_OK(ExtAudioFileSeek(ext, 0));

	Float32* samples = malloc(FRAMES * 2 * sizeof(Float32));
	UInt32 total = 0;
	while (total < FRAMES)
	{
		frames = FRAMES - total;
		if (frames > 777)
			frames = 777;
		list.mBuffers[0].mData = samples + total * 2;
		list.mBuffers[0].mDataByteSize = frames * 8;
		CHECK_OK(ExtAudioFileRead(ext, &frames, &list));
		if (!frames)
			break;
		total += frames;
	}
	CHECK(total == FRAMES);

	int wrong = 0;
	for (UInt32 i = 0; i < total; i++)
	{
		if (fabsf(samples[i * 2] - sampleAt(i, 0) / 32768.0f) > 1e-4f)
			wrong++;
	}
	CHECK(wrong == 0);
	free(samples);

	// Seeking after a conversion has started must not leave old audio behind
	CHECK_OK(ExtAudioFileSeek(ext, 20000));
	Float32 after[2];
	frames = 1;
	list.mBuffers[0].mData = after;
	list.mBuffers[0].mDataByteSize = sizeof(after);
	CHECK_OK(ExtAudioFileRead(ext, &frames, &list));
	CHECK(frames == 1 && fabsf(after[0] - sampleAt(20000, 0) / 32768.0f) < 1e-4f);

	// Separate buffers for each channel
	AudioStreamBasicDescription planar = pcm(RATE, 2, 16, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsNonInterleaved);
	planar.mBytesPerFrame = planar.mBytesPerPacket = 2;
	CHECK_OK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(planar), &planar));
	CHECK_OK(ExtAudioFileSeek(ext, 100));

	SInt16 left[16], right[16];
	struct { UInt32 count; AudioBuffer buffers[2]; } two;
	two.count = 2;
	two.buffers[0].mNumberChannels = two.buffers[1].mNumberChannels = 1;
	two.buffers[0].mData = left;
	two.buffers[1].mData = right;
	two.buffers[0].mDataByteSize = two.buffers[1].mDataByteSize = sizeof(left);
	frames = 16;
	CHECK_OK(ExtAudioFileRead(ext, &frames, (AudioBufferList*) &two));
	CHECK(frames == 16 && left[3] == sampleAt(103, 0) && right[3] == sampleAt(103, 1));

	CHECK_OK(ExtAudioFileDispose(ext));
}

static SInt64 readAllFrames(ExtAudioFileRef ext, const AudioStreamBasicDescription* client, double* outPeak)
{
	const UInt32 bufferFrames = 4096;
	SInt16* buffer = malloc(bufferFrames * client->mBytesPerFrame);
	SInt64 total = 0;
	double peak = 0;

	while (true)
	{
		AudioBufferList list;
		UInt32 frames = bufferFrames;
		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = client->mChannelsPerFrame;
		list.mBuffers[0].mData = buffer;
		list.mBuffers[0].mDataByteSize = frames * client->mBytesPerFrame;
		CHECK_OK(ExtAudioFileRead(ext, &frames, &list));
		if (!frames)
			break;
		for (UInt32 i = 0; i < frames * client->mChannelsPerFrame; i++)
			peak = fmax(peak, fabs((double) buffer[i]));
		total += frames;
	}

	free(buffer);
	*outPeak = peak;
	return total;
}

static void testExtConversions(const char* path)
{
	CFURLRef url = urlFor(path);
	ExtAudioFileRef ext = NULL;
	CHECK_OK(ExtAudioFileOpenURL(url, &ext));
	CFRelease(url);
	if (!ext)
		return;

	double peak = 0;

	// A different rate gives a proportional length, with none of the end lost
	AudioStreamBasicDescription resampled = pcm(48000, 2, 16, kAudioFormatFlagIsSignedInteger);
	CHECK_OK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(resampled), &resampled));
	CHECK_OK(ExtAudioFileSeek(ext, 0));
	SInt64 frames = readAllFrames(ext, &resampled, &peak);
	CHECK(llabs(frames - 48000) < 64);
	CHECK(peak > 5000);

	// Fewer channels
	AudioStreamBasicDescription mono = pcm(RATE, 1, 16, kAudioFormatFlagIsSignedInteger);
	CHECK_OK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(mono), &mono));
	CHECK_OK(ExtAudioFileSeek(ext, 0));
	frames = readAllFrames(ext, &mono, &peak);
	CHECK(frames == FRAMES);
	CHECK(peak > 5000);

	CHECK_OK(ExtAudioFileDispose(ext));
}

static void testExtWrite(const char* dir)
{
	char path[1024];
	snprintf(path, sizeof(path), "%s/ext-write.wav", dir);
	CFURLRef url = urlFor(path);

	const AudioStreamBasicDescription fileFormat = pcm(RATE, 2, 16, kAudioFormatFlagIsSignedInteger);
	const AudioStreamBasicDescription client = pcm(RATE, 2, 32, kAudioFormatFlagIsFloat);
	ExtAudioFileRef ext = NULL;

	CHECK_OK(ExtAudioFileCreateWithURL(url, kAudioFileWAVEType, &fileFormat, NULL, kAudioFileFlags_EraseFile, &ext));
	if (!ext)
	{
		CFRelease(url);
		return;
	}
	CHECK_OK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client));

	// The first of these only prepares for writes from a real-time thread
	CHECK_OK(ExtAudioFileWriteAsync(ext, 0, NULL));

	Float32* samples = malloc(FRAMES * 8);
	for (int i = 0; i < FRAMES; i++)
	{
		samples[i * 2] = sampleAt(i, 0) / 32768.0f;
		samples[i * 2 + 1] = sampleAt(i, 1) / 32768.0f;
	}

	UInt32 done = 0;
	while (done < FRAMES)
	{
		UInt32 frames = FRAMES - done;
		if (frames > 1234)
			frames = 1234;

		AudioBufferList list;
		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = 2;
		list.mBuffers[0].mData = samples + done * 2;
		list.mBuffers[0].mDataByteSize = frames * 8;
		CHECK_OK(ExtAudioFileWrite(ext, frames, &list));
		done += frames;
	}
	free(samples);
	CHECK_OK(ExtAudioFileDispose(ext));

	// All of it has to be there, including what the converter had buffered
	AudioFileID file = NULL;
	CHECK_OK(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file));
	if (file)
	{
		UInt64 count = 0;
		UInt32 size = sizeof(count);
		CHECK_OK(AudioFileGetProperty(file, kAudioFilePropertyAudioDataPacketCount, &size, &count));
		CHECK(count == FRAMES);

		SInt16* back = malloc(FRAMES * 4);
		UInt32 bytes = FRAMES * 4;
		UInt32 packets = FRAMES;
		CHECK_OK(AudioFileReadPacketData(file, false, &bytes, NULL, 0, &packets, back));

		int wrong = 0;
		for (UInt32 i = 0; i < packets; i++)
		{
			if (abs(back[i * 2] - sampleAt(i, 0)) > 1)
				wrong++;
		}
		CHECK(wrong == 0);
		free(back);
		CHECK_OK(AudioFileClose(file));
	}
	CFRelease(url);
}

// A compressed file, if the caller has one. Pass the expected frame count and the PCM it decodes to.
static void testCompressed(const char* path, SInt64 expectedFrames)
{
	CFURLRef url = urlFor(path);
	ExtAudioFileRef ext = NULL;
	CHECK_OK(ExtAudioFileOpenURL(url, &ext));
	CFRelease(url);
	if (!ext)
		return;

	AudioStreamBasicDescription fileFormat;
	UInt32 size = sizeof(fileFormat);
	CHECK_OK(ExtAudioFileGetProperty(ext, kExtAudioFileProperty_FileDataFormat, &size, &fileFormat));

	// Nothing to read into until the client picks a format
	SInt16 buffer[4];
	AudioBufferList list;
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 2;
	list.mBuffers[0].mData = buffer;
	list.mBuffers[0].mDataByteSize = sizeof(buffer);
	UInt32 frames = 1;
	CHECK(ExtAudioFileRead(ext, &frames, &list) == kExtAudioFileError_NonPCMClientFormat);

	SInt64 length = 0;
	size = sizeof(length);
	CHECK_OK(ExtAudioFileGetProperty(ext, kExtAudioFileProperty_FileLengthFrames, &size, &length));
	CHECK(length == expectedFrames);

	AudioStreamBasicDescription client = pcm(fileFormat.mSampleRate, fileFormat.mChannelsPerFrame, 16, kAudioFormatFlagIsSignedInteger);
	CHECK_OK(ExtAudioFileSetProperty(ext, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client));

	double peak = 0;
	SInt64 total = readAllFrames(ext, &client, &peak);
	CHECK(total == expectedFrames);
	CHECK(peak > 1000);

	// Into the middle of a packet
	CHECK_OK(ExtAudioFileSeek(ext, expectedFrames / 2));
	total = readAllFrames(ext, &client, &peak);
	CHECK(total == expectedFrames - expectedFrames / 2);

	CHECK_OK(ExtAudioFileDispose(ext));
}

int main(int argc, const char** argv)
{
	char dir[] = "/tmp/audiofile-test-XXXXXX";
	char path[1024];

	if (!mkdtemp(dir))
	{
		perror("mkdtemp");
		return 1;
	}

	static const struct { const char* name; AudioFileTypeID type; UInt32 flags; } CONTAINERS[] = {
		{ "test.wav", kAudioFileWAVEType, 0 },
		{ "test.aiff", kAudioFileAIFFType, kAudioFormatFlagIsBigEndian },
		{ "test.caf", kAudioFileCAFType, 0 },
		{ "test.au", kAudioFileNextType, kAudioFormatFlagIsBigEndian },
	};

	for (size_t i = 0; i < sizeof(CONTAINERS) / sizeof(CONTAINERS[0]); i++)
	{
		snprintf(path, sizeof(path), "%s/%s", dir, CONTAINERS[i].name);
		testContainer(path, CONTAINERS[i].type, CONTAINERS[i].flags);
	}

	testErrors(dir);
	testWrongExtension(dir);

	snprintf(path, sizeof(path), "%s/test.wav", dir);
	testCallbacks(path);
	testExtRead(path);
	testExtConversions(path);
	testExtWrite(dir);

	// argv: pairs of compressed file and its length in frames
	for (int i = 1; i + 1 < argc; i += 2)
		testCompressed(argv[i], atoll(argv[i + 1]));

	char cmd[1100];
	snprintf(cmd, sizeof(cmd), "rm -rf %s", dir);
	system(cmd);

	if (g_failures)
	{
		printf("test_audiofile: %d failure(s)\n", g_failures);
		return 1;
	}

	printf("test_audiofile: OK\n");
	return 0;
}
