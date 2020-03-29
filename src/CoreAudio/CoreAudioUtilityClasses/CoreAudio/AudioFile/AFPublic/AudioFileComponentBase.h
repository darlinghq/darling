/*
     File: AudioFileComponentBase.h
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
#ifndef __AudioFileComponentBase_h__
#define __AudioFileComponentBase_h__

#include "ComponentBase.h"
#include "AudioFileFormat.h"

struct AudioFileComponentLookup {
	static AudioComponentMethod Lookup (SInt16 selector);
};
template <class Implementor>
class AudioFileComponentFactory : public APFactory<AudioFileComponentLookup, Implementor>
{
};

/* subclass, override AudioFileComponentBase() and GetAudioFormat() and that's it. */

class AudioFileComponentBase : public ComponentBase
{
public:
	AudioFileComponentBase(AudioComponentInstance inInstance);
	virtual ~AudioFileComponentBase();

	virtual AudioFileFormatBase* GetAudioFileFormatBase() const = 0;
	
	/* Public API Function Support */
	virtual OSStatus AFAPI_CreateURL(
								CFURLRef							inFileRef,
                                const AudioStreamBasicDescription	*inFormat,
                                UInt32								inFlags) { return kAudio_UnimplementedError; }

	virtual OSStatus AFAPI_OpenURL(
									CFURLRef		inFileRef, 
									SInt8  			inPermissions,
									int				inFD) { return kAudio_UnimplementedError; }
									

	virtual OSStatus AFAPI_Create(
								const FSRef							*inParentRef, 
                                CFStringRef							inFileName,
                                const AudioStreamBasicDescription	*inFormat,
                                UInt32								inFlags,
                                FSRef								*outNewFileRef) { return kAudio_UnimplementedError; }
								
	virtual OSStatus AFAPI_Initialize(
									const FSRef							*inFileRef,
                                    const AudioStreamBasicDescription	*inFormat,
                                    UInt32								inFlags) { return kAudio_UnimplementedError; }
								
	virtual OSStatus AFAPI_Open(
									const FSRef		*inFileRef, 
									SInt8  			inPermissions,
									SInt16			inRefNum) { return kAudio_UnimplementedError; }

	virtual OSStatus AFAPI_OpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc)=0;

	virtual OSStatus AFAPI_InitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
				UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags)=0;
									
	virtual OSStatus AFAPI_Close()=0;
	virtual OSStatus AFAPI_Optimize()=0;
	virtual OSStatus AFAPI_ReadBytes(		Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											void			*outBuffer)=0;
									
	virtual OSStatus AFAPI_WriteBytes(		Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											const void		*inBuffer)=0;
									
	virtual OSStatus AFAPI_ReadPackets(		Boolean							inUseCache,
											UInt32							*outNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer)=0;
											
	virtual OSStatus AFAPI_ReadPacketData(	Boolean							inUseCache,
											UInt32							*ioNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer)=0;
									
	virtual OSStatus AFAPI_WritePackets(	Boolean								inUseCache,
											UInt32								inNumBytes,
											const AudioStreamPacketDescription	*inPacketDescriptions,
											SInt64								inStartingPacket, 
											UInt32								*ioNumPackets, 
											const void							*inBuffer)=0;
									
	virtual OSStatus AFAPI_GetPropertyInfo(	AudioFilePropertyID		inPropertyID,
											UInt32					*outDataSize,
											UInt32					*isWritable)=0;
										
	virtual OSStatus AFAPI_GetProperty(		AudioFilePropertyID		inPropertyID,
											UInt32					*ioDataSize,
											void					*ioPropertyData)=0;
									
	virtual OSStatus AFAPI_SetProperty(		AudioFilePropertyID		inPropertyID,
											UInt32					inDataSize,
											const void				*inPropertyData)=0;
	virtual OSStatus AFAPI_GetGlobalInfoSize(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inSpecifierSize,
											const void*				inSpecifier,
											UInt32					*outPropertySize);

	virtual OSStatus AFAPI_GetGlobalInfo(		
											AudioFilePropertyID		inPropertyID,
											UInt32					inSpecifierSize,
											const void*				inSpecifier,
											UInt32					*ioPropertySize,
											void					*ioPropertyData);

	virtual OSStatus AFAPI_CountUserData(   UInt32					inUserDataID,
											UInt32					*outNumberItems)=0;
											
	virtual OSStatus AFAPI_GetUserDataSize( UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					*outDataSize)=0;
											
	virtual OSStatus AFAPI_GetUserData(		UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					*ioDataSize,
											void					*ioUserData)=0;
											
	virtual OSStatus AFAPI_SetUserData(		UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					inDataSize,
											const void				*inUserData)=0;
											
	virtual OSStatus AFAPI_RemoveUserData(	UInt32				inUserDataID,
											UInt32					inIndex)=0;
