/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1993 NeXT Computer, Inc.
 *
 * Architecture computing functions.
 *
 * HISTORY
 *
 * 11 April 1997
 *	Update m98k to ppc and removed the never supported architectures (mips,
 *	and vax).  Apple Computer, Inc.
 *
 * 4 February 1993 Lennart Lovstrand <lennart@next.com>
 *	Redesigned to use NXArchInfo based names and signatures.
 *
 * Originally written at NeXT, Inc.
 *
 */
#ifndef RLD
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "mach/machine.h"
#include "mach/mach.h"
#include "stuff/openstep_mach.h"
#include <mach-o/fat.h>
#include <mach-o/arch.h>

/* The array of all currently know architecture flags (terminated with an entry
 * with all zeros).  Pointer to this returned with NXGetAllArchInfos().
 */
static const NXArchInfo ArchInfoTable[] = {
    /* architecture families */
    {"hppa",   CPU_TYPE_HPPA,	 CPU_SUBTYPE_HPPA_ALL,	   NX_BigEndian,
	 "HP-PA"},
    {"i386",   CPU_TYPE_I386,    CPU_SUBTYPE_I386_ALL,	   NX_LittleEndian,
	 "Intel 80x86"},
    { "x86_64",    CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL, NX_LittleEndian,
	 "Intel x86-64" },
    { "x86_64h",   CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H,  NX_LittleEndian,
	 "Intel x86-64h Haswell" },
    {"i860",   CPU_TYPE_I860,    CPU_SUBTYPE_I860_ALL,     NX_BigEndian,
	 "Intel 860"},
    {"m68k",   CPU_TYPE_MC680x0, CPU_SUBTYPE_MC680x0_ALL,  NX_BigEndian,
	 "Motorola 68K"},
    {"m88k",   CPU_TYPE_MC88000, CPU_SUBTYPE_MC88000_ALL,  NX_BigEndian,
	 "Motorola 88K"},
    {"ppc",    CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_ALL,  NX_BigEndian,
	 "PowerPC"},
    {"ppc64",  CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_ALL,  NX_BigEndian,
	 "PowerPC 64-bit"},
    {"sparc",  CPU_TYPE_SPARC,   CPU_SUBTYPE_SPARC_ALL,	   NX_BigEndian,
	 "SPARC"},
    {"arm",    CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_ALL,	   NX_LittleEndian,
	 "ARM"},
    {"arm64",  CPU_TYPE_ARM64,   CPU_SUBTYPE_ARM64_ALL,	   NX_LittleEndian,
	 "ARM64"},
    {"any",    CPU_TYPE_ANY,     CPU_SUBTYPE_MULTIPLE,     NX_UnknownByteOrder,
	 "Architecture Independent"},
#ifndef DARLING
    {"veo",    CPU_TYPE_VEO,	 CPU_SUBTYPE_VEO_ALL,  	   NX_BigEndian,
	 "veo"},
#endif
    /* specific architecture implementations */
    {"hppa7100LC", CPU_TYPE_HPPA, CPU_SUBTYPE_HPPA_7100LC, NX_BigEndian,
	 "HP-PA 7100LC"},
    {"m68030", CPU_TYPE_MC680x0, CPU_SUBTYPE_MC68030_ONLY, NX_BigEndian,
	 "Motorola 68030"},
    {"m68040", CPU_TYPE_MC680x0, CPU_SUBTYPE_MC68040,	   NX_BigEndian,
	 "Motorola 68040"},
    {"i486",   CPU_TYPE_I386,    CPU_SUBTYPE_486,	   NX_LittleEndian,
	 "Intel 80486"},
    {"i486SX", CPU_TYPE_I386,    CPU_SUBTYPE_486SX,	   NX_LittleEndian,
	 "Intel 80486SX"},
    {"pentium",CPU_TYPE_I386,    CPU_SUBTYPE_PENT,	   NX_LittleEndian,
	 "Intel Pentium"}, /* same as 586 */
    {"i586",   CPU_TYPE_I386,    CPU_SUBTYPE_586,	   NX_LittleEndian,
	 "Intel 80586"},
    {"pentpro", CPU_TYPE_I386, CPU_SUBTYPE_PENTPRO,	   NX_LittleEndian,
	 "Intel Pentium Pro"}, /* same as 686 */
    {"i686",    CPU_TYPE_I386, CPU_SUBTYPE_PENTPRO,	   NX_LittleEndian,
	 "Intel Pentium Pro"},
    {"pentIIm3", CPU_TYPE_I386, CPU_SUBTYPE_PENTII_M3, NX_LittleEndian,
	 "Intel Pentium II Model 3" },
    {"pentIIm5", CPU_TYPE_I386, CPU_SUBTYPE_PENTII_M5, NX_LittleEndian,
	 "Intel Pentium II Model 5" },
    {"pentium4", CPU_TYPE_I386, CPU_SUBTYPE_PENTIUM_4, NX_LittleEndian,
	 "Intel Pentium 4" },
    { "x86_64h", CPU_TYPE_I386, CPU_SUBTYPE_X86_64_H,  NX_LittleEndian,
	 "Intel x86-64h Haswell" },
    {"ppc601", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_601,  NX_BigEndian,
	 "PowerPC 601" },
    {"ppc603", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_603,  NX_BigEndian,
	 "PowerPC 603" },
    {"ppc603e",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_603e, NX_BigEndian,
	 "PowerPC 603e" },
    {"ppc603ev",CPU_TYPE_POWERPC,CPU_SUBTYPE_POWERPC_603ev,NX_BigEndian,
	 "PowerPC 603ev" },
    {"ppc604", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_604,  NX_BigEndian,
	 "PowerPC 604" },
    {"ppc604e",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_604e, NX_BigEndian,
	 "PowerPC 604e" },
    {"ppc750", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_750,  NX_BigEndian,
	 "PowerPC 750" },
    {"ppc7400",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_7400,  NX_BigEndian,
	 "PowerPC 7400" },
    {"ppc7450",CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_7450,  NX_BigEndian,
	 "PowerPC 7450" },
    {"ppc970", CPU_TYPE_POWERPC, CPU_SUBTYPE_POWERPC_970,  NX_BigEndian,
	 "PowerPC 970" },
    {"ppc970-64",  CPU_TYPE_POWERPC64, CPU_SUBTYPE_POWERPC_970,  NX_BigEndian,
	 "PowerPC 970 64-bit"},
    {"armv4t", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V4T,	   NX_LittleEndian,
	 "arm v4t"},
    {"armv5",  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V5TEJ,	   NX_LittleEndian,
	 "arm v5"},
    {"xscale", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_XSCALE,   NX_LittleEndian,
	 "arm xscale"},
    {"armv6",  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V6,	   NX_LittleEndian,
	 "arm v6"},
    {"armv6m", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V6M,	   NX_LittleEndian,
	 "arm v6m"},
    {"armv7",  CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7,	   NX_LittleEndian,
	 "arm v7"},
    {"armv7f", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7F,	   NX_LittleEndian,
	 "arm v7f"},
    {"armv7s", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7S,	   NX_LittleEndian,
	 "arm v7s"},
    {"armv7k", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7K,	   NX_LittleEndian,
	 "arm v7k"},
    {"armv7m", CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7M,	   NX_LittleEndian,
	 "arm v7m"},
    {"armv7em",CPU_TYPE_ARM,     CPU_SUBTYPE_ARM_V7EM,	   NX_LittleEndian,
	 "arm v7em"},
    {"armv8", CPU_TYPE_ARM,      CPU_SUBTYPE_ARM_V8,	   NX_LittleEndian,
	 "arm v8"},
    {"arm64",CPU_TYPE_ARM64,   CPU_SUBTYPE_ARM64_V8,	   NX_LittleEndian,
	 "arm64 v8"},
    {"little", CPU_TYPE_ANY,     CPU_SUBTYPE_LITTLE_ENDIAN, NX_LittleEndian,
         "Little Endian"},
    {"big",    CPU_TYPE_ANY,     CPU_SUBTYPE_BIG_ENDIAN,   NX_BigEndian,
         "Big Endian"},
#ifndef DARLING
    {"veo1",CPU_TYPE_VEO,	 CPU_SUBTYPE_VEO_1,	   NX_BigEndian,
	 "veo 1" },
    {"veo2",CPU_TYPE_VEO,	 CPU_SUBTYPE_VEO_2,	   NX_BigEndian,
	 "veo 2" },
#endif
    {NULL,     0,		  0,			   0,
	 NULL}
};

