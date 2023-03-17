#include <CoreServices/Components.h>
#include <CoreAudio/AudioHardware.h>

extern "C"{
OSStatus AudioHardwareServiceGetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *ioDataSize, void *outData);
Boolean AudioHardwareServiceHasProperty(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress);
OSStatus AudioHardwareServiceIsPropertySettable(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable);
OSStatus AudioHardwareServiceSetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData);
}
