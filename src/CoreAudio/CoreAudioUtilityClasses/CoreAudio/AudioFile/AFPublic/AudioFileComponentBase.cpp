/*
     File: AudioFileComponentBase.cpp
 Abstract: AudioFileComponentBase.h
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
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioFileComponent.h>
#else
	#include "AudioFileComponent.h"
#endif

#include "AudioFileComponentBase.h"

#if TARGET_OS_MAC
	#if __LP64__
		// comp instance, parameters in forward order
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_index + 1];
	#else
		// parameters in reverse order, then comp instance
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_nparams - 1 - _index];
	#endif
#elif TARGET_OS_WIN32
		// (no comp instance), parameters in forward order
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_index];
#endif

#define ACPI ((AudioComponentPlugInInstance *)self)
#define AFC	((AudioFileComponentBase *)&ACPI->mInstanceStorage)

//----------------------------------------------------------------------------------------

static OSStatus CreateURL(
								void *                            self,
								CFURLRef							inFileRef,
								const AudioStreamBasicDescription	*inFormat,
								UInt32								inFlags)
{
	return AFC->AFAPI_CreateURL(inFileRef, inFormat, inFlags);
}

static OSStatus OpenURL(
								void *                            self,
								CFURLRef							inFileRef, 
								SInt8								inPermissions, 
								int									inFileDescriptor)
{
	return AFC->AFAPI_OpenURL(inFileRef, inPermissions, inFileDescriptor);
}

static OSStatus OpenWithCallbacks(
								void *                            self,
								void *								inRefCon, 
								AudioFile_ReadProc					inReadFunc, 
								AudioFile_WriteProc					inWriteFunc, 
								AudioFile_GetSizeProc				inGetSizeFunc,
								AudioFile_SetSizeProc				inSetSizeFunc)
{
	return AFC->AFAPI_OpenWithCallbacks(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
}

static OSStatus InitializeWithCallbacks(
								void *                            self,
								void *								inRefCon, 
								AudioFile_ReadProc					inReadFunc, 
								AudioFile_WriteProc					inWriteFunc, 
								AudioFile_GetSizeProc				inGetSizeFunc,
								AudioFile_SetSizeProc				inSetSizeFunc,
								UInt32								inFileType,
								const AudioStreamBasicDescription	*inFormat,
								UInt32								inFlags)
{
	return AFC->AFAPI_InitializeWithCallbacks(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc, inFileType, inFormat, inFlags);
}

static OSStatus Close(
								void *                            self)
{
	return AFC->AFAPI_Close();
}

static OSStatus Optimize(
								void *                            self)
{
	return AFC->AFAPI_Optimize();
}

static OSStatus ReadBytes(
								void *                            self,
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								void			*outBuffer)
{
	return AFC->AFAPI_ReadBytes(inUseCache, inStartingByte, ioNumBytes, outBuffer);
}


static OSStatus WriteBytes(
								void *         self,
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								const void		*inBuffer)
{
	return AFC->AFAPI_WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
}


static OSStatus ReadPackets(
								void *                        self,
								Boolean							inUseCache,
								UInt32							*outNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32							*ioNumPackets, 
								void							*outBuffer)
{
	return AFC->AFAPI_ReadPackets(inUseCache, outNumBytes, outPacketDescriptions, 
		inStartingPacket, ioNumPackets, outBuffer);
}

static OSStatus ReadPacketData(
								void *                        self,
								Boolean							inUseCache,
								UInt32							*ioNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32							*ioNumPackets, 
								void							*outBuffer)
{
	return AFC->AFAPI_ReadPacketData(inUseCache, ioNumBytes, outPacketDescriptions, 
		inStartingPacket, ioNumPackets, outBuffer);
}

#if COREAUDIOTYPES_VERSION < 1050
static OSStatus WritePackets(
								void *                            self,
								Boolean								inUseCache,
								UInt32								inNumBytes,
								AudioStreamPacketDescription		*inPacketDescriptions,
								SInt64								inStartingPacket, 
								UInt32								*ioNumPackets, 
								const void							*inBuffer)
#else
static OSStatus WritePackets(
								void *                            self,
								Boolean								inUseCache,
								UInt32								inNumBytes,
								const AudioStreamPacketDescription	*inPacketDescriptions,
								SInt64								inStartingPacket, 
								UInt32								*ioNumPackets, 
								const void							*inBuffer)
#endif
{
	return AFC->AFAPI_WritePackets(inUseCache, inNumBytes, 
		(const AudioStreamPacketDescription	*)inPacketDescriptions, // this should be const (and is in 10.5 headers)
		inStartingPacket, ioNumPackets, inBuffer);
}

static OSStatus GetPropertyInfo(
								void *                 self,
								AudioFilePropertyID    inPropertyID,
								UInt32					*outDataSize,
								UInt32					*isWritable)
{
	return AFC->AFAPI_GetPropertyInfo(inPropertyID, outDataSize, isWritable);
}

static OSStatus GetProperty(
								void *                 self,
								AudioFilePropertyID		inPropertyID,
								UInt32					*ioDataSize,
								void					*ioData)
{
	return AFC->AFAPI_GetProperty(inPropertyID, ioDataSize, ioData);
}


static OSStatus SetProperty(
								void *                 self,
								AudioFilePropertyID		inPropertyID,
								UInt32					inDataSize,
								const void				*inData)
{
	return AFC->AFAPI_SetProperty(inPropertyID, inDataSize, inData);
}

static OSStatus CountUserData(
								void *                 self,
								UInt32					inUserDataID,
								UInt32					*outNumberItems)
{
	return AFC->AFAPI_CountUserData(inUserDataID, outNumberItems);
}

static OSStatus GetUserDataSize(
								void *                 self,
								UInt32					inUserDataID,
								UInt32					inIndex,
								UInt32					*outDataSize)
{
	return AFC->AFAPI_GetUserDataSize(inUserDataID, inIndex, outDataSize);
}

static OSStatus GetUserData(
								void *                 self,
								UInt32					inUserDataID,
								UInt32					inIndex,
								UInt32					*ioUserDataSize,
								void					*outUserData)
{
	return AFC->AFAPI_GetUserData(inUserDataID, inIndex, ioUserDataSize, outUserData);
}

static OSStatus SetUserData(
								void *                 self,
								UInt32					inUserDataID,
								UInt32					inIndex,
								UInt32					inUserDataSize,
								const void				*inUserData)
{
	return AFC->AFAPI_SetUserData(inUserDataID, inIndex, inUserDataSize, inUserData);
}

static OSStatus RemoveUserData(
								void *                 self,
								UInt32					inUserDataID,
								UInt32					inIndex)
{
	return AFC->AFAPI_RemoveUserData(inUserDataID, inIndex);
}

static OSStatus ExtensionIsThisFormat(
								void *                 self,
								CFStringRef				inExtension,
								UInt32					*outResult)
{
	AudioFileFormatBase* aff = AFC->GetAudioFileFormatBase();
	if (!aff) return kAudio_ParamError;
	
	UInt32 res = aff->ExtensionIsThisFormat(inExtension);
	if (outResult) *outResult = res;
	return noErr;
}

static OSStatus FileDataIsThisFormat(
								void *                 self,
								UInt32					inDataByteSize, 
								const void*				inData,
								UInt32					*outResult)
{
	AudioFileFormatBase* aff = AFC->GetAudioFileFormatBase();
	if (!aff) return kAudio_ParamError;
	
	UncertainResult res = aff->FileDataIsThisFormat(inDataByteSize, inData);
	if (outResult) *outResult = res;
	return noErr;
}

static OSStatus GetGlobalInfoSize(
								void *                 self,
								AudioFilePropertyID	inPropertyID,
								UInt32					inSpecifierSize,
								const void*				inSpecifier,
								UInt32					*outPropertySize)
{
	return AFC->AFAPI_GetGlobalInfoSize(inPropertyID, inSpecifierSize, inSpecifier, outPropertySize);
}

static OSStatus GetGlobalInfo(
								void *                 self,
								AudioFilePropertyID	inPropertyID,
								UInt32					inSpecifierSize,
								const void*				inSpecifier,
								UInt32					*ioPropertySize,
								void					*ioPropertyData)
{
	return AFC->AFAPI_GetGlobalInfo(inPropertyID, inSpecifierSize, inSpecifier, ioPropertySize, ioPropertyData);
}

//----------------------------------------------------------------------------------------


AudioFileComponentBase::AudioFileComponentBase(AudioComponentInstance inInstance)
	: ComponentBase(inInstance)
{
}

AudioFileComponentBase::~AudioFileComponentBase()
{
}

AudioFileObjectComponentBase::AudioFileObjectComponentBase(AudioComponentInstance inInstance)
	: AudioFileComponentBase(inInstance), mAudioFileObject(0)
{
	// derived class should create an AudioFileObject here and if NULL, the AudioFormat as well.
}

AudioFileObjectComponentBase::~AudioFileObjectComponentBase()
{
	delete mAudioFileObject;
}

OSStatus AudioFileObjectComponentBase::AFAPI_CreateURL(
								CFURLRef							inFileRef, 
                                const AudioStreamBasicDescription	*inFormat,
                                UInt32								inFlags)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	
	OSStatus result = mAudioFileObject->DoCreate (inFileRef, inFormat, inFlags);
	return result;
}

								
OSStatus AudioFileObjectComponentBase::AFAPI_OpenURL(
									CFURLRef		inFileRef, 
									SInt8  			inPermissions,
									int				inFD)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	
	OSStatus result = mAudioFileObject->DoOpen(inFileRef, inPermissions, inFD);
 	return result;
}


OSStatus AudioFileObjectComponentBase::AFAPI_OpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->DoOpenWithCallbacks(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
}


OSStatus AudioFileObjectComponentBase::AFAPI_InitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
				UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->DoInitializeWithCallbacks(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc, 
											inFileType, inFormat, inFlags);
}

									
OSStatus AudioFileObjectComponentBase::AFAPI_Close()
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->DoClose();
}

OSStatus AudioFileObjectComponentBase::AFAPI_Optimize()
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->DoOptimize();
}

OSStatus AudioFileObjectComponentBase::AFAPI_ReadBytes(		
											Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											void			*outBuffer)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->ReadBytes(inUseCache, inStartingByte, ioNumBytes, outBuffer);
}


OSStatus AudioFileObjectComponentBase::AFAPI_WriteBytes(		
											Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											const void		*inBuffer)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
}




OSStatus AudioFileObjectComponentBase::AFAPI_ReadPackets(		
											Boolean							inUseCache,
											UInt32							*outNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->ReadPackets(inUseCache, outNumBytes, outPacketDescriptions,
												inStartingPacket, ioNumPackets, outBuffer);
}

OSStatus AudioFileObjectComponentBase::AFAPI_ReadPacketData(		
											Boolean							inUseCache,
											UInt32							*ioNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->ReadPacketData(inUseCache, ioNumBytes, outPacketDescriptions,
												inStartingPacket, ioNumPackets, outBuffer);
}

									
OSStatus AudioFileObjectComponentBase::AFAPI_WritePackets(	
											Boolean								inUseCache,
											UInt32								inNumBytes,
											const AudioStreamPacketDescription	*inPacketDescriptions,
											SInt64								inStartingPacket, 
											UInt32								*ioNumPackets, 
											const void							*inBuffer)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->WritePackets(inUseCache, inNumBytes, inPacketDescriptions,
												inStartingPacket, ioNumPackets, inBuffer);
}


									
OSStatus AudioFileObjectComponentBase::AFAPI_GetPropertyInfo(	
											AudioFilePropertyID		inPropertyID,
											UInt32					*outDataSize,
											UInt32					*isWritable)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->GetPropertyInfo(inPropertyID, outDataSize, isWritable);
}

										
OSStatus AudioFileObjectComponentBase::AFAPI_GetProperty(		
											AudioFilePropertyID		inPropertyID,
											UInt32					*ioPropertySize,
											void					*ioPropertyData)
{
	OSStatus err = noErr;
	
	if (!ioPropertyData) return kAudio_ParamError;
	
	if (!mAudioFileObject) return kAudio_ParamError;
	err = mAudioFileObject->GetProperty(inPropertyID, ioPropertySize, ioPropertyData);
	return err;
}

									
OSStatus AudioFileObjectComponentBase::AFAPI_SetProperty(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inPropertySize,
											const void				*inPropertyData)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->SetProperty(inPropertyID, inPropertySize, inPropertyData);
}


OSStatus AudioFileObjectComponentBase::AFAPI_CountUserData(		
											UInt32				inUserDataID,
											UInt32				*outNumberItems)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->CountUserData(inUserDataID, outNumberItems);
}

OSStatus AudioFileObjectComponentBase::AFAPI_GetUserDataSize(		
											UInt32				inUserDataID,
											UInt32				inIndex,
											UInt32				*outUserDataSize)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->GetUserDataSize(inUserDataID, inIndex, outUserDataSize);
}

OSStatus AudioFileObjectComponentBase::AFAPI_GetUserData(		
											UInt32				inUserDataID,
											UInt32				inIndex,
											UInt32				*ioUserDataSize,
											void				*outUserData)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->GetUserData(inUserDataID, inIndex, ioUserDataSize, outUserData);
}

OSStatus AudioFileObjectComponentBase::AFAPI_SetUserData(		
											UInt32				inUserDataID,
											UInt32				inIndex,
											UInt32				inUserDataSize,
											const void			*inUserData)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->SetUserData(inUserDataID, inIndex, inUserDataSize, inUserData);
}

OSStatus AudioFileObjectComponentBase::AFAPI_RemoveUserData(		
											UInt32				inUserDataID,
											UInt32				inIndex)
{
	if (!mAudioFileObject) return kAudio_ParamError;
	return mAudioFileObject->RemoveUserData(inUserDataID, inIndex);
}


OSStatus AudioFileComponentBase::AFAPI_GetGlobalInfoSize(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inSpecifierSize,
											const void*				inSpecifier,
											UInt32					*outPropertySize)
{
	OSStatus err = noErr;
		
	switch (inPropertyID)
	{
		case kAudioFileComponent_CanRead :
		case kAudioFileComponent_CanWrite :
			*outPropertySize = sizeof(UInt32);
			break;

		case kAudioFileComponent_FileTypeName :
			*outPropertySize = sizeof(CFStringRef);
			break;

		case kAudioFileComponent_ExtensionsForType :
			*outPropertySize = sizeof(CFArrayRef);
			break;
			
		case kAudioFileComponent_UTIsForType :
			*outPropertySize = sizeof(CFArrayRef);
			break;
			
		case kAudioFileComponent_MIMETypesForType :
			*outPropertySize = sizeof(CFArrayRef);
			break;
			
		case kAudioFileComponent_AvailableFormatIDs :
		{
			UInt32 size = 0xFFFFFFFF;
			err = GetAudioFileFormatBase()->GetAvailableFormatIDs(&size, NULL);
			if (!err) *outPropertySize = size;
			break;
		}
		case kAudioFileComponent_HFSTypeCodesForType :
		{
			UInt32 size = 0xFFFFFFFF;
			err = GetAudioFileFormatBase()->GetHFSCodes(&size, NULL);
			if (!err) *outPropertySize = size;
			break;
		}
		case kAudioFileComponent_AvailableStreamDescriptionsForFormat :
			{
				if (inSpecifierSize != sizeof(UInt32)) return kAudioFileBadPropertySizeError;
				UInt32 inFormatID = *(UInt32*)inSpecifier;
				err = GetAudioFileFormatBase()->GetAvailableStreamDescriptions(inFormatID, outPropertySize, NULL);
			}
			break;
			
		default:
			err = kAudioFileUnsupportedPropertyError;
	}
	return err;
}


OSStatus AudioFileComponentBase::AFAPI_GetGlobalInfo(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inSpecifierSize,
											const void*				inSpecifier,
											UInt32					*ioPropertySize,
											void					*ioPropertyData)
{
	OSStatus err = noErr;
	
	if (!ioPropertyData || !ioPropertySize) return kAudio_ParamError;
	
	switch (inPropertyID)
	{
		case kAudioFileComponent_CanRead :
			{
				if (*ioPropertySize != sizeof(UInt32)) return kAudioFileBadPropertySizeError;
				UInt32* flag = (UInt32*)ioPropertyData;
				*flag = GetAudioFileFormatBase()->CanRead();
			}
			break;
			
		case kAudioFileComponent_CanWrite :
			{
				if (*ioPropertySize != sizeof(UInt32)) return kAudioFileBadPropertySizeError;
				UInt32* flag = (UInt32*)ioPropertyData;
				*flag = GetAudioFileFormatBase()->CanWrite();
			}
			break;
			
		case kAudioFileComponent_FileTypeName :
			{
				if (*ioPropertySize != sizeof(CFStringRef)) return kAudioFileBadPropertySizeError;
				CFStringRef* name = (CFStringRef*)ioPropertyData;
				GetAudioFileFormatBase()->GetFileTypeName(name);
			}
			break;

		case kAudioFileComponent_ExtensionsForType :
			{
				if (*ioPropertySize != sizeof(CFArrayRef)) return kAudioFileBadPropertySizeError;
				CFArrayRef* array = (CFArrayRef*)ioPropertyData;
				GetAudioFileFormatBase()->GetExtensions(array);
			}
			break;

		case kAudioFileComponent_UTIsForType :
			{
				if (*ioPropertySize != sizeof(CFArrayRef)) return kAudioFileBadPropertySizeError;
				CFArrayRef* array = (CFArrayRef*)ioPropertyData;
				GetAudioFileFormatBase()->GetUTIs(array);
			}
			break;

		case kAudioFileComponent_MIMETypesForType :
			{
				if (*ioPropertySize != sizeof(CFArrayRef)) return kAudioFileBadPropertySizeError;
				CFArrayRef* array = (CFArrayRef*)ioPropertyData;
				GetAudioFileFormatBase()->GetMIMETypes(array);
			}
			break;

		case kAudioFileComponent_HFSTypeCodesForType :
			{
				err = GetAudioFileFormatBase()->GetHFSCodes(ioPropertySize, ioPropertyData);
			}
			break;
			
		case kAudioFileComponent_AvailableFormatIDs :
			{
				err = GetAudioFileFormatBase()->GetAvailableFormatIDs(ioPropertySize, ioPropertyData);
			}
			break;
			
		case kAudioFileComponent_AvailableStreamDescriptionsForFormat :
			{
				if (inSpecifierSize != sizeof(UInt32)) return kAudioFileBadPropertySizeError;
				UInt32 inFormatID = *(UInt32*)inSpecifier; 
				err = GetAudioFileFormatBase()->GetAvailableStreamDescriptions(inFormatID, ioPropertySize, ioPropertyData);
			}
			break;
						
		default:
			err = kAudioFileUnsupportedPropertyError;
	}
	return err;
}


#if !CA_USE_AUDIO_PLUGIN_ONLY
OSStatus AudioFileComponentBase::ComponentEntryDispatch(ComponentParameters* params, AudioFileComponentBase* inThis)
{
	OSStatus		result = noErr;
	if (inThis == NULL) return kAudio_ParamError;
	
	try
	{
		switch (params->what)
		{						
			case kComponentCanDoSelect:
				switch (GetSelectorForCanDo(params))
				{
					case kAudioFileCreateURLSelect:
					case kAudioFileOpenURLSelect:
					case kAudioFileOpenWithCallbacksSelect:
					case kAudioFileInitializeWithCallbacksSelect:
					case kAudioFileCloseSelect:
					case kAudioFileOptimizeSelect:
					case kAudioFileReadBytesSelect:
					case kAudioFileWriteBytesSelect:
					case kAudioFileReadPacketsSelect:
					case kAudioFileWritePacketsSelect:
					case kAudioFileGetPropertyInfoSelect:
					case kAudioFileGetPropertySelect:
					case kAudioFileSetPropertySelect:
					case kAudioFileExtensionIsThisFormatSelect:
					case kAudioFileFileDataIsThisFormatSelect:
					case kAudioFileGetGlobalInfoSizeSelect:
					case kAudioFileGetGlobalInfoSelect:
					
					case kAudioFileCountUserDataSelect:
					case kAudioFileGetUserDataSizeSelect:
					case kAudioFileGetUserDataSelect:
					case kAudioFileSetUserDataSelect:
					case kAudioFileRemoveUserDataSelect:
						result = 1;
						break;
					default:
					 // these are no longer supported
/*					case kAudioFileCreateSelect:
					case kAudioFileOpenSelect:
					case kAudioFileInitializeSelect:
*/						result = ComponentBase::ComponentEntryDispatch(params, inThis);
				};
				break;

				case kAudioFileCreateURLSelect:
					{
						PARAM(CFURLRef, inFileRef, 0, 3);
						PARAM(const AudioStreamBasicDescription*, inFormat, 1, 3);
						PARAM(UInt32, inFlags, 2, 3);
						
						result = inThis->AFAPI_CreateURL(inFileRef, inFormat, inFlags);
					}
					break;
				case kAudioFileOpenURLSelect:
					{
						PARAM(CFURLRef, inFileRef, 0, 3);
						PARAM(SInt32, inPermissions, 1, 3);
						PARAM(int, inFileDescriptor, 2, 3);
						
						result = inThis->AFAPI_OpenURL(inFileRef, inPermissions, inFileDescriptor);
					}
					break;
				case kAudioFileOpenWithCallbacksSelect:
					{
						PARAM(void*, inRefCon, 0, 5);
						PARAM(AudioFile_ReadProc, inReadFunc, 1, 5);
						PARAM(AudioFile_WriteProc, inWriteFunc, 2, 5);
						PARAM(AudioFile_GetSizeProc, inGetSizeFunc, 3, 5);
						PARAM(AudioFile_SetSizeProc, inSetSizeFunc, 4, 5);
						
						result = inThis->AFAPI_OpenWithCallbacks(inRefCon, inReadFunc, inWriteFunc, 
													inGetSizeFunc, inSetSizeFunc);
					}
					break;
				case kAudioFileInitializeWithCallbacksSelect:
					{
						PARAM(void*, inRefCon, 0, 8);
						PARAM(AudioFile_ReadProc, inReadFunc, 1, 8);
						PARAM(AudioFile_WriteProc, inWriteFunc, 2, 8);
						PARAM(AudioFile_GetSizeProc, inGetSizeFunc, 3, 8);
						PARAM(AudioFile_SetSizeProc, inSetSizeFunc, 4, 8);
						PARAM(UInt32, inFileType, 5, 8);
						PARAM(const AudioStreamBasicDescription*, inFormat, 6, 8);
						PARAM(UInt32, inFlags, 7, 8);
						
						result = inThis->AFAPI_InitializeWithCallbacks(inRefCon, inReadFunc, inWriteFunc, 
													inGetSizeFunc, inSetSizeFunc,
													inFileType, inFormat, inFlags);
					}
					break;
				case kAudioFileCloseSelect:
					{
						result = inThis->AFAPI_Close();
					}
					break;
				case kAudioFileOptimizeSelect:
					{
						result = inThis->AFAPI_Optimize();
					}
					break;
				case kAudioFileReadBytesSelect:
					{
						PARAM(UInt32, inUseCache, 0, 4);
						PARAM(SInt64*, inStartingByte, 1, 4);
						PARAM(UInt32*, ioNumBytes, 2, 4);
						PARAM(void*, outBuffer, 3, 4);
						
						result = inThis->AFAPI_ReadBytes(inUseCache, *inStartingByte, ioNumBytes,
							outBuffer);
					}
					break;
				case kAudioFileWriteBytesSelect:
					{
						PARAM(UInt32, inUseCache, 0, 4);
						PARAM(SInt64*, inStartingByte, 1, 4);
						PARAM(UInt32*, ioNumBytes, 2, 4);
						PARAM(const void*, inBuffer, 3, 4);
						
						result = inThis->AFAPI_WriteBytes(inUseCache, *inStartingByte, ioNumBytes,
							inBuffer);
					}
					break;
				case kAudioFileReadPacketsSelect:
					{
						PARAM(UInt32, inUseCache, 0, 6);
						PARAM(UInt32*, outNumBytes, 1, 6);
						PARAM(AudioStreamPacketDescription*, outPacketDescriptions, 2, 6);
						PARAM(SInt64*, inStartingPacket, 3, 6);
						PARAM(UInt32*, ioNumPackets, 4, 6);
						PARAM(void*, outBuffer, 5, 6);
						
						result = inThis->AFAPI_ReadPackets(inUseCache, outNumBytes, outPacketDescriptions, 
							*inStartingPacket, ioNumPackets, outBuffer);
					}
					break;
				case kAudioFileWritePacketsSelect:
					{
						PARAM(UInt32, inUseCache, 0, 6);
						PARAM(UInt32, inNumBytes, 1, 6);
						PARAM(const AudioStreamPacketDescription*, inPacketDescriptions, 2, 6);
						PARAM(SInt64*, inStartingPacket, 3, 6);
						PARAM(UInt32*, ioNumPackets, 4, 6);
						PARAM(const void*, inBuffer, 5, 6);
						
						result = inThis->AFAPI_WritePackets(inUseCache, inNumBytes, inPacketDescriptions, 
							*inStartingPacket, ioNumPackets, inBuffer);
					}
					break;
					
				case kAudioFileGetPropertyInfoSelect:
					{
						PARAM(AudioFileComponentPropertyID, inPropertyID, 0, 3);
						PARAM(UInt32*, outPropertySize, 1, 3);
						PARAM(UInt32*, outWritable, 2, 3);
						
						result = inThis->AFAPI_GetPropertyInfo(inPropertyID, outPropertySize, outWritable);
					}
					break;
					
				case kAudioFileGetPropertySelect:
					{
						PARAM(AudioFileComponentPropertyID, inPropertyID, 0, 3);
						PARAM(UInt32*, ioPropertyDataSize, 1, 3);
						PARAM(void*, outPropertyData, 2, 3);
						
						result = inThis->AFAPI_GetProperty(inPropertyID, ioPropertyDataSize, outPropertyData);
					}
					break;
				case kAudioFileSetPropertySelect:
					{
						PARAM(AudioFileComponentPropertyID, inPropertyID, 0, 3);
						PARAM(UInt32, inPropertyDataSize, 1, 3);
						PARAM(const void*, inPropertyData, 2, 3);
						
						result = inThis->AFAPI_SetProperty(inPropertyID, inPropertyDataSize, inPropertyData);
					}
					break;
					
				case kAudioFileGetGlobalInfoSizeSelect:
					{
						PARAM(AudioFileComponentPropertyID, inPropertyID, 0, 4);
						PARAM(UInt32, inSpecifierSize, 1, 4);
						PARAM(const void*, inSpecifier, 2, 4);
						PARAM(UInt32*, outPropertyDataSize, 3, 4);
						
						result = inThis->AFAPI_GetGlobalInfoSize(inPropertyID, inSpecifierSize, inSpecifier,
							outPropertyDataSize);
					}
					break;
				case kAudioFileGetGlobalInfoSelect:
					{
						PARAM(AudioFileComponentPropertyID, inPropertyID, 0, 5);
						PARAM(UInt32, inSpecifierSize, 1, 5);
						PARAM(const void*, inSpecifier, 2, 5);
						PARAM(UInt32*, ioPropertyDataSize, 3, 5);
						PARAM(void*, outPropertyData, 4, 5);
						
						result = inThis->AFAPI_GetGlobalInfo(inPropertyID, inSpecifierSize, inSpecifier,
							ioPropertyDataSize, outPropertyData);
					}
					break;
					
				case kAudioFileExtensionIsThisFormatSelect:
					{
						PARAM(CFStringRef, inExtension, 0, 2);
						PARAM(UInt32*, outResult, 1, 2);
						
						AudioFileFormatBase* aff = inThis->GetAudioFileFormatBase();
						if (!aff) return kAudio_ParamError;
						
						UInt32 res = aff->ExtensionIsThisFormat(inExtension);
						if (outResult) *outResult = res;
					}
					break;
					
				case kAudioFileFileDataIsThisFormatSelect:
					{
						PARAM(UInt32, inDataByteSize, 0, 3);
						PARAM(const void*, inData, 1, 3);
						PARAM(UInt32*, outResult, 2, 3);
						
						AudioFileFormatBase* aff = inThis->GetAudioFileFormatBase();
						if (!aff) return kAudio_ParamError;
						
						UncertainResult res = aff->FileDataIsThisFormat(inDataByteSize, inData);
						if (outResult) *outResult = res;
					}
					break;

				case kAudioFileCountUserDataSelect:
					{
						PARAM(UInt32, inUserDataID, 0, 2);
						PARAM(UInt32*, outNumberItems, 1, 2);
					
						result = inThis->AFAPI_CountUserData(inUserDataID, outNumberItems);
					}
					break;
					
				case kAudioFileGetUserDataSizeSelect:
					{
						PARAM(UInt32, inUserDataID, 0, 3);
						PARAM(UInt32, inIndex, 1, 3);
						PARAM(UInt32*, outUserDataSize, 2, 3);
						
						result = inThis->AFAPI_GetUserDataSize(inUserDataID, inIndex, outUserDataSize);
					}
					break;
					
				case kAudioFileGetUserDataSelect:
					{
						PARAM(UInt32, inUserDataID, 0, 4);
						PARAM(UInt32, inIndex, 1, 4);
						PARAM(UInt32*, ioUserDataSize, 2, 4);
						PARAM(void*, outUserData, 3, 4);
						
						result = inThis->AFAPI_GetUserData(inUserDataID, inIndex, 
										ioUserDataSize, outUserData);
					}
					break;
					
				case kAudioFileSetUserDataSelect:
					{
						PARAM(UInt32, inUserDataID, 0, 4);
						PARAM(UInt32, inIndex, 1, 4);
						PARAM(UInt32, inUserDataSize, 2, 4);
						PARAM(const void*, inUserData, 3, 4);
						
						result = inThis->AFAPI_SetUserData(inUserDataID, inIndex, 
										inUserDataSize, inUserData);
					}
					break;
	
				case kAudioFileRemoveUserDataSelect:
					{
						PARAM(UInt32, inUserDataID, 0, 2);
						PARAM(UInt32, inIndex, 1, 2);
						
						result = inThis->AFAPI_RemoveUserData(inUserDataID, inIndex);
					}
					break;
	
		
				default:
					result = ComponentBase::ComponentEntryDispatch(params, inThis);
					break;
		}
	}
	COMPONENT_CATCH
	return result;
} 
#endif

AudioComponentMethod AudioFileComponentLookup::Lookup (SInt16 selector)
{
	switch (selector) {
	
#define DefCase(NAME) case kAudioFile##NAME##Select: return (AudioComponentMethod)NAME

		DefCase(OpenWithCallbacks);
		DefCase(InitializeWithCallbacks);
		DefCase(Close);
		DefCase(Optimize);
		DefCase(ReadBytes);
		DefCase(WriteBytes);
		DefCase(ReadPackets);
		DefCase(WritePackets);
		DefCase(GetPropertyInfo);
		DefCase(GetProperty);
		DefCase(SetProperty);
		
		DefCase(ExtensionIsThisFormat);
		DefCase(GetGlobalInfoSize);
		DefCase(GetGlobalInfo);

		DefCase(CountUserData);
		DefCase(GetUserDataSize);
		DefCase(GetUserData);
		DefCase(SetUserData);
		DefCase(RemoveUserData);
		DefCase(CreateURL);
		DefCase(OpenURL);
		DefCase(FileDataIsThisFormat);
		DefCase(ReadPacketData);
		
		// These selectors are deprecated and do not appear: Create, Open, Initialize, FileIsThisFormat, DataIsThisFormat.

		default:
			break;
	}
	return NULL;
}
