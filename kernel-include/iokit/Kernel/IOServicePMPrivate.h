/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_IOSERVICEPMPRIVATE_H
#define _IOKIT_IOSERVICEPMPRIVATE_H

#include <IOKit/IOCommand.h>
#include <IOKit/IOEventSource.h>

//******************************************************************************
// PM command types
//******************************************************************************

enum {
	/* Command Types */
	kIOPMRequestTypeInvalid                     = 0x00,
	kIOPMRequestTypePMStop                      = 0x01,
	kIOPMRequestTypeAddPowerChild1              = 0x02,
	kIOPMRequestTypeAddPowerChild2              = 0x03,
	kIOPMRequestTypeAddPowerChild3              = 0x04,
	kIOPMRequestTypeRegisterPowerDriver         = 0x05,
	kIOPMRequestTypeAdjustPowerState            = 0x06,
	kIOPMRequestTypePowerDomainWillChange       = 0x07,
	kIOPMRequestTypePowerDomainDidChange        = 0x08,
	kIOPMRequestTypePowerOverrideOnPriv         = 0x09,
	kIOPMRequestTypePowerOverrideOffPriv        = 0x0A,
	kIOPMRequestTypeActivityTickle              = 0x0B,
	kIOPMRequestTypeRequestPowerState           = 0x0C,
	kIOPMRequestTypeSynchronizePowerTree        = 0x0D,
	kIOPMRequestTypeRequestPowerStateOverride   = 0x0E,
	kIOPMRequestTypeSetIdleTimerPeriod          = 0x0F,
	kIOPMRequestTypeIgnoreIdleTimer             = 0x10,
	kIOPMRequestTypeQuiescePowerTree            = 0x11,

	/* Reply Types */
	kIOPMRequestTypeReplyStart                  = 0x80,
	kIOPMRequestTypeAckPowerChange              = 0x81,
	kIOPMRequestTypeAckSetPowerState            = 0x82,
	kIOPMRequestTypeAllowPowerChange            = 0x83,
	kIOPMRequestTypeCancelPowerChange           = 0x84,
	kIOPMRequestTypeInterestChanged             = 0x85,
	kIOPMRequestTypeIdleCancel                  = 0x86,
	kIOPMRequestTypeChildNotifyDelayCancel      = 0x87
};

//******************************************************************************
// PM actions - For root domain only
//******************************************************************************

struct IOPMActions;

typedef void
(*IOPMActionPowerChangeStart)(
	void *                  target,
	IOService *             service,
	IOPMActions *           actions,
	IOPMPowerStateIndex     powerState,
	IOPMPowerChangeFlags *  changeFlags,
	IOPMRequestTag          requestTag );

typedef void
(*IOPMActionPowerChangeDone)(
	void *                  target,
	IOService *             service,
	IOPMActions *           actions,
	IOPMPowerStateIndex     powerState,
	IOPMPowerChangeFlags    changeFlags,
	IOPMRequestTag          requestTag );

typedef void
(*IOPMActionPowerChangeOverride)(
	void *                  target,
	IOService *             service,
	IOPMActions *           actions,
	IOPMPowerStateIndex *   powerState,
	IOPMPowerChangeFlags *  changeFlags,
	IOPMRequestTag          requestTag );

typedef void
(*IOPMActionActivityTickle)(
	void *                  target,
	IOService *             service,
	IOPMActions *           actions );

typedef void
(*IOPMActionUpdatePowerClient)(
	void *                  target,
	IOService *             service,
	IOPMActions *           actions,
	const OSSymbol *        powerClient,
	IOPMPowerStateIndex     oldPowerState,
	IOPMPowerStateIndex     newPowerState
	);

struct IOPMActions {
	void *                          target;
	uint32_t                        parameter;
	IOPMActionPowerChangeStart      actionPowerChangeStart;
	IOPMActionPowerChangeDone       actionPowerChangeDone;
	IOPMActionPowerChangeOverride   actionPowerChangeOverride;
	IOPMActionActivityTickle        actionActivityTickle;
	IOPMActionUpdatePowerClient     actionUpdatePowerClient;
};

// IOPMActions parameter flags
enum {
	kPMActionsFlagIsDisplayWrangler = 0x00000100,
	kPMActionsFlagIsGraphicsDevice  = 0x00000200,
	kPMActionsFlagIsAudioDevice     = 0x00000400,
	kPMActionsFlagLimitPower        = 0x00000800,
	kPMActionsPCIBitNumberMask      = 0x000000ff
};

