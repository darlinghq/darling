/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _ScreenSaver_H_
#define _ScreenSaver_H_

#import <Foundation/Foundation.h>

#import <ScreenSaver/NSApplicationDelegate.h>
#import <ScreenSaver/NSExtensionRequestHandling.h>
#import <ScreenSaver/NSFileManagerDelegate.h>
#import <ScreenSaver/NSWindowDelegate.h>
#import <ScreenSaver/NSXPCListenerDelegate.h>
#import <ScreenSaver/ScreenSaverControl.h>
#import <ScreenSaver/ScreenSaverExtension.h>
#import <ScreenSaver/ScreenSaverExtensionContext.h>
#import <ScreenSaver/ScreenSaverExtensionHosting.h>
#import <ScreenSaver/ScreenSaverExtensionPosting.h>
#import <ScreenSaver/ScreenSaverHostExtensionContext.h>
#import <ScreenSaver/ScreenSaverView.h>
#import <ScreenSaver/ScreenSaverExtensionModule.h>
#import <ScreenSaver/ScreenSaverModules.h>
#import <ScreenSaver/ScreenSaverDefaults.h>
#import <ScreenSaver/ScreenSaverDefaultsManager.h>
#import <ScreenSaver/ScreenSaverWindow.h>
#import <ScreenSaver/ScreenSaverController.h>
#import <ScreenSaver/ScreenSaverEngine.h>
#import <ScreenSaver/ScreenSaverClockView.h>
#import <ScreenSaver/ScreenSaverClockWindow.h>
#import <ScreenSaver/ScreenSaverViewController.h>
#import <ScreenSaver/LegacyScreenSaverModule.h>
#import <ScreenSaver/ScreenSaverExtensionContext.h>
#import <ScreenSaver/ScreenSaverHostExtensionContext.h>
#import <ScreenSaver/ScreenSaverMessageTracerLogger.h>
#import <ScreenSaver/ScreenSaverExtensionView.h>
#import <ScreenSaver/ScreenSaverExtension.h>
#import <ScreenSaver/ScreenSaverConfigurationViewController.h>
#import <ScreenSaver/ScreenSaverModule.h>
#import <ScreenSaver/ScreenSaverMessage.h>
#import <ScreenSaver/ScreenSaverExtensionManager.h>
#import <ScreenSaver/LegacyScreenSaverExtensionModule.h>
#import <ScreenSaver/ScreenSaverPhotoChooser.h>

#include <ScreenSaver/inlines.h>

void* OSLogForCategory(void);
void* SSCenterRectInRect(void);
void* SSGetScaleFactor(void);
void* SSHasGfxAcceleration(void);
void* SSInitRandom(void);
void* SSInitRandomWithSeed(void);
void* SSIsAppleSignedModule(void);
void* SSModuleIsCompatibleArch(void);
void* SSPreflightModuleLoadForPath(void);
void* SSRandomFloat(void);
void* SSRandomInt(void);
void* SSRandomPointForSizeWithinBounds(void);
void* SSRunningAsNormalUser(void);
void* SSScreenIsMirrored(void);
void* SSScreenSaverWindowLevel(void);
void* SSSetCrashReporterSignature(void);
void* SSShouldUseAlernateDefault(void);
void* SSSystemScreenSaversPath(void);

#endif
