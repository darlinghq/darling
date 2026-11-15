// Copyright (C) 2026 Lubos Dolezel
//
// Writes the audio that the other fixtures are made from: a CAF file of 16-bit little-endian
// samples, with a different ramp on every channel.
// usage: make-source <file.caf> <channels> [frames]
#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("usage: make-source <file.caf> <channels> [frames]\n");
		return 2;
	}

	const int channels = atoi(argv[2]);
	const int frames = argc > 3 ? atoi(argv[3]) : 1000;
	AudioStreamBasicDescription format = { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
		2 * channels, 1, 2 * channels, channels, 16, 0 };
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) argv[1], strlen(argv[1]), false);
	AudioFileID file;
	short* samples = malloc((size_t) frames * channels * 2);
	UInt32 packets = frames;

	unlink(argv[1]);
	OSStatus status = AudioFileCreateWithURL(url, kAudioFileCAFType, &format, 0, &file);
	if (status != noErr)
	{
		printf("AudioFileCreateWithURL: %d\n", (int) status);
		return 1;
	}

	for (int i = 0; i < frames; i++)
	{
		for (int c = 0; c < channels; c++)
		{
			if (c == 0)
				samples[i * channels + c] = (short) (i * 60 - 30000);
			else if (c == 1)
				samples[i * channels + c] = (short) (15000 - i * 30);
			else
				samples[i * channels + c] = (short) (i * (c + 1) * 7 - 2000);
		}
	}

	status = AudioFileWritePackets(file, false, frames * 2 * channels, NULL, 0, &packets, samples);
	AudioFileClose(file);
	return status == noErr ? 0 : 1;
}
