/*
 * Copyright (c) 2000-2014 by Apple Inc.. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
 
/*
     File:       TargetConditionals.h
 
     Contains:   Autoconfiguration of TARGET_ conditionals for Mac OS X and iPhone
     
                 Note:  TargetConditionals.h in 3.4 Universal Interfaces works
                        with all compilers.  This header only recognizes compilers
                        known to run on Mac OS X.
  
*/

#ifndef __TARGETCONDITIONALS__
#define __TARGETCONDITIONALS__
/****************************************************************************************************

    TARGET_CPU_*    
    These conditionals specify which microprocessor instruction set is being
    generated.  At most one of these is true, the rest are false.

        TARGET_CPU_PPC          - Compiler is generating PowerPC instructions for 32-bit mode
        TARGET_CPU_PPC64        - Compiler is generating PowerPC instructions for 64-bit mode
        TARGET_CPU_68K          - Compiler is generating 680x0 instructions
        TARGET_CPU_X86          - Compiler is generating x86 instructions
        TARGET_CPU_ARM          - Compiler is generating ARM instructions
        TARGET_CPU_MIPS         - Compiler is generating MIPS instructions
        TARGET_CPU_SPARC        - Compiler is generating Sparc instructions
        TARGET_CPU_ALPHA        - Compiler is generating Dec Alpha instructions


    TARGET_OS_* 
    These conditionals specify in which Operating System the generated code will
    run. The MAC/WIN32/UNIX conditionals are mutually exclusive.  The EMBEDDED/IPHONE 
	conditionals are variants of TARGET_OS_MAC. 

        TARGET_OS_MAC           - Generate code will run under Mac OS
        TARGET_OS_WIN32         - Generate code will run under 32-bit Windows
        TARGET_OS_UNIX          - Generate code will run under some non Mac OS X unix 
        TARGET_OS_EMBEDDED      - Generate code will run under an embedded OS variant
                                  of TARGET_OS_MAC
        TARGET_OS_IPHONE        - Generate code will run under iPhone OS which 
                                  is a variant of TARGET_OS_MAC.

    TARGET_RT_* 
    These conditionals specify in which runtime the generated code will
    run. This is needed when the OS and CPU support more than one runtime
    (e.g. Mac OS X supports CFM and mach-o).

        TARGET_RT_LITTLE_ENDIAN - Generated code uses little endian format for integers
        TARGET_RT_BIG_ENDIAN    - Generated code uses big endian format for integers    
        TARGET_RT_64_BIT        - Generated code uses 64-bit pointers    
        TARGET_RT_MAC_CFM       - TARGET_OS_MAC is true and CFM68K or PowerPC CFM (TVectors) are used
        TARGET_RT_MAC_MACHO     - TARGET_OS_MAC is true and Mach-O/dlyd runtime is used


    TARGET_IPHONE_SIMULATOR     - Generate code for running under iPhone Simulator
        

****************************************************************************************************/


/*
 *    gcc based compiler used on Mac OS X
 */
