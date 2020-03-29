/*
     File: AUCarbonViewControl.cpp
 Abstract: AUCarbonViewControl.h
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
#include "AUCarbonViewControl.h"
#include "AUCarbonViewBase.h"
#include "AUViewLocalizedStringKeys.h"

AUCarbonViewControl::AUCarbonViewControl(AUCarbonViewBase *ownerView, AUParameterListenerRef listener, ControlType type, const CAAUParameter &param, ControlRef control) :
	mOwnerView(ownerView),
	mListener(listener),
	mType(type),
	mParam(param),
	mControl(control),
	mInControlInitialization(0)
{
#if !__LP64__
	SetControlReference(control, SRefCon(this));
#endif
}

AUCarbonViewControl::~AUCarbonViewControl()
{
	AUListenerRemoveParameter(mListener, this, &mParam);
}

AUCarbonViewControl* AUCarbonViewControl::mLastControl = NULL;

void	AUCarbonViewControl::Bind()
{
#if !__LP64__
	mInControlInitialization = 1;   // true
	AUListenerAddParameter(mListener, this, &mParam);
		// will cause an almost-immediate callback
	
	EventTypeSpec events[] = {
		{ kEventClassControl, kEventControlValueFieldChanged }	// N.B. OS X only
	};
	
	WantEventTypes(GetControlEventTarget(mControl), GetEventTypeCount(events), events);

	if (mType == kTypeContinuous || mType == kTypeText || mType == kTypeDiscrete) {
		EventTypeSpec events[] = {
			{ kEventClassControl, kEventControlHit },
			{ kEventClassControl, kEventControlClick },
		    { kEventClassControl, kEventControlTrack }
		};
		WantEventTypes(GetControlEventTarget(mControl), GetEventTypeCount(events), events);
	} 

	if (mType == kTypeText) {
		EventTypeSpec events[] = {
			{ kEventClassControl, kEventControlSetFocusPart }
		};
		WantEventTypes(GetControlEventTarget(mControl), GetEventTypeCount(events), events); 
		ControlKeyFilterUPP proc = mParam.ValuesHaveStrings() ? StdKeyFilterCallback : NumericKeyFilterCallback;
			// this will fail for a static text field
		SetControlData(mControl, 0, kControlEditTextKeyFilterTag, sizeof(proc), &proc);
	}
	
	Update(true);
	mInControlInitialization = 0;   // false
#endif
}

void	AUCarbonViewControl::ParameterToControl(Float32 paramValue)
{
#if !__LP64__
	++mInControlInitialization;
	switch (mType) {
	case kTypeContinuous:
		SetValueFract(AUParameterValueToLinear(paramValue, &mParam));
		break;
	case kTypeDiscrete:
		{
			long value = long(paramValue);
			
			// special case [1] -- menu parameters
			if (mParam.HasNamedParams()) {
				// if we're dealing with menus they behave differently!
				// becaue setting min and max doesn't work correctly for the control value
				// first menu item always reports a control value of 1
				ControlKind ctrlKind;
				if (GetControlKind(mControl, &ctrlKind) == noErr) {
					if ((ctrlKind.kind == kControlKindPopupArrow) 
						|| (ctrlKind.kind == kControlKindPopupButton))				
					{
						value = value - long(mParam.ParamInfo().minValue) + 1;
					}
				}
			}
			
			// special case [2] -- Write-only boolean parameters
			AudioUnitParameterInfo AUPI = mParam.ParamInfo();
			
			bool isWriteOnlyBoolParameter = (	(AUPI.unit == kAudioUnitParameterUnit_Boolean) &&
												(AUPI.flags & kAudioUnitParameterFlag_IsWritable) &&
												!(AUPI.flags & kAudioUnitParameterFlag_IsReadable)	);
			if (!isWriteOnlyBoolParameter) {
				SetValue (value);
			}
		}
		break;
	case kTypeText:
		{
			CFStringRef cfstr = mParam.GetStringFromValueCopy(&paramValue);

			if ( !(mParam.ParamInfo().flags & kAudioUnitParameterFlag_IsWritable)			//READ ONLY PARAMS
					&& (mParam.ParamInfo().flags & kAudioUnitParameterFlag_IsReadable)) 
			{
				if (mParam.GetParamTag()) {
					CFMutableStringRef str = CFStringCreateMutableCopy(NULL, 256, cfstr);
					CFRelease (cfstr);
					CFStringAppend (str, CFSTR(" "));
					CFStringAppend (str, mParam.GetParamTag());
					cfstr = str;
				}
			}
			SetTextValue(cfstr);
			CFRelease (cfstr);
		}
		break;
	}
	--mInControlInitialization;
#endif
}

void	AUCarbonViewControl::ControlToParameter()
{
#if !__LP64__
	if (mInControlInitialization)
		return;

	switch (mType) {
	case kTypeContinuous:
		{
			double controlValue = GetValueFract();
			Float32 paramValue = AUParameterValueFromLinear(controlValue, &mParam);
			mParam.SetValue(mListener, this, paramValue);
		}
		break;
	case kTypeDiscrete:
		{
			long value = GetValue();
			
			// special case [1] -- Menus
			if (mParam.HasNamedParams()) {
				// if we're dealing with menus they behave differently!
				// becaue setting min and max doesn't work correctly for the control value
				// first menu item always reports a control value of 1
				ControlKind ctrlKind;
				if (GetControlKind(mControl, &ctrlKind) == noErr) {
					if ((ctrlKind.kind == kControlKindPopupArrow) 
						|| (ctrlKind.kind == kControlKindPopupButton))				
					{
						value = value + long(mParam.ParamInfo().minValue) - 1;
					}
				}
			}
			
			// special case [2] -- Write-only boolean parameters
			AudioUnitParameterInfo AUPI = mParam.ParamInfo();
			
			bool isWriteOnlyBoolParameter = (	(AUPI.unit == kAudioUnitParameterUnit_Boolean) &&
												(AUPI.flags & kAudioUnitParameterFlag_IsWritable) &&
												!(AUPI.flags & kAudioUnitParameterFlag_IsReadable)	);
			if (isWriteOnlyBoolParameter) {
				value = 1;
			}
			
			mParam.SetValue (mListener, this, value);
		}
		break;
	case kTypeText:
		{
			Float32 val = mParam.GetValueFromString (GetTextValue());
			mParam.SetValue(mListener, this, (mParam.IsIndexedParam() ? (int)val : val));
			if (mParam.ValuesHaveStrings())
				ParameterToControl(val); //make sure we display the correct text (from the AU)
		}
		break;
	}
#endif
}

void	AUCarbonViewControl::SetValueFract(double value)
{
#if !__LP64__
	SInt32 minimum = GetControl32BitMinimum(mControl);
	SInt32 maximum = GetControl32BitMaximum(mControl);
	SInt32 cval = SInt32(value * (maximum - minimum) + minimum + 0.5);
	SetControl32BitValue(mControl, cval);
//	printf("set: value=%lf, min=%ld, max=%ld, ctl value=%ld\n", value, minimum, maximum, cval);
#endif
}

double	AUCarbonViewControl::GetValueFract()
{
#if !__LP64__
	SInt32 minimum = GetControl32BitMinimum(mControl);
	SInt32 maximum = GetControl32BitMaximum(mControl);
	SInt32 cval = GetControl32BitValue(mControl);
	double result = double(cval - minimum) / double(maximum - minimum);
//	printf("get: min=%ld, max=%ld, value=%ld, result=%f\n", minimum, maximum, cval, result);
	return result;
#else
	return 0;
#endif
}

void	AUCarbonViewControl::SetTextValue(CFStringRef cfstr)
{
#if !__LP64__
	verify_noerr(SetControlData(mControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr));
#endif
}

CFStringRef	AUCarbonViewControl::GetTextValue()
{
#if !__LP64__
	CFStringRef cfstr;
	verify_noerr(GetControlData(mControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &cfstr, NULL));
	return cfstr;
#else
	return CFSTR("");
#endif
}

void	AUCarbonViewControl::SetValue(long value)
{
#if !__LP64__
	SetControl32BitValue(mControl, value);
#endif
}

long	AUCarbonViewControl::GetValue()
{
#if !__LP64__
	return GetControl32BitValue(mControl);
#else
	return 0;
#endif
}

/* Notes on event handling 
	
	Button (Click and release on button)
		kEventControlClick received
		kEventControlTrack received
		kEventControlValueFieldChanged received
		kEventControlHit received
	
	Button (Click and release outside of button bounds)
		kEventControlClick received
		kEventControlTrack received
	
	Slider (Click, drag, and release)
		kEventControlClick received
		kEventControlTrack received
		kEventControlValueFieldChanged received
		kEventControlValueFieldChanged received
		kEventControlHit received
		
	Slider (Click, release without changing value)
		kEventControlClick received
		kEventControlTrack received
*/
bool	AUCarbonViewControl::HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event)
{
	UInt32 eclass = GetEventClass(event);
	UInt32 ekind = GetEventKind(event);
	ControlRef control;
	bool		handled = true;
	
	switch (eclass) {
		case kEventClassControl:
		{
			AudioUnitParameterInfo AUPI = mParam.ParamInfo();
			
			bool isWriteOnlyBoolParameter = (	(AUPI.unit == kAudioUnitParameterUnit_Boolean) &&
												(AUPI.flags & kAudioUnitParameterFlag_IsWritable) &&
												!(AUPI.flags & kAudioUnitParameterFlag_IsReadable)	);
			
			switch (ekind) {
				case kEventControlSetFocusPart:	// tab
					handled = !handled;		// fall through to next case
					mLastControl = this;
				case kEventControlValueFieldChanged:
					GetEventParameter(event, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &control);
					verify(control == mControl);
					ControlToParameter();
					return handled;			
				case kEventControlClick:
					if (isWriteOnlyBoolParameter) {
						GetEventParameter(event, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &control);
						verify(control == mControl);
						ControlToParameter();
					} else if (mLastControl != this) {
						if (mLastControl != NULL) {
							mLastControl->Update(false);
						}
						mLastControl = this;	
					}
					mOwnerView->TellListener(mParam, kAudioUnitCarbonViewEvent_MouseDownInControl, NULL);
					break;	// don't return true, continue normal processing
				case kEventControlHit:
					if (mLastControl != this) {
						if (mLastControl != NULL)
							mLastControl->Update(false);
						mLastControl = this;	
					} 
					mOwnerView->TellListener(mParam, kAudioUnitCarbonViewEvent_MouseUpInControl, NULL);
					break;	// don't return true, continue normal processing
				case kEventControlTrack:		
					if (mLastControl != this) {
						if (mLastControl != NULL)
							mLastControl->Update(false);
						mLastControl = this;	
					}
					
					CallNextEventHandler(inHandlerRef, event);
					ControlToParameter();						// new code
					mOwnerView->TellListener(mParam, kAudioUnitCarbonViewEvent_MouseUpInControl, NULL);
					// old code:
					//		break;	// don't return true, continue normal processing

					return handled;	// don't return true, continue normal processing
			}
		}
	}
	return !handled;
}

