#ifndef AUDIOHARDWAREBASE_H
#define AUDIOHARDWAREBASE_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include <CoreAudio/CoreAudioTypes.h>

typedef UInt32 AudioObjectID;
typedef UInt32 AudioClassID;
typedef UInt32 AudioObjectPropertySelector;
typedef UInt32 AudioObjectPropertyScope;
typedef UInt32 AudioObjectPropertyElement;

struct AudioObjectPropertyAddress
{
	AudioObjectPropertySelector mSelector;
	AudioObjectPropertyScope mScope;
	AudioObjectPropertyElement mElement;
};

#ifdef __cplusplus
}
#endif

#endif /* AUDIOHARDWAREBASE_H */