//******************************************************************************
// Internal concise representation of IOPMPowerState
struct IOPMPSEntry {
	IOPMPowerFlags      capabilityFlags;
	IOPMPowerFlags      outputPowerFlags;
	IOPMPowerFlags      inputPowerFlags;
	uint32_t            staticPower;
	uint32_t            settleUpTime;
	uint32_t            settleDownTime;
	IOPMPowerStateIndex stateOrder;
	IOPMPowerStateIndex stateOrderToIndex;
};

//******************************************************************************
// IOServicePM
//******************************************************************************

class IOServicePM : public OSObject
{
	friend class IOService;
	friend class IOPMWorkQueue;

	OSDeclareDefaultStructors( IOServicePM );

private:
// Link IOServicePM objects on IOPMWorkQueue.
	queue_chain_t           WorkChain;

// Queue of IOPMRequest objects.
	queue_head_t            RequestHead;

// IOService creator and owner.
	IOService *             Owner;

// List of interested drivers (protected by PMLock).
	IOPMinformeeList *      InterestedDrivers;

// How long to wait for controlling driver to acknowledge.
	IOReturn                DriverTimer;

// Current power management machine state.
	uint32_t                MachineState;

	thread_call_t           AckTimer;
	thread_call_t           SettleTimer;
	thread_call_t           IdleTimer;
	thread_call_t           WatchdogTimer;
	thread_call_t           SpinDumpTimer;

	IOLock  *               WatchdogLock;
	OSArray *               BlockedArray;
	uint64_t                PendingResponseDeadline;
	uint64_t                WatchdogDeadline;

// Settle time after changing power state.
	uint32_t                SettleTimeUS;
	uint32_t                IdleTimerGeneration;

// The flags describing current change note.
	IOPMPowerChangeFlags    HeadNoteChangeFlags;

// The new power state number being changed to.
	IOPMPowerStateIndex     HeadNotePowerState;

// Points to the entry in the power state array.
	IOPMPSEntry *           HeadNotePowerArrayEntry;

// Power flags supplied by all parents (domain).
	IOPMPowerFlags          HeadNoteDomainFlags;

// Power flags supplied by domain accounting for parent changes.
	IOPMPowerFlags          HeadNoteDomainTargetFlags;

// Connection attached to the changing parent.
	IOPowerConnection *     HeadNoteParentConnection;

// Power flags supplied by the changing parent.
	IOPMPowerFlags          HeadNoteParentFlags;

// Number of acks still outstanding.
	uint32_t                HeadNotePendingAcks;

// PM state lock.
	IOLock *                PMLock;

	unsigned int            InitialPowerChange          :1;
	unsigned int            InitialSetPowerState        :1;
	unsigned int            DeviceOverrideEnabled       :1;
	unsigned int            DoNotPowerDown              :1;
	unsigned int            ParentsKnowState            :1;
	unsigned int            StrictTreeOrder             :1;
	unsigned int            IdleTimerStopped            :1;
	unsigned int            AdjustPowerScheduled        :1;