#if defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )
    #define TARGET_OS_MAC               1
    #define TARGET_OS_WIN32             0
    #define TARGET_OS_UNIX              0
    #define TARGET_OS_EMBEDDED          0 
    #define TARGET_OS_IPHONE            0 
    #define TARGET_IPHONE_SIMULATOR     0 
    #if defined(__ppc__) 
        #define TARGET_CPU_PPC          1
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          0
        #define TARGET_CPU_X86_64       0
        #define TARGET_CPU_ARM          0
        #define TARGET_CPU_ARM64        0
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0   
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_LITTLE_ENDIAN 0
        #define TARGET_RT_BIG_ENDIAN    1
        #define TARGET_RT_64_BIT        0
        #ifdef __MACOS_CLASSIC__
           #define TARGET_RT_MAC_CFM    1
           #define TARGET_RT_MAC_MACHO  0
        #else
           #define TARGET_RT_MAC_CFM    0
           #define TARGET_RT_MAC_MACHO  1
       #endif
    #elif defined(__ppc64__) 
        #define TARGET_CPU_PPC          0
        #define TARGET_CPU_PPC64        1
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          0
        #define TARGET_CPU_X86_64       0
        #define TARGET_CPU_ARM          0
        #define TARGET_CPU_ARM64        0
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0   
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_LITTLE_ENDIAN 0
        #define TARGET_RT_BIG_ENDIAN    1
        #define TARGET_RT_64_BIT        1
        #define TARGET_RT_MAC_CFM       0
        #define TARGET_RT_MAC_MACHO     1
    #elif defined(__i386__) 
        #define TARGET_CPU_PPC          0
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          1
        #define TARGET_CPU_X86_64       0
        #define TARGET_CPU_ARM          0
        #define TARGET_CPU_ARM64        0
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_MAC_CFM       0
        #define TARGET_RT_MAC_MACHO     1
        #define TARGET_RT_LITTLE_ENDIAN 1
        #define TARGET_RT_BIG_ENDIAN    0
        #define TARGET_RT_64_BIT        0
    #elif defined(__x86_64__) 
        #define TARGET_CPU_PPC          0
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          0
        #define TARGET_CPU_X86_64       1
        #define TARGET_CPU_ARM          0
        #define TARGET_CPU_ARM64        0
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_MAC_CFM       0
        #define TARGET_RT_MAC_MACHO     1
        #define TARGET_RT_LITTLE_ENDIAN 1
        #define TARGET_RT_BIG_ENDIAN    0
        #define TARGET_RT_64_BIT        1
    #elif defined(__arm__) 
        #define TARGET_CPU_PPC          0
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          0
        #define TARGET_CPU_X86_64       0
        #define TARGET_CPU_ARM          1
        #define TARGET_CPU_ARM64        0
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_MAC_CFM       0
        #define TARGET_RT_MAC_MACHO     1
        #define TARGET_RT_LITTLE_ENDIAN 1
        #define TARGET_RT_BIG_ENDIAN    0
        #define TARGET_RT_64_BIT        0
    #elif defined(__arm64__)
        #define TARGET_CPU_PPC          0
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          0
        #define TARGET_CPU_X86_64       0
        #define TARGET_CPU_ARM          0
        #define TARGET_CPU_ARM64        1
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_MAC_CFM       0
        #define TARGET_RT_MAC_MACHO     1
        #define TARGET_RT_LITTLE_ENDIAN 1
        #define TARGET_RT_BIG_ENDIAN    0
        #define TARGET_RT_64_BIT        1
    #else
        #error unrecognized GNU C compiler
    #endif



/*
 *   CodeWarrior compiler from Metrowerks/Motorola
 */
#elif defined(__MWERKS__)
    #define TARGET_OS_MAC               1
    #define TARGET_OS_WIN32             0
    #define TARGET_OS_UNIX              0
    #define TARGET_OS_EMBEDDED          0
    #if defined(__POWERPC__)
        #define TARGET_CPU_PPC          1
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          0
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_LITTLE_ENDIAN 0
        #define TARGET_RT_BIG_ENDIAN    1
    #elif defined(__INTEL__)
        #define TARGET_CPU_PPC          0
        #define TARGET_CPU_PPC64        0
        #define TARGET_CPU_68K          0
        #define TARGET_CPU_X86          1
        #define TARGET_CPU_MIPS         0
        #define TARGET_CPU_SPARC        0
        #define TARGET_CPU_ALPHA        0
        #define TARGET_RT_LITTLE_ENDIAN 1
        #define TARGET_RT_BIG_ENDIAN    0
    #else
        #error unknown Metrowerks CPU type
    #endif
    #define TARGET_RT_64_BIT            0
    #ifdef __MACH__
        #define TARGET_RT_MAC_CFM       0
        #define TARGET_RT_MAC_MACHO     1
    #else
        #define TARGET_RT_MAC_CFM       1
        #define TARGET_RT_MAC_MACHO     0
    #endif

/*
 *   unknown compiler
 */
