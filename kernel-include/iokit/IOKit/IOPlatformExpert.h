/*
 * Copyright (c) 1998-2019 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1998 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */


#ifndef _IOKIT_IOPLATFORMEXPERT_H
#define _IOKIT_IOPLATFORMEXPERT_H

#ifdef __cplusplus
#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOInterrupts.h>
#include <IOKit/IOInterruptController.h>

extern "C" {
#endif

#include <libkern/OSTypes.h>

typedef enum {
	kCoprocessorVersionNone    =   0x00000000,
	kCoprocessorVersion1       =   0x00010000,
	kCoprocessorVersion2       =   0x00020000,
} coprocessor_type_t;


extern boolean_t PEGetMachineName( char * name, int maxLength );
extern boolean_t PEGetModelName( char * name, int maxLength );
extern int PEGetPlatformEpoch( void );

enum {
	kPEHaltCPU,
	kPERestartCPU,
	kPEHangCPU,
	kPEUPSDelayHaltCPU,
	kPEPanicRestartCPU,
	kPEPanicSync,
	kPEPagingOff,
	kPEPanicBegin,
	kPEPanicEnd,
	kPEPanicDiskShutdown,
	kPEPanicRestartCPUNoPanicEndCallouts,
	kPEPanicRestartCPUNoCallouts
};
extern int (*PE_halt_restart)(unsigned int type);
extern int PEHaltRestart(unsigned int type);

#ifdef XNU_KERNEL_PRIVATE
enum {
	kIOSystemShutdownNotificationStageProcessExit = 0,
	kIOSystemShutdownNotificationStageRootUnmount = 1,
};
extern void IOSystemShutdownNotification(int stage);

extern uint32_t gEnforceQuiesceSafety;
#endif /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
extern boolean_t IOPMRootDomainGetWillShutdown(void);
#endif /* KERNEL_PRIVATE */

// Save the Panic Info.  Returns the number of bytes saved.
extern UInt32 PESavePanicInfo(UInt8 *buffer, UInt32 length);
extern void PESavePanicInfoAction(void *buffer, UInt32 offset, UInt32 length);

/*
 * SMC requires that all data is flushed in multiples of 16 bytes at 16 byte
 * boundaries.
 */
#define PANIC_FLUSH_BOUNDARY 16

extern long PEGetGMTTimeOfDay( void );
extern void PESetGMTTimeOfDay( long secs );
extern void PEGetUTCTimeOfDay( clock_sec_t * secs, clock_usec_t * usecs );
extern void PESetUTCTimeOfDay( clock_sec_t secs, clock_usec_t usecs );

/* unless it's a "well-known" property, these will read/write out the value as raw data */

extern boolean_t PEWriteNVRAMBooleanProperty(const char *symbol, boolean_t value);
extern boolean_t PEWriteNVRAMProperty(const char *symbol, const void *value, const unsigned int len);
extern boolean_t PEWriteNVRAMPropertyWithCopy(const char *symbol, const void *value, const unsigned int len);

extern boolean_t PEReadNVRAMProperty(const char *symbol, void *value, unsigned int *len);

extern boolean_t PERemoveNVRAMProperty(const char *symbol);

extern coprocessor_type_t PEGetCoprocessorVersion( void );

#ifdef __cplusplus
} /* extern "C" */

#define kIOPlatformMapperPresentKey "IOPlatformMapperPresent"




extern OSSymbol *               gPlatformInterruptControllerName;

/*
 * IOPlatformSleepAction
 *
 * Sleep is called after power management has finished all of the power plane
 * driver power management notifications and state transitions and has
 * committed to sleep, but before the other CPUs are powered off.
 * The scheduler is still active.
 */
extern const OSSymbol *gIOPlatformSleepActionKey;

/*
 * IOPlatformWakeAction
 *
 * Wake is called with the scheduler enabled, but before
 * powering on other CPUs, so try to minimize work done in this path to speed
 * up wake time.
 */
extern const OSSymbol *gIOPlatformWakeActionKey;

/*
 * IOPlatformQuiesceAction
 *
 * Quiesce is called after all CPUs are off, scheduling is disabled,
 * and the boot CPU is about to pull the plug.
 * Mutexes and blocking are disallowed in this context and will panic.
 * Do not pass this action to super() (incl. IOService, IOPlatformExpert)
 */
extern const OSSymbol *gIOPlatformQuiesceActionKey;

/*
 * IOPlatformActiveAction
 *
 * Active is called very early in the wake path before enabling the scheduler
 * on the boot CPU.
 * Mutexes and blocking are disallowed in this context and will panic.
 * Do not pass this action to super() (incl. IOService, IOPlatformExpert)
 */
extern const OSSymbol *gIOPlatformActiveActionKey;

/*
 * IOPlatformHaltRestartAction
 *
 * Halt/Restart is called after the kernel finishes shutting down the
 * system and is ready to power off or reboot.
 *
 * It is not guaranteed to be called in non-graceful shutdown scenarios.
 */
extern const OSSymbol *gIOPlatformHaltRestartActionKey;

