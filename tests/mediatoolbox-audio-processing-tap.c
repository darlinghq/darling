// MTAudioProcessingTap (MediaToolbox, documented in Apple's
// MTAudioProcessingTap reference): creating a tap calls its init callback with
// the client info, the storage init gives is what GetStorage returns, the tap
// is a CF object of its own type, and finalize is called when it is released.
// Creating a tap needs callbacks with a process callback.
#include <MediaToolbox/MediaToolbox.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s\n", #cond); failures++; } else printf("PASS: %s\n", #cond); } while (0)

static int storage;
static void *seenClientInfo;
static int finalized;

static void tapInit(MTAudioProcessingTapRef tap, void *clientInfo, void **tapStorageOut) {
    (void) tap;
    seenClientInfo = clientInfo;
    *tapStorageOut = &storage;
}

static void tapFinalize(MTAudioProcessingTapRef tap) {
    (void) tap;
    finalized++;
}

static void tapProcess(MTAudioProcessingTapRef tap, CMItemCount numberFrames, MTAudioProcessingTapFlags flags,
    AudioBufferList *bufferListInOut, CMItemCount *numberFramesOut, MTAudioProcessingTapFlags *flagsOut) {
    MTAudioProcessingTapGetSourceAudio(tap, numberFrames, bufferListInOut, flagsOut, NULL, numberFramesOut);
    (void) flags;
}

int main(void) {
    int clientInfo;
    MTAudioProcessingTapCallbacks callbacks = {
        kMTAudioProcessingTapCallbacksVersion_0, &clientInfo, tapInit, tapFinalize, NULL, NULL, tapProcess
    };
    MTAudioProcessingTapRef tap = NULL;
    CHECK(MTAudioProcessingTapCreate(kCFAllocatorDefault, &callbacks, kMTAudioProcessingTapCreationFlag_PostEffects, &tap) == noErr);
    CHECK(tap != NULL);
    CHECK(seenClientInfo == &clientInfo);
    CHECK(MTAudioProcessingTapGetStorage(tap) == &storage);
    CHECK(CFGetTypeID(tap) == MTAudioProcessingTapGetTypeID());
    CHECK(finalized == 0);
    CFRelease(tap);
    CHECK(finalized == 1);

    MTAudioProcessingTapCallbacks noProcess = callbacks;
    noProcess.process = NULL;
    tap = (MTAudioProcessingTapRef) 1;
    CHECK(MTAudioProcessingTapCreate(kCFAllocatorDefault, &noProcess, kMTAudioProcessingTapCreationFlag_PreEffects, &tap) != noErr);
    CHECK(tap == NULL);
    CHECK(MTAudioProcessingTapCreate(kCFAllocatorDefault, NULL, kMTAudioProcessingTapCreationFlag_PreEffects, &tap) != noErr);

    printf("%s\n", failures ? "FAILED" : "ALL PASSED");
    return failures != 0;
}