/*
 * NXGetAllArchInfos() returns a pointer to an array of all currently know
 * architecture flags (terminated with an entry with all zeros).
 */
const
NXArchInfo *
NXGetAllArchInfos(void)
{
	return(ArchInfoTable);
}

/*
 * NXGetLocalArchInfo() returns the NXArchInfo matching the cputype and
 * cpusubtype of the local host.  NULL is returned if there is no matching
 * entry in the ArchInfoTable.
 */
const
NXArchInfo *
NXGetLocalArchInfo(void)
{
    struct host_basic_info hbi;
    kern_return_t ret;
    unsigned int count;
    mach_port_t my_mach_host_self;

	count = HOST_BASIC_INFO_COUNT;
	my_mach_host_self = mach_host_self();
	ret = host_info(my_mach_host_self, HOST_BASIC_INFO, (host_info_t)&hbi,
			&count);
	mach_port_deallocate(mach_task_self(), my_mach_host_self);
	if(ret != KERN_SUCCESS)
	    return(NULL);

	/*
	 * There is a "bug" in the kernel for compatiblity that on
	 * an 030 machine host_info() returns cpusubtype
	 * CPU_SUBTYPE_MC680x0_ALL and not CPU_SUBTYPE_MC68030_ONLY.
	 */
	if(hbi.cpu_type == CPU_TYPE_MC680x0 &&
	   hbi.cpu_subtype == CPU_SUBTYPE_MC680x0_ALL)
	    hbi.cpu_subtype = CPU_SUBTYPE_MC68030_ONLY;

	return(NXGetArchInfoFromCpuType(hbi.cpu_type, hbi.cpu_subtype));
}

