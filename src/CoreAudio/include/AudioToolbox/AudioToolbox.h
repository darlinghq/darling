#ifndef AUDIOTOOLBOX_H
#define AUDIOTOOLBOX_H

#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioConverter.h>
#include <AudioToolbox/AUGraph.h>
#include <AudioToolbox/AudioUnitParameters.h>
#include <AudioToolbox/AudioFormat.h>
#include <AudioToolbox/AudioUnit.h>
#include <AudioToolbox/ExtendedAudioFile.h>
#include <AudioToolbox/MusicDevice.h>
#include <AudioToolbox/AudioCodec.h>
#include <AudioToolbox/AudioHardwareService.h>

// CA helper classes expect this
#include <CoreServices/FileManager.h>
#include <CoreServices/Components.h>

// OpenAL expects this
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void CAShow(void* inObject);

#ifdef __cplusplus
}
#endif

#endif
