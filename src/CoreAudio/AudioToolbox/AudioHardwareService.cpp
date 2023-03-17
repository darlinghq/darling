#include <AudioToolbox/AudioHardwareService.h>
#include "stub.h"

OSStatus AudioHardwareServiceGetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 *ioDataSize, void *outData)
{
    STUB();
    return unimpErr;
}

Boolean AudioHardwareServiceHasProperty(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress)
{
    STUB();
    return false;
}

OSStatus AudioHardwareServiceIsPropertySettable(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, Boolean *outIsSettable)
{
    STUB();
    return unimpErr;
}

OSStatus AudioHardwareServiceSetPropertyData(AudioObjectID inObjectID, const AudioObjectPropertyAddress *inAddress, UInt32 inQualifierDataSize, const void *inQualifierData, UInt32 inDataSize, const void *inData)
{
    STUB();
    return unimpErr;
}
