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


#ifndef _IOBluetoothUI_H_
#define _IOBluetoothUI_H_

#import <Foundation/Foundation.h>

#import <IOBluetoothUI/CBCentralManagerDelegate.h>
#import <IOBluetoothUI/CBPeripheralDelegate.h>
#import <IOBluetoothUI/IOBluetoothDeviceInquiryTableViewDelegate.h>
#import <IOBluetoothUI/IOBluetoothDevicePairDelegate.h>
#import <IOBluetoothUI/NSControlTextEditingDelegate.h>
#import <IOBluetoothUI/NSMenuDelegate.h>
#import <IOBluetoothUI/NSTableViewDataSource.h>
#import <IOBluetoothUI/NSTableViewDelegate.h>
#import <IOBluetoothUI/NSTextFieldDelegate.h>
#import <IOBluetoothUI/NSWindowDelegate.h>
#import <IOBluetoothUI/AudioOptionsViewInEarDetection.h>
#import <IOBluetoothUI/AudioOptionsViewMicSwitching.h>
#import <IOBluetoothUI/AudioOptionsViewListeningMode.h>
#import <IOBluetoothUI/IOBluetooth_HIDNameFormatter.h>
#import <IOBluetoothUI/IOBluetoothUI_BatteryControl.h>
#import <IOBluetoothUI/IOBluetoothUICollectionView.h>
#import <IOBluetoothUI/IOBluetoothUIDisableProgressBar.h>
#import <IOBluetoothUI/AudioOptionsViewController.h>
#import <IOBluetoothUI/IOBluetoothPairingController.h>
#import <IOBluetoothUI/AudioOptionsViewDoubleTapExtended.h>
#import <IOBluetoothUI/IOBluetoothServiceBrowserController.h>
#import <IOBluetoothUI/IOBluetoothDeviceSelectorController.h>
#import <IOBluetoothUI/IOBluetoothConcreteDeviceSelectorController.h>
#import <IOBluetoothUI/IOBluetoothConcretePairingController.h>
#import <IOBluetoothUI/AudioOptionsViewDoubleTap.h>
#import <IOBluetoothUI/AudioOptions.h>
#import <IOBluetoothUI/IOBluetoothConcreteServiceBrowserController.h>
#import <IOBluetoothUI/OBEXObjectReceiveWindowController.h>
#import <IOBluetoothUI/OBEXObjectReceiveConcreteWindowController.h>
#import <IOBluetoothUI/TerminateEnabledModalPanel.h>
#import <IOBluetoothUI/IOBluetoothConcreteDeviceController.h>
#import <IOBluetoothUI/IOBluetoothTransferProgressUI.h>
#import <IOBluetoothUI/IOBluetoothConcreteObjectPush.h>
#import <IOBluetoothUI/IOBluetoothObjectPushUIController.h>
#import <IOBluetoothUI/IOBluetoothHardwareValidation.h>
#import <IOBluetoothUI/IOBluetoothDeviceInquiryTableView.h>
#import <IOBluetoothUI/ObjectTableDataSource.h>
#import <IOBluetoothUI/AssetHelper.h>
#import <IOBluetoothUI/PlainFormatter.h>
#import <IOBluetoothUI/IOBluetoothPasskeyDisplayNibOwner.h>
#import <IOBluetoothUI/IOBluetoothPasskeyDisplay.h>
#import <IOBluetoothUI/IOBluetoothAccessibilityIgnoredTextFieldCell.h>
#import <IOBluetoothUI/IOBluetoothAccessibilityIgnoredImageCell.h>
#import <IOBluetoothUI/IOBluetoothDeviceImageVault.h>
#import <IOBluetoothUI/FauxUIElement.h>
#import <IOBluetoothUI/IOBluetoothUIPairedColorTransformer.h>
#import <IOBluetoothUI/IOBluetoothUIBool2BWColorTransformer.h>
#import <IOBluetoothUI/IOBluetoothUIPaired2ActionTitle.h>
#import <IOBluetoothUI/IOBluetoothUIBackgroundStyle2GrayColorTransformer.h>
#import <IOBluetoothUI/IOBluetoothUIBackgroundStyle2RemoveButton.h>
#import <IOBluetoothUI/IOBluetoothUIBackgroundStyle2AlternateRemoveButton.h>
#import <IOBluetoothUI/IOBluetoothUIConnected2ImageIndicatorTransformer.h>
#import <IOBluetoothUI/IOBluetoothUIConnected2StatusTransformer.h>
#import <IOBluetoothUI/AudioOptionsViewClick.h>
#import <IOBluetoothUI/IOBluetoothDevicePairViewControllerExpansion.h>
#import <IOBluetoothUI/IOBluetoothDevicePairViewController.h>
#import <IOBluetoothUI/IOBluetoothUISortedArrayController.h>

