#ifndef AT_AUDIO_OUTPUT_UNIT_H
#define AT_AUDIO_OUTPUT_UNIT_H
#include <AudioToolbox/AUComponent.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	kAudioOutputUnitRange = 0x0200,
	kAudioOutputUnitStartSelect = 0x0201,
	kAudioOutputUnitStopSelect = 0x0202
};


extern OSStatus
AudioOutputUnitStart(AudioUnit ci);

extern OSStatus
AudioOutputUnitStop(AudioUnit ci);

#ifdef __cplusplus
}
#endif


#endif

