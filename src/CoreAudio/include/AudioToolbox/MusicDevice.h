#ifndef _MUSIC_DEVICE_H
#define _MUSIC_DEVICE_H

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AUComponent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UInt32 MusicDeviceInstrumentID;
struct MusicDeviceStdNoteParams
{
 UInt32 argCount;
 Float32 mPitch;
 Float32 mVelocity;
};
typedef struct MusicDeviceStdNoteParams MusicDeviceStdNoteParams;
struct NoteParamsControlValue
{
 AudioUnitParameterID mID;
 AudioUnitParameterValue mValue;
};
typedef struct NoteParamsControlValue NoteParamsControlValue;
struct MusicDeviceNoteParams
{
 UInt32 argCount;
 Float32 mPitch;
 Float32 mVelocity;
 NoteParamsControlValue mControls[1];
};
typedef struct MusicDeviceNoteParams MusicDeviceNoteParams;
enum {
 kMusicNoteEvent_UseGroupInstrument = 0xFFFFFFFF,
 kMusicNoteEvent_Unused = 0xFFFFFFFF
};






typedef UInt32 MusicDeviceGroupID;
typedef UInt32 NoteInstanceID;





typedef AudioComponentInstance MusicDeviceComponent;
extern OSStatus
MusicDeviceMIDIEvent( MusicDeviceComponent inUnit,
      UInt32 inStatus,
      UInt32 inData1,
      UInt32 inData2,
      UInt32 inOffsetSampleFrame);
extern OSStatus
MusicDeviceSysEx( MusicDeviceComponent inUnit,
      const UInt8 * inData,
      UInt32 inLength);
extern OSStatus
MusicDeviceStartNote( MusicDeviceComponent inUnit,
      MusicDeviceInstrumentID inInstrument,
      MusicDeviceGroupID inGroupID,
      NoteInstanceID * outNoteInstanceID,
      UInt32 inOffsetSampleFrame,
      const MusicDeviceNoteParams * inParams);
extern OSStatus
MusicDeviceStopNote( MusicDeviceComponent inUnit,
      MusicDeviceGroupID inGroupID,
      NoteInstanceID inNoteInstanceID,
      UInt32 inOffsetSampleFrame);
enum {
  kMusicDeviceRange = 0x0100,
  kMusicDeviceMIDIEventSelect = 0x0101,
  kMusicDeviceSysExSelect = 0x0102,
  kMusicDevicePrepareInstrumentSelect = 0x0103,
  kMusicDeviceReleaseInstrumentSelect = 0x0104,
  kMusicDeviceStartNoteSelect = 0x0105,
  kMusicDeviceStopNoteSelect = 0x0106
};
typedef OSStatus
(*MusicDeviceMIDIEventProc)( void * self,
        UInt32 inStatus,
        UInt32 inData1,
        UInt32 inData2,
        UInt32 inOffsetSampleFrame);
typedef OSStatus
(*MusicDeviceSysExProc)( void * self,
       const UInt8 * inData,
       UInt32 inLength);
typedef OSStatus
(*MusicDeviceStartNoteProc)( void * self,
      MusicDeviceInstrumentID inInstrument,
      MusicDeviceGroupID inGroupID,
      NoteInstanceID * outNoteInstanceID,
      UInt32 inOffsetSampleFrame,
      const MusicDeviceNoteParams * inParams);
typedef OSStatus
(*MusicDeviceStopNoteProc)( void * self,
      MusicDeviceGroupID inGroupID,
      NoteInstanceID inNoteInstanceID,
      UInt32 inOffsetSampleFrame);
extern OSStatus
MusicDevicePrepareInstrument( MusicDeviceComponent inUnit,
        MusicDeviceInstrumentID inInstrument);


extern OSStatus
MusicDeviceReleaseInstrument( MusicDeviceComponent inUnit,
        MusicDeviceInstrumentID inInstrument);

#ifdef __cplusplus
}
#endif

#endif