	unsigned int            IsPreChange                 :1;
	unsigned int            DriverCallBusy              :1;
	unsigned int            PCDFunctionOverride         :1;
	unsigned int            IdleTimerIgnored            :1;
	unsigned int            HasAdvisoryDesire           :1;
	unsigned int            AdvisoryTickleUsed          :1;
	unsigned int            ResetPowerStateOnWake       :1;

// Time of last device activity.
	AbsoluteTime            DeviceActiveTimestamp;
	AbsoluteTime            MaxPowerStateEntryTime;
	AbsoluteTime            MaxPowerStateExitTime;

// Used to protect activity flag.
	IOLock *                ActivityLock;

// Idle timer's period in seconds.
	unsigned long           IdleTimerPeriod;
	unsigned long           IdleTimerMinPowerState;
	unsigned long           NextIdleTimerPeriod;
	AbsoluteTime            IdleTimerStartTime;

// Power state desired by a subclassed device object.
	IOPMPowerStateIndex     DeviceDesire;

// This is the power state we desire currently.
	IOPMPowerStateIndex     DesiredPowerState;

// This is what our parent thinks our need is.
	IOPMPowerFlags          PreviousRequestPowerFlags;

// Cache result from getName(), used in logging.
	const char *            Name;

// Number of power states in the power array.
	IOPMPowerStateIndex     NumberOfPowerStates;

// Ordered highest power state in the power array.
	IOPMPowerStateIndex     HighestPowerState;

// Power state array.
	IOPMPSEntry *           PowerStates;

// The controlling driver.
	IOService *             ControllingDriver;

// Our current power state.
	IOPMPowerStateIndex     CurrentPowerState;

// Logical OR of power flags for each power domain parent.
	IOPMPowerFlags          ParentsCurrentPowerFlags;

// The highest power state we can achieve in current power domain.
	IOPMPowerStateIndex     MaxPowerState;

// Logical OR of all output power flags in the power state array.
	IOPMPowerFlags          MergedOutputPowerFlags;

// OSArray which manages responses from notified apps and clients.
	OSArray *               ResponseArray;
	OSArray *               NotifyClientArray;

// Used to uniquely identify power management notification to apps and clients.
	UInt16                  SerialNumber;

// Used to communicate desired function to tellClientsWithResponse().
// This is used because it avoids changing the signatures of the affected virtual methods.
	int                     OutOfBandParameter;

	AbsoluteTime            DriverCallStartTime;
	IOPMPowerFlags          CurrentCapabilityFlags;
	unsigned long           CurrentPowerConsumption;
	IOPMPowerStateIndex     TempClampPowerState;
	OSArray *               NotifyChildArray;
	OSDictionary *          PowerClients;
	thread_call_t           DriverCallEntry;
	void *                  DriverCallParamPtr;
	IOItemCount             DriverCallParamCount;
	IOItemCount             DriverCallParamSlots;
	uint32_t                DriverCallReason;
	uint32_t                OutOfBandMessage;
	uint32_t                TempClampCount;
	uint32_t                OverrideMaxPowerState;
	uint32_t                DeviceUsablePowerState;

// Protected by ActivityLock - BEGIN
	IOPMPowerStateIndex     ActivityTicklePowerState;
	IOPMPowerStateIndex     AdvisoryTicklePowerState;
	uint32_t                ActivityTickleCount;
	uint32_t                DeviceWasActive     : 1;
	uint32_t                AdvisoryTickled     : 1;
// Protected by ActivityLock - END

	uint32_t                WaitReason;
	uint32_t                SavedMachineState;

// Protected by PMLock - BEGIN
	struct {
		uint32_t            PMStop              : 1;
		uint32_t            PMDriverCallWait    : 1;
	} LockedFlags;

	queue_head_t            PMDriverCallQueue;
	OSSet *                 InsertInterestSet;
	OSSet *                 RemoveInterestSet;

// IOReporter Data
	uint32_t                ReportClientCnt;
	void *                  ReportBuf;
// Protected by PMLock - END

#if PM_VARS_SUPPORT
	IOPMprot *              PMVars;
#endif

	IOPMActions             PMActions;

// Serialize IOServicePM state for debug output.
	IOReturn gatedSerialize( OSSerialize * s ) const;
	virtual bool serialize( OSSerialize * s ) const APPLE_KEXT_OVERRIDE;

// PM log and trace
	void pmPrint( uint32_t event, uintptr_t param1, uintptr_t param2 ) const;
	void pmTrace( uint32_t event, uint32_t eventFunc, uintptr_t param1, uintptr_t param2 ) const;
};

