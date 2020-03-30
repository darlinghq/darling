/*	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	main.cpp
	
=============================================================================*/

/*
	auprocess
		- takes a source audio file, an AU and generates a processed file 
*/

#include "CAAUProcessor.h"
#include "CAAudioFile.h"
#include "CAXException.h"
#include "CAHostTimeBase.h"
#include "CAFilePathUtils.h"
#include "CAAudioFileFormats.h"

#if TARGET_OS_MAC
	#include <pthread.h>
	#include <mach/mach.h>
#endif

#define require_noerr ca_require_noerr

#define CA_AU_PROFILE_TIME 1

#if CA_AU_PROFILE_TIME
			UInt64 sReadTime = 0;
			UInt64 sRenderTime = 0;
#endif

#pragma mark __print helpers

void PRINT_MARKS ()
{
	printf ("| ");
	for (int i = 0; i < 48; ++i)
		printf (" ");
	printf ("|\n");
}

void PerfResult(const char *toolname, int group, const char *testname, double value, const char *units, const char *fmt="%.3f")
{
	printf("<result tool='%s' group='%d' test='%s' value='", toolname, group, testname);
	printf(fmt, value);
	printf("' units='%s' />\n", units);
}

static int lastProgressPrintDone = -1;
void PRINT_PROGRESS (Float32 inPercent) 
{
	int current = int(inPercent / 4.0);
	for (int i = lastProgressPrintDone; i < current; ++i)
		printf ("* ");
	lastProgressPrintDone = current;
}

#pragma mark __Inpput Callback Definitions

static AURenderCallbackStruct sInputCallback; // we set one of these two callbacks based on AU type

