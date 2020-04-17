/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
	auprofile.cpp
	
=============================================================================*/

/*
	auprofile
		- takes a source audio file, an AU and processes this for performance metrics
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

#if DEBUG
	#define VERBOSE 0
#endif

UInt64 sLastReadTime = 0;
Float64 sMinTime = 9999999999.; // too big a time!
Float64 sMaxTime = 0;

#pragma mark __print helpers

void PerfResult(const char *toolname, int group, const char *testname, double value, const char *units, const char *fmt="%.3f")
{
	printf("<result tool='%s' group='%d' test='%s' value='", toolname, group, testname);
	printf(fmt, value);
	printf("' units='%s' />\n", units);
}

#pragma mark __Inpput Callback Definitions


// the file is read into memory - the input's role is to parse through this memory

struct ReadBuffer {
	AUOutputBL *readData;
	UInt64 totalInputFrames;
	UInt32 lastInputFrames;
};

const int kEndOfInput = 12345;

static OSStatus MemoryInputCallback (void		*inRefCon, 
					AudioUnitRenderActionFlags 	*ioActionFlags, 
					const AudioTimeStamp 		*inTimeStamp, 
					UInt32 						inBusNumber, 
					UInt32 						inNumberFrames, 
					AudioBufferList 			*ioData)
{
														UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
	OSStatus result = 0;
	
	ReadBuffer *readBuffer = (ReadBuffer*)inRefCon;
	if (inTimeStamp->mSampleTime >= readBuffer->totalInputFrames) {
			#if VERBOSE
				printf ("reading: %.0f past input: %.0f\n", inTimeStamp->mSampleTime, 
							(double)readBuffer->totalInputFrames); 
			#endif
		result = kEndOfInput;
		readBuffer->lastInputFrames = 0;
		goto end;
	}
		
		// can get pulled multiple times
	readBuffer->lastInputFrames += inNumberFrames;

	if (UInt64(inTimeStamp->mSampleTime + inNumberFrames) > readBuffer->totalInputFrames) {
		// first set this to zero as we're only going to read a partial number of frames
		AudioBuffer *buf = ioData->mBuffers;
		for (UInt32 i = ioData->mNumberBuffers; i--; ++buf)
			memset((Byte *)buf->mData, 0, buf->mDataByteSize);

		inNumberFrames = UInt32 (readBuffer->totalInputFrames - UInt64(inTimeStamp->mSampleTime));
	}
	
		// copy data from the source to the ioData buffers
	{
		AudioBuffer *buf = ioData->mBuffers;
		AudioBuffer *rBuf = readBuffer->readData->ABL()->mBuffers;
		for (UInt32 i = ioData->mNumberBuffers; i--; ++buf, ++rBuf) {
			AudioBuffer readB = *rBuf;
			readB.mData = static_cast<Float32*>(rBuf->mData) + UInt32(inTimeStamp->mSampleTime);
			memcpy (buf->mData, readB.mData, (inNumberFrames * sizeof(Float32)));
		}
	}
	
end:	
														sLastReadTime += (CAHostTimeBase::GetTheCurrentTime() - now); 

	return result;
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
#define AU_PRESET_CMD		"[-p /Path/To/AUPreset/File] Specify an AU Preset File to establish the state of the AU\n\t"
#define USE_MAX_FRAMES		"[-f max_frames] default is 4096"
 
static char* usageStr = "Usage: auprofile\n\t" 
				OFFLINE_AU_CMD 
				INPUT_FILE
				AU_PRESET_CMD
				USE_MAX_FRAMES;

int main(int argc, const char * argv[])
{
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
	char* auPresetFile = NULL;
	OSType manu, subType, type = 0;
	UInt32 numFrames = 4096;
	
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
		else if (strcmp (argv[i], "-p") == 0) {
			auPresetFile = const_cast<char*>(argv[++i]);
		}
		else if (strcmp (argv[i], "-f") == 0) {
			sscanf(argv[++i], "%ld", &numFrames);
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
	
	CAComponentDescription desc(type, subType, manu);
	
	CFPropertyListRef presetDict = ReadPresetFromPresetFile(auPresetFile);
	
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
										processor.AU().Comp().Print();
		
		CAAudioFile srcFile;
		
		srcFile.Open(srcFilePath);

#if !CAAF_USE_EXTAUDIOFILE
		UInt64 numInputSamples = srcFile.GetNumberPackets();
#else
		UInt64 numInputSamples = srcFile.GetNumberFrames();
#endif
		
		Float64 inputSecs = (numInputSamples / srcFile.GetFileDataFormat().mSampleRate);
		
		CAStreamBasicDescription procFormat (srcFile.GetFileDataFormat());
		procFormat.SetCanonical (srcFile.GetFileDataFormat().NumberChannels(), false);

										printf ("Processing file: %s, %.1f secs [proc: %ld frames]\n", srcFilePath, inputSecs, numFrames);
										#if VERBOSE
											printf("\t");
											procFormat.Print();
										#endif
		
		srcFile.SetClientFormat (procFormat);
		
		AUOutputBL outputList(procFormat);
	
		// read the entire file into memory
		ReadBuffer* readBuf = new ReadBuffer;
		readBuf->readData = new AUOutputBL(procFormat);
		readBuf->totalInputFrames = numInputSamples;
		readBuf->readData->Allocate (numInputSamples);
		readBuf->readData->Prepare();
		UInt32 readSamps = (UInt32)numInputSamples;
		srcFile.Read (readSamps, readBuf->readData->ABL());
			
		AURenderCallbackStruct inputCallback;
		inputCallback.inputProc = MemoryInputCallback;
		inputCallback.inputProcRefCon = readBuf;
				
		OSStatus result;
		require_noerr (result = processor.EstablishInputCallback (inputCallback), home);
		require_noerr (result = processor.SetMaxFramesPerRender (numFrames), home); 
		require_noerr (result = processor.Initialize (procFormat, numInputSamples), home);
		if (presetDict) {
			require_noerr (result = processor.SetAUPreset (presetDict), home);
			CFRelease (presetDict);
		}
			// this does ALL of the preflighting.. could be specialise for an OfflineAU type
			// to do this piecemeal and do a progress bar by using the OfflineAUPreflight method
		readBuf->lastInputFrames = 0;
		require_noerr (result = processor.Preflight (), home);
	
	float mean;
	
	// now do the processing....
	{
		const int kThrasherSize = 4000000;
		char* thrasher = new char[kThrasherSize];
		
		bool isDone = false;
		
		UInt32 numMeasures = 0;
		Float64 totalMSqrd;
		Float64 totalM;
		
		int i = 0;
		int discardResults = 3;
						
			// this is the render loop
		while (!isDone) 
		{
			bool isSilence, postProcess;

			outputList.Prepare(); // have to do this every time...
			readBuf->lastInputFrames = 0;
			sLastReadTime = 0;
			memset (thrasher, numMeasures, kThrasherSize);

												UInt64 now = CAHostTimeBase::GetTheCurrentTime(); 
			require_noerr (result = processor.Render (outputList.ABL(), numFrames, isSilence, &isDone, &postProcess), home);
												UInt64 renderTime = (CAHostTimeBase::GetTheCurrentTime() - now);
			
			if (i++ < discardResults) continue;
			if (!readBuf->lastInputFrames) break;

			Float64 renderTimeSecs = CAHostTimeBase::ConvertToNanos (renderTime - sLastReadTime) / 1.0e9;
			
			Float64 cpuTime = (renderTimeSecs / (readBuf->lastInputFrames / procFormat.mSampleRate)) * 100.;
			numMeasures++;
			
			totalMSqrd += (cpuTime * cpuTime);
			totalM += cpuTime;

			if (cpuTime > sMaxTime)
				sMaxTime = cpuTime;
			if (cpuTime < sMinTime)
				sMinTime = cpuTime;
									
#if VERBOSE
//			printf ("current measure: %.2f\n", cpuTime);
			if (numMeasures % 5 == 0) {
				Float64 mean = totalM / numMeasures;				
					//	stdDev = (sum of Xsquared -((sum of X)*(sum of X)/N)) / (N-1))
				Float64 stdDev = sqrt ((totalMSqrd - ((totalM * totalM) / numMeasures)) / (numMeasures-1.0));
				printf ("ave: %.2f, min: %.2f, max: %.2f, stdev: %.2f, numMeasures: %ld, current: %f\n", 
					mean, sMinTime, sMaxTime, stdDev, numMeasures, cpuTime);
			}
#endif
		}
		delete [] thrasher;
		
		mean = totalM / numMeasures;
			//	stdDev = (sum of Xsquared -((sum of X)*(sum of X)/N)) / (N-1))
		Float64 stdDev = sqrt ((totalMSqrd - ((totalM * totalM) / numMeasures)) / (numMeasures-1.0));

		printf ("ave: %.2f, min: %.2f, max: %.2f, sd: %.2f, sd / mean: %.2f%%\n", 
			mean, sMinTime, sMaxTime, stdDev, (stdDev / mean * 100.));
	}

		// we don't care about post-processing

home:
		if (result) {
			printf ("Exit with bad result:%ld\n", result);
			exit(result);
		}
		
		if (readBuf) {
			delete readBuf->readData;
			delete readBuf;
		}
		

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
		
		UInt32 groupID = comp.Desc().componentSubType;
		
		char* cstr = (char*)malloc (compNameLen + presetLen + 2 + 1);
		CFStringGetCString (str, cstr, (CFStringGetLength (str) + 1), kCFStringEncodingASCII);
		if (presetName) {
			cstr[compNameLen] = ':';
			cstr[compNameLen+1] = ':';
			CFStringGetCString (presetName, cstr + compNameLen + 2, (CFStringGetLength (presetName) + 1), kCFStringEncodingASCII);
			int len = strlen(cstr);
			for (int i = 0; i < len; ++i)
				groupID += cstr[i];
		}
		PerfResult("AU Profile", EndianU32_NtoB(groupID), cstr, mean, "%realtime");
		free (cstr);

	}
	catch (CAXException &e) {
		char buf[256];
		printf("Error: %s (%s)\n", e.mOperation, e.FormatError(buf));
		exit(1);
	}
	catch (...) {
		printf("An unknown error occurred\n");
		exit(1);
	}
			
	return 0;
}

