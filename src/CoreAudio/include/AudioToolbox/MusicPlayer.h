#ifndef _MUSIC_PLAYER_H
#define _MUSIC_PLAYER_H

// Music sequencing: sequences, tracks, event iterators and players.
// https://developer.apple.com/documentation/audiotoolbox/music-player

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/MusicDevice.h>
#include <AudioToolbox/AUGraph.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

// Event types of a music track. The extended control event (value 2) was removed from the
// supported API and is not declared.
CF_ENUM(UInt32)
{
	kMusicEventType_NULL = 0,
	kMusicEventType_ExtendedNote = 1,
	kMusicEventType_ExtendedTempo = 3,
	kMusicEventType_User = 4,
	kMusicEventType_Meta = 5,
	kMusicEventType_MIDINoteMessage = 6,
	kMusicEventType_MIDIChannelMessage = 7,
	kMusicEventType_MIDIRawData = 8,
	kMusicEventType_Parameter = 9,
	kMusicEventType_AUPreset = 10
};
typedef UInt32 MusicEventType;

// How a standard MIDI file is distributed over the tracks of a sequence.
typedef CF_OPTIONS(UInt32, MusicSequenceLoadFlags) {
	kMusicSequenceLoadSMF_PreserveTracks = 0,
	kMusicSequenceLoadSMF_ChannelsToTracks = (1 << 0)
};

// What a beat of a sequence means: a musical beat, a second, or a sample.
typedef CF_ENUM(UInt32, MusicSequenceType) {
	kMusicSequenceType_Beats = 'beat',
	kMusicSequenceType_Seconds = 'secs',
	kMusicSequenceType_Samples = 'samp'
};

typedef CF_ENUM(UInt32, MusicSequenceFileTypeID) {
	kMusicSequenceFile_AnyType = 0,
	kMusicSequenceFile_MIDIType = 'midi',
	kMusicSequenceFile_iMelodyType = 'imel'
};

typedef CF_OPTIONS(UInt32, MusicSequenceFileFlags) {
	kMusicSequenceFileFlags_Default = 0,
	kMusicSequenceFileFlags_EraseFile = 1
};

// A position in a sequence, in beats.
typedef Float64 MusicTimeStamp;

// A time beyond the last event of a track.
#define kMusicTimeStamp_EndOfTrack DBL_MAX

typedef struct MIDINoteMessage
{
	UInt8 channel;
	UInt8 note;
	UInt8 velocity;
	UInt8 releaseVelocity;
	Float32 duration;
} MIDINoteMessage;

typedef struct MIDIChannelMessage
{
	UInt8 status;
	UInt8 data1;
	UInt8 data2;
	UInt8 reserved;
} MIDIChannelMessage;

// A system-exclusive message; data holds length bytes.
typedef struct MIDIRawData
{
	UInt32 length;
	UInt8 data[1];
} MIDIRawData;

// A MIDI file meta event; data holds dataLength bytes.
typedef struct MIDIMetaEvent
{
	UInt8 metaEventType;
	UInt8 unused1;
	UInt8 unused2;
	UInt8 unused3;
	UInt32 dataLength;
	UInt8 data[1];
} MIDIMetaEvent;

// A user-defined event; data holds length bytes.
typedef struct MusicEventUserData
{
	UInt32 length;
	UInt8 data[1];
} MusicEventUserData;

typedef struct ExtendedNoteOnEvent
{
	MusicDeviceInstrumentID instrumentID;
	MusicDeviceGroupID groupID;
	Float32 duration;
	MusicDeviceNoteParams extendedParams;
} ExtendedNoteOnEvent;

typedef struct ParameterEvent
{
	AudioUnitParameterID parameterID;
	AudioUnitScope scope;
	AudioUnitElement element;
	AudioUnitParameterValue value;
} ParameterEvent;

typedef struct ExtendedTempoEvent
{
	Float64 bpm;
} ExtendedTempoEvent;

typedef struct AUPresetEvent
{
	AudioUnitScope scope;
	AudioUnitElement element;
	CFPropertyListRef preset;
} AUPresetEvent;

// A musical time for display: bar and beat numbers count from 1, and subbeat is the numerator of
// a fraction of a beat with denominator subbeatDivisor.
struct CABarBeatTime {
	SInt32 bar;
	UInt16 beat;
	UInt16 subbeat;
	UInt16 subbeatDivisor;
	UInt16 reserved;
};
typedef struct CABarBeatTime CABarBeatTime;

typedef struct OpaqueMusicPlayer *MusicPlayer;
typedef struct OpaqueMusicSequence *MusicSequence;
typedef struct OpaqueMusicTrack *MusicTrack;
typedef struct OpaqueMusicEventIterator *MusicEventIterator;

typedef void (*MusicSequenceUserCallback)(void *inClientData,
	MusicSequence inSequence,
	MusicTrack inTrack,
	MusicTimeStamp inEventTime,
	const MusicEventUserData *inEventData,
	MusicTimeStamp inStartSliceBeat,
	MusicTimeStamp inEndSliceBeat);

// Result codes of the music sequencing functions. The last one is reported when a track has no
// destination.
CF_ENUM(OSStatus)
{
	kAudioToolboxErr_InvalidSequenceType = -10846,
	kAudioToolboxErr_TrackIndexError = -10859,
	kAudioToolboxErr_TrackNotFound = -10858,
	kAudioToolboxErr_EndOfTrack = -10857,
	kAudioToolboxErr_StartOfTrack = -10856,
	kAudioToolboxErr_IllegalTrackDestination = -10855,
	kAudioToolboxErr_NoSequence = -10854,
	kAudioToolboxErr_InvalidEventType = -10853,
	kAudioToolboxErr_InvalidPlayerState = -10852,
	kAudioToolboxErr_CannotDoInCurrentContext = -10863,
	kAudioToolboxError_NoTrackDestination = -66720
};

// Property IDs of MusicTrackSetProperty and MusicTrackGetProperty.
CF_ENUM(UInt32)
{
	kSequenceTrackProperty_LoopInfo = 0,
	kSequenceTrackProperty_OffsetTime = 1,
	kSequenceTrackProperty_MuteStatus = 2,
	kSequenceTrackProperty_SoloStatus = 3,
	kSequenceTrackProperty_AutomatedParameters = 4,
	kSequenceTrackProperty_TrackLength = 5,
	kSequenceTrackProperty_TimeResolution = 6
};

// The value of kSequenceTrackProperty_LoopInfo.
typedef struct MusicTrackLoopInfo
{
	MusicTimeStamp loopDuration;
	SInt32 numberOfLoops;
} MusicTrackLoopInfo;

#ifdef __cplusplus
}
#endif

#endif
