/*
     File: AUCarbonViewControl.h
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
#ifndef __AUCarbonViewControl_h__
#define __AUCarbonViewControl_h__

#include <Carbon/Carbon.h>
#include <AudioUnit/AudioUnitCarbonView.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#include "CarbonEventHandler.h"
#include "CAAUParameter.h"

class AUCarbonViewBase;

// ____________________________________________________________________________
//	AUCarbonViewControl
//	Wrapper for a control that is wired to an AudioUnit parameter.
	/*! @class AUCarbonViewControl */
class AUCarbonViewControl : public CarbonEventHandler {
	// note that the controls are never disposed; that's managed by the AUCarbonViewBase's
	// parent pane which contains all of them ... if we later need to be able to delete
	// individual controls on the fly, extra work needed
public:
	enum ControlType {
		kTypeContinuous,	// e.g. slider
		kTypeDiscrete,		// e.g. pop-up menu
		kTypeText
	};
	
	AUCarbonViewControl(AUCarbonViewBase *ownerView, AUParameterListenerRef listener, ControlType type, const CAAUParameter &param, ControlRef control);
	~AUCarbonViewControl();

	/*! @method Bind */
	virtual void		Bind();		// second-stage construction

	/*! @method ControlToParameter */
	virtual void		ControlToParameter();
	/*! @method ParameterToControl */
	virtual void		ParameterToControl(Float32 newValue);
	
	/*! @method SetValueFract */
	virtual void		SetValueFract(double value);
	/*! @method GetValueFract */
	virtual double		GetValueFract();
	/*! @method SetTextValue */
	virtual void		SetTextValue(CFStringRef str);
	/*! @method GetTextValue */
	virtual CFStringRef	GetTextValue();
	/*! @method SetValue */
	virtual void		SetValue(long value);
	/*! @method GetValue */
	virtual long		GetValue();
	
	/*! @method GetOwnerView */
	AUCarbonViewBase * GetOwnerView() {return mOwnerView;}

	/*! @method Update */
	void				Update (bool inUIThread) 
	{ 
				if (inUIThread)
					ParameterToControl (mParam.GetValue());
				else
					AUParameterListenerNotify (mListener, this, &mParam);
	}
	
	
	// CarbonEventHandler overrides
	/*! @method HandleEvent */
	virtual bool		HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event);
	
	/*! @method ControlRef */
	operator ControlRef() { return mControl; }
	
	/*! @method SizeControlToFit */
	static Boolean SizeControlToFit(ControlRef inControl, SInt16 *outWidth = NULL, SInt16 *outHeight = NULL);
	
	/*! @method SliderTrackProc */
	static pascal void SliderTrackProc(ControlRef theControl, ControlPartCode partCode);
	/*! @method NumericKeyFilterCallback */
	static pascal ControlKeyFilterResult NumericKeyFilterCallback(ControlRef theControl, SInt16 *keyCode, SInt16 *charCode, 
															EventModifiers *modifiers);
protected:
	/*! @method ParamInfo */
	const AudioUnitParameterInfo &ParamInfo() { return mParam.ParamInfo(); }

	/*! @var mOwnerView */
	AUCarbonViewBase * 		mOwnerView;
	/*! @var mListener */
	AUParameterListenerRef	mListener;
	/*! @var mType */
	ControlType				mType;
	/*! @var mParam */
	CAAUParameter			mParam;

	/*! @var mControl */
	ControlRef				mControl;
	
	/*! @method StdKeyFilterCallback */
	static pascal ControlKeyFilterResult StdKeyFilterCallback(ControlRef theControl, SInt16 *keyCode,   SInt16 *charCode, 
																										EventModifiers *modifiers);
	SInt16					mInControlInitialization;
	
	static AUCarbonViewControl*	mLastControl;
};

	/*! @class AUPropertyControl */
class AUPropertyControl : public CarbonEventHandler {
public:
	/*! @ctor AUPropertyControl */
	AUPropertyControl (AUCarbonViewBase * 		inBase) : mControl(0), mView (inBase), mHeight(0) {}
	
	/*! @method HandleEvent */
	virtual bool	HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event);

	/*! @method HandlePropertyChange */
	virtual bool	HandlePropertyChange (const AudioUnitProperty &inProp) = 0;
	
	/*! @method AddInterest */
	virtual void	AddInterest (AUEventListenerRef		inListener,
								void *					inObject) = 0;
	
	/*! @method RemoveInterest */
	virtual void	RemoveInterest  (AUEventListenerRef	inListener,
								void *					inObject) = 0;
	
	/*! @method GetHeight */
	int			GetHeight()	{ return mHeight;}
	
protected:
	/*! @method HandleControlChange */
	virtual void	HandleControlChange () = 0;	

	/*! @method RegisterEvents */
			void	RegisterEvents ();

	/*! @method EmbedControl */
			void	EmbedControl (ControlRef theControl);
			
	/*! @method GetCarbonWindow */
	WindowRef 	GetCarbonWindow();

	/*! @var mControl */
	ControlRef			mControl;
	/*! @var mView */
	AUCarbonViewBase*	mView;
	/*! @var mHeight */
	SInt16				mHeight;
};

	/*! @class AUVPresets */
class AUVPresets : public AUPropertyControl {
public:
					/*! @ctor HandleControlChange */
					AUVPresets (AUCarbonViewBase * 		inBase, 
								CFArrayRef& 			inPresets,
								Point 					inLocation, 
								int 					nameWidth, 
								int 					controlWidth, 
								ControlFontStyleRec & 	inFontStyle);

					virtual ~AUVPresets () { CFRelease (mPresets); }

	/*! @method HandlePropertyChange */
	virtual bool	HandlePropertyChange (const AudioUnitProperty &inProp);
	
	/*! @method AddInterest */
	virtual void	AddInterest (AUEventListenerRef		inListener,
								void *					inObject);
	
	/*! @method RemoveInterest */
	virtual void	RemoveInterest  (AUEventListenerRef	inListener,
								void *					inObject);

protected:
	/*! @method HandleControlChange */
	virtual void	HandleControlChange ();	
			
	/*! @var mPresets */
	CFArrayRef			mPresets;
	/*! @var mView */
	AUCarbonViewBase*	mView;
	AudioUnitPropertyID mPropertyID;

	void	HandlePropertyChange(AUPreset &preset);
};

#endif // __AUCarbonViewControl_h__
