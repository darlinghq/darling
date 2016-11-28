/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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


#ifndef _IOKIT_IOPOWERCONNECTION_H
#define _IOKIT_IOPOWERCONNECTION_H

#include <IOKit/IOService.h>
#include <IOKit/pwr_mgt/IOPM.h>

/*! @class IOPowerConnection
 *  Do not use IOPowerConnection. This class is an implementation detail defined
 *  for IOPM's management of the IORegistry IOPower plane.
 *
 *  Only Kernel IOKit power management should reference the IOPowerConnection class.
 */

class IOPowerConnection : public IOService
{
    OSDeclareDefaultStructors(IOPowerConnection)

protected:
    /*! @field parentKnowsState	true: parent knows state of its domain
					used by child */
    bool            stateKnown;

    /*! @field currentPowerFlags	power flags which describe  the current state of the power domain
					used by child */
    IOPMPowerFlags 	currentPowerFlags;

    /*! @field desiredDomainState	state number which corresponds to the child's desire
					used by parent */
    unsigned long	desiredDomainState;

    /*! @field requestFlag		set to true when desiredDomainState is set */
    bool            requestFlag;

    /*! @field preventIdleSleepFlag	true if child has this bit set in its desired state
					used by parent */
    unsigned long	preventIdleSleepFlag;

    /*! @field preventSystemSleepFlag	true if child has this bit set in its desired state
					used by parent */
    unsigned long	preventSystemSleepFlag;

    /*! @field awaitingAck		true if child has not yet acked our notification
					used by parent */
    bool            awaitingAck;

    /*! @field readyFlag		true if the child has been added as a power child
					used by parent */
	bool            readyFlag;

#ifdef XNU_KERNEL_PRIVATE
public:
    bool            delayChildNotification;
#endif

public:
    /*! @function setParentKnowsState
        @abstract Sets the stateKnown variable.
        @discussion Called by the parent when the object is created and called by the child when it discovers that the parent now knows its state. */
    void setParentKnowsState (bool );

    /*! @function setParentCurrentPowerFlags
        @abstract Sets the currentPowerFlags variable.
        @discussion Called by the parent when the object is created and called by the child when it discovers that the parent state is changing. */
    void setParentCurrentPowerFlags (IOPMPowerFlags );

    /*! @function parentKnowsState
        @abstract Returns the stateKnown variable. */
    bool parentKnowsState (void );

    /*! @function parentCurrentPowerFlags
        @abstract Returns the currentPowerFlags variable. */
    IOPMPowerFlags parentCurrentPowerFlags (void );

    /*! @function setDesiredDomainState
        @abstract Sets the desiredDomainState variable.
        @discussion Called by the parent. */
    void setDesiredDomainState (unsigned long );

    /*! @function getDesiredDomainState
        @abstract Returns the desiredDomainState variable.
    @discussion Called by the parent. */
    unsigned long getDesiredDomainState ( void );

    /*! @function setChildHasRequestedPower
        @abstract Set the flag that says that the child has called requestPowerDomainState.
    @discussion Called by the parent. */
    void setChildHasRequestedPower ( void );

    /*! @function childHasRequestedPower
        @abstract Return the flag that says whether the child has called requestPowerDomainState.
    @discussion Called by the PCI Aux Power Supply Driver to see if a device driver
        is power managed. */
    bool childHasRequestedPower ( void );
    
    /*! @function setPreventIdleSleepFlag
        @abstract Sets the preventIdleSleepFlag variable.
        @discussion Called by the parent. */
    void setPreventIdleSleepFlag (unsigned long );

    /*! @function getPreventIdleSleepFlag
        @abstract Returns the preventIdleSleepFlag variable.
    @discussion Called by the parent. */
    bool getPreventIdleSleepFlag ( void );
    
    /*! @function setPreventSystemSleepFlag
        @abstract Sets the preventSystemSleepFlag variable.
        @discussion Called by the parent. */
    void setPreventSystemSleepFlag (unsigned long );

    /*! @function getPreventSystemSleepFlag
        @abstract Returns the preventSystemSleepFlag variable.
        @discussion Called by the parent. */
    bool getPreventSystemSleepFlag ( void );
    
    /*! @function setAwaitingAck
        @abstract Sets the awaitingAck variable.
        @discussion Called by the parent. */
    void setAwaitingAck ( bool );

    /*! @function getAwaitingAck
        @abstract Returns the awaitingAck variable.
        @discussion Called by the parent. */
    bool getAwaitingAck ( void );

    /*! @function setReadyFlag
        @abstract Sets the readyFlag variable.
        @discussion Called by the parent. */
	void setReadyFlag( bool flag );

    /*! @function getReadyFlag
        @abstract Returns the readyFlag variable.
        @discussion Called by the parent. */
	bool getReadyFlag( void ) const;
};

#endif /* ! _IOKIT_IOPOWERCONNECTION_H */

