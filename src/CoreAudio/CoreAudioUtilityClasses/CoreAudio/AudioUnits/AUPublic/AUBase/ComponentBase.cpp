/*
     File: ComponentBase.cpp
 Abstract: ComponentBase.h
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
#include "ComponentBase.h"
#include "CAXException.h"

#if TARGET_OS_MAC
pthread_mutex_t ComponentInitLocker::sComponentOpenMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_once_t ComponentInitLocker::sOnce = PTHREAD_ONCE_INIT;

void ComponentInitLocker::InitComponentInitLocker()
{
	// have to do this because OS X lacks PTHREAD_MUTEX_RECURSIVE_INITIALIZER_NP
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&sComponentOpenMutex, &attr);
	pthread_mutexattr_destroy(&attr);
}

#elif TARGET_OS_WIN32
CAGuard	ComponentInitLocker::sComponentOpenGuard("sComponentOpenGuard");
#endif

ComponentBase::EInstanceType ComponentBase::sNewInstanceType;

static OSStatus CB_GetComponentDescription (const AudioComponentInstance inInstance, AudioComponentDescription * outDesc);
#if !CA_USE_AUDIO_PLUGIN_ONLY && !TARGET_OS_WIN32
	static OSStatus CMgr_GetComponentDescription (const AudioComponentInstance inInstance, AudioComponentDescription * outDesc);
#endif

ComponentBase::ComponentBase(AudioComponentInstance inInstance) 
	: mComponentInstance(inInstance), 
	  mInstanceType(sNewInstanceType) 
{ 
	GetComponentDescription(); 
}

ComponentBase::~ComponentBase()
{
}

void			ComponentBase::PostConstructor()
{
}

void			ComponentBase::PreDestructor()
{
}

#define ACPI	((AudioComponentPlugInInstance *)self)
#define ACImp	((ComponentBase *)&ACPI->mInstanceStorage)

OSStatus ComponentBase::AP_Open(void *self, AudioUnit compInstance)
{
	OSStatus result = noErr;
	try {
		ComponentInitLocker lock;
		
		ComponentBase::sNewInstanceType = ComponentBase::kAudioComponentInstance;
		ComponentBase *cb = (ComponentBase *)(*ACPI->mConstruct)(&ACPI->mInstanceStorage, compInstance);
		cb->PostConstructor();	// allows base class to do additional initialization
		// once the derived class is fully constructed
		result = noErr;
	}
	COMPONENT_CATCH
	if (result)
		delete ACPI;
	return result;
}

OSStatus ComponentBase::AP_Close(void *self)
{
	OSStatus result = noErr;
	try {
		if (ACImp) {
			ACImp->PreDestructor();
			(*ACPI->mDestruct)(&ACPI->mInstanceStorage);
			free(self);
		}
	}
	COMPONENT_CATCH
	return result;
}

#if !CA_USE_AUDIO_PLUGIN_ONLY
OSStatus		ComponentBase::Version()
{
	return 0x00000001;
}

OSStatus		ComponentBase::ComponentEntryDispatch(ComponentParameters *p, ComponentBase *This)
{
	if (This == NULL) return kAudio_ParamError;

	OSStatus result = noErr;
	
	switch (p->what) {
	case kComponentCloseSelect:
		This->PreDestructor();
		delete This;
		break;
	
	case kComponentVersionSelect:
		result = This->Version();
		break;

	case kComponentCanDoSelect:
		switch (GetSelectorForCanDo(p)) {
		case kComponentOpenSelect:
		case kComponentCloseSelect:
		case kComponentVersionSelect:
		case kComponentCanDoSelect:
			return 1;
		default:
			return 0;
		}
		
	default:
		result = badComponentSelector;
		break;
	}
	return result;
}

SInt16		ComponentBase::GetSelectorForCanDo(ComponentParameters *params)
{
	if (params->what != kComponentCanDoSelect) return 0;
	
	#if TARGET_CPU_X86
		SInt16 sel = params->params[0];
	#elif TARGET_CPU_X86_64
		SInt16 sel = params->params[1];
	#elif TARGET_CPU_PPC
		SInt16 sel = (params->params[0] >> 16);
	#else
		SInt16 sel = params->params[0];
	#endif
	
	return sel;
/*		
		printf ("flags:%d, paramSize: %d, what: %d\n\t", params->flags, params->paramSize, params->what);
		for (int i = 0; i < params->paramSize; ++i) {
			printf ("[%d]:%d(0x%x), ", i, params->params[i], params->params[i]);
		}
		printf("\n\tsel:%d\n", sel);
*/
}

#endif

#if CA_DO_NOT_USE_AUDIO_COMPONENT 
static OSStatus ComponentBase_GetComponentDescription (const AudioComponentInstance & inInstance, AudioComponentDescription &outDesc);
#endif

AudioComponentDescription ComponentBase::GetComponentDescription() const
{
	AudioComponentDescription desc;
	OSStatus result = 1;
	
	if (IsPluginObject()) {
		ca_require_noerr(result = CB_GetComponentDescription (mComponentInstance, &desc), home);
	}
#if !CA_USE_AUDIO_PLUGIN_ONLY
	else {
		ca_require_noerr(result = CMgr_GetComponentDescription (mComponentInstance, &desc), home);	
	}
#endif

home:
	if (result)
		memset (&desc, 0, sizeof(AudioComponentDescription));
	
	return desc;
}

