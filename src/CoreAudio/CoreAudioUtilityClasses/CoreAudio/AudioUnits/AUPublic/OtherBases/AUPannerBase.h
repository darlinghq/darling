/*
     File: AUPannerBase.h
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
#ifndef __AUPannerBase_h__
#define __AUPannerBase_h__

#include "AUBase.h"
#include <math.h>
#include "CAAutoDisposer.h"
#include "CAAudioChannelLayout.h"


/*! @class AUPannerBase */
class AUPannerBase : public AUBase
{
public:
/*! @ctor AUPannerBase */
    AUPannerBase(AudioComponentInstance inAudioUnit);
/*! @dtor ~AUPannerBase */
    virtual ~AUPannerBase();
    
	/*! @method Initialize */
	virtual OSStatus			Initialize();

	/*! @method Cleanup */
	virtual void				Cleanup();


	/*! @method Reset */
	virtual OSStatus			Reset(		AudioUnitScope 				inScope,
											AudioUnitElement 			inElement);

	/*! @method CanScheduleParameters */
	virtual bool				CanScheduleParameters() const { return false; }

	/*! @method GetParameterInfo */
	virtual OSStatus			GetParameterInfo(	AudioUnitScope			inScope,
													AudioUnitParameterID	inParameterID,
													AudioUnitParameterInfo	&outParameterInfo );


	/*! @method GetPropertyInfo */
	virtual OSStatus			GetPropertyInfo (AudioUnitPropertyID	inID,
											AudioUnitScope				inScope,
											AudioUnitElement			inElement,
											UInt32 &					outDataSize,
											Boolean &					outWritable);

	/*! @method GetProperty */
	virtual OSStatus			GetProperty (AudioUnitPropertyID 		inID,
											AudioUnitScope 				inScope,
											AudioUnitElement	 		inElement,
											void *						outData);

	/*! @method SetProperty */
	virtual OSStatus			SetProperty(AudioUnitPropertyID 		inID,
											AudioUnitScope 				inScope,
											AudioUnitElement 			inElement,
											const void *				inData,
											UInt32 						inDataSize);


	/*! @method StreamFormatWritable */
	virtual bool				StreamFormatWritable (AudioUnitScope	scope,
											AudioUnitElement			element);

	/*! @method ChangeStreamFormat */
	virtual	OSStatus			ChangeStreamFormat (
										AudioUnitScope						inScope,
										AudioUnitElement					inElement,
										const CAStreamBasicDescription & 	inPrevFormat,
										const CAStreamBasicDescription &	inNewFormat);

	
	/*! @method IsBypassEffect */
	// This is used for the property value - to reflect to the UI if an effect is bypassed
	bool						IsBypassEffect () { return mBypassEffect; }
						
	/*! @method SetBypassEffect */
	virtual void				SetBypassEffect (bool inFlag) { mBypassEffect = inFlag; }
	

	/*! @method Render */
	virtual OSStatus 	Render(AudioUnitRenderActionFlags &		ioActionFlags,
										const AudioTimeStamp &			inTimeStamp,
										UInt32							inNumberFrames);

	/*! @method Render */
	virtual OSStatus 	PannerRender(AudioUnitRenderActionFlags &		ioActionFlags,
										const AudioTimeStamp &			inTimeStamp,
										UInt32							inNumberFrames) = 0;

	/*! @method BypassRender */
	virtual OSStatus 	BypassRender(AudioUnitRenderActionFlags &		ioActionFlags,
										const AudioTimeStamp &			inTimeStamp,
										UInt32							inNumberFrames);


	/*! @method GetAudioChannelLayout */
	virtual UInt32				GetAudioChannelLayout(	AudioUnitScope				inScope,
														AudioUnitElement 			inElement,
														AudioChannelLayout *		outLayoutPtr,
														Boolean &					outWritable);

	/*! @method SetAudioChannelLayout */
	virtual OSStatus			SetAudioChannelLayout(	AudioUnitScope 				inScope, 
														AudioUnitElement 			inElement,
														const AudioChannelLayout *	inLayout);

	/*! @method RemoveAudioChannelLayout */
	virtual OSStatus			RemoveAudioChannelLayout(	AudioUnitScope			inScope, 
															AudioUnitElement		inElement);

	/*! @method GetChannelLayoutTags */
	virtual UInt32				GetChannelLayoutTags(		AudioUnitScope				inScope,
															AudioUnitElement 			inElement, 
															AudioChannelLayoutTag*		outTags);

	/*! @method GetNumberOfInputChannels */
	UInt32						GetNumberOfInputChannels() { return ((AUIOElement*)Inputs().SafeGetElement(0))->NumberChannels(); };
	/*! @method GetNumberOfOutputChannels */
	UInt32						GetNumberOfOutputChannels() { return ((AUIOElement*)Outputs().SafeGetElement(0))->NumberChannels(); }

