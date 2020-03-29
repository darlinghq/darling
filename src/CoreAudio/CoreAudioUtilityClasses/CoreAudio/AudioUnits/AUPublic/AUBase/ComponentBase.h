/*
     File: ComponentBase.h
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
#ifndef __ComponentBase_h__
#define __ComponentBase_h__

#include <new>
#include "CADebugMacros.h"
#include "CAXException.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
	#include <AudioUnit/AudioUnit.h>

	#if !CA_USE_AUDIO_PLUGIN_ONLY
		#include <CoreServices/Components.h>
	
		#if	(MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_5)
			#define AudioComponentInstance			ComponentInstance
			#define AudioComponentDescription		ComponentDescription
			#define	AudioComponent					Component
		#endif
		Handle CMgr_GetComponentInstanceStorage(ComponentInstance aComponentInstance);
		void CMgr_SetComponentInstanceStorage(ComponentInstance aComponentInstance, Handle theStorage);
	#endif

	#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_4
		typedef Float32 AudioUnitParameterValue;
	#endif
	#if COREAUDIOTYPES_VERSION < 1051
		typedef Float32 AudioUnitSampleType;
	#endif

	#if !TARGET_OS_WIN32
		#include <pthread.h>
	#endif

	#if TARGET_OS_WIN32
		#include "CAGuard.h"
	#endif
#else
	#include "CoreAudioTypes.h"
	#if !CA_USE_AUDIO_PLUGIN_ONLY
		#include "ComponentManagerDependenciesWin.h"
	#endif
	#include "AudioUnit.h"
	#include "CAGuard.h"
#endif

#ifndef COMPONENT_THROW
	#if VERBOSE_COMPONENT_THROW
		#define COMPONENT_THROW(throw_err) \
			do { DebugMessage(#throw_err); throw static_cast<OSStatus>(throw_err); } while (0)
	#else
		#define COMPONENT_THROW(throw_err) \
			throw static_cast<OSStatus>(throw_err)
	#endif
#endif

#define COMPONENT_CATCH \
	catch (const CAXException &ex) { result = ex.mError; } \
	catch (std::bad_alloc &) { result = kAudio_MemFullError; } \
	catch (OSStatus catch_err) { result = catch_err; } \
	catch (OSErr catch_err) { result = catch_err; } \
	catch (...) { result = -1; }

/*! @class ComponentBase */
class ComponentBase {
public:
	// classic MacErrors
	enum { noErr = 0};

	/*! @ctor ComponentBase */
				ComponentBase(AudioComponentInstance inInstance);
				
	/*! @dtor ~ComponentBase */
	virtual 	~ComponentBase();
	
	/*! @method PostConstructor */
	virtual void			PostConstructor();
	
	/*! @method PreDestructor */
	virtual void			PreDestructor();

#if !CA_USE_AUDIO_PLUGIN_ONLY
	/*! @method Version */
	virtual OSStatus		Version();

	/*! @method ComponentEntryDispatch */
	static OSStatus		ComponentEntryDispatch(ComponentParameters *p, ComponentBase *This);

	/*! GetSelectorForCanDo */
	static SInt16		GetSelectorForCanDo(ComponentParameters *params);
#endif
	
	/*! @method GetComponentInstance */
	AudioComponentInstance		GetComponentInstance() const { return mComponentInstance; }

	/*! @method GetComponentDescription */
	AudioComponentDescription	GetComponentDescription() const;

	// This global variable is so that new instances know how they were instantiated: via the Component Manager, 
	// or as AudioComponents. It's ugly, but preferable to altering the constructor of every class in the hierarchy.
	// It's safe because construction is protected by ComponentInitLocker.
	enum EInstanceType { kComponentMgrInstance, kAudioComponentInstance };
	static EInstanceType sNewInstanceType;

	/*! @method IsPluginObject */
	bool			IsPluginObject () const { return mInstanceType == kAudioComponentInstance; }
	/*! @method IsCMgrObject */
	bool			IsCMgrObject () const { return mInstanceType == kComponentMgrInstance; }