#define fOwner                      pwrMgt->Owner
#define fInterestedDrivers          pwrMgt->InterestedDrivers
#define fDriverTimer                pwrMgt->DriverTimer
#define fMachineState               pwrMgt->MachineState
#define fAckTimer                   pwrMgt->AckTimer
#define fSettleTimer                pwrMgt->SettleTimer
#define fIdleTimer                  pwrMgt->IdleTimer
#define fWatchdogTimer              pwrMgt->WatchdogTimer
#define fWatchdogDeadline           pwrMgt->WatchdogDeadline
#define fWatchdogLock               pwrMgt->WatchdogLock
#define fBlockedArray               pwrMgt->BlockedArray
#define fPendingResponseDeadline    pwrMgt->PendingResponseDeadline
#define fSettleTimeUS               pwrMgt->SettleTimeUS
#define fIdleTimerGeneration        pwrMgt->IdleTimerGeneration
#define fHeadNoteChangeFlags        pwrMgt->HeadNoteChangeFlags
#define fHeadNotePowerState         pwrMgt->HeadNotePowerState
#define fHeadNotePowerArrayEntry    pwrMgt->HeadNotePowerArrayEntry
#define fHeadNoteDomainFlags        pwrMgt->HeadNoteDomainFlags
#define fHeadNoteDomainTargetFlags  pwrMgt->HeadNoteDomainTargetFlags
#define fHeadNoteParentConnection   pwrMgt->HeadNoteParentConnection
#define fHeadNoteParentFlags        pwrMgt->HeadNoteParentFlags
#define fHeadNotePendingAcks        pwrMgt->HeadNotePendingAcks
#define fPMLock                     pwrMgt->PMLock
#define fInitialPowerChange         pwrMgt->InitialPowerChange
#define fInitialSetPowerState       pwrMgt->InitialSetPowerState
#define fDeviceOverrideEnabled      pwrMgt->DeviceOverrideEnabled
#define fDoNotPowerDown             pwrMgt->DoNotPowerDown
#define fParentsKnowState           pwrMgt->ParentsKnowState
#define fStrictTreeOrder            pwrMgt->StrictTreeOrder
#define fIdleTimerStopped           pwrMgt->IdleTimerStopped
#define fAdjustPowerScheduled       pwrMgt->AdjustPowerScheduled
#define fIsPreChange                pwrMgt->IsPreChange
#define fDriverCallBusy             pwrMgt->DriverCallBusy
#define fPCDFunctionOverride        pwrMgt->PCDFunctionOverride
#define fIdleTimerIgnored           pwrMgt->IdleTimerIgnored
#define fHasAdvisoryDesire          pwrMgt->HasAdvisoryDesire
#define fAdvisoryTickleUsed         pwrMgt->AdvisoryTickleUsed
#define fResetPowerStateOnWake      pwrMgt->ResetPowerStateOnWake
#define fDeviceActiveTimestamp      pwrMgt->DeviceActiveTimestamp
#define fMaxPowerStateEntryTime     pwrMgt->MaxPowerStateEntryTime
#define fMaxPowerStateExitTime      pwrMgt->MaxPowerStateExitTime
#define fActivityLock               pwrMgt->ActivityLock
#define fIdleTimerPeriod            pwrMgt->IdleTimerPeriod
#define fIdleTimerMinPowerState     pwrMgt->IdleTimerMinPowerState
#define fNextIdleTimerPeriod        pwrMgt->NextIdleTimerPeriod
#define fIdleTimerStartTime         pwrMgt->IdleTimerStartTime
#define fDeviceDesire               pwrMgt->DeviceDesire
#define fDesiredPowerState          pwrMgt->DesiredPowerState
#define fPreviousRequestPowerFlags  pwrMgt->PreviousRequestPowerFlags
#define fName                       pwrMgt->Name
#define fNumberOfPowerStates        pwrMgt->NumberOfPowerStates
#define fHighestPowerState          pwrMgt->HighestPowerState
#define fPowerStates                pwrMgt->PowerStates
#define fControllingDriver          pwrMgt->ControllingDriver
#define fCurrentPowerState          pwrMgt->CurrentPowerState
#define fParentsCurrentPowerFlags   pwrMgt->ParentsCurrentPowerFlags
#define fMaxPowerState              pwrMgt->MaxPowerState
#define fMergedOutputPowerFlags     pwrMgt->MergedOutputPowerFlags
#define fResponseArray              pwrMgt->ResponseArray
#define fNotifyClientArray          pwrMgt->NotifyClientArray
#define fSerialNumber               pwrMgt->SerialNumber
#define fOutOfBandParameter         pwrMgt->OutOfBandParameter
#define fDriverCallStartTime        pwrMgt->DriverCallStartTime
#define fCurrentCapabilityFlags     pwrMgt->CurrentCapabilityFlags
#define fCurrentPowerConsumption    pwrMgt->CurrentPowerConsumption
#define fTempClampPowerState        pwrMgt->TempClampPowerState
#define fNotifyChildArray           pwrMgt->NotifyChildArray
#define fPowerClients               pwrMgt->PowerClients
#define fDriverCallEntry            pwrMgt->DriverCallEntry
#define fDriverCallParamPtr         pwrMgt->DriverCallParamPtr
#define fDriverCallParamCount       pwrMgt->DriverCallParamCount
#define fDriverCallParamSlots       pwrMgt->DriverCallParamSlots
#define fDriverCallReason           pwrMgt->DriverCallReason
#define fOutOfBandMessage           pwrMgt->OutOfBandMessage
#define fTempClampCount             pwrMgt->TempClampCount
#define fOverrideMaxPowerState      pwrMgt->OverrideMaxPowerState
#define fDeviceUsablePowerState     pwrMgt->DeviceUsablePowerState
#define fActivityTicklePowerState   pwrMgt->ActivityTicklePowerState
#define fAdvisoryTicklePowerState   pwrMgt->AdvisoryTicklePowerState
#define fActivityTickleCount        pwrMgt->ActivityTickleCount
#define fDeviceWasActive            pwrMgt->DeviceWasActive
#define fAdvisoryTickled            pwrMgt->AdvisoryTickled
#define fWaitReason                 pwrMgt->WaitReason
#define fSavedMachineState          pwrMgt->SavedMachineState
#define fLockedFlags                pwrMgt->LockedFlags
#define fPMDriverCallQueue          pwrMgt->PMDriverCallQueue
#define fInsertInterestSet          pwrMgt->InsertInterestSet
#define fRemoveInterestSet          pwrMgt->RemoveInterestSet
#define fReportClientCnt            pwrMgt->ReportClientCnt
#define fReportBuf                  pwrMgt->ReportBuf
#define fPMVars                     pwrMgt->PMVars
#define fPMActions                  pwrMgt->PMActions