pascal void	AUCarbonViewControl::SliderTrackProc(ControlRef theControl, ControlPartCode partCode)
{
	// this doesn't need to actually do anything
//	AUCarbonViewControl *This = (AUCarbonViewControl *)GetControlReference(theControl);
}

pascal ControlKeyFilterResult	AUCarbonViewControl::StdKeyFilterCallback(ControlRef theControl, 
												SInt16 *keyCode, SInt16 *charCode, 
												EventModifiers *modifiers)
{
#if !__LP64__
	SInt16 c = *charCode;
	if (c >= ' ' || c == '\b' || c == 0x7F || (c >= 0x1c && c <= 0x1f) || c == '\t')
		return kControlKeyFilterPassKey;
	if (c == '\r' || c == 3) {	// return or Enter
		AUCarbonViewControl *This = (AUCarbonViewControl *)GetControlReference(theControl);
		ControlEditTextSelectionRec sel = { 0, 32767 };
		SetControlData(This->mControl, 0, kControlEditTextSelectionTag, sizeof(sel), &sel);
		This->ControlToParameter();
	}
#endif
	return kControlKeyFilterBlockKey;
}

pascal ControlKeyFilterResult	AUCarbonViewControl::NumericKeyFilterCallback(ControlRef theControl, 
												SInt16 *keyCode, SInt16 *charCode, 
												EventModifiers *modifiers)
{
#if !__LP64__
	SInt16 c = *charCode;
	if (isdigit(c) || c == '+' || c == '-' || c == '.' || c == '\b' || c == 0x7F || (c >= 0x1c && c <= 0x1f)
	|| c == '\t')
		return kControlKeyFilterPassKey;
	if (c == '\r' || c == 3) {	// return or Enter
		AUCarbonViewControl *This = (AUCarbonViewControl *)GetControlReference(theControl);
		ControlEditTextSelectionRec sel = { 0, 32767 };
		SetControlData(This->mControl, 0, kControlEditTextSelectionTag, sizeof(sel), &sel);
		This->ControlToParameter();
	}
#endif
	return kControlKeyFilterBlockKey;
}