#else
    #if defined(TARGET_CPU_PPC) && TARGET_CPU_PPC
        #define TARGET_CPU_PPC64    0
        #define TARGET_CPU_68K      0
        #define TARGET_CPU_X86      0
        #define TARGET_CPU_X86_64   0
        #define TARGET_CPU_ARM      0
        #define TARGET_CPU_ARM64    0
        #define TARGET_CPU_MIPS     0
        #define TARGET_CPU_SPARC    0
        #define TARGET_CPU_ALPHA    0
    #elif defined(TARGET_CPU_PPC64) && TARGET_CPU_PPC64
        #define TARGET_CPU_PPC      0
        #define TARGET_CPU_68K      0
        #define TARGET_CPU_X86      0
        #define TARGET_CPU_X86_64   0
        #define TARGET_CPU_ARM      0
        #define TARGET_CPU_ARM64    0
        #define TARGET_CPU_MIPS     0
        #define TARGET_CPU_SPARC    0
        #define TARGET_CPU_ALPHA    0
    #elif defined(TARGET_CPU_X86) && TARGET_CPU_X86
        #define TARGET_CPU_PPC      0
        #define TARGET_CPU_PPC64    0
        #define TARGET_CPU_X86_64   0
        #define TARGET_CPU_68K      0
        #define TARGET_CPU_ARM      0
        #define TARGET_CPU_ARM64    0
        #define TARGET_CPU_MIPS     0
        #define TARGET_CPU_SPARC    0
        #define TARGET_CPU_ALPHA    0
    #elif defined(TARGET_CPU_X86_64) && TARGET_CPU_X86_64
        #define TARGET_CPU_PPC      0
        #define TARGET_CPU_PPC64    0
        #define TARGET_CPU_X86      0
        #define TARGET_CPU_68K      0
        #define TARGET_CPU_ARM      0
        #define TARGET_CPU_ARM64    0
        #define TARGET_CPU_MIPS     0
        #define TARGET_CPU_SPARC    0
        #define TARGET_CPU_ALPHA    0
    #elif defined(TARGET_CPU_ARM) && TARGET_CPU_ARM
        #define TARGET_CPU_PPC      0
        #define TARGET_CPU_PPC64    0
        #define TARGET_CPU_X86      0
        #define TARGET_CPU_X86_64   0
        #define TARGET_CPU_68K      0
        #define TARGET_CPU_ARM64    0
        #define TARGET_CPU_MIPS     0
        #define TARGET_CPU_SPARC    0
        #define TARGET_CPU_ALPHA    0
    #elif defined(TARGET_CPU_ARM64) && TARGET_CPU_ARM64
        #define TARGET_CPU_PPC      0
        #define TARGET_CPU_PPC64    0
        #define TARGET_CPU_X86      0
        #define TARGET_CPU_X86_64   0
        #define TARGET_CPU_68K      0
        #define TARGET_CPU_ARM      0
        #define TARGET_CPU_MIPS     0
        #define TARGET_CPU_SPARC    0
        #define TARGET_CPU_ALPHA    0
    #else
        /*
            NOTE:   If your compiler errors out here then support for your compiler 
            has not yet been added to TargetConditionals.h.  
            
            TargetConditionals.h is designed to be plug-and-play.  It auto detects
            which compiler is being run and configures the TARGET_ conditionals
            appropriately.  
            
            The short term work around is to set the TARGET_CPU_ and TARGET_OS_
            on the command line to the compiler (e.g. -DTARGET_CPU_MIPS=1 -DTARGET_OS_UNIX=1)
            
            The long term solution is to add a new case to this file which
            auto detects your compiler and sets up the TARGET_ conditionals.
            Then submit the changes to Apple Computer.
        */
        #error TargetConditionals.h: unknown compiler (see comment above)
        #define TARGET_CPU_PPC    0
        #define TARGET_CPU_68K    0
        #define TARGET_CPU_X86    0
        #define TARGET_CPU_ARM    0
        #define TARGET_CPU_ARM64  0
        #define TARGET_CPU_MIPS   0
        #define TARGET_CPU_SPARC  0
        #define TARGET_CPU_ALPHA  0
    #endif
    #define TARGET_OS_MAC                1
    #define TARGET_OS_WIN32              0
    #define TARGET_OS_UNIX               0
    #define TARGET_OS_EMBEDDED           0
    #if TARGET_CPU_PPC || TARGET_CPU_PPC64
        #define TARGET_RT_BIG_ENDIAN     1
        #define TARGET_RT_LITTLE_ENDIAN  0
    #else
        #define TARGET_RT_BIG_ENDIAN     0
        #define TARGET_RT_LITTLE_ENDIAN  1
    #endif
    #if TARGET_CPU_PPC64 || TARGET_CPU_X86_64
        #define TARGET_RT_64_BIT         1
    #else
        #define TARGET_RT_64_BIT         0
    #endif
    #ifdef __MACH__
        #define TARGET_RT_MAC_MACHO      1
        #define TARGET_RT_MAC_CFM        0
    #else
        #define TARGET_RT_MAC_MACHO      0
        #define TARGET_RT_MAC_CFM        1
    #endif
    
#endif

#endif  /* __TARGETCONDITIONALS__ */
