/*
 <codex>
 <abstract>Part of CoreAudio Utility Classes</abstract>
 </codex>
*/
#ifndef __AUMIDIEffectBase_h__
#define __AUMIDIEffectBase_h__

#include "AUMIDIBase.h"
#include "AUEffectBase.h"

// ________________________________________________________________________
//	AUMIDIEffectBase
//
	/*! @class AUMIDIEffectBase */
class AUMIDIEffectBase : public AUEffectBase, public AUMIDIBase {
public:
	/*! @ctor AUMIDIEffectBase */
								AUMIDIEffectBase(	AudioComponentInstance	inInstance,
													bool					inProcessesInPlace = false );
    /*! @method MIDIEvent */
    virtual OSStatus            MIDIEvent(UInt32            inStatus,
                                          UInt32            inData1,
                                          UInt32            inData2,
                                          UInt32            inOffsetSampleFrame)
	{
		return AUMIDIBase::MIDIEvent (inStatus, inData1, inData2, inOffsetSampleFrame);
	}
    
	/*! @method SysEx */
	virtual OSStatus            SysEx(const UInt8 *         inData,
                                      UInt32                inLength)
	{
		return AUMIDIBase::SysEx (inData, inLength);
	}

	/*! @method GetPropertyInfo */
	virtual OSStatus			GetPropertyInfo(AudioUnitPropertyID			inID,
												AudioUnitScope				inScope,
												AudioUnitElement			inElement,
												UInt32 &					outDataSize,
												Boolean &					outWritable);

	/*! @method GetProperty */
	virtual OSStatus			GetProperty(	AudioUnitPropertyID 		inID,
												AudioUnitScope 				inScope,
												AudioUnitElement		 	inElement,
												void *						outData);
	/*! @method SetProperty */
	virtual OSStatus			SetProperty(	AudioUnitPropertyID 		inID,
												AudioUnitScope 				inScope,
												AudioUnitElement		 	inElement,
												const void *				inData,
												UInt32						inDataSize);

#if !CA_USE_AUDIO_PLUGIN_ONLY
#if !TARGET_OS_IPHONE
	// component dispatcher
	/*! @method ComponentEntryDispatch */
	static OSStatus			ComponentEntryDispatch(	ComponentParameters *			params,
														AUMIDIEffectBase *				This);
#endif
#endif
};

#endif // __AUMIDIEffectBase_h__
