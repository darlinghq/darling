// tests/coreaudio/test_io_race.c
// Starts and stops two IOProcs of one device from separate threads, while another
// IOProc stops itself from its own callback. Run it inside Darling; only silence is played.
// A deadlock shows as the program never finishing, and memory corruption as a crash.
#include <CoreAudio/CoreAudio.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(expr) do { OSStatus s_ = (expr); if (s_ != noErr) { printf("FAIL %s:%d: %s = %d\n", __FILE__, __LINE__, #expr, (int) s_); exit(1); } } while (0)

static AudioDeviceID g_device;
static volatile int g_finished;
static volatile int g_callbacks, g_selfStops;
static AudioDeviceIOProcID g_selfStopping;

static OSStatus silence(AudioObjectID device, const AudioTimeStamp* now, const AudioBufferList* input, const AudioTimeStamp* inputTime,
	AudioBufferList* output, const AudioTimeStamp* outputTime, void* context)
{
	for (UInt32 i = 0; i < output->mNumberBuffers; i++)
		memset(output->mBuffers[i].mData, 0, output->mBuffers[i].mDataByteSize);
	__sync_fetch_and_add(&g_callbacks, 1);

	if (context == &g_selfStopping)
	{
		// Only a double stop may report that the IOProc isn't running.
		AudioDeviceStop(device, g_selfStopping);
		__sync_fetch_and_add(&g_selfStops, 1);
	}
	return noErr;
}

static void* churn(void* context)
{
	AudioDeviceIOProcID proc;
	CHECK(AudioDeviceCreateIOProcID(g_device, silence, NULL, &proc));

	while (!g_finished)
	{
		CHECK(AudioDeviceStart(g_device, proc));
		usleep(5000);
		CHECK(AudioDeviceStop(g_device, proc));
	}

	CHECK(AudioDeviceDestroyIOProcID(g_device, proc));
	return NULL;
}

int main(void)
{
	UInt32 size = sizeof(g_device);
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, &g_device));

	pthread_t threads[3];
	for (int i = 0; i < 3; i++)
		pthread_create(&threads[i], NULL, churn, NULL);

	CHECK(AudioDeviceCreateIOProcID(g_device, silence, &g_selfStopping, &g_selfStopping));

	for (int i = 0; i < 40; i++)
	{
		// Fails only if the previous round's stop hasn't happened yet.
		AudioDeviceStart(g_device, g_selfStopping);
		usleep(100000);
	}

	g_finished = 1;
	for (int i = 0; i < 3; i++)
		pthread_join(threads[i], NULL);

	AudioDeviceStop(g_device, g_selfStopping);
	CHECK(AudioDeviceDestroyIOProcID(g_device, g_selfStopping));

	printf("test_io_race: %d callbacks, %d self stops\n", g_callbacks, g_selfStops);
	if (!g_callbacks || !g_selfStops)
	{
		printf("FAIL: the IOProcs never ran\n");
		return 1;
	}
	printf("test_io_race: OK\n");
	return 0;
}