	/*! @method GetParameter */
	virtual OSStatus 	GetParameter(			AudioUnitParameterID			inParamID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														Float32 &						outValue);
												
	/*! @method SetParameter */
	virtual OSStatus 	SetParameter(			AudioUnitParameterID			inParamID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														Float32							inValue,
														UInt32							inBufferOffsetInFrames);

	// convenience wrappers for accessing parameters in the global scope
	/*! @method SetParameter */
	void						SetParameter(			UInt32							inParamID,
														Float32							inValue)
								{
									OSStatus err = SetParameter(inParamID, kAudioUnitScope_Global, 0, inValue, 0);
									if (err) throw err;
								}
								
	/*! @method GetParameter */
	Float32						GetParameter(			UInt32							inParamID )
								{
									Float32 outValue = 0.;
									OSStatus err = GetParameter(inParamID, kAudioUnitScope_Global, 0, outValue);
									if (err) throw err;
									return outValue;
								}

	/*! @method InputChannelConfigIsSupported */
	bool						InputChannelConfigIsSupported(UInt32 inNumberChannels);
	/*! @method OutputChannelConfigIsSupported */
	bool						OutputChannelConfigIsSupported(UInt32 inNumberChannels);
	/*! @method ChannelConfigIsSupported */
	bool						ChannelConfigIsSupported();

	/*! @method SupportsTail */
	virtual	bool				SupportsTail () { return true; }
	/*! @method GetTailTime */
    virtual Float64				GetTailTime() { return 0; }


	/*! @method GetGain */
	Float32						GetGain()        { return GetParameter(kPannerParam_Gain); }
	/*! @method GetTailTime */
	Float32						GetAzimuth()     { return GetParameter(kPannerParam_Azimuth); }
	/*! @method GetElevation */
	Float32						GetElevation()   { return GetParameter(kPannerParam_Elevation); }
	/*! @method GetDistance */
	Float32						GetDistance()    { return GetParameter(kPannerParam_Distance); }
	/*! @method GetCoordScale */
	Float32						GetCoordScale()  { return GetParameter(kPannerParam_CoordScale); }
	/*! @method GetRefDistance */
	Float32						GetRefDistance() { return GetParameter(kPannerParam_RefDistance); }	                        	
	                        	
	/*! @method SetGain */
	void						SetGain(Float32 inValue)        { SetParameter(kPannerParam_Gain, inValue); }
	/*! @method SetAzimuth */
	void						SetAzimuth(Float32 inValue)     { SetParameter(kPannerParam_Azimuth, inValue); }
	/*! @method SetElevation */
	void						SetElevation(Float32 inValue)   { SetParameter(kPannerParam_Elevation, inValue); }
	/*! @method SetDistance */
	void						SetDistance(Float32 inValue)    { SetParameter(kPannerParam_Distance, inValue); }
	/*! @method SetCoordScale */
	void						SetCoordScale(Float32 inValue)  { SetParameter(kPannerParam_CoordScale, inValue); }
	/*! @method SetRefDistance */
	void						SetRefDistance(Float32 inValue) { SetParameter(kPannerParam_RefDistance, inValue); }
	
protected:
	/*! @method ShouldBypassEffect */
	// This is used in the render call to see if an effect is bypassed
	// It can return a different status than IsBypassEffect (though it MUST take that into account)
	virtual	bool				ShouldBypassEffect () { return IsBypassEffect(); }

	/*! @method AllocBypassMatrix */
	void						AllocBypassMatrix();

	/*! @method UpdateBypassMatrix */
	OSStatus					UpdateBypassMatrix();

	/*! @method SetDefaultChannelLayoutsIfNone */
	OSStatus					SetDefaultChannelLayoutsIfNone();
	
	/*! @method ChannelLayoutTagIsSupported */
	bool						ChannelLayoutTagIsSupported(	AudioUnitScope			inScope, 
																AudioUnitElement		inElement, 
																AudioChannelLayoutTag	inTag);
	
	const AudioChannelLayout&	GetInputLayout() const { return mInputLayout.Layout(); }
	const AudioChannelLayout&	GetOutputLayout() const { return mOutputLayout.Layout(); }
	
private:
	
	/*! @var UpdateBypassMatrix */
	bool mBypassEffect;
	/*! @var mBypassMatrix */
	CAAutoFree<Float32> mBypassMatrix;
	/*! @var mInputLayout */
	CAAudioChannelLayout mInputLayout;
	/*! @var mOutputLayout */
	CAAudioChannelLayout mOutputLayout;
};

#endif /* __AUPannerBase_h__ */
