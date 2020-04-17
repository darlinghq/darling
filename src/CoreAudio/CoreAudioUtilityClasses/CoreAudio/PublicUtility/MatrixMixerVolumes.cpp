/*
     File: MatrixMixerVolumes.cpp
 Abstract: MatrixMixerVolumes.h
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
#include "MatrixMixerVolumes.h"
#include "CAXException.h"

OSStatus	NumberChannels (AudioUnit 		 	au,
							AudioUnitScope		inScope,
							AudioUnitElement	inEl,
							UInt32				&outChans);


OSStatus PrintBuses (FILE* file, const char* str, AudioUnit au, AudioUnitScope inScope)
{
	OSStatus result;
	UInt32 busCount;
	UInt32 theSize = sizeof(busCount);

	ca_require_noerr (result = AudioUnitGetProperty (au, kAudioUnitProperty_ElementCount,	
							inScope, 0, &busCount, &theSize), home);
		
	fprintf (file, "\t%s Elements:\n\t\t", str);
	for (UInt32 i = 0; i < busCount; ++i) {
		Float32 val;
		ca_require_noerr (result = AudioUnitGetParameter (au, kMatrixMixerParam_Enable, inScope, i, &val), home);
		UInt32 numChans;
		ca_require_noerr (result = NumberChannels (au, inScope, i, numChans), home);
		char frameCharStart = (val != 0 ? '[' : '{');
		char frameCharEnd = (val != 0 ? ']' : '}');
		fprintf (file, "%d:%c%d, %c%c  ", (int)i, frameCharStart, (int)numChans, (val != 0 ? 'T' : 'F'), frameCharEnd);
	}
	fprintf (file, "\n");
home:
	return result;
}

void	PrintMatrixMixerVolumes (FILE* file, AudioUnit au)
{
	UInt32 dims[2];
	UInt32 theSize =  sizeof(UInt32) * 2;
	Float32 *theVols = NULL;
	OSStatus result;
	
// this call will fail if the unit is NOT initialized as it would present an incomplete state	
	ca_require_noerr (result = AudioUnitGetProperty (au, kAudioUnitProperty_MatrixDimensions,	
							kAudioUnitScope_Global, 0, dims, &theSize), home);

	theSize = ((dims[0] + 1) * (dims[1] + 1)) * sizeof(Float32);
	
	theVols	= static_cast<Float32*> (malloc (theSize));
	
	ca_require_noerr (result = AudioUnitGetProperty (au, kAudioUnitProperty_MatrixLevels,	
							kAudioUnitScope_Global, 0, theVols, &theSize), home);

home:
	if (result) {
		if (theVols)
			free(theVols);
		return;
	}
	
	theSize /= sizeof(Float32);
	
	unsigned int inputs = dims[0];
	unsigned int outputs = dims[1];

	fprintf (file, "\tInput Channels = %d, Output Channels = %d\n", (int)dims[0], (int)dims[1]);
	PrintBuses (file, "Input", au, kAudioUnitScope_Input);
	PrintBuses (file, "Output", au, kAudioUnitScope_Output);
	fprintf (file, "\tGlobal Volume: %.3f\n", theVols [theSize - 1]);
	for (unsigned int i = 0; i < (inputs + 1); ++i) {
		if (i < inputs) {
			fprintf (file, "\t%.3f   ", theVols[(i + 1) * (outputs + 1) - 1]);
			
			for (unsigned int j = 0; j < outputs; ++j)
				fprintf (file, "(%.3f) ", theVols[(i * (outputs  + 1)) + j]);
		} else {
			fprintf (file, "\t        ");
			for (unsigned int j = 0; j < outputs; ++j)
				fprintf (file, " %.3f  ", theVols[(i * (outputs + 1)) + j]);
		}
		fprintf (file, "\n");
	}

#if 0
	for (unsigned int i = 0; i < theSize; ++i)
		printf ("%f, ", theVols[i]);
#endif
	free(theVols);
}

// Utility routine that gets the number of channels from an audio unit
OSStatus	NumberChannels (AudioUnit 		 	au,
							AudioUnitScope		inScope,
							AudioUnitElement	inEl,
							UInt32				&outChans)
{
	AudioStreamBasicDescription desc;
	UInt32 dataSize = sizeof (AudioStreamBasicDescription);
	OSStatus result = AudioUnitGetProperty (au, kAudioUnitProperty_StreamFormat,
								inScope, inEl, 
								&desc, &dataSize);
	if (!result)
		outChans = desc.mChannelsPerFrame;
	return result;
}