Boolean	AUCarbonViewControl::SizeControlToFit(ControlRef inControl, SInt16 *outWidth, SInt16 *outHeight)
{
#if !__LP64__
	if (inControl == 0) return false;
	
	Boolean bValue = false;
	// this only works on text controls -- returns an error for other controls, but doesn't do anything,
	// so the error is irrelevant
	SetControlData(inControl, kControlEntireControl, 'stim' /* kControlStaticTextIsMultilineTag */, sizeof(Boolean), &bValue);
	
	SInt16 baseLineOffset;
	Rect bestRect;
	OSErr err = GetBestControlRect(inControl, &bestRect, &baseLineOffset);  
	if (err != noErr) return false;
	
	int width = (bestRect.right - bestRect.left) + 1;
	int height = (bestRect.bottom - bestRect.top) + 1;
	
	Rect boundsRect;
	GetControlBounds (inControl, &boundsRect);
	
	Rect newRect;
	newRect.top = boundsRect.top;
	newRect.bottom = newRect.top + height;
	newRect.left = boundsRect.left;
	newRect.right = newRect.left + width;
	
	SetControlBounds (inControl, &newRect);
	
	if (outWidth)
		*outWidth = width;
	
	if (outHeight)
		*outHeight = height;
#endif	
	return true;
}