#if CA_USE_AUDIO_PLUGIN_ONLY
// everything we need is there and we should be linking against it
static OSStatus CB_GetComponentDescription (const AudioComponentInstance inInstance, AudioComponentDescription * outDesc)
{
	AudioComponent comp = AudioComponentInstanceGetComponent(inInstance);
	if (comp)
		return AudioComponentGetDescription(comp, outDesc);

	return kAudio_ParamError;
}

#elif !TARGET_OS_WIN32
// these are the direct dependencies on ComponentMgr calls that an AU
// that is a component mgr is dependent on

// these are dynamically loaded so that these calls will work on Leopard
#include <dlfcn.h>

static OSStatus CB_GetComponentDescription (const AudioComponentInstance inInstance, AudioComponentDescription * outDesc)
{
	typedef AudioComponent (*AudioComponentInstanceGetComponentProc) (AudioComponentInstance);
	static AudioComponentInstanceGetComponentProc aciGCProc = NULL;
	
	typedef OSStatus (*AudioComponentGetDescriptionProc)(AudioComponent, AudioComponentDescription *);
	static AudioComponentGetDescriptionProc acGDProc = NULL;
	
	static int doneInit = 0;
	if (doneInit == 0) {
		doneInit = 1;	
		void* theImage = dlopen("/System/Library/Frameworks/AudioUnit.framework/AudioUnit", RTLD_LAZY);
		if (theImage != NULL)
		{
			aciGCProc = (AudioComponentInstanceGetComponentProc)dlsym (theImage, "AudioComponentInstanceGetComponent");
			if (aciGCProc) {
				acGDProc = (AudioComponentGetDescriptionProc)dlsym (theImage, "AudioComponentGetDescription");
			}
		}
	}
	
	OSStatus result = kAudio_UnimplementedError;
	if (acGDProc && aciGCProc) {
		AudioComponent comp = (*aciGCProc)(inInstance);
		if (comp)
			result = (*acGDProc)(comp, outDesc);
	} 
#if !CA_USE_AUDIO_PLUGIN_ONLY
	else {
		result = CMgr_GetComponentDescription (inInstance, outDesc);
	}
#endif

	return result;
}

#if !CA_USE_AUDIO_PLUGIN_ONLY
// these are the direct dependencies on ComponentMgr calls that an AU
// that is a component mgr is dependent on

// these are dynamically loaded

#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include "CAXException.h"
#include "ComponentBase.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Component Manager
// Used for fast dispatch with audio units
typedef Handle (*GetComponentInstanceStorageProc)(ComponentInstance aComponentInstance);
static GetComponentInstanceStorageProc sGetComponentInstanceStorageProc = NULL;

typedef OSErr (*GetComponentInfoProc)(Component, ComponentDescription *, void*, void*, void*);
static GetComponentInfoProc sGetComponentInfoProc = NULL;

typedef void (*SetComponentInstanceStorageProc)(ComponentInstance, Handle);
static SetComponentInstanceStorageProc sSetComponentInstanceStorageProc = NULL;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static void CSInitOnce(void* /*unused*/)
{
	void *theImage = dlopen("/System/Library/Frameworks/CoreServices.framework/CoreServices", RTLD_LAZY);
	if (!theImage) return;

	sGetComponentInstanceStorageProc = (GetComponentInstanceStorageProc) dlsym(theImage, "GetComponentInstanceStorage");
	sGetComponentInfoProc = (GetComponentInfoProc)dlsym (theImage, "GetComponentInfo");
	sSetComponentInstanceStorageProc = (SetComponentInstanceStorageProc) dlsym(theImage, "SetComponentInstanceStorage");
}

#if TARGET_OS_MAC

#include <dispatch/dispatch.h>

static dispatch_once_t sCSInitOnce = 0;

static void CSInit ()
{
	dispatch_once_f(&sCSInitOnce, NULL, CSInitOnce);
}

#else 

static void CSInit ()
{
	static int sDoCSLoad = 1;
	if (sDoCSLoad) {
		sDoCSLoad = 0;
		CSInitOnce(NULL);
	}
}

#endif

OSStatus CMgr_GetComponentDescription (const AudioComponentInstance inInstance, AudioComponentDescription * outDesc)
{
	CSInit();
	if (sGetComponentInfoProc)
		return (*sGetComponentInfoProc)((Component)inInstance, (ComponentDescription*)outDesc, NULL, NULL, NULL);
	return kAudio_UnimplementedError;
}

Handle CMgr_GetComponentInstanceStorage(ComponentInstance aComponentInstance)
{
	CSInit();
	if (sGetComponentInstanceStorageProc)
		return (*sGetComponentInstanceStorageProc)(aComponentInstance);
	return NULL;
}

void CMgr_SetComponentInstanceStorage(ComponentInstance aComponentInstance, Handle theStorage)
{
	CSInit();
	if (sSetComponentInstanceStorageProc)
		(*sSetComponentInstanceStorageProc)(aComponentInstance, theStorage);
}
#endif // !CA_USE_AUDIO_PLUGIN_ONLY

#else
//#include "ComponentManagerDependenciesWin.h"
// everything we need is there and we should be linking against it
static OSStatus CB_GetComponentDescription (const AudioComponentInstance inInstance, AudioComponentDescription * outDesc)
{
	AudioComponent comp = AudioComponentInstanceGetComponent(inInstance);
	if (comp)
		return AudioComponentGetDescription(comp, outDesc);

	return kAudio_ParamError;
}

#endif

