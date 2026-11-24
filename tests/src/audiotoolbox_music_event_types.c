// Copyright (C) 2026 Lubos Dolezel
//
// The event types of a MusicTrack (MusicPlayer.h) and the editing functions of
// MusicEventIterator. Every MusicTrackNew...Event stores the documented structure at its time
// stamp; the variable length ones (MIDIRawData, MIDIMetaEvent, MusicEventUserData and
// ExtendedNoteOnEvent, whose MusicDeviceNoteParams carry argCount - 2 controls) are stored with
// their bytes. SetEventInfo changes type and data but not the time; SetEventTime moves the event
// and the iterator keeps its place; DeleteEvent leaves the iterator at the next event, or beyond the
// last. Results, sizes and orders that the documentation leaves open are what macOS 26.7.3 and
// 10.14.4 do, observed with these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void info(MusicEventIterator it, MusicTimeStamp* t, MusicEventType* type, const void** data, UInt32* size)
{
	CHECK(MusicEventIteratorGetEventInfo(it, t, type, data, size) == noErr);
}

int main(void)
{
	MusicSequence seq = NULL;
	MusicTrack track = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &track) == noErr);

	MIDIChannelMessage channel = { 0xB1, 7, 100, 0 };
	CHECK(MusicTrackNewMIDIChannelEvent(track, 0.0, &channel) == noErr);

	ParameterEvent param = { 5, kAudioUnitScope_Global, 0, 0.25f };
	CHECK(MusicTrackNewParameterEvent(track, 1.0, &param) == noErr);

	// A system exclusive message: the header and 3 bytes.
	MIDIRawData* raw = malloc(sizeof(MIDIRawData) + 3);
	raw->length = 3;
	raw->data[0] = 0xF0; raw->data[1] = 0x7E; raw->data[2] = 0xF7;
	CHECK(MusicTrackNewMIDIRawDataEvent(track, 2.0, raw) == noErr);

	MIDIMetaEvent* meta = malloc(sizeof(MIDIMetaEvent) + 5);
	memset(meta, 0, offsetof(MIDIMetaEvent, data));
	meta->metaEventType = 0x05;
	meta->dataLength = 5;
	memcpy(meta->data, "hello", 5);
	CHECK(MusicTrackNewMetaEvent(track, 3.0, meta) == noErr);

	MusicEventUserData* user = malloc(sizeof(MusicEventUserData) + 2);
	user->length = 2;
	user->data[0] = 0xAB; user->data[1] = 0xCD;
	CHECK(MusicTrackNewUserEvent(track, 4.0, user) == noErr);

	// An extended note with two controls: argCount is 4.
	size_t noteSize = offsetof(ExtendedNoteOnEvent, extendedParams) + offsetof(MusicDeviceNoteParams, mControls)
		+ 2 * sizeof(NoteParamsControlValue);
	ExtendedNoteOnEvent* note = calloc(1, noteSize);
	note->instrumentID = 3; note->groupID = 2; note->duration = 1.5f;
	note->extendedParams.argCount = 4;
	note->extendedParams.mPitch = 60.5f; note->extendedParams.mVelocity = 90;
	note->extendedParams.mControls[0].mID = 10; note->extendedParams.mControls[0].mValue = 0.5f;
	note->extendedParams.mControls[1].mID = 11; note->extendedParams.mControls[1].mValue = 0.75f;
	CHECK(MusicTrackNewExtendedNoteEvent(track, 5.0, note) == noErr);

	// An extended note with only a pitch and a velocity (argCount 2).
	ExtendedNoteOnEvent plain;
	memset(&plain, 0, sizeof(plain));
	plain.extendedParams.argCount = 2;
	plain.extendedParams.mPitch = 64; plain.extendedParams.mVelocity = 80; plain.duration = 2.0f;
	CHECK(MusicTrackNewExtendedNoteEvent(track, 6.0, &plain) == noErr);

	CFStringRef keys[1] = { CFSTR("name") };
	CFStringRef values[1] = { CFSTR("preset") };
	CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void**) keys, (const void**) values, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	AUPresetEvent preset = { kAudioUnitScope_Global, 0, dict };
	// The track keeps the property list of an AU preset event alive (it retains it), so the caller may release
	// its own reference (observed on macOS 26.7.3 and 10.14.4).
	CFIndex retainsBefore = CFGetRetainCount(dict);
	CHECK(MusicTrackNewAUPresetEvent(track, 7.0, &preset) == noErr);
	CHECK(CFGetRetainCount(dict) == retainsBefore + 1);

	MusicEventIterator it = NULL;
	CHECK(NewMusicEventIterator(track, &it) == noErr);
	MusicTimeStamp t;
	MusicEventType type;
	const void* data;
	UInt32 size;

	info(it, &t, &type, &data, &size);
	CHECK(t == 0.0 && type == kMusicEventType_MIDIChannelMessage && size == sizeof(MIDIChannelMessage));
	CHECK(((const MIDIChannelMessage*) data)->status == 0xB1 && ((const MIDIChannelMessage*) data)->data2 == 100);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 1.0 && type == kMusicEventType_Parameter && size == sizeof(ParameterEvent));
	CHECK(((const ParameterEvent*) data)->parameterID == 5 && ((const ParameterEvent*) data)->value == 0.25f);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 2.0 && type == kMusicEventType_MIDIRawData);
	CHECK(size == offsetof(MIDIRawData, data) + 3);
	CHECK(((const MIDIRawData*) data)->length == 3);
	CHECK(((const MIDIRawData*) data)->data[0] == 0xF0 && ((const MIDIRawData*) data)->data[2] == 0xF7);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 3.0 && type == kMusicEventType_Meta);
	CHECK(size == offsetof(MIDIMetaEvent, data) + 5);
	CHECK(((const MIDIMetaEvent*) data)->metaEventType == 0x05 && ((const MIDIMetaEvent*) data)->dataLength == 5);
	CHECK(memcmp(((const MIDIMetaEvent*) data)->data, "hello", 5) == 0);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 4.0 && type == kMusicEventType_User);
	CHECK(size == offsetof(MusicEventUserData, data) + 2);
	CHECK(((const MusicEventUserData*) data)->length == 2 && ((const MusicEventUserData*) data)->data[1] == 0xCD);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 5.0 && type == kMusicEventType_ExtendedNote);
	// QUIRK test: the stored size of an extended note is 28 bytes and 4 for each argument (argCount is 4 here: 44,
	// where the structure made above is 40 bytes); observed on macOS 26.7.3 and 10.14.4
	CHECK(noteSize == 40 && size == 28 + 4 * 4);
	const ExtendedNoteOnEvent* gotNote = data;
	CHECK(gotNote->instrumentID == 3 && gotNote->groupID == 2 && gotNote->duration == 1.5f);
	CHECK(gotNote->extendedParams.argCount == 4 && gotNote->extendedParams.mPitch == 60.5f);
	CHECK(gotNote->extendedParams.mControls[1].mID == 11 && gotNote->extendedParams.mControls[1].mValue == 0.75f);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 6.0 && type == kMusicEventType_ExtendedNote);
	// QUIRK test: with only a pitch and a velocity (argCount 2) the stored size is 36, more than
	// sizeof(ExtendedNoteOnEvent), which is 32
	CHECK(size == 36 && sizeof(ExtendedNoteOnEvent) == 32);
	CHECK(((const ExtendedNoteOnEvent*) data)->extendedParams.argCount == 2);
	CHECK(((const ExtendedNoteOnEvent*) data)->extendedParams.mPitch == 64);

	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 7.0 && type == kMusicEventType_AUPreset && size == sizeof(AUPresetEvent));
	CHECK(CFEqual(((const AUPresetEvent*) data)->preset, dict));

	// The track keeps its own copy of the bytes.
	raw->data[0] = 0;
	CHECK(MusicEventIteratorSeek(it, 2.0) == noErr);
	info(it, NULL, NULL, &data, NULL);
	CHECK(((const MIDIRawData*) data)->data[0] == 0xF0);

	// Structures that cannot be stored.
	CHECK(MusicTrackNewMIDIRawDataEvent(track, 8.0, NULL) == kAudio_ParamError);
	AUPresetEvent noPreset = { kAudioUnitScope_Global, 0, NULL };
	CHECK(MusicTrackNewAUPresetEvent(track, 8.0, &noPreset) == kAudio_ParamError);

	// What macOS 26.7.3 and 10.14.4 accept: an extended note with fewer than the two arguments of pitch and
	// velocity (the stored size is 28 + 4 argCount), and a time stamp below zero. A track of its own takes them,
	// as an event with a time stamp below zero is not kept in the order of the others there.
	MusicTrack scratch = NULL, below = NULL;
	CHECK(MusicSequenceNewTrack(seq, &scratch) == noErr && MusicSequenceNewTrack(seq, &below) == noErr);
	for (UInt32 args = 0; args < 2; args++)
	{
		plain.extendedParams.argCount = args;
		CHECK(MusicTrackNewExtendedNoteEvent(scratch, 8.0 + args, &plain) == noErr);
	}
	CHECK(MusicTrackNewUserEvent(below, -1.0, user) == noErr);
	MusicEventIterator scratchIt = NULL;
	CHECK(NewMusicEventIterator(scratch, &scratchIt) == noErr);
	info(scratchIt, NULL, NULL, NULL, &size);
	CHECK(size == 28);
	CHECK(MusicEventIteratorNextEvent(scratchIt) == noErr);
	info(scratchIt, NULL, NULL, NULL, &size);
	CHECK(size == 32);
	CHECK(DisposeMusicEventIterator(scratchIt) == noErr);

	// SetEventInfo: the type and the data change, the time and the position stay.
	CHECK(MusicEventIteratorSeek(it, 1.0) == noErr);
	MIDINoteMessage newNote = { 4, 72, 99, 0, 0.5f };
	CHECK(MusicEventIteratorSetEventInfo(it, kMusicEventType_MIDINoteMessage, &newNote) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(t == 1.0 && type == kMusicEventType_MIDINoteMessage && size == sizeof(MIDINoteMessage));
	CHECK(((const MIDINoteMessage*) data)->note == 72);
	// Back to a variable length type.
	CHECK(MusicEventIteratorSetEventInfo(it, kMusicEventType_User, user) == noErr);
	info(it, &t, &type, &data, &size);
	CHECK(type == kMusicEventType_User && size == offsetof(MusicEventUserData, data) + 2);
	// A type without an event structure leaves the event as it was.
	CHECK(MusicEventIteratorSetEventInfo(it, kMusicEventType_NULL, &newNote) == kAudioToolboxErr_InvalidEventType);
	CHECK(MusicEventIteratorSetEventInfo(it, 77, &newNote) == kAudioToolboxErr_InvalidEventType);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 1.0 && type == kMusicEventType_User);

	// SetEventTime: the event moves to its new place and the iterator keeps its position, which then holds
	// another event. Now the events are the user event at 1, the raw data at 2, the meta event at 3, the
	// channel message at 0, the user event at 4, ...
	CHECK(MusicEventIteratorSeek(it, 0.0) == noErr);
	CHECK(MusicEventIteratorSetEventTime(it, 3.5) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 1.0 && type == kMusicEventType_User);
	Boolean b = 1;
	CHECK(MusicEventIteratorHasPreviousEvent(it, &b) == noErr && b == 0);
	CHECK(MusicEventIteratorSeek(it, 3.5) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 3.5 && type == kMusicEventType_MIDIChannelMessage);
	// QUIRK test: moved to an earlier time that other events have, the event goes after them: the channel message
	// to 2, where the raw data is; observed on macOS 26.7.3 and 10.14.4
	CHECK(MusicEventIteratorSetEventTime(it, 2.0) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 3.0 && type == kMusicEventType_Meta);
	CHECK(MusicEventIteratorPreviousEvent(it) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 2.0 && type == kMusicEventType_MIDIChannelMessage);
	CHECK(MusicEventIteratorPreviousEvent(it) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 2.0 && type == kMusicEventType_MIDIRawData);
	// QUIRK test: moved to a later time (or the same one) that other events have, it goes before them: the user
	// event at 1 to 2
	CHECK(MusicEventIteratorSeek(it, 1.0) == noErr);
	CHECK(MusicEventIteratorSetEventTime(it, 2.0) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 2.0 && type == kMusicEventType_User);
	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 2.0 && type == kMusicEventType_MIDIRawData);
	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 2.0 && type == kMusicEventType_MIDIChannelMessage);
	// A time below zero is accepted (and sorts before the others here)
	CHECK(MusicEventIteratorSeek(it, 0.0) == noErr);
	CHECK(MusicEventIteratorSetEventTime(it, -0.5) == noErr);
	MusicEventIterator front = NULL;
	CHECK(NewMusicEventIterator(track, &front) == noErr);
	info(front, &t, &type, NULL, NULL);
	CHECK(t == -0.5);
	CHECK(DisposeMusicEventIterator(front) == noErr);

	// DeleteEvent: the iterator then points at the next event.
	CHECK(MusicEventIteratorSeek(it, 3.0) == noErr);
	CHECK(MusicEventIteratorDeleteEvent(it) == noErr);
	info(it, &t, &type, NULL, NULL);
	CHECK(t == 4.0 && type == kMusicEventType_User);
	// Deleting the last event leaves the iterator beyond the track's events.
	CHECK(MusicEventIteratorSeek(it, 7.0) == noErr);
	CHECK(MusicEventIteratorDeleteEvent(it) == noErr);
	CHECK(CFGetRetainCount(dict) == retainsBefore);
	b = 1;
	CHECK(MusicEventIteratorHasCurrentEvent(it, &b) == noErr && b == 0);
	// QUIRK test: with no current event DeleteEvent and SetEventTime do nothing and return noErr, SetEventInfo
	// gives kAudioToolboxErr_EndOfTrack (observed on macOS 26.7.3 and 10.14.4)
	CHECK(MusicEventIteratorDeleteEvent(it) == noErr);
	CHECK(MusicEventIteratorSetEventTime(it, 1.0) == noErr);
	CHECK(MusicEventIteratorSetEventInfo(it, kMusicEventType_MIDINoteMessage, &newNote) == kAudioToolboxErr_EndOfTrack);
	CHECK(MusicEventIteratorDeleteEvent(NULL) == kAudio_ParamError);

	// The event count is what the edits leave: 8 added, 2 deleted.
	CHECK(MusicEventIteratorSeek(it, kMusicTimeStamp_EndOfTrack) == noErr);
	int count = 0;
	while (MusicEventIteratorHasPreviousEvent(it, &b) == noErr && b && count < 20)
	{
		CHECK(MusicEventIteratorPreviousEvent(it) == noErr);
		count++;
	}
	CHECK(count == 6);

	CHECK(DisposeMusicEventIterator(it) == noErr);
	CHECK(DisposeMusicSequence(seq) == noErr);
	CFRelease(dict);
	free(raw); free(meta); free(user); free(note);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