/*
 * NXGetArchInfoFromName() is passed an architecture name (like "m68k")
 * and returns the matching NXArchInfo struct, or NULL if none is found.
 */
const
NXArchInfo *
NXGetArchInfoFromName(
const char *name)
{
    const NXArchInfo *ai;

	for(ai = ArchInfoTable; ai->name != NULL; ai++)
	    if(strcmp(ai->name, name) == 0)
		return(ai);

	return(NULL);
}

/*
 * NXGetArchInfoFromName() is passed a cputype and cpusubtype and returns
 * the matching NXArchInfo struct, or NULL if none is found.  If the
 * cpusubtype is given as CPU_SUBTYPE_MULTIPLE, the first entry that
 * matches the given cputype is returned.  This is the NXArchInfo struct
 * describing the CPU "family".
 */
const
NXArchInfo *
NXGetArchInfoFromCpuType(
cpu_type_t cputype,
cpu_subtype_t cpusubtype)
{
    const NXArchInfo *ai;
    NXArchInfo *q;

    for(ai = ArchInfoTable; ai->name != NULL; ai++)
	if(ai->cputype == cputype &&
	   (cpusubtype == CPU_SUBTYPE_MULTIPLE ||
	    ((ai->cpusubtype & ~CPU_SUBTYPE_MASK) ==
	     (cpusubtype & ~CPU_SUBTYPE_MASK))))
	    return(ai);

    if(cputype == CPU_TYPE_I386){
	q = malloc(sizeof(NXArchInfo));
	for(ai = ArchInfoTable; ai->name != NULL; ai++){
	    if(ai->cputype == cputype){
		*q = *ai;
		break;
	    }
	}
	q->cpusubtype = cpusubtype;
	q->description = malloc(sizeof("Intel family  model ") + 2 + 8);
	if(q->description == NULL){
	    free(q);
	    return(NULL);
	}
	sprintf((char *)q->description, "Intel family %u model %u", 
		CPU_SUBTYPE_INTEL_FAMILY(cpusubtype & ~CPU_SUBTYPE_MASK),
		CPU_SUBTYPE_INTEL_MODEL(cpusubtype & ~CPU_SUBTYPE_MASK));
	return((const NXArchInfo *)q);
    }
    else if(cputype == CPU_TYPE_POWERPC){
	q = malloc(sizeof(NXArchInfo));
	for(ai = ArchInfoTable; ai->name != NULL; ai++){
	    if(ai->cputype == cputype){
		*q = *ai;
		break;
	    }
	}
	q->cpusubtype = cpusubtype;
	q->description = malloc(sizeof("PowerPC cpusubtype ") + 10);
	if(q->description == NULL){
	    free(q);
	    return(NULL);
	}
	sprintf((char *)q->description, "PowerPC cpusubtype %u", cpusubtype);
	return((const NXArchInfo *)q);
    }

    return(NULL);
}