#pragma mark ___AUPropertyControl
bool	AUPropertyControl::HandleEvent(EventHandlerCallRef inHandlerRef, EventRef event)
{	
	UInt32 eclass = GetEventClass(event);
	UInt32 ekind = GetEventKind(event);
	switch (eclass) {
	case kEventClassControl:
		switch (ekind) {
		case kEventControlValueFieldChanged:
			HandleControlChange();
			return true;	// handled
		}
	}

	return false;
}

void	AUPropertyControl::RegisterEvents ()
{
#if !__LP64__
	EventTypeSpec events[] = {
		{ kEventClassControl, kEventControlValueFieldChanged }	// N.B. OS X only
	};
	
	WantEventTypes(GetControlEventTarget(mControl), GetEventTypeCount(events), events);
#endif
}

void	AUPropertyControl::EmbedControl (ControlRef theControl) 
{ 
	mView->EmbedControl (theControl); 
}

WindowRef 	AUPropertyControl::GetCarbonWindow()
{
	return mView->GetCarbonWindow();
}

#pragma mark ___AUVPreset
#if !__LP64__
static CFStringRef kStringFactoryPreset = kAUViewLocalizedStringKey_FactoryPreset;
static bool sAUVPresetLocalized = false;
#endif

AUVPresets::AUVPresets (AUCarbonViewBase* 		inParentView, 
						CFArrayRef& 			inPresets,
						Point 					inLocation, 
						int 					nameWidth, 
						int 					controlWidth, 
						ControlFontStyleRec & 	inFontStyle)
	: AUPropertyControl (inParentView),
	  mPresets (inPresets),
	  mView (inParentView)
{
#if !__LP64__
	Rect r;
	
	// ok we now have an array of factory presets
	// get their strings and display them

	r.top = inLocation.v;		r.bottom = r.top;
	r.left = inLocation.h;		r.right = r.left;
	
    // localize as necessary
    if (!sAUVPresetLocalized) {
        CFBundleRef mainBundle = CFBundleGetBundleWithIdentifier(kLocalizedStringBundle_AUView);
        if (mainBundle) {
            kStringFactoryPreset =	CFCopyLocalizedStringFromTableInBundle(
                                        kAUViewLocalizedStringKey_FactoryPreset, kLocalizedStringTable_AUView,
                                        mainBundle, CFSTR("FactoryPreset title string"));
            sAUVPresetLocalized = true;
        }
    }
    
    // create localized title string
    CFMutableStringRef factoryPresetsTitle = CFStringCreateMutable(NULL, 0);
    CFStringAppend(factoryPresetsTitle, kStringFactoryPreset);
    CFStringAppend(factoryPresetsTitle, kAUViewUnlocalizedString_TitleSeparator);
    
	ControlRef theControl;
    verify_noerr(CreateStaticTextControl(mView->GetCarbonWindow(), &r, factoryPresetsTitle, &inFontStyle, &theControl));
	SInt16 width = 0;
	AUCarbonViewControl::SizeControlToFit(theControl, &width, &mHeight);
    CFRelease(factoryPresetsTitle);
	EmbedControl(theControl);
	
	r.top -= 2;
	r.left += width + 10;
	r.right = r.left;
	r.bottom = r.top;
	
	verify_noerr(CreatePopupButtonControl (	mView->GetCarbonWindow(), &r, NULL, 
											-12345,	// DON'T GET MENU FROM RESOURCE mMenuID,!!!
											FALSE,	// variableWidth, 
											0,		// titleWidth, 
											0,		// titleJustification, 
											0,		// titleStyle, 
											&mControl));
	
	MenuRef menuRef;
	verify_noerr(CreateNewMenu(1, 0, &menuRef));
	
	int numPresets = CFArrayGetCount(mPresets);
	
	for (int i = 0; i < numPresets; ++i)
	{
		AUPreset* preset = (AUPreset*) CFArrayGetValueAtIndex (mPresets, i);
		verify_noerr(AppendMenuItemTextWithCFString (menuRef, preset->presetName, 0, 0, 0));
	}
	
	verify_noerr(SetControlData(mControl, 0, kControlPopupButtonMenuRefTag, sizeof(menuRef), &menuRef));
	verify_noerr (SetControlFontStyle (mControl, &inFontStyle));
	
	SetControl32BitMaximum (mControl, numPresets);
	
	// size popup
	SInt16 height = 0;
	
	AUCarbonViewControl::SizeControlToFit(mControl, &width, &height);
	
	if (height > mHeight) mHeight = height;
	if (mHeight < 0) mHeight = 0;
	
	// find which menu item is the Default preset
	UInt32 propertySize = sizeof(AUPreset);
	AUPreset defaultPreset;
	OSStatus result = AudioUnitGetProperty (mView->GetEditAudioUnit(), 
									kAudioUnitProperty_PresentPreset,
									kAudioUnitScope_Global, 
									0, 
									&defaultPreset, 
									&propertySize);
	
	mPropertyID = kAudioUnitProperty_PresentPreset;
#endif	
#ifndef __LP64__
	if (result != noErr) {	// if the PresentPreset property is not implemented, fall back to the CurrentPreset property
		OSStatus result = AudioUnitGetProperty (mView->GetEditAudioUnit(), 
									kAudioUnitProperty_CurrentPreset,
									kAudioUnitScope_Global, 
									0, 
									&defaultPreset, 
									&propertySize);
		mPropertyID = kAudioUnitProperty_CurrentPreset;
		if (result == noErr)
			CFRetain (defaultPreset.presetName);
	} 
#endif
#if !__LP64__		
	EmbedControl (mControl);
	
	HandlePropertyChange(defaultPreset);
	
	RegisterEvents();
#endif
}