#define StateOrder(state)           (((state) < fNumberOfPowerStates)               \
	                            ? pwrMgt->PowerStates[(state)].stateOrder       \
	                            : (state))
#define StateMax(a, b)               (StateOrder((a)) < StateOrder((b)) ? (b) : (a))
#define StateMin(a, b)               (StateOrder((a)) < StateOrder((b)) ? (a) : (b))

#define kPowerStateZero             (0)

/*
 *  When an IOService is waiting for acknowledgement to a power change
 *  notification from an interested driver or the controlling driver,
 *  the ack timer is ticking every tenth of a second.
 *  (100000000 nanoseconds are one tenth of a second).
 */
#define ACK_TIMER_PERIOD            100000000

#if defined(__i386__) || defined(__x86_64__)
#define WATCHDOG_SLEEP_TIMEOUT      (180)   // 180 secs
#define WATCHDOG_WAKE_TIMEOUT       (180)   // 180  secs
#else
#define WATCHDOG_SLEEP_TIMEOUT      (180)   // 180 secs
#define WATCHDOG_WAKE_TIMEOUT       (180)   // 180 secs
#endif

// Max wait time in microseconds for kernel priority and capability clients
// with async message handlers to acknowledge.
//
#define kPriorityClientMaxWait      (90 * 1000 * 1000)
#define kCapabilityClientMaxWait    (240 * 1000 * 1000)

// Attributes describing a power state change.
// See IOPMPowerChangeFlags data type.
//
#define kIOPMParentInitiated        0x0001  // power change initiated by our  parent
#define kIOPMSelfInitiated          0x0002  // power change initiated by this device
#define kIOPMNotDone                0x0004  // we couldn't make this change
#define kIOPMDomainWillChange       0x0008  // change started by PowerDomainWillChangeTo
#define kIOPMDomainDidChange        0x0010  // change started by PowerDomainDidChangeTo
#define kIOPMDomainPowerDrop        0x0020  // Domain is lowering power
#define kIOPMIgnoreChildren         0x0040  // Ignore children and driver power desires
#define kIOPMSkipAskPowerDown       0x0080  // skip the ask app phase
#define kIOPMSynchronize            0x0100  // change triggered by power tree re-sync
#define kIOPMSyncNoChildNotify      0x0200  // sync root domain only, not entire tree
#define kIOPMSyncTellPowerDown      0x0400  // send the ask/will power off messages
#define kIOPMSyncCancelPowerDown    0x0800  // sleep cancel for maintenance wake
#define kIOPMInitialPowerChange     0x1000  // set for initial power change
#define kIOPMRootChangeUp           0x2000  // Root power domain change up
#define kIOPMRootChangeDown         0x4000  // Root power domain change down
#define kIOPMExpireIdleTimer        0x8000  // Accelerate idle timer expiration