/*
 * internal_NXFindBestFatArch() is passed a cputype and cpusubtype and a
 * either set of fat_arch structs or fat_arch_64 structs and selects the best
 * one that matches (if any) and returns an index to the array of structs or
 * -1 if none works for the cputype and cpusubtype.  The fat_arch structs or
 * fat_arch_64 structs must be in the host byte sex and correct such that the
 * fat_archs really points to enough memory for nfat_arch structs.  It is
 * possible that this routine could fail if new cputypes or cpusubtypes are
 * added and an old version of this routine is used.  But if there is an exact
 * match between the cputype and cpusubtype and one of the structs this routine
 * will always succeed.
 */
int32_t
internal_NXFindBestFatArch(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct fat_arch *fat_archs,
struct fat_arch_64 *fat_archs64,
uint32_t nfat_archs)
{
    uint32_t i;
    int32_t lowest_family, lowest_model, lowest_index;
    cpu_type_t fat_cputype;
    cpu_subtype_t fat_cpusubtype;

    /*
     * Look for the first exact match.
     */
    for(i = 0; i < nfat_archs; i++){
	if(fat_archs64 != NULL){
	    fat_cputype = fat_archs64[i].cputype;
	    fat_cpusubtype = fat_archs64[i].cpusubtype;
	}
	else{
	    fat_cputype = fat_archs[i].cputype;
	    fat_cpusubtype = fat_archs[i].cpusubtype;
	}
	if(fat_cputype == cputype &&
		(fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
		(cpusubtype & ~CPU_SUBTYPE_MASK))
	    return(i);
    }

    /*
     * An exact match was not found so find the next best match which is
     * cputype dependent.
     */
    switch(cputype){
	case CPU_TYPE_I386:
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		default:
		    /*
		     * Intel cpusubtypes after the pentium (same as 586) are handled
		     * such that they require an exact match or they can use the
		     * pentium.  If that is not found call into the loop for the
		     * earilier subtypes.
		     */
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_PENT)
			    return(i);
		    }
		case CPU_SUBTYPE_PENT:
		case CPU_SUBTYPE_486SX:
		    /*
		     * Since an exact match as not found look for the i486 else
		     * break into the loop to look for the i386_ALL.
		     */
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_486)
			    return(i);
		    }
		    break;
		case CPU_SUBTYPE_I386_ALL:
		    /* case CPU_SUBTYPE_I386: same as above */
		case CPU_SUBTYPE_486:
		    break;
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_I386_ALL)
		    return(i);
	    }

	    /*
	     * A match failed, promote as little as possible.
	     */
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_486)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_486SX)
		    return(i);
	    }
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_586)
		    return(i);
	    }
	    /*
	     * Now look for the lowest family and in that the lowest model.
	     */
	    lowest_family = CPU_SUBTYPE_INTEL_FAMILY_MAX + 1;
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if(CPU_SUBTYPE_INTEL_FAMILY(fat_cpusubtype &
			    ~CPU_SUBTYPE_MASK) < lowest_family)
		    lowest_family = CPU_SUBTYPE_INTEL_FAMILY(
			    fat_cpusubtype & ~CPU_SUBTYPE_MASK);
	    }
	    /* if no intel cputypes found return NULL */
	    if(lowest_family == CPU_SUBTYPE_INTEL_FAMILY_MAX + 1)
		return(-1);
	    lowest_model = INT_MAX;
	    lowest_index = -1;
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if(CPU_SUBTYPE_INTEL_FAMILY(fat_cpusubtype &
			    ~CPU_SUBTYPE_MASK) == lowest_family){
		    if(CPU_SUBTYPE_INTEL_MODEL(fat_cpusubtype &
				~CPU_SUBTYPE_MASK) < lowest_model){
			lowest_model = CPU_SUBTYPE_INTEL_MODEL(
				fat_cpusubtype &
				~CPU_SUBTYPE_MASK);
			lowest_index = i;
		    }
		}
	    }
	    return(lowest_index);
	case CPU_TYPE_X86_64:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_X86_64_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_MC680x0:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_MC680x0_ALL)
		    return(i);
	    }
	    /*
	     * Try to promote if starting from CPU_SUBTYPE_MC680x0_ALL and
	     * favor the CPU_SUBTYPE_MC68040 over the CPU_SUBTYPE_MC68030_ONLY.
	     */
	    if((cpusubtype & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC680x0_ALL){
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    CPU_SUBTYPE_MC68040)
			return(i);
		}
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    CPU_SUBTYPE_MC68030_ONLY)
			return(i);
		}
	    }
	    break;
	case CPU_TYPE_POWERPC:
	    /*
	     * An exact match as not found.  So for all the PowerPC subtypes
	     * pick the subtype from the following order starting from a subtype
	     * that will work (contains 64-bit instructions or altivec if
	     * needed):
	     *	970, 7450, 7400, 750, 604e, 604, 603ev, 603e, 603, ALL
	     * Note the 601 is NOT in the list above.  It is only picked via
	     * an exact match.  For an unknown subtype pick only the ALL type if
	     * it exists.
	     */
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_ALL:
		    /*
		     * The CPU_SUBTYPE_POWERPC_ALL is only used by the development
		     * environment tools when building a generic ALL type binary.
		     * In the case of a non-exact match we pick the most current
		     * processor.
		     */
		case CPU_SUBTYPE_POWERPC_970:
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_970)
			    return(i);
		    }
		case CPU_SUBTYPE_POWERPC_7450:
		case CPU_SUBTYPE_POWERPC_7400:
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_7450)
			    return(i);
		    }
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_7400)
			    return(i);
		    }
		case CPU_SUBTYPE_POWERPC_750:
		case CPU_SUBTYPE_POWERPC_604e:
		case CPU_SUBTYPE_POWERPC_604:
		case CPU_SUBTYPE_POWERPC_603ev:
		case CPU_SUBTYPE_POWERPC_603e:
		case CPU_SUBTYPE_POWERPC_603:
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_750)
			    return(i);
		    }
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_604e)
			    return(i);
		    }
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_604)
			    return(i);
		    }
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if((fat_cputype & ~CPU_SUBTYPE_MASK) != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_603ev)
			    return(i);
		    }
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_603e)
			    return(i);
		    }
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_603)
			    return(i);
		    }
		default:
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_ALL)
			    return(i);
		    }
	    }
	    break;
	case CPU_TYPE_POWERPC64:
	    /*
	     * An exact match as not found.  So for all the PowerPC64 subtypes
	     * pick the subtype from the following order starting from a subtype
	     * that will work (contains 64-bit instructions or altivec if
	     * needed):
	     *	970 (currently only the one 64-bit subtype)
	     * For an unknown subtype pick only the ALL type if it exists.
	     */
	    switch(cpusubtype & ~CPU_SUBTYPE_MASK){
		case CPU_SUBTYPE_POWERPC_ALL:
		    /*
		     * The CPU_SUBTYPE_POWERPC_ALL is only used by the development
		     * environment tools when building a generic ALL type binary.
		     * In the case of a non-exact match we pick the most current
		     * processor.
		     */
		case CPU_SUBTYPE_POWERPC_970:
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_970)
			    return(i);
		    }
		default:
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != cputype)
			    continue;
			if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
				CPU_SUBTYPE_POWERPC_ALL)
			    return(i);
		    }
	    }
	    break;
	case CPU_TYPE_MC88000:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_MC88000_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_I860:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_I860_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_HPPA:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_HPPA_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_SPARC:
	    for(i = 0; i < nfat_archs; i++){
		if(fat_archs64 != NULL){
		    fat_cputype = fat_archs64[i].cputype;
		    fat_cpusubtype = fat_archs64[i].cpusubtype;
		}
		else{
		    fat_cputype = fat_archs[i].cputype;
		    fat_cpusubtype = fat_archs[i].cpusubtype;
		}
		if(fat_cputype != cputype)
		    continue;
		if((fat_cpusubtype & ~CPU_SUBTYPE_MASK) ==
			CPU_SUBTYPE_SPARC_ALL)
		    return(i);
	    }
	    break;
	case CPU_TYPE_ARM:
	case CPU_TYPE_ARM64:
	    {
		/* 
		 * ARM is straightforward, since each architecture is backward
		 * compatible with previous architectures.  So, we just take the
		 * highest that is less than our target.
		 */
		int fat_match_found = 0;
		uint32_t best_fat_arch = 0;
		for(i = 0; i < nfat_archs; i++){
		    if(fat_archs64 != NULL){
			fat_cputype = fat_archs64[i].cputype;
			fat_cpusubtype = fat_archs64[i].cpusubtype;
		    }
		    else{
			fat_cputype = fat_archs[i].cputype;
			fat_cpusubtype = fat_archs[i].cpusubtype;
		    }
		    if(fat_cputype != cputype)
			continue;
		    if(fat_cpusubtype > cpusubtype)
			continue;
		    if(!fat_match_found){
			fat_match_found = 1;
			best_fat_arch = i;
			continue;
		    }
		    if(fat_archs64 != NULL){
			if(fat_cpusubtype >
			   fat_archs64[best_fat_arch].cpusubtype)
			    best_fat_arch = i;
		    }
		    else{
			if(fat_cpusubtype >
			   fat_archs[best_fat_arch].cpusubtype)
			    best_fat_arch = i;
		    }
		}
		if(fat_match_found)
		    return(best_fat_arch);
		/*
		 * For CPU_TYPE_ARM64, we will fall back to a CPU_TYPE_ARM
		 * with the highest subtype.
		 */
		if(cputype == CPU_TYPE_ARM64){
		    int fat_match_found = 0;
		    uint32_t best_fat_arch = 0;
		    for(i = 0; i < nfat_archs; i++){
			if(fat_archs64 != NULL){
			    fat_cputype = fat_archs64[i].cputype;
			    fat_cpusubtype = fat_archs64[i].cpusubtype;
			}
			else{
			    fat_cputype = fat_archs[i].cputype;
			    fat_cpusubtype = fat_archs[i].cpusubtype;
			}
			if(fat_cputype != CPU_TYPE_ARM)
			    continue;
			if(!fat_match_found){
			    fat_match_found = 1;
			    best_fat_arch = i;
			    continue;
			}
			if(fat_archs64 != NULL){
			    if(fat_cpusubtype >
			       fat_archs64[best_fat_arch].cpusubtype)
				best_fat_arch = i;
			}
			else{
			    if(fat_cpusubtype >
			       fat_archs[best_fat_arch].cpusubtype)
				best_fat_arch = i;
			}
		    }
		    if(fat_match_found)
			return(best_fat_arch);
		}
	    }
	    break;
	default:
	    return(-1);
    }
    return(-1);
}

