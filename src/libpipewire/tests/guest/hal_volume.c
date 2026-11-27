// Copyright (C) 2026 Lubos Dolezel
// Volume and mute of the default output device follow the PipeWire server's default sink.
//   hal_volume watch <volume> <mute>   registers property listeners, prints READY, and waits until a change made
//                                      from outside (wpctl) has fired the listeners and the device reports
//                                      <volume> (scalar, within 0.01) and <mute>
//   hal_volume set <volume> <mute>     sets both through the device, reads them back and prints SET; the caller
//                                      compares the server's values
#include <CoreAudio/CoreAudio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile int volumeFired, muteFired;
static AudioObjectPropertyAddress volumeAddr = { kAudioDevicePropertyVolumeScalar, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
static AudioObjectPropertyAddress muteAddr = { kAudioDevicePropertyMute, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };

static OSStatus listener(AudioObjectID obj, UInt32 n, const AudioObjectPropertyAddress *a, void *ctx)
{
    for (UInt32 i = 0; i < n; i++) {
        if (a[i].mSelector == kAudioDevicePropertyVolumeScalar) volumeFired++;
        if (a[i].mSelector == kAudioDevicePropertyMute) muteFired++;
    }
    return noErr;
}

static float getVolume(AudioObjectID dev) { Float32 v = -1; UInt32 sz = sizeof(v); AudioObjectGetPropertyData(dev, &volumeAddr, 0, NULL, &sz, &v); return v; }
static int getMute(AudioObjectID dev) { UInt32 m = 99; UInt32 sz = sizeof(m); AudioObjectGetPropertyData(dev, &muteAddr, 0, NULL, &sz, &m); return (int) m; }

int main(int argc, char **argv)
{
    int failures = 0;
    const char *mode = argc > 1 ? argv[1] : "local";
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    AudioObjectID dev = 0; UInt32 sz = sizeof(dev);
    OSStatus st = AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &sz, &dev);
    printf("default device %u status %d\n", dev, (int) st);
    if (st || !dev) { printf("FAILED\n"); return 1; }
    Boolean settable = 0;
    AudioObjectIsPropertySettable(dev, &volumeAddr, &settable);
    if (!settable) { printf("FAIL: volume not settable\n"); failures++; }

    if (!strcmp(mode, "watch")) {
        float want = argc > 2 ? atof(argv[2]) : 0.5f; int wantMute = argc > 3 ? atoi(argv[3]) : 0;
        AudioObjectAddPropertyListener(dev, &volumeAddr, listener, NULL);
        AudioObjectAddPropertyListener(dev, &muteAddr, listener, NULL);
        printf("initial volume %.3f mute %d\nREADY\n", getVolume(dev), getMute(dev));
        fflush(stdout);
        for (int i = 0; i < 300; i++) {
            if (volumeFired && muteFired && fabsf(getVolume(dev) - want) < 0.01f && getMute(dev) == wantMute) break;
            usleep(100000);
        }
        printf("after: volume %.3f mute %d listeners volume %d mute %d\n", getVolume(dev), getMute(dev), volumeFired, muteFired);
        if (!volumeFired) { printf("FAIL: no volume notification\n"); failures++; }
        if (!muteFired) { printf("FAIL: no mute notification\n"); failures++; }
        if (fabsf(getVolume(dev) - want) >= 0.01f) { printf("FAIL: volume does not match\n"); failures++; }
        if (getMute(dev) != wantMute) { printf("FAIL: mute does not match\n"); failures++; }
    } else if (!strcmp(mode, "set")) {
        Float32 v = argc > 2 ? atof(argv[2]) : 0.5f; UInt32 m = argc > 3 ? atoi(argv[3]) : 0;
        OSStatus s1 = AudioObjectSetPropertyData(dev, &volumeAddr, 0, NULL, sizeof(v), &v);
        OSStatus s2 = AudioObjectSetPropertyData(dev, &muteAddr, 0, NULL, sizeof(m), &m);
        printf("set status %d %d, read back volume %.3f mute %d\n", (int) s1, (int) s2, getVolume(dev), getMute(dev));
        if (s1 || s2) failures++;
        if (fabsf(getVolume(dev) - v) >= 0.01f || getMute(dev) != (int) m) { printf("FAIL: read back differs\n"); failures++; }
        usleep(1500000);   /* let the request reach the server before the connection goes away */
        printf("SET\n");
    } else {
        printf("usage: hal_volume watch|set <volume> <mute>\n");
        failures++;
    }
    printf(failures ? "FAILED\n" : "PASS\n");
    return failures != 0;
}
