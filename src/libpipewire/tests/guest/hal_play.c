// Copyright (C) 2026 Lubos Dolezel
// HAL output through AudioDeviceCreateIOProcID on the default output device: the IOProc is
// called with float32 buffers of the device's format at about real time, writing a 440 Hz tone;
// the device latency is under half a second (a unit mix-up in the delay once made it a minute),
// AudioDeviceStop and AudioDeviceDestroyIOProcID return, and the process exits.
#include <CoreAudio/CoreAudio.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
static UInt32 latency;
static volatile unsigned calls; static volatile unsigned long long frames; static double ph;
static OSStatus proc(AudioObjectID dev, const AudioTimeStamp *now, const AudioBufferList *in, const AudioTimeStamp *inT, AudioBufferList *out, const AudioTimeStamp *outT, void *ctx)
{
    calls++;
    for (UInt32 b = 0; b < out->mNumberBuffers; b++) {
        float *d = out->mBuffers[b].mData; UInt32 ch = out->mBuffers[b].mNumberChannels ? out->mBuffers[b].mNumberChannels : 1;
        UInt32 n = out->mBuffers[b].mDataByteSize / (sizeof(float) * ch);
        for (UInt32 i = 0; i < n; i++) { float v = 0.25f * sinf(ph); ph += 2 * M_PI * 440 / 48000.0; for (UInt32 c = 0; c < ch; c++) d[i * ch + c] = v; }
        if (b == 0) frames += n;
    }
    return noErr;
}
int main(void)
{
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    AudioObjectID dev = 0; UInt32 sz = sizeof(dev);
    OSStatus st = AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &sz, &dev);
    printf("default device %u status %d\n", dev, (int) st);
    Float64 rate = 0; sz = sizeof(rate);
    AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    st = AudioObjectGetPropertyData(dev, &ra, 0, NULL, &sz, &rate); printf("nominal rate %g status %d\n", rate, (int) st);
    AudioObjectPropertyAddress fa = { kAudioDevicePropertyStreamFormat, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
    AudioStreamBasicDescription f; sz = sizeof(f); st = AudioObjectGetPropertyData(dev, &fa, 0, NULL, &sz, &f);
    printf("format status %d rate %g ch %u bits %u flags %x bpf %u\n", (int) st, f.mSampleRate, (unsigned) f.mChannelsPerFrame, (unsigned) f.mBitsPerChannel, (unsigned) f.mFormatFlags, (unsigned) f.mBytesPerFrame);
    AudioDeviceIOProcID id;
    st = AudioDeviceCreateIOProcID(dev, proc, NULL, &id); printf("create %d\n", (int) st);
    st = AudioDeviceStart(dev, id); printf("start %d (%.4s)\n", (int) st, (char *) &st);
    if (st) { printf("FAILED\n"); return 1; }
    sleep(3);
    { UInt32 lat = 0, safety = 0; sz = sizeof(lat);
      AudioObjectPropertyAddress la = { kAudioDevicePropertyLatency, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
      st = AudioObjectGetPropertyData(dev, &la, 0, NULL, &sz, &lat); printf("device latency %u frames (status %d)\n", (unsigned) lat, (int) st); latency = lat;
      AudioObjectPropertyAddress sa = { kAudioDevicePropertySafetyOffset, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
      sz = sizeof(safety); st = AudioObjectGetPropertyData(dev, &sa, 0, NULL, &sz, &safety); printf("safety offset %u (status %d)\n", (unsigned) safety, (int) st); }
    printf("calls %u frames %llu\n", calls, frames);
    st = AudioDeviceStop(dev, id); printf("stop %d\n", (int) st);
    st = AudioDeviceDestroyIOProcID(dev, id); printf("destroy %d\n", (int) st);
    int ok = calls > 30 && frames > 100000 && latency < 24000;   /* real time rate, latency under half a second */
    printf("%s\n", ok ? "PASS" : "FAILED");
    return !ok;
}