/*
 * NXFindBestFatArch() is passed a cputype and cpusubtype and a set of
 * fat_arch structs and selects the best one that matches (if any) and returns
 * a pointer to that fat_arch struct (or NULL).  The fat_arch structs must be
 * in the host byte order and correct such that the fat_archs really points to
 * enough memory for nfat_arch structs.  It is possible that this routine could
 * fail if new cputypes or cpusubtypes are added and an old version of this
 * routine is used.  But if there is an exact match between the cputype and
 * cpusubtype and one of the fat_arch structs this routine will always succeed.
 */
struct fat_arch *
NXFindBestFatArch(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct fat_arch *fat_archs,
uint32_t nfat_archs)
{
    int32_t i;

    i = internal_NXFindBestFatArch(cputype, cpusubtype, fat_archs, NULL,
				   nfat_archs);
    if(i == -1)
	return(NULL);
    return(fat_archs + i);
}

/* NXFindBestFatArch_64() is passed a cputype and cpusubtype and a set of
 * fat_arch_64 structs and selects the best one that matches (if any) and
 * returns a pointer to that fat_arch_64 struct (or NULL).  The fat_arch_64
 * structs must be in the host byte order and correct such that the fat_archs64
 * really points to enough memory for nfat_arch structs.  It is possible that
 * this routine could fail if new cputypes or cpusubtypes are added and an old
 * version of this routine is used.  But if there is an exact match between the
 * cputype and cpusubtype and one of the fat_arch_64 structs this routine will
 * always succeed.
 */