#if !CA_USE_AUDIO_PLUGIN_ONLY
	static OSStatus		ComponentEntryDispatch(ComponentParameters *p, AudioFileComponentBase *This);
#endif

protected:
};



class AudioFileObjectComponentBase : public AudioFileComponentBase
{
public:
	AudioFileObjectComponentBase(AudioComponentInstance inInstance);
	virtual ~AudioFileObjectComponentBase();

	virtual AudioFileFormat* GetAudioFormat() const = 0;
	virtual AudioFileFormatBase* GetAudioFileFormatBase() const { return GetAudioFormat(); }
	
	void SetAudioFileObject(AudioFileObject* inObject) { mAudioFileObject = inObject; }
	
	/* Public API Function Support */

	virtual OSStatus AFAPI_CreateURL(
								CFURLRef							inFileRef,
                                const AudioStreamBasicDescription	*inFormat,
                                UInt32								inFlags);

	virtual OSStatus AFAPI_OpenURL(
									CFURLRef		inFileRef, 
									SInt8  			inPermissions,
									int				inFD);

	virtual OSStatus AFAPI_OpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc);

	virtual OSStatus AFAPI_InitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
				UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags);
									
	virtual OSStatus AFAPI_Close();
	virtual OSStatus AFAPI_Optimize();
	virtual OSStatus AFAPI_ReadBytes(		Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											void			*outBuffer);
									
	virtual OSStatus AFAPI_WriteBytes(		Boolean			inUseCache,
											SInt64			inStartingByte, 
											UInt32			*ioNumBytes, 
											const void		*inBuffer);
									
	virtual OSStatus AFAPI_ReadPackets(		Boolean							inUseCache,
											UInt32							*outNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer);
									
	virtual OSStatus AFAPI_ReadPacketData(	Boolean							inUseCache,
											UInt32							*ioNumBytes,
											AudioStreamPacketDescription	*outPacketDescriptions,
											SInt64							inStartingPacket, 
											UInt32  						*ioNumPackets, 
											void							*outBuffer);
									
	virtual OSStatus AFAPI_WritePackets(	Boolean								inUseCache,
											UInt32								inNumBytes,
											const AudioStreamPacketDescription	*inPacketDescriptions,
											SInt64								inStartingPacket, 
											UInt32								*ioNumPackets, 
											const void							*inBuffer);
									
	virtual OSStatus AFAPI_GetPropertyInfo(	AudioFilePropertyID		inPropertyID,
											UInt32					*outDataSize,
											UInt32					*isWritable);
										
	virtual OSStatus AFAPI_GetProperty(		AudioFilePropertyID		inPropertyID,
											UInt32					*ioDataSize,
											void					*ioPropertyData);
									
	virtual OSStatus AFAPI_SetProperty(		AudioFilePropertyID		inPropertyID,
											UInt32					inDataSize,
											const void				*inPropertyData);

	virtual OSStatus AFAPI_CountUserData(   UInt32					inUserDataID,
											UInt32					*outNumberItems);
											
	virtual OSStatus AFAPI_GetUserDataSize( UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					*outDataSize);
											
	virtual OSStatus AFAPI_GetUserData(		UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					*ioDataSize,
											void					*ioUserData);
											
	virtual OSStatus AFAPI_SetUserData(		UInt32					inUserDataID,
											UInt32					inIndex,
											UInt32					inDataSize,
											const void				*inUserData);
												
	virtual OSStatus AFAPI_RemoveUserData(	UInt32					inUserDataID,
											UInt32					inIndex);
												
protected:
	AudioFileObject* mAudioFileObject;
};

#endif