static OSStatus InputCallback (void 			*inRefCon, 
					AudioUnitRenderActionFlags 	*ioActionFlags, 
					const AudioTimeStamp 		*inTimeStamp, 
					UInt32 						inBusNumber, 
					UInt32 						inNumberFrames, 
					AudioBufferList 			*ioData)
{
													#if CA_AU_PROFILE_TIME 
														UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
													#endif

	CAAudioFile &readFile = *(static_cast<CAAudioFile*>(inRefCon));

#if !CAAF_USE_EXTAUDIOFILE
	if (SInt64(inTimeStamp->mSampleTime) > readFile.GetNumberPackets()) {
#else
	if (SInt64(inTimeStamp->mSampleTime) > readFile.GetNumberFrames()) {
#endif
#if DEBUG
	printf ("reading past end of input\n");
#endif
		return -1;
	}

	readFile.Seek (SInt64(inTimeStamp->mSampleTime));
	readFile.Read (inNumberFrames, ioData);

													#if CA_AU_PROFILE_TIME 
														sReadTime += (CAHostTimeBase::GetTheCurrentTime() - now); 
													#endif

	return noErr;
}

static OSStatus FConvInputCallback (void 			*inRefCon, 
					AudioUnitRenderActionFlags 	*ioActionFlags, 
					const AudioTimeStamp 		*inTimeStamp, 
					UInt32 						inBusNumber, 
					UInt32 						inNumberFrames, 
					AudioBufferList 			*ioData)
{
												#if CA_AU_PROFILE_TIME 
													UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
												#endif

	CAAudioFile &readFile = *(static_cast<CAAudioFile*>(inRefCon));

		// this test is ONLY needed in case of processing with a Format Converter type of AU
		// in all other cases, the CAAUProcessor class will NEVER call you for input
		// beyond the end of the file....

#if !CAAF_USE_EXTAUDIOFILE
	if (SInt64(inTimeStamp->mSampleTime) >= readFile.GetNumberPackets()) {
#else
	if (SInt64(inTimeStamp->mSampleTime) >= readFile.GetNumberFrames()) {
#endif
		return -1;
	}
	
	readFile.Seek (SInt64(inTimeStamp->mSampleTime));
	UInt32 readPackets = inNumberFrames;
		
		// also, have to do this for a format converter AU - otherwise we'd just read what we're told
#if !CAAF_USE_EXTAUDIOFILE
	if (SInt64(inTimeStamp->mSampleTime + inNumberFrames) > readFile.GetNumberPackets()) {
#else
	if (SInt64(inTimeStamp->mSampleTime + inNumberFrames) > readFile.GetNumberFrames()) {
#endif
		// first set this to zero as we're only going to read a partial number of frames
		AudioBuffer *buf = ioData->mBuffers;
		for (UInt32 i = ioData->mNumberBuffers; i--; ++buf)
			memset((Byte *)buf->mData, 0, buf->mDataByteSize);
#if !CAAF_USE_EXTAUDIOFILE
		readPackets = UInt32 (readFile.GetNumberPackets() - SInt64(inTimeStamp->mSampleTime));
#else
		readPackets = UInt32 (readFile.GetNumberFrames() - SInt64(inTimeStamp->mSampleTime));
#endif
	}
	
	readFile.Read (readPackets, ioData);

													#if CA_AU_PROFILE_TIME 
														sReadTime += (CAHostTimeBase::GetTheCurrentTime() - now); 
													#endif

	return noErr;
}

struct ReadBuffer {
	AUOutputBL *readData;
	UInt32 readFrames;
};

static OSStatus MemoryInputCallback (void		*inRefCon, 
					AudioUnitRenderActionFlags 	*ioActionFlags, 
					const AudioTimeStamp 		*inTimeStamp, 
					UInt32 						inBusNumber, 
					UInt32 						inNumberFrames, 
					AudioBufferList 			*ioData)
{
													#if CA_AU_PROFILE_TIME 
														UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
													#endif

	ReadBuffer *readBuffer = (ReadBuffer*)inRefCon;
	
	if (((readBuffer->readFrames + inNumberFrames) * sizeof(Float32)) > (readBuffer->readData->ABL()->mBuffers[0].mDataByteSize)) 
	{
		// going past read size
		AudioBuffer *buf = ioData->mBuffers;
		for (UInt32 i = ioData->mNumberBuffers; i--; ++buf)
			memset((Byte *)buf->mData, 0, buf->mDataByteSize);
	}
	else
	{
		AudioBuffer *buf = ioData->mBuffers;
		AudioBuffer *rBuf = readBuffer->readData->ABL()->mBuffers;
		for (UInt32 i = ioData->mNumberBuffers; i--; ++buf, ++rBuf) {
			AudioBuffer readB = *rBuf;
			readB.mData = static_cast<Float32*>(rBuf->mData) + readBuffer->readFrames;
			memcpy (buf->mData, readB.mData, buf->mDataByteSize);
		}
		readBuffer->readFrames += inNumberFrames;
	}

													#if CA_AU_PROFILE_TIME 
														sReadTime += (CAHostTimeBase::GetTheCurrentTime() - now); 
													#endif

	return noErr;
}

#pragma mark __Utility Helpers

CFPropertyListRef	 ReadPresetFromPresetFile (char* filePath)
{	
	if (!filePath)
		return NULL;
	
	FSRef ref;
	if (FSPathMakeRef((UInt8 *)filePath, &ref, NULL))
		return NULL;
		
	CFDataRef			resourceData = NULL;
	CFPropertyListRef   theData = NULL;
	CFStringRef			errString = NULL;
	CFURLRef			fileURL = CFURLCreateFromFSRef (kCFAllocatorDefault, &ref);
		if (fileURL == NULL) {
			goto home;
		}
		
	SInt32				result;
    
   // Read the XML file.
   Boolean status; status = CFURLCreateDataAndPropertiesFromResource (kCFAllocatorDefault, fileURL,
                                                                &resourceData,	// place to put file data
                                                                NULL, NULL, &result);
        if (status == false || result) {
            goto home;
        }
    
	theData = CFPropertyListCreateFromXMLData (kCFAllocatorDefault, resourceData,  
													kCFPropertyListImmutable, &errString);
        if (theData == NULL || errString) {
            if (theData)
				CFRelease (theData);
			theData = NULL;
			goto home;
       }
	
home:
	if (fileURL)
		CFRelease (fileURL);
	if (resourceData)
		CFRelease (resourceData);
    if (errString)
		CFRelease (errString);
		
	return theData;
}

#pragma mark __the setup code

#define OFFLINE_AU_CMD 		"[-au TYPE SUBTYPE MANU] The Audio Unit component description\n\t"
#define INPUT_FILE	 		"[-i /Path/To/File] The file that is to be processed.\n\t"
#define OUTPUT_FILE			"[-o /Path/To/File/To/Create] This will be in the same format as the input file\n\t"
#define AU_PRESET_CMD		"[-p /Path/To/AUPreset/File] Specify an AU Preset File to establish the state of the AU\n\t"
#define SHORT_MEM_CMD		"[-m] Just reads and processes the first half second of the input file\n\t"
#define USE_MAX_FRAMES		"[-f max_frames] default is 32768 (512 for aufc units)"
 
static char* usageStr = "Usage: auprocess\n\t" 
				OFFLINE_AU_CMD 
				INPUT_FILE
				OUTPUT_FILE
				AU_PRESET_CMD
				SHORT_MEM_CMD
				USE_MAX_FRAMES;

int main(int argc, const char * argv[])
{
	setbuf (stdout, NULL);


#if TARGET_OS_MAC
	{
		thread_extended_policy_data_t		theFixedPolicy;
		theFixedPolicy.timeshare = false;	// set to true for a non-fixed thread
		thread_policy_set(pthread_mach_thread_np(pthread_self()), 
													THREAD_EXTENDED_POLICY, 
													(thread_policy_t)&theFixedPolicy, 
													THREAD_EXTENDED_POLICY_COUNT);

		// We keep a reference to the spawning thread's priority around (initialized in the constructor), 
		// and set the importance of the child thread relative to the spawning thread's priority.
		thread_precedence_policy_data_t		thePrecedencePolicy;
		
		thePrecedencePolicy.importance = 63 - 36;
		thread_policy_set(pthread_mach_thread_np(pthread_self()), 
													THREAD_PRECEDENCE_POLICY, 
													(thread_policy_t)&thePrecedencePolicy, 
													THREAD_PRECEDENCE_POLICY_COUNT);
	}
#endif


// These are the variables that are set up from the input parsing
	char* srcFilePath = NULL;
	char* destFilePath = NULL;
	char* auPresetFile = NULL;
	bool shortMemoryProfile = false;
	OSType manu, subType, type = 0;
	int userSetFrames = -1;
	
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp (argv[i], "-au") == 0) {
            if ( (i + 3) < argc ) {                
                StrToOSType (argv[i + 1], type);
                StrToOSType (argv[i + 2], subType);
                StrToOSType (argv[i + 3], manu);
				i += 3;
			} else {
				printf ("Which Audio Unit:\n%s", usageStr);
				exit(1);
			}
		}
		else if (strcmp (argv[i], "-i") == 0) {
			srcFilePath = const_cast<char*>(argv[++i]);
		}
		else if (strcmp (argv[i], "-o") == 0) {
			destFilePath = const_cast<char*>(argv[++i]);
		}
		else if (strcmp (argv[i], "-p") == 0) {
			auPresetFile = const_cast<char*>(argv[++i]);
		}
		else if (strcmp (argv[i], "-m") == 0) {
			shortMemoryProfile = true;
		}
		else if (strcmp (argv[i], "-f") == 0) {
			sscanf(argv[++i], "%d", &userSetFrames);
		}
		else {
			printf ("%s\n", usageStr);
			exit(1);
		}
	}
	
	if (!type || !srcFilePath) {
		printf ("%s\n", usageStr);
		exit(1);
	}
	if (!destFilePath) {
		if (!shortMemoryProfile) {
			printf ("%s\n", usageStr);
			exit(1);
		}
	}
			// delete pre-existing output file
	if (!shortMemoryProfile) {
		FSRef destFSRef;
		if (FSPathMakeRef((UInt8 *)destFilePath, &destFSRef, NULL) == noErr) {
			// output file exists - delete it
			if (FSDeleteObject(&destFSRef)) {
				printf ("Cannot Delete Output File\n");
				exit(1);
			}
		}
	}
	
	CAComponentDescription desc(type, subType, manu);
	
	CFPropertyListRef presetDict = ReadPresetFromPresetFile(auPresetFile);
	
		// the num of frames to use when processing the file with the Render call
	UInt32 maxFramesToUse = shortMemoryProfile ? 512 : 32768;

		// not set from command line
	if (userSetFrames > 0) {
		maxFramesToUse = userSetFrames; 
	}
		
		// in some settings (for instance a delay with 100% feedback) tail time is essentially infinite
		// so you should safeguard the final OL render stage (post process) which is aimed at pulling the tail through
		// if you want to bypass this completely, just set this to zero.
	Float64 maxTailTimeSecs = 10.;
	
#pragma mark -
#pragma mark __ The driving code
#pragma mark -

	try 
	{
		CAComponent comp(desc);
			
			 // CAAUProcessor's constructor throws... so make sure the component is valid
		if (comp.IsValid() == false) {
			printf ("Can't Find Component\n");
			desc.Print();
			exit(1);
		}
			
		CAAUProcessor processor(comp);
													processor.AU().Print();
		
		CAAudioFile srcFile;
		CAAudioFile destFile; 
		
		srcFile.Open(srcFilePath);

		CAStreamBasicDescription procFormat (srcFile.GetFileDataFormat());
		procFormat.SetCanonical (srcFile.GetFileDataFormat().NumberChannels(), false);

													printf ("Processing Format:\n\t");
													procFormat.Print();
		
		
		if (!shortMemoryProfile) {
			FSRef parentDir;
			CFStringRef filename;
			PosixPathToParentFSRefAndName(destFilePath, parentDir, filename);
			destFile.CreateNew (parentDir, filename, 'AIFF', srcFile.GetFileDataFormat());
			destFile.SetClientFormat (procFormat);
		}
	
		srcFile.SetClientFormat (procFormat);
		
		AUOutputBL outputList(procFormat);

		ReadBuffer* readBuf = NULL;	

#if !CAAF_USE_EXTAUDIOFILE
		UInt64 numInputSamples = srcFile.GetNumberPackets();
#else
		UInt64 numInputSamples = srcFile.GetNumberFrames();
#endif
	
		if (shortMemoryProfile) {
			readBuf = new ReadBuffer;
			readBuf->readData = new AUOutputBL(procFormat);
			readBuf->readFrames = 0;
			UInt32 numFrames = UInt32(procFormat.mSampleRate / 2);
			readBuf->readData->Allocate (numFrames); // half a second of audio data
			readBuf->readData->Prepare(); // half a second of audio data
				
				// read 1/2 second of audio into this read buffer
			srcFile.Read (numFrames, readBuf->readData->ABL());
			
			sInputCallback.inputProc = MemoryInputCallback;
			sInputCallback.inputProcRefCon = readBuf;
			numInputSamples = numFrames;
		}
		else {
			if (desc.IsFConv()) {
				maxFramesToUse = userSetFrames == -1 ? 512 : maxFramesToUse; 
				// some format converter's can call you several times in small granularities
				// so you can't use a large buffer to render or you won't return all of the input data
				// this also lessens the final difference between what you should get and what you do
				// converter units *really* should have a version that are offline AU's to 
				// handle this for you.
				sInputCallback.inputProc = FConvInputCallback;
			} else
				sInputCallback.inputProc = InputCallback;
			
			sInputCallback.inputProcRefCon = &srcFile;
		}
				
		OSStatus result;
		require_noerr (result = processor.EstablishInputCallback (sInputCallback), home);
		require_noerr (result = processor.SetMaxFramesPerRender (maxFramesToUse), home); 
		processor.SetMaxTailTime (maxTailTimeSecs);
		require_noerr (result = processor.Initialize (procFormat, numInputSamples), home);
		if (presetDict) {
			require_noerr (result = processor.SetAUPreset (presetDict), home);
			CFRelease (presetDict);
		}
			// this does ALL of the preflighting.. could be specialise for an OfflineAU type
			// to do this piecemeal and do a progress bar by using the OfflineAUPreflight method
		require_noerr (result = processor.Preflight (), home);
		
		bool isDone; isDone = false;
		bool needsPostProcessing;
		bool isSilence;
		UInt32 numFrames; numFrames = processor.MaxFramesPerRender();

#if CA_AU_PROFILE_TIME
		sReadTime = 0;
		sRenderTime = 0;
#endif
					
PRINT_MARKS();
			// this is the render loop
		while (!isDone) 
		{
											#if CA_AU_PROFILE_TIME 
												UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
											#endif
			outputList.Prepare(); // have to do this every time...
			require_noerr (result = processor.Render (outputList.ABL(), numFrames, isSilence, &isDone,
											&needsPostProcessing), home);
											#if CA_AU_PROFILE_TIME 
												sRenderTime += (CAHostTimeBase::GetTheCurrentTime() - now);
											#endif

if (!shortMemoryProfile)
	PRINT_PROGRESS(processor.GetOLPercentComplete());
else
	PRINT_PROGRESS(((processor.SampleTime() / numInputSamples) * 100.));
	
			if (numFrames && !shortMemoryProfile)
				destFile.Write (numFrames, outputList.ABL());
		}
			
			// this is the postprocessing if needed
		if (!shortMemoryProfile && needsPostProcessing) 
		{
			isDone = false;
			numFrames = processor.MaxFramesPerRender();
			while (!isDone) {
				outputList.Prepare(); // have to do this every time...
											#if CA_AU_PROFILE_TIME 
												UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
											#endif
				require_noerr (result = processor.PostProcess (outputList.ABL(), numFrames, 
													isSilence, isDone), home);
											#if CA_AU_PROFILE_TIME 
												sRenderTime += (CAHostTimeBase::GetTheCurrentTime() - now); 
											#endif

PRINT_PROGRESS(processor.GetOLPercentComplete());

				if (numFrames && !shortMemoryProfile)
					destFile.Write (numFrames, outputList.ABL());
			}
		}

printf ("\n");

home:
		if (result) {
			printf ("Exit with bad result:%ld\n", result);
			exit(result);
		}
		
		if (readBuf) {
			delete readBuf->readData;
			delete readBuf;
		}
					
#if CA_AU_PROFILE_TIME
	if (!shortMemoryProfile) {
			// this flushes any remaing data to be written to the disk. 
			// the source file is closed in its destructor of course
		destFile.Close(); 
			// open the file again, to get stats about it for profiling
		destFile.Open(destFilePath);
	}

	SInt64 numWritten;
	if (shortMemoryProfile)
		numWritten = 0;
	else {
#if !CAAF_USE_EXTAUDIOFILE
		numWritten = destFile.GetNumberPackets();
#else
		numWritten = destFile.GetNumberFrames();
#endif
	}

	printf ("Read File Time:%.2f secs for %lld packets (%.1f secs), wrote %lld packets\n", 
						(CAHostTimeBase::ConvertToNanos (sReadTime) / 1.0e9),
						numInputSamples,
						(numInputSamples / procFormat.mSampleRate),
						numWritten);

	if (!shortMemoryProfile) 
	{
#if !CAAF_USE_EXTAUDIOFILE
		UInt64 numOutputSamples = destFile.GetNumberPackets();
#else
		UInt64 numOutputSamples = destFile.GetNumberFrames();
#endif
	
		if (numOutputSamples == numInputSamples) {
			printf ("\tWrote the same number of packets as read\n");
		} else {
			bool expectationMet = !desc.IsOffline(); // we don't have any expectations for offline AU's
			if (processor.LatencySampleCount() || processor.TailSampleCount()) {
				if (numOutputSamples - numInputSamples == processor.TailSampleCount())
					expectationMet = true;
				if (expectationMet)	
					printf ("Correctly wrote \'Read Size + Tail\'. ");
				printf ("AU reports (samples): %ld latency, %ld tail\n", 
										processor.LatencySampleCount(), processor.TailSampleCount());
			}
			if (expectationMet == false) 
			{
				if (numOutputSamples > numInputSamples) {
					printf ("\tWrote %lld packets (%.2f secs) more than read\n", 
								(numOutputSamples - numInputSamples), 
								((numOutputSamples - numInputSamples) / procFormat.mSampleRate));
				} else {
					printf ("\tRead %lld packets (%.2f secs) more than wrote\n", 
								(numInputSamples - numOutputSamples), 
								((numInputSamples - numOutputSamples) / procFormat.mSampleRate));
				}
			}
		}
	}
	
	Float64 renderTimeSecs = CAHostTimeBase::ConvertToNanos (sRenderTime - sReadTime) / 1.0e9;
	printf ("Total Render Time:%.2f secs, using render slice size of %ld frames\n", 
							renderTimeSecs, maxFramesToUse);
	
	Float64 cpuUsage;
	if (shortMemoryProfile)
		cpuUsage = (renderTimeSecs / 0.5) * 100.;
	else
		cpuUsage = (renderTimeSecs / (numInputSamples / procFormat.mSampleRate)) * 100.;
	printf ("CPU Usage for Render Time:%.2f%%\n", cpuUsage);

	CFStringRef str = comp.GetCompName();
	UInt32 compNameLen = CFStringGetLength (str);
	
	CFStringRef presetName = NULL;
	if (auPresetFile) {
		CFPropertyListRef dict;
		if (processor.AU().GetAUPreset (dict) == noErr) {
			presetName = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)dict, CFSTR("name"));
			CFRelease (dict);
		}
	}

	UInt32 presetLen = presetName ? CFStringGetLength(presetName) : 0;

	char* cstr = (char*)malloc (compNameLen + presetLen + 2 + 1);
	CFStringGetCString (str, cstr, (CFStringGetLength (str) + 1), kCFStringEncodingASCII);
	if (presetName) {
		cstr[compNameLen] = ':';
		cstr[compNameLen+1] = ':';
		CFStringGetCString (presetName, cstr + compNameLen + 2, (CFStringGetLength (presetName) + 1), kCFStringEncodingASCII);
	}
	PerfResult("AudioUnitProcess", EndianU32_NtoB(comp.Desc().componentSubType), cstr, cpuUsage, "%realtime");
	free (cstr);
#endif


	}
	catch (CAXException &e) {
		char buf[256];
		printf("Error: %s (%s)\n", e.mOperation, e.FormatError(buf, sizeof(buf)));
		exit(1);
	}
	catch (...) {
		printf("An unknown error occurred\n");
		exit(1);
	}
			
	return 0;
}