struct fat_arch_64 *
NXFindBestFatArch_64(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct fat_arch_64 *fat_archs64,
uint32_t nfat_archs)
{
    int32_t i;

    i = internal_NXFindBestFatArch(cputype, cpusubtype, NULL,
	    fat_archs64, nfat_archs);
    if(i == -1)
	return(NULL);
    return(fat_archs64 + i);
}

/*
 * NXCombineCpuSubtypes() returns the resulting cpusubtype when combining two
 * different cpusubtypes for the specified cputype.  If the two cpusubtypes
 * can't be combined (the specific subtypes are mutually exclusive) -1 is
 * returned indicating it is an error to combine them.  This can also fail and
 * return -1 if new cputypes or cpusubtypes are added and an old version of
 * this routine is used.  But if the cpusubtypes are the same they can always
 * be combined and this routine will return the cpusubtype pass in.
 */
cpu_subtype_t
NXCombineCpuSubtypes(
cpu_type_t cputype,
cpu_subtype_t cpusubtype1,
cpu_subtype_t cpusubtype2)
{
	/*
	 * If this is an x86_64 cputype and either subtype is the
	 * "Haswell and compatible" it does not combine with anything else.
	 */
	if(cputype == CPU_TYPE_X86_64 &&
	   (cpusubtype1 == CPU_SUBTYPE_X86_64_H ||
	    cpusubtype2 == CPU_SUBTYPE_X86_64_H))
	    return((cpu_subtype_t)-1);

	/*
	 * We now combine any i386 or x86-64 subtype to the ALL subtype.
	 */
	if(cputype == CPU_TYPE_I386)
	    return(CPU_SUBTYPE_I386_ALL);

	if(cputype == CPU_TYPE_X86_64)
	    return(CPU_SUBTYPE_X86_64_ALL);

	if((cpusubtype1 & ~CPU_SUBTYPE_MASK) ==
	   (cpusubtype2 & ~CPU_SUBTYPE_MASK))
	    return(cpusubtype1);

	switch(cputype){
	case CPU_TYPE_MC680x0:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC680x0_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68030_ONLY &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68040)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC680x0_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68030_ONLY &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC68040)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040)
		return((cpu_subtype_t)-1);
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040 &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68030_ONLY)
		return(CPU_SUBTYPE_MC68030_ONLY);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC68040)
		return(CPU_SUBTYPE_MC68040);
	    break; /* logically can't get here */

	case CPU_TYPE_POWERPC:
	    /*
	     * Combining with the ALL type becomes the other type. Combining
	     * anything with the 601 becomes 601.  All other non exact matches
	     * combine to the higher value subtype.
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_ALL)
		return(cpusubtype2);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_ALL)
		return(cpusubtype1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_601 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_POWERPC_601)
		return(CPU_SUBTYPE_POWERPC_601);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) >
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK))
		return(cpusubtype1);
	    else
		return(cpusubtype2);
	    break; /* logically can't get here */

	case CPU_TYPE_MC88000:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88000_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88110)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88000_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_MC88110)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC88110 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_MC88110)
		return(CPU_SUBTYPE_MC88110);

	    break; /* logically can't get here */

	case CPU_TYPE_I860:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_860)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_I860_860)
		return((cpu_subtype_t)-1);

	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_I860_860 ||
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_I860_860)
		return(CPU_SUBTYPE_I860_860);
	    break; /* logically can't get here */

	case CPU_TYPE_HPPA:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_ALL &&
	       (cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_7100LC)
		return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_ALL &&
	       (cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_HPPA_7100LC)
		return((cpu_subtype_t)-1);

	    return(CPU_SUBTYPE_HPPA_7100LC);
	    break; /* logically can't get here */

	case CPU_TYPE_SPARC:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_SPARC_ALL)
			return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_SPARC_ALL)
			return((cpu_subtype_t)-1);
	    break; /* logically can't get here */

	case CPU_TYPE_ARM:
	    /*
	     * Combinability matrix for ARM:
	     *            V4T      V5  XSCALE      V6     V7   ALL
	     *            ~~~      ~~  ~~~~~~      ~~     ~~   ~~~
	     * V4T        V4T      V5  XSCALE      V6     V7   ALL
	     * V5          V5      V5      --      V6     V7   ALL
	     * XSCALE  XSCALE      --  XSCALE      --     --   ALL
	     * V6          V6      V6      --      V6     V7   ALL
	     * V7          V7      V7      --      V7     V7   ALL
	     * ALL        ALL     ALL     ALL     ALL     ALL  ALL
	     */
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_ARM_ALL)
		return(cpusubtype2);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) == CPU_SUBTYPE_ARM_ALL)
		return(cpusubtype1);
	    switch((cpusubtype1 & ~CPU_SUBTYPE_MASK)){
		case CPU_SUBTYPE_ARM_V7:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_XSCALE:
			    return((cpu_subtype_t)-1);
			default:
			    return(CPU_SUBTYPE_ARM_V7);
		    }
		case CPU_SUBTYPE_ARM_V6:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_XSCALE:
			    return((cpu_subtype_t)-1);
			default:
			    return(CPU_SUBTYPE_ARM_V6);
		    }
		case CPU_SUBTYPE_ARM_XSCALE:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_V7:
			case CPU_SUBTYPE_ARM_V6:
			case CPU_SUBTYPE_ARM_V5TEJ:
			    return((cpu_subtype_t)-1);
			default:
			    return(CPU_SUBTYPE_ARM_XSCALE);
		    }
		case CPU_SUBTYPE_ARM_V5TEJ:
		    switch((cpusubtype2 & ~CPU_SUBTYPE_MASK)){
			case CPU_SUBTYPE_ARM_XSCALE:
			    return((cpu_subtype_t)-1);
			case CPU_SUBTYPE_ARM_V7:
			    return(CPU_SUBTYPE_ARM_V7);
			case CPU_SUBTYPE_ARM_V6:
			    return(CPU_SUBTYPE_ARM_V6);
			default:
			    return(CPU_SUBTYPE_ARM_V5TEJ);
		    }
		case CPU_SUBTYPE_ARM_V4T:
		    return((cpusubtype2 & ~CPU_SUBTYPE_MASK));
		default:
		    return((cpu_subtype_t)-1);
	    }

	case CPU_TYPE_ARM64:
	    if((cpusubtype1 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_ARM64_ALL)
			return((cpu_subtype_t)-1);
	    if((cpusubtype2 & ~CPU_SUBTYPE_MASK) != CPU_SUBTYPE_ARM64_ALL)
			return((cpu_subtype_t)-1);
	    break; /* logically can't get here */

	default:
	    return((cpu_subtype_t)-1);
	}
	return((cpu_subtype_t)-1); /* logically can't get here */
}
#endif /* !defined(RLD) */
