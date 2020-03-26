/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUInstrument Base Classes
*/

#include "SynthNoteList.h"
#include <stdexcept>

void SynthNoteList::SanityCheck() const
{
	if (mState >= kNoteState_Unset) {
		throw std::runtime_error("SanityCheck: mState is bad");
	}
	
	if (mHead == NULL) {
		if (mTail != NULL) 
			throw std::runtime_error("SanityCheck: mHead is NULL but not mTail");
		return;
	}
	if (mTail == NULL) {
		throw std::runtime_error("SanityCheck: mTail is NULL but not mHead");
	}
	
	if (mHead->mPrev) {
		throw std::runtime_error("SanityCheck: mHead has a mPrev");
	}
	if (mTail->mNext) {
		throw std::runtime_error("SanityCheck: mTail has a mNext");
	}
	
	SynthNote *note = mHead;
	while (note)
	{
		if (note->mState != mState)
			throw std::runtime_error("SanityCheck: note in wrong state");
		if (note->mNext) {
			if (note->mNext->mPrev != note)
				throw std::runtime_error("SanityCheck: bad link 1");
		} else {
			if (mTail != note)
				throw std::runtime_error("SanityCheck: note->mNext is nil, but mTail != note");
		}
		if (note->mPrev) {
			if (note->mPrev->mNext != note)
				throw std::runtime_error("SanityCheck: bad link 2");
		} else {
			if (mHead != note)
				throw std::runtime_error("SanityCheck: note->mPrev is nil, but mHead != note");
		}
		note = note->mNext;
	}
}