/*
 * IOPlatformPanicAction
 *
 * Panic is called when the system is panicking before it records a core file
 * (if it is configured to do so)
 *
 * It can be called at any time, in any context, in any state.  Don't depend
 * on anything being powered on in a useful state.
 *
 * Mutexes and blocking are disallowed in this context and will fail.
 *
 * If you hang or panic again in this callout, the panic log may not be recorded,
 * leading to the loss of field reports about customer issues.
 */
extern const OSSymbol *gIOPlatformPanicActionKey;

class IORangeAllocator;
class IONVRAMController;
class IOPMrootDomain;

class IOPlatformExpert : public IOService
{
	OSDeclareDefaultStructors(IOPlatformExpert);

private:
	long _peBootROMType;
	long _peChipSetType;
	long _peMachineType;

protected:
	IOPMrootDomain * root;
	int _pePMFeatures;
	int _pePrivPMFeatures;
	int _peNumBatteriesSupported;
	OSArray  * thePowerTree;

	bool       searchingForAdditionalParents;
	OSNumber * multipleParentKeyValue;
	int        numInstancesRegistered;

	struct ExpansionData { };
	ExpansionData *iope_reserved __unused;

	virtual void setBootROMType(long peBootROMType);
	virtual void setChipSetType(long peChipSetType);
	virtual void setMachineType(long peMachineType);

	virtual bool CheckSubTree(OSArray * inSubTree, IOService * theNub, IOService * theDevice, OSDictionary * theParent);
	virtual bool RegisterServiceInTree(IOService * theService, OSDictionary * theTreeNode, OSDictionary * theTreeParentNode, IOService * theProvider);

	virtual void PMInstantiatePowerDomains( void );

public:
	virtual bool attach( IOService * provider ) APPLE_KEXT_OVERRIDE;
	virtual bool start( IOService * provider ) APPLE_KEXT_OVERRIDE;
	virtual bool configure( IOService * provider );
	virtual IOService * createNub( OSDictionary * from );

	virtual bool compareNubName( const IOService * nub, OSString * name,
	    OSString ** matched = NULL ) const;
	virtual IOReturn getNubResources( IOService * nub );

	virtual long getBootROMType(void);
	virtual long getChipSetType(void);
	virtual long getMachineType(void);

	virtual bool getModelName( char * name, int maxLength );
	virtual bool getMachineName( char * name, int maxLength );

	virtual int haltRestart(unsigned int type);
	virtual void sleepKernel(void);

	virtual long getGMTTimeOfDay( void );
	virtual void setGMTTimeOfDay( long secs );

	virtual IOReturn getConsoleInfo( PE_Video * consoleInfo );
	virtual IOReturn setConsoleInfo( PE_Video * consoleInfo, unsigned int op );

	virtual void registerNVRAMController( IONVRAMController * nvram );

	virtual IOReturn registerInterruptController(OSSymbol *name, IOInterruptController *interruptController);
	virtual LIBKERN_RETURNS_NOT_RETAINED IOInterruptController *
	lookUpInterruptController(OSSymbol *name);

	virtual void setCPUInterruptProperties(IOService *service);
	virtual bool atInterruptLevel(void);

	virtual IOReturn callPlatformFunction(const OSSymbol *functionName,
	    bool waitForFunction,
	    void *param1, void *param2,
	    void *param3, void *param4) APPLE_KEXT_OVERRIDE;

	virtual IORangeAllocator * getPhysicalRangeAllocator(void);

	virtual bool platformAdjustService(IOService *service);

	virtual void PMRegisterDevice(IOService * theNub, IOService * theDevice);
	virtual void PMLog( const char *, unsigned long, unsigned long, unsigned long );

	virtual bool hasPMFeature(unsigned long featureMask);
	virtual bool hasPrivPMFeature(unsigned long privFeatureMask);
	virtual int  numBatteriesSupported(void);

	virtual IOByteCount savePanicInfo(UInt8 *buffer, IOByteCount length);

	virtual OSString* createSystemSerialNumberString(OSData* myProperty);

	virtual IOReturn deregisterInterruptController(OSSymbol *name);

	virtual void getUTCTimeOfDay( clock_sec_t * secs, clock_nsec_t * nsecs );
	virtual void setUTCTimeOfDay( clock_sec_t secs, clock_nsec_t nsecs );

	OSMetaClassDeclareReservedUsed(IOPlatformExpert, 0);
	OSMetaClassDeclareReservedUsed(IOPlatformExpert, 1);
	OSMetaClassDeclareReservedUsed(IOPlatformExpert, 2);
	OSMetaClassDeclareReservedUsed(IOPlatformExpert, 3);
	OSMetaClassDeclareReservedUsed(IOPlatformExpert, 4);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 5);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 6);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 7);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 8);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 9);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 10);
	OSMetaClassDeclareReservedUnused(IOPlatformExpert, 11);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class IODTNVRAM;

class IODTPlatformExpert : public IOPlatformExpert
{
	OSDeclareAbstractStructors(IODTPlatformExpert);

private:
	IODTNVRAM *dtNVRAM;