#define kIOPMRootBroadcastFlags     (kIOPMSynchronize  | \
	                             kIOPMRootChangeUp | kIOPMRootChangeDown)

// Activity tickle request flags
#define kTickleTypePowerDrop        0x01
#define kTickleTypePowerRise        0x02
#define kTickleTypeActivity         0x04
#define kTickleTypeAdvisory         0x08

enum {
	kDriverCallInformPreChange,
	kDriverCallInformPostChange,
	kDriverCallSetPowerState,
	kRootDomainInformPreChange
};

struct DriverCallParam {
	OSObject *  Target;
	IOReturn    Result;
};

// values of OutOfBandParameter
enum {
	kNotifyApps,
	kNotifyPriority,
	kNotifyCapabilityChangeApps,
	kNotifyCapabilityChangePriority
};

typedef bool (*IOPMMessageFilter)(
	void * target, void * object, void * arg1, void * arg2, void * arg3 );

// used for applyToInterested
struct IOPMInterestContext {
	OSArray *               responseArray;
	OSArray *               notifyClients;
	uint16_t                serialNumber;
	uint8_t                 isPreChange;
	uint8_t                 enableTracing;
	uint32_t                maxTimeRequested;
	uint32_t                messageType;
	uint32_t                notifyType;
	uint32_t                skippedInDark;
	uint32_t                notSkippedInDark;
	IOService *             us;
	IOPMPowerStateIndex     stateNumber;
	IOPMPowerFlags          stateFlags;
	IOPMPowerChangeFlags    changeFlags;
	const char *            errorLog;
	IOPMMessageFilter       messageFilter;
};

// assertPMDriverCall() options
enum {
	kIOPMDriverCallNoInactiveCheck = 1
};

// assertPMDriverCall() method
enum {
	kIOPMDriverCallMethodUnknown       = 0,
	kIOPMDriverCallMethodSetPowerState = 1,
	kIOPMDriverCallMethodWillChange    = 2,
	kIOPMDriverCallMethodDidChange     = 3,
	kIOPMDriverCallMethodChangeDone    = 4,
	kIOPMDriverCallMethodSetAggressive = 5
};

//******************************************************************************
// PM Statistics & Diagnostics
//******************************************************************************

extern const OSSymbol *gIOPMStatsResponseTimedOut;
extern const OSSymbol *gIOPMStatsResponseCancel;
extern const OSSymbol *gIOPMStatsResponseSlow;
extern const OSSymbol *gIOPMStatsResponsePrompt;
extern const OSSymbol *gIOPMStatsDriverPSChangeSlow;

//******************************************************************************
// IOPMRequest
//******************************************************************************

class IOPMRequest : public IOCommand
{
	OSDeclareDefaultStructors( IOPMRequest );

protected:
	IOService *          fTarget;       // request target
	IOPMRequest *        fRequestNext;  // the next request in the chain
	IOPMRequest *        fRequestRoot;  // the root request in the call tree
	IOItemCount          fWorkWaitCount;// execution blocked if non-zero
	IOItemCount          fFreeWaitCount;// completion blocked if non-zero
	uint32_t             fRequestType;  // request type
	bool                 fIsQuiesceBlocker;

	IOPMCompletionAction fCompletionAction;
	void *               fCompletionTarget;
	void *               fCompletionParam;

public:
	uint32_t             fRequestTag;
	void *               fArg0;
	void *               fArg1;
	void *               fArg2;

	inline bool
	isWorkBlocked( void ) const
	{
		return fWorkWaitCount != 0;
	}

	inline bool
	isFreeBlocked( void ) const
	{
		return fFreeWaitCount != 0;
	}

	inline IOPMRequest *
	getNextRequest( void ) const
	{
		return fRequestNext;
	}

	inline IOPMRequest *
	getRootRequest( void ) const
	{
		if (fRequestRoot) {
			return fRequestRoot;
		}
#if NOT_READY
		if (fCompletionAction) {
			return (IOPMRequest *) this;
		}
#endif
		return NULL;
	}

	inline uint32_t
	getType( void ) const
	{
		return fRequestType;
	}

