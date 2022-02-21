/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _AssertionServices_H_
#define _AssertionServices_H_

#import <Foundation/Foundation.h>

#import <AssertionServices/BSDescriptionProviding.h>
#import <AssertionServices/BSXPCCoding.h>
#import <AssertionServices/RBSAssertionObserving.h>
#import <AssertionServices/RBSServiceDelegate.h>
#import <AssertionServices/BKSWorkspace.h>
#import <AssertionServices/BKSAssertion.h>
#import <AssertionServices/BKSProcessExitContext.h>
#import <AssertionServices/BKSProcess.h>
#import <AssertionServices/BKSTerminationContext.h>
#import <AssertionServices/BKSProcessAssertion.h>
#import <AssertionServices/BKSTerminationAssertionObserverManager.h>
#import <AssertionServices/BKSTerminationAssertion.h>
#import <AssertionServices/BKSApplicationStateMonitor.h>
#import <AssertionServices/BKSLaunchdJobSpecification.h>

void* BKSAssertionErrorDescription(void);
void* BKSProcessAssertionBackgroundTimeRemaining(void);
void* BKSProcessAssertionSetExpirationHandler(void);
void* BKSProcessDiagnosticReportTypeDescription(void);
void* BKSProcessVisibilityIsForeground(void);
void* BKSProcessVisibilityMax(void);
void* BKSTerminationAssertionActiveEfficacyForBundleIdentifier(void);
void* BKSTerminationAssertionEfficacyDescription(void);
void* BKSTerminationAssertionHeldForBundleIdentifier(void);
void* BKSTerminationAssertionRegisterObserver(void);
void* BKSTerminationAssertionUnregisterObserver(void);
void* BKSWatchdogAssertionCreateForPID(void);
void* BKSWatchdogAssertionGetTypeID(void);
void* NSStringFromBKSProcessErrorCode(void);
void* NSStringFromBKSProcessExitReason(void);
void* NSStringFromBKSProcessTaskState(void);
void* NSStringFromBKSProcessVisibility(void);
void* RBSProcessLegacyStateDescriptor(void);

#endif