void	AUVPresets::AddInterest (AUEventListenerRef		inListener,
											void *		inObject)
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mView->GetEditAudioUnit();
	e.mArgument.mProperty.mPropertyID = mPropertyID;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;
	
	AUEventListenerAddEventType(inListener, inObject, &e);
}

void	AUVPresets::RemoveInterest (AUEventListenerRef	inListener,
											void *		inObject)
{
	AudioUnitEvent e;
	e.mEventType = kAudioUnitEvent_PropertyChange;
	e.mArgument.mProperty.mAudioUnit = mView->GetEditAudioUnit();
	e.mArgument.mProperty.mPropertyID = mPropertyID;
	e.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	e.mArgument.mProperty.mElement = 0;

	AUEventListenerRemoveEventType(inListener, inObject, &e);
}

void	AUVPresets::HandleControlChange ()
{
#if !__LP64__
	SInt32 i = GetControl32BitValue(mControl);
	if (i > 0)
	{
		AUPreset* preset = (AUPreset*) CFArrayGetValueAtIndex (mPresets, i-1);
	
		verify_noerr(AudioUnitSetProperty (mView->GetEditAudioUnit(), 
									mPropertyID,	// either currentPreset or PresentPreset depending on which is supported
									kAudioUnitScope_Global, 
									0, 
									preset, 
									sizeof(AUPreset)));
									
		// when we change a preset we can't expect the AU to update its state
		// as it isn't meant to know that its being viewed!
		// so we broadcast a notification to all listeners that all parameters on this AU have changed
		AudioUnitParameter changedUnit;
		changedUnit.mAudioUnit = mView->GetEditAudioUnit();
		changedUnit.mParameterID = kAUParameterListener_AnyParameter;
		verify_noerr (AUParameterListenerNotify (NULL, NULL, &changedUnit) );
	}
#endif
}

