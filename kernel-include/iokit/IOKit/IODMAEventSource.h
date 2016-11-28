/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOKIT_IODMAEVENTSOURCE_H
#define _IOKIT_IODMAEVENTSOURCE_H

#include <IOKit/IOService.h>
#include <IOKit/IODMACommand.h>
#include <IOKit/IODMAController.h>
#include <IOKit/IOEventSource.h>

class IODMAController;

class IODMAEventSource : public IOEventSource
{
  OSDeclareDefaultStructors(IODMAEventSource);
  
  friend class IODMAController;
  
 public:
  typedef void (*Action)(OSObject *owner, IODMAEventSource *dmaES, IODMACommand *dmaCommand, IOReturn status, IOByteCount actualByteCount, AbsoluteTime timeStamp);
#define IODMAEventAction IODMAEventSource::Action
  
 protected:
  virtual void completeDMACommand(IODMACommand *dmaCommand);
  virtual void notifyDMACommand(IODMACommand *dmaCommand, IOReturn status, IOByteCount actualByteCount, AbsoluteTime timeStamp);
  
 public:
  static IODMAEventSource *dmaEventSource(OSObject *owner,
					  IOService *provider,
					  Action completion = 0,
					  Action notification = 0,
					  UInt32 dmaIndex = 0);
  
  virtual IOReturn startDMACommand(IODMACommand *dmaCommand, IODirection direction, IOByteCount byteCount = 0, IOByteCount byteOffset = 0);
  virtual IOReturn stopDMACommand(bool flush = false, uint64_t timeout = UINT64_MAX);

  virtual IOReturn queryDMACommand(IODMACommand **dmaCommand, IOByteCount *transferCount, bool waitForIdle = false);

  virtual IOByteCount getFIFODepth(IODirection direction = kIODirectionNone);
  virtual IOReturn setFIFODepth(IOByteCount depth);
  virtual IOByteCount validFIFODepth(IOByteCount depth, IODirection direction);

  virtual IOReturn setFrameSize(UInt8 byteCount);

  virtual IOReturn setDMAConfig(UInt32 dmaIndex);
  virtual bool validDMAConfig(UInt32 dmaIndex);
  
 private:
  IOService       *dmaProvider;
  IODMAController *dmaController;
  UInt32          dmaIndex;
  queue_head_t    dmaCommandsCompleted;
  IOSimpleLock	  *dmaCommandsCompletedLock;
  Action          dmaCompletionAction;
  Action          dmaNotificationAction;
  bool            dmaSynchBusy;
  
  virtual bool init(OSObject *owner,
		    IOService *provider,
		    Action completion = 0,
		    Action notification = 0,
		    UInt32 dmaIndex = 0);
  virtual bool checkForWork(void) APPLE_KEXT_OVERRIDE;
  virtual void free(void) APPLE_KEXT_OVERRIDE;
};

#endif /* _IOKIT_IODMAEVENTSOURCE_H */
