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


#ifndef _DiscRecordingUI_H_
#define _DiscRecordingUI_H_

#import <Foundation/Foundation.h>

#import <DiscRecordingUI/NSFileManagerDelegate.h>
#import <DiscRecordingUI/NSWindowDelegate.h>
#import <DiscRecordingUI/DRDeviceChooser.h>
#import <DiscRecordingUI/DRDeviceChooserCell.h>
#import <DiscRecordingUI/DRBurnProgressPanel.h>
#import <DiscRecordingUI/DREraseProgressPanel.h>
#import <DiscRecordingUI/DRSetupPanel.h>
#import <DiscRecordingUI/DRSetupPanelDeviceFilter.h>
#import <DiscRecordingUI/DRSetupPanelPrivate.h>
#import <DiscRecordingUI/DRBurnSetupPanel.h>
#import <DiscRecordingUI/DRBurnSetupPanelPrivate.h>
#import <DiscRecordingUI/DREraseSetupPanel.h>
#import <DiscRecordingUI/DREraseSetupPanelPrivate.h>
#import <DiscRecordingUI/DRMoveHelper.h>
#import <DiscRecordingUI/DRUIKitPanelController.h>
#import <DiscRecordingUI/DRUIPrivateFrameworkClassForFindingBundle.h>
#import <DiscRecordingUI/DRUIPrivateFrameworkInitializationClass.h>
#import <DiscRecordingUI/DRLazyIconRefImageRep.h>
#import <DiscRecordingUI/DRBurnSessionSetupPanelGlueObject.h>
#import <DiscRecordingUI/DRBurnSessionProgressPanelGlueObject.h>
#import <DiscRecordingUI/DREraseSessionSetupPanelGlueObject.h>
#import <DiscRecordingUI/DREraseSessionProgressPanelGlueObject.h>
#import <DiscRecordingUI/DRMediaEraseController.h>
#import <DiscRecordingUI/DRMediaEraseControllerInternal.h>
#import <DiscRecordingUI/DRDeviceObjectController.h>
#import <DiscRecordingUI/DRDeviceArrayController.h>
#import <DiscRecordingUI/DROpticalDiscEraseView.h>

void* DRBurnSessionBeginProgressDialog(void);
void* DRBurnSessionCreate(void);
void* DRBurnSessionGetBurn(void);
void* DRBurnSessionGetTypeID(void);
void* DRBurnSessionSetBurn(void);
void* DRBurnSessionSetupDialog(void);
void* DREraseSessionBeginProgressDialog(void);
void* DREraseSessionCreate(void);
void* DREraseSessionGetErase(void);
void* DREraseSessionGetTypeID(void);
void* DREraseSessionSetErase(void);
void* DREraseSessionSetupDialog(void);

#endif