	inline bool
	isReplyType( void ) const
	{
		return fRequestType > kIOPMRequestTypeReplyStart;
	}

	inline IOService *
	getTarget( void ) const
	{
		return fTarget;
	}

	inline bool
	isQuiesceBlocker( void ) const
	{
		return fIsQuiesceBlocker;
	}

	inline bool
	isQuiesceType( void ) const
	{
		return (kIOPMRequestTypeQuiescePowerTree == fRequestType) &&
		       (fCompletionAction != NULL) && (fCompletionTarget != NULL);
	}

	inline void
	installCompletionAction(
		void *               target,
		IOPMCompletionAction action,
		void *               param )
	{
		fCompletionTarget = target;
		fCompletionAction = action;
		fCompletionParam  = param;
	}

	static IOPMRequest * create( void );
	bool   init( IOService * owner, IOOptionBits type );
	void   reset( void );
	bool   attachNextRequest( IOPMRequest * next );
	bool   detachNextRequest( void );
	bool   attachRootRequest( IOPMRequest * root );
	bool   detachRootRequest( void );
};

//******************************************************************************
// IOPMRequestQueue
//******************************************************************************

class IOPMRequestQueue : public IOEventSource
{
	OSDeclareDefaultStructors( IOPMRequestQueue );

public:
	typedef bool (*Action)( IOService *, IOPMRequest *, IOPMRequestQueue * );

protected:
	queue_head_t    fQueue;
	IOLock *        fLock;

	enum { kMaxDequeueCount = 256 };

	virtual bool checkForWork( void ) APPLE_KEXT_OVERRIDE;
	virtual void free( void ) APPLE_KEXT_OVERRIDE;
	virtual bool init( IOService * inOwner, Action inAction );

public:
	static  IOPMRequestQueue * create( IOService * inOwner, Action inAction );
	void    queuePMRequest( IOPMRequest * request );
	void    queuePMRequestChain( IOPMRequest ** requests, IOItemCount count );
};

//******************************************************************************
// IOPMWorkQueue
//******************************************************************************

#define WORK_QUEUE_STATS    1

class IOPMWorkQueue : public IOEventSource
{
	OSDeclareDefaultStructors( IOPMWorkQueue );

public:
	typedef bool (*Action)( IOService *, IOPMRequest *, IOPMWorkQueue * );

#if WORK_QUEUE_STATS
	uint64_t            fStatCheckForWork;
	uint64_t            fStatScanEntries;
	uint64_t            fStatQueueEmpty;
	uint64_t            fStatNoWorkDone;
#endif

protected:
	queue_head_t        fWorkQueue;
	Action              fInvokeAction;
	Action              fRetireAction;
	uint32_t            fQueueLength;
	uint32_t            fConsumerCount;
	volatile uint32_t   fProducerCount;
	IOPMRequest *       fQuiesceRequest;
	AbsoluteTime        fQuiesceStartTime;
	AbsoluteTime        fQuiesceFinishTime;

	virtual bool checkForWork( void ) APPLE_KEXT_OVERRIDE;
	virtual bool init( IOService * inOwner, Action invoke, Action retire );
	bool    checkRequestQueue( queue_head_t * queue, bool * empty );

public:
	static  IOPMWorkQueue * create( IOService * inOwner, Action invoke, Action retire );
	bool    queuePMRequest( IOPMRequest * request, IOServicePM * pwrMgt );
	void    signalWorkAvailable( void );
	void    incrementProducerCount( void );
	void    attachQuiesceRequest( IOPMRequest * quiesceRequest );
	void    finishQuiesceRequest( IOPMRequest * quiesceRequest );
};

//******************************************************************************
// IOPMCompletionQueue
//******************************************************************************

class IOPMCompletionQueue : public IOEventSource
{
	OSDeclareDefaultStructors( IOPMCompletionQueue );

public:
	typedef bool (*Action)( IOService *, IOPMRequest *, IOPMCompletionQueue * );

protected:
	queue_head_t    fQueue;

	virtual bool checkForWork( void ) APPLE_KEXT_OVERRIDE;
	virtual bool init( IOService * inOwner, Action inAction );

public:
	static  IOPMCompletionQueue * create( IOService * inOwner, Action inAction );
	bool    queuePMRequest( IOPMRequest * request );
};

#endif /* !_IOKIT_IOSERVICEPMPRIVATE_H */