	/*! @method AP_Open */
	static OSStatus AP_Open(void *self, AudioUnit compInstance);

	/*! @method AP_Close */
	static OSStatus AP_Close(void *self);
	
protected:
	/*! @var mComponentInstance */
	AudioComponentInstance		mComponentInstance;
	EInstanceType				mInstanceType;
};

class ComponentInitLocker 
{
#if TARGET_OS_MAC
public:
	ComponentInitLocker() 
	{ 
		pthread_once(&sOnce, InitComponentInitLocker);
		pthread_mutex_lock(&sComponentOpenMutex); 
		mPreviousNewInstanceType = ComponentBase::sNewInstanceType;
	}
	~ComponentInitLocker() 
	{ 
		ComponentBase::sNewInstanceType = mPreviousNewInstanceType;
		pthread_mutex_unlock(&sComponentOpenMutex); 
	}

	// There are situations (11844772) where we need to be able to release the lock early.
	class Unlocker {
	public:
		Unlocker()
		{
			pthread_mutex_unlock(&sComponentOpenMutex);
		}
		~Unlocker()
		{
			pthread_mutex_lock(&sComponentOpenMutex); 
		}
	};

private:
	static pthread_mutex_t sComponentOpenMutex;
	static pthread_once_t sOnce;
	static void InitComponentInitLocker();
	
#elif TARGET_OS_WIN32
public:
	bool sNeedsUnlocking;
	ComponentInitLocker() { sNeedsUnlocking = sComponentOpenGuard.Lock(); }
	~ComponentInitLocker() { if(sNeedsUnlocking) { sComponentOpenGuard.Unlock(); } }
private:
	static CAGuard	sComponentOpenGuard;
#endif
	
private:
	ComponentBase::EInstanceType	mPreviousNewInstanceType;
};

/*! @class AudioComponentPlugInInstance */ 
struct AudioComponentPlugInInstance {
	AudioComponentPlugInInterface		mPlugInInterface;
	void *								(*mConstruct)(void *memory, AudioComponentInstance ci);
	void								(*mDestruct)(void *memory);
	void *								mPad[2];				// pad to a 16-byte boundary (in either 32 or 64 bit mode)
	UInt32								mInstanceStorage;		// the ACI implementation object is constructed into this memory
																// this member is just a placeholder. it is aligned to a 16byte boundary
};

/*! @class APFactory */ 
template <class APMethodLookup, class Implementor>
class APFactory {
public:
	static void *Construct(void *memory, AudioComponentInstance compInstance)
	{
		return new(memory) Implementor(compInstance);
	}
	
	static void Destruct(void *memory)
	{
		((Implementor *)memory)->~Implementor();
	}

	// This is the AudioComponentFactoryFunction. It returns an AudioComponentPlugInInstance.
	// The actual implementation object is not created until Open().
	static AudioComponentPlugInInterface *Factory(const AudioComponentDescription * /* inDesc */)
	{
		AudioComponentPlugInInstance *acpi = 
				(AudioComponentPlugInInstance *)malloc( offsetof(AudioComponentPlugInInstance, mInstanceStorage) + sizeof(Implementor) );
		acpi->mPlugInInterface.Open = ComponentBase::AP_Open;
		acpi->mPlugInInterface.Close = ComponentBase::AP_Close;
		acpi->mPlugInInterface.Lookup = APMethodLookup::Lookup;
		acpi->mPlugInInterface.reserved = NULL;
		acpi->mConstruct = Construct;
		acpi->mDestruct = Destruct;
		acpi->mPad[0] = NULL;
		acpi->mPad[1] = NULL;
		return (AudioComponentPlugInInterface*)acpi;
	}
	
	// This is for runtime registration (not for plug-ins loaded from bundles).
	static AudioComponent Register(UInt32 type, UInt32 subtype, UInt32 manuf, CFStringRef name, UInt32 vers, UInt32 flags=0)
	{
		AudioComponentDescription desc = { type, subtype, manuf, flags, 0 };
		return AudioComponentRegister(&desc, name, vers, Factory); 
	}
};

