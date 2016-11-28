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

#ifndef _IOKIT_IODMACONTROLLER_H
#define _IOKIT_IODMACONTROLLER_H

#include <IOKit/IODMACommand.h>
#include <IOKit/IODMAEventSource.h>
#include <IOKit/IOService.h>

class IODMAEventSource;

class IODMAController : public IOService
{
  OSDeclareAbstractStructors(IODMAController);
  
  friend class IODMAEventSource;
  
 private:
  IOService       *_provider;
  const OSSymbol  *_dmaControllerName;
  
 protected:
  virtual void registerDMAController(IOOptionBits options = 0);  
  virtual IOReturn initDMAChannel(IOService *provider, IODMAEventSource *dmaES, UInt32 *dmaIndex, UInt32 reqIndex) = 0;
  virtual IOReturn startDMACommand(UInt32 dmaIndex, IODMACommand *dmaCommand, IODirection direction,
				   IOByteCount byteCount = 0, IOByteCount byteOffset = 0) = 0;
  virtual IOReturn stopDMACommand(UInt32 dmaIndex, bool flush = false, uint64_t timeout = UINT64_MAX) = 0;
  virtual void completeDMACommand(IODMAEventSource *dmaES, IODMACommand *dmaCommand);
  virtual void notifyDMACommand(IODMAEventSource *dmaES, IODMACommand *dmaCommand, IOReturn status, IOByteCount actualByteCount, AbsoluteTime timeStamp);
  virtual IOReturn queryDMACommand(UInt32 dmaIndex, IODMACommand **dmaCommand, IOByteCount *transferCount, bool waitForIdle = false) = 0;
  virtual IOByteCount getFIFODepth(UInt32 dmaIndex, IODirection direction) = 0;
  virtual IOReturn setFIFODepth(UInt32 dmaIndex, IOByteCount depth) = 0;
  virtual IOByteCount validFIFODepth(UInt32 dmaIndex, IOByteCount depth, IODirection direction) = 0;
  virtual IOReturn setFrameSize(UInt32 dmaIndex, UInt8 byteCount) = 0;
  virtual IOReturn setDMAConfig(UInt32 dmaIndex, IOService *provider, UInt32 reqIndex) = 0;
  virtual bool validDMAConfig(UInt32 dmaIndex, IOService *provider, UInt32 reqIndex) = 0;
  
 public:
  static const OSSymbol *createControllerName(UInt32 phandle);
  static IODMAController *getController(IOService *provider, UInt32 dmaIndex);
  
  virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
};


#endif /* _IOKIT_IODMACONTROLLER_H */
