/*
     File: AUCarbonViewBase.h
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
#ifndef __AUCarbonViewBase_h__
#define __AUCarbonViewBase_h__

#include <vector>
#include "AUCarbonViewControl.h"
#include "ComponentBase.h"

static const Float32 kDefaultNotificationInterval = 0.100;

	/*! @class AUCarbonViewBase */
class AUCarbonViewBase : public ComponentBase, public CarbonEventHandler
{
public:
    /*! @ctor AUCarbonViewBase */
    AUCarbonViewBase (	AudioUnitCarbonView inInstance,
						Float32 inNotificationInterval = kDefaultNotificationInterval /* in seconds */);
	/*! @dtor ~AUCarbonViewBase */
	virtual ~AUCarbonViewBase();
	
	// AUViewBase overrides
	/*! @method CreateCarbonView */
	virtual OSStatus			CreateCarbonView (AudioUnit inAudioUnit, WindowRef inWindow, ControlRef inParentControl, const Float32Point &inLocation, const Float32Point &inSize, ControlRef &outParentControl);

	// our own virtual methods
	/*! @method CreateUI */
	virtual OSStatus			CreateUI (Float32	inXOffset, Float32 	inYOffset);

	/*! @method HandleEvent */
	virtual bool				HandleEvent (EventHandlerCallRef inHandlerRef, EventRef event);
	
	/*! @method GetEditAudioUnit */
	const AudioUnit				GetEditAudioUnit () const { return mEditAudioUnit; }
	//
	/*! @method ComponentEntryDispatch */
	static OSStatus			ComponentEntryDispatch (
									ComponentParameters *			params,
									AUCarbonViewBase *				This);

	/*! @method AddCarbonControl */
	void						AddCarbonControl (
									AUCarbonViewControl::ControlType	type, 
									const CAAUParameter &				param, 
									ControlRef							control);

	/*! @method GetCarbonWindow */
	WindowRef					GetCarbonWindow () { return mCarbonWindow; }
	/*! @method GetCarbonPane */
	ControlRef					GetCarbonPane () { return mCarbonPane; }
	/*! @method EmbedControl */
	OSStatus					EmbedControl (ControlRef ctl);
	
	/*! @method TellListener */
	void						TellListener (const CAAUParameter &auvp, AudioUnitCarbonViewEventID event, void *evpar);
	
	// pass in true if wanting an update to the view and you're calling this from a thread
	// that is safe to do UI in.
	// If you don't know, pass in false!
	/*! @method Update */
	void						Update (bool inUIThread);
	
	/*! @method GetXOffset */
	Float32						GetXOffset () { return mXOffset; }
	/*! @method GetYOffset */
	Float32						GetYOffset () { return mYOffset; }
	
	/*! @method ClearControls */
	void						ClearControls ();
	
	/*! @method IsCompositWindow */
	bool						IsCompositWindow () const { return mCompositWindow; }
	
protected:
#if !__LP64__
	/*! @method SetEventListener */
	void						SetEventListener (AudioUnitCarbonViewEventListener listener, void *userData)
								{
									mEventListener = listener;
									mEventListenerUserData = userData;
								}
#endif

	/*! @method AddControl */
	void						AddControl (AUCarbonViewControl *control);
	/*! @method RemoveControl */
	void						RemoveControl (AUCarbonViewControl *control);

	OSStatus					CreateEventLoopTimer (Float32 inDelay, Float32 inInterval);
	
	/*! @method ParameterListener */
	static void ParameterListener (void *						inCallbackRefCon,
									void *						inObject,
									const AudioUnitEvent *		inEvent,
									UInt64						inEventHostTime,
									Float32						inParameterValue);
									
	static pascal void TheTimerProc (	EventLoopTimerRef 		inTimer, 
										void *					inUserData);

	virtual void 				RespondToEventTimer (EventLoopTimerRef inTimer);

	/*! @var mEditAudioUnit */
	AudioUnit					mEditAudioUnit;	// the AU we're controlling
	/*! @var mParameterListener */
	AUEventListenerRef			mParameterListener;

#if !__LP64__
	/*! @var mEventListener */
	AudioUnitCarbonViewEventListener
								mEventListener;
#endif

	/*! @var mEventListenerUserData */
	void *						mEventListenerUserData;

private:
	typedef std::vector<AUCarbonViewControl *> ControlList;
	/*! @var mControlList */
	ControlList					mControlList;

	EventLoopTimerRef 			mTimerRef;

	EventLoopTimerUPP			mTimerUPP;

protected:
	/*! @var mCarbonWindow */
	WindowRef			mCarbonWindow;
	/*! @var mCarbonPane */
	ControlRef			mCarbonPane;	// user pane, contains all other controls
	/*! @var mBottomRight */
	Point				mBottomRight;	// largest width and height of child controls
	/*! @var mXOffset */
	Float32				mXOffset;
	/*! @var mYOffset */
	Float32				mYOffset;
	/*! @var mCompositWindow */
	bool				mCompositWindow;
	/*! @var mCurrentScrollPoint */
	HIPoint				mCurrentScrollPoint;		// needed for scrolling
};


#endif // __AUCarbonViewBase_h__