	struct ExpansionData { };
	ExpansionData *iodtpe_reserved;

public:
	virtual IOService * probe(  IOService *     provider,
	    SInt32    *     score ) APPLE_KEXT_OVERRIDE;
	virtual bool configure( IOService * provider ) APPLE_KEXT_OVERRIDE;

	virtual void processTopLevel( IORegistryEntry * root );
	virtual const char * deleteList( void ) = 0;
	virtual const char * excludeList( void ) = 0;
	virtual IOService * createNub( IORegistryEntry * from );
	virtual bool createNubs( IOService * parent, LIBKERN_CONSUMED OSIterator * iter );

	virtual bool compareNubName( const IOService * nub, OSString * name,
	    OSString ** matched = NULL ) const APPLE_KEXT_OVERRIDE;

	virtual IOReturn getNubResources( IOService * nub ) APPLE_KEXT_OVERRIDE;

	virtual bool getModelName( char * name, int maxLength ) APPLE_KEXT_OVERRIDE;
	virtual bool getMachineName( char * name, int maxLength ) APPLE_KEXT_OVERRIDE;

	virtual void registerNVRAMController( IONVRAMController * nvram ) APPLE_KEXT_OVERRIDE;

	virtual int haltRestart(unsigned int type) APPLE_KEXT_OVERRIDE;

/* virtual */ IOReturn readXPRAM(IOByteCount offset, UInt8 * buffer,
	    IOByteCount length);

/* virtual */ IOReturn writeXPRAM(IOByteCount offset, UInt8 * buffer,
	    IOByteCount length);

	virtual IOReturn readNVRAMProperty(
		IORegistryEntry * entry,
		const OSSymbol ** name, OSData ** value );

	virtual IOReturn writeNVRAMProperty(
		IORegistryEntry * entry,
		const OSSymbol * name, OSData * value );

// This returns a dictionary describing all the NVRAM partitions.
// The keys will be the partitionIDs of the form "0x52,nvram".
// The values will be OSNumbers of the partition's byte count.
/* virtual */ OSDictionary *getNVRAMPartitions(void);

/* virtual */ IOReturn readNVRAMPartition(const OSSymbol * partitionID,
	    IOByteCount offset, UInt8 * buffer,
	    IOByteCount length);

/* virtual */ IOReturn writeNVRAMPartition(const OSSymbol * partitionID,
	    IOByteCount offset, UInt8 * buffer,
	    IOByteCount length);

	virtual IOByteCount savePanicInfo(UInt8 *buffer, IOByteCount length) APPLE_KEXT_OVERRIDE;
	virtual OSString* createSystemSerialNumberString(OSData* myProperty) APPLE_KEXT_OVERRIDE;

	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 0);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 1);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 2);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 3);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 4);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 5);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 6);
	OSMetaClassDeclareReservedUnused(IODTPlatformExpert, 7);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* generic root nub of service tree */

class IOPlatformExpertDevice : public IOService
{
	OSDeclareDefaultStructors(IOPlatformExpertDevice);

private:
	IOWorkLoop *workLoop;

	struct ExpansionData { };
	ExpansionData *ioped_reserved __unused;

public:
	virtual bool initWithArgs( void * p1, void * p2,
	    void * p3, void *p4 );
	virtual bool compareName( OSString * name, OSString ** matched = NULL ) const APPLE_KEXT_OVERRIDE;

	virtual IOWorkLoop *getWorkLoop() const APPLE_KEXT_OVERRIDE;
	virtual IOReturn setProperties( OSObject * properties ) APPLE_KEXT_OVERRIDE;

	virtual void free() APPLE_KEXT_OVERRIDE;

	virtual IOReturn newUserClient( task_t owningTask, void * securityID,
	    UInt32 type, OSDictionary * properties,
	    IOUserClient ** handler) APPLE_KEXT_OVERRIDE;


	OSMetaClassDeclareReservedUnused(IOPlatformExpertDevice, 0);
	OSMetaClassDeclareReservedUnused(IOPlatformExpertDevice, 1);
	OSMetaClassDeclareReservedUnused(IOPlatformExpertDevice, 2);
	OSMetaClassDeclareReservedUnused(IOPlatformExpertDevice, 3);
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* generic nub for motherboard devices */

class IOPlatformDevice : public IOService
{
	OSDeclareDefaultStructors(IOPlatformDevice);

	struct ExpansionData { };
	ExpansionData *iopd_reserved;

public:
	virtual bool compareName( OSString * name, OSString ** matched = NULL ) const APPLE_KEXT_OVERRIDE;
	virtual IOService * matchLocation( IOService * client ) APPLE_KEXT_OVERRIDE;
	virtual IOReturn getResources( void ) APPLE_KEXT_OVERRIDE;

	OSMetaClassDeclareReservedUnused(IOPlatformDevice, 0);
	OSMetaClassDeclareReservedUnused(IOPlatformDevice, 1);
	OSMetaClassDeclareReservedUnused(IOPlatformDevice, 2);
	OSMetaClassDeclareReservedUnused(IOPlatformDevice, 3);
};

#endif /* __cplusplus */

#endif /* ! _IOKIT_IOPLATFORMEXPERT_H */
