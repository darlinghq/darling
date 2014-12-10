#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H
#include <CoreServices/MacTypes.h>
#include <CoreAudio/CoreAudioTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
class AudioConverter;
#else
typedef struct AudioConverter AudioConverter;
#endif

typedef UInt32 AudioConverterPropertyID;
typedef AudioConverter* AudioConverterRef;
typedef OSStatus (*AudioConverterComplexInputDataProc)(AudioConverterRef audioConverter, UInt32* numberDataPackets, AudioBufferList* data, AudioStreamPacketDescription** dataPacketDescription, void* userData);

OSStatus AudioConverterNew(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, AudioConverterRef* outAudioConverter);
OSStatus AudioConverterNewSpecific(const AudioStreamBasicDescription* inSourceFormat, const AudioStreamBasicDescription* inDestinationFormat, UInt32 inNumberClassDescriptions, AudioClassDescription* nClassDescriptions, AudioConverterRef* outAudioConverter);
OSStatus AudioConverterDispose(AudioConverterRef inAudioConverter);
OSStatus AudioConverterReset(AudioConverterRef inAudioConverter);

OSStatus AudioConverterGetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *ioPropertyDataSize, void *outPropertyData);
OSStatus AudioConverterGetPropertyInfo(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable);
OSStatus AudioConverterSetProperty(AudioConverterRef inAudioConverter, AudioConverterPropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData);

OSStatus AudioConverterConvertBuffer(AudioConverterRef inAudioConverter, UInt32 inInputDataSize, const void* inInputData, UInt32 *ioOutputDataSize, void *outOutputData);
OSStatus AudioConverterFillComplexBuffer(AudioConverterRef inAudioConverter, AudioConverterComplexInputDataProc inInputDataProc, void *inInputDataProcUserData, UInt32 *ioOutputDataPacketSize, AudioBufferList *outOutputData, AudioStreamPacketDescription*outPacketDescription);
OSStatus AudioConverterConvertComplexBuffer(AudioConverterRef inAudioConverter, UInt32 inNumberPCMFrames, const AudioBufferList *inInputData, AudioBufferList *outOutputData);

#ifdef __cplusplus
}
#endif


#endif

