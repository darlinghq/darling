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


#ifndef _CoreAudioKit_H_
#define _CoreAudioKit_H_

#import <Foundation/Foundation.h>

#import <CoreAudioKit/AUAudioUnitHostViewProtocol.h>
#import <CoreAudioKit/AUAudioUnitRemoteViewProtocol.h>
#import <CoreAudioKit/AUAudioUnitViewProtocol.h>
#import <CoreAudioKit/AUCocoaUIBase.h>
#import <CoreAudioKit/AUCustomViewPersistentData.h>
#import <CoreAudioKit/BTLEConnectionTable.h>
#import <CoreAudioKit/CAAUViewToolTipHandling.h>
#import <CoreAudioKit/CBCentralManagerDelegate.h>
#import <CoreAudioKit/CBPeripheralDelegate.h>
#import <CoreAudioKit/CBPeripheralManagerDelegate.h>
#import <CoreAudioKit/NSControlTextEditingDelegate.h>
#import <CoreAudioKit/NSExtensionRequestHandling.h>
#import <CoreAudioKit/NSOutlineViewDataSource.h>
#import <CoreAudioKit/NSTableViewDataSource.h>
#import <CoreAudioKit/NSTableViewDelegate.h>
#import <CoreAudioKit/NSWindowDelegate.h>
#import <CoreAudioKit/AudioBox.h>
#import <CoreAudioKit/AUNSButtonActionOnMouseDown.h>
#import <CoreAudioKit/CACentralTableViewCell.h>
#import <CoreAudioKit/CABTLEMIDIWindowController.h>
#import <CoreAudioKit/CABTLEMIDIImpl.h>
#import <CoreAudioKit/AUViewController.h>
#import <CoreAudioKit/AUAudioUnitViewConfiguration.h>
#import <CoreAudioKit/AUNSDeadButton.h>
#import <CoreAudioKit/AUAudioUnitRemoteViewController.h>
#import <CoreAudioKit/CAToolTipEditTextField.h>
#import <CoreAudioKit/CAToolTipParameterStrip.h>
#import <CoreAudioKit/CAToolTipPopupMenuStrip.h>
#import <CoreAudioKit/CAAxisSettings.h>
#import <CoreAudioKit/CAAppleAUGraphView.h>
#import <CoreAudioKit/AUNSFineSlider.h>
#import <CoreAudioKit/CAFilterControl.h>
#import <CoreAudioKit/CAInterDeviceAudioViewController.h>
#import <CoreAudioKit/AMSNetworkDeviceCapabilitiesCellView.h>
#import <CoreAudioKit/AUNSFlippedView.h>
#import <CoreAudioKit/AUAudioUnitViewService.h>
#import <CoreAudioKit/AUGenericView.h>
#import <CoreAudioKit/AUParameterClump.h>
#import <CoreAudioKit/CAUIActionCell.h>
#import <CoreAudioKit/TransportInfo.h>
#import <CoreAudioKit/AMSNetworkBoxGroup.h>
#import <CoreAudioKit/AMSNetworkDeviceOutlineView.h>
#import <CoreAudioKit/CANetworkBrowserImpl.h>
#import <CoreAudioKit/CANetworkBrowserWindowController.h>
#import <CoreAudioKit/AUCocoaPropertyControlBase.h>
#import <CoreAudioKit/AUCPULoadView.h>
#import <CoreAudioKit/iDamDevice.h>
#import <CoreAudioKit/AUDiskStreamingCheckbox.h>
#import <CoreAudioKit/AUMeterView.h>
#import <CoreAudioKit/AMSNetworkInfoView.h>
#import <CoreAudioKit/AURenderQualityPopUp.h>
#import <CoreAudioKit/AUChannelLayoutPopUp.h>
#import <CoreAudioKit/iDamDeviceImageView.h>
#import <CoreAudioKit/AMSBTLEConnectionManager.h>
#import <CoreAudioKit/AMSBTLEAdvertisementManager.h>
#import <CoreAudioKit/AUPannerViewLoader.h>
#import <CoreAudioKit/AUPannerView.h>
#import <CoreAudioKit/AUPannerViewPriv.h>
#import <CoreAudioKit/CAAppleAU_ToolTipWindow.h>
#import <CoreAudioKit/CACustomToolTipView.h>
#import <CoreAudioKit/CAAttenuationView.h>
#import <CoreAudioKit/CAGraphView.h>
#import <CoreAudioKit/CAUITableView.h>
#import <CoreAudioKit/CAAppleEQGraphView.h>
#import <CoreAudioKit/iDamDeviceNameField.h>
#import <CoreAudioKit/AMSBTLEPeripheral.h>
#import <CoreAudioKit/CAPannerView.h>
#import <CoreAudioKit/CASurroundPannerView.h>
#import <CoreAudioKit/AUGenericViewFactory.h>
#import <CoreAudioKit/AMSNetworkDeviceTableCellView.h>
#import <CoreAudioKit/CAAppleAUCustomViewBase.h>
#import <CoreAudioKit/AUParameterStripSizingManager.h>
#import <CoreAudioKit/AUAdvancedParameterStrip.h>
#import <CoreAudioKit/AUCollapsableParameterClump.h>
#import <CoreAudioKit/AUParameterStrip.h>
#import <CoreAudioKit/NetworkPanelUtilities.h>
#import <CoreAudioKit/AUHistoryView.h>

void* logValueForNumber(void);

#endif