void	AUVPresets::HandlePropertyChange(AUPreset &preset) 
{
#if !__LP64__
	// check to see if the preset is in our menu
	int numPresets = CFArrayGetCount(mPresets);
	if (preset.presetNumber < 0) {	
		SetControl32BitValue (mControl, 0); //controls are one-based
	} else {
		for (SInt32 i = 0; i < numPresets; ++i) {
			AUPreset* currPreset = (AUPreset*) CFArrayGetValueAtIndex (mPresets, i);
			if (preset.presetNumber == currPreset->presetNumber) {
				SetControl32BitValue (mControl, ++i); //controls are one-based
				break;
			}
		}
	}
	
	if (preset.presetName)
		CFRelease (preset.presetName);
#endif
}

bool	AUVPresets::HandlePropertyChange (const AudioUnitProperty &inProp)
{
	if (inProp.mPropertyID == mPropertyID) 
	{
		UInt32 theSize = sizeof(AUPreset);
		AUPreset currentPreset;
		
		OSStatus result = AudioUnitGetProperty(inProp.mAudioUnit, 
												inProp.mPropertyID, 
												inProp.mScope, 
												inProp.mElement, &currentPreset, &theSize);
		
		if (result == noErr) {
#ifndef __LP64__
			if (inProp.mPropertyID == kAudioUnitProperty_CurrentPreset && currentPreset.presetName)
				CFRetain (currentPreset.presetName);
#endif
			HandlePropertyChange(currentPreset);
			return true;
		}
	}
	return false;
}
