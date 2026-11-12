#ifndef AUDIOTOOLBOX_AUDIOHARDWARESERVICE_H
#define AUDIOTOOLBOX_AUDIOHARDWARESERVICE_H

#include <CarbonCore/Components.h>
#include <CoreAudio/AudioHardware.h>

#ifdef __cplusplus
extern "C" {
#endif

OSStatus AudioHardwareServiceGetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *ioDataSize, void *outData);
Boolean AudioHardwareServiceHasProperty(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress);
OSStatus AudioHardwareServiceIsPropertySettable(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable);
OSStatus AudioHardwareServiceSetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);

#ifdef __cplusplus
}
#endif


#endif