#if !CA_USE_AUDIO_PLUGIN_ONLY
/*! @class ComponentEntryPoint 
 *	@discussion This is only used for a component manager version
*/
template <class Class>
class ComponentEntryPoint {
public:
	/*! @method Dispatch */
	static OSStatus Dispatch(ComponentParameters *params, Class *obj)
	{
		OSStatus result = noErr;
		
		try {
			if (params->what == kComponentOpenSelect) {
				// solve a host of initialization thread safety issues.
				ComponentInitLocker lock;

				ComponentBase::sNewInstanceType = ComponentBase::kComponentMgrInstance;
				ComponentInstance ci = (ComponentInstance)(params->params[0]);
				Class *This = new Class((AudioComponentInstance)ci);
				This->PostConstructor();	// allows base class to do additional initialization
											// once the derived class is fully constructed
				
				CMgr_SetComponentInstanceStorage(ci, (Handle)This);
			} else
				result = Class::ComponentEntryDispatch(params, obj);
		}
		COMPONENT_CATCH
		
		return result;
	}
	
	/*! @method Register */
	static Component Register(OSType compType, OSType subType, OSType manufacturer)
	{
		ComponentDescription	description = {compType, subType, manufacturer, 0, 0};
		Component	component = RegisterComponent(&description, (ComponentRoutineUPP) Dispatch, registerComponentGlobal, NULL, NULL, NULL);
		if (component != NULL) {
			SetDefaultComponent(component, defaultComponentAnyFlagsAnyManufacturerAnySubType);
		}
		return component;
	}
};

// NOTE: Component Mgr is deprecated in ML.
// this macro should not be used with new audio components
// it is only for backwards compatibility with Lion and SL.
// this macro registers both a plugin and a component mgr version.
#define AUDIOCOMPONENT_ENTRY(FactoryType, Class) \
    extern "C" OSStatus Class##Entry(ComponentParameters *params, Class *obj); \
    extern "C" OSStatus Class##Entry(ComponentParameters *params, Class *obj) { \
        return ComponentEntryPoint<Class>::Dispatch(params, obj); \
    } \
    extern "C" void * Class##Factory(const AudioComponentDescription *inDesc); \
    extern "C" void * Class##Factory(const AudioComponentDescription *inDesc) { \
        return FactoryType<Class>::Factory(inDesc); \
    }
    // the only component we still support are the carbon based view components
    // you should be using this macro now to exclusively register those types
#define VIEW_COMPONENT_ENTRY(Class) \
    extern "C" OSStatus Class##Entry(ComponentParameters *params, Class *obj); \
    extern "C" OSStatus Class##Entry(ComponentParameters *params, Class *obj) { \
        return ComponentEntryPoint<Class>::Dispatch(params, obj); \
    }

	/*! @class ComponentRegistrar */
template <class Class, OSType Type, OSType Subtype, OSType Manufacturer>
class ComponentRegistrar {
public:
	/*! @ctor ComponentRegistrar */
	ComponentRegistrar() { ComponentEntryPoint<Class>::Register(Type, Subtype, Manufacturer); }
};

#define	COMPONENT_REGISTER(Class,Type,Subtype,Manufacturer) \
	static ComponentRegistrar<Class, Type, Subtype, Manufacturer>	gRegistrar##Class
#else
#define COMPONENT_ENTRY(Class)
#define COMPONENT_REGISTER(Class)
// this macro is used to generate the Entry Point for a given Audio Plugin
// you should be using this macro now with audio components
#define AUDIOCOMPONENT_ENTRY(FactoryType, Class) \
    extern "C" void * Class##Factory(const AudioComponentDescription *inDesc); \
    extern "C" void * Class##Factory(const AudioComponentDescription *inDesc) { \
        return FactoryType<Class>::Factory(inDesc); \
    }

#endif // !CA_USE_AUDIO_PLUGIN_ONLY


#endif // __ComponentBase_h__
