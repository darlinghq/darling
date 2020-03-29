/*
     File: CAVectorUnit.cpp
 Abstract: CAVectorUnit.h
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
#include "CAVectorUnit.h"

#if !TARGET_OS_WIN32
	#include <sys/sysctl.h>
#elif HAS_IPP
	#include "ippdefs.h"
	#include "ippcore.h"
#endif

int gCAVectorUnitType = kVecUninitialized;

#if TARGET_OS_WIN32
// Use cpuid to check if SSE2 is available.
// Before calling this function make sure cpuid is available
static SInt32 IsSSE2Available()
{
	int return_value;

	{
		int r_edx;
		_asm
		{
			mov eax, 0x01
			cpuid
			mov r_edx, edx
		}
		return_value = (r_edx >> 26) & 0x1;
	}
	return return_value;
}

// Use cpuid to check if SSE3 is available.
// Before calling this function make sure cpuid is available
static SInt32 IsSSE3Available()
{
	SInt32 return_value;

	{
		SInt32 r_ecx;
		_asm
		{
			mov eax, 0x01
			cpuid
			mov r_ecx, ecx
		}
		return_value = r_ecx & 0x1;
	}
	return return_value;
}

// Return true if the cpuid instruction is available.
// The cpuid instruction is available if bit 21 in the EFLAGS register can be changed
// This function may not work on Intel CPUs prior to Pentium (didn't test)
static bool IsCpuidAvailable()
{
	SInt32 return_value = 0x0;
	_asm{
		pushfd    ;			//push original EFLAGS 
		pop eax   ;			//get original EFLAGS 
		mov ecx, eax   ;	//save original EFLAGS 
		xor eax, 200000h  ; //flip ID bit in EFLAGS 
		push eax   ;		//save new EFLAGS value on stack 
		popfd    ;			//replace current EFLAGS value 
		pushfd    ;			//get new EFLAGS 
		pop eax   ;			//store new EFLAGS in EAX 
		xor eax, ecx   ;	 
		je end_cpuid_identify  ; //can't toggle ID bit
		mov return_value, 0x1;	
end_cpuid_identify:
		nop;
		}
		return return_value;
}

#endif

SInt32	CAVectorUnit_Examine()
{
	int result = kVecNone;
	
#if TARGET_OS_WIN32
#if HAS_IPP	
	// Initialize the static IPP library! This needs to be done before
	// any IPP function calls, otherwise we may have a performance penalty
	int status = ippStaticInit();
	if ( status == ippStsNonIntelCpu )
	{
		IppCpuType cpuType = ippGetCpuType();
		if ( cpuType >= ippCpuSSE || cpuType <= ippCpuSSE42 )
			ippStaticInitCpu( cpuType );
	}
#endif
	{
		// On Windows we use cpuid to detect the vector unit because it works on Intel and AMD.
		// The IPP library does not detect SSE on AMD processors.
		if (IsCpuidAvailable())
		{
			if(IsSSE3Available())
			{
				result = kVecSSE3;
			}
			else if(IsSSE2Available())
			{
				result = kVecSSE2;
			}
		}
	}
#elif TARGET_OS_MAC
#if DEBUG
	if (getenv("CA_NoVector")) {
		fprintf(stderr, "CA_NoVector set; Vector unit optimized routines will be bypassed\n");
		return result;
	} 
	else
#endif
	{
	#if (TARGET_CPU_PPC || TARGET_CPU_PPC64)
		int sels[2] = { CTL_HW, HW_VECTORUNIT };
		int vType = 0; //0 == scalar only
		size_t length = sizeof(vType);
		int error = sysctl(sels, 2, &vType, &length, NULL, 0);
		if (!error && vType > 0)
			result = kVecAltivec;
	#elif (TARGET_CPU_X86 || TARGET_CPU_X86_64)
		static const struct { const char* kName; const int kVectype; } kStringVectypes[] = {
			{ "hw.optional.avx1_0", kVecAVX1 }, { "hw.optional.sse3", kVecSSE3 }, { "hw.optional.sse2", kVecSSE2 }
		};
		static const size_t kNumStringVectypes = sizeof(kStringVectypes)/sizeof(kStringVectypes[0]);
		int i = 0, answer = 0;
		while(i != kNumStringVectypes)
		{
			size_t length = sizeof(answer);
			int error = sysctlbyname(kStringVectypes[i].kName, &answer, &length, NULL, 0);
			if (!error && answer)
			{
				result = kStringVectypes[i].kVectype;
				break;
			}
			++i;
		};
	#elif CA_ARM_NEON
		result = kVecNeon;
	#endif
	}
#endif
	gCAVectorUnitType = result;
	return result;
}

