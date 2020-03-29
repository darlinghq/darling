/*
     File: AUViewLocalizedStringKeys.h
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
#ifndef __AUViewLocalizedStringKeys_h__
#define __AUViewLocalizedStringKeys_h__

// ACCESS POINT:
#define	kLocalizedStringBundle_AUView 		CFSTR("com.apple.audio.units.Components")
#define	kLocalizedStringTable_AUView 		CFSTR("CustomUI")

// UNLOCALIZED STRINGS:
    #define	kAUViewUnlocalizedString_TitleSeparator  			CFSTR(": ")
    
// Generic View:
    #define	kAUViewLocalizedStringKey_AudioUnit 				CFSTR("Audio Unit")
    #define	kAUViewLocalizedStringKey_Manufacturer 			CFSTR("Manufacturer")
    
    #define	kAUViewLocalizedStringKey_FactoryPreset 			CFSTR("Factory Preset")
    
    #define	kAUViewLocalizedStringKey_Properties 				CFSTR("Properties")
    #define	kAUViewLocalizedStringKey_Parameters 				CFSTR("Parameters")
    
    #define	kAUViewLocalizedStringKey_Standard 				CFSTR("Standard")
    #define	kAUViewLocalizedStringKey_Expert 					CFSTR("Expert")
    
// AULoadCPU:
    #define	kAUViewLocalizedStringKey_RestrictCPULoad 			CFSTR("Restrict CPU Load")
    #define	kAUViewLocalizedStringKey_PercentSymbol 			CFSTR("%")
    #define	kAUViewLocalizedStringKey_NotApplicable 			CFSTR("n/a")
    
// AUDiskStreamingCheckbox:
    #define	kAUViewLocalizedStringKey_StreamFromDisk 			CFSTR("Stream From Disk")
    
// AURenderQualityPopup:
    #define	kAUViewLocalizedStringKey_RenderQuality 			CFSTR("Render Quality")
    #define	kAUViewLocalizedStringKey_Maximum 					CFSTR("Maximum")
    #define	kAUViewLocalizedStringKey_High 					CFSTR("High")
    #define	kAUViewLocalizedStringKey_Medium 					CFSTR("Medium")
    #define	kAUViewLocalizedStringKey_Low 						CFSTR("Low")
    #define	kAUViewLocalizedStringKey_Minimum 					CFSTR("Minimum")

// AUChannelLayoutPopUp:
	#define	kAUViewLocalizedStringKey_AudioChannelLayout 		CFSTR("Audio Channel Layout")
    
#endif //__AUViewLocalizedStringKeys_h__
