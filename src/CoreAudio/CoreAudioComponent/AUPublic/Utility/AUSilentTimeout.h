/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#ifndef __AUSilentTimeout
#define __AUSilentTimeout

class AUSilentTimeout
{
public:
	AUSilentTimeout()
		: 	mTimeoutCounter(0),
			mResetTimer(true)
				{};

	void				Process(UInt32 inFramesToProcess, UInt32 inTimeoutLimit, bool &ioSilence )
	{
		if(ioSilence )
		{
			if(mResetTimer )
			{
				mTimeoutCounter = inTimeoutLimit;
				mResetTimer = false;
			}
			
			if(mTimeoutCounter > 0 )
			{
				mTimeoutCounter -= inFramesToProcess;
				ioSilence = false;
			}
		}
		else
		{
			// signal to reset the next time we receive silence
			mResetTimer = true;	
		}
	}

	void				Reset()
	{
		mResetTimer = true;
	};



private:
	SInt32				mTimeoutCounter;
	bool				mResetTimer;
};

#endif // __AUSilentTimeout
