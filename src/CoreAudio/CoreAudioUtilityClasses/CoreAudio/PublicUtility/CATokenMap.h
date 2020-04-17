/*
     File: CATokenMap.h
 Abstract: Part of CoreAudio Utility Classes
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
*/
#if !defined(__CATokenMap_h__)
#define __CATokenMap_h__

//=============================================================================
//	Includes
//=============================================================================

//  System Includes
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
#else
	#include <CoreAudioTypes.h>
#endif

//  PublicUtility Includes
#include "CAMutex.h"

//  Standard Library Includes
#include <map>
#include <iterator>

//=============================================================================
//	CATokenMap
//=============================================================================

template <class T>
class	CATokenMap
{

//	Types
private:
	typedef std::map<UInt32, T*>	TokenMap;

//	Construction/Destruction
public:
			CATokenMap() : mTokenMap(), mNextToken(256), mTokenMapMutex("CATokenMap Mutex") {}
			~CATokenMap() {}

//	Operations
public:
	bool	Lock()
	{
		return mTokenMapMutex.Lock();
	}
	
	void	Unlock()
	{
		mTokenMapMutex.Unlock();
	}

	UInt32	GetToken(T* inObject) const
	{
		CAMutex::Locker theLocker(const_cast<CAMutex&>(mTokenMapMutex));
		UInt32 theAnswer = 0;
		typename TokenMap::const_iterator i = mTokenMap.begin();
		while((theAnswer == 0) && (i != mTokenMap.end()))
		{
			if(i->second == inObject)
			{
				theAnswer = i->first;
			}
			std::advance(i, 1);
		}
		return theAnswer;
	}
	
	T*		GetObject(UInt32 inToken) const
	{
		CAMutex::Locker theLocker(const_cast<CAMutex&>(mTokenMapMutex));
		typename TokenMap::const_iterator i = mTokenMap.find(inToken);
		return i != mTokenMap.end() ? i->second : NULL;
	}

	T*		GetObject(const void* inToken) const
	{
		#if __LP64__
			return GetObject((UInt32)((UInt64)inToken));
		#else
			return GetObject((UInt32)inToken);
		#endif
	}
	
	UInt32	GetNumberObjects() const
	{
		CAMutex::Locker theLocker(const_cast<CAMutex&>(mTokenMapMutex));
		return static_cast<UInt32>(mTokenMap.size());
	}
	
	T*		GetObjectByIndex(UInt32 inIndex) const
	{
		T* theAnswer = NULL;
		CAMutex::Locker theLocker(const_cast<CAMutex&>(mTokenMapMutex));
		if(inIndex < mTokenMap.size())
		{
			typename TokenMap::const_iterator i = mTokenMap.begin();
			std::advance(i, inIndex);
			theAnswer = (i != mTokenMap.end()) ? i->second : NULL;
		}
		return theAnswer;
	}
	
	void	AddMapping(UInt32 inToken, T* inObject)
	{
		CAMutex::Locker theLocker(mTokenMapMutex);
		typename TokenMap::iterator i = mTokenMap.find(inToken);
		if(i != mTokenMap.end())
		{
			i->second = inObject;
		}
		else
		{
			mTokenMap.insert(typename TokenMap::value_type(inToken, inObject));
		}
	}
	
	void	RemoveMapping(UInt32 inToken, T* /*inObject*/)
	{
		CAMutex::Locker theLocker(mTokenMapMutex);
		typename TokenMap::iterator i = mTokenMap.find(inToken);
		if(i != mTokenMap.end())
		{
			mTokenMap.erase(i);
		}
	}
	
	UInt32	GetNextToken()
	{
		return mNextToken++;
	}
	
	UInt32	MapObject(T* inObject)
	{
		CAMutex::Locker theLocker(mTokenMapMutex);
		UInt32 theToken = GetNextToken();
		mTokenMap.insert(typename TokenMap::value_type(theToken, inObject));
		return theToken;
	}
	
	void	UnmapObject(T* inObject)
	{
		CAMutex::Locker theLocker(mTokenMapMutex);
		bool isDone = false;
		typename TokenMap::iterator i = mTokenMap.begin();
		while(!isDone && (i != mTokenMap.end()))
		{
			if(i->second == inObject)
			{
				mTokenMap.erase(i);
				isDone = true;
			}
			else
			{
				std::advance(i, 1);
			}
		}
	}

//	Implementation
private:	
	TokenMap	mTokenMap;
	UInt32		mNextToken;
	CAMutex		mTokenMapMutex;

};

#endif
