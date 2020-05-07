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


#ifndef _GameController_H_
#define _GameController_H_

#import <Foundation/Foundation.h>

#import <GameController/CBCentralManagerDelegate.h>
#import <GameController/CBPeripheralDelegate.h>
#import <GameController/DigitizerValueChangedDelegate.h>
#import <GameController/GCNamedProfile.h>
#import <GameController/GameControllerClientProtocol.h>
#import <GameController/GameControllerDaemon.h>
#import <GameController/NSUserActivityDelegate.h>
#import <GameController/_GCACHomeButtonDelegate.h>
#import <GameController/_GCSnapshot.h>
#import <GameController/_GCCControllerHIDServiceInfo.h>
#import <GameController/_GCControllerManager.h>
#import <GameController/GCControllerElement.h>
#import <GameController/GCControllerButtonInput.h>
#import <GameController/_GCControllerButtonInput.h>
#import <GameController/GCControllerAxisInput.h>
#import <GameController/_GCControllerAxisInput.h>
#import <GameController/_GCControllerAxisButtonInput.h>
#import <GameController/GCControllerDirectionPad.h>
#import <GameController/_GCControllerDirectionPad.h>
#import <GameController/GCGamepad.h>
#import <GameController/GCMicroGamepad.h>
#import <GameController/_GCMicroGamepad.h>
#import <GameController/_GCGamepad.h>
#import <GameController/GCExtendedGamepad.h>
#import <GameController/_GCExtendedGamepad.h>
#import <GameController/GCControllerSFDelegate.h>
#import <GameController/GCControllerMBDelegate.h>
#import <GameController/GCControllerCBDelegate.h>
#import <GameController/GCController.h>
#import <GameController/GCEventViewController.h>
#import <GameController/_GCController.h>
#import <GameController/GCGamepadSnapshot.h>
#import <GameController/_GCGamepadSnapshot.h>
#import <GameController/GCMicroGamepadSnapshot.h>
#import <GameController/_GCMicroGamepadSnapshot.h>
#import <GameController/GCExtendedGamepadSnapshot.h>
#import <GameController/_GCExtendedGamepadSnapshot.h>
#import <GameController/GCMotion.h>
#import <GameController/_GCACHomeButton.h>
#import <GameController/_GCMFiGamepadControllerProfile.h>
#import <GameController/_GCMFiExtendedGamepadControllerProfile.h>
#import <GameController/_GCAppleTVRemoteControllerProfile.h>

void* ADClientAddValueForScalarKey(void);
void* GCCommonGetPreferencesBooleanValue(void);
void* GCControllerManagerInit(void);
void* GCControllerManagerInitXPC(void);
void* GCExtendedGamepadSnapShotDataV100FromNSData(void);
void* GCExtendedGamepadSnapshotDataFromNSData(void);
void* GCGamepadSnapShotDataV100FromNSData(void);
void* GCMicroGamepadSnapShotDataV100FromNSData(void);
void* GCMicroGamepadSnapshotDataFromNSData(void);
void* NSDataFromGCExtendedGamepadSnapShotDataV100(void);
void* NSDataFromGCExtendedGamepadSnapshotData(void);
void* NSDataFromGCGamepadSnapShotDataV100(void);
void* NSDataFromGCMicroGamepadSnapShotDataV100(void);
void* NSDataFromGCMicroGamepadSnapshotData(void);
void* _GCHIDEventCallback(void);
void* clamp(void);
void* threadRunLoopSourceCallback(void);

#endif