void* BluetoothPreferencesChanged(void);
void* DeviceControllerHCIControllerNotificationHandler(void);
void* IOBluetoothAlertHCIControllerPublished(void);
void* IOBluetoothAlertHCIPowerStateOn(void);
void* IOBluetoothDecodeCommand(void);
void* IOBluetoothDecodeEvent(void);
void* IOBluetoothDecodeGroup(void);
void* IOBluetoothDecodeManufacturerName(void);
void* IOBluetoothDecodeStatus(void);
void* IOBluetoothDeviceSelectorRunPanelWithAttributes(void);
void* IOBluetoothGetDefaultNameOfL2CAPPSMString(void);
void* IOBluetoothGetDefaultNameOfServiceString(void);
void* IOBluetoothGetDeviceSelectorController(void);
void* IOBluetoothGetGenericTypeStringForDevice(void);
void* IOBluetoothGetGenericTypeStringForDeviceClasses(void);
void* IOBluetoothGetLocalizedUIStringForKey(void);
void* IOBluetoothGetMajorClassOfDeviceString(void);
void* IOBluetoothGetMajorClassOfDeviceStringClass(void);
void* IOBluetoothGetMinorClassOfDeviceString(void);
void* IOBluetoothGetMinorClassOfDeviceStringClass(void);
void* IOBluetoothGetPairingController(void);
void* IOBluetoothIsOptionKeyDown(void);
void* IOBluetoothIsServiceUsableByUser(void);
void* IOBluetoothIsShiftKeyDown(void);
void* IOBluetoothOpenBluetoothHelpAtPath(void);
void* IOBluetoothPairingControllerRunPanelWithAttributes(void);
void* IOBluetoothPreferenceGetMenuExtraShowState(void);
void* IOBluetoothPreferenceSetMenuExtraShowState(void);
void* IOBluetoothServiceBrowserControllerBrowseDevices(void);
void* IOBluetoothServiceBrowserControllerBrowseDevicesAsSheet(void);
void* IOBluetoothServiceBrowserControllerCreate(void);
void* IOBluetoothServiceBrowserControllerDiscover(void);
void* IOBluetoothServiceBrowserControllerDiscoverAsSheet(void);
void* IOBluetoothServiceBrowserControllerDiscoverWithDeviceAttributes(void);
void* IOBluetoothServiceBrowserControllerSetOptions(void);
void* IOBluetoothValidateHardware(void);
void* IOBluetoothValidateHardwareWithDescription(void);
void* ISCopyDeviceIconPath(void);
void* ISShouldProposeToAddDeviceToISync(void);
void* ImageForTypeStr(void);
void* ObjectTableDataSource_ColumnSorter(void);
void* TypeForModelStr(void);
void* _IOBluetoothUI_AlignCGPointToUserSpace(void);
void* _IOBluetoothUI_AlignCGRectToUserSpace(void);
void* _IOBluetoothUI_AlignPointToUserSpace(void);
void* _IOBluetoothUI_AlignRectToUserSpace(void);
void* isDebugLogEnabled(void);
void* registerValueTransformers(void);

#endif
