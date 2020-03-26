/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#ifndef __CAReferenceCounted_h__
#define __CAReferenceCounted_h__

#include "CAAtomic.h"

// base class for reference-counted objects
class CAReferenceCounted {
public:
	CAReferenceCounted() : mRefCount(1) {}
	
	void	retain() { CAAtomicIncrement32(&mRefCount); }
	
	void	release() 
			{ 
				SInt32 rc = CAAtomicDecrement32(&mRefCount);
				if (rc == 0) {
					releaseObject();
				}
			}


	class Retainer {
	public:
		Retainer(CAReferenceCounted *obj) : mObject(obj) { mObject->retain(); }
		~Retainer() { mObject->release(); }
	
	private:
		CAReferenceCounted *	mObject;
	};

protected:
    virtual	~CAReferenceCounted() { }
	
	virtual void releaseObject () 
			{ 
				delete this; 
			}

#if DEBUG
public:
#endif
	SInt32	GetReferenceCount() const { return mRefCount; }
private:
	SInt32		mRefCount;

	CAReferenceCounted(const CAReferenceCounted &a);
	CAReferenceCounted &operator=(const CAReferenceCounted &a);
};


#endif // __CAReferenceCounted_h__
