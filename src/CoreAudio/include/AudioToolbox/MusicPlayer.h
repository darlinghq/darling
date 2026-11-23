#ifndef _MUSIC_PLAYER_H
#define _MUSIC_PLAYER_H

// Music sequencing: sequences, tracks, event iterators and players.
// https://developer.apple.com/documentation/audiotoolbox/music-player

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/MusicDevice.h>
#include <AudioToolbox/AUGraph.h>
#include <float.h>

// A MIDI endpoint is a number. CoreMIDI declares the same type; Darling's MIDIServices.h does not
// declare it yet, and a second typedef of the same type is allowed.
typedef UInt32 MIDIEndpointRef;

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

// Result codes of the music sequencing functions. The SDK header lists the last one, which is not
// in the same numeric range, with the others; its name says it is for a track with no destination.
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

// Music players

extern OSStatus
NewMusicPlayer(MusicPlayer *outPlayer);

extern OSStatus
DisposeMusicPlayer(MusicPlayer inPlayer);

extern OSStatus
MusicPlayerSetSequence(MusicPlayer inPlayer, MusicSequence inSequence);

extern OSStatus
MusicPlayerGetSequence(MusicPlayer inPlayer, MusicSequence *outSequence);

extern OSStatus
MusicPlayerSetTime(MusicPlayer inPlayer, MusicTimeStamp inTime);

extern OSStatus
MusicPlayerGetTime(MusicPlayer inPlayer, MusicTimeStamp *outTime);

extern OSStatus
MusicPlayerGetHostTimeForBeats(MusicPlayer inPlayer, MusicTimeStamp inBeats, UInt64 *outHostTime);

extern OSStatus
MusicPlayerGetBeatsForHostTime(MusicPlayer inPlayer, UInt64 inHostTime, MusicTimeStamp *outBeats);

extern OSStatus
MusicPlayerPreroll(MusicPlayer inPlayer);

extern OSStatus
MusicPlayerStart(MusicPlayer inPlayer);

extern OSStatus
MusicPlayerStop(MusicPlayer inPlayer);

extern OSStatus
MusicPlayerIsPlaying(MusicPlayer inPlayer, Boolean *outIsPlaying);

extern OSStatus
MusicPlayerSetPlayRateScalar(MusicPlayer inPlayer, Float64 inScaleRate);

extern OSStatus
MusicPlayerGetPlayRateScalar(MusicPlayer inPlayer, Float64 *outScaleRate);

// Music sequences

extern OSStatus
NewMusicSequence(MusicSequence *outSequence);

extern OSStatus
DisposeMusicSequence(MusicSequence inSequence);

extern OSStatus
MusicSequenceNewTrack(MusicSequence inSequence, MusicTrack *outTrack);

extern OSStatus
MusicSequenceDisposeTrack(MusicSequence inSequence, MusicTrack inTrack);

extern OSStatus
MusicSequenceGetTrackCount(MusicSequence inSequence, UInt32 *outNumberOfTracks);

extern OSStatus
MusicSequenceGetIndTrack(MusicSequence inSequence, UInt32 inTrackIndex, MusicTrack *outTrack);

extern OSStatus
MusicSequenceGetTrackIndex(MusicSequence inSequence, MusicTrack inTrack, UInt32 *outTrackIndex);

extern OSStatus
MusicSequenceGetTempoTrack(MusicSequence inSequence, MusicTrack *outTrack);

extern OSStatus
MusicSequenceSetSequenceType(MusicSequence inSequence, MusicSequenceType inType);

extern OSStatus
MusicSequenceGetSequenceType(MusicSequence inSequence, MusicSequenceType *outType);

extern OSStatus
MusicSequenceGetSecondsForBeats(MusicSequence inSequence, MusicTimeStamp inBeats, Float64 *outSeconds);

extern OSStatus
MusicSequenceGetBeatsForSeconds(MusicSequence inSequence, Float64 inSeconds, MusicTimeStamp *outBeats);

// The 16 bit division field of a MIDI file header for a SMPTE time base: the frame rate as a
// negative number in the high byte (a positive rate is negated) and the ticks per frame in the low
// bits.
CF_INLINE SInt16
MusicSequenceSetSMPTEResolution(SignedByte fps, Byte ticks)
{
	SInt16 frames = fps < 0 ? fps : -fps;
	return (SInt16)(frames * 256 + ticks);
}

CF_INLINE void
MusicSequenceGetSMPTEResolution(SInt16 inRes, SignedByte *fps, Byte *ticks)
{
	*fps = (SignedByte)(inRes >> 8);
	*ticks = (Byte)(inRes & 0x7F);
}

