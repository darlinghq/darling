/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
*/

#ifndef __SynthNoteList__
#define __SynthNoteList__

#include "SynthNote.h"

#if DEBUG
#ifndef DEBUG_PRINT
	#define DEBUG_PRINT 0
#endif
	#define USE_SANITY_CHECK 0
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SynthNoteList
{
	SynthNoteList() : mState(kNoteState_Unset), mHead(0), mTail(0) {}
	
	bool NotEmpty() const { return mHead != NULL; }
	bool IsEmpty() const { return mHead == NULL; }
	void Empty() { 
#if USE_SANITY_CHECK
		SanityCheck();
#endif
		mHead = mTail = NULL; 
	}
	
	UInt32 Length() const {
#if USE_SANITY_CHECK
		SanityCheck();
#endif
		UInt32 length = 0;
		for (SynthNote* note = mHead; note; note = note->mNext) 
			length++;
		return length;
	};
	
	void AddNote(SynthNote *inNote)
	{
#if DEBUG_PRINT
		printf("AddNote(inNote=%p) to state: %lu\n", inNote, mState);
#endif
#if USE_SANITY_CHECK
		SanityCheck();
#endif
		inNote->SetState(mState);
		inNote->mNext = mHead;
		inNote->mPrev = NULL;
		
		if (mHead) { mHead->mPrev = inNote; mHead = inNote; }
		else mHead = mTail = inNote;
#if USE_SANITY_CHECK
		SanityCheck();
#endif
	}
	
	void RemoveNote(SynthNote *inNote)
	{
#if DEBUG_PRINT
		printf("RemoveNote(inNote=%p) from state: %lu\n", inNote, mState);
#endif
#if USE_SANITY_CHECK
		SanityCheck();
#endif
		if (inNote->mPrev) inNote->mPrev->mNext = inNote->mNext;
		else mHead = inNote->mNext;

		if (inNote->mNext) inNote->mNext->mPrev = inNote->mPrev;
		else mTail = inNote->mPrev;
		
		inNote->mPrev = 0;
		inNote->mNext = 0;
#if USE_SANITY_CHECK
		SanityCheck();
#endif
	}

	void TransferAllFrom(SynthNoteList *inNoteList, UInt32 inFrame)
	{
#if DEBUG_PRINT
		printf("TransferAllFrom: from state %lu into state %lu\n", inNoteList->mState, mState);
#endif
#if USE_SANITY_CHECK
		SanityCheck();
		inNoteList->SanityCheck();
#endif
		if (!inNoteList->mTail) return;
		
		if (mState == kNoteState_Released)
		{
			for (SynthNote* note = inNoteList->mHead; note; note = note->mNext)
			{
#if DEBUG_PRINT
				printf("TransferAllFrom: releasing note %p\n", note);
#endif
				note->Release(inFrame);
				note->SetState(mState);
			}
		}
		else
		{
			for (SynthNote* note = inNoteList->mHead; note; note = note->mNext)
			{
				note->SetState(mState);
			}
		}
		
		inNoteList->mTail->mNext = mHead;
		
		if (mHead) mHead->mPrev = inNoteList->mTail;
		else mTail = inNoteList->mTail;
		
		mHead = inNoteList->mHead;
		
		inNoteList->mHead = NULL;
		inNoteList->mTail = NULL;
#if USE_SANITY_CHECK
		SanityCheck();
		inNoteList->SanityCheck();
#endif
	}
	
	SynthNote* FindOldestNote()
	{
#if DEBUG_PRINT
		printf("FindOldestNote\n");
#endif
#if USE_SANITY_CHECK
		SanityCheck();
#endif
		UInt64 minStartFrame = -1;
		SynthNote* oldestNote = NULL;
		for (SynthNote* note = mHead; note; note = note->mNext)
		{
			if (note->mAbsoluteStartFrame < minStartFrame)
			{
				oldestNote = note;
				minStartFrame = note->mAbsoluteStartFrame;
			}
		}
		return oldestNote;
	}
	
	SynthNote* FindMostQuietNote()
	{
#if DEBUG_PRINT
		printf("FindMostQuietNote\n");
#endif
		Float32 minAmplitude = 1e9f;
		UInt64 minStartFrame = -1;
		SynthNote* mostQuietNote = NULL;
		for (SynthNote* note = mHead; note; note = note->mNext)
		{
			Float32 amp = note->Amplitude();
#if DEBUG_PRINT
			printf("   amp %g   minAmplitude %g\n", amp, minAmplitude);
#endif
			if (amp < minAmplitude)
			{
				mostQuietNote = note;
				minAmplitude = amp;
				minStartFrame = note->mAbsoluteStartFrame;
			}
			else if (amp == minAmplitude && note->mAbsoluteStartFrame < minStartFrame)
			{
				// use earliest start time as a tie breaker
				mostQuietNote = note;
				minStartFrame = note->mAbsoluteStartFrame;
			}
		}
#if USE_SANITY_CHECK
		SanityCheck();
#endif
		return mostQuietNote;
	}
	
	void SanityCheck() const;
	
	SynthNoteState	mState;
	SynthNote *		mHead;
	SynthNote *		mTail;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
