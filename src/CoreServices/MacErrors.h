#ifndef MACERRORS_H
#define MACERRORS_H
#include <CoreFoundation/CFBase.h>

inline OSStatus makeOSStatus(int errNo) { return 100000 + errNo; }

#define memFullErr -108
#define internalComponentErr -2070
#define noErr		0
#define unimpErr	-4
#define fnfErr		-43 // file not found
#define paramErr	-50
#define handlerNotFoundErr -1856
#define kLocalesBufferTooSmallErr -30001
#define kUCOutputBufferTooSmall -25340

#define kAudioUnitErr_InvalidProperty	-10879	
#define kAudioUnitErr_InvalidParameter	-10878	
#define kAudioUnitErr_InvalidElement	-10877	
#define kAudioUnitErr_NoConnection	-10876	
#define kAudioUnitErr_FailedInitialization	-10875	
#define kAudioUnitErr_TooManyFramesToProcess	-10874	
#define kAudioUnitErr_IllegalInstrument	-10873	
#define kAudioUnitErr_InstrumentTypeNotFound	-10872	
#define kAudioUnitErr_InvalidFile	-10871	
#define kAudioUnitErr_UnknownFileType	-10870	
#define kAudioUnitErr_FileNotSpecified	-10869	
#define kAudioUnitErr_FormatNotSupported	-10868	
#define kAudioUnitErr_Uninitialized	-10867	
#define kAudioUnitErr_InvalidScope	-10866	
#define kAudioUnitErr_PropertyNotWritable	-10865	
#define kAudioUnitErr_CannotDoInCurrentContext	-10863	
#define kAudioUnitErr_InvalidPropertyValue	-10851	
#define kAudioUnitErr_PropertyNotInUse	-10850	
#define kAudioUnitErr_Initialized	-10849	
#define kAudioUnitErr_InvalidOfflineRender	-10848	
#define kAudioUnitErr_Unauthorized	-10847

#define kMPTimeoutErr -29296

#endif