extern OSStatus
MusicSequenceSetAUGraph(MusicSequence inSequence, AUGraph inGraph);

extern OSStatus
MusicSequenceGetAUGraph(MusicSequence inSequence, AUGraph *outGraph);

extern OSStatus
MusicSequenceSetMIDIEndpoint(MusicSequence inSequence, MIDIEndpointRef inEndpoint);

extern OSStatus
MusicSequenceSetUserCallback(MusicSequence inSequence, MusicSequenceUserCallback inCallback, void *inClientData);

// Music tracks

extern OSStatus
MusicTrackGetSequence(MusicTrack inTrack, MusicSequence *outSequence);

extern OSStatus
MusicTrackNewMIDINoteEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDINoteMessage *inMessage);

extern OSStatus
MusicTrackNewMIDIChannelEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDIChannelMessage *inMessage);

extern OSStatus
MusicTrackNewMIDIRawDataEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDIRawData *inRawData);

extern OSStatus
MusicTrackNewExtendedNoteEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const ExtendedNoteOnEvent *inInfo);

extern OSStatus
MusicTrackNewParameterEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const ParameterEvent *inInfo);

extern OSStatus
MusicTrackNewExtendedTempoEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, Float64 inBPM);

extern OSStatus
MusicTrackNewMetaEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDIMetaEvent *inMetaEvent);

extern OSStatus
MusicTrackNewUserEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MusicEventUserData *inUserData);

extern OSStatus
MusicTrackNewAUPresetEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const AUPresetEvent *inPresetEvent);

extern OSStatus
MusicTrackSetDestNode(MusicTrack inTrack, AUNode inNode);

extern OSStatus
MusicTrackSetDestMIDIEndpoint(MusicTrack inTrack, MIDIEndpointRef inEndpoint);

extern OSStatus
MusicTrackGetDestNode(MusicTrack inTrack, AUNode *outNode);

extern OSStatus
MusicTrackGetDestMIDIEndpoint(MusicTrack inTrack, MIDIEndpointRef *outEndpoint);

extern OSStatus
MusicTrackMoveEvents(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime, MusicTimeStamp inMoveTime);

extern OSStatus
MusicTrackClear(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime);

extern OSStatus
MusicTrackCut(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime);

extern OSStatus
MusicTrackCopyInsert(MusicTrack inSourceTrack, MusicTimeStamp inSourceStartTime, MusicTimeStamp inSourceEndTime,
	MusicTrack inDestTrack, MusicTimeStamp inDestInsertTime);

extern OSStatus
MusicTrackMerge(MusicTrack inSourceTrack, MusicTimeStamp inSourceStartTime, MusicTimeStamp inSourceEndTime,
	MusicTrack inDestTrack, MusicTimeStamp inDestInsertTime);

extern OSStatus
MusicTrackSetProperty(MusicTrack inTrack, UInt32 inPropertyID, void *inData, UInt32 inLength);

extern OSStatus
MusicTrackGetProperty(MusicTrack inTrack, UInt32 inPropertyID, void *outData, UInt32 *ioLength);

// Music event iterators

extern OSStatus
NewMusicEventIterator(MusicTrack inTrack, MusicEventIterator *outIterator);

extern OSStatus
DisposeMusicEventIterator(MusicEventIterator inIterator);

extern OSStatus
MusicEventIteratorSeek(MusicEventIterator inIterator, MusicTimeStamp inTimeStamp);

extern OSStatus
MusicEventIteratorNextEvent(MusicEventIterator inIterator);

extern OSStatus
MusicEventIteratorPreviousEvent(MusicEventIterator inIterator);

extern OSStatus
MusicEventIteratorGetEventInfo(MusicEventIterator inIterator,
	MusicTimeStamp *outTimeStamp,
	MusicEventType *outEventType,
	const void **outEventData,
	UInt32 *outEventDataSize);

extern OSStatus
MusicEventIteratorSetEventInfo(MusicEventIterator inIterator, MusicEventType inEventType, const void *inEventData);

extern OSStatus
MusicEventIteratorSetEventTime(MusicEventIterator inIterator, MusicTimeStamp inTimeStamp);

extern OSStatus
MusicEventIteratorDeleteEvent(MusicEventIterator inIterator);

extern OSStatus
MusicEventIteratorHasPreviousEvent(MusicEventIterator inIterator, Boolean *outHasPrevEvent);

extern OSStatus
MusicEventIteratorHasNextEvent(MusicEventIterator inIterator, Boolean *outHasNextEvent);

extern OSStatus
MusicEventIteratorHasCurrentEvent(MusicEventIterator inIterator, Boolean *outHasCurEvent);

#ifdef __cplusplus
}
#endif

#endif
